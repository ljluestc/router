#pragma once

#include <mutex>
#include <chrono>

namespace RouterSim {

// Token bucket configuration
struct TokenBucketConfig {
    uint64_t capacity;      // Maximum number of tokens
    uint64_t rate;          // Tokens per second
    uint64_t burst_size;    // Maximum burst size
    bool allow_burst;       // Whether to allow bursting
    
    TokenBucketConfig() : capacity(1000000), rate(100000), burst_size(500000), allow_burst(true) {}
};

// Token bucket implementation for rate limiting
class TokenBucket {
public:
    explicit TokenBucket(const TokenBucketConfig& config);
    ~TokenBucket();
    
    // Token operations
    bool consume_tokens(uint64_t bytes);
    uint64_t get_available_tokens() const;
    bool is_available(uint64_t bytes) const;
    
    // Configuration
    void update_config(const TokenBucketConfig& config);
    TokenBucketConfig get_config() const;
    
private:
    // Add tokens based on elapsed time
    void add_tokens();
    
    // Configuration
    TokenBucketConfig config_;
    
    // State
    uint64_t tokens_;
    std::chrono::steady_clock::time_point last_update_;
    
    // Thread safety
    mutable std::mutex mutex_;
};

} // namespace RouterSim
