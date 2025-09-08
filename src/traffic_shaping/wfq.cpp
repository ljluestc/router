#include "traffic_shaping.h"
#include <iostream>
#include <algorithm>
#include <chrono>
#include <cmath>

namespace RouterSim {

WFQQueue::WFQQueue(uint32_t weight, uint64_t max_size)
    : weight_(weight), max_size_(max_size), current_size_(0), 
      finish_time_(0.0), virtual_time_(0.0) {
}

WFQQueue::~WFQQueue() {
}

bool WFQQueue::enqueue(const Packet& packet) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (current_size_ + packet.size > max_size_) {
        stats_.packets_dropped++;
        stats_.bytes_dropped += packet.size;
        return false;
    }
    
    // Calculate finish time for this packet
    double packet_finish_time = virtual_time_ + (static_cast<double>(packet.size) / weight_);
    
    PacketInfo packet_info;
    packet_info.packet = packet;
    packet_info.finish_time = packet_finish_time;
    packet_info.arrival_time = std::chrono::steady_clock::now();
    
    queue_.push_back(packet_info);
    current_size_ += packet.size;
    
    stats_.packets_enqueued++;
    stats_.bytes_enqueued += packet.size;
    
    return true;
}

bool WFQQueue::dequeue(Packet& packet) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (queue_.empty()) {
        return false;
    }
    
    // Find packet with minimum finish time
    auto min_it = std::min_element(queue_.begin(), queue_.end(),
        [](const PacketInfo& a, const PacketInfo& b) {
            return a.finish_time < b.finish_time;
        });
    
    packet = min_it->packet;
    current_size_ -= packet.size;
    
    // Update virtual time
    virtual_time_ = min_it->finish_time;
    
    queue_.erase(min_it);
    
    stats_.packets_dequeued++;
    stats_.bytes_dequeued += packet.size;
    
    return true;
}

bool WFQQueue::peek(Packet& packet) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (queue_.empty()) {
        return false;
    }
    
    // Find packet with minimum finish time
    auto min_it = std::min_element(queue_.begin(), queue_.end(),
        [](const PacketInfo& a, const PacketInfo& b) {
            return a.finish_time < b.finish_time;
        });
    
    packet = min_it->packet;
    return true;
}

bool WFQQueue::is_empty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.empty();
}

size_t WFQQueue::size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
}

uint64_t WFQQueue::current_size_bytes() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return current_size_;
}

void WFQQueue::set_weight(uint32_t weight) {
    std::lock_guard<std::mutex> lock(mutex_);
    weight_ = weight;
}

void WFQQueue::set_max_size(uint64_t max_size) {
    std::lock_guard<std::mutex> lock(mutex_);
    max_size_ = max_size;
}

WFQQueue::Statistics WFQQueue::get_statistics() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return stats_;
}

void WFQQueue::reset_statistics() {
    std::lock_guard<std::mutex> lock(mutex_);
    stats_ = Statistics();
}

// WFQScheduler implementation
WFQScheduler::WFQScheduler(uint64_t total_bandwidth_bps)
    : total_bandwidth_bps_(total_bandwidth_bps), 
      virtual_time_(0.0), last_update_time_(std::chrono::steady_clock::now()) {
}

WFQScheduler::~WFQScheduler() {
}

bool WFQScheduler::add_queue(const std::string& name, uint32_t weight, uint64_t max_size) {
    std::lock_guard<std::mutex> lock(queues_mutex_);
    
    auto queue = std::make_unique<WFQQueue>(weight, max_size);
    queues_[name] = std::move(queue);
    
    std::cout << "Added WFQ queue: " << name 
              << " (weight: " << weight << ", max_size: " << max_size << " bytes)" << std::endl;
    return true;
}

bool WFQScheduler::remove_queue(const std::string& name) {
    std::lock_guard<std::mutex> lock(queues_mutex_);
    
    auto it = queues_.find(name);
    if (it != queues_.end()) {
        queues_.erase(it);
        std::cout << "Removed WFQ queue: " << name << std::endl;
        return true;
    }
    
    return false;
}

bool WFQScheduler::enqueue_packet(const std::string& queue_name, const Packet& packet) {
    std::lock_guard<std::mutex> lock(queues_mutex_);
    
    auto it = queues_.find(queue_name);
    if (it != queues_.end()) {
        return it->second->enqueue(packet);
    }
    
    return false;
}

bool WFQScheduler::dequeue_packet(Packet& packet) {
    std::lock_guard<std::mutex> lock(queues_mutex_);
    
    // Update virtual time
    update_virtual_time();
    
    // Find queue with minimum finish time
    std::string min_queue;
    double min_finish_time = std::numeric_limits<double>::max();
    
    for (const auto& pair : queues_) {
        if (!pair.second->is_empty()) {
            Packet peek_packet;
            if (pair.second->peek(peek_packet)) {
                // Calculate finish time for this queue
                double finish_time = virtual_time_ + (static_cast<double>(peek_packet.size) / pair.second->weight_);
                if (finish_time < min_finish_time) {
                    min_finish_time = finish_time;
                    min_queue = pair.first;
                }
            }
        }
    }
    
    if (!min_queue.empty()) {
        auto it = queues_.find(min_queue);
        if (it != queues_.end()) {
            bool success = it->second->dequeue(packet);
            if (success) {
                stats_.packets_scheduled++;
                stats_.bytes_scheduled += packet.size;
            }
            return success;
        }
    }
    
    return false;
}

bool WFQScheduler::peek_next_packet(Packet& packet) const {
    std::lock_guard<std::mutex> lock(queues_mutex_);
    
    // Find queue with minimum finish time
    std::string min_queue;
    double min_finish_time = std::numeric_limits<double>::max();
    
    for (const auto& pair : queues_) {
        if (!pair.second->is_empty()) {
            Packet peek_packet;
            if (pair.second->peek(peek_packet)) {
                // Calculate finish time for this queue
                double finish_time = virtual_time_ + (static_cast<double>(peek_packet.size) / pair.second->weight_);
                if (finish_time < min_finish_time) {
                    min_finish_time = finish_time;
                    min_queue = pair.first;
                }
            }
        }
    }
    
    if (!min_queue.empty()) {
        auto it = queues_.find(min_queue);
        if (it != queues_.end()) {
            return it->second->peek(packet);
        }
    }
    
    return false;
}

bool WFQScheduler::is_empty() const {
    std::lock_guard<std::mutex> lock(queues_mutex_);
    
    for (const auto& pair : queues_) {
        if (!pair.second->is_empty()) {
            return false;
        }
    }
    
    return true;
}

size_t WFQScheduler::total_packets() const {
    std::lock_guard<std::mutex> lock(queues_mutex_);
    
    size_t total = 0;
    for (const auto& pair : queues_) {
        total += pair.second->size();
    }
    
    return total;
}

uint64_t WFQScheduler::total_bytes() const {
    std::lock_guard<std::mutex> lock(queues_mutex_);
    
    uint64_t total = 0;
    for (const auto& pair : queues_) {
        total += pair.second->current_size_bytes();
    }
    
    return total;
}

void WFQScheduler::set_queue_weight(const std::string& name, uint32_t weight) {
    std::lock_guard<std::mutex> lock(queues_mutex_);
    
    auto it = queues_.find(name);
    if (it != queues_.end()) {
        it->second->set_weight(weight);
    }
}

void WFQScheduler::set_queue_max_size(const std::string& name, uint64_t max_size) {
    std::lock_guard<std::mutex> lock(queues_mutex_);
    
    auto it = queues_.find(name);
    if (it != queues_.end()) {
        it->second->set_max_size(max_size);
    }
}

void WFQScheduler::set_total_bandwidth(uint64_t bandwidth_bps) {
    total_bandwidth_bps_ = bandwidth_bps;
}

std::vector<std::string> WFQScheduler::get_queue_names() const {
    std::lock_guard<std::mutex> lock(queues_mutex_);
    
    std::vector<std::string> names;
    for (const auto& pair : queues_) {
        names.push_back(pair.first);
    }
    return names;
}

WFQQueue::Statistics WFQScheduler::get_queue_statistics(const std::string& name) const {
    std::lock_guard<std::mutex> lock(queues_mutex_);
    
    auto it = queues_.find(name);
    if (it != queues_.end()) {
        return it->second->get_statistics();
    }
    
    return WFQQueue::Statistics();
}

WFQScheduler::Statistics WFQScheduler::get_total_statistics() const {
    return stats_;
}

void WFQScheduler::reset_all_statistics() {
    std::lock_guard<std::mutex> lock(queues_mutex_);
    
    stats_ = Statistics();
    for (const auto& pair : queues_) {
        pair.second->reset_statistics();
    }
}

void WFQScheduler::update_virtual_time() {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
        now - last_update_time_).count();
    
    if (elapsed > 0) {
        // Update virtual time based on elapsed time and total bandwidth
        double time_increment = static_cast<double>(elapsed) / 1000000.0; // Convert to seconds
        virtual_time_ += time_increment * total_bandwidth_bps_;
        last_update_time_ = now;
    }
}

// PriorityWFQScheduler implementation
PriorityWFQScheduler::PriorityWFQScheduler(uint64_t total_bandwidth_bps)
    : WFQScheduler(total_bandwidth_bps) {
}

PriorityWFQScheduler::~PriorityWFQScheduler() {
}

bool PriorityWFQScheduler::add_priority_queue(const std::string& name, 
                                             uint8_t priority, 
                                             uint32_t weight, 
                                             uint64_t max_size) {
    std::lock_guard<std::mutex> lock(priority_queues_mutex_);
    
    auto queue = std::make_unique<WFQQueue>(weight, max_size);
    priority_queues_[priority][name] = std::move(queue);
    
    std::cout << "Added priority WFQ queue: " << name 
              << " (priority: " << static_cast<int>(priority) 
              << ", weight: " << weight << ", max_size: " << max_size << " bytes)" << std::endl;
    return true;
}

bool PriorityWFQScheduler::remove_priority_queue(const std::string& name, uint8_t priority) {
    std::lock_guard<std::mutex> lock(priority_queues_mutex_);
    
    auto priority_it = priority_queues_.find(priority);
    if (priority_it != priority_queues_.end()) {
        auto queue_it = priority_it->second.find(name);
        if (queue_it != priority_it->second.end()) {
            priority_it->second.erase(queue_it);
            std::cout << "Removed priority WFQ queue: " << name << std::endl;
            return true;
        }
    }
    
    return false;
}

bool PriorityWFQScheduler::enqueue_priority_packet(const std::string& queue_name, 
                                                  uint8_t priority, 
                                                  const Packet& packet) {
    std::lock_guard<std::mutex> lock(priority_queues_mutex_);
    
    auto priority_it = priority_queues_.find(priority);
    if (priority_it != priority_queues_.end()) {
        auto queue_it = priority_it->second.find(queue_name);
        if (queue_it != priority_it->second.end()) {
            return queue_it->second->enqueue(packet);
        }
    }
    
    return false;
}

bool PriorityWFQScheduler::dequeue_priority_packet(Packet& packet) {
    std::lock_guard<std::mutex> lock(priority_queues_mutex_);
    
    // Process queues in priority order (lower number = higher priority)
    for (auto& priority_pair : priority_queues_) {
        uint8_t priority = priority_pair.first;
        auto& queues = priority_pair.second;
        
        // Find queue with minimum finish time in this priority level
        std::string min_queue;
        double min_finish_time = std::numeric_limits<double>::max();
        
        for (const auto& queue_pair : queues) {
            if (!queue_pair.second->is_empty()) {
                Packet peek_packet;
                if (queue_pair.second->peek(peek_packet)) {
                    // Calculate finish time for this queue
                    double finish_time = virtual_time_ + (static_cast<double>(peek_packet.size) / queue_pair.second->weight_);
                    if (finish_time < min_finish_time) {
                        min_finish_time = finish_time;
                        min_queue = queue_pair.first;
                    }
                }
            }
        }
        
        if (!min_queue.empty()) {
            auto it = queues.find(min_queue);
            if (it != queues.end()) {
                bool success = it->second->dequeue(packet);
                if (success) {
                    stats_.packets_scheduled++;
                    stats_.bytes_scheduled += packet.size;
                }
                return success;
            }
        }
    }
    
    return false;
}

std::vector<std::string> PriorityWFQScheduler::get_priority_queue_names(uint8_t priority) const {
    std::lock_guard<std::mutex> lock(priority_queues_mutex_);
    
    std::vector<std::string> names;
    auto priority_it = priority_queues_.find(priority);
    if (priority_it != priority_queues_.end()) {
        for (const auto& pair : priority_it->second) {
            names.push_back(pair.first);
        }
    }
    return names;
}

} // namespace RouterSim
