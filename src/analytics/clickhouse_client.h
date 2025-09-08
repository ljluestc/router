#pragma once

#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <atomic>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <future>

namespace router_sim {
namespace analytics {

// Forward declarations
struct PacketMetrics;
struct RouteMetrics;
struct SystemMetrics;
struct TrafficFlow;

/**
 * @brief High-performance ClickHouse client for analytics data ingestion
 * 
 * This class provides a thread-safe, high-throughput interface to ClickHouse
 * for storing router simulation analytics data. It uses connection pooling,
 * batch insertion, and asynchronous processing to maximize performance.
 */
class ClickHouseClient {
public:
    struct Config {
        std::string host = "localhost";
        uint16_t port = 9000;
        std::string database = "router_analytics";
        std::string username = "default";
        std::string password = "";
        size_t max_connections = 10;
        size_t batch_size = 1000;
        std::chrono::milliseconds flush_interval{1000};
        bool compression_enabled = true;
        size_t max_queue_size = 100000;
    };

    explicit ClickHouseClient(const Config& config);
    ~ClickHouseClient();

    // Connection management
    bool connect();
    void disconnect();
    bool is_connected() const;

    // Data insertion methods
    void insert_packet_metrics(const PacketMetrics& metrics);
    void insert_route_metrics(const RouteMetrics& metrics);
    void insert_system_metrics(const SystemMetrics& metrics);
    void insert_traffic_flow(const TrafficFlow& flow);

    // Batch operations
    void insert_packet_metrics_batch(const std::vector<PacketMetrics>& metrics);
    void insert_route_metrics_batch(const std::vector<RouteMetrics>& metrics);
    void insert_system_metrics_batch(const std::vector<SystemMetrics>& metrics);
    void insert_traffic_flow_batch(const std::vector<TrafficFlow>& flows);

    // Statistics and monitoring
    size_t get_queue_size() const;
    size_t get_processed_count() const;
    size_t get_error_count() const;
    double get_throughput() const; // records per second

    // Control methods
    void flush();
    void start_background_processing();
    void stop_background_processing();

private:
    class ConnectionPool;
    class BatchProcessor;
    class MetricsCollector;

    Config config_;
    std::unique_ptr<ConnectionPool> connection_pool_;
    std::unique_ptr<BatchProcessor> batch_processor_;
    std::unique_ptr<MetricsCollector> metrics_collector_;
    
    std::atomic<bool> running_{false};
    std::atomic<size_t> processed_count_{0};
    std::atomic<size_t> error_count_{0};
    
    // Background processing
    std::thread background_thread_;
    std::atomic<bool> should_stop_{false};
};

/**
 * @brief Packet-level metrics for analytics
 */
struct PacketMetrics {
    std::chrono::system_clock::time_point timestamp;
    std::string interface_name;
    std::string source_ip;
    std::string destination_ip;
    uint16_t source_port;
    uint16_t destination_port;
    uint8_t protocol; // IP protocol number
    uint16_t packet_size;
    uint8_t dscp;
    uint8_t ttl;
    bool is_fragmented;
    bool is_routed;
    bool is_dropped;
    std::chrono::microseconds processing_time;
    std::string next_hop;
    std::string route_source; // BGP, OSPF, ISIS, static, etc.
};

/**
 * @brief Route-level metrics for analytics
 */
struct RouteMetrics {
    std::chrono::system_clock::time_point timestamp;
    std::string network;
    uint8_t prefix_length;
    std::string next_hop;
    uint32_t metric;
    std::string protocol;
    uint32_t as_path_length;
    std::string communities;
    bool is_active;
    std::chrono::milliseconds age;
    uint32_t packet_count;
    uint64_t byte_count;
};

/**
 * @brief System-level metrics for analytics
 */
struct SystemMetrics {
    std::chrono::system_clock::time_point timestamp;
    double cpu_usage;
    double memory_usage;
    double disk_usage;
    uint64_t total_packets_processed;
    uint64_t total_bytes_processed;
    uint64_t packets_dropped;
    uint64_t routing_table_size;
    uint32_t active_neighbors;
    uint32_t active_interfaces;
    double packets_per_second;
    double bytes_per_second;
    double average_latency;
    double packet_loss_rate;
};

/**
 * @brief Traffic flow metrics for analytics
 */
struct TrafficFlow {
    std::chrono::system_clock::time_point timestamp;
    std::string flow_id;
    std::string source_ip;
    std::string destination_ip;
    uint16_t source_port;
    uint16_t destination_port;
    uint8_t protocol;
    uint64_t packet_count;
    uint64_t byte_count;
    std::chrono::milliseconds duration;
    std::string application;
    uint8_t dscp;
    bool is_encrypted;
    std::string traffic_class;
    double throughput;
    double jitter;
    double latency;
};

} // namespace analytics
} // namespace router_sim
