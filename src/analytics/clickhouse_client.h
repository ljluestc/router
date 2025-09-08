#pragma once

#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <chrono>
#include <atomic>
#include <thread>
#include <queue>
#include <condition_variable>

namespace RouterSim {

// Forward declarations
struct PacketMetrics;
struct RoutingMetrics;
struct SystemMetrics;
struct TrafficFlow;

/**
 * ClickHouse Analytics Client
 * High-performance analytics engine for router simulation data
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
        size_t buffer_size = 10000;
    };

    struct Statistics {
        std::atomic<uint64_t> packets_processed{0};
        std::atomic<uint64_t> bytes_processed{0};
        std::atomic<uint64_t> queries_executed{0};
        std::atomic<uint64_t> errors{0};
        std::atomic<uint64_t> buffer_size{0};
        std::atomic<uint64_t> dropped_metrics{0};
    };

    explicit ClickHouseClient(const Config& config);
    ~ClickHouseClient();

    // Initialization and lifecycle
    bool initialize();
    void shutdown();
    bool is_connected() const;

    // Data insertion methods
    bool insert_packet_metrics(const PacketMetrics& metrics);
    bool insert_routing_metrics(const RoutingMetrics& metrics);
    bool insert_system_metrics(const SystemMetrics& metrics);
    bool insert_traffic_flow(const TrafficFlow& flow);

    // Batch operations
    bool insert_packet_metrics_batch(const std::vector<PacketMetrics>& metrics);
    bool insert_routing_metrics_batch(const std::vector<RoutingMetrics>& metrics);
    bool insert_system_metrics_batch(const std::vector<SystemMetrics>& metrics);
    bool insert_traffic_flow_batch(const std::vector<TrafficFlow>& flows);

    // Query methods
    std::vector<PacketMetrics> query_packet_metrics(
        const std::string& where_clause = "",
        const std::string& order_by = "timestamp DESC",
        size_t limit = 1000
    );

    std::vector<RoutingMetrics> query_routing_metrics(
        const std::string& where_clause = "",
        const std::string& order_by = "timestamp DESC",
        size_t limit = 1000
    );

    std::vector<SystemMetrics> query_system_metrics(
        const std::string& where_clause = "",
        const std::string& order_by = "timestamp DESC",
        size_t limit = 1000
    );

    std::vector<TrafficFlow> query_traffic_flows(
        const std::string& where_clause = "",
        const std::string& order_by = "timestamp DESC",
        size_t limit = 1000
    );

    // Analytics queries
    struct TrafficSummary {
        uint64_t total_packets;
        uint64_t total_bytes;
        double avg_packet_size;
        double packets_per_second;
        double bytes_per_second;
        std::chrono::system_clock::time_point start_time;
        std::chrono::system_clock::time_point end_time;
    };

    TrafficSummary get_traffic_summary(
        const std::chrono::system_clock::time_point& start,
        const std::chrono::system_clock::time_point& end
    );

    struct ProtocolDistribution {
        std::string protocol;
        uint64_t packet_count;
        uint64_t byte_count;
        double percentage;
    };

    std::vector<ProtocolDistribution> get_protocol_distribution(
        const std::chrono::system_clock::time_point& start,
        const std::chrono::system_clock::time_point& end
    );

    struct TopTalkers {
        std::string source_ip;
        std::string dest_ip;
        uint64_t packet_count;
        uint64_t byte_count;
        double percentage;
    };

    std::vector<TopTalkers> get_top_talkers(
        const std::chrono::system_clock::time_point& start,
        const std::chrono::system_clock::time_point& end,
        size_t limit = 10
    );

    // Statistics and monitoring
    Statistics get_statistics() const;
    void reset_statistics();

private:
    Config config_;
    std::atomic<bool> connected_{false};
    std::atomic<bool> running_{false};
    Statistics stats_;

    // Connection pool
    struct Connection {
        int socket_fd = -1;
        bool in_use = false;
        std::chrono::system_clock::time_point last_used;
    };
    std::vector<Connection> connections_;
    std::mutex connection_mutex_;

    // Background processing
    std::thread worker_thread_;
    std::queue<std::string> insert_queue_;
    std::mutex queue_mutex_;
    std::condition_variable queue_cv_;

    // Internal methods
    bool connect_to_clickhouse();
    void disconnect_from_clickhouse();
    Connection* get_connection();
    void release_connection(Connection* conn);
    bool execute_query(const std::string& query, Connection* conn = nullptr);
    bool execute_insert(const std::string& insert_query, Connection* conn = nullptr);
    
    void worker_loop();
    void flush_buffers();
    
    std::string escape_string(const std::string& str);
    std::string format_timestamp(const std::chrono::system_clock::time_point& tp);
    
    // Table creation
    bool create_tables();
    bool create_packet_metrics_table();
    bool create_routing_metrics_table();
    bool create_system_metrics_table();
    bool create_traffic_flows_table();
};

// Data structures for analytics
struct PacketMetrics {
    std::chrono::system_clock::time_point timestamp;
    std::string source_ip;
    std::string dest_ip;
    uint16_t source_port;
    uint16_t dest_port;
    std::string protocol;
    uint32_t packet_size;
    uint8_t dscp;
    uint8_t ttl;
    std::string interface_name;
    std::string router_id;
    bool is_fragmented;
    bool is_encrypted;
    double processing_time_ms;
};

struct RoutingMetrics {
    std::chrono::system_clock::time_point timestamp;
    std::string router_id;
    std::string protocol;
    uint32_t routes_count;
    uint32_t active_routes;
    uint32_t routes_added;
    uint32_t routes_removed;
    uint32_t routes_modified;
    double convergence_time_ms;
    std::string neighbor_ip;
    std::string neighbor_as;
    std::string route_prefix;
    uint8_t route_prefix_length;
    uint32_t route_metric;
    std::string route_origin;
    std::string route_community;
};

struct SystemMetrics {
    std::chrono::system_clock::time_point timestamp;
    std::string router_id;
    double cpu_usage_percent;
    double memory_usage_percent;
    double disk_usage_percent;
    uint64_t network_bytes_in;
    uint64_t network_bytes_out;
    uint64_t packets_in;
    uint64_t packets_out;
    uint64_t packets_dropped;
    uint64_t packets_forwarded;
    double temperature_celsius;
    uint32_t fan_speed_rpm;
    std::string power_status;
    std::string uptime_seconds;
};

struct TrafficFlow {
    std::chrono::system_clock::time_point timestamp;
    std::string source_ip;
    std::string dest_ip;
    uint16_t source_port;
    uint16_t dest_port;
    std::string protocol;
    uint64_t packet_count;
    uint64_t byte_count;
    std::chrono::milliseconds duration;
    std::string application;
    std::string service;
    std::string country_code;
    std::string as_number;
    bool is_encrypted;
    double throughput_mbps;
    double latency_ms;
    double packet_loss_percent;
};

} // namespace RouterSim
