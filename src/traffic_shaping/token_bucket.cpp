#include "traffic_shaping.h"
#include <iostream>
#include <algorithm>
#include <chrono>

namespace router_sim {

TokenBucketShaper::TokenBucketShaper() 
    : running_(false), refill_running_(false) {
}

bool TokenBucketShaper::initialize(const std::map<std::string, std::string>& config) {
    // Parse configuration
    auto it = config.find("rate_bps");
    if (it != config.end()) {
        config_.rate_bps = std::stoull(it->second);
    }
    
    it = config.find("burst_size");
    if (it != config.end()) {
        config_.burst_size = std::stoull(it->second);
    }
    
    // Set defaults if not specified
    if (config_.rate_bps == 0) {
        config_.rate_bps = 1000000; // 1 Mbps default
    }
    if (config_.burst_size == 0) {
        config_.burst_size = 10000; // 10 KB default
    }
    
    config_.current_tokens = config_.burst_size;
    config_.last_update = std::chrono::steady_clock::now();
    
    return true;
}

bool TokenBucketShaper::start() {
    if (running_) {
        return true;
    }
    
    running_ = true;
    refill_running_ = true;
    
    // Start token refill thread
    refill_thread_ = std::thread(&TokenBucketShaper::token_refill_loop, this);
    
    return true;
}

bool TokenBucketShaper::stop() {
    if (!running_) {
        return true;
    }
    
    running_ = false;
    refill_running_ = false;
    
    if (refill_thread_.joinable()) {
        refill_thread_.join();
    }
    
    return true;
}

bool TokenBucketShaper::is_running() const {
    return running_;
}

bool TokenBucketShaper::enqueue_packet(const PacketInfo& packet) {
    if (!running_) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(queue_mutex_);
    
    // Check if queue is full (simple size limit for now)
    const size_t MAX_QUEUE_SIZE = 1000;
    if (packet_queue_.size() >= MAX_QUEUE_SIZE) {
        if (queue_full_callback_) {
            queue_full_callback_();
        }
        return false;
    }
    
    packet_queue_.push(packet);
    return true;
}

bool TokenBucketShaper::dequeue_packet(PacketInfo& packet, int timeout_ms) {
    if (!running_) {
        return false;
    }
    
    auto start_time = std::chrono::steady_clock::now();
    
    while (running_) {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        
        if (!packet_queue_.empty()) {
            PacketInfo queued_packet = packet_queue_.front();
            
            // Check if we have enough tokens
            if (consume_tokens(queued_packet.size)) {
                packet = queued_packet;
                packet_queue_.pop();
                
                // Update statistics
                {
                    std::lock_guard<std::mutex> stats_lock(stats_mutex_);
                    statistics_.packets_processed++;
                    statistics_.bytes_processed += packet.size;
                }
                
                return true;
            }
        }
        
        // Check timeout
        auto current_time = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time);
        if (elapsed.count() >= timeout_ms) {
            break;
        }
        
        // Small delay before retry
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    
    return false;
}

size_t TokenBucketShaper::queue_size() const {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    return packet_queue_.size();
}

bool TokenBucketShaper::is_queue_full() const {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    const size_t MAX_QUEUE_SIZE = 1000;
    return packet_queue_.size() >= MAX_QUEUE_SIZE;
}

bool TokenBucketShaper::add_traffic_class(const TrafficClass& traffic_class) {
    std::lock_guard<std::mutex> lock(classes_mutex_);
    traffic_classes_[traffic_class.class_id] = traffic_class;
    return true;
}

bool TokenBucketShaper::remove_traffic_class(int class_id) {
    std::lock_guard<std::mutex> lock(classes_mutex_);
    auto it = traffic_classes_.find(class_id);
    if (it != traffic_classes_.end()) {
        traffic_classes_.erase(it);
        return true;
    }
    return false;
}

bool TokenBucketShaper::update_traffic_class(const TrafficClass& traffic_class) {
    std::lock_guard<std::mutex> lock(classes_mutex_);
    auto it = traffic_classes_.find(traffic_class.class_id);
    if (it != traffic_classes_.end()) {
        it->second = traffic_class;
        return true;
    }
    return false;
}

std::vector<TrafficClass> TokenBucketShaper::get_traffic_classes() const {
    std::lock_guard<std::mutex> lock(classes_mutex_);
    std::vector<TrafficClass> classes;
    for (const auto& pair : traffic_classes_) {
        classes.push_back(pair.second);
    }
    return classes;
}

ShapingStatistics TokenBucketShaper::get_statistics() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    return statistics_;
}

ShapingStatistics TokenBucketShaper::get_class_statistics(int class_id) const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    auto it = class_statistics_.find(class_id);
    if (it != class_statistics_.end()) {
        return it->second;
    }
    return ShapingStatistics{};
}

void TokenBucketShaper::set_packet_dropped_callback(std::function<void(const PacketInfo&)> callback) {
    packet_dropped_callback_ = callback;
}

void TokenBucketShaper::set_queue_full_callback(std::function<void()> callback) {
    queue_full_callback_ = callback;
}

bool TokenBucketShaper::add_tokens(uint64_t tokens) {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    config_.current_tokens = std::min(config_.current_tokens + tokens, config_.burst_size);
    statistics_.tokens_generated += tokens;
    return true;
}

bool TokenBucketShaper::consume_tokens(uint64_t tokens) {
    if (config_.current_tokens >= tokens) {
        config_.current_tokens -= tokens;
        {
            std::lock_guard<std::mutex> lock(stats_mutex_);
            statistics_.tokens_consumed += tokens;
        }
        return true;
    }
    return false;
}

uint64_t TokenBucketShaper::get_available_tokens() const {
    return config_.current_tokens;
}

bool TokenBucketShaper::refill_tokens() {
    auto current_time = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
        current_time - config_.last_update);
    
    // Calculate tokens to add based on elapsed time and rate
    double elapsed_seconds = elapsed.count() / 1000000.0;
    uint64_t tokens_to_add = static_cast<uint64_t>(config_.rate_bps * elapsed_seconds / 8.0);
    
    if (tokens_to_add > 0) {
        add_tokens(tokens_to_add);
        config_.last_update = current_time;
        return true;
    }
    
    return false;
}

void TokenBucketShaper::token_refill_loop() {
    while (refill_running_) {
        refill_tokens();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

bool TokenBucketShaper::process_packet(const PacketInfo& packet) {
    // This method can be used for additional packet processing logic
    return true;
}

} // namespace router_sim