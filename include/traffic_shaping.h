#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <chrono>
#include <mutex>
#include <queue>
#include <atomic>

namespace RouterSim {

// Traffic shaping algorithms
enum class ShapingAlgorithm {
    TOKEN_BUCKET,
    LEAKY_BUCKET,
    WEIGHTED_FAIR_QUEUING,
    PRIORITY_QUEUING,
    CLASS_BASED_WEIGHTED_FAIR_QUEUING
};

// QoS classes
enum class QoSClass {
    BEST_EFFORT = 0,
    BACKGROUND = 1,
    BULK = 2,
    VIDEO = 3,
    VOICE = 4,
    NETWORK_CONTROL = 5,
    INTERNETWORK_CONTROL = 6,
    CRITICAL = 7
};

// Packet information
struct Packet {
    std::vector<uint8_t> data;
    std::string interface;
    QoSClass qos_class;
    uint32_t priority;
    uint64_t timestamp;
    uint32_t size;
    
    Packet() : qos_class(QoSClass::BEST_EFFORT), priority(0), timestamp(0), size(0) {}
    
    Packet(const std::vector<uint8_t>& packet_data, const std::string& iface, QoSClass qos = QoSClass::BEST_EFFORT)
        : data(packet_data), interface(iface), qos_class(qos), priority(static_cast<uint32_t>(qos)), 
          timestamp(std::chrono::duration_cast<std::chrono::microseconds>(
              std::chrono::steady_clock::now().time_since_epoch()).count()), size(packet_data.size()) {}
};

// Traffic shaping statistics
struct ShapingStats {
    uint64_t packets_processed;
    uint64_t packets_dropped;
    uint64_t packets_delayed;
    uint64_t bytes_processed;
    uint64_t bytes_dropped;
    uint64_t bytes_delayed;
    std::chrono::steady_clock::time_point start_time;
    
    ShapingStats() : packets_processed(0), packets_dropped(0), packets_delayed(0),
                     bytes_processed(0), bytes_dropped(0), bytes_delayed(0) {
        start_time = std::chrono::steady_clock::now();
    }
};

// Traffic shaping configuration
struct ShapingConfig {
    ShapingAlgorithm algorithm;
    uint64_t rate_bps;           // Rate in bits per second
    uint64_t burst_size;         // Burst size in bytes
    uint32_t queue_size;         // Maximum queue size
    std::map<QoSClass, uint32_t> class_weights;
    std::map<QoSClass, uint32_t> class_priorities;
    bool enable_red;             // Random Early Detection
    uint32_t red_min_threshold;
    uint32_t red_max_threshold;
    double red_probability;
    
    ShapingConfig() : algorithm(ShapingAlgorithm::TOKEN_BUCKET), rate_bps(1000000), 
                      burst_size(10000), queue_size(1000), enable_red(false),
                      red_min_threshold(100), red_max_threshold(200), red_probability(0.1) {
        // Default class weights
        class_weights[QoSClass::CRITICAL] = 8;
        class_weights[QoSClass::NETWORK_CONTROL] = 7;
        class_weights[QoSClass::INTERNETWORK_CONTROL] = 6;
        class_weights[QoSClass::VOICE] = 5;
        class_weights[QoSClass::VIDEO] = 4;
        class_weights[QoSClass::BULK] = 2;
        class_weights[QoSClass::BACKGROUND] = 1;
        class_weights[QoSClass::BEST_EFFORT] = 1;
        
        // Default class priorities
        class_priorities[QoSClass::CRITICAL] = 7;
        class_priorities[QoSClass::NETWORK_CONTROL] = 6;
        class_priorities[QoSClass::INTERNETWORK_CONTROL] = 5;
        class_priorities[QoSClass::VOICE] = 4;
        class_priorities[QoSClass::VIDEO] = 3;
        class_priorities[QoSClass::BULK] = 2;
        class_priorities[QoSClass::BACKGROUND] = 1;
        class_priorities[QoSClass::BEST_EFFORT] = 0;
    }
};

// Base traffic shaper interface
class ITrafficShaper {
public:
    virtual ~ITrafficShaper() = default;
    
    // Lifecycle
    virtual bool initialize(const ShapingConfig& config) = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual bool is_running() const = 0;
    
    // Configuration
    virtual void set_config(const ShapingConfig& config) = 0;
    virtual ShapingConfig get_config() const = 0;
    
    // Packet processing
    virtual bool process_packet(const Packet& packet) = 0;
    virtual bool process_packet(const std::vector<uint8_t>& data, const std::string& interface, QoSClass qos_class = QoSClass::BEST_EFFORT) = 0;
    
    // Queue management
    virtual size_t get_queue_size() const = 0;
    virtual size_t get_queue_size(QoSClass qos_class) const = 0;
    virtual void clear_queue() = 0;
    virtual void clear_queue(QoSClass qos_class) = 0;
    
    // Statistics
    virtual ShapingStats get_statistics() const = 0;
    virtual ShapingStats get_statistics(QoSClass qos_class) const = 0;
    virtual void reset_statistics() = 0;
    
    // Rate control
    virtual void set_rate(uint64_t rate_bps) = 0;
    virtual uint64_t get_rate() const = 0;
    virtual void set_burst_size(uint64_t burst_size) = 0;
    virtual uint64_t get_burst_size() const = 0;
};

// Token Bucket implementation
class TokenBucket : public ITrafficShaper {
public:
    TokenBucket();
    ~TokenBucket() override;

    bool initialize(const ShapingConfig& config) override;
    void start() override;
    void stop() override;
    bool is_running() const override;

    void set_config(const ShapingConfig& config) override;
    ShapingConfig get_config() const override;

    bool process_packet(const Packet& packet) override;
    bool process_packet(const std::vector<uint8_t>& data, const std::string& interface, QoSClass qos_class = QoSClass::BEST_EFFORT) override;

    size_t get_queue_size() const override;
    size_t get_queue_size(QoSClass qos_class) const override;
    void clear_queue() override;
    void clear_queue(QoSClass qos_class) override;

    ShapingStats get_statistics() const override;
    ShapingStats get_statistics(QoSClass qos_class) const override;
    void reset_statistics() override;

    void set_rate(uint64_t rate_bps) override;
    uint64_t get_rate() const override;
    void set_burst_size(uint64_t burst_size) override;
    uint64_t get_burst_size() const override;

private:
    void token_refill_loop();
    bool consume_tokens(uint32_t packet_size);
    void add_tokens();
    
    std::atomic<bool> running_;
    ShapingConfig config_;
    
    std::atomic<uint64_t> tokens_;
    std::atomic<uint64_t> last_refill_time_;
    
    std::queue<Packet> packet_queue_;
    mutable std::mutex queue_mutex_;
    
    std::thread refill_thread_;
    
    ShapingStats stats_;
    mutable std::mutex stats_mutex_;
};

// Weighted Fair Queuing implementation
class WeightedFairQueuing : public ITrafficShaper {
public:
    WeightedFairQueuing();
    ~WeightedFairQueuing() override;

    bool initialize(const ShapingConfig& config) override;
    void start() override;
    void stop() override;
    bool is_running() const override;

    void set_config(const ShapingConfig& config) override;
    ShapingConfig get_config() const override;

    bool process_packet(const Packet& packet) override;
    bool process_packet(const std::vector<uint8_t>& data, const std::string& interface, QoSClass qos_class = QoSClass::BEST_EFFORT) override;

    size_t get_queue_size() const override;
    size_t get_queue_size(QoSClass qos_class) const override;
    void clear_queue() override;
    void clear_queue(QoSClass qos_class) override;

    ShapingStats get_statistics() const override;
    ShapingStats get_statistics(QoSClass qos_class) const override;
    void reset_statistics() override;

    void set_rate(uint64_t rate_bps) override;
    uint64_t get_rate() const override;
    void set_burst_size(uint64_t burst_size) override;
    uint64_t get_burst_size() const override;

private:
    void scheduling_loop();
    Packet select_next_packet();
    double calculate_finish_time(const Packet& packet, QoSClass qos_class);
    
    std::atomic<bool> running_;
    ShapingConfig config_;
    
    std::map<QoSClass, std::queue<Packet>> class_queues_;
    std::map<QoSClass, double> virtual_finish_times_;
    std::map<QoSClass, ShapingStats> class_stats_;
    
    mutable std::mutex queues_mutex_;
    std::thread scheduling_thread_;
    
    ShapingStats total_stats_;
    mutable std::mutex stats_mutex_;
};

// Priority Queuing implementation
class PriorityQueuing : public ITrafficShaper {
public:
    PriorityQueuing();
    ~PriorityQueuing() override;

    bool initialize(const ShapingConfig& config) override;
    void start() override;
    void stop() override;
    bool is_running() const override;

    void set_config(const ShapingConfig& config) override;
    ShapingConfig get_config() const override;

    bool process_packet(const Packet& packet) override;
    bool process_packet(const std::vector<uint8_t>& data, const std::string& interface, QoSClass qos_class = QoSClass::BEST_EFFORT) override;

    size_t get_queue_size() const override;
    size_t get_queue_size(QoSClass qos_class) const override;
    void clear_queue() override;
    void clear_queue(QoSClass qos_class) override;

    ShapingStats get_statistics() const override;
    ShapingStats get_statistics(QoSClass qos_class) const override;
    void reset_statistics() override;

    void set_rate(uint64_t rate_bps) override;
    uint64_t get_rate() const override;
    void set_burst_size(uint64_t burst_size) override;
    uint64_t get_burst_size() const override;

private:
    void processing_loop();
    Packet get_next_packet();
    
    std::atomic<bool> running_;
    ShapingConfig config_;
    
    std::map<QoSClass, std::queue<Packet>> priority_queues_;
    std::map<QoSClass, ShapingStats> class_stats_;
    
    mutable std::mutex queues_mutex_;
    std::thread processing_thread_;
    
    ShapingStats total_stats_;
    mutable std::mutex stats_mutex_;
};

// Traffic shaper factory
class TrafficShaperFactory {
public:
    static std::unique_ptr<ITrafficShaper> create_shaper(ShapingAlgorithm algorithm);
    static std::vector<ShapingAlgorithm> get_supported_algorithms();
    static std::string get_algorithm_name(ShapingAlgorithm algorithm);
};

// Traffic shaping manager
class TrafficShaper {
public:
    TrafficShaper();
    ~TrafficShaper();

    // Lifecycle
    bool initialize(const ShapingConfig& config);
    void start();
    void stop();
    bool is_running() const;

    // Configuration
    void set_config(const ShapingConfig& config);
    ShapingConfig get_config() const;

    // Packet processing
    bool process_packet(const Packet& packet);
    bool process_packet(const std::vector<uint8_t>& data, const std::string& interface, QoSClass qos_class = QoSClass::BEST_EFFORT);

    // Queue management
    size_t get_queue_size() const;
    size_t get_queue_size(QoSClass qos_class) const;
    void clear_queue();
    void clear_queue(QoSClass qos_class);

    // Statistics
    ShapingStats get_statistics() const;
    ShapingStats get_statistics(QoSClass qos_class) const;
    void reset_statistics();

    // Rate control
    void set_rate(uint64_t rate_bps);
    uint64_t get_rate() const;
    void set_burst_size(uint64_t burst_size);
    uint64_t get_burst_size() const;

    // Algorithm management
    void set_algorithm(ShapingAlgorithm algorithm);
    ShapingAlgorithm get_algorithm() const;

private:
    std::unique_ptr<ITrafficShaper> shaper_;
    ShapingConfig config_;
    std::atomic<bool> running_;
    mutable std::mutex shaper_mutex_;
};

} // namespace RouterSim