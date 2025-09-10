#pragma once

#include "traffic_shaping.h"
#include <queue>
#include <map>
#include <functional>

namespace RouterSim {

// WFQ-specific structures
struct WFQClass {
    uint8_t class_id;
    uint32_t weight;
    uint64_t min_bandwidth;
    uint64_t max_bandwidth;
    std::string name;
    bool is_active;
    std::map<std::string, std::string> attributes;
};

struct QueueItem {
    PacketInfo packet;
    uint8_t class_id;
    std::chrono::steady_clock::time_point enqueue_time;
    uint64_t virtual_finish_time;
};

struct ClassStatistics {
    uint8_t class_id;
    uint64_t packets_queued;
    uint64_t packets_dequeued;
    uint64_t bytes_queued;
    uint64_t bytes_dequeued;
    size_t current_queue_length;
    std::chrono::steady_clock::time_point last_activity;
};

struct WFQStatistics {
    uint64_t total_packets_queued;
    uint64_t total_packets_dequeued;
    uint64_t total_bytes_queued;
    uint64_t total_bytes_dequeued;
    size_t current_queue_length;
    std::map<uint8_t, ClassStatistics> class_statistics;
};

class WeightedFairQueue {
public:
    WeightedFairQueue();
    ~WeightedFairQueue();

    // Core WFQ operations
    bool initialize(const std::vector<WFQClass>& classes);
    bool enqueue_packet(const PacketInfo& packet, uint8_t class_id);
    bool dequeue_packet(PacketInfo& packet);
    bool is_empty() const;
    size_t queue_size() const;
    size_t queue_size(uint8_t class_id) const;

    // Class management
    bool add_class(const WFQClass& wfq_class);
    bool remove_class(uint8_t class_id);
    bool update_class(const WFQClass& wfq_class);
    std::vector<WFQClass> get_classes() const;

    // Packet classification
    void set_classifier(std::function<uint8_t(const PacketInfo&)> classifier);
    uint8_t classify_packet(const PacketInfo& packet) const;

    // Statistics
    WFQStatistics get_statistics() const;
    void reset_statistics();

private:
    // Internal state
    std::vector<WFQClass> classes_;
    std::map<uint8_t, std::queue<QueueItem>> queues_;
    uint64_t virtual_time_;
    std::function<uint8_t(const PacketInfo&)> classifier_;
    
    mutable std::mutex mutex_;

    // Internal methods
    uint64_t calculate_virtual_finish_time(const PacketInfo& packet, uint8_t class_id) const;
    bool select_next_packet(QueueItem& item);
};

} // namespace RouterSim
