#include "traffic_shaping.h"
#include "common_types.h"
#include <iostream>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <map>
#include <functional>

namespace RouterSim {

// TokenBucket implementation
TokenBucket::TokenBucket(uint64_t capacity, uint64_t refill_rate, uint64_t burst_size)
    : capacity_(capacity), refill_rate_(refill_rate), burst_size_(burst_size), 
      tokens_(capacity), last_refill_time_(std::chrono::steady_clock::now()),
      total_packets_processed_(0), total_bytes_processed_(0), 
      packets_dropped_(0), bytes_dropped_(0) {
}

TokenBucket::~TokenBucket() = default;

bool TokenBucket::consume(uint64_t tokens) {
    std::lock_guard<std::mutex> lock(mutex_);
    refillTokens();
    
    if (tokens_ >= tokens) {
        tokens_ -= tokens;
        return true;
    }
    return false;
}

bool TokenBucket::consumePacket(const PacketInfo& packet) {
    return consume(packet.size);
}

void TokenBucket::refillTokens() {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_refill_time_);
    
    if (elapsed.count() > 0) {
        uint64_t tokens_to_add = (refill_rate_ * elapsed.count()) / 1000;
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
    const_cast<TokenBucket*>(this)->refillTokens();
    return tokens_;
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
    stats.utilization_percentage = capacity_ > 0 ? (double)total_bytes_processed_ / capacity_ * 100.0 : 0.0;
    return stats;
}

// WFQ implementation
WFQ::WFQ(uint32_t max_queues) 
    : max_queues_(max_queues), total_weight_(0), virtual_time_(0.0),
      last_update_time_(std::chrono::steady_clock::now()),
      total_packets_processed_(0), total_bytes_processed_(0),
      packets_dropped_(0), bytes_dropped_(0) {
    queues_.resize(max_queues_);
    for (uint32_t i = 0; i < max_queues_; ++i) {
        queues_[i].weight = 1;
        queues_[i].packets = 0;
        queues_[i].bytes = 0;
        queues_[i].finish_time = 0.0;
    }
}

WFQ::~WFQ() = default;

bool WFQ::enqueue(uint32_t queue_id, const PacketInfo& packet) {
    if (queue_id >= max_queues_) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    updateVirtualTime();
    
    if (queues_[queue_id].packets >= MAX_QUEUE_SIZE) {
        packets_dropped_++;
        bytes_dropped_ += packet.size;
        return false;
    }
    
    queues_[queue_id].packets++;
    queues_[queue_id].bytes += packet.size;
    total_packets_processed_++;
    total_bytes_processed_ += packet.size;
    
    return true;
}

bool WFQ::dequeue(PacketInfo& packet) {
    std::lock_guard<std::mutex> lock(mutex_);
    updateVirtualTime();
    
    // Find queue with minimum finish time
    uint32_t selected_queue = 0;
    double min_finish_time = std::numeric_limits<double>::max();
    
    for (uint32_t i = 0; i < max_queues_; ++i) {
        if (queues_[i].packets > 0) {
            double finish_time = queues_[i].finish_time;
            if (finish_time < min_finish_time) {
                min_finish_time = finish_time;
                selected_queue = i;
            }
        }
    }
    
    if (queues_[selected_queue].packets == 0) {
        return false;
    }
    
    // Dequeue packet (simplified - just decrement counters)
    queues_[selected_queue].packets--;
    queues_[selected_queue].bytes -= packet.size; // This is simplified
    
    return true;
}

void WFQ::setQueueWeight(uint32_t queue_id, uint32_t weight) {
    if (queue_id >= max_queues_) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    total_weight_ = total_weight_ - queues_[queue_id].weight + weight;
    queues_[queue_id].weight = weight;
}

uint32_t WFQ::getQueueWeight(uint32_t queue_id) const {
    if (queue_id >= max_queues_) {
        return 0;
    }
    return queues_[queue_id].weight;
}

uint32_t WFQ::getQueueSize(uint32_t queue_id) const {
    if (queue_id >= max_queues_) {
        return 0;
    }
    return queues_[queue_id].packets;
}

uint64_t WFQ::getQueueBytes(uint32_t queue_id) const {
    if (queue_id >= max_queues_) {
        return 0;
    }
    return queues_[queue_id].bytes;
}

void WFQ::updateVirtualTime() {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_update_time_);
    
    if (elapsed.count() > 0) {
        // Simplified virtual time update
        virtual_time_ += elapsed.count() / 1000.0;
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
    
    stats.queue_stats.clear();
    for (uint32_t i = 0; i < max_queues_; ++i) {
        QueueStatistics queue_stat;
        queue_stat.queue_id = i;
        queue_stat.weight = queues_[i].weight;
        queue_stat.packets = queues_[i].packets;
        queue_stat.bytes = queues_[i].bytes;
        queue_stat.finish_time = queues_[i].finish_time;
        stats.queue_stats.push_back(queue_stat);
    }
    
    return stats;
}

void WFQ::reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& queue : queues_) {
        queue.packets = 0;
        queue.bytes = 0;
        queue.finish_time = 0.0;
    }
    virtual_time_ = 0.0;
    total_packets_processed_ = 0;
    total_bytes_processed_ = 0;
    packets_dropped_ = 0;
    bytes_dropped_ = 0;
}

// TrafficShaper implementation
TrafficShaper::TrafficShaper() 
    : enabled_(false), total_packets_processed_(0), total_bytes_processed_(0),
      packets_dropped_(0), bytes_dropped_(0) {
    token_bucket_ = std::make_unique<TokenBucket>(1000, 100, 500);
    wfq_ = std::make_unique<WFQ>(8);
}

TrafficShaper::~TrafficShaper() = default;

bool TrafficShaper::initialize() {
    return true;
}

bool TrafficShaper::processPacket(const PacketInfo& packet) {
    if (!enabled_) {
        return true;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Token bucket check
    if (!token_bucket_->consumePacket(packet)) {
        packets_dropped_++;
        bytes_dropped_ += packet.size;
        return false;
    }
    
    // WFQ processing
    if (!wfq_->enqueue(packet.priority % wfq_->getMaxQueues(), packet)) {
        packets_dropped_++;
        bytes_dropped_ += packet.size;
        return false;
    }
    
    total_packets_processed_++;
    total_bytes_processed_ += packet.size;
    
    return true;
}

bool TrafficShaper::dequeuePacket(PacketInfo& packet) {
    if (!enabled_) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    return wfq_->dequeue(packet);
}

void TrafficShaper::setTokenBucketConfig(uint64_t capacity, uint64_t refill_rate, uint64_t burst_size) {
    std::lock_guard<std::mutex> lock(mutex_);
    token_bucket_->setCapacity(capacity);
    token_bucket_->setRefillRate(refill_rate);
    token_bucket_->setBurstSize(burst_size);
}

void TrafficShaper::setQueueWeight(uint32_t queue_id, uint32_t weight) {
    std::lock_guard<std::mutex> lock(mutex_);
    wfq_->setQueueWeight(queue_id, weight);
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
    stats.token_bucket_stats = token_bucket_->getStatistics();
    stats.wfq_stats = wfq_->getStatistics();
    return stats;
}

void TrafficShaper::reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    token_bucket_->reset();
    wfq_->reset();
    total_packets_processed_ = 0;
    total_bytes_processed_ = 0;
    packets_dropped_ = 0;
    bytes_dropped_ = 0;
}

// TrafficShapingManager implementation
TrafficShapingManager::TrafficShapingManager() 
    : running_(false), initialized_(false) {
}

TrafficShapingManager::~TrafficShapingManager() {
    stop();
}

bool TrafficShapingManager::initialize() {
    if (initialized_) {
        return true;
    }
    
    initialized_ = true;
    return true;
}

bool TrafficShapingManager::start() {
    if (!initialized_) {
        return false;
    }
    
    if (running_) {
        return true;
    }
    
    running_ = true;
    return true;
}

bool TrafficShapingManager::stop() {
    if (!running_) {
        return true;
    }
    
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    for (auto& [interface_name, shaper] : interfaces_) {
        shaper->setEnabled(false);
    }
    
    running_ = false;
    return true;
}

bool TrafficShapingManager::is_running() const {
    return running_;
}

bool TrafficShapingManager::add_interface(const std::string& interface_name) {
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    if (interfaces_.find(interface_name) != interfaces_.end()) {
        return false; // Interface already exists
    }
    
    auto shaper = std::make_unique<TrafficShaper>();
    if (!shaper->initialize()) {
        return false;
    }
    
    interfaces_[interface_name] = std::move(shaper);
    return true;
}

bool TrafficShapingManager::remove_interface(const std::string& interface_name) {
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    auto it = interfaces_.find(interface_name);
    if (it == interfaces_.end()) {
        return false; // Interface not found
    }
    
    interfaces_.erase(it);
    return true;
}

bool TrafficShapingManager::configure_interface(const std::string& interface_name, 
                                               ShapingAlgorithm algorithm,
                                               const std::map<std::string, std::string>& config) {
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    auto it = interfaces_.find(interface_name);
    if (it == interfaces_.end()) {
        return false; // Interface not found
    }
    
    auto& shaper = it->second;
    
    if (algorithm == ShapingAlgorithm::TOKEN_BUCKET) {
        if (config.find("capacity") != config.end() && 
            config.find("rate") != config.end() && 
            config.find("burst_size") != config.end()) {
            uint64_t capacity = std::stoull(config.at("capacity"));
            uint64_t rate = std::stoull(config.at("rate"));
            uint64_t burst_size = std::stoull(config.at("burst_size"));
            shaper->setTokenBucketConfig(capacity, rate, burst_size);
        }
    } else if (algorithm == ShapingAlgorithm::WEIGHTED_FAIR_QUEUE) {
        // Configure WFQ weights
        for (const auto& [key, value] : config) {
            if (key.find("weight_") == 0) {
                uint32_t queue_id = std::stoul(key.substr(7));
                uint32_t weight = std::stoul(value);
                shaper->setQueueWeight(queue_id, weight);
            }
        }
    }
    
    shaper->setEnabled(true);
    return true;
}

bool TrafficShapingManager::process_packet(const std::string& interface_name, const PacketInfo& packet) {
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    auto it = interfaces_.find(interface_name);
    if (it == interfaces_.end()) {
        return false; // Interface not found
    }
    
    return it->second->processPacket(packet);
}

std::map<std::string, TrafficStats> TrafficShapingManager::get_interface_statistics() const {
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    std::map<std::string, TrafficStats> stats;
    
    for (const auto& [interface_name, shaper] : interfaces_) {
        auto shaper_stats = shaper->getStatistics();
        TrafficStats interface_stats;
        interface_stats.packets_processed = shaper_stats.total_packets_processed;
        interface_stats.bytes_processed = shaper_stats.total_bytes_processed;
        interface_stats.packets_dropped = shaper_stats.packets_dropped;
        interface_stats.bytes_dropped = shaper_stats.bytes_dropped;
        stats[interface_name] = interface_stats;
    }
    
    return stats;
}

TrafficStats TrafficShapingManager::get_global_statistics() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    return global_stats_;
}

bool TrafficShapingManager::load_config(const std::string& config_file) {
    // Simplified implementation
    return true;
}

bool TrafficShapingManager::save_config(const std::string& config_file) const {
    // Simplified implementation
    return true;
}

void TrafficShapingManager::set_packet_callback(PacketCallback callback) {
    packet_callback_ = callback;
}

void TrafficShapingManager::set_drop_callback(DropCallback callback) {
    drop_callback_ = callback;
}

void TrafficShapingManager::processing_loop() {
    // Simplified implementation
}

bool TrafficShapingManager::process_packet_internal(const PacketInfo& packet) {
    // Simplified implementation
    return true;
}

void TrafficShapingManager::update_statistics(const PacketInfo& packet, bool dropped) {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    if (dropped) {
        global_stats_.packets_dropped++;
        global_stats_.bytes_dropped += packet.size;
    } else {
        global_stats_.packets_processed++;
        global_stats_.bytes_processed += packet.size;
    }
}

void TrafficShapingManager::notify_packet_processed(const PacketInfo& packet) {
    if (packet_callback_) {
        packet_callback_(packet);
    }
}

void TrafficShapingManager::notify_packet_dropped(const PacketInfo& packet, const std::string& reason) {
    if (drop_callback_) {
        drop_callback_(packet, reason);
    }
}

} // namespace RouterSim
