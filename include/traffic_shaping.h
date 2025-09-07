#pragma once

#include "common_structures.h"
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <atomic>
#include <thread>
#include <queue>

namespace RouterSim {

// Token bucket implementation
class TokenBucket {
public:
    TokenBucket(uint64_t rate_bps, uint64_t burst_size);
    ~TokenBucket();
    
    bool consume_tokens(uint32_t packet_size);
    void refill_tokens();
    uint64_t get_available_tokens() const;
    void reset();
    
private:
    uint64_t rate_bps_;
    uint64_t burst_size_;
    uint64_t tokens_;
    std::chrono::steady_clock::time_point last_refill_;
    mutable std::mutex mutex_;
};

// Weighted Fair Queueing implementation
class WFQ {
public:
    WFQ(uint32_t num_queues);
    ~WFQ();
    
    bool enqueue_packet(const Packet& packet, uint32_t queue_id);
    bool dequeue_packet(Packet& packet);
    void set_queue_weight(uint32_t queue_id, uint32_t weight);
    void set_queue_priority(uint32_t queue_id, uint8_t priority);
    uint32_t get_queue_size(uint32_t queue_id) const;
    void clear_queue(uint32_t queue_id);
    void clear_all_queues();
    
private:
    struct Queue {
        std::queue<Packet> packets;
        uint32_t weight;
        uint8_t priority;
        uint64_t virtual_time;
        uint64_t last_served_time;
    };
    
    uint32_t num_queues_;
    std::vector<Queue> queues_;
    mutable std::mutex mutex_;
    
    uint32_t select_next_queue();
    void update_virtual_time(uint32_t queue_id, uint32_t packet_size);
};

// Leaky bucket implementation
class LeakyBucket {
public:
    LeakyBucket(uint64_t rate_bps, uint64_t bucket_size);
    ~LeakyBucket();
    
    bool add_packet(const Packet& packet);
    bool get_packet(Packet& packet);
    uint32_t get_queue_size() const;
    void reset();
    
private:
    uint64_t rate_bps_;
    uint64_t bucket_size_;
    std::queue<Packet> packet_queue_;
    std::chrono::steady_clock::time_point last_leak_;
    mutable std::mutex mutex_;
    
    void leak_packets();
};

// Traffic shaper main class
class TrafficShaper {
public:
    TrafficShaper();
    ~TrafficShaper();
    
    // Core functionality
    bool start();
    bool stop();
    bool is_running() const;
    
    // Interface management
    bool add_interface(const std::string& name, const ShapingConfig& config);
    bool remove_interface(const std::string& name);
    bool update_interface_config(const std::string& name, const ShapingConfig& config);
    bool has_interface(const std::string& name) const;
    
    // Traffic shaping
    bool shape_packet(const std::string& interface, Packet& packet);
    bool process_packet(const std::string& interface, const Packet& packet);
    
    // WFQ management
    bool enable_wfq(const std::string& interface, uint32_t num_queues);
    bool disable_wfq(const std::string& interface);
    bool set_queue_weight(const std::string& interface, uint32_t queue_id, uint32_t weight);
    bool set_queue_priority(const std::string& interface, uint32_t queue_id, uint8_t priority);
    
    // Statistics
    std::map<std::string, uint64_t> get_interface_stats(const std::string& interface) const;
    std::map<std::string, uint64_t> get_global_stats() const;
    void reset_statistics();
    void reset_interface_statistics(const std::string& interface);
    
    // Configuration
    ShapingConfig get_interface_config(const std::string& interface) const;
    std::vector<std::string> get_interfaces() const;
    
private:
    struct InterfaceShaping {
        ShapingConfig config;
        std::unique_ptr<TokenBucket> token_bucket;
        std::unique_ptr<WFQ> wfq;
        std::unique_ptr<LeakyBucket> leaky_bucket;
        
        // Statistics
        uint64_t packets_processed;
        uint64_t packets_dropped;
        uint64_t bytes_processed;
        uint64_t bytes_dropped;
        uint64_t tokens_consumed;
        uint64_t tokens_available;
        
        InterfaceShaping() : packets_processed(0), packets_dropped(0), bytes_processed(0), 
                           bytes_dropped(0), tokens_consumed(0), tokens_available(0) {}
    };
    
    // Internal methods
    void processing_loop();
    bool process_interface_packet(const std::string& interface, Packet& packet);
    void update_statistics(const std::string& interface, const Packet& packet, bool dropped);
    
    // State
    std::atomic<bool> running_;
    std::thread processing_thread_;
    
    // Interface management
    std::map<std::string, InterfaceShaping> interfaces_;
    mutable std::mutex interfaces_mutex_;
    
    // Global statistics
    uint64_t total_packets_processed_;
    uint64_t total_packets_dropped_;
    uint64_t total_bytes_processed_;
    uint64_t total_bytes_dropped_;
    mutable std::mutex stats_mutex_;
};

} // namespace RouterSim