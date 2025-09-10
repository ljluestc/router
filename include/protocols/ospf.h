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

// OSPF-specific structures
struct OSPFRoute {
    std::string destination;
    uint8_t prefix_length;
    std::string next_hop;
    std::string area_id;
    uint32_t metric;
    uint8_t type; // 1=Intra-area, 2=Inter-area, 3=External
    bool is_valid;
    std::chrono::steady_clock::time_point last_updated;
};

struct OSPFNeighbor {
    std::string router_id;
    std::string state;
    std::string interface;
    std::string area_id;
    uint32_t priority;
    uint32_t cost;
    uint32_t hello_interval;
    uint32_t dead_interval;
    std::string dr; // Designated Router
    std::string bdr; // Backup Designated Router
    uint32_t mtu;
    std::chrono::steady_clock::time_point last_hello;
    std::map<std::string, std::string> capabilities;
};

struct OSPFConfig {
    std::string router_id;
    std::string area_id;
    uint32_t hello_interval;
    uint32_t dead_interval;
    uint32_t retransmit_interval;
    uint32_t lsa_refresh_interval;
    bool enable_graceful_restart;
    bool stub_router;
    std::vector<std::string> interfaces;
    std::map<std::string, std::string> interface_costs;
    std::map<std::string, std::string> interface_areas;
    std::map<std::string, std::string> interface_priorities;
};

// Callback types
using RouteUpdateCallback = std::function<void(const RouteInfo&, bool)>;
using NeighborUpdateCallback = std::function<void(const NeighborInfo&, bool)>;

class OSPFProtocol {
public:
    OSPFProtocol();
    ~OSPFProtocol();

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
    bool advertise_network(const std::string& network, const std::string& mask, const std::string& area_id = "");
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

    // OSPF-specific methods
    std::vector<OSPFNeighbor> get_ospf_neighbors() const;
    std::vector<OSPFRoute> get_ospf_routes() const;
    OSPFNeighbor get_neighbor(const std::string& router_id) const;

private:
    // Internal state
    std::atomic<bool> running_;
    OSPFConfig config_;
    mutable std::mutex config_mutex_;
    mutable std::mutex routes_mutex_;
    mutable std::mutex neighbors_mutex_;
    mutable std::mutex policies_mutex_;

    // Route storage
    std::map<std::string, OSPFRoute> advertised_routes_;
    std::map<std::string, OSPFRoute> learned_routes_;

    // Neighbor storage
    std::map<std::string, OSPFNeighbor> neighbors_;

    // Policy storage
    std::map<std::string, std::string> export_policies_;
    std::map<std::string, std::string> import_policies_;

    // Threading
    std::thread ospf_thread_;
    std::thread neighbor_thread_;
    std::thread route_thread_;
    std::thread lsa_thread_;
    std::thread spf_thread_;

    // Callbacks
    RouteUpdateCallback route_update_callback_;
    NeighborUpdateCallback neighbor_callback_;

    // Statistics
    ProtocolStatistics statistics_;

    // Internal methods
    void ospf_main_loop();
    void neighbor_management_loop();
    void route_processing_loop();
    void lsa_generation_loop();
    void spf_calculation_loop();

    // OSPF message handling
    bool send_hello_message(const std::string& interface);
    bool send_lsa_update(const std::string& neighbor_address, const std::vector<uint8_t>& lsa);
    bool send_lsa_ack(const std::string& neighbor_address, const std::vector<uint8_t>& lsa);
    void process_hello_message(const std::string& neighbor_address, const std::vector<uint8_t>& message);
    void process_lsa_update(const std::string& neighbor_address, const std::vector<uint8_t>& message);
    void process_lsa_ack(const std::string& neighbor_address, const std::vector<uint8_t>& message);

    // LSA management
    void generate_router_lsa();
    void generate_network_lsa();
    void generate_summary_lsa();
    void process_lsa_database();
    void age_lsas();

    // SPF calculation
    void calculate_shortest_path_tree();
    void update_routing_table();

    // Adjacency management
    bool establish_adjacency(const std::string& neighbor_address);
    bool maintain_adjacency(const std::string& neighbor_address);
    void update_neighbor_state(const std::string& router_id, const std::string& new_state);

    // LSA flooding
    void flood_lsa(const std::vector<uint8_t>& lsa);
};

} // namespace router_sim
