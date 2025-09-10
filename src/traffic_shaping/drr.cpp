#include "traffic_shaping.h"
#include <algorithm>
#include <limits>

namespace RouterSim {

DRR::DRR(uint32_t max_queues)
    : max_queues_(max_queues)
    , current_queue_(0)
    , total_packets_processed_(0)
    , total_bytes_processed_(0)
    , packets_dropped_(0)
    , bytes_dropped_(0)
{
    queues_.resize(max_queues_);
    for (auto& queue : queues_) {
        queue.quantum = 1500; // Default quantum
        queue.deficit = 0;
        queue.packets = 0;
        queue.bytes = 0;
    }
}

DRR::~DRR() {
}

bool DRR::enqueue(uint32_t queue_id, const Packet& packet) {
    if (queue_id >= max_queues_) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Check if queue is full
    if (queues_[queue_id].packets >= MAX_QUEUE_SIZE) {
        packets_dropped_++;
        bytes_dropped_ += packet.size;
        return false;
    }
    
    // Add packet to queue
    queues_[queue_id].packet_queue.push(packet);
    queues_[queue_id].packets++;
    queues_[queue_id].bytes += packet.size;
    
    return true;
}

bool DRR::dequeue(Packet& packet) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Try to dequeue from each queue in round-robin fashion
    for (uint32_t i = 0; i < max_queues_; ++i) {
        uint32_t queue_id = (current_queue_ + i) % max_queues_;
        auto& queue = queues_[queue_id];
        
        if (queue.packets == 0) {
            continue;
        }
        
        // Add quantum to deficit
        queue.deficit += queue.quantum;
        
        // Try to dequeue packets while we have deficit
        while (queue.deficit > 0 && !queue.packet_queue.empty()) {
            const Packet& front_packet = queue.packet_queue.front();
            
            if (front_packet.size <= queue.deficit) {
                // Can dequeue this packet
                packet = front_packet;
                queue.packet_queue.pop();
                queue.deficit -= front_packet.size;
                queue.packets--;
                queue.bytes -= front_packet.size;
                
                total_packets_processed_++;
                total_bytes_processed_ += packet.size;
                
                current_queue_ = (queue_id + 1) % max_queues_;
                return true;
            } else {
                // Not enough deficit, move to next queue
                break;
            }
        }
    }
    
    return false; // No packets to dequeue
}

void DRR::setQuantum(uint32_t queue_id, uint32_t quantum) {
    if (queue_id >= max_queues_) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    queues_[queue_id].quantum = quantum;
}

uint32_t DRR::getQuantum(uint32_t queue_id) const {
    if (queue_id >= max_queues_) {
        return 0;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    return queues_[queue_id].quantum;
}

uint32_t DRR::getQueueSize(uint32_t queue_id) const {
    if (queue_id >= max_queues_) {
        return 0;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    return queues_[queue_id].packets;
}

uint64_t DRR::getQueueBytes(uint32_t queue_id) const {
    if (queue_id >= max_queues_) {
        return 0;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    return queues_[queue_id].bytes;
}

DRR::Statistics DRR::getStatistics() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    Statistics stats;
    stats.max_queues = max_queues_;
    stats.total_packets_processed = total_packets_processed_;
    stats.total_bytes_processed = total_bytes_processed_;
    stats.packets_dropped = packets_dropped_;
    stats.bytes_dropped = bytes_dropped_;
    
    // Calculate queue statistics
    for (uint32_t i = 0; i < max_queues_; ++i) {
        QueueStatistics queue_stats;
        queue_stats.queue_id = i;
        queue_stats.quantum = queues_[i].quantum;
        queue_stats.deficit = queues_[i].deficit;
        queue_stats.packets = queues_[i].packets;
        queue_stats.bytes = queues_[i].bytes;
        stats.queue_stats.push_back(queue_stats);
    }
    
    return stats;
}

void DRR::reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (auto& queue : queues_) {
        queue.deficit = 0;
        queue.packets = 0;
        queue.bytes = 0;
        while (!queue.packet_queue.empty()) {
            queue.packet_queue.pop();
        }
    }
    
    current_queue_ = 0;
    total_packets_processed_ = 0;
    total_bytes_processed_ = 0;
    packets_dropped_ = 0;
    bytes_dropped_ = 0;
}

} // namespace RouterSim
