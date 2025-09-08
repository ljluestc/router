#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>

namespace router_sim {
namespace frr {

struct BGPNeighbor {
    std::string ip;
    uint32_t asn;
    std::string password;
    std::string description;
    bool enabled;
    std::string state;
    uint32_t uptime;
    uint32_t messages_sent;
    uint32_t messages_received;
    uint32_t routes_received;
    uint32_t routes_advertised;
};

struct BGPConfig {
    uint32_t local_asn;
    std::string router_id;
    std::vector<BGPNeighbor> neighbors;
    std::vector<std::string> networks;
    std::map<std::string, std::string> policies;
};

struct OSPFInterface {
    std::string name;
    uint32_t area;
    uint32_t cost;
    uint32_t priority;
    bool enabled;
    std::string state;
    uint32_t hello_interval;
    uint32_t dead_interval;
};

struct OSPFConfig {
    std::string router_id;
    std::vector<OSPFInterface> interfaces;
    std::vector<uint32_t> areas;
    std::vector<std::string> redistribute;
};

struct ISISInterface {
    std::string name;
    uint8_t level;
    uint32_t cost;
    bool enabled;
    std::string state;
    std::string circuit_type;
};

struct ISISConfig {
    std::string system_id;
    uint8_t level;
    std::vector<ISISInterface> interfaces;
    std::vector<std::string> redistribute;
};

struct Route {
    std::string destination;
    std::string gateway;
    std::string interface;
    uint32_t metric;
    std::string protocol;
    uint32_t age;
    bool active;
};

class FRRClient {
public:
    FRRClient();
    ~FRRClient();
    
    // Connection management
    bool connect(const std::string& host = "localhost", uint16_t port = 2605);
    void disconnect();
    bool is_connected() const;
    
    // BGP operations
    bool configure_bgp(const BGPConfig& config);
    bool start_bgp();
    bool stop_bgp();
    bool is_bgp_running() const;
    std::vector<BGPNeighbor> get_bgp_neighbors() const;
    std::vector<Route> get_bgp_routes() const;
    
    // OSPF operations
    bool configure_ospf(const OSPFConfig& config);
    bool start_ospf();
    bool stop_ospf();
    bool is_ospf_running() const;
    std::vector<OSPFInterface> get_ospf_interfaces() const;
    std::vector<Route> get_ospf_routes() const;
    
    // ISIS operations
    bool configure_isis(const ISISConfig& config);
    bool start_isis();
    bool stop_isis();
    bool is_isis_running() const;
    std::vector<ISISInterface> get_isis_interfaces() const;
    std::vector<Route> get_isis_routes() const;
    
    // General operations
    std::vector<Route> get_all_routes() const;
    std::vector<Route> get_routes_by_protocol(const std::string& protocol) const;
    bool add_static_route(const Route& route);
    bool remove_static_route(const std::string& destination);
    
    // Event handling
    void set_route_change_callback(std::function<void(const Route&, bool)> callback);
    void set_neighbor_change_callback(std::function<void(const BGPNeighbor&, bool)> callback);
    
    // Statistics
    uint64_t get_total_routes() const;
    uint64_t get_bgp_routes() const;
    uint64_t get_ospf_routes() const;
    uint64_t get_isis_routes() const;
    uint64_t get_static_routes() const;
    
private:
    std::string host_;
    uint16_t port_;
    bool connected_;
    
    // Protocol states
    std::atomic<bool> bgp_running_;
    std::atomic<bool> ospf_running_;
    std::atomic<bool> isis_running_;
    
    // Configuration
    BGPConfig bgp_config_;
    OSPFConfig ospf_config_;
    ISISConfig isis_config_;
    
    // Event callbacks
    std::function<void(const Route&, bool)> route_change_callback_;
    std::function<void(const BGPNeighbor&, bool)> neighbor_change_callback_;
    
    // Statistics
    std::atomic<uint64_t> total_routes_;
    std::atomic<uint64_t> bgp_routes_;
    std::atomic<uint64_t> ospf_routes_;
    std::atomic<uint64_t> isis_routes_;
    std::atomic<uint64_t> static_routes_;
    
    // Internal methods
    bool send_command(const std::string& command);
    std::string receive_response();
    bool execute_config_command(const std::string& command);
    bool execute_show_command(const std::string& command, std::string& output);
    
    // BGP specific
    bool configure_bgp_router(const BGPConfig& config);
    bool configure_bgp_neighbors(const std::vector<BGPNeighbor>& neighbors);
    bool configure_bgp_networks(const std::vector<std::string>& networks);
    std::vector<BGPNeighbor> parse_bgp_neighbors(const std::string& output);
    std::vector<Route> parse_bgp_routes(const std::string& output);
    
    // OSPF specific
    bool configure_ospf_router(const OSPFConfig& config);
    bool configure_ospf_interfaces(const std::vector<OSPFInterface>& interfaces);
    std::vector<OSPFInterface> parse_ospf_interfaces(const std::string& output);
    std::vector<Route> parse_ospf_routes(const std::string& output);
    
    // ISIS specific
    bool configure_isis_router(const ISISConfig& config);
    bool configure_isis_interfaces(const std::vector<ISISInterface>& interfaces);
    std::vector<ISISInterface> parse_isis_interfaces(const std::string& output);
    std::vector<Route> parse_isis_routes(const std::string& output);
    
    // Route parsing
    std::vector<Route> parse_routes(const std::string& output, const std::string& protocol);
    Route parse_route_line(const std::string& line, const std::string& protocol);
    
    // Event monitoring
    void start_event_monitoring();
    void stop_event_monitoring();
    void event_monitoring_loop();
    
    std::thread event_monitor_thread_;
    std::atomic<bool> stop_event_monitoring_;
    std::mutex event_mutex_;
    std::condition_variable event_cv_;
};

} // namespace frr
} // namespace router_sim
