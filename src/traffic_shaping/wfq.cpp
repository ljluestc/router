#include "traffic_shaping.h"
#include <algorithm>
#include <climits>

namespace RouterSim {

WFQ::WFQ(uint32_t num_queues, uint32_t weight_base)
    : num_queues_(num_queues)
    , weight_base_(weight_base) {
    queues_.resize(num_queues_);
    
    // Initialize queue weights and priorities
    for (uint32_t i = 0; i < num_queues_; ++i) {
        queues_[i].weight = weight_base_;
        queues_[i].priority = i;
        queues_[i].virtual_time = 0;
    }
}

bool WFQ::enqueue_packet(const Packet& packet, uint32_t queue_id) {
    if (queue_id >= num_queues_) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    queues_[queue_id].packets.push(packet);
    return true;
}

bool WFQ::dequeue_packet(Packet& packet) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (is_empty()) {
        return false;
    }
    
    uint32_t selected_queue = select_next_queue();
    if (selected_queue >= num_queues_ || queues_[selected_queue].packets.empty()) {
        return false;
    }
    
    packet = queues_[selected_queue].packets.front();
    queues_[selected_queue].packets.pop();
    
    // Update virtual time for the selected queue
    queues_[selected_queue].virtual_time = calculate_virtual_time(selected_queue, packet.size);
    
    return true;
}

void WFQ::set_queue_weight(uint32_t queue_id, uint32_t weight) {
    if (queue_id >= num_queues_) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    queues_[queue_id].weight = weight;
}

void WFQ::set_queue_priority(uint32_t queue_id, uint32_t priority) {
    if (queue_id >= num_queues_) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    queues_[queue_id].priority = priority;
}

uint32_t WFQ::get_queue_size(uint32_t queue_id) const {
    if (queue_id >= num_queues_) {
        return 0;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    return static_cast<uint32_t>(queues_[queue_id].packets.size());
}

uint32_t WFQ::get_total_queue_size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    uint32_t total_size = 0;
    for (const auto& queue : queues_) {
        total_size += static_cast<uint32_t>(queue.packets.size());
    }
    
    return total_size;
}

void WFQ::clear_queues() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (auto& queue : queues_) {
        while (!queue.packets.empty()) {
            queue.packets.pop();
        }
        queue.virtual_time = 0;
    }
}

bool WFQ::is_empty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (const auto& queue : queues_) {
        if (!queue.packets.empty()) {
            return false;
        }
    }
    
    return true;
}

uint32_t WFQ::calculate_virtual_time(uint32_t queue_id, uint32_t packet_size) {
    if (queue_id >= num_queues_ || queues_[queue_id].weight == 0) {
        return 0;
    }
    
    // Virtual time calculation: current_virtual_time + packet_size / weight
    return queues_[queue_id].virtual_time + (packet_size * weight_base_) / queues_[queue_id].weight;
}

uint32_t WFQ::select_next_queue() {
    uint32_t selected_queue = 0;
    uint32_t min_virtual_time = UINT_MAX;
    uint32_t min_priority = UINT_MAX;
    
    // First pass: find queues with minimum virtual time
    for (uint32_t i = 0; i < num_queues_; ++i) {
        if (!queues_[i].packets.empty()) {
            if (queues_[i].virtual_time < min_virtual_time) {
                min_virtual_time = queues_[i].virtual_time;
                selected_queue = i;
            }
        }
    }
    
    // Second pass: if multiple queues have same virtual time, select by priority
    for (uint32_t i = 0; i < num_queues_; ++i) {
        if (!queues_[i].packets.empty() && 
            queues_[i].virtual_time == min_virtual_time &&
            queues_[i].priority < min_priority) {
            min_priority = queues_[i].priority;
            selected_queue = i;
        }
    }
    
    return selected_queue;
}

} // namespace RouterSim
