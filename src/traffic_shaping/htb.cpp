#include "traffic_shaping.h"
#include <algorithm>
#include <queue>

namespace RouterSim {

HTB::HTB(uint32_t max_classes)
    : max_classes_(max_classes)
    , total_packets_processed_(0)
    , total_bytes_processed_(0)
    , packets_dropped_(0)
    , bytes_dropped_(0)
{
    classes_.resize(max_classes_);
    for (auto& cls : classes_) {
        cls.rate = 1000000; // 1 Mbps default
        cls.ceil = 1000000;
        cls.burst = 1500;
        cls.priority = 1;
        cls.tokens = cls.burst;
        cls.packets = 0;
        cls.bytes = 0;
        cls.last_refill_time = std::chrono::steady_clock::now();
    }
}

HTB::~HTB() {
}

bool HTB::enqueue(uint32_t class_id, const Packet& packet) {
    if (class_id >= max_classes_) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto& cls = classes_[class_id];
    
    // Check if we have enough tokens
    refillTokens();
    updateClassTokens(cls);
    
    if (packet.size <= cls.tokens) {
        // Can enqueue packet
        cls.packet_queue.push(packet);
        cls.tokens -= packet.size;
        cls.packets++;
        cls.bytes += packet.size;
        return true;
    } else {
        // Not enough tokens, drop packet
        packets_dropped_++;
        bytes_dropped_ += packet.size;
        return false;
    }
}

bool HTB::dequeue(Packet& packet) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    refillTokens();
    
    // Find highest priority class with packets and tokens
    int32_t selected_class = -1;
    uint32_t highest_priority = std::numeric_limits<uint32_t>::max();
    
    for (uint32_t i = 0; i < max_classes_; ++i) {
        auto& cls = classes_[i];
        updateClassTokens(cls);
        
        if (!cls.packet_queue.empty() && cls.tokens > 0 && cls.priority < highest_priority) {
            highest_priority = cls.priority;
            selected_class = i;
        }
    }
    
    if (selected_class == -1) {
        return false; // No packets to dequeue
    }
    
    auto& cls = classes_[selected_class];
    packet = cls.packet_queue.front();
    cls.packet_queue.pop();
    
    cls.tokens -= packet.size;
    cls.packets--;
    cls.bytes -= packet.size;
    
    total_packets_processed_++;
    total_bytes_processed_ += packet.size;
    
    return true;
}

void HTB::setClassRate(uint32_t class_id, uint64_t rate) {
    if (class_id >= max_classes_) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    classes_[class_id].rate = rate;
}

void HTB::setClassCeil(uint32_t class_id, uint64_t ceil) {
    if (class_id >= max_classes_) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    classes_[class_id].ceil = ceil;
}

void HTB::setClassBurst(uint32_t class_id, uint64_t burst) {
    if (class_id >= max_classes_) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    classes_[class_id].burst = burst;
    classes_[class_id].tokens = std::min(classes_[class_id].tokens, burst);
}

void HTB::setClassPriority(uint32_t class_id, uint32_t priority) {
    if (class_id >= max_classes_) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    classes_[class_id].priority = priority;
}

HTB::Statistics HTB::getStatistics() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    Statistics stats;
    stats.max_classes = max_classes_;
    stats.total_packets_processed = total_packets_processed_;
    stats.total_bytes_processed = total_bytes_processed_;
    stats.packets_dropped = packets_dropped_;
    stats.bytes_dropped = bytes_dropped_;
    
    // Calculate class statistics
    for (uint32_t i = 0; i < max_classes_; ++i) {
        const auto& cls = classes_[i];
        ClassStatistics class_stats;
        class_stats.class_id = i;
        class_stats.rate = cls.rate;
        class_stats.ceil = cls.ceil;
        class_stats.burst = cls.burst;
        class_stats.priority = cls.priority;
        class_stats.tokens = cls.tokens;
        class_stats.packets = cls.packets;
        class_stats.bytes = cls.bytes;
        stats.class_stats.push_back(class_stats);
    }
    
    return stats;
}

void HTB::reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (auto& cls : classes_) {
        cls.tokens = cls.burst;
        cls.packets = 0;
        cls.bytes = 0;
        cls.last_refill_time = std::chrono::steady_clock::now();
        while (!cls.packet_queue.empty()) {
            cls.packet_queue.pop();
        }
    }
    
    total_packets_processed_ = 0;
    total_bytes_processed_ = 0;
    packets_dropped_ = 0;
    bytes_dropped_ = 0;
}

void HTB::refillTokens() {
    auto now = std::chrono::steady_clock::now();
    
    for (auto& cls : classes_) {
        updateClassTokens(cls);
    }
}

void HTB::updateClassTokens(Class& cls) {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - cls.last_refill_time);
    
    if (elapsed.count() > 0) {
        double elapsed_seconds = elapsed.count() / 1000000.0;
        uint64_t tokens_to_add = static_cast<uint64_t>(cls.rate * elapsed_seconds);
        
        // Add tokens, but don't exceed ceil
        cls.tokens = std::min(cls.ceil, cls.tokens + tokens_to_add);
        cls.last_refill_time = now;
    }
}

} // namespace RouterSim
