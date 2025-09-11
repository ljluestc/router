#pragma once

#include "traffic_shaping.h"
#include <queue>
#include <map>
#include <functional>

namespace RouterSim {

// DRR-specific structures
struct DRRClass {
    uint8_t class_id;
    uint32_t quantum;
    uint32_t deficit;
    uint64_t min_bandwidth;
    uint64_t max_bandwidth;
    std::string name;
    bool is_active;
    std::map<std::string, std::string> attributes;
};

struct DRRQueueItem {
    PacketInfo packet;
    uint8_t class_id;
    std::chrono::steady_clock::time_point enqueue_time;
};

struct DRRClassStatistics {
    uint8_t class_id;
    uint64_t packets_queued;
    uint64_t packets_dequeued;
    uint64_t bytes_queued;
    uint64_t bytes_dequeued;
    size_t current_queue_length;
    uint32_t current_deficit;
    std::chrono::steady_clock::time_point last_activity;
};

struct DRRStatistics {
    uint64_t total_packets_queued;
    uint64_t total_packets_dequeued;
    uint64_t total_bytes_queued;
    uint64_t total_bytes_dequeued;
    size_t current_queue_length;
    std::map<uint8_t, DRRClassStatistics> class_statistics;
};

class DeficitRoundRobin {
public:
    DeficitRoundRobin();
    ~DeficitRoundRobin();

    // Core DRR operations
    bool initialize(const std::vector<DRRClass>& classes);
    bool enqueue_packet(const PacketInfo& packet, uint8_t class_id);
    bool dequeue_packet(PacketInfo& packet);
    bool is_empty() const;
    size_t queue_size() const;
    size_t queue_size(uint8_t class_id) const;

    // Class management
    bool add_class(const DRRClass& drr_class);
    bool remove_class(uint8_t class_id);
    bool update_class(const DRRClass& drr_class);
    std::vector<DRRClass> get_classes() const;

    // Packet classification
    void set_classifier(std::function<uint8_t(const PacketInfo&)> classifier);
    uint8_t classify_packet(const PacketInfo& packet) const;

    // Statistics
    DRRStatistics get_statistics() const;
    void reset_statistics();

private:
    // Internal state
    std::vector<DRRClass> classes_;
    std::map<uint8_t, std::queue<DRRQueueItem>> queues_;
    std::function<uint8_t(const PacketInfo&)> classifier_;
    
    // Statistics tracking
    uint64_t total_packets_queued_;
    uint64_t total_packets_dequeued_;
    uint64_t total_bytes_queued_;
    uint64_t total_bytes_dequeued_;
    std::map<uint8_t, uint64_t> class_packets_queued_;
    std::map<uint8_t, uint64_t> class_packets_dequeued_;
    std::map<uint8_t, uint64_t> class_bytes_queued_;
    std::map<uint8_t, uint64_t> class_bytes_dequeued_;
    std::map<uint8_t, std::chrono::steady_clock::time_point> class_last_activity_;
    
    mutable std::mutex mutex_;

    // Internal methods
    bool select_next_packet(DRRQueueItem& item);
    void update_deficit(uint8_t class_id, uint32_t packet_size);
};

} // namespace RouterSim
