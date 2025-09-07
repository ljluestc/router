#include "traffic_shaping.h"
#include <iostream>
#include <algorithm>
#include <chrono>

using namespace RouterSim;

// TokenBucket implementation
TokenBucket::TokenBucket(uint32_t rate_bps, uint32_t burst_size)
    : rate_bps_(rate_bps), burst_size_(burst_size), tokens_(burst_size),
      last_refill_(std::chrono::steady_clock::now()) {
}

bool TokenBucket::consume_tokens(uint32_t tokens) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    refill_tokens();
    
    if (tokens_ >= tokens) {
        tokens_ -= tokens;
        return true;
    }
    
    return false;
}

void TokenBucket::refill_tokens() {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - last_refill_);
    
    uint64_t tokens_to_add = (rate_bps_ * elapsed.count()) / 1000000;
    tokens_ = std::min(burst_size_, tokens_ + static_cast<uint32_t>(tokens_to_add));
    last_refill_ = now;
}

uint32_t TokenBucket::get_available_tokens() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return tokens_;
}

void TokenBucket::set_rate(uint32_t rate_bps) {
    std::lock_guard<std::mutex> lock(mutex_);
    rate_bps_ = rate_bps;
}

void TokenBucket::set_burst_size(uint32_t burst_size) {
    std::lock_guard<std::mutex> lock(mutex_);
    burst_size_ = burst_size;
    tokens_ = std::min(tokens_, burst_size);
}

// WFQQueue implementation
WFQQueue::WFQQueue(uint32_t weight, uint32_t max_size)
    : weight_(weight), max_size_(max_size), virtual_finish_time_(0) {
}

bool WFQQueue::enqueue(const Packet& packet) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (packets_.size() >= max_size_) {
        return false; // Queue full
    }
    
    packets_.push(packet);
    return true;
}

bool WFQQueue::dequeue(Packet& packet) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (packets_.empty()) {
        return false;
    }
    
    packet = packets_.front();
    packets_.pop();
    return true;
}

bool WFQQueue::is_empty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return packets_.empty();
}

uint32_t WFQQueue::size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return packets_.size();
}

uint32_t WFQQueue::get_weight() const {
    return weight_;
}

void WFQQueue::set_weight(uint32_t weight) {
    weight_ = weight;
}

uint64_t WFQQueue::get_virtual_finish_time() const {
    return virtual_finish_time_;
}

void WFQQueue::set_virtual_finish_time(uint64_t time) {
    virtual_finish_time_ = time;
}

// TrafficShaper implementation
TrafficShaper::TrafficShaper() : running_(false) {
}

TrafficShaper::~TrafficShaper() {
    stop();
}

bool TrafficShaper::add_interface(const std::string& interface, const ShapingConfig& config) {
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    InterfaceShaping shaping;
    shaping.wfq_enabled = config.enable_wfq;
    shaping.current_queue = 0;
    
    // Create token bucket if rate limiting is enabled
    if (config.rate_bps > 0) {
        shaping.token_bucket = std::make_unique<TokenBucket>(config.rate_bps, config.burst_size);
    }
    
    // Create WFQ queues if enabled
    if (config.enable_wfq) {
        for (uint32_t i = 0; i < config.num_queues; i++) {
            uint32_t weight = (i == 0) ? 1 : (i + 1); // Default weights
            shaping.wfq_queues.push_back(std::make_unique<WFQQueue>(weight, config.queue_size));
        }
    }
    
    interfaces_[interface] = std::move(shaping);
    return true;
}

bool TrafficShaper::remove_interface(const std::string& interface) {
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    auto it = interfaces_.find(interface);
    if (it == interfaces_.end()) {
        return false;
    }
    
    interfaces_.erase(it);
    return true;
}

bool TrafficShaper::update_interface_config(const std::string& interface, const ShapingConfig& config) {
    return add_interface(interface, config); // Same as add for now
}

bool TrafficShaper::enable_token_bucket(const std::string& interface, uint32_t rate_bps, uint32_t burst_size) {
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    auto it = interfaces_.find(interface);
    if (it == interfaces_.end()) {
        return false;
    }
    
    it->second.token_bucket = std::make_unique<TokenBucket>(rate_bps, burst_size);
    return true;
}

bool TrafficShaper::disable_token_bucket(const std::string& interface) {
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    auto it = interfaces_.find(interface);
    if (it == interfaces_.end()) {
        return false;
    }
    
    it->second.token_bucket.reset();
    return true;
}

bool TrafficShaper::enable_wfq(const std::string& interface, uint32_t num_queues) {
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    auto it = interfaces_.find(interface);
    if (it == interfaces_.end()) {
        return false;
    }
    
    it->second.wfq_enabled = true;
    it->second.wfq_queues.clear();
    
    for (uint32_t i = 0; i < num_queues; i++) {
        uint32_t weight = (i == 0) ? 1 : (i + 1); // Default weights
        it->second.wfq_queues.push_back(std::make_unique<WFQQueue>(weight, 1000));
    }
    
    return true;
}

bool TrafficShaper::disable_wfq(const std::string& interface) {
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    auto it = interfaces_.find(interface);
    if (it == interfaces_.end()) {
        return false;
    }
    
    it->second.wfq_enabled = false;
    it->second.wfq_queues.clear();
    
    return true;
}

bool TrafficShaper::set_queue_weight(const std::string& interface, uint32_t queue_id, uint32_t weight) {
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    auto it = interfaces_.find(interface);
    if (it == interfaces_.end() || !it->second.wfq_enabled) {
        return false;
    }
    
    if (queue_id >= it->second.wfq_queues.size()) {
        return false;
    }
    
    it->second.wfq_queues[queue_id]->set_weight(weight);
    return true;
}

bool TrafficShaper::shape_packet(const std::string& interface, Packet& packet) {
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    auto it = interfaces_.find(interface);
    if (it == interfaces_.end()) {
        return false;
    }
    
    // Apply token bucket shaping
    if (it->second.token_bucket) {
        if (!process_token_bucket(interface, packet)) {
            return false; // Packet dropped by token bucket
        }
    }
    
    // Apply WFQ shaping
    if (it->second.wfq_enabled) {
        if (!process_wfq(interface, packet)) {
            return false; // Packet dropped by WFQ
        }
    }
    
    return true;
}

bool TrafficShaper::process_shaped_packets() {
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    bool processed = false;
    
    for (auto& pair : interfaces_) {
        auto& shaping = pair.second;
        
        // Process WFQ queues
        if (shaping.wfq_enabled) {
            for (auto& queue : shaping.wfq_queues) {
                Packet packet;
                if (queue->dequeue(packet)) {
                    shaping.output_queue.push(packet);
                    processed = true;
                }
            }
        }
    }
    
    return processed;
}

std::map<std::string, uint64_t> TrafficShaper::get_interface_stats(const std::string& interface) const {
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    auto it = interfaces_.find(interface);
    if (it == interfaces_.end()) {
        return {};
    }
    
    return it->second.stats;
}

std::map<std::string, uint64_t> TrafficShaper::get_queue_stats(const std::string& interface, uint32_t queue_id) const {
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    auto it = interfaces_.find(interface);
    if (it == interfaces_.end() || !it->second.wfq_enabled) {
        return {};
    }
    
    if (queue_id >= it->second.wfq_queues.size()) {
        return {};
    }
    
    std::map<std::string, uint64_t> stats;
    stats["queue_size"] = it->second.wfq_queues[queue_id]->size();
    stats["weight"] = it->second.wfq_queues[queue_id]->get_weight();
    
    return stats;
}

void TrafficShaper::reset_statistics() {
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    for (auto& pair : interfaces_) {
        pair.second.stats.clear();
    }
    
    global_stats_.clear();
}

void TrafficShaper::start() {
    if (running_) {
        return;
    }
    
    running_ = true;
    processing_thread_ = std::thread(&TrafficShaper::processing_loop, this);
    refill_thread_ = std::thread(&TrafficShaper::token_refill_loop, this);
}

void TrafficShaper::stop() {
    if (!running_) {
        return;
    }
    
    running_ = false;
    
    if (processing_thread_.joinable()) {
        processing_thread_.join();
    }
    
    if (refill_thread_.joinable()) {
        refill_thread_.join();
    }
}

bool TrafficShaper::is_running() const {
    return running_;
}

bool TrafficShaper::process_token_bucket(const std::string& interface, Packet& packet) {
    auto it = interfaces_.find(interface);
    if (it == interfaces_.end() || !it->second.token_bucket) {
        return true; // No token bucket, allow packet
    }
    
    uint32_t tokens_needed = packet.size * 8; // Convert bytes to bits
    if (it->second.token_bucket->consume_tokens(tokens_needed)) {
        it->second.stats["packets_passed"]++;
        it->second.stats["bytes_passed"] += packet.size;
        return true;
    } else {
        it->second.stats["packets_dropped"]++;
        it->second.stats["bytes_dropped"] += packet.size;
        return false;
    }
}

bool TrafficShaper::process_wfq(const std::string& interface, Packet& packet) {
    auto it = interfaces_.find(interface);
    if (it == interfaces_.end() || !it->second.wfq_enabled) {
        return true; // No WFQ, allow packet
    }
    
    uint32_t queue_id = select_wfq_queue(interface, packet);
    if (queue_id >= it->second.wfq_queues.size()) {
        return false; // Invalid queue
    }
    
    if (it->second.wfq_queues[queue_id]->enqueue(packet)) {
        it->second.stats["packets_queued"]++;
        it->second.stats["bytes_queued"] += packet.size;
        return true;
    } else {
        it->second.stats["packets_dropped"]++;
        it->second.stats["bytes_dropped"] += packet.size;
        return false;
    }
}

uint32_t TrafficShaper::select_wfq_queue(const std::string& interface, const Packet& packet) {
    // Simple round-robin selection for now
    // In a real implementation, this would use packet priority, DSCP, or other criteria
    auto it = interfaces_.find(interface);
    if (it == interfaces_.end() || !it->second.wfq_enabled) {
        return 0;
    }
    
    uint32_t queue_id = it->second.current_queue;
    it->second.current_queue = (it->second.current_queue + 1) % it->second.wfq_queues.size();
    
    return queue_id;
}

void TrafficShaper::refill_all_token_buckets() {
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    for (auto& pair : interfaces_) {
        if (pair.second.token_bucket) {
            pair.second.token_bucket->refill_tokens();
        }
    }
}

void TrafficShaper::processing_loop() {
    while (running_) {
        process_shaped_packets();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void TrafficShaper::token_refill_loop() {
    while (running_) {
        refill_all_token_buckets();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

// ShapingAlgorithms implementation
bool ShapingAlgorithms::leaky_bucket(const Packet& packet, uint32_t rate_bps, uint32_t bucket_size) {
    // Simple leaky bucket implementation
    static std::map<std::string, uint32_t> buckets;
    static std::map<std::string, std::chrono::steady_clock::time_point> last_update;
    
    std::string key = packet.source_interface;
    auto now = std::chrono::steady_clock::now();
    
    if (last_update.find(key) == last_update.end()) {
        last_update[key] = now;
        buckets[key] = bucket_size;
    }
    
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - last_update[key]);
    uint32_t tokens_to_add = (rate_bps * elapsed.count()) / 1000000;
    
    buckets[key] = std::min(bucket_size, buckets[key] + tokens_to_add);
    last_update[key] = now;
    
    uint32_t tokens_needed = packet.size * 8;
    if (buckets[key] >= tokens_needed) {
        buckets[key] -= tokens_needed;
        return true;
    }
    
    return false;
}

bool ShapingAlgorithms::token_bucket(const Packet& packet, uint32_t rate_bps, uint32_t burst_size) {
    // This is a simplified version - the actual implementation is in TokenBucket class
    return true; // Placeholder
}

uint32_t ShapingAlgorithms::calculate_wfq_weight(const Packet& packet, uint32_t num_queues) {
    // Simple weight calculation based on packet priority
    uint32_t priority = packet.priority % num_queues;
    return priority + 1; // Weight starts from 1
}

uint32_t ShapingAlgorithms::calculate_priority_queue(const Packet& packet, uint32_t num_queues) {
    // Calculate priority queue based on packet characteristics
    return packet.priority % num_queues;
}

bool ShapingAlgorithms::rate_limit(const Packet& packet, uint32_t rate_bps, uint32_t window_ms) {
    // Simple rate limiting implementation
    static std::map<std::string, std::vector<std::chrono::steady_clock::time_point>> timestamps;
    
    std::string key = packet.source_interface;
    auto now = std::chrono::steady_clock::now();
    
    // Clean old timestamps
    auto cutoff = now - std::chrono::milliseconds(window_ms);
    timestamps[key].erase(
        std::remove_if(timestamps[key].begin(), timestamps[key].end(),
                      [cutoff](const std::chrono::steady_clock::time_point& t) {
                          return t < cutoff;
                      }),
        timestamps[key].end()
    );
    
    // Check if we're within rate limit
    uint32_t max_packets = (rate_bps * window_ms) / (packet.size * 8 * 1000);
    if (timestamps[key].size() < max_packets) {
        timestamps[key].push_back(now);
        return true;
    }
    
    return false;
}
