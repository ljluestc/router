#include "traffic_shaping.h"
#include <algorithm>
#include <chrono>

namespace RouterSim {

// TokenBucket Implementation
TokenBucket::TokenBucket(uint64_t capacity, uint64_t refill_rate)
    : capacity_(capacity), refill_rate_(refill_rate), tokens_(capacity) {
    last_refill_ = std::chrono::steady_clock::now();
}

bool TokenBucket::consume_tokens(uint64_t tokens) {
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
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_refill_);
    
    uint64_t tokens_to_add = (elapsed.count() * refill_rate_) / 1000;
    if (tokens_to_add > 0) {
        tokens_ = std::min(capacity_, tokens_ + tokens_to_add);
        last_refill_ = now;
    }
}

uint64_t TokenBucket::get_available_tokens() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return tokens_;
}

void TokenBucket::set_capacity(uint64_t capacity) {
    std::lock_guard<std::mutex> lock(mutex_);
    capacity_ = capacity;
    tokens_ = std::min(tokens_, capacity_);
}

void TokenBucket::set_refill_rate(uint64_t rate) {
    std::lock_guard<std::mutex> lock(mutex_);
    refill_rate_ = rate;
}

// WFQScheduler Implementation
WFQScheduler::WFQScheduler() : current_time_(0) {}

void WFQScheduler::add_class(uint32_t class_id, uint32_t weight) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    WFQClass wfq_class;
    wfq_class.weight = weight;
    wfq_class.quantum = weight * 100; // Quantum = weight * base_quantum
    wfq_class.deficit_counter = 0;
    wfq_class.last_served_time = current_time_;
    
    classes_[class_id] = wfq_class;
}

void WFQScheduler::remove_class(uint32_t class_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    classes_.erase(class_id);
}

void WFQScheduler::enqueue_packet(uint32_t class_id, const Packet& packet) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = classes_.find(class_id);
    if (it != classes_.end()) {
        it->second.queue.push(packet);
    }
}

bool WFQScheduler::dequeue_packet(Packet& packet) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (classes_.empty()) {
        return false;
    }
    
    update_deficit_counters();
    
    uint32_t selected_class = select_next_class();
    if (selected_class == 0) {
        return false;
    }
    
    auto& wfq_class = classes_[selected_class];
    if (wfq_class.queue.empty()) {
        return false;
    }
    
    packet = wfq_class.queue.front();
    wfq_class.queue.pop();
    
    // Update deficit counter
    wfq_class.deficit_counter -= packet.size;
    wfq_class.last_served_time = current_time_;
    
    return true;
}

bool WFQScheduler::is_empty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (const auto& pair : classes_) {
        if (!pair.second.queue.empty()) {
            return false;
        }
    }
    
    return true;
}

size_t WFQScheduler::get_queue_size(uint32_t class_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = classes_.find(class_id);
    if (it != classes_.end()) {
        return it->second.queue.size();
    }
    
    return 0;
}

void WFQScheduler::set_class_weight(uint32_t class_id, uint32_t weight) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = classes_.find(class_id);
    if (it != classes_.end()) {
        it->second.weight = weight;
        it->second.quantum = weight * 100;
    }
}

void WFQScheduler::update_deficit_counters() {
    current_time_++;
    
    for (auto& pair : classes_) {
        auto& wfq_class = pair.second;
        
        // Add quantum to deficit counter
        wfq_class.deficit_counter += wfq_class.quantum;
    }
}

uint32_t WFQScheduler::select_next_class() {
    uint32_t selected_class = 0;
    uint64_t min_deficit = UINT64_MAX;
    
    for (const auto& pair : classes_) {
        const auto& wfq_class = pair.second;
        
        if (!wfq_class.queue.empty() && wfq_class.deficit_counter >= wfq_class.queue.front().size) {
            if (wfq_class.deficit_counter < min_deficit) {
                min_deficit = wfq_class.deficit_counter;
                selected_class = pair.first;
            }
        }
    }
    
    return selected_class;
}

// TrafficShaper Implementation
TrafficShaper::TrafficShaper() : stats_{} {
    token_bucket_ = std::make_unique<TokenBucket>(1000000, 100000); // 1MB capacity, 100KB/s refill
    wfq_scheduler_ = std::make_unique<WFQScheduler>();
}

void TrafficShaper::set_token_bucket(uint64_t capacity, uint64_t refill_rate) {
    token_bucket_ = std::make_unique<TokenBucket>(capacity, refill_rate);
}

void TrafficShaper::add_wfq_class(uint32_t class_id, uint32_t weight) {
    wfq_scheduler_->add_class(class_id, weight);
}

void TrafficShaper::remove_wfq_class(uint32_t class_id) {
    wfq_scheduler_->remove_class(class_id);
}

bool TrafficShaper::shape_packet(const Packet& packet, uint32_t class_id) {
    // First, check token bucket
    if (!token_bucket_->consume_tokens(packet.size)) {
        std::lock_guard<std::mutex> lock(stats_mutex_);
        stats_.packets_dropped++;
        stats_.bytes_dropped += packet.size;
        stats_.token_bucket_drops++;
        return false;
    }
    
    // Then, enqueue in WFQ scheduler
    wfq_scheduler_->enqueue_packet(class_id, packet);
    
    std::lock_guard<std::mutex> lock(stats_mutex_);
    stats_.packets_shaped++;
    stats_.bytes_shaped += packet.size;
    
    return true;
}

bool TrafficShaper::get_shaped_packet(Packet& packet) {
    bool result = wfq_scheduler_->dequeue_packet(packet);
    
    if (!result) {
        std::lock_guard<std::mutex> lock(stats_mutex_);
        stats_.wfq_drops++;
    }
    
    return result;
}

TrafficShaper::ShapingStats TrafficShaper::get_statistics() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    return stats_;
}

} // namespace RouterSim
