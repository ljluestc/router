#pragma once

#include "protocol_interface.h"
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <thread>
#include <atomic>
#include <mutex>
#include <functional>

namespace RouterSim {

// Forward declarations
struct RouteInfo;
struct NeighborInfo;
struct FRRConfig;

// FRR protocol integration
class FRRIntegration {
public:
    FRRIntegration();
    ~FRRIntegration();

    // Initialization
    bool initialize(const FRRConfig& config);
    bool start();
    bool stop();
    bool is_running() const;

    // Protocol management
    bool start_protocol(const std::string& protocol, const std::map<std::string, std::string>& config);
    bool stop_protocol(const std::string& protocol);
    bool is_protocol_running(const std::string& protocol) const;

    // BGP operations
    bool add_bgp_neighbor(const std::string& address, const std::map<std::string, std::string>& config);
    bool remove_bgp_neighbor(const std::string& address);
    bool advertise_bgp_route(const std::string& prefix, const std::map<std::string, std::string>& attributes);
    bool withdraw_bgp_route(const std::string& prefix);
    std::vector<RouteInfo> get_bgp_routes() const;
    std::vector<NeighborInfo> get_bgp_neighbors() const;

    // OSPF operations
    bool add_ospf_interface(const std::string& interface, const std::map<std::string, std::string>& config);
    bool remove_ospf_interface(const std::string& interface);
    bool advertise_ospf_route(const std::string& prefix, const std::map<std::string, std::string>& attributes);
    bool withdraw_ospf_route(const std::string& prefix);
    std::vector<RouteInfo> get_ospf_routes() const;
    std::vector<NeighborInfo> get_ospf_neighbors() const;

    // IS-IS operations
    bool add_isis_interface(const std::string& interface, const std::map<std::string, std::string>& config);
    bool remove_isis_interface(const std::string& interface);
    bool advertise_isis_route(const std::string& prefix, const std::map<std::string, std::string>& attributes);
    bool withdraw_isis_route(const std::string& prefix);
    std::vector<RouteInfo> get_isis_routes() const;
    std::vector<NeighborInfo> get_isis_neighbors() const;

    // Route management
    std::vector<RouteInfo> get_routes() const;
    std::vector<RouteInfo> get_routes_by_protocol(const std::string& protocol) const;
    std::vector<NeighborInfo> get_neighbors() const;
    std::vector<NeighborInfo> get_neighbors_by_protocol(const std::string& protocol) const;

    // Statistics
    std::map<std::string, uint64_t> get_protocol_statistics(const std::string& protocol) const;
    std::map<std::string, uint64_t> get_bgp_statistics() const;
    std::map<std::string, uint64_t> get_ospf_statistics() const;
    std::map<std::string, uint64_t> get_isis_statistics() const;

    // Configuration
    bool update_config(const std::map<std::string, std::string>& config);
    std::map<std::string, std::string> get_config() const;
    bool save_config() const;
    bool load_config();

    // Event callbacks
    void set_route_update_callback(std::function<void(const RouteInfo&, bool)> callback);
    void set_neighbor_update_callback(std::function<void(const NeighborInfo&, bool)> callback);

    // VTY interface
    bool execute_vty_command(const std::string& command);
    std::string get_vty_output(const std::string& command);

    // Logging
    void set_log_level(const std::string& level);
    std::vector<std::string> get_logs() const;
    void clear_logs();

private:
    // FRR daemon management
    bool initialize_frr_daemon();
    bool start_frr_daemon();
    bool stop_frr_daemon();
    bool is_frr_daemon_running() const;

    // Configuration management
    bool load_frr_config();
    bool save_frr_config() const;
    bool create_default_config() const;
    bool validate_frr_config() const;

    // VTY communication
    bool connect_to_vty();
    void disconnect_from_vty();
    bool send_vty_command(const std::string& command);
    std::string receive_vty_output();

    // Event handling
    void on_route_update(const RouteInfo& route, bool is_advertisement);
    void on_neighbor_update(const NeighborInfo& neighbor, bool is_up);

    // Logging
    void log_message(const std::string& level, const std::string& message);
    void parse_frr_logs();

    // State
    std::atomic<bool> running_;
    std::atomic<bool> daemon_running_;
    FRRConfig config_;
    
    // Protocol handlers
    std::unique_ptr<router_sim::ProtocolInterface> bgp_protocol_;
    std::unique_ptr<router_sim::ProtocolInterface> ospf_protocol_;
    std::unique_ptr<router_sim::ProtocolInterface> isis_protocol_;
    
    // Threading
    std::thread frr_daemon_thread_;
    int vty_socket_;
    
    // Synchronization
    mutable std::mutex config_mutex_;
    mutable std::mutex logs_mutex_;
    
    // Callbacks
    std::function<void(const RouteInfo&, bool)> route_update_callback_;
    std::function<void(const NeighborInfo&, bool)> neighbor_update_callback_;
    
    // Logs
    std::vector<std::string> logs_;
};

// FRR Configuration structure
struct FRRConfig {
    std::string config_file = "/etc/frr/frr.conf";
    std::string log_file = "/var/log/frr/frr.log";
    std::map<std::string, std::string> global_config;
    std::map<std::string, std::map<std::string, std::string>> protocol_configs;
    
    FRRConfig() {
        global_config["hostname"] = "router-sim";
        global_config["log"] = "syslog informational";
    }
};

} // namespace RouterSim