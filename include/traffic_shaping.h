#pragma once

#include <cstdint>
#include <chrono>
#include <mutex>
#include <memory>
#include <vector>
#include <queue>
#include <map>
#include <string>

namespace RouterSim {

struct Packet {
    uint64_t size;
    uint32_t priority;
    uint32_t flow_id;
    std::chrono::steady_clock::time_point timestamp;
    std::map<std::string, std::string> metadata;
};

// Token Bucket Algorithm
class TokenBucket {
public:
    TokenBucket(uint64_t capacity, uint64_t refill_rate, uint64_t burst_size);
    ~TokenBucket();

    bool consume(uint64_t tokens);
    bool consumePacket(const Packet& packet);
    void refillTokens();
    
    void setCapacity(uint64_t capacity);
    void setRefillRate(uint64_t refill_rate);
    void setBurstSize(uint64_t burst_size);
    
    uint64_t getAvailableTokens() const;
    
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

// Weighted Fair Queueing Algorithm
class WFQ {
public:
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
    
    uint64_t total_packets_processed_;
    uint64_t total_bytes_processed_;
    uint64_t packets_dropped_;
    uint64_t bytes_dropped_;
    
    mutable std::mutex mutex_;
    
    void updateVirtualTime();
};

// Deficit Round Robin Algorithm
class DRR {
public:
    static constexpr uint32_t MAX_QUEUE_SIZE = 1000;
    
    DRR(uint32_t max_queues);
    ~DRR();

    bool enqueue(uint32_t queue_id, const Packet& packet);
    bool dequeue(Packet& packet);
    
    void setQuantum(uint32_t queue_id, uint32_t quantum);
    uint32_t getQuantum(uint32_t queue_id) const;
    uint32_t getQueueSize(uint32_t queue_id) const;
    uint64_t getQueueBytes(uint32_t queue_id) const;
    uint32_t getMaxQueues() const { return max_queues_; }
    
    struct QueueStatistics {
        uint32_t queue_id;
        uint32_t quantum;
        uint32_t deficit;
        uint32_t packets;
        uint64_t bytes;
    };
    
    struct Statistics {
        uint32_t max_queues;
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
        uint32_t quantum;
        uint32_t deficit;
        uint32_t packets;
        uint64_t bytes;
        std::queue<Packet> packet_queue;
    };
    
    uint32_t max_queues_;
    std::vector<Queue> queues_;
    uint32_t current_queue_;
    
    uint64_t total_packets_processed_;
    uint64_t total_bytes_processed_;
    uint64_t packets_dropped_;
    uint64_t bytes_dropped_;
    
    mutable std::mutex mutex_;
};

// Hierarchical Token Bucket
class HTB {
public:
    HTB(uint32_t max_classes);
    ~HTB();

    bool enqueue(uint32_t class_id, const Packet& packet);
    bool dequeue(Packet& packet);
    
    void setClassRate(uint32_t class_id, uint64_t rate);
    void setClassCeil(uint32_t class_id, uint64_t ceil);
    void setClassBurst(uint32_t class_id, uint64_t burst);
    void setClassPriority(uint32_t class_id, uint32_t priority);
    
    uint32_t getMaxClasses() const { return max_classes_; }
    
    struct ClassStatistics {
        uint32_t class_id;
        uint64_t rate;
        uint64_t ceil;
        uint64_t burst;
        uint32_t priority;
        uint64_t tokens;
        uint32_t packets;
        uint64_t bytes;
    };
    
    struct Statistics {
        uint32_t max_classes;
        uint64_t total_packets_processed;
        uint64_t total_bytes_processed;
        uint64_t packets_dropped;
        uint64_t bytes_dropped;
        std::vector<ClassStatistics> class_stats;
    };
    
    Statistics getStatistics() const;
    void reset();

private:
    struct Class {
        uint64_t rate;
        uint64_t ceil;
        uint64_t burst;
        uint32_t priority;
        uint64_t tokens;
        uint32_t packets;
        uint64_t bytes;
        std::queue<Packet> packet_queue;
        std::chrono::steady_clock::time_point last_refill_time;
    };
    
    uint32_t max_classes_;
    std::vector<Class> classes_;
    
    uint64_t total_packets_processed_;
    uint64_t total_bytes_processed_;
    uint64_t packets_dropped_;
    uint64_t bytes_dropped_;
    
    mutable std::mutex mutex_;
    
    void refillTokens();
    void updateClassTokens(Class& cls);
};

// Traffic Shaper Factory
class TrafficShaperFactory {
public:
    static std::unique_ptr<TokenBucket> createTokenBucket(uint64_t capacity, uint64_t refill_rate, uint64_t burst_size);
    static std::unique_ptr<WFQ> createWFQ(uint32_t max_queues);
    static std::unique_ptr<DRR> createDRR(uint32_t max_queues);
    static std::unique_ptr<HTB> createHTB(uint32_t max_classes);
    
    static std::vector<std::string> getAvailableAlgorithms();
};

// Main Traffic Shaper
class TrafficShaper {
public:
    TrafficShaper();
    ~TrafficShaper();

    bool initialize();
    bool processPacket(const Packet& packet);
    bool dequeuePacket(Packet& packet);
    
    void setTokenBucketConfig(uint64_t capacity, uint64_t refill_rate, uint64_t burst_size);
    void setQueueWeight(uint32_t queue_id, uint32_t weight);
    void setEnabled(bool enabled);
    bool isEnabled() const;
    
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
    uint64_t total_packets_processed_;
    uint64_t total_bytes_processed_;
    uint64_t packets_dropped_;
    uint64_t bytes_dropped_;
    
    mutable std::mutex mutex_;
};

} // namespace RouterSim