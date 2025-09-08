#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <thread>
#include <atomic>
#include <mutex>
#include <queue>
#include <chrono>
#include <functional>

namespace router_sim {

// Traffic Shaping Types
enum class ShapingType {
    TOKEN_BUCKET = 0,
    WFQ = 1,
    CBWFQ = 2,
    PQ = 3,
    CQ = 4
};

// Packet Priority Levels
enum class Priority {
    CRITICAL = 0,
    HIGH = 1,
    NORMAL = 2,
    LOW = 3,
    BEST_EFFORT = 4
};

// Traffic Class Configuration
struct TrafficClass {
    int class_id;
    std::string name;
    Priority priority;
    uint64_t bandwidth_bps;  // Bandwidth in bits per second
    uint64_t burst_size;     // Burst size in bytes
    double weight;           // Weight for WFQ
    bool enabled;
    
    TrafficClass() : class_id(0), priority(Priority::NORMAL), 
                    bandwidth_bps(0), burst_size(0), weight(1.0), enabled(true) {}
};

// Packet Information
struct PacketInfo {
    std::vector<uint8_t> data;
    size_t size;
    Priority priority;
    int traffic_class;
    std::chrono::steady_clock::time_point timestamp;
    std::string source_interface;
    std::string destination_interface;
    
    PacketInfo() : size(0), priority(Priority::NORMAL), traffic_class(0) {
        timestamp = std::chrono::steady_clock::now();
    }
};

// Token Bucket Configuration
struct TokenBucketConfig {
    uint64_t rate_bps;       // Token generation rate in bits per second
    uint64_t burst_size;     // Maximum bucket size in bytes
    uint64_t current_tokens; // Current token count
    std::chrono::steady_clock::time_point last_update;
    
    TokenBucketConfig() : rate_bps(0), burst_size(0), current_tokens(0) {
        last_update = std::chrono::steady_clock::now();
    }
};

// WFQ Configuration
struct WFQConfig {
    std::map<int, TrafficClass> classes;
    uint64_t total_bandwidth;
    bool enable_flow_control;
    uint32_t max_flows;
    
    WFQConfig() : total_bandwidth(0), enable_flow_control(true), max_flows(1000) {}
};

// Traffic Shaping Statistics
struct ShapingStatistics {
    uint64_t packets_processed;
    uint64_t packets_dropped;
    uint64_t bytes_processed;
    uint64_t bytes_dropped;
    uint64_t tokens_generated;
    uint64_t tokens_consumed;
    std::chrono::steady_clock::time_point last_update;
    
    void reset() {
        packets_processed = 0;
        packets_dropped = 0;
        bytes_processed = 0;
        bytes_dropped = 0;
        tokens_generated = 0;
        tokens_consumed = 0;
        last_update = std::chrono::steady_clock::now();
    }
};

// Base Traffic Shaper Interface
class TrafficShaper {
public:
    virtual ~TrafficShaper() = default;
    
    // Core operations
    virtual bool initialize(const std::map<std::string, std::string>& config) = 0;
    virtual bool start() = 0;
    virtual bool stop() = 0;
    virtual bool is_running() const = 0;
    
    // Packet processing
    virtual bool enqueue_packet(const PacketInfo& packet) = 0;
    virtual bool dequeue_packet(PacketInfo& packet, int timeout_ms = 1000) = 0;
    virtual size_t queue_size() const = 0;
    virtual bool is_queue_full() const = 0;
    
    // Configuration
    virtual bool add_traffic_class(const TrafficClass& traffic_class) = 0;
    virtual bool remove_traffic_class(int class_id) = 0;
    virtual bool update_traffic_class(const TrafficClass& traffic_class) = 0;
    virtual std::vector<TrafficClass> get_traffic_classes() const = 0;
    
    // Statistics
    virtual ShapingStatistics get_statistics() const = 0;
    virtual ShapingStatistics get_class_statistics(int class_id) const = 0;
    
    // Callbacks
    virtual void set_packet_dropped_callback(std::function<void(const PacketInfo&)> callback) = 0;
    virtual void set_queue_full_callback(std::function<void()> callback) = 0;
};

// Token Bucket Implementation
class TokenBucketShaper : public TrafficShaper {
public:
    TokenBucketShaper();
    virtual ~TokenBucketShaper() = default;
    
    // TrafficShaper implementation
    bool initialize(const std::map<std::string, std::string>& config) override;
    bool start() override;
    bool stop() override;
    bool is_running() const override;
    
    bool enqueue_packet(const PacketInfo& packet) override;
    bool dequeue_packet(PacketInfo& packet, int timeout_ms = 1000) override;
    size_t queue_size() const override;
    bool is_queue_full() const override;
    
    bool add_traffic_class(const TrafficClass& traffic_class) override;
    bool remove_traffic_class(int class_id) override;
    bool update_traffic_class(const TrafficClass& traffic_class) override;
    std::vector<TrafficClass> get_traffic_classes() const override;
    
    ShapingStatistics get_statistics() const override;
    ShapingStatistics get_class_statistics(int class_id) const override;
    
    void set_packet_dropped_callback(std::function<void(const PacketInfo&)> callback) override;
    void set_queue_full_callback(std::function<void()> callback) override;
    
    // Token bucket specific methods
    bool add_tokens(uint64_t tokens);
    bool consume_tokens(uint64_t tokens);
    uint64_t get_available_tokens() const;
    bool refill_tokens();

private:
    void token_refill_loop();
    bool process_packet(const PacketInfo& packet);
    
    TokenBucketConfig config_;
    std::atomic<bool> running_;
    std::atomic<bool> refill_running_;
    
    // Packet queues
    std::queue<PacketInfo> packet_queue_;
    mutable std::mutex queue_mutex_;
    
    // Traffic classes
    std::map<int, TrafficClass> traffic_classes_;
    mutable std::mutex classes_mutex_;
    
    // Statistics
    mutable std::mutex stats_mutex_;
    ShapingStatistics statistics_;
    std::map<int, ShapingStatistics> class_statistics_;
    
    // Callbacks
    std::function<void(const PacketInfo&)> packet_dropped_callback_;
    std::function<void()> queue_full_callback_;
    
    // Threading
    std::thread refill_thread_;
};

// Weighted Fair Queuing Implementation
class WFQShaper : public TrafficShaper {
public:
    WFQShaper();
    virtual ~WFQShaper() = default;
    
    // TrafficShaper implementation
    bool initialize(const std::map<std::string, std::string>& config) override;
    bool start() override;
    bool stop() override;
    bool is_running() const override;
    
    bool enqueue_packet(const PacketInfo& packet) override;
    bool dequeue_packet(PacketInfo& packet, int timeout_ms = 1000) override;
    size_t queue_size() const override;
    bool is_queue_full() const override;
    
    bool add_traffic_class(const TrafficClass& traffic_class) override;
    bool remove_traffic_class(int class_id) override;
    bool update_traffic_class(const TrafficClass& traffic_class) override;
    std::vector<TrafficClass> get_traffic_classes() const override;
    
    ShapingStatistics get_statistics() const override;
    ShapingStatistics get_class_statistics(int class_id) const override;
    
    void set_packet_dropped_callback(std::function<void(const PacketInfo&)> callback) override;
    void set_queue_full_callback(std::function<void()> callback) override;
    
    // WFQ specific methods
    bool set_class_weight(int class_id, double weight);
    bool set_class_bandwidth(int class_id, uint64_t bandwidth_bps);
    double calculate_finish_time(const PacketInfo& packet, int class_id) const;

private:
    struct FlowInfo {
        int flow_id;
        int class_id;
        double finish_time;
        std::queue<PacketInfo> packets;
        
        FlowInfo() : flow_id(0), class_id(0), finish_time(0.0) {}
    };
    
    void scheduling_loop();
    bool process_packet(const PacketInfo& packet);
    int calculate_flow_id(const PacketInfo& packet) const;
    
    WFQConfig config_;
    std::atomic<bool> running_;
    std::atomic<bool> scheduling_running_;
    
    // Flow queues
    std::map<int, FlowInfo> flows_;
    std::priority_queue<std::pair<double, int>> finish_time_queue_;
    mutable std::mutex flows_mutex_;
    
    // Traffic classes
    std::map<int, TrafficClass> traffic_classes_;
    mutable std::mutex classes_mutex_;
    
    // Statistics
    mutable std::mutex stats_mutex_;
    ShapingStatistics statistics_;
    std::map<int, ShapingStatistics> class_statistics_;
    
    // Callbacks
    std::function<void(const PacketInfo&)> packet_dropped_callback_;
    std::function<void()> queue_full_callback_;
    
    // Threading
    std::thread scheduling_thread_;
};

// Traffic Shaping Manager
class TrafficShapingManager {
public:
    TrafficShapingManager();
    virtual ~TrafficShapingManager();
    
    // Core operations
    bool initialize(const std::map<std::string, std::string>& config);
    bool start();
    bool stop();
    bool is_running() const;
    
    // Shaper management
    bool add_shaper(const std::string& name, std::shared_ptr<TrafficShaper> shaper);
    bool remove_shaper(const std::string& name);
    std::shared_ptr<TrafficShaper> get_shaper(const std::string& name) const;
    std::vector<std::string> get_shaper_names() const;
    
    // Packet processing
    bool process_packet(const PacketInfo& packet, const std::string& shaper_name);
    bool get_processed_packet(PacketInfo& packet, const std::string& shaper_name, int timeout_ms = 1000);
    
    // Configuration
    bool configure_shaper(const std::string& name, const std::map<std::string, std::string>& config);
    bool add_traffic_class(const std::string& shaper_name, const TrafficClass& traffic_class);
    bool remove_traffic_class(const std::string& shaper_name, int class_id);
    
    // Statistics
    std::map<std::string, ShapingStatistics> get_all_statistics() const;
    ShapingStatistics get_shaper_statistics(const std::string& name) const;
    
    // Callbacks
    void set_packet_dropped_callback(std::function<void(const PacketInfo&, const std::string&)> callback);
    void set_queue_full_callback(std::function<void(const std::string&)> callback);

private:
    std::map<std::string, std::shared_ptr<TrafficShaper>> shapers_;
    mutable std::mutex shapers_mutex_;
    
    std::function<void(const PacketInfo&, const std::string&)> packet_dropped_callback_;
    std::function<void(const std::string&)> queue_full_callback_;
};

} // namespace router_sim