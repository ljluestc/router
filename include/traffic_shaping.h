#pragma once

#include <cstdint>
#include <chrono>
#include <mutex>
#include <memory>
#include <vector>
#include <limits>

namespace RouterSim {

struct Packet {
    uint64_t size;
    uint32_t priority;
    uint64_t timestamp;
    std::string source;
    std::string destination;
    uint16_t protocol;
};

class TokenBucket {
public:
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

    TokenBucket(uint64_t capacity, uint64_t refill_rate, uint64_t burst_size);
    ~TokenBucket();

    bool consume(uint64_t tokens);
    bool consumePacket(const Packet& packet);
    void refillTokens();
    
    void setCapacity(uint64_t capacity);
    void setRefillRate(uint64_t refill_rate);
    void setBurstSize(uint64_t burst_size);
    
    uint64_t getAvailableTokens() const;
    Statistics getStatistics() const;
    void reset();

private:
    uint64_t capacity_;
    uint64_t refill_rate_;
    uint64_t burst_size_;
    uint64_t tokens_;
    std::chrono::steady_clock::time_point last_refill_time_;
    
    uint64_t total_packets_processed_;
    uint64_t total_bytes_processed_;
    uint64_t packets_dropped_;
    uint64_t bytes_dropped_;
    
    mutable std::mutex mutex_;
};

class WFQ {
public:
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

    static constexpr uint32_t MAX_QUEUE_SIZE = 1000;

    WFQ(uint32_t max_queues);
    ~WFQ();

    bool enqueue(uint32_t queue_id, const Packet& packet);
    bool dequeue(Packet& packet);
    
    void setQueueWeight(uint32_t queue_id, uint32_t weight);
    uint32_t getQueueWeight(uint32_t queue_id) const;
    uint32_t getQueueSize(uint32_t queue_id) const;
    uint64_t getQueueBytes(uint32_t queue_id) const;
    uint32_t getMaxQueues() const { return max_queues_; }
    
    Statistics getStatistics() const;
    void reset();

private:
    struct Queue {
        uint32_t weight;
        uint32_t packets;
        uint64_t bytes;
        double finish_time;
    };

    void updateVirtualTime();

    uint32_t max_queues_;
    std::vector<Queue> queues_;
    uint32_t total_weight_;
    double virtual_time_;
    std::chrono::steady_clock::time_point last_update_time_;
    
    uint64_t total_packets_processed_;
    uint64_t total_bytes_processed_;
    uint64_t packets_dropped_;
    uint64_t bytes_dropped_;
    
    mutable std::mutex mutex_;
};

class TrafficShaper {
public:
    struct Statistics {
        bool enabled;
        uint64_t total_packets_processed;
        uint64_t total_bytes_processed;
        uint64_t packets_dropped;
        uint64_t bytes_dropped;
        TokenBucket::Statistics token_bucket_stats;
        WFQ::Statistics wfq_stats;
    };

    TrafficShaper();
    ~TrafficShaper();

    bool initialize();
    bool processPacket(const Packet& packet);
    bool dequeuePacket(Packet& packet);
    
    void setTokenBucketConfig(uint64_t capacity, uint64_t refill_rate, uint64_t burst_size);
    void setQueueWeight(uint32_t queue_id, uint32_t weight);
    void setEnabled(bool enabled);
    bool isEnabled() const;
    
    Statistics getStatistics() const;
    void reset();

private:
    bool enabled_;
    std::unique_ptr<TokenBucket> token_bucket_;
    std::unique_ptr<WFQ> wfq_;
    
    uint64_t total_packets_processed_;
    uint64_t total_bytes_processed_;
    uint64_t packets_dropped_;
    uint64_t bytes_dropped_;
    
    mutable std::mutex mutex_;
};

} // namespace RouterSim