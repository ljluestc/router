#pragma once

#include <chrono>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <queue>
#include <memory>

namespace router_sim {
namespace traffic_shaping {

struct TokenBucketConfig {
    uint64_t rate_bps;        // Tokens per second
    uint64_t burst_bytes;     // Maximum burst size in bytes
    uint64_t bucket_size;     // Current bucket size in bytes
    std::chrono::milliseconds refill_interval{10}; // Refill interval
    bool enabled{true};
};

class TokenBucket {
public:
    explicit TokenBucket(const TokenBucketConfig& config);
    ~TokenBucket();
    
    // Configuration
    void set_rate(uint64_t rate_bps);
    void set_burst_size(uint64_t burst_bytes);
    void set_enabled(bool enabled);
    
    // Token operations
    bool consume_tokens(uint64_t bytes);
    bool try_consume_tokens(uint64_t bytes);
    uint64_t get_available_tokens() const;
    uint64_t get_bucket_size() const;
    
    // Statistics
    uint64_t get_total_tokens_consumed() const;
    uint64_t get_total_packets_processed() const;
    uint64_t get_total_packets_dropped() const;
    double get_drop_rate() const;
    
    // Control
    void start();
    void stop();
    bool is_running() const;
    
private:
    TokenBucketConfig config_;
    std::atomic<uint64_t> current_tokens_;
    std::atomic<uint64_t> total_tokens_consumed_;
    std::atomic<uint64_t> total_packets_processed_;
    std::atomic<uint64_t> total_packets_dropped_;
    
    std::atomic<bool> running_;
    std::thread refill_thread_;
    std::mutex bucket_mutex_;
    std::condition_variable bucket_cv_;
    
    std::chrono::steady_clock::time_point last_refill_;
    
    void refill_loop();
    void refill_tokens();
    uint64_t calculate_tokens_to_add() const;
};

class TokenBucketShaper {
public:
    TokenBucketShaper();
    ~TokenBucketShaper();
    
    // Configuration
    bool add_bucket(const std::string& name, const TokenBucketConfig& config);
    bool remove_bucket(const std::string& name);
    bool update_bucket_config(const std::string& name, const TokenBucketConfig& config);
    
    // Shaping operations
    bool shape_packet(const std::string& bucket_name, uint64_t packet_size);
    bool try_shape_packet(const std::string& bucket_name, uint64_t packet_size);
    
    // Statistics
    std::map<std::string, uint64_t> get_bucket_statistics() const;
    uint64_t get_total_packets_processed() const;
    uint64_t get_total_packets_dropped() const;
    
    // Control
    void start_all();
    void stop_all();
    void start_bucket(const std::string& name);
    void stop_bucket(const std::string& name);
    
private:
    std::map<std::string, std::unique_ptr<TokenBucket>> buckets_;
    std::mutex buckets_mutex_;
    
    std::atomic<uint64_t> total_packets_processed_;
    std::atomic<uint64_t> total_packets_dropped_;
};

} // namespace traffic_shaping
} // namespace router_sim
