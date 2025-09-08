#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include <chrono>

#ifdef CLICKHOUSE_ENABLED
#include <clickhouse/client.h>
#endif

namespace RouterSim {

// ClickHouse Data Structures
struct Metric {
    std::chrono::system_clock::time_point timestamp;
    std::string router_id;
    std::string interface_name;
    std::string metric_name;
    double metric_value;
    std::map<std::string, std::string> tags;
    
    Metric() : metric_value(0.0) {
        timestamp = std::chrono::system_clock::now();
    }
};

struct PacketAnalytics {
    std::chrono::system_clock::time_point timestamp;
    std::string router_id;
    std::string interface_name;
    std::string source_ip;
    std::string dest_ip;
    uint16_t source_port;
    uint16_t dest_port;
    uint8_t protocol;
    uint32_t packet_size;
    uint64_t packet_count;
    uint64_t bytes_transferred;
    
    PacketAnalytics() : source_port(0), dest_port(0), protocol(0), packet_size(0), 
                       packet_count(0), bytes_transferred(0) {
        timestamp = std::chrono::system_clock::now();
    }
};

struct RoutingEvent {
    std::chrono::system_clock::time_point timestamp;
    std::string router_id;
    std::string event_type;
    std::string protocol;
    std::string destination;
    std::string gateway;
    std::string interface;
    uint32_t metric;
    std::string action;
    std::map<std::string, std::string> details;
    
    RoutingEvent() : metric(0) {
        timestamp = std::chrono::system_clock::now();
    }
};

struct TrafficShapingEvent {
    std::chrono::system_clock::time_point timestamp;
    std::string router_id;
    std::string interface_name;
    std::string shaping_type;
    uint32_t queue_id;
    uint64_t packets_processed;
    uint64_t bytes_processed;
    uint64_t packets_dropped;
    uint64_t bytes_dropped;
    double utilization_percentage;
    
    TrafficShapingEvent() : queue_id(0), packets_processed(0), bytes_processed(0),
                           packets_dropped(0), bytes_dropped(0), utilization_percentage(0.0) {
        timestamp = std::chrono::system_clock::now();
    }
};

// ClickHouse Client Class
class ClickHouseClient {
public:
    ClickHouseClient(const std::string& host = "localhost", 
                    uint16_t port = 9000, 
                    const std::string& database = "router_analytics");
    ~ClickHouseClient();
    
    // Connection management
    bool connect();
    void disconnect();
    bool isConnected() const;
    
    // Table management
    bool createTables();
    
    // Data insertion
    bool insertMetric(const Metric& metric);
    bool insertPacketAnalytics(const PacketAnalytics& analytics);
    bool insertRoutingEvent(const RoutingEvent& event);
    bool insertTrafficShapingEvent(const TrafficShapingEvent& event);
    
    // Data querying
    std::vector<Metric> queryMetrics(const std::string& query);
    
    // Query execution
    bool executeQuery(const std::string& query);
    
    // Statistics
    struct Statistics {
        bool connected;
        std::string host;
        uint16_t port;
        std::string database;
        uint64_t total_queries_executed;
        uint64_t total_queries_failed;
        uint64_t total_bytes_sent;
        uint64_t total_bytes_received;
        double success_rate;
    };
    
    Statistics getStatistics() const;
    void reset();

private:
    // Internal methods
    bool testConnection();
    
    // Configuration
    std::string host_;
    uint16_t port_;
    std::string database_;
    
    // Connection state
    bool connected_;
    
#ifdef CLICKHOUSE_ENABLED
    std::unique_ptr<clickhouse::Client> client_;
#else
    std::unique_ptr<void> client_; // Placeholder for when ClickHouse is disabled
#endif
    
    // Statistics
    uint64_t total_queries_executed_;
    uint64_t total_queries_failed_;
    uint64_t total_bytes_sent_;
    uint64_t total_bytes_received_;
    
    mutable std::mutex mutex_;
};

} // namespace RouterSim
