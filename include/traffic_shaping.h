#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include <atomic>
#include <chrono>
#include <queue>
#include <thread>

namespace RouterSim {

// Forward declaration
struct Packet;

// Traffic shaping configuration
struct ShapingConfig {
    uint32_t rate_bps = 0;  // 0 means no shaping
    uint32_t burst_size = 0;
    uint32_t queue_size = 1000;
    bool enable_wfq = false;
    uint32_t num_queues = 8;
    uint32_t weight_base = 1000; // Base weight for WFQ
};

// Token Bucket implementation
class TokenBucket {
public:
    TokenBucket(uint32_t rate_bps, uint32_t burst_size);
    ~TokenBucket() = default;
    
    bool consume_tokens(uint32_t bytes);
    void refill_tokens();
    uint32_t get_available_tokens() const;
    void set_rate(uint32_t rate_bps);
    void set_burst_size(uint32_t burst_size);
    void reset();

private:
    uint32_t rate_bps_;
    uint32_t burst_size_;
    uint32_t tokens_;
    std::chrono::steady_clock::time_point last_refill_;
    mutable std::mutex mutex_;
};

// Weighted Fair Queueing implementation
class WFQ {
public:
    WFQ(uint32_t num_queues, uint32_t weight_base = 1000);
    ~WFQ() = default;
    
    bool enqueue_packet(const Packet& packet, uint32_t queue_id);
    bool dequeue_packet(Packet& packet);
    void set_queue_weight(uint32_t queue_id, uint32_t weight);
    void set_queue_priority(uint32_t queue_id, uint32_t priority);
    uint32_t get_queue_size(uint32_t queue_id) const;
    uint32_t get_total_queue_size() const;
    void clear_queues();
    bool is_empty() const;

private:
    struct QueueInfo {
        std::queue<Packet> packets;
        uint32_t weight;
        uint32_t priority;
        uint32_t virtual_time;
    };
    
    uint32_t num_queues_;
    uint32_t weight_base_;
    std::vector<QueueInfo> queues_;
    mutable std::mutex mutex_;
    
    uint32_t calculate_virtual_time(uint32_t queue_id, uint32_t packet_size);
    uint32_t select_next_queue();
};

// Traffic Shaper main class
class TrafficShaper {
public:
    TrafficShaper();
    ~TrafficShaper();
    
    bool initialize();
    bool configure_interface(const std::string& interface, const ShapingConfig& config);
    bool clear_interface(const std::string& interface);
    bool enable_wfq(const std::string& interface, uint32_t num_queues = 8);
    bool disable_wfq(const std::string& interface);
    
    bool process_packet(Packet& packet);
    bool enqueue_packet(const Packet& packet);
    bool dequeue_packet(Packet& packet);
    
    // Statistics
    std::map<std::string, uint64_t> get_interface_stats(const std::string& interface) const;
    std::map<std::string, uint64_t> get_global_stats() const;
    void reset_statistics();
    
    // Control
    void start();
    void stop();
    bool is_running() const;

private:
    struct InterfaceShaping {
        std::unique_ptr<TokenBucket> token_bucket;
        std::unique_ptr<WFQ> wfq;
        ShapingConfig config;
        std::queue<Packet> packet_queue;
        uint64_t packets_processed;
        uint64_t packets_dropped;
        uint64_t bytes_processed;
        uint64_t bytes_dropped;
    };
    
    std::map<std::string, InterfaceShaping> interfaces_;
    mutable std::mutex interfaces_mutex_;
    std::atomic<bool> running_;
    std::thread processing_thread_;
    
    // Global statistics
    uint64_t total_packets_processed_;
    uint64_t total_packets_dropped_;
    uint64_t total_bytes_processed_;
    uint64_t total_bytes_dropped_;
    
    void processing_loop();
    bool process_interface_packets(const std::string& interface, InterfaceShaping& shaping);
    uint32_t calculate_queue_id(const Packet& packet) const;
    bool apply_token_bucket(InterfaceShaping& shaping, Packet& packet);
    bool apply_wfq(InterfaceShaping& shaping, Packet& packet);
};

} // namespace RouterSim
