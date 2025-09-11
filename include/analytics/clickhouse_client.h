#pragma once

#include <string>
#include <map>
#include <vector>
#include <memory>

namespace RouterSim {

// ClickHouse connection configuration
struct ClickHouseConfig {
    std::string host;
    int port;
    std::string database;
    std::string username;
    std::string password;
    bool ssl;
    int timeout_seconds;
    
    ClickHouseConfig() : port(9000), ssl(false), timeout_seconds(30) {}
};

// Analytics data structures
struct PacketFlow {
    uint64_t timestamp;
    std::string src_ip;
    std::string dst_ip;
    uint16_t src_port;
    uint16_t dst_port;
    uint8_t protocol;
    uint32_t bytes;
    uint32_t packets;
    std::string interface;
};

struct RouteUpdate {
    uint64_t timestamp;
    std::string destination;
    uint8_t prefix_length;
    std::string next_hop;
    std::string protocol;
    uint32_t metric;
    bool is_add;
    std::string reason;
};

struct NeighborEvent {
    uint64_t timestamp;
    std::string neighbor_ip;
    std::string protocol;
    std::string event_type; // "up", "down", "established", "lost"
    std::string reason;
};

// ClickHouse analytics client
class ClickHouseClient {
public:
    ClickHouseClient();
    ~ClickHouseClient();
    
    // Connection management
    bool initialize(const ClickHouseConfig& config);
    bool connect();
    void disconnect();
    bool is_connected() const;
    
    // Data insertion
    bool insert_packet_flow(const PacketFlow& flow);
    bool insert_route_update(const RouteUpdate& update);
    bool insert_neighbor_event(const NeighborEvent& event);
    
    // Batch operations
    bool insert_packet_flows(const std::vector<PacketFlow>& flows);
    bool insert_route_updates(const std::vector<RouteUpdate>& updates);
    bool insert_neighbor_events(const std::vector<NeighborEvent>& events);
    
    // Query operations
    std::vector<PacketFlow> query_packet_flows(const std::string& query);
    std::vector<RouteUpdate> query_route_updates(const std::string& query);
    std::vector<NeighborEvent> query_neighbor_events(const std::string& query);
    
    // Statistics queries
    struct FlowStatistics {
        uint64_t total_packets;
        uint64_t total_bytes;
        std::string top_source_ip;
        std::string top_destination_ip;
        uint8_t top_protocol;
        double average_packet_size;
    };
    
    FlowStatistics get_flow_statistics(const std::string& time_range = "1h");
    
    // Configuration
    void set_config(const ClickHouseConfig& config);
    ClickHouseConfig get_config() const;

private:
    ClickHouseConfig config_;
    bool connected_;
    void* connection_; // Opaque pointer for ClickHouse connection
    
    // Internal methods
    bool execute_query(const std::string& query);
    std::string escape_string(const std::string& str);
};

} // namespace RouterSim
