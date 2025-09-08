#include "traffic_shaping.h"
#include <iostream>
#include <chrono>
#include <algorithm>

namespace RouterSim {

TokenBucket::TokenBucket(uint64_t capacity, uint64_t refill_rate, uint64_t burst_size)
    : capacity_(capacity), refill_rate_(refill_rate), burst_size_(burst_size),
      tokens_(capacity), last_refill_time_(std::chrono::steady_clock::now()) {
}

TokenBucket::~TokenBucket() {
}

bool TokenBucket::consume_tokens(uint64_t tokens) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Refill tokens based on time elapsed
    refill_tokens();
    
    if (tokens_ >= tokens) {
        tokens_ -= tokens;
        return true;
    }
    
    return false;
}

bool TokenBucket::try_consume_tokens(uint64_t tokens) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Refill tokens based on time elapsed
    refill_tokens();
    
    if (tokens_ >= tokens) {
        tokens_ -= tokens;
        return true;
    }
    
    return false;
}

uint64_t TokenBucket::get_available_tokens() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Refill tokens based on time elapsed
    const_cast<TokenBucket*>(this)->refill_tokens();
    
    return tokens_;
}

void TokenBucket::refill_tokens() {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
        now - last_refill_time_).count();
    
    if (elapsed > 0) {
        // Calculate tokens to add based on elapsed time
        uint64_t tokens_to_add = (refill_rate_ * elapsed) / 1000000; // Convert to tokens per microsecond
        
        // Apply burst limit
        tokens_to_add = std::min(tokens_to_add, burst_size_);
        
        tokens_ = std::min(capacity_, tokens_ + tokens_to_add);
        last_refill_time_ = now;
    }
}

void TokenBucket::set_capacity(uint64_t capacity) {
    std::lock_guard<std::mutex> lock(mutex_);
    capacity_ = capacity;
    tokens_ = std::min(tokens_, capacity_);
}

void TokenBucket::set_refill_rate(uint64_t refill_rate) {
    std::lock_guard<std::mutex> lock(mutex_);
    refill_rate_ = refill_rate;
}

void TokenBucket::set_burst_size(uint64_t burst_size) {
    std::lock_guard<std::mutex> lock(mutex_);
    burst_size_ = burst_size;
}

TokenBucket::Statistics TokenBucket::get_statistics() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return stats_;
}

void TokenBucket::reset_statistics() {
    std::lock_guard<std::mutex> lock(mutex_);
    stats_ = Statistics();
}

// TokenBucketShaper implementation
TokenBucketShaper::TokenBucketShaper(uint64_t rate_bps, uint64_t burst_bytes)
    : rate_bps_(rate_bps), burst_bytes_(burst_bytes),
      token_bucket_(burst_bytes * 8, rate_bps, burst_bytes * 8) {
}

TokenBucketShaper::~TokenBucketShaper() {
}

bool TokenBucketShaper::shape_packet(const Packet& packet) {
    uint64_t packet_bits = packet.size * 8;
    
    if (token_bucket_.consume_tokens(packet_bits)) {
        stats_.packets_passed++;
        stats_.bytes_passed += packet.size;
        return true;
    } else {
        stats_.packets_dropped++;
        stats_.bytes_dropped += packet.size;
        return false;
    }
}

bool TokenBucketShaper::shape_packet_non_blocking(const Packet& packet) {
    uint64_t packet_bits = packet.size * 8;
    
    if (token_bucket_.try_consume_tokens(packet_bits)) {
        stats_.packets_passed++;
        stats_.bytes_passed += packet.size;
        return true;
    } else {
        stats_.packets_dropped++;
        stats_.bytes_dropped += packet.size;
        return false;
    }
}

void TokenBucketShaper::set_rate(uint64_t rate_bps) {
    rate_bps_ = rate_bps;
    token_bucket_.set_refill_rate(rate_bps);
}

void TokenBucketShaper::set_burst_size(uint64_t burst_bytes) {
    burst_bytes_ = burst_bytes;
    token_bucket_.set_capacity(burst_bytes * 8);
    token_bucket_.set_burst_size(burst_bytes * 8);
}

TokenBucketShaper::Statistics TokenBucketShaper::get_statistics() const {
    return stats_;
}

void TokenBucketShaper::reset_statistics() {
    stats_ = Statistics();
}

// MultiTokenBucketShaper implementation
MultiTokenBucketShaper::MultiTokenBucketShaper() {
}

MultiTokenBucketShaper::~MultiTokenBucketShaper() {
}

bool MultiTokenBucketShaper::add_shaper(const std::string& name, 
                                       uint64_t rate_bps, 
                                       uint64_t burst_bytes) {
    std::lock_guard<std::mutex> lock(shapers_mutex_);
    
    auto shaper = std::make_unique<TokenBucketShaper>(rate_bps, burst_bytes);
    shapers_[name] = std::move(shaper);
    
    std::cout << "Added token bucket shaper: " << name 
              << " (rate: " << rate_bps << " bps, burst: " << burst_bytes << " bytes)" << std::endl;
    return true;
}

bool MultiTokenBucketShaper::remove_shaper(const std::string& name) {
    std::lock_guard<std::mutex> lock(shapers_mutex_);
    
    auto it = shapers_.find(name);
    if (it != shapers_.end()) {
        shapers_.erase(it);
        std::cout << "Removed token bucket shaper: " << name << std::endl;
        return true;
    }
    
    return false;
}

bool MultiTokenBucketShaper::shape_packet(const std::string& shaper_name, const Packet& packet) {
    std::lock_guard<std::mutex> lock(shapers_mutex_);
    
    auto it = shapers_.find(shaper_name);
    if (it != shapers_.end()) {
        return it->second->shape_packet(packet);
    }
    
    return false;
}

bool MultiTokenBucketShaper::shape_packet_non_blocking(const std::string& shaper_name, const Packet& packet) {
    std::lock_guard<std::mutex> lock(shapers_mutex_);
    
    auto it = shapers_.find(shaper_name);
    if (it != shapers_.end()) {
        return it->second->shape_packet_non_blocking(packet);
    }
    
    return false;
}

void MultiTokenBucketShaper::set_shaper_rate(const std::string& name, uint64_t rate_bps) {
    std::lock_guard<std::mutex> lock(shapers_mutex_);
    
    auto it = shapers_.find(name);
    if (it != shapers_.end()) {
        it->second->set_rate(rate_bps);
    }
}

void MultiTokenBucketShaper::set_shaper_burst_size(const std::string& name, uint64_t burst_bytes) {
    std::lock_guard<std::mutex> lock(shapers_mutex_);
    
    auto it = shapers_.find(name);
    if (it != shapers_.end()) {
        it->second->set_burst_size(burst_bytes);
    }
}

std::vector<std::string> MultiTokenBucketShaper::get_shaper_names() const {
    std::lock_guard<std::mutex> lock(shapers_mutex_);
    
    std::vector<std::string> names;
    for (const auto& pair : shapers_) {
        names.push_back(pair.first);
    }
    return names;
}

TokenBucketShaper::Statistics MultiTokenBucketShaper::get_shaper_statistics(const std::string& name) const {
    std::lock_guard<std::mutex> lock(shapers_mutex_);
    
    auto it = shapers_.find(name);
    if (it != shapers_.end()) {
        return it->second->get_statistics();
    }
    
    return TokenBucketShaper::Statistics();
}

MultiTokenBucketShaper::Statistics MultiTokenBucketShaper::get_total_statistics() const {
    std::lock_guard<std::mutex> lock(shapers_mutex_);
    
    Statistics total;
    for (const auto& pair : shapers_) {
        auto stats = pair.second->get_statistics();
        total.packets_passed += stats.packets_passed;
        total.bytes_passed += stats.bytes_passed;
        total.packets_dropped += stats.packets_dropped;
        total.bytes_dropped += stats.bytes_dropped;
    }
    return total;
}

void MultiTokenBucketShaper::reset_all_statistics() {
    std::lock_guard<std::mutex> lock(shapers_mutex_);
    
    for (const auto& pair : shapers_) {
        pair.second->reset_statistics();
    }
}

} // namespace RouterSim
