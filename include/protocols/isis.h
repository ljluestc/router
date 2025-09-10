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

// IS-IS-specific structures
struct ISISRoute {
    std::string destination;
    uint8_t prefix_length;
    std::string next_hop;
    std::string level;
    uint32_t metric;
    uint8_t type; // 1=Internal, 2=External
    bool is_valid;
    std::chrono::steady_clock::time_point last_updated;
};

struct ISISNeighbor {
    std::string system_id;
    std::string state;
    std::string interface;
    std::string area_id;
    std::string level;
    uint32_t priority;
    uint32_t hold_time;
    std::chrono::steady_clock::time_point last_hello;
    std::map<std::string, std::string> capabilities;
};

struct ISISConfig {
    std::string system_id;
    std::string area_id;
    std::string level;
    uint32_t hello_interval;
    uint32_t hold_time;
    uint32_t retransmit_interval;
    bool enable_graceful_restart;
    std::vector<std::string> interfaces;
    std::map<std::string, std::string> interface_metrics;
    std::map<std::string, std::string> interface_levels;
};

// Callback types
using RouteUpdateCallback = std::function<void(const RouteInfo&, bool)>;
using NeighborUpdateCallback = std::function<void(const NeighborInfo&, bool)>;

class ISISProtocol {
public:
    ISISProtocol();
    ~ISISProtocol();

    // Core protocol management
    bool initialize(const std::map<std::string, std::string>& config);
    bool start();
    bool stop();
    bool is_running() const;

    // Route management
    bool advertise_route(const RouteInfo& route);
    bool withdraw_route(const std::string& destination, uint8_t prefix_length);
    std::vector<RouteInfo> get_routes() const;

    // Neighbor management
    bool add_neighbor(const std::string& address, const std::map<std::string, std::string>& config);
    bool remove_neighbor(const std::string& address);
    std::vector<NeighborInfo> get_neighbors() const;
    bool is_neighbor_established(const std::string& address) const;

    // Interface management
    bool add_interface(const std::string& interface, const std::map<std::string, std::string>& config);
    bool remove_interface(const std::string& interface);

    // Network management
    bool advertise_network(const std::string& network, const std::string& mask, const std::string& level = "");
    bool withdraw_network(const std::string& network, const std::string& mask);

    // Configuration
    bool update_config(const std::map<std::string, std::string>& config);
    std::map<std::string, std::string> get_config() const;

    // Policy management
    bool set_export_policy(const std::string& policy_name, const std::string& policy_definition);
    bool set_import_policy(const std::string& policy_name, const std::string& policy_definition);

    // Callbacks
    void set_route_update_callback(RouteUpdateCallback callback);
    void set_neighbor_update_callback(NeighborUpdateCallback callback);

    // Statistics
    ProtocolStatistics get_statistics() const;

    // IS-IS-specific methods
    std::vector<ISISNeighbor> get_isis_neighbors() const;
    std::vector<ISISRoute> get_isis_routes() const;
    ISISNeighbor get_neighbor(const std::string& system_id) const;

private:
    // Internal state
    std::atomic<bool> running_;
    ISISConfig config_;
    mutable std::mutex config_mutex_;
    mutable std::mutex routes_mutex_;
    mutable std::mutex neighbors_mutex_;
    mutable std::mutex policies_mutex_;

    // Route storage
    std::map<std::string, ISISRoute> advertised_routes_;
    std::map<std::string, ISISRoute> learned_routes_;

    // Neighbor storage
    std::map<std::string, ISISNeighbor> neighbors_;

    // Policy storage
    std::map<std::string, std::string> export_policies_;
    std::map<std::string, std::string> import_policies_;

    // Threading
    std::thread isis_thread_;
    std::thread neighbor_thread_;
    std::thread route_thread_;
    std::thread lsp_thread_;
    std::thread spf_thread_;

    // Callbacks
    RouteUpdateCallback route_update_callback_;
    NeighborUpdateCallback neighbor_callback_;

    // Statistics
    ProtocolStatistics statistics_;

    // Internal methods
    void isis_main_loop();
    void neighbor_management_loop();
    void route_processing_loop();
    void lsp_generation_loop();
    void spf_calculation_loop();

    // IS-IS message handling
    bool send_hello_message(const std::string& interface, const std::string& level);
    bool send_lsp(const std::string& neighbor_address, const std::vector<uint8_t>& lsp);
    bool send_psnp(const std::string& neighbor_address, const std::vector<uint8_t>& psnp);
    bool send_csnp(const std::string& neighbor_address, const std::vector<uint8_t>& csnp);
    void process_hello_message(const std::string& neighbor_address, const std::vector<uint8_t>& message);
    void process_lsp(const std::string& neighbor_address, const std::vector<uint8_t>& message);
    void process_psnp(const std::string& neighbor_address, const std::vector<uint8_t>& message);
    void process_csnp(const std::string& neighbor_address, const std::vector<uint8_t>& message);

    // LSP management
    void generate_lsp();
    void process_lsp_database();
    void age_lsps();

    // SPF calculation
    void calculate_shortest_path_tree();
    void update_routing_table();

    // Adjacency management
    bool establish_adjacency(const std::string& neighbor_address);
    bool maintain_adjacency(const std::string& neighbor_address);
    void update_neighbor_state(const std::string& system_id, const std::string& new_state);

    // LSP flooding
    void flood_lsp(const std::vector<uint8_t>& lsp);
};

} // namespace router_sim
