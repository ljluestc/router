#pragma once

#include <cstdint>
#include <chrono>
#include <memory>
#include <map>
#include <string>
#include <vector>
#include <functional>

namespace RouterSim {

// Forward declarations
struct Packet;
struct ShaperStats;

// Token Bucket implementation
class TokenBucket {
public:
    TokenBucket(uint64_t capacity, uint64_t refill_rate, uint64_t refill_interval_ms);
    ~TokenBucket();

    // Core operations
    bool consume(uint64_t tokens);
    bool tryConsume(uint64_t tokens);
    uint64_t getAvailableTokens() const;
    
    // Configuration
    uint64_t getCapacity() const;
    uint64_t getRefillRate() const;
    void setRefillRate(uint64_t rate);
    void setCapacity(uint64_t capacity);

private:
    uint64_t capacity_;
    uint64_t refill_rate_;
    uint64_t refill_interval_ms_;
    uint64_t tokens_;
    std::chrono::steady_clock::time_point last_refill_time_;
    
    void refill();
};

// Token Bucket Shaper
class TokenBucketShaper {
public:
    TokenBucketShaper(uint64_t capacity, uint64_t refill_rate, uint64_t refill_interval_ms);
    ~TokenBucketShaper();

    // Packet shaping
    bool shapePacket(const Packet& packet);
    bool shapePacket(uint64_t packet_size);
    
    // Statistics
    ShaperStats getStats() const;
    void resetStats();
    
    // Configuration
    void updateRate(uint64_t new_rate);
    void updateCapacity(uint64_t new_capacity);

private:
    TokenBucket bucket_;
    uint64_t total_packets_;
    uint64_t dropped_packets_;
};

// Multi-rate token bucket for different traffic classes
class MultiRateTokenBucket {
public:
    MultiRateTokenBucket();
    ~MultiRateTokenBucket();

    // Bucket management
    void addBucket(const std::string& name, uint64_t capacity, uint64_t refill_rate, uint64_t refill_interval_ms);
    void removeBucket(const std::string& name);
    std::vector<std::string> getBucketNames() const;

    // Token operations
    bool consume(const std::string& bucket_name, uint64_t tokens);
    bool tryConsume(const std::string& bucket_name, uint64_t tokens);
    uint64_t getAvailableTokens(const std::string& bucket_name) const;
    std::map<std::string, uint64_t> getAllAvailableTokens() const;

private:
    std::map<std::string, std::unique_ptr<TokenBucket>> buckets_;
};

// Weighted Fair Queuing (WFQ) implementation
class WFQQueue {
public:
    WFQQueue(uint32_t weight, uint64_t max_size);
    ~WFQQueue();

    // Queue operations
    bool enqueue(const Packet& packet);
    bool dequeue(Packet& packet);
    bool isEmpty() const;
    uint64_t getSize() const;
    uint64_t getMaxSize() const;
    
    // Configuration
    uint32_t getWeight() const;
    void setWeight(uint32_t weight);
    void setMaxSize(uint64_t max_size);

private:
    std::vector<Packet> queue_;
    uint32_t weight_;
    uint64_t max_size_;
    uint64_t virtual_finish_time_;
};

class WFQScheduler {
public:
    WFQScheduler();
    ~WFQScheduler();

    // Queue management
    void addQueue(const std::string& name, uint32_t weight, uint64_t max_size);
    void removeQueue(const std::string& name);
    std::vector<std::string> getQueueNames() const;

    // Scheduling
    bool schedulePacket(const std::string& queue_name, const Packet& packet);
    bool getNextPacket(Packet& packet);
    bool hasPackets() const;

    // Statistics
    std::map<std::string, uint64_t> getQueueSizes() const;
    std::map<std::string, uint64_t> getQueueWeights() const;
    void resetStats();

private:
    std::map<std::string, std::unique_ptr<WFQQueue>> queues_;
    uint64_t total_weight_;
    
    void updateTotalWeight();
};

// Traffic Shaper combining multiple algorithms
class TrafficShaper {
public:
    TrafficShaper();
    ~TrafficShaper();

    // Configuration
    bool addTokenBucket(const std::string& name, uint64_t capacity, uint64_t refill_rate, uint64_t refill_interval_ms);
    bool addWFQQueue(const std::string& name, uint32_t weight, uint64_t max_size);
    bool removeShaper(const std::string& name);

    // Traffic shaping
    bool shapePacket(const std::string& shaper_name, const Packet& packet);
    bool getShapedPacket(Packet& packet);
    bool hasShapedPackets() const;

    // Statistics
    std::map<std::string, ShaperStats> getAllStats() const;
    ShaperStats getStats(const std::string& shaper_name) const;
    void resetAllStats();

    // Configuration management
    bool loadConfiguration(const std::string& config_file);
    bool saveConfiguration(const std::string& config_file);

private:
    std::map<std::string, std::unique_ptr<TokenBucketShaper>> token_buckets_;
    std::unique_ptr<WFQScheduler> wfq_scheduler_;
    
    // Internal methods
    bool isTokenBucket(const std::string& name) const;
    bool isWFQQueue(const std::string& name) const;
};

// Packet structure
struct Packet {
    uint64_t size;
    uint64_t timestamp;
    uint32_t priority;
    std::string source;
    std::string destination;
    std::string protocol;
    std::vector<uint8_t> data;
    
    Packet() : size(0), timestamp(0), priority(0) {}
    Packet(uint64_t s) : size(s), timestamp(0), priority(0) {}
};

// Statistics structure
struct ShaperStats {
    uint64_t total_packets;
    uint64_t dropped_packets;
    uint64_t accepted_packets;
    double drop_rate;
    uint64_t available_tokens;
    uint64_t capacity;
    uint64_t refill_rate;
    uint64_t queue_size;
    uint32_t weight;
    
    ShaperStats() : total_packets(0), dropped_packets(0), accepted_packets(0), 
                   drop_rate(0.0), available_tokens(0), capacity(0), 
                   refill_rate(0), queue_size(0), weight(0) {}
};

} // namespace RouterSim