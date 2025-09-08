#pragma once

#include <queue>
#include <vector>
#include <map>
#include <mutex>
#include <atomic>
#include <chrono>
#include <memory>
#include <thread>
#include <condition_variable>

namespace router_sim {
namespace traffic_shaping {

struct Packet {
    std::vector<uint8_t> data;
    uint64_t arrival_time;
    uint64_t size;
    uint8_t priority;
    uint32_t flow_id;
    uint64_t finish_time;
    uint64_t virtual_time;
};

struct QueueConfig {
    uint32_t queue_id;
    uint32_t weight;
    uint64_t max_size;
    uint64_t min_guaranteed_rate;
    uint64_t max_rate;
    bool enabled;
    std::string name;
};

struct WFQConfig {
    uint64_t total_bandwidth;
    std::vector<QueueConfig> queues;
    uint64_t max_packet_size;
    uint64_t min_packet_size;
    std::chrono::milliseconds scheduling_interval{1};
    bool enabled;
};

class WFQScheduler {
public:
    explicit WFQScheduler(const WFQConfig& config);
    ~WFQScheduler();
    
    // Configuration
    bool add_queue(const QueueConfig& config);
    bool remove_queue(uint32_t queue_id);
    bool update_queue_config(uint32_t queue_id, const QueueConfig& config);
    void set_total_bandwidth(uint64_t bandwidth);
    
    // Packet operations
    bool enqueue_packet(uint32_t queue_id, const Packet& packet);
    bool dequeue_packet(Packet& packet);
    bool try_dequeue_packet(Packet& packet);
    
    // Queue management
    size_t get_queue_size(uint32_t queue_id) const;
    size_t get_total_packets() const;
    bool is_queue_empty(uint32_t queue_id) const;
    bool is_empty() const;
    
    // Statistics
    std::map<uint32_t, uint64_t> get_queue_statistics() const;
    uint64_t get_total_packets_processed() const;
    uint64_t get_total_packets_dropped() const;
    double get_utilization() const;
    
    // Control
    void start();
    void stop();
    bool is_running() const;
    
private:
    struct Queue {
        std::queue<Packet> packets;
        uint32_t weight;
        uint64_t max_size;
        uint64_t min_guaranteed_rate;
        uint64_t max_rate;
        bool enabled;
        std::string name;
        
        // WFQ specific
        uint64_t virtual_time;
        uint64_t last_finish_time;
        uint64_t packets_processed;
        uint64_t packets_dropped;
        uint64_t bytes_processed;
    };
    
    WFQConfig config_;
    std::map<uint32_t, Queue> queues_;
    mutable std::mutex queues_mutex_;
    
    std::atomic<bool> running_;
    std::thread scheduler_thread_;
    std::condition_variable scheduler_cv_;
    
    // WFQ state
    uint64_t virtual_time_;
    uint64_t last_schedule_time_;
    
    // Statistics
    std::atomic<uint64_t> total_packets_processed_;
    std::atomic<uint64_t> total_packets_dropped_;
    std::atomic<uint64_t> total_bytes_processed_;
    
    // Internal methods
    void scheduler_loop();
    uint32_t select_next_queue();
    void update_virtual_time();
    uint64_t calculate_finish_time(const Packet& packet, uint32_t queue_id);
    bool is_queue_eligible(uint32_t queue_id) const;
    void process_packet(const Packet& packet);
};

class WFQShaper {
public:
    WFQShaper();
    ~WFQShaper();
    
    // Configuration
    bool add_scheduler(const std::string& name, const WFQConfig& config);
    bool remove_scheduler(const std::string& name);
    bool update_scheduler_config(const std::string& name, const WFQConfig& config);
    
    // Shaping operations
    bool shape_packet(const std::string& scheduler_name, uint32_t queue_id, const Packet& packet);
    bool try_shape_packet(const std::string& scheduler_name, uint32_t queue_id, const Packet& packet);
    
    // Statistics
    std::map<std::string, std::map<uint32_t, uint64_t>> get_scheduler_statistics() const;
    uint64_t get_total_packets_processed() const;
    uint64_t get_total_packets_dropped() const;
    
    // Control
    void start_all();
    void stop_all();
    void start_scheduler(const std::string& name);
    void stop_scheduler(const std::string& name);
    
private:
    std::map<std::string, std::unique_ptr<WFQScheduler>> schedulers_;
    std::mutex schedulers_mutex_;
    
    std::atomic<uint64_t> total_packets_processed_;
    std::atomic<uint64_t> total_packets_dropped_;
};

} // namespace traffic_shaping
} // namespace router_sim
