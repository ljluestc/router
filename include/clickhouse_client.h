#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <cstdint>
#include <clickhouse/client.h>

namespace router_sim {

struct MetricData {
    uint64_t timestamp;
    std::string router_id;
    std::string interface;
    std::string metric_name;
    double metric_value;
    std::map<std::string, std::string> tags;
};

struct PacketFlowData {
    uint64_t timestamp;
    std::string source_ip;
    std::string dest_ip;
    uint16_t source_port;
    uint16_t dest_port;
    uint8_t protocol;
    uint32_t packet_size;
    std::string interface;
    std::string router_id;
    std::string flow_id;
};

struct RoutingEventData {
    uint64_t timestamp;
    std::string router_id;
    std::string event_type;
    std::string destination;
    std::string next_hop;
    std::string protocol;
    uint32_t metric;
    std::vector<std::string> as_path;
    std::vector<std::string> communities;
};

struct TrafficShapingStats {
    uint64_t timestamp;
    std::string router_id;
    std::string interface;
    std::string algorithm;
    uint64_t packets_processed;
    uint64_t bytes_processed;
    uint64_t packets_dropped;
    uint64_t bytes_dropped;
    double utilization_percent;
};

struct NetworkImpairmentData {
    uint64_t timestamp;
    std::string router_id;
    std::string interface;
    std::string impairment_type;
    std::map<std::string, std::string> parameters;
    bool active;
};

class ClickHouseClient {
public:
    ClickHouseClient(const std::string& host, uint16_t port, 
                    const std::string& database, const std::string& username, 
                    const std::string& password);
    ~ClickHouseClient();

    // Connection management
    bool connect();
    bool disconnect();
    bool is_connected() const;

    // Schema management
    bool create_tables();

    // Data insertion
    bool insert_metric(const MetricData& metric);
    bool insert_packet_flow(const PacketFlowData& flow);
    bool insert_routing_event(const RoutingEventData& event);
    bool insert_traffic_shaping_stats(const TrafficShapingStats& stats);
    bool insert_network_impairment(const NetworkImpairmentData& impairment);

    // Data querying
    std::vector<MetricData> query_metrics(const std::string& query);
    bool execute_query(const std::string& query);

    // Utility
    std::string get_connection_info() const;

private:
    std::string host_;
    uint16_t port_;
    std::string database_;
    std::string username_;
    std::string password_;
    bool connected_;
    std::unique_ptr<clickhouse::Client> client_;
};

} // namespace router_sim
