#pragma once

#include <string>
#include <map>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <chrono>
#include <functional>
#include <memory>

namespace RouterSim {

// Forward declarations
struct PacketInfo;
struct TokenBucketConfig;
struct WFQClass;

// Traffic shaping algorithms
enum class ShapingAlgorithm {
    TOKEN_BUCKET,
    WEIGHTED_FAIR_QUEUE,
    PRIORITY_QUEUE,
    RATE_LIMITING
};

// Packet information
struct PacketInfo {
    uint64_t id;
    std::string src_ip;
    std::string dst_ip;
    uint16_t src_port;
    uint16_t dst_port;
    uint8_t protocol;
    uint32_t size;
    uint8_t dscp;
    uint8_t ttl;
    uint32_t priority;
    std::vector<uint8_t> data;
    std::chrono::steady_clock::time_point timestamp;
    
    PacketInfo() : id(0), src_port(0), dst_port(0), protocol(0), size(0), dscp(0), ttl(64), priority(0) {
        timestamp = std::chrono::steady_clock::now();
    }
};

// Traffic statistics
struct TrafficStats {
    uint64_t packets_processed;
    uint64_t packets_dropped;
    uint64_t bytes_processed;
    uint64_t bytes_dropped;
    uint64_t queue_length;
    double current_throughput_bps;
    double peak_throughput_bps;
    std::chrono::steady_clock::time_point last_update;
    
    void reset() {
        packets_processed = 0;
        packets_dropped = 0;
        bytes_processed = 0;
        bytes_dropped = 0;
        queue_length = 0;
        current_throughput_bps = 0.0;
        peak_throughput_bps = 0.0;
        last_update = std::chrono::steady_clock::now();
    }
};

// Token bucket implementation
class TokenBucket {
public:
    explicit TokenBucket(const TokenBucketConfig& config);
    ~TokenBucket();
    
    bool consume_tokens(uint64_t bytes);
    uint64_t get_available_tokens() const;
    bool is_available(uint64_t bytes) const;
    void update_config(const TokenBucketConfig& config);
    TokenBucketConfig get_config() const;
    
private:
    void add_tokens();
    
    TokenBucketConfig config_;
    uint64_t tokens_;
    std::chrono::steady_clock::time_point last_update_;
    mutable std::mutex mutex_;
};

// Weighted Fair Queue implementation
class WeightedFairQueue {
public:
    WeightedFairQueue();
    ~WeightedFairQueue();
    
    bool initialize(const std::vector<WFQClass>& classes);
    bool enqueue_packet(const PacketInfo& packet, uint8_t class_id);
    bool dequeue_packet(PacketInfo& packet);
    bool is_empty() const;
    size_t queue_size() const;
    size_t queue_size(uint8_t class_id) const;
    bool add_class(const WFQClass& wfq_class);
    bool remove_class(uint8_t class_id);
    bool update_class(const WFQClass& wfq_class);
    std::vector<WFQClass> get_classes() const;
    void set_classifier(std::function<uint8_t(const PacketInfo&)> classifier);
    uint8_t classify_packet(const PacketInfo& packet) const;
    
private:
    uint64_t calculate_virtual_finish_time(const PacketInfo& packet, uint8_t class_id) const;
    bool select_next_packet(struct QueueItem& item);
    
    std::vector<WFQClass> classes_;
    std::map<uint8_t, std::queue<struct QueueItem>> queues_;
    uint64_t virtual_time_;
    std::function<uint8_t(const PacketInfo&)> classifier_;
    mutable std::mutex mutex_;
};

// Traffic shaper implementation
class TrafficShaper {
public:
    TrafficShaper();
    ~TrafficShaper();
    
    // Lifecycle
    bool initialize();
    bool start();
    bool stop();
    bool is_running() const;
    
    // Configuration
    bool configure_token_bucket(const TokenBucketConfig& config);
    bool configure_wfq(const std::vector<WFQClass>& classes);
    bool set_shaping_algorithm(ShapingAlgorithm algorithm);
    
    // Packet processing
    bool process_packet(const PacketInfo& packet);
    bool process_packet_async(const PacketInfo& packet);
    bool enqueue_packet(const PacketInfo& packet);
    bool dequeue_packet(PacketInfo& packet);
    
    // Queue management
    size_t get_queue_size() const;
    bool is_queue_empty() const;
    
    // Statistics
    TrafficStats get_statistics() const;
    void reset_statistics();
    
    // Callbacks
    void set_packet_callback(std::function<void(const PacketInfo&)> callback);
    void set_drop_callback(std::function<void(const PacketInfo&, const std::string&)> callback);
    
private:
    void processing_loop();
    bool process_packet_internal(const PacketInfo& packet);
    void update_statistics(const PacketInfo& packet, bool dropped);
    void notify_packet_processed(const PacketInfo& packet);
    void notify_packet_dropped(const PacketInfo& packet, const std::string& reason);
    
    // Components
    std::unique_ptr<TokenBucket> token_bucket_;
    std::unique_ptr<WeightedFairQueue> wfq_;
    
    // Configuration
    ShapingAlgorithm algorithm_;
    
    // State
    bool running_;
    bool initialized_;
    bool stop_processing_;
    
    // Threading
    std::thread processing_thread_;
    
    // Queue
    std::queue<PacketInfo> packet_queue_;
    mutable std::mutex queue_mutex_;
    
    // Statistics
    mutable std::mutex stats_mutex_;
    TrafficStats stats_;
    
    // Callbacks
    std::function<void(const PacketInfo&)> packet_callback_;
    std::function<void(const PacketInfo&, const std::string&)> drop_callback_;
};

// Traffic shaping manager
class TrafficShapingManager {
public:
    TrafficShapingManager();
    ~TrafficShapingManager();
    
    // Lifecycle
    bool initialize();
    bool start();
    bool stop();
    
    // Interface management
    bool add_interface(const std::string& interface_name);
    bool remove_interface(const std::string& interface_name);
    bool configure_interface(const std::string& interface_name, 
                            ShapingAlgorithm algorithm,
                            const std::map<std::string, std::string>& config);
    
    // Packet processing
    bool process_packet(const std::string& interface_name, const PacketInfo& packet);
    
    // Statistics
    std::map<std::string, TrafficStats> get_interface_statistics() const;
    TrafficStats get_global_statistics() const;
    
    // Configuration
    bool load_config(const std::string& config_file);
    bool save_config(const std::string& config_file) const;
    
private:
    bool initialized_;
    bool running_;
    
    // Interface shapers
    std::map<std::string, std::unique_ptr<TrafficShaper>> interfaces_;
    mutable std::mutex interfaces_mutex_;
    
    // Global statistics
    TrafficStats global_stats_;
    mutable std::mutex global_stats_mutex_;
};

} // namespace RouterSim