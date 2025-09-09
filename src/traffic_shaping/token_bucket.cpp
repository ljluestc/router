#include "traffic_shaping.h"
#include <iostream>
#include <algorithm>
#include <chrono>

namespace RouterSim {

// Token Bucket Implementation
TokenBucket::TokenBucket() 
    : running_(false)
    , tokens_(0)
    , last_refill_time_(0) {
    
    stats_.packets_processed = 0;
    stats_.packets_dropped = 0;
    stats_.packets_delayed = 0;
    stats_.bytes_processed = 0;
    stats_.bytes_dropped = 0;
    stats_.bytes_delayed = 0;
}

TokenBucket::~TokenBucket() {
    stop();
}

bool TokenBucket::initialize(const ShapingConfig& config) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    
    config_ = config;
    tokens_ = config.burst_size;
    last_refill_time_ = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    
    // Clear any existing packets
    while (!packet_queue_.empty()) {
        packet_queue_.pop();
    }
    
    std::cout << "Token bucket initialized with rate: " << config.rate_bps 
              << " bps, burst size: " << config.burst_size << " bytes" << std::endl;
    
    return true;
}

void TokenBucket::start() {
    if (running_) {
        return;
    }
    
    std::cout << "Starting token bucket..." << std::endl;
    running_ = true;
    
    // Start token refill thread
    refill_thread_ = std::thread(&TokenBucket::token_refill_loop, this);
}

void TokenBucket::stop() {
    if (!running_) {
        return;
    }
    
    std::cout << "Stopping token bucket..." << std::endl;
    running_ = false;
    
    if (refill_thread_.joinable()) {
        refill_thread_.join();
    }
}

bool TokenBucket::is_running() const {
    return running_;
}

void TokenBucket::set_config(const ShapingConfig& config) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    config_ = config;
    tokens_ = config.burst_size;
}

ShapingConfig TokenBucket::get_config() const {
    return config_;
}

bool TokenBucket::process_packet(const Packet& packet) {
    if (!running_) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(queue_mutex_);
    
    // Check if we have enough tokens
    if (consume_tokens(packet.size)) {
        // Process packet immediately
        {
            std::lock_guard<std::mutex> stats_lock(stats_mutex_);
            stats_.packets_processed++;
            stats_.bytes_processed += packet.size;
        }
        return true;
    } else {
        // Check if queue has space
        if (packet_queue_.size() >= config_.queue_size) {
            // Drop packet
            {
                std::lock_guard<std::mutex> stats_lock(stats_mutex_);
                stats_.packets_dropped++;
                stats_.bytes_dropped += packet.size;
            }
            return false;
        } else {
            // Queue packet
            packet_queue_.push(packet);
            {
                std::lock_guard<std::mutex> stats_lock(stats_mutex_);
                stats_.packets_delayed++;
                stats_.bytes_delayed += packet.size;
            }
            return true;
        }
    }
}

bool TokenBucket::process_packet(const std::vector<uint8_t>& data, const std::string& interface, QoSClass qos_class) {
    Packet packet(data, interface, qos_class);
    return process_packet(packet);
}

size_t TokenBucket::get_queue_size() const {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    return packet_queue_.size();
}

size_t TokenBucket::get_queue_size(QoSClass qos_class) const {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    
    size_t count = 0;
    std::queue<Packet> temp_queue = packet_queue_;
    
    while (!temp_queue.empty()) {
        if (temp_queue.front().qos_class == qos_class) {
            count++;
        }
        temp_queue.pop();
    }
    
    return count;
}

void TokenBucket::clear_queue() {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    while (!packet_queue_.empty()) {
        packet_queue_.pop();
    }
}

void TokenBucket::clear_queue(QoSClass qos_class) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    
    std::queue<Packet> temp_queue;
    while (!packet_queue_.empty()) {
        Packet packet = packet_queue_.front();
        packet_queue_.pop();
        
        if (packet.qos_class != qos_class) {
            temp_queue.push(packet);
        }
    }
    
    packet_queue_ = temp_queue;
}

ShapingStats TokenBucket::get_statistics() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    return stats_;
}

ShapingStats TokenBucket::get_statistics(QoSClass qos_class) const {
    // For token bucket, we don't separate stats by QoS class
    return get_statistics();
}

void TokenBucket::reset_statistics() {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    stats_.packets_processed = 0;
    stats_.packets_dropped = 0;
    stats_.packets_delayed = 0;
    stats_.bytes_processed = 0;
    stats_.bytes_dropped = 0;
    stats_.bytes_delayed = 0;
    stats_.start_time = std::chrono::steady_clock::now();
}

void TokenBucket::set_rate(uint64_t rate_bps) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    config_.rate_bps = rate_bps;
}

uint64_t TokenBucket::get_rate() const {
    return config_.rate_bps;
}

void TokenBucket::set_burst_size(uint64_t burst_size) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    config_.burst_size = burst_size;
    tokens_ = burst_size;
}

uint64_t TokenBucket::get_burst_size() const {
    return config_.burst_size;
}

void TokenBucket::token_refill_loop() {
    std::cout << "Token refill loop started" << std::endl;
    
    while (running_) {
        add_tokens();
        
        // Process queued packets
        std::queue<Packet> processed_packets;
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            
            while (!packet_queue_.empty()) {
                Packet packet = packet_queue_.front();
                packet_queue_.pop();
                
                if (consume_tokens(packet.size)) {
                    // Process packet
                    {
                        std::lock_guard<std::mutex> stats_lock(stats_mutex_);
                        stats_.packets_processed++;
                        stats_.bytes_processed += packet.size;
                        stats_.packets_delayed--;
                        stats_.bytes_delayed -= packet.size;
                    }
                } else {
                    // Put packet back in queue
                    processed_packets.push(packet);
                }
            }
            
            // Put unprocessed packets back
            while (!processed_packets.empty()) {
                packet_queue_.push(processed_packets.front());
                processed_packets.pop();
            }
        }
        
        // Sleep for a short time to avoid busy waiting
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
    
    std::cout << "Token refill loop stopped" << std::endl;
}

bool TokenBucket::consume_tokens(uint32_t packet_size) {
    if (tokens_ >= packet_size) {
        tokens_ -= packet_size;
        return true;
    }
    return false;
}

void TokenBucket::add_tokens() {
    uint64_t current_time = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    
    uint64_t time_diff = current_time - last_refill_time_;
    last_refill_time_ = current_time;
    
    // Calculate tokens to add based on time elapsed
    // Rate is in bits per second, convert to bytes per microsecond
    double rate_bytes_per_us = static_cast<double>(config_.rate_bps) / (8.0 * 1000000.0);
    uint64_t tokens_to_add = static_cast<uint64_t>(rate_bytes_per_us * time_diff);
    
    // Add tokens, but don't exceed burst size
    tokens_ = std::min(tokens_ + tokens_to_add, config_.burst_size);
}

} // namespace RouterSim