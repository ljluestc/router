#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <cstdint>

// Forward declaration to avoid including ClickHouse headers in header
namespace clickhouse {
    class Client;
}

namespace RouterSim {
namespace Analytics {

// Data structures for analytics
struct RoutingEvent {
    int64_t timestamp;
    std::string router_id;
    std::string protocol;
    std::string event_type;
    std::string prefix;
    std::string next_hop;
    uint32_t metric;
    std::vector<std::string> as_path;
    std::vector<std::string> community;
    uint32_t local_pref;
    std::string origin;
    uint32_t path_id;
    std::string peer_ip;
    uint32_t peer_as;
    std::string session_id;
    uint32_t duration_ms;
    uint8_t success;
};

struct TrafficFlow {
    int64_t timestamp;
    std::string router_id;
    std::string interface;
    std::string src_ip;
    std::string dst_ip;
    uint16_t src_port;
    uint16_t dst_port;
    std::string protocol;
    uint64_t bytes;
    uint64_t packets;
    uint32_t duration_ms;
    uint8_t tcp_flags;
    uint8_t tos;
    uint8_t ttl;
    std::string flow_id;
    uint16_t vlan_id;
    uint32_t mpls_label;
};

struct PerformanceMetric {
    int64_t timestamp;
    std::string router_id;
    std::string metric_name;
    double metric_value;
    std::string metric_unit;
    std::map<std::string, std::string> tags;
    std::string aggregation_level;
};

struct NetworkTopology {
    int64_t timestamp;
    std::string router_id;
    std::string neighbor_id;
    std::string neighbor_ip;
    uint32_t neighbor_as;
    std::string protocol;
    std::string state;
    uint32_t uptime;
    uint16_t keepalive_interval;
    uint16_t hold_time;
    std::vector<std::string> capabilities;
    std::vector<std::string> remote_capabilities;
    std::string session_id;
};

class ClickHouseClient {
public:
    ClickHouseClient(const std::string& host = "localhost", 
                     uint16_t port = 9000, 
                     const std::string& database = "router_analytics");
    ~ClickHouseClient();

    bool connect();
    void disconnect();
    bool isConnected() const { return connected_; }

    // Database operations
    void createDatabase();
    void createTables();

    // Data insertion
    void insertRoutingEvent(const RoutingEvent& event);
    void insertTrafficFlow(const TrafficFlow& flow);
    void insertPerformanceMetric(const PerformanceMetric& metric);
    void insertNetworkTopology(const NetworkTopology& topology);
    
    // Batch operations for high performance
    void batchInsert(const std::vector<RoutingEvent>& events);
    void batchInsert(const std::vector<TrafficFlow>& flows);
    void batchInsert(const std::vector<PerformanceMetric>& metrics);
    void batchInsert(const std::vector<NetworkTopology>& topologies);

    // Data querying
    std::vector<RoutingEvent> queryRoutingEvents(const std::string& filter = "", uint32_t limit = 1000);
    std::vector<TrafficFlow> queryTrafficFlows(const std::string& filter = "", uint32_t limit = 1000);
    std::vector<PerformanceMetric> queryPerformanceMetrics(const std::string& filter = "", uint32_t limit = 1000);
    std::vector<NetworkTopology> queryNetworkTopology(const std::string& filter = "", uint32_t limit = 1000);

    // Analytics queries
    std::vector<std::pair<std::string, uint64_t>> getTopPrefixes(uint32_t limit = 10);
    std::vector<std::pair<std::string, uint64_t>> getTopASes(uint32_t limit = 10);
    std::vector<std::pair<std::string, double>> getProtocolDistribution();
    std::vector<std::pair<std::string, double>> getTrafficDistribution();
    
    // Performance metrics
    double getAverageConvergenceTime(const std::string& protocol = "");
    uint64_t getTotalTrafficBytes(const std::string& timeRange = "1h");
    uint32_t getActiveFlowsCount();
    double getPacketLossRate(const std::string& interface = "");

    // CloudPods specific analytics
    std::vector<std::pair<std::string, uint64_t>> getVPCTrafficStats();
    std::vector<std::pair<std::string, uint64_t>> getNATGatewayStats();
    std::vector<std::pair<std::string, uint64_t>> getLoadBalancerStats();
    std::vector<std::pair<std::string, uint64_t>> getServiceMeshStats();

private:
    std::string host_;
    uint16_t port_;
    std::string database_;
    bool connected_;
    std::unique_ptr<clickhouse::Client> client_;

    // Helper methods
    std::string buildQuery(const std::string& table, const std::string& filter, uint32_t limit);
    void executeQuery(const std::string& query);
};

} // namespace Analytics
} // namespace RouterSim
