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

// IS-IS-specific structures
struct ISISNeighbor {
    std::string system_id;
    std::string interface;
    std::string state;
    std::string level;
    std::string area_id;
    uint32_t priority;
    uint32_t hold_time;
    std::chrono::steady_clock::time_point last_hello;
    std::map<std::string, std::string> capabilities;
    std::map<std::string, std::string> attributes;
    
    ISISNeighbor() : priority(64), hold_time(30) {
        last_hello = std::chrono::steady_clock::now();
    }
};

struct ISISRoute {
    std::string destination;
    uint8_t prefix_length;
    std::string next_hop;
    std::string level;
    uint32_t metric;
    uint32_t type; // 1=Internal, 2=External
    bool is_valid;
    std::chrono::steady_clock::time_point last_updated;
    
    ISISRoute() : prefix_length(0), metric(0), type(0), is_valid(false) {
        last_updated = std::chrono::steady_clock::now();
    }
};

struct ISISConfig {
    std::string system_id;
    std::string area_id;
    std::string level; // "1", "2", or "1-2"
    uint32_t hello_interval;
    uint32_t hold_time;
    uint32_t retransmit_interval;
    bool enable_graceful_restart;
    std::vector<std::string> interfaces;
    std::map<std::string, std::string> interface_metrics;
    std::map<std::string, std::string> interface_levels;
    
    ISISConfig() : hello_interval(10), hold_time(30), 
                   retransmit_interval(5), enable_graceful_restart(false) {}
};

// IS-IS Protocol Implementation
class ISISProtocol : public ProtocolInterface {
public:
    ISISProtocol();
    ~ISISProtocol();

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

    // IS-IS-specific methods
    bool add_interface(const std::string& interface, const std::map<std::string, std::string>& config);
    bool remove_interface(const std::string& interface);
    bool advertise_network(const std::string& network, const std::string& mask, const std::string& level);
    bool withdraw_network(const std::string& network, const std::string& mask);
    
    std::vector<ISISNeighbor> get_isis_neighbors() const;
    std::vector<ISISRoute> get_isis_routes() const;
    ISISNeighbor get_neighbor(const std::string& system_id) const;
    
    bool set_export_policy(const std::string& policy_name, const std::string& policy_definition);
    bool set_import_policy(const std::string& policy_name, const std::string& policy_definition);

private:
    // IS-IS state machine
    void isis_main_loop();
    void neighbor_management_loop();
    void route_processing_loop();
    void lsp_generation_loop();
    void spf_calculation_loop();

    // IS-IS message processing
    bool send_hello_message(const std::string& interface, const std::string& level);
    bool send_lsp(const std::string& neighbor_address, const std::vector<uint8_t>& lsp);
    bool send_psnp(const std::string& neighbor_address, const std::vector<uint8_t>& psnp);
    bool send_csnp(const std::string& neighbor_address, const std::vector<uint8_t>& csnp);

    // LSP processing
    void process_hello_message(const std::string& neighbor_address, const std::vector<uint8_t>& message);
    void process_lsp(const std::string& neighbor_address, const std::vector<uint8_t>& message);
    void process_psnp(const std::string& neighbor_address, const std::vector<uint8_t>& message);
    void process_csnp(const std::string& neighbor_address, const std::vector<uint8_t>& message);

    // SPF calculation
    void calculate_shortest_path_tree();
    void update_routing_table();
    void flood_lsp(const std::vector<uint8_t>& lsp);

    // Neighbor state management
    void update_neighbor_state(const std::string& system_id, const std::string& new_state);
    bool establish_adjacency(const std::string& neighbor_address);
    bool maintain_adjacency(const std::string& neighbor_address);

    // LSP management
    void generate_lsp();
    void process_lsp_database();
    void age_lsps();

    // State
    std::atomic<bool> running_;
    ISISConfig config_;
    mutable std::mutex config_mutex_;
    mutable std::mutex neighbors_mutex_;
    mutable std::mutex routes_mutex_;
    mutable std::mutex policies_mutex_;
    mutable std::mutex lsp_mutex_;

    // Neighbors and routes
    std::map<std::string, ISISNeighbor> neighbors_;
    std::map<std::string, ISISRoute> advertised_routes_;
    std::map<std::string, ISISRoute> learned_routes_;

    // LSP database
    std::map<std::string, std::vector<uint8_t>> lsp_database_;

    // Policies
    std::map<std::string, std::string> export_policies_;
    std::map<std::string, std::string> import_policies_;

    // Statistics
    ProtocolStatistics statistics_;

    // Threading
    std::thread isis_thread_;
    std::thread neighbor_thread_;
    std::thread route_thread_;
    std::thread lsp_thread_;
    std::thread spf_thread_;

    // Callbacks
    RouteUpdateCallback route_update_callback_;
    NeighborUpdateCallback neighbor_callback_;
};

} // namespace router_sim
