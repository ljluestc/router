#pragma once

#include "common_structures.h"
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <atomic>

namespace RouterSim {

// Forward declarations
class RouterSimulator;

// FRR integration configuration
struct FRRConfig {
    std::string config_file;
    std::string log_file;
    bool enable_debug;
    uint32_t log_level;
    std::string vty_socket;
    
    FRRConfig() : enable_debug(false), log_level(1), vty_socket("/var/run/frr/vtysh") {}
};

// BGP configuration
struct BGPConfig {
    uint32_t as_number;
    std::string router_id;
    std::vector<std::string> networks;
    std::map<std::string, std::string> neighbor_configs;
    bool enable_graceful_restart;
    uint32_t hold_time;
    uint32_t keepalive_time;
    
    BGPConfig() : as_number(0), enable_graceful_restart(false), hold_time(180), keepalive_time(60) {}
};

// OSPF configuration
struct OSPFConfig {
    std::string area_id;
    std::string router_id;
    std::vector<std::string> networks;
    std::map<std::string, std::string> interface_configs;
    bool enable_graceful_restart;
    uint32_t hello_interval;
    uint32_t dead_interval;
    
    OSPFConfig() : enable_graceful_restart(false), hello_interval(10), dead_interval(40) {}
};

// IS-IS configuration
struct ISISConfig {
    std::string system_id;
    std::string area_id;
    std::vector<std::string> networks;
    std::map<std::string, std::string> interface_configs;
    bool enable_graceful_restart;
    uint8_t level;
    uint32_t hello_interval;
    uint32_t hold_time;
    
    ISISConfig() : enable_graceful_restart(false), level(2), hello_interval(10), hold_time(30) {}
};

// FRR integration class
class FRRIntegration {
public:
    FRRIntegration();
    ~FRRIntegration();
    
    // Initialization
    bool initialize(const FRRConfig& config);
    bool initialize(const RouterSimulator& router);
    void cleanup();
    
    // BGP operations
    bool start_bgp(const BGPConfig& config);
    bool stop_bgp();
    bool is_bgp_running() const;
    bool configure_bgp_neighbor(const std::string& address, const std::map<std::string, std::string>& config);
    bool unconfigure_bgp_neighbor(const std::string& address);
    std::vector<router_sim::NeighborInfo> get_bgp_neighbors() const;
    std::vector<router_sim::RouteInfo> get_bgp_routes() const;
    std::map<std::string, uint64_t> get_bgp_stats() const;
    
    // OSPF operations
    bool start_ospf(const OSPFConfig& config);
    bool stop_ospf();
    bool is_ospf_running() const;
    bool configure_ospf_interface(const std::string& interface, const std::map<std::string, std::string>& config);
    bool unconfigure_ospf_interface(const std::string& interface);
    std::vector<router_sim::NeighborInfo> get_ospf_neighbors() const;
    std::vector<router_sim::RouteInfo> get_ospf_routes() const;
    std::map<std::string, uint64_t> get_ospf_stats() const;
    
    // IS-IS operations
    bool start_isis(const ISISConfig& config);
    bool stop_isis();
    bool is_isis_running() const;
    bool configure_isis_interface(const std::string& interface, const std::map<std::string, std::string>& config);
    bool unconfigure_isis_interface(const std::string& interface);
    std::vector<router_sim::NeighborInfo> get_isis_neighbors() const;
    std::vector<router_sim::RouteInfo> get_isis_routes() const;
    std::map<std::string, uint64_t> get_isis_stats() const;
    
    // Interface management
    bool configure_interface(const std::string& name, const InterfaceConfig& config);
    bool unconfigure_interface(const std::string& name);
    bool set_interface_up(const std::string& name, bool up);
    bool set_interface_ip(const std::string& name, const std::string& ip, const std::string& mask);
    
    // Route management
    bool add_route(const router_sim::RouteInfo& route);
    bool remove_route(const std::string& destination, uint8_t prefix_length);
    bool redistribute_route(const router_sim::RouteInfo& route, const std::string& protocol);
    
    // VTY operations
    bool execute_vty_command(const std::string& command);
    std::string get_vty_output(const std::string& command);
    bool is_vty_connected() const;
    
    // Statistics and monitoring
    std::map<std::string, uint64_t> get_global_stats() const;
    void reset_statistics();
    
    // Event callbacks
    void set_route_update_callback(std::function<void(const router_sim::RouteInfo&, bool)> callback);
    void set_neighbor_update_callback(std::function<void(const router_sim::NeighborInfo&, bool)> callback);
    
private:
    // Internal methods
    bool start_frr_daemon();
    bool stop_frr_daemon();
    bool is_frr_running() const;
    bool connect_vty();
    void disconnect_vty();
    bool send_vty_command(const std::string& command);
    std::string receive_vty_output();
    
    // Configuration management
    bool write_frr_config();
    bool load_frr_config();
    std::string generate_bgp_config(const BGPConfig& config);
    std::string generate_ospf_config(const OSPFConfig& config);
    std::string generate_isis_config(const ISISConfig& config);
    
    // State
    std::atomic<bool> initialized_;
    std::atomic<bool> frr_running_;
    std::atomic<bool> bgp_running_;
    std::atomic<bool> ospf_running_;
    std::atomic<bool> isis_running_;
    std::atomic<bool> vty_connected_;
    
    // Configuration
    FRRConfig config_;
    BGPConfig bgp_config_;
    OSPFConfig ospf_config_;
    ISISConfig isis_config_;
    
    // VTY connection
    int vty_socket_;
    std::string vty_buffer_;
    mutable std::mutex vty_mutex_;
    
    // Statistics
    std::map<std::string, uint64_t> global_stats_;
    std::map<std::string, uint64_t> bgp_stats_;
    std::map<std::string, uint64_t> ospf_stats_;
    std::map<std::string, uint64_t> isis_stats_;
    mutable std::mutex stats_mutex_;
    
    // Callbacks
    std::function<void(const router_sim::RouteInfo&, bool)> route_update_callback_;
    std::function<void(const router_sim::NeighborInfo&, bool)> neighbor_update_callback_;
    
    // Threading
    std::thread monitoring_thread_;
    std::atomic<bool> monitoring_running_;
    void monitoring_loop();
};

} // namespace RouterSim