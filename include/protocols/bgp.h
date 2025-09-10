#pragma once

#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <atomic>
#include <thread>
#include <functional>
#include <cstdint>

namespace router_sim {

struct ProtocolConfig {
    std::map<std::string, std::string> parameters;
    bool enabled = false;
    uint32_t update_interval_ms = 1000;
};

struct BGPRoute {
    std::string prefix;
    uint8_t prefix_length;
    uint32_t metric;
    bool is_valid;
    std::string next_hop;
    std::string origin;
    std::vector<uint32_t> as_path;
    std::map<std::string, std::string> communities;
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
    std::chrono::steady_clock::time_point last_seen;
};

class BGPProtocol {
public:
    BGPProtocol();
    ~BGPProtocol();

    // Protocol interface
    bool initialize(const ProtocolConfig& config);
    bool start();
    bool stop();
    bool is_running() const;
    
    // Route management
    std::vector<std::string> get_advertised_routes() const;
    std::vector<std::string> get_learned_routes() const;
    bool advertise_route(const std::string& prefix, uint8_t prefix_length, uint32_t metric = 0);
    bool withdraw_route(const std::string& prefix, uint8_t prefix_length);
    
    // Configuration
    void update_configuration(const ProtocolConfig& config);
    ProtocolConfig get_configuration() const;
    
    // Callbacks
    using RouteUpdateCallback = std::function<void(const std::string&, const BGPRoute&)>;
    using NeighborCallback = std::function<void(const std::string&, const BGPNeighbor&)>;
    void set_route_update_callback(RouteUpdateCallback callback);
    void set_neighbor_callback(NeighborCallback callback);
    
    // Neighbor management
    bool add_neighbor(const std::string& address, uint32_t as_number);
    bool remove_neighbor(const std::string& address);
    std::vector<BGPNeighbor> get_neighbors() const;
    BGPNeighbor get_neighbor(const std::string& address) const;
    
    // Policy management
    bool set_export_policy(const std::string& policy_name, const std::string& policy_definition);
    bool set_import_policy(const std::string& policy_name, const std::string& policy_definition);
    
    // Route information
    std::vector<BGPRoute> get_bgp_routes() const;
    bool apply_route_policy(const std::string& policy_name, BGPRoute& route);

private:
    struct BGPConfig {
        std::map<std::string, std::string> parameters;
        bool enabled = false;
        uint32_t update_interval_ms = 1000;
        uint32_t local_as = 0;
        std::string router_id;
        std::vector<std::string> neighbors;
        std::map<std::string, uint32_t> neighbor_as;
        bool enable_graceful_restart = false;
        uint32_t hold_time = 180;
        uint32_t keepalive_interval = 60;
    };

    // Thread functions
    void bgp_main_loop();
    void neighbor_management_loop();
    void route_processing_loop();
    
    // BGP message handling
    bool establish_session(const std::string& neighbor_address);
    bool send_open_message(const std::string& neighbor_address);
    bool send_keepalive(const std::string& neighbor_address);
    bool send_update_message(const std::string& neighbor_address, const std::vector<BGPRoute>& routes);
    
    // Message processing
    void process_bgp_message(const std::string& neighbor_address, const std::vector<uint8_t>& message);
    void process_open_message(const std::string& neighbor_address, const std::vector<uint8_t>& message);
    void process_update_message(const std::string& neighbor_address, const std::vector<uint8_t>& message);
    void process_notification_message(const std::string& neighbor_address, const std::vector<uint8_t>& message);

    // State
    std::atomic<bool> running_;
    mutable std::mutex config_mutex_;
    mutable std::mutex routes_mutex_;
    mutable std::mutex neighbors_mutex_;
    mutable std::mutex policies_mutex_;
    
    BGPConfig config_;
    std::map<std::string, BGPRoute> advertised_routes_;
    std::map<std::string, BGPRoute> learned_routes_;
    std::map<std::string, BGPNeighbor> neighbors_;
    std::map<std::string, std::string> export_policies_;
    std::map<std::string, std::string> import_policies_;
    
    // Callbacks
    RouteUpdateCallback route_update_callback_;
    NeighborCallback neighbor_callback_;
    
    // Threads
    std::thread bgp_thread_;
    std::thread neighbor_thread_;
    std::thread route_thread_;
};

} // namespace router_sim
