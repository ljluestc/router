#pragma once

#include <memory>
#include <vector>
#include <mutex>
#include <chrono>
#include <atomic>
#include <limits>

namespace RouterSim {

// Packet representation
struct Packet {
    uint64_t id;
    uint32_t size;
    uint32_t priority;
    std::string source_ip;
    std::string dest_ip;
    uint16_t source_port;
    uint16_t dest_port;
    uint8_t protocol;
    std::chrono::steady_clock::time_point timestamp;
    
    Packet() : id(0), size(0), priority(0), source_port(0), dest_port(0), protocol(0) {
        timestamp = std::chrono::steady_clock::now();
    }
};

// Token Bucket Implementation
class TokenBucket {
public:
    TokenBucket(uint64_t capacity, uint64_t refill_rate, uint64_t burst_size);
    ~TokenBucket();
    
    // Core functionality
    bool consume(uint64_t tokens);
    bool consumePacket(const Packet& packet);
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
    bool enqueue(uint32_t queue_id, const Packet& packet);
    bool dequeue(Packet& packet);
    
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
    bool processPacket(const Packet& packet);
    bool dequeuePacket(Packet& packet);
    
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

} // namespace RouterSim