#include "traffic_shaping.h"
#include <algorithm>
#include <chrono>

namespace RouterSim {

TokenBucket::TokenBucket(uint32_t rate_bps, uint32_t burst_size)
    : rate_bps_(rate_bps)
    , burst_size_(burst_size)
    , tokens_(burst_size)
    , last_refill_(std::chrono::steady_clock::now()) {
}

bool TokenBucket::consume_tokens(uint32_t bytes) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Refill tokens based on elapsed time
    refill_tokens();
    
    if (tokens_ >= bytes) {
        tokens_ -= bytes;
        return true;
    }
    
    return false;
}

void TokenBucket::refill_tokens() {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - last_refill_);
    
    // Calculate tokens to add based on elapsed time
    uint32_t tokens_to_add = (rate_bps_ * elapsed.count()) / 1000000; // Convert microseconds to seconds
    
    if (tokens_to_add > 0) {
        tokens_ = std::min(burst_size_, tokens_ + tokens_to_add);
        last_refill_ = now;
    }
}

uint32_t TokenBucket::get_available_tokens() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Refill tokens for accurate count
    const_cast<TokenBucket*>(this)->refill_tokens();
    
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

void TokenBucket::reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    tokens_ = burst_size_;
    last_refill_ = std::chrono::steady_clock::now();
}

} // namespace RouterSim
