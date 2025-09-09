#include "traffic_shaping.h"
#include <iostream>
#include <chrono>
#include <algorithm>

namespace RouterSim {

TokenBucket::TokenBucket(uint64_t rate, uint64_t burst_size)
    : rate_(rate), burst_size_(burst_size), tokens_(burst_size), 
      last_update_(std::chrono::steady_clock::now()) {
    std::cout << "TokenBucket initialized: rate=" << rate_ 
              << " bps, burst=" << burst_size_ << " bytes\n";
}

TokenBucket::~TokenBucket() = default;

bool TokenBucket::consume(uint64_t bytes) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - last_update_);
    
    // Add tokens based on elapsed time
    uint64_t tokens_to_add = (rate_ * elapsed.count()) / 1000000; // Convert to bytes
    tokens_ = std::min(tokens_ + tokens_to_add, burst_size_);
    last_update_ = now;
    
    // Check if we have enough tokens
    if (tokens_ >= bytes) {
        tokens_ -= bytes;
        return true;
    }
    
    return false;
}

void TokenBucket::set_rate(uint64_t rate) {
    std::lock_guard<std::mutex> lock(mutex_);
    rate_ = rate;
    std::cout << "TokenBucket rate updated to " << rate_ << " bps\n";
}

void TokenBucket::set_burst_size(uint64_t burst_size) {
    std::lock_guard<std::mutex> lock(mutex_);
    burst_size_ = burst_size;
    tokens_ = std::min(tokens_, burst_size_);
    std::cout << "TokenBucket burst size updated to " << burst_size_ << " bytes\n";
}

uint64_t TokenBucket::get_tokens() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return tokens_;
}

uint64_t TokenBucket::get_rate() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return rate_;
}

uint64_t TokenBucket::get_burst_size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return burst_size_;
}

void TokenBucket::reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    tokens_ = burst_size_;
    last_update_ = std::chrono::steady_clock::now();
}

// WFQ (Weighted Fair Queuing) implementation
WFQScheduler::WFQScheduler(const std::vector<uint32_t>& weights)
    : weights_(weights), queues_(weights.size()), 
      current_round_(0), last_update_(std::chrono::steady_clock::now()) {
    
    std::cout << "WFQScheduler initialized with " << weights.size() << " queues\n";
    for (size_t i = 0; i < weights.size(); ++i) {
        std::cout << "  Queue " << i << ": weight " << weights[i] << "\n";
    }
}

WFQScheduler::~WFQScheduler() = default;

bool WFQScheduler::enqueue(uint32_t queue_id, const Packet& packet) {
    if (queue_id >= queues_.size()) {
        std::cerr << "Invalid queue ID: " << queue_id << "\n";
        return false;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    QueuedPacket qp;
    qp.packet = packet;
    qp.enqueue_time = std::chrono::steady_clock::now();
    qp.finish_time = calculate_finish_time(queue_id, packet.size);
    
    queues_[queue_id].push(qp);
    
    std::cout << "Enqueued packet of " << packet.size << " bytes to queue " << queue_id << "\n";
    return true;
}

bool WFQScheduler::dequeue(Packet& packet) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Find the queue with the minimum finish time
    int selected_queue = -1;
    uint64_t min_finish_time = UINT64_MAX;
    
    for (size_t i = 0; i < queues_.size(); ++i) {
        if (!queues_[i].empty()) {
            uint64_t finish_time = queues_[i].front().finish_time;
            if (finish_time < min_finish_time) {
                min_finish_time = finish_time;
                selected_queue = i;
            }
        }
    }
    
    if (selected_queue == -1) {
        return false; // No packets available
    }
    
    packet = queues_[selected_queue].front().packet;
    queues_[selected_queue].pop();
    
    std::cout << "Dequeued packet of " << packet.size << " bytes from queue " << selected_queue << "\n";
    return true;
}

bool WFQScheduler::is_empty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (const auto& queue : queues_) {
        if (!queue.empty()) {
            return false;
        }
    }
    return true;
}

size_t WFQScheduler::queue_size(uint32_t queue_id) const {
    if (queue_id >= queues_.size()) {
        return 0;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    return queues_[queue_id].size();
}

size_t WFQScheduler::total_packets() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    size_t total = 0;
    for (const auto& queue : queues_) {
        total += queue.size();
    }
    return total;
}

void WFQScheduler::set_weight(uint32_t queue_id, uint32_t weight) {
    if (queue_id >= weights_.size()) {
        std::cerr << "Invalid queue ID: " << queue_id << "\n";
        return;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    weights_[queue_id] = weight;
    std::cout << "Queue " << queue_id << " weight updated to " << weight << "\n";
}

uint32_t WFQScheduler::get_weight(uint32_t queue_id) const {
    if (queue_id >= weights_.size()) {
        return 0;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    return weights_[queue_id];
}

uint64_t WFQScheduler::calculate_finish_time(uint32_t queue_id, uint64_t packet_size) {
    if (queue_id >= weights_.size() || weights_[queue_id] == 0) {
        return current_round_ + packet_size;
    }
    
    // WFQ finish time calculation
    uint64_t finish_time = current_round_ + (packet_size * 8) / weights_[queue_id];
    current_round_ = finish_time;
    
    return finish_time;
}

// Traffic Shaper implementation
TrafficShaper::TrafficShaper(uint64_t rate, uint64_t burst_size, 
                           const std::vector<uint32_t>& wfq_weights)
    : token_bucket_(rate, burst_size), wfq_scheduler_(wfq_weights),
      enabled_(true), total_packets_processed_(0), total_bytes_processed_(0) {
    
    std::cout << "TrafficShaper initialized\n";
}

TrafficShaper::~TrafficShaper() = default;

bool TrafficShaper::shape_packet(const Packet& packet) {
    if (!enabled_) {
        return true; // Pass through without shaping
    }
    
    // Add packet to WFQ scheduler
    if (!wfq_scheduler_.enqueue(packet.priority % wfq_scheduler_.get_num_queues(), packet)) {
        std::cerr << "Failed to enqueue packet\n";
        return false;
    }
    
    // Try to dequeue and send packets
    Packet dequeued_packet;
    while (wfq_scheduler_.dequeue(dequeued_packet)) {
        if (token_bucket_.consume(dequeued_packet.size)) {
            // Packet can be sent
            total_packets_processed_++;
            total_bytes_processed_ += dequeued_packet.size;
            
            std::cout << "Shaped packet: " << dequeued_packet.size << " bytes\n";
            return true;
        } else {
            // Not enough tokens, re-enqueue packet
            wfq_scheduler_.enqueue(dequeued_packet.priority % wfq_scheduler_.get_num_queues(), 
                                 dequeued_packet);
            break;
        }
    }
    
    return false;
}

void TrafficShaper::set_rate(uint64_t rate) {
    token_bucket_.set_rate(rate);
    std::cout << "TrafficShaper rate updated to " << rate << " bps\n";
}

void TrafficShaper::set_burst_size(uint64_t burst_size) {
    token_bucket_.set_burst_size(burst_size);
    std::cout << "TrafficShaper burst size updated to " << burst_size << " bytes\n";
}

void TrafficShaper::set_wfq_weight(uint32_t queue_id, uint32_t weight) {
    wfq_scheduler_.set_weight(queue_id, weight);
    std::cout << "TrafficShaper WFQ weight for queue " << queue_id 
              << " updated to " << weight << "\n";
}

void TrafficShaper::enable() {
    enabled_ = true;
    std::cout << "TrafficShaper enabled\n";
}

void TrafficShaper::disable() {
    enabled_ = false;
    std::cout << "TrafficShaper disabled\n";
}

bool TrafficShaper::is_enabled() const {
    return enabled_;
}

TrafficShaper::Statistics TrafficShaper::get_statistics() const {
    Statistics stats;
    stats.total_packets_processed = total_packets_processed_;
    stats.total_bytes_processed = total_bytes_processed_;
    stats.current_tokens = token_bucket_.get_tokens();
    stats.rate = token_bucket_.get_rate();
    stats.burst_size = token_bucket_.get_burst_size();
    stats.total_queued_packets = wfq_scheduler_.total_packets();
    
    return stats;
}

void TrafficShaper::reset_statistics() {
    total_packets_processed_ = 0;
    total_bytes_processed_ = 0;
    token_bucket_.reset();
    std::cout << "TrafficShaper statistics reset\n";
}

} // namespace RouterSim