#pragma once

#include "router_core.h"
#include <chrono>
#include <queue>
#include <vector>
#include <memory>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

namespace RouterSim {

// Token Bucket Algorithm implementation
class TokenBucket {
public:
    TokenBucket(double rate, double burst_size);
    ~TokenBucket() = default;

    // Token bucket operations
    bool consume_tokens(int tokens);
    void add_tokens();
    double get_current_tokens() const;
    double get_rate() const;
    double get_burst_size() const;
    void set_rate(double rate);
    void set_burst_size(double burst_size);
    void reset();

private:
    double rate_;           // tokens per second
    double burst_size_;     // maximum tokens
    double tokens_;         // current tokens
    std::chrono::steady_clock::time_point last_update_;
    mutable std::mutex mutex_;
};

// Weighted Fair Queuing implementation
class WFQScheduler {
public:
    struct QueueConfig {
        int queue_id;
        int weight;
        int priority;
        std::string name;
    };

    WFQScheduler();
    ~WFQScheduler();

    // Queue management
    bool add_queue(const QueueConfig& config);
    bool remove_queue(int queue_id);
    bool update_queue_weight(int queue_id, int weight);
    std::vector<QueueConfig> get_queues() const;

    // Packet scheduling
    bool enqueue_packet(int queue_id, const std::vector<uint8_t>& packet);
    std::vector<uint8_t> dequeue_packet();
    bool is_empty() const;
    size_t get_queue_size(int queue_id) const;
    size_t get_total_packets() const;

    // Configuration
    void set_scheduling_algorithm(const std::string& algorithm);
    void set_max_queue_size(size_t max_size);
    void set_bandwidth_limit(double bandwidth_mbps);

private:
    struct WFQQueue {
        int queue_id;
        int weight;
        int priority;
        std::string name;
        std::queue<std::vector<uint8_t>> packets;
        double virtual_time;
        double finish_time;
        mutable std::mutex mutex;
    };

    // Scheduling algorithms
    std::vector<uint8_t> schedule_wfq();
    std::vector<uint8_t> schedule_wrr();
    std::vector<uint8_t> schedule_priority();

    // State
    std::map<int, std::unique_ptr<WFQQueue>> queues_;
    std::string scheduling_algorithm_;
    size_t max_queue_size_;
    double bandwidth_limit_;
    mutable std::mutex scheduler_mutex_;
    std::condition_variable cv_;
};

// Hierarchical Token Bucket (HTB) implementation
class HTBShaper {
public:
    struct ClassConfig {
        int class_id;
        int parent_id;
        double rate;
        double ceil;
        int priority;
        std::string name;
    };

    HTBShaper();
    ~HTBShaper();

    // Class management
    bool add_class(const ClassConfig& config);
    bool remove_class(int class_id);
    bool update_class_rate(int class_id, double rate, double ceil);
    std::vector<ClassConfig> get_classes() const;

    // Packet classification and shaping
    bool classify_and_shape(const std::vector<uint8_t>& packet, int class_id);
    std::vector<uint8_t> get_shaped_packet();
    bool is_empty() const;

    // Configuration
    void set_root_rate(double rate);
    void set_quantum(int quantum);

private:
    struct HTBClass {
        int class_id;
        int parent_id;
        double rate;
        double ceil;
        int priority;
        std::string name;
        double tokens;
        double ctokens;
        std::chrono::steady_clock::time_point last_update;
        std::queue<std::vector<uint8_t>> packets;
        mutable std::mutex mutex;
    };

    // HTB operations
    void update_tokens(HTBClass& htb_class);
    bool can_send(HTBClass& htb_class, int packet_size);
    void send_packet(HTBClass& htb_class, int packet_size);

    // State
    std::map<int, std::unique_ptr<HTBClass>> classes_;
    double root_rate_;
    int quantum_;
    mutable std::mutex htb_mutex_;
};

// Traffic Shaper main class
class TrafficShaper {
public:
    TrafficShaper();
    ~TrafficShaper();

    // Interface-based shaping
    bool configure_interface_shaping(const std::string& interface,
                                    const std::string& algorithm,
                                    const std::map<std::string, double>& parameters);
    bool remove_interface_shaping(const std::string& interface);

    // Token bucket configuration
    bool configure_token_bucket(const std::string& interface,
                               double rate, double burst_size);

    // WFQ configuration
    bool configure_wfq(const std::string& interface,
                      const std::vector<WFQScheduler::QueueConfig>& queues);

    // HTB configuration
    bool configure_htb(const std::string& interface,
                      const std::vector<HTBShaper::ClassConfig>& classes);

    // Packet processing
    bool process_packet(const std::string& interface,
                       const std::vector<uint8_t>& packet);
    std::vector<std::vector<uint8_t>> get_shaped_packets(const std::string& interface);

    // Statistics
    struct ShapingStats {
        uint64_t packets_processed;
        uint64_t packets_dropped;
        uint64_t bytes_processed;
        uint64_t bytes_dropped;
        double current_rate;
        double peak_rate;
        std::map<int, uint64_t> queue_stats;
    };

    ShapingStats get_interface_stats(const std::string& interface) const;
    void reset_interface_stats(const std::string& interface);

    // Configuration
    void set_global_bandwidth_limit(double bandwidth_mbps);
    void set_packet_size_limit(size_t max_packet_size);

private:
    // Per-interface shapers
    struct InterfaceShaper {
        std::unique_ptr<TokenBucket> token_bucket;
        std::unique_ptr<WFQScheduler> wfq_scheduler;
        std::unique_ptr<HTBShaper> htb_shaper;
        std::string algorithm;
        ShapingStats stats;
        mutable std::mutex mutex;
    };

    // Shaping algorithms
    bool apply_token_bucket_shaping(InterfaceShaper& shaper, 
                                   const std::vector<uint8_t>& packet);
    bool apply_wfq_shaping(InterfaceShaper& shaper, 
                          const std::vector<uint8_t>& packet);
    bool apply_htb_shaping(InterfaceShaper& shaper, 
                          const std::vector<uint8_t>& packet);

    // State
    std::map<std::string, std::unique_ptr<InterfaceShaper>> interface_shapers_;
    double global_bandwidth_limit_;
    size_t packet_size_limit_;
    mutable std::mutex shaper_mutex_;

    // Background processing
    std::thread shaping_thread_;
    std::atomic<bool> shaping_running_;
    std::condition_variable cv_;

    void shaping_loop();
};

} // namespace RouterSim