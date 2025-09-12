#pragma once

#include "common_types.h"
#include <memory>
#include <vector>
#include <mutex>
#include <chrono>
#include <atomic>
#include <limits>
#include <functional>
#include <map>

namespace RouterSim {

// Token Bucket Implementation
class TokenBucket {
public:
    TokenBucket(uint64_t capacity, uint64_t refill_rate, uint64_t burst_size);
    ~TokenBucket();
    
    // Core functionality
    bool consume(uint64_t tokens);
    bool consumePacket(const PacketInfo& packet);
    void refillTokens();
    
    // Configuration
    void setCapacity(uint64_t capacity);
    void setRefillRate(uint64_t refill_rate);
    void setBurstSize(uint64_t burst_size);
    
    // Information
    uint64_t getAvailableTokens() const;
    void reset();
    
    // Statistics
    struct Statistics {
        uint64_t capacity;
        uint64_t refill_rate;
        uint64_t burst_size;
        uint64_t available_tokens;
        uint64_t total_packets_processed;
        uint64_t total_bytes_processed;
        uint64_t packets_dropped;
        uint64_t bytes_dropped;
        double utilization_percentage;
    };
    
    Statistics getStatistics() const;

private:
    uint64_t capacity_;
    uint64_t refill_rate_;
    uint64_t burst_size_;
    uint64_t tokens_;
    std::chrono::steady_clock::time_point last_refill_time_;
    
    // Statistics
    uint64_t total_packets_processed_;
    uint64_t total_bytes_processed_;
    uint64_t packets_dropped_;
    uint64_t bytes_dropped_;
    
    mutable std::mutex mutex_;
};

// Weighted Fair Queueing Implementation
class WFQ {
public:
    static const uint32_t MAX_QUEUE_SIZE = 1000;
    
    WFQ(uint32_t max_queues);
    ~WFQ();
    
    // Core functionality
    bool enqueue(uint32_t queue_id, const PacketInfo& packet);
    bool dequeue(PacketInfo& packet);
    
    // Configuration
    void setQueueWeight(uint32_t queue_id, uint32_t weight);
    
    // Information
    uint32_t getMaxQueues() const { return max_queues_; }
    uint32_t getQueueWeight(uint32_t queue_id) const;
    uint32_t getQueueSize(uint32_t queue_id) const;
    uint64_t getQueueBytes(uint32_t queue_id) const;
    
    // Statistics
    struct QueueStatistics {
        uint32_t queue_id;
        uint32_t weight;
        uint32_t packets;
        uint64_t bytes;
        double finish_time;
    };
    
    struct Statistics {
        uint32_t max_queues;
        uint32_t total_weight;
        double virtual_time;
        uint64_t total_packets_processed;
        uint64_t total_bytes_processed;
        uint64_t packets_dropped;
        uint64_t bytes_dropped;
        std::vector<QueueStatistics> queue_stats;
    };
    
    Statistics getStatistics() const;
    void reset();

private:
    struct Queue {
        uint32_t weight;
        uint32_t packets;
        uint64_t bytes;
        double finish_time;
    };
    
    uint32_t max_queues_;
    std::vector<Queue> queues_;
    uint32_t total_weight_;
    double virtual_time_;
    std::chrono::steady_clock::time_point last_update_time_;
    
    // Statistics
    uint64_t total_packets_processed_;
    uint64_t total_bytes_processed_;
    uint64_t packets_dropped_;
    uint64_t bytes_dropped_;
    
    mutable std::mutex mutex_;
    
    void updateVirtualTime();
};

// Traffic Shaper - Combines Token Bucket and WFQ
class TrafficShaper {
public:
    TrafficShaper();
    ~TrafficShaper();
    
    // Core functionality
    bool initialize();
    bool processPacket(const PacketInfo& packet);
    bool dequeuePacket(PacketInfo& packet);
    
    // Configuration
    void setTokenBucketConfig(uint64_t capacity, uint64_t refill_rate, uint64_t burst_size);
    void setQueueWeight(uint32_t queue_id, uint32_t weight);
    void setEnabled(bool enabled);
    bool isEnabled() const;
    
    // Statistics
    struct Statistics {
        bool enabled;
        uint64_t total_packets_processed;
        uint64_t total_bytes_processed;
        uint64_t packets_dropped;
        uint64_t bytes_dropped;
        TokenBucket::Statistics token_bucket_stats;
        WFQ::Statistics wfq_stats;
    };
    
    Statistics getStatistics() const;
    void reset();

private:
    std::unique_ptr<TokenBucket> token_bucket_;
    std::unique_ptr<WFQ> wfq_;
    bool enabled_;
    
    // Statistics
    uint64_t total_packets_processed_;
    uint64_t total_bytes_processed_;
    uint64_t packets_dropped_;
    uint64_t bytes_dropped_;
    
    mutable std::mutex mutex_;
};

// Traffic Shaping Manager - Manages multiple traffic shapers
class TrafficShapingManager {
public:
    TrafficShapingManager();
    ~TrafficShapingManager();
    
    // Core functionality
    bool initialize();
    bool start();
    bool stop();
    bool is_running() const;
    
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
    
    // Callbacks
    void set_packet_callback(PacketCallback callback);
    void set_drop_callback(DropCallback callback);

private:
    std::atomic<bool> running_;
    std::atomic<bool> initialized_;
    std::map<std::string, std::unique_ptr<TrafficShaper>> interfaces_;
    mutable std::mutex interfaces_mutex_;
    
    // Callbacks
    PacketCallback packet_callback_;
    DropCallback drop_callback_;
    
    // Statistics
    TrafficStats global_stats_;
    mutable std::mutex stats_mutex_;
    
    // Internal methods
    void processing_loop();
    bool process_packet_internal(const PacketInfo& packet);
    void update_statistics(const PacketInfo& packet, bool dropped);
    void notify_packet_processed(const PacketInfo& packet);
    void notify_packet_dropped(const PacketInfo& packet, const std::string& reason);
};

} // namespace RouterSim