#pragma once

#include "protocol_interface.h"
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>

namespace router_sim {

// BGP-specific structures
struct BGPNeighbor {
    std::string address;
    uint32_t as_number;
    std::string state;
    uint32_t hold_time;
    uint32_t keepalive_interval;
    uint64_t messages_sent;
    uint64_t messages_received;
    std::string last_error;
    std::chrono::steady_clock::time_point last_hello;
    std::map<std::string, std::string> capabilities;
    std::map<std::string, std::string> attributes;
    
    BGPNeighbor() : as_number(0), hold_time(180), keepalive_interval(60),
                    messages_sent(0), messages_received(0) {
        last_hello = std::chrono::steady_clock::now();
    }
};

struct BGPRoute {
    std::string prefix;
    uint8_t prefix_length;
    uint32_t metric;
    std::string next_hop;
    std::string origin;
    std::vector<uint32_t> as_path;
    std::map<std::string, std::string> communities;
    bool is_valid;
    std::chrono::steady_clock::time_point last_updated;
    
    BGPRoute() : prefix_length(0), metric(0), is_valid(false) {
        last_updated = std::chrono::steady_clock::now();
    }
};

struct BGPConfig {
    uint32_t local_as;
    std::string router_id;
    bool enable_graceful_restart;
    uint32_t hold_time;
    uint32_t keepalive_interval;
    std::vector<std::string> neighbors;
    std::map<std::string, uint32_t> neighbor_as;
    std::map<std::string, std::string> neighbor_configs;
    
    BGPConfig() : local_as(0), enable_graceful_restart(false), 
                  hold_time(180), keepalive_interval(60) {}
};

// BGP Protocol Implementation
class BGPProtocol : public ProtocolInterface {
public:
    BGPProtocol();
    ~BGPProtocol();

    // ProtocolInterface implementation
    bool initialize(const std::map<std::string, std::string>& config) override;
    bool start() override;
    bool stop() override;
    bool is_running() const override;

    // Neighbor management
    bool add_neighbor(const std::string& address, const std::map<std::string, std::string>& config) override;
    bool remove_neighbor(const std::string& address) override;
    std::vector<NeighborInfo> get_neighbors() const override;
    bool is_neighbor_established(const std::string& address) const override;

    // Route management
    bool advertise_route(const RouteInfo& route) override;
    bool withdraw_route(const std::string& destination, uint8_t prefix_length) override;
    std::vector<RouteInfo> get_routes() const override;

    // Configuration
    bool update_config(const std::map<std::string, std::string>& config) override;
    std::map<std::string, std::string> get_config() const override;

    // Statistics
    ProtocolStatistics get_statistics() const override;

    // Event callbacks
    void set_route_update_callback(RouteUpdateCallback callback) override;
    void set_neighbor_update_callback(NeighborUpdateCallback callback) override;

    // BGP-specific methods
    bool advertise_route(const std::string& prefix, uint8_t prefix_length, uint32_t metric);
    bool withdraw_route(const std::string& prefix, uint8_t prefix_length);
    bool add_neighbor(const std::string& address, uint32_t as_number);
    bool remove_neighbor(const std::string& address);
    
    std::vector<BGPNeighbor> get_bgp_neighbors() const;
    std::vector<BGPRoute> get_bgp_routes() const;
    BGPNeighbor get_neighbor(const std::string& address) const;
    
    bool set_export_policy(const std::string& policy_name, const std::string& policy_definition);
    bool set_import_policy(const std::string& policy_name, const std::string& policy_definition);

private:
    // BGP state machine
    void bgp_main_loop();
    void neighbor_management_loop();
    void route_processing_loop();

    // BGP message processing
    bool send_open_message(const std::string& neighbor_address);
    bool send_keepalive(const std::string& neighbor_address);
    bool send_update_message(const std::string& neighbor_address, const std::vector<BGPRoute>& routes);
    bool send_notification_message(const std::string& neighbor_address, uint8_t error_code, uint8_t error_subcode);

    // Message handlers
    void process_bgp_message(const std::string& neighbor_address, const std::vector<uint8_t>& message);
    void process_open_message(const std::string& neighbor_address, const std::vector<uint8_t>& message);
    void process_update_message(const std::string& neighbor_address, const std::vector<uint8_t>& message);
    void process_notification_message(const std::string& neighbor_address, const std::vector<uint8_t>& message);

    // Session management
    bool establish_session(const std::string& neighbor_address);
    bool maintain_session(const std::string& neighbor_address);
    void close_session(const std::string& neighbor_address);

    // Route processing
    void process_route_advertisement(const std::string& neighbor_address, const BGPRoute& route);
    void process_route_withdrawal(const std::string& neighbor_address, const std::string& prefix, uint8_t prefix_length);
    bool apply_route_policy(const std::string& policy_name, BGPRoute& route);

    // State
    std::atomic<bool> running_;
    BGPConfig config_;
    mutable std::mutex config_mutex_;
    mutable std::mutex neighbors_mutex_;
    mutable std::mutex routes_mutex_;
    mutable std::mutex policies_mutex_;

    // Neighbors and routes
    std::map<std::string, BGPNeighbor> neighbors_;
    std::map<std::string, BGPRoute> advertised_routes_;
    std::map<std::string, BGPRoute> learned_routes_;

    // Policies
    std::map<std::string, std::string> export_policies_;
    std::map<std::string, std::string> import_policies_;

    // Statistics
    ProtocolStatistics statistics_;

    // Threading
    std::thread bgp_thread_;
    std::thread neighbor_thread_;
    std::thread route_thread_;

    // Callbacks
    RouteUpdateCallback route_update_callback_;
    NeighborUpdateCallback neighbor_callback_;
};

} // namespace router_sim
