#pragma once

#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <atomic>
#include <thread>
#include <chrono>
#include <functional>
#include <memory>

namespace router_sim {

struct RouteInfo {
    std::string prefix;
    uint8_t prefix_length = 0;
    std::string next_hop;
    uint32_t metric = 0;
    std::string protocol;
    std::chrono::system_clock::time_point timestamp;
};

struct NeighborInfo {
    std::string address;
    std::string state;
    std::string protocol;
    std::chrono::system_clock::time_point established_time;
};

struct BGPRoute {
    std::string prefix;
    uint8_t prefix_length;
    uint32_t metric;
    std::string next_hop;
    bool is_valid;
    std::map<std::string, std::string> attributes;
    std::chrono::system_clock::time_point timestamp;
};

struct BGPNeighbor {
    std::string address;
    uint32_t as_number;
    std::string state;
    uint32_t hold_time;
    uint32_t keepalive_interval;
    uint64_t messages_sent;
    uint64_t messages_received;
    std::string last_error;
    std::string password;
    std::chrono::system_clock::time_point established_time;
    std::chrono::system_clock::time_point last_keepalive_sent;
    std::chrono::system_clock::time_point last_keepalive_received;
    uint64_t hello_sent = 0;
    uint64_t hello_received = 0;
};

struct BGPConfig {
    std::map<std::string, std::string> parameters;
    bool enabled;
    uint32_t update_interval_ms;
    uint32_t local_as;
    std::string router_id;
    std::vector<std::string> neighbors;
    std::map<std::string, uint32_t> neighbor_as;
    uint32_t hold_time;
    uint32_t keepalive_interval;
    bool enable_graceful_restart;
    uint32_t graceful_restart_time;
    uint32_t stale_route_time;
};

struct BGPStatistics {
    uint64_t messages_sent;
    uint64_t messages_received;
    uint64_t routes_advertised;
    uint64_t routes_withdrawn;
    uint64_t routes_learned;
    uint64_t neighbors_established;
    uint64_t neighbors_failed;
};

struct ProtocolConfig {
    std::map<std::string, std::string> parameters;
    bool enabled;
    uint32_t update_interval_ms;
};

using RouteUpdateCallback = std::function<void(const RouteInfo&, bool)>;
using NeighborCallback = std::function<void(const NeighborInfo&, bool)>;

class BGPProtocol {
public:
    BGPProtocol();
    ~BGPProtocol();

    bool initialize(const ProtocolConfig& config);
    bool start();
    bool stop();
    bool is_running() const;

    // Route management
    std::vector<std::string> get_advertised_routes() const;
    std::vector<std::string> get_learned_routes() const;
    bool advertise_route(const std::string& prefix, uint8_t prefix_length, 
                        uint32_t metric, const std::map<std::string, std::string>& attributes = {});
    bool withdraw_route(const std::string& prefix, uint8_t prefix_length);

    // Neighbor management
    bool add_neighbor(const std::string& address, const std::map<std::string, std::string>& config);
    bool remove_neighbor(const std::string& address);
    std::vector<BGPNeighbor> get_neighbors() const;
    BGPNeighbor get_neighbor(const std::string& address) const;

    // Route and neighbor info
    std::vector<RouteInfo> get_routes() const;
    std::vector<NeighborInfo> get_neighbors() const;

    // Statistics
    std::map<std::string, uint64_t> get_statistics() const;

    // Configuration
    void update_configuration(const ProtocolConfig& config);
    ProtocolConfig get_configuration() const;

    // Callbacks
    void set_route_update_callback(RouteUpdateCallback callback);
    void set_neighbor_update_callback(NeighborCallback callback);

private:
    // Thread functions
    void bgp_main_loop();
    void neighbor_management_loop();
    void route_processing_loop();
    void keepalive_loop();

    // BGP operations
    bool establish_session(const std::string& neighbor_address);
    bool send_open_message(const std::string& neighbor_address);
    bool send_keepalive(const std::string& neighbor_address);
    bool send_update_message(const std::string& neighbor_address, 
                           const std::vector<BGPRoute>& routes);

    // Message processing
    void process_incoming_messages();
    void update_neighbor_states();
    void process_route_updates();
    void apply_route_policies();

    // Configuration and state
    BGPConfig config_;
    std::string next_hop_;
    std::atomic<bool> running_;
    mutable std::mutex config_mutex_;
    mutable std::mutex routes_mutex_;
    mutable std::mutex neighbors_mutex_;
    mutable std::mutex policies_mutex_;
    mutable std::mutex stats_mutex_;

    // Data structures
    std::map<std::string, BGPRoute> advertised_routes_;
    std::map<std::string, BGPRoute> learned_routes_;
    std::map<std::string, BGPNeighbor> neighbors_;
    std::map<std::string, std::string> export_policies_;
    std::map<std::string, std::string> import_policies_;

    // Statistics
    BGPStatistics stats_;

    // Threads
    std::thread bgp_thread_;
    std::thread neighbor_thread_;
    std::thread route_thread_;
    std::thread keepalive_thread_;

    // Callbacks
    RouteUpdateCallback route_update_callback_;
    NeighborCallback neighbor_update_callback_;
};

} // namespace router_sim
