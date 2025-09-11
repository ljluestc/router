#include "traffic_shaping/drr.h"
#include <iostream>
#include <algorithm>
#include <chrono>

namespace RouterSim {

DeficitRoundRobin::DeficitRoundRobin() : 
    total_packets_queued_(0), total_packets_dequeued_(0),
    total_bytes_queued_(0), total_bytes_dequeued_(0) {
}

DeficitRoundRobin::~DeficitRoundRobin() = default;

bool DeficitRoundRobin::initialize(const std::vector<DRRClass>& classes) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    classes_ = classes;
    queues_.clear();
    
    // Initialize queues and deficits for each class
    for (const auto& drr_class : classes_) {
        queues_[drr_class.class_id] = std::queue<DRRQueueItem>();
        // Initialize deficit to quantum for immediate service
        for (auto& cls : classes_) {
            if (cls.class_id == drr_class.class_id) {
                cls.deficit = cls.quantum;
                break;
            }
        }
    }
    
    return true;
}

bool DeficitRoundRobin::enqueue_packet(const PacketInfo& packet, uint8_t class_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Check if class exists
    if (queues_.find(class_id) == queues_.end()) {
        return false;
    }
    
    // Create queue item
    DRRQueueItem item;
    item.packet = packet;
    item.class_id = class_id;
    item.enqueue_time = std::chrono::steady_clock::now();
    
    // Add to queue
    queues_[class_id].push(item);
    
    // Update statistics
    total_packets_queued_++;
    total_bytes_queued_ += packet.size;
    class_packets_queued_[class_id]++;
    class_bytes_queued_[class_id] += packet.size;
    class_last_activity_[class_id] = item.enqueue_time;
    
    return true;
}

bool DeficitRoundRobin::dequeue_packet(PacketInfo& packet) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    DRRQueueItem item;
    if (select_next_packet(item)) {
        packet = item.packet;
        
        // Update statistics
        total_packets_dequeued_++;
        total_bytes_dequeued_ += packet.size;
        class_packets_dequeued_[item.class_id]++;
        class_bytes_dequeued_[item.class_id] += packet.size;
        
        return true;
    }
    
    return false;
}

bool DeficitRoundRobin::is_empty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (const auto& [class_id, queue] : queues_) {
        if (!queue.empty()) {
            return false;
        }
    }
    
    return true;
}

size_t DeficitRoundRobin::queue_size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    size_t total_size = 0;
    for (const auto& [class_id, queue] : queues_) {
        total_size += queue.size();
    }
    
    return total_size;
}

size_t DeficitRoundRobin::queue_size(uint8_t class_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = queues_.find(class_id);
    if (it != queues_.end()) {
        return it->second.size();
    }
    
    return 0;
}

bool DeficitRoundRobin::add_class(const DRRClass& drr_class) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Check if class already exists
    for (const auto& existing_class : classes_) {
        if (existing_class.class_id == drr_class.class_id) {
            return false;
        }
    }
    
    classes_.push_back(drr_class);
    queues_[drr_class.class_id] = std::queue<DRRQueueItem>();
    
    return true;
}

bool DeficitRoundRobin::remove_class(uint8_t class_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Remove from classes vector
    auto it = std::find_if(classes_.begin(), classes_.end(),
                          [class_id](const DRRClass& c) { return c.class_id == class_id; });
    
    if (it != classes_.end()) {
        classes_.erase(it);
    }
    
    // Remove from queues
    queues_.erase(class_id);
    
    return true;
}

bool DeficitRoundRobin::update_class(const DRRClass& drr_class) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Find and update class
    for (auto& existing_class : classes_) {
        if (existing_class.class_id == drr_class.class_id) {
            existing_class = drr_class;
            return true;
        }
    }
    
    return false;
}

std::vector<DRRClass> DeficitRoundRobin::get_classes() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return classes_;
}

void DeficitRoundRobin::set_classifier(std::function<uint8_t(const PacketInfo&)> classifier) {
    std::lock_guard<std::mutex> lock(mutex_);
    classifier_ = classifier;
}

uint8_t DeficitRoundRobin::classify_packet(const PacketInfo& packet) const {
    if (classifier_) {
        return classifier_(packet);
    }
    
    // Default classification based on DSCP
    if (packet.dscp >= 48) {
        return 1; // High priority
    } else if (packet.dscp >= 32) {
        return 2; // Medium priority
    } else {
        return 3; // Low priority
    }
}

bool DeficitRoundRobin::select_next_packet(DRRQueueItem& item) {
    // DRR algorithm: serve each class in round-robin fashion
    // A class can send packets as long as its deficit allows
    
    for (auto& drr_class : classes_) {
        if (queues_[drr_class.class_id].empty()) {
            continue;
        }
        
        // Check if we can send a packet from this class
        const DRRQueueItem& front_item = queues_[drr_class.class_id].front();
        uint32_t packet_size = front_item.packet.size;
        
        if (drr_class.deficit >= packet_size) {
            // Can send this packet
            item = queues_[drr_class.class_id].front();
            queues_[drr_class.class_id].pop();
            
            // Update deficit
            drr_class.deficit -= packet_size;
            
            return true;
        } else {
            // Cannot send packet, add quantum to deficit for next round
            drr_class.deficit += drr_class.quantum;
        }
    }
    
    return false;
}

void DeficitRoundRobin::update_deficit(uint8_t class_id, uint32_t packet_size) {
    // Find the class and update its deficit
    for (auto& drr_class : classes_) {
        if (drr_class.class_id == class_id) {
            if (drr_class.deficit >= packet_size) {
                drr_class.deficit -= packet_size;
            } else {
                // Add quantum for next round
                drr_class.deficit += drr_class.quantum;
            }
            break;
        }
    }
}

// DRR Statistics
DRRStatistics DeficitRoundRobin::get_statistics() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    DRRStatistics stats;
    stats.total_packets_queued = total_packets_queued_;
    stats.total_packets_dequeued = total_packets_dequeued_;
    stats.total_bytes_queued = total_bytes_queued_;
    stats.total_bytes_dequeued = total_bytes_dequeued_;
    stats.current_queue_length = queue_size();
    
    for (const auto& [class_id, queue] : queues_) {
        DRRClassStatistics class_stats;
        class_stats.class_id = class_id;
        class_stats.packets_queued = class_packets_queued_[class_id];
        class_stats.packets_dequeued = class_packets_dequeued_[class_id];
        class_stats.bytes_queued = class_bytes_queued_[class_id];
        class_stats.bytes_dequeued = class_bytes_dequeued_[class_id];
        class_stats.current_queue_length = queue.size();
        class_stats.last_activity = class_last_activity_[class_id];
        
        // Find current deficit for this class
        for (const auto& drr_class : classes_) {
            if (drr_class.class_id == class_id) {
                class_stats.current_deficit = drr_class.deficit;
                break;
            }
        }
        
        stats.class_statistics[class_id] = class_stats;
    }
    
    return stats;
}

void DeficitRoundRobin::reset_statistics() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Reset global statistics
    total_packets_queued_ = 0;
    total_packets_dequeued_ = 0;
    total_bytes_queued_ = 0;
    total_bytes_dequeued_ = 0;
    
    // Reset class statistics
    class_packets_queued_.clear();
    class_packets_dequeued_.clear();
    class_bytes_queued_.clear();
    class_bytes_dequeued_.clear();
    class_last_activity_.clear();
    
    // Reset deficits to quantum
    for (auto& drr_class : classes_) {
        drr_class.deficit = drr_class.quantum;
    }
}

} // namespace RouterSim
