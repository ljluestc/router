#pragma once

#include <string>
#include <vector>
#include <map>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>
#include <functional>

namespace router_sim {

// Forward declarations
struct RouteInfo;
struct NeighborInfo;
struct ProtocolStatistics;

// BGP-specific structures
struct BGPRoute {
    std::string prefix;
    uint8_t prefix_length;
    uint32_t metric;
    std::string next_hop;
    std::string origin;
    std::vector<uint32_t> as_path;
    std::map<std::string, std::string> attributes;
    bool is_valid;
    std::chrono::steady_clock::time_point last_updated;
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
    std::chrono::steady_clock::time_point last_hello;
    std::map<std::string, std::string> capabilities;
};

struct BGPConfig {
    uint32_t local_as;
    std::string router_id;
    std::vector<std::string> neighbors;
    std::map<std::string, uint32_t> neighbor_as;
    uint32_t hold_time;
    uint32_t keepalive_interval;
    bool enable_graceful_restart;
    std::map<std::string, std::string> parameters;
    bool enabled;
    uint32_t update_interval_ms;
};

// Callback types
using RouteUpdateCallback = std::function<void(const RouteInfo&, bool)>;
using NeighborCallback = std::function<void(const NeighborInfo&, bool)>;

class BGPProtocol {
public:
    BGPProtocol();
    ~BGPProtocol();

    // Core protocol management
    bool initialize(const std::map<std::string, std::string>& config);
    bool start();
    bool stop();
    bool is_running() const;

    // Route management
    bool advertise_route(const std::string& prefix, uint8_t prefix_length, uint32_t metric);
    bool withdraw_route(const std::string& prefix, uint8_t prefix_length);
    std::vector<std::string> get_advertised_routes() const;
    std::vector<std::string> get_learned_routes() const;
    std::vector<BGPRoute> get_bgp_routes() const;

    // Neighbor management
    bool add_neighbor(const std::string& address, const std::map<std::string, std::string>& config);
    bool remove_neighbor(const std::string& address);
    std::vector<NeighborInfo> get_neighbors() const;
    BGPNeighbor get_neighbor(const std::string& address) const;

    // Configuration
    void update_configuration(const std::map<std::string, std::string>& config);
    std::map<std::string, std::string> get_configuration() const;

    // Policy management
    bool set_export_policy(const std::string& policy_name, const std::string& policy_definition);
    bool set_import_policy(const std::string& policy_name, const std::string& policy_definition);

    // Callbacks
    void set_route_update_callback(RouteUpdateCallback callback);
    void set_neighbor_callback(NeighborCallback callback);

    // Statistics
    ProtocolStatistics get_statistics() const;

private:
    // Internal state
    std::atomic<bool> running_;
    BGPConfig config_;
    mutable std::mutex config_mutex_;
    mutable std::mutex routes_mutex_;
    mutable std::mutex neighbors_mutex_;
    mutable std::mutex policies_mutex_;

    // Route storage
    std::map<std::string, BGPRoute> advertised_routes_;
    std::map<std::string, BGPRoute> learned_routes_;

    // Neighbor storage
    std::map<std::string, BGPNeighbor> neighbors_;

    // Policy storage
    std::map<std::string, std::string> export_policies_;
    std::map<std::string, std::string> import_policies_;

    // Threading
    std::thread bgp_thread_;
    std::thread neighbor_thread_;
    std::thread route_thread_;

    // Callbacks
    RouteUpdateCallback route_update_callback_;
    NeighborCallback neighbor_callback_;

    // Statistics
    ProtocolStatistics statistics_;

    // Internal methods
    void bgp_main_loop();
    void neighbor_management_loop();
    void route_processing_loop();

    // BGP message handling
    bool establish_session(const std::string& neighbor_address);
    bool send_open_message(const std::string& neighbor_address);
    bool send_keepalive(const std::string& neighbor_address);
    bool send_update_message(const std::string& neighbor_address, const std::vector<BGPRoute>& routes);
    void process_bgp_message(const std::string& neighbor_address, const std::vector<uint8_t>& message);
    void process_open_message(const std::string& neighbor_address, const std::vector<uint8_t>& message);
    void process_update_message(const std::string& neighbor_address, const std::vector<uint8_t>& message);
    void process_notification_message(const std::string& neighbor_address, const std::vector<uint8_t>& message);

    // Policy application
    bool apply_route_policy(const std::string& policy_name, BGPRoute& route);
};

} // namespace router_sim