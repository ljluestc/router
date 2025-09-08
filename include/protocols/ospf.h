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

// OSPF-specific structures
struct OSPFNeighbor {
    std::string address;
    std::string interface;
    std::string state;
    std::string area_id;
    uint32_t priority;
    uint32_t dead_interval;
    uint32_t hello_interval;
    std::chrono::steady_clock::time_point last_hello;
    std::map<std::string, std::string> capabilities;
    
    OSPFNeighbor() : priority(1), dead_interval(40), hello_interval(10) {
        last_hello = std::chrono::steady_clock::now();
    }
};

struct OSPFRoute {
    std::string destination;
    uint8_t prefix_length;
    std::string next_hop;
    std::string area_id;
    uint32_t cost;
    uint32_t type; // 1=Intra-area, 2=Inter-area, 3=External
    bool is_valid;
    std::chrono::steady_clock::time_point last_updated;
    
    OSPFRoute() : prefix_length(0), cost(0), type(0), is_valid(false) {
        last_updated = std::chrono::steady_clock::now();
    }
};

struct OSPFConfig {
    std::string router_id;
    std::string area_id;
    uint32_t hello_interval;
    uint32_t dead_interval;
    uint32_t retransmit_interval;
    uint32_t transit_delay;
    bool enable_graceful_restart;
    std::vector<std::string> interfaces;
    std::map<std::string, std::string> interface_costs;
    std::map<std::string, std::string> interface_areas;
    
    OSPFConfig() : hello_interval(10), dead_interval(40), 
                   retransmit_interval(5), transit_delay(1),
                   enable_graceful_restart(false) {}
};

// OSPF Protocol Implementation
class OSPFProtocol : public ProtocolInterface {
public:
    OSPFProtocol();
    ~OSPFProtocol();

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

    // OSPF-specific methods
    bool add_interface(const std::string& interface, const std::map<std::string, std::string>& config);
    bool remove_interface(const std::string& interface);
    bool advertise_network(const std::string& network, const std::string& mask, const std::string& area_id);
    bool withdraw_network(const std::string& network, const std::string& mask);
    
    std::vector<OSPFNeighbor> get_ospf_neighbors() const;
    std::vector<OSPFRoute> get_ospf_routes() const;
    OSPFNeighbor get_neighbor(const std::string& address) const;
    
    bool set_export_policy(const std::string& policy_name, const std::string& policy_definition);
    bool set_import_policy(const std::string& policy_name, const std::string& policy_definition);

private:
    // OSPF state machine
    void ospf_main_loop();
    void neighbor_management_loop();
    void route_processing_loop();
    void lsa_generation_loop();
    void spf_calculation_loop();

    // OSPF message processing
    bool send_hello_message(const std::string& interface);
    bool send_database_description(const std::string& neighbor_address);
    bool send_link_state_request(const std::string& neighbor_address);
    bool send_link_state_update(const std::string& neighbor_address);
    bool send_link_state_acknowledgment(const std::string& neighbor_address);

    // LSA processing
    void process_hello_message(const std::string& neighbor_address, const std::vector<uint8_t>& message);
    void process_database_description(const std::string& neighbor_address, const std::vector<uint8_t>& message);
    void process_link_state_request(const std::string& neighbor_address, const std::vector<uint8_t>& message);
    void process_link_state_update(const std::string& neighbor_address, const std::vector<uint8_t>& message);
    void process_link_state_acknowledgment(const std::string& neighbor_address, const std::vector<uint8_t>& message);

    // SPF calculation
    void calculate_shortest_path_tree();
    void update_routing_table();
    void flood_lsa(const std::vector<uint8_t>& lsa);

    // Neighbor state management
    void update_neighbor_state(const std::string& address, const std::string& new_state);
    bool establish_adjacency(const std::string& neighbor_address);
    bool maintain_adjacency(const std::string& neighbor_address);

    // State
    std::atomic<bool> running_;
    OSPFConfig config_;
    mutable std::mutex config_mutex_;
    mutable std::mutex neighbors_mutex_;
    mutable std::mutex routes_mutex_;
    mutable std::mutex policies_mutex_;

    // Neighbors and routes
    std::map<std::string, OSPFNeighbor> neighbors_;
    std::map<std::string, OSPFRoute> advertised_routes_;
    std::map<std::string, OSPFRoute> learned_routes_;

    // Policies
    std::map<std::string, std::string> export_policies_;
    std::map<std::string, std::string> import_policies_;

    // Statistics
    ProtocolStatistics statistics_;

    // Threading
    std::thread ospf_thread_;
    std::thread neighbor_thread_;
    std::thread route_thread_;
    std::thread lsa_thread_;
    std::thread spf_thread_;

    // Callbacks
    RouteUpdateCallback route_update_callback_;
    NeighborUpdateCallback neighbor_callback_;
};

} // namespace router_sim
