#pragma once

#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <map>
#include <atomic>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>

namespace router_sim {
namespace analytics {

struct MetricData {
    std::string name;
    std::string value;
    std::string labels;
    std::chrono::system_clock::time_point timestamp;
    std::string source;
    std::string type;
};

struct PacketMetrics {
    uint64_t total_packets;
    uint64_t bytes_transferred;
    uint64_t packets_dropped;
    uint64_t packets_duplicated;
    uint64_t packets_reordered;
    double avg_latency_ms;
    double max_latency_ms;
    double min_latency_ms;
    std::chrono::system_clock::time_point timestamp;
};

struct RoutingMetrics {
    uint64_t total_routes;
    uint64_t bgp_routes;
    uint64_t ospf_routes;
    uint64_t isis_routes;
    uint64_t static_routes;
    uint64_t route_changes;
    double convergence_time_ms;
    std::chrono::system_clock::time_point timestamp;
};

class ClickHouseClient {
public:
    ClickHouseClient(const std::string& host = "localhost", 
                    uint16_t port = 9000,
                    const std::string& database = "router_sim",
                    const std::string& username = "default",
                    const std::string& password = "");
    
    ~ClickHouseClient();
    
    bool connect();
    void disconnect();
    bool is_connected() const;
    
    // High-performance batch insert operations
    bool insert_metrics(const std::vector<MetricData>& metrics);
    bool insert_packet_metrics(const std::vector<PacketMetrics>& metrics);
    bool insert_routing_metrics(const std::vector<RoutingMetrics>& metrics);
    
    // Async operations for high throughput
    void async_insert_metrics(const std::vector<MetricData>& metrics);
    void async_insert_packet_metrics(const std::vector<PacketMetrics>& metrics);
    void async_insert_routing_metrics(const std::vector<RoutingMetrics>& metrics);
    
    // Query operations
    std::vector<MetricData> query_metrics(const std::string& query);
    std::vector<PacketMetrics> query_packet_metrics(const std::string& query);
    std::vector<RoutingMetrics> query_routing_metrics(const std::string& query);
    
    // Performance monitoring
    uint64_t get_insert_count() const;
    uint64_t get_query_count() const;
    double get_avg_insert_time_ms() const;
    double get_avg_query_time_ms() const;
    
    // Configuration
    void set_batch_size(size_t size);
    void set_flush_interval(std::chrono::milliseconds interval);
    void set_max_queue_size(size_t size);
    
private:
    std::string host_;
    uint16_t port_;
    std::string database_;
    std::string username_;
    std::string password_;
    
    bool connected_;
    std::atomic<uint64_t> insert_count_;
    std::atomic<uint64_t> query_count_;
    std::atomic<double> total_insert_time_ms_;
    std::atomic<double> total_query_time_ms_;
    
    // Batch processing
    size_t batch_size_;
    std::chrono::milliseconds flush_interval_;
    size_t max_queue_size_;
    
    // Async processing
    std::queue<std::vector<MetricData>> metrics_queue_;
    std::queue<std::vector<PacketMetrics>> packet_metrics_queue_;
    std::queue<std::vector<RoutingMetrics>> routing_metrics_queue_;
    
    std::mutex metrics_mutex_;
    std::mutex packet_metrics_mutex_;
    std::mutex routing_metrics_mutex_;
    
    std::condition_variable metrics_cv_;
    std::condition_variable packet_metrics_cv_;
    std::condition_variable routing_metrics_cv_;
    
    std::thread metrics_worker_;
    std::thread packet_metrics_worker_;
    std::thread routing_metrics_worker_;
    
    std::atomic<bool> stop_workers_;
    
    // Internal methods
    void metrics_worker_loop();
    void packet_metrics_worker_loop();
    void routing_metrics_worker_loop();
    
    bool execute_query(const std::string& query);
    bool execute_insert(const std::string& table, const std::string& data);
    
    std::string serialize_metrics(const std::vector<MetricData>& metrics);
    std::string serialize_packet_metrics(const std::vector<PacketMetrics>& metrics);
    std::string serialize_routing_metrics(const std::vector<RoutingMetrics>& metrics);
    
    void create_tables();
    void create_metrics_table();
    void create_packet_metrics_table();
    void create_routing_metrics_table();
};

} // namespace analytics
} // namespace router_sim
