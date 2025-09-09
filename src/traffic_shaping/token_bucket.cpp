#include "traffic_shaping.h"
#include <chrono>
#include <algorithm>
#include <cmath>

namespace RouterSim {

TokenBucket::TokenBucket(uint64_t capacity, uint64_t refill_rate, uint64_t burst_size)
    : capacity_(capacity)
    , refill_rate_(refill_rate)
    , burst_size_(burst_size)
    , tokens_(capacity)
    , last_refill_time_(std::chrono::steady_clock::now())
    , total_packets_processed_(0)
    , total_bytes_processed_(0)
    , packets_dropped_(0)
    , bytes_dropped_(0)
{
}

TokenBucket::~TokenBucket() {
}

bool TokenBucket::consume(uint64_t tokens) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Refill tokens based on time elapsed
    refillTokens();
    
    // Check if we have enough tokens
    if (tokens_ >= tokens) {
        tokens_ -= tokens;
        total_packets_processed_++;
        total_bytes_processed_ += tokens;
        return true;
    }
    
    // Not enough tokens, drop packet
    packets_dropped_++;
    bytes_dropped_ += tokens;
    return false;
}

bool TokenBucket::consumePacket(const Packet& packet) {
    uint64_t packet_size = packet.size;
    
    // Apply burst size limit
    if (packet_size > burst_size_) {
        packets_dropped_++;
        bytes_dropped_ += packet_size;
        return false;
    }
    
    return consume(packet_size);
}

void TokenBucket::refillTokens() {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - last_refill_time_);
    
    if (elapsed.count() > 0) {
        // Calculate tokens to add based on refill rate and elapsed time
        double elapsed_seconds = elapsed.count() / 1000000.0;
        uint64_t tokens_to_add = static_cast<uint64_t>(refill_rate_ * elapsed_seconds);
        
        // Add tokens, but don't exceed capacity
        tokens_ = std::min(capacity_, tokens_ + tokens_to_add);
        last_refill_time_ = now;
    }
}

void TokenBucket::setCapacity(uint64_t capacity) {
    std::lock_guard<std::mutex> lock(mutex_);
    capacity_ = capacity;
    tokens_ = std::min(tokens_, capacity_);
}

void TokenBucket::setRefillRate(uint64_t refill_rate) {
    std::lock_guard<std::mutex> lock(mutex_);
    refill_rate_ = refill_rate;
}

void TokenBucket::setBurstSize(uint64_t burst_size) {
    std::lock_guard<std::mutex> lock(mutex_);
    burst_size_ = burst_size;
}

uint64_t TokenBucket::getAvailableTokens() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return tokens_;
}

TokenBucket::Statistics TokenBucket::getStatistics() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    Statistics stats;
    stats.capacity = capacity_;
    stats.refill_rate = refill_rate_;
    stats.burst_size = burst_size_;
    stats.available_tokens = tokens_;
    stats.total_packets_processed = total_packets_processed_;
    stats.total_bytes_processed = total_bytes_processed_;
    stats.packets_dropped = packets_dropped_;
    stats.bytes_dropped = bytes_dropped_;
    
    // Calculate utilization percentage
    if (total_bytes_processed_ + bytes_dropped_ > 0) {
        stats.utilization_percentage = (double)total_bytes_processed_ / 
                                     (total_bytes_processed_ + bytes_dropped_) * 100.0;
    } else {
        stats.utilization_percentage = 0.0;
    }
    
    return stats;
}

void TokenBucket::reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    tokens_ = capacity_;
    last_refill_time_ = std::chrono::steady_clock::now();
    total_packets_processed_ = 0;
    total_bytes_processed_ = 0;
    packets_dropped_ = 0;
    bytes_dropped_ = 0;
}

// Weighted Fair Queueing Implementation
WFQ::WFQ(uint32_t max_queues)
    : max_queues_(max_queues)
    , total_weight_(0)
    , virtual_time_(0)
    , last_update_time_(std::chrono::steady_clock::now())
    , total_packets_processed_(0)
    , total_bytes_processed_(0)
    , packets_dropped_(0)
    , bytes_dropped_(0)
{
    queues_.resize(max_queues_);
    for (auto& queue : queues_) {
        queue.weight = 1;
        queue.packets = 0;
        queue.bytes = 0;
        queue.finish_time = 0;
    }
}

WFQ::~WFQ() {
}

bool WFQ::enqueue(uint32_t queue_id, const Packet& packet) {
    if (queue_id >= max_queues_) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Check if queue is full
    if (queues_[queue_id].packets >= MAX_QUEUE_SIZE) {
        packets_dropped_++;
        bytes_dropped_ += packet.size;
        return false;
    }
    
    // Add packet to queue
    queues_[queue_id].packets++;
    queues_[queue_id].bytes += packet.size;
    
    // Calculate finish time for this packet
    updateVirtualTime();
    double finish_time = virtual_time_ + (double)packet.size / queues_[queue_id].weight;
    queues_[queue_id].finish_time = finish_time;
    
    return true;
}

bool WFQ::dequeue(Packet& packet) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    updateVirtualTime();
    
    // Find queue with minimum finish time
    int32_t selected_queue = -1;
    double min_finish_time = std::numeric_limits<double>::max();
    
    for (uint32_t i = 0; i < max_queues_; ++i) {
        if (queues_[i].packets > 0 && queues_[i].finish_time < min_finish_time) {
            min_finish_time = queues_[i].finish_time;
            selected_queue = i;
        }
    }
    
    if (selected_queue == -1) {
        return false; // No packets to dequeue
    }
    
    // Dequeue packet (simplified - in real implementation, we'd have actual packet storage)
    queues_[selected_queue].packets--;
    queues_[selected_queue].bytes -= packet.size;
    
    total_packets_processed_++;
    total_bytes_processed_ += packet.size;
    
    return true;
}

void WFQ::setQueueWeight(uint32_t queue_id, uint32_t weight) {
    if (queue_id >= max_queues_) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    total_weight_ -= queues_[queue_id].weight;
    queues_[queue_id].weight = weight;
    total_weight_ += weight;
}

uint32_t WFQ::getQueueWeight(uint32_t queue_id) const {
    if (queue_id >= max_queues_) {
        return 0;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    return queues_[queue_id].weight;
}

uint32_t WFQ::getQueueSize(uint32_t queue_id) const {
    if (queue_id >= max_queues_) {
        return 0;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    return queues_[queue_id].packets;
}

uint64_t WFQ::getQueueBytes(uint32_t queue_id) const {
    if (queue_id >= max_queues_) {
        return 0;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    return queues_[queue_id].bytes;
}

void WFQ::updateVirtualTime() {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - last_update_time_);
    
    if (elapsed.count() > 0 && total_weight_ > 0) {
        double elapsed_seconds = elapsed.count() / 1000000.0;
        virtual_time_ += elapsed_seconds * total_weight_;
        last_update_time_ = now;
    }
}

WFQ::Statistics WFQ::getStatistics() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    Statistics stats;
    stats.max_queues = max_queues_;
    stats.total_weight = total_weight_;
    stats.virtual_time = virtual_time_;
    stats.total_packets_processed = total_packets_processed_;
    stats.total_bytes_processed = total_bytes_processed_;
    stats.packets_dropped = packets_dropped_;
    stats.bytes_dropped = bytes_dropped_;
    
    // Calculate queue statistics
    for (uint32_t i = 0; i < max_queues_; ++i) {
        QueueStatistics queue_stats;
        queue_stats.queue_id = i;
        queue_stats.weight = queues_[i].weight;
        queue_stats.packets = queues_[i].packets;
        queue_stats.bytes = queues_[i].bytes;
        queue_stats.finish_time = queues_[i].finish_time;
        stats.queue_stats.push_back(queue_stats);
    }
    
    return stats;
}

void WFQ::reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (auto& queue : queues_) {
        queue.packets = 0;
        queue.bytes = 0;
        queue.finish_time = 0;
    }
    
    virtual_time_ = 0;
    last_update_time_ = std::chrono::steady_clock::now();
    total_packets_processed_ = 0;
    total_bytes_processed_ = 0;
    packets_dropped_ = 0;
    bytes_dropped_ = 0;
}

// Traffic Shaper Implementation
TrafficShaper::TrafficShaper()
    : enabled_(false)
    , total_packets_processed_(0)
    , total_bytes_processed_(0)
    , packets_dropped_(0)
    , bytes_dropped_(0)
{
}

TrafficShaper::~TrafficShaper() {
}

bool TrafficShaper::initialize() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Initialize default token bucket
    token_bucket_ = std::make_unique<TokenBucket>(1000000, 100000, 1500); // 1MB capacity, 100KB/s rate, 1500 byte burst
    
    // Initialize WFQ with 8 queues
    wfq_ = std::make_unique<WFQ>(8);
    
    // Set default queue weights (equal weights)
    for (uint32_t i = 0; i < 8; ++i) {
        wfq_->setQueueWeight(i, 1);
    }
    
    enabled_ = true;
    return true;
}

bool TrafficShaper::processPacket(const Packet& packet) {
    if (!enabled_) {
        return true; // Pass through if disabled
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    // First, apply token bucket rate limiting
    if (!token_bucket_->consumePacket(packet)) {
        packets_dropped_++;
        bytes_dropped_ += packet.size;
        return false;
    }
    
    // Then, apply WFQ scheduling
    uint32_t queue_id = packet.priority % wfq_->getMaxQueues();
    if (!wfq_->enqueue(queue_id, packet)) {
        packets_dropped_++;
        bytes_dropped_ += packet.size;
        return false;
    }
    
    total_packets_processed_++;
    total_bytes_processed_ += packet.size;
    
    return true;
}

bool TrafficShaper::dequeuePacket(Packet& packet) {
    if (!enabled_) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    return wfq_->dequeue(packet);
}

void TrafficShaper::setTokenBucketConfig(uint64_t capacity, uint64_t refill_rate, uint64_t burst_size) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (token_bucket_) {
        token_bucket_->setCapacity(capacity);
        token_bucket_->setRefillRate(refill_rate);
        token_bucket_->setBurstSize(burst_size);
    }
}

void TrafficShaper::setQueueWeight(uint32_t queue_id, uint32_t weight) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (wfq_) {
        wfq_->setQueueWeight(queue_id, weight);
    }
}

void TrafficShaper::setEnabled(bool enabled) {
    std::lock_guard<std::mutex> lock(mutex_);
    enabled_ = enabled;
}

bool TrafficShaper::isEnabled() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return enabled_;
}

TrafficShaper::Statistics TrafficShaper::getStatistics() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    Statistics stats;
    stats.enabled = enabled_;
    stats.total_packets_processed = total_packets_processed_;
    stats.total_bytes_processed = total_bytes_processed_;
    stats.packets_dropped = packets_dropped_;
    stats.bytes_dropped = bytes_dropped_;
    
    if (token_bucket_) {
        stats.token_bucket_stats = token_bucket_->getStatistics();
    }
    
    if (wfq_) {
        stats.wfq_stats = wfq_->getStatistics();
    }
    
    return stats;
}

void TrafficShaper::reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (token_bucket_) {
        token_bucket_->reset();
    }
    
    if (wfq_) {
        wfq_->reset();
    }
    
    total_packets_processed_ = 0;
    total_bytes_processed_ = 0;
    packets_dropped_ = 0;
    bytes_dropped_ = 0;
}

} // namespace RouterSim