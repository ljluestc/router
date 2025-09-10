#pragma once

#include <queue>
#include <map>
#include <vector>
#include <mutex>
#include <functional>
#include <chrono>

namespace RouterSim {

// Forward declaration
struct PacketInfo;

// WFQ class definition
struct WFQClass {
    uint8_t class_id;
    uint32_t weight;
    uint64_t min_bandwidth;
    uint64_t max_bandwidth;
    std::string name;
    bool is_active;
    
    WFQClass() : class_id(0), weight(1), min_bandwidth(0), max_bandwidth(0), is_active(true) {}
};

// Queue item for WFQ
struct QueueItem {
    PacketInfo packet;
    uint8_t class_id;
    std::chrono::steady_clock::time_point enqueue_time;
    uint64_t virtual_finish_time;
};

// Weighted Fair Queue implementation
class WeightedFairQueue {
public:
    WeightedFairQueue();
    ~WeightedFairQueue();
    
    // Initialization
    bool initialize(const std::vector<WFQClass>& classes);
    
    // Packet operations
    bool enqueue_packet(const PacketInfo& packet, uint8_t class_id);
    bool dequeue_packet(PacketInfo& packet);
    
    // Queue status
    bool is_empty() const;
    size_t queue_size() const;
    size_t queue_size(uint8_t class_id) const;
    
    // Class management
    bool add_class(const WFQClass& wfq_class);
    bool remove_class(uint8_t class_id);
    bool update_class(const WFQClass& wfq_class);
    std::vector<WFQClass> get_classes() const;
    
    // Classification
    void set_classifier(std::function<uint8_t(const PacketInfo&)> classifier);
    uint8_t classify_packet(const PacketInfo& packet) const;
    
private:
    // Virtual finish time calculation
    uint64_t calculate_virtual_finish_time(const PacketInfo& packet, uint8_t class_id) const;
    
    // Packet selection
    bool select_next_packet(QueueItem& item);
    
    // Data structures
    std::vector<WFQClass> classes_;
    std::map<uint8_t, std::queue<QueueItem>> queues_;
    uint64_t virtual_time_;
    
    // Classification function
    std::function<uint8_t(const PacketInfo&)> classifier_;
    
    // Thread safety
    mutable std::mutex mutex_;
};

} // namespace RouterSim
