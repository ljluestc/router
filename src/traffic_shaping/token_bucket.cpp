#include "traffic_shaping/token_bucket.h"
#include <iostream>
#include <algorithm>
#include <chrono>

namespace RouterSim {

TokenBucket::TokenBucket(const TokenBucketConfig& config) 
    : config_(config), tokens_(config.capacity), last_update_(std::chrono::steady_clock::now()) {
}

TokenBucket::~TokenBucket() = default;

bool TokenBucket::consume_tokens(uint64_t bytes) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Add tokens based on elapsed time
    add_tokens();
    
    if (tokens_ >= bytes) {
        tokens_ -= bytes;
        return true;
    }
    
    return false;
}

void TokenBucket::add_tokens() {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_update_);
    
    if (elapsed.count() > 0) {
        uint64_t tokens_to_add = (config_.rate * elapsed.count()) / 1000;
        tokens_ = std::min(config_.capacity, tokens_ + tokens_to_add);
        last_update_ = now;
    }
}

uint64_t TokenBucket::get_available_tokens() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Add tokens based on elapsed time
    const_cast<TokenBucket*>(this)->add_tokens();
    
    return tokens_;
}

bool TokenBucket::is_available(uint64_t bytes) const {
    return get_available_tokens() >= bytes;
}

void TokenBucket::update_config(const TokenBucketConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    config_ = config;
    tokens_ = config.capacity;
    last_update_ = std::chrono::steady_clock::now();
}

TokenBucketConfig TokenBucket::get_config() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return config_;
}

} // namespace RouterSim