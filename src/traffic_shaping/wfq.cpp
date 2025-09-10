#include "traffic_shaping/wfq.h"
#include <iostream>
#include <algorithm>
#include <chrono>
#include <cmath>

namespace RouterSim {

WeightedFairQueue::WeightedFairQueue() : virtual_time_(0) {
}

WeightedFairQueue::~WeightedFairQueue() = default;

bool WeightedFairQueue::initialize(const std::vector<WFQClass>& classes) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    classes_ = classes;
    queues_.clear();
    
    // Initialize queues for each class
    for (const auto& wfq_class : classes_) {
        queues_[wfq_class.class_id] = std::queue<QueueItem>();
    }
    
    return true;
}

bool WeightedFairQueue::enqueue_packet(const PacketInfo& packet, uint8_t class_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Check if class exists
    if (queues_.find(class_id) == queues_.end()) {
        return false;
    }
    
    // Create queue item
    QueueItem item;
    item.packet = packet;
    item.class_id = class_id;
    item.enqueue_time = std::chrono::steady_clock::now();
    item.virtual_finish_time = calculate_virtual_finish_time(packet, class_id);
    
    // Add to queue
    queues_[class_id].push(item);
    
    return true;
}

bool WeightedFairQueue::dequeue_packet(PacketInfo& packet) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    QueueItem item;
    if (select_next_packet(item)) {
        packet = item.packet;
        return true;
    }
    
    return false;
}

bool WeightedFairQueue::is_empty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (const auto& [class_id, queue] : queues_) {
        if (!queue.empty()) {
            return false;
        }
    }
    
    return true;
}

size_t WeightedFairQueue::queue_size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    size_t total_size = 0;
    for (const auto& [class_id, queue] : queues_) {
        total_size += queue.size();
    }
    
    return total_size;
}

size_t WeightedFairQueue::queue_size(uint8_t class_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = queues_.find(class_id);
    if (it != queues_.end()) {
        return it->second.size();
    }
    
    return 0;
}

bool WeightedFairQueue::add_class(const WFQClass& wfq_class) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Check if class already exists
    for (const auto& existing_class : classes_) {
        if (existing_class.class_id == wfq_class.class_id) {
            return false;
        }
    }
    
    classes_.push_back(wfq_class);
    queues_[wfq_class.class_id] = std::queue<QueueItem>();
    
    return true;
}

bool WeightedFairQueue::remove_class(uint8_t class_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Remove from classes vector
    auto it = std::find_if(classes_.begin(), classes_.end(),
                          [class_id](const WFQClass& c) { return c.class_id == class_id; });
    
    if (it != classes_.end()) {
        classes_.erase(it);
    }
    
    // Remove from queues
    queues_.erase(class_id);
    
    return true;
}

bool WeightedFairQueue::update_class(const WFQClass& wfq_class) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Find and update class
    for (auto& existing_class : classes_) {
        if (existing_class.class_id == wfq_class.class_id) {
            existing_class = wfq_class;
            return true;
        }
    }
    
    return false;
}

std::vector<WFQClass> WeightedFairQueue::get_classes() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return classes_;
}

void WeightedFairQueue::set_classifier(std::function<uint8_t(const PacketInfo&)> classifier) {
    std::lock_guard<std::mutex> lock(mutex_);
    classifier_ = classifier;
}

uint8_t WeightedFairQueue::classify_packet(const PacketInfo& packet) const {
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

uint64_t WeightedFairQueue::calculate_virtual_finish_time(const PacketInfo& packet, uint8_t class_id) const {
    // Find class weight
    uint32_t weight = 1;
    for (const auto& wfq_class : classes_) {
        if (wfq_class.class_id == class_id) {
            weight = wfq_class.weight;
            break;
        }
    }
    
    // Calculate virtual finish time
    uint64_t virtual_finish_time = virtual_time_ + (packet.size * 8) / weight;
    
    return virtual_finish_time;
}

bool WeightedFairQueue::select_next_packet(QueueItem& item) {
    // Find the packet with the smallest virtual finish time
    uint64_t min_finish_time = UINT64_MAX;
    uint8_t selected_class = 0;
    bool found = false;
    
    for (const auto& [class_id, queue] : queues_) {
        if (!queue.empty()) {
            const QueueItem& front_item = queue.front();
            if (front_item.virtual_finish_time < min_finish_time) {
                min_finish_time = front_item.virtual_finish_time;
                selected_class = class_id;
                found = true;
            }
        }
    }
    
    if (found) {
        item = queues_[selected_class].front();
        queues_[selected_class].pop();
        virtual_time_ = std::max(virtual_time_, item.virtual_finish_time);
        return true;
    }
    
    return false;
}

} // namespace RouterSim
