#pragma once

#include "router_core.h"
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <thread>
#include <atomic>
#include <mutex>

namespace RouterSim {

// FRR protocol integration
class FRRIntegration {
public:
    FRRIntegration();
    ~FRRIntegration();

    // Initialization
    bool initialize();
    void shutdown();

    // BGP integration
    bool start_bgp(const std::string& as_number, const std::string& router_id);
    bool stop_bgp();
    bool add_bgp_neighbor(const std::string& neighbor_ip, 
                         const std::string& remote_as,
                         const std::string& local_as = "");
    bool remove_bgp_neighbor(const std::string& neighbor_ip);
    bool advertise_network(const std::string& network, 
                          const std::string& mask,
                          const std::string& next_hop = "");
    bool withdraw_network(const std::string& network, 
                         const std::string& mask);
    std::vector<Route> get_bgp_routes() const;
    std::vector<Neighbor> get_bgp_neighbors() const;

    // OSPF integration
    bool start_ospf(const std::string& router_id, const std::string& area_id = "0.0.0.0");
    bool stop_ospf();
    bool add_ospf_interface(const std::string& interface, 
                           const std::string& area_id = "0.0.0.0",
                           int cost = 1);
    bool remove_ospf_interface(const std::string& interface);
    bool add_ospf_network(const std::string& network, 
                         const std::string& mask,
                         const std::string& area_id = "0.0.0.0");
    std::vector<Route> get_ospf_routes() const;
    std::vector<Neighbor> get_ospf_neighbors() const;

    // IS-IS integration
    bool start_isis(const std::string& system_id, const std::string& area_id = "49.0001");
    bool stop_isis();
    bool add_isis_interface(const std::string& interface, 
                           const std::string& level = "2");
    bool remove_isis_interface(const std::string& interface);
    bool add_isis_network(const std::string& network, 
                         const std::string& mask);
    std::vector<Route> get_isis_routes() const;
    std::vector<Neighbor> get_isis_neighbors() const;

    // Route management
    bool install_route(const Route& route);
    bool uninstall_route(const std::string& destination);
    std::vector<Route> get_all_routes() const;

    // Configuration management
    bool load_config(const std::string& config_file);
    bool save_config(const std::string& config_file);
    std::string get_running_config() const;

    // Status and monitoring
    bool is_protocol_running(Protocol protocol) const;
    std::map<std::string, std::string> get_protocol_status() const;
    std::map<std::string, uint64_t> get_protocol_statistics() const;

    // Event callbacks
    using RouteUpdateCallback = std::function<void(const Route&, bool)>;
    using NeighborUpdateCallback = std::function<void(const Neighbor&, bool)>;
    using ProtocolStatusCallback = std::function<void(Protocol, bool)>;

    void set_route_update_callback(RouteUpdateCallback callback);
    void set_neighbor_update_callback(NeighborUpdateCallback callback);
    void set_protocol_status_callback(ProtocolStatusCallback callback);

private:
    // FRR daemon management
    bool start_frr_daemon();
    bool stop_frr_daemon();
    bool is_frr_running() const;

    // Protocol-specific implementations
    bool configure_bgp();
    bool configure_ospf();
    bool configure_isis();

    // VTY shell integration
    bool execute_vty_command(const std::string& command);
    std::string get_vty_output(const std::string& command);

    // Route parsing
    std::vector<Route> parse_bgp_routes(const std::string& output) const;
    std::vector<Route> parse_ospf_routes(const std::string& output) const;
    std::vector<Route> parse_isis_routes(const std::string& output) const;
    std::vector<Neighbor> parse_bgp_neighbors(const std::string& output) const;
    std::vector<Neighbor> parse_ospf_neighbors(const std::string& output) const;
    std::vector<Neighbor> parse_isis_neighbors(const std::string& output) const;

    // State
    std::atomic<bool> initialized_;
    std::atomic<bool> frr_running_;
    std::map<Protocol, bool> protocol_status_;
    std::map<Protocol, std::string> protocol_configs_;
    
    // Threading
    std::thread monitor_thread_;
    std::atomic<bool> monitor_running_;
    mutable std::mutex state_mutex_;

    // Callbacks
    RouteUpdateCallback route_update_callback_;
    NeighborUpdateCallback neighbor_update_callback_;
    ProtocolStatusCallback protocol_status_callback_;

    // Internal monitoring
    void monitor_loop();
    void process_route_updates();
    void process_neighbor_updates();
    void process_protocol_status();
};

} // namespace RouterSim