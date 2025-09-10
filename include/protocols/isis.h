#pragma once

#include <string>
#include <map>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <functional>

namespace router_sim {

// Forward declarations
struct RouteInfo;
struct NeighborInfo;

// IS-IS interface states
enum class ISISInterfaceState {
    DOWN,
    UP
};

// IS-IS neighbor states
enum class ISISNeighborState {
    DOWN,
    INIT,
    UP
};

// IS-IS states
enum class ISISState {
    DOWN,
    UP
};

// IS-IS neighbor information
struct ISISNeighbor {
    std::string neighbor_id;
    ISISNeighborState state;
    uint32_t priority;
    std::chrono::steady_clock::time_point last_update;
    uint32_t hello_sent;
    uint32_t hello_received;
    uint32_t lsp_sent;
    uint32_t lsp_received;
    uint32_t lsp_ack_sent;
    uint32_t lsp_ack_received;
};

// IS-IS interface information
struct ISISInterface {
    std::string name;
    std::string area_id;
    uint32_t hello_interval;
    uint32_t hold_time;
    uint32_t lsp_interval;
    uint32_t priority;
    uint32_t cost;
    ISISInterfaceState state;
    std::chrono::steady_clock::time_point last_hello;
    std::map<std::string, ISISNeighbor> neighbors;
};

// IS-IS route information
struct ISISRoute {
    std::string prefix;
    std::string area_id;
    std::string type;
    uint32_t cost;
    std::string next_hop;
    std::string advertising_router;
    std::chrono::steady_clock::time_point last_update;
    bool is_active;
};

// IS-IS statistics
struct ISISStats {
    uint64_t packets_sent;
    uint64_t packets_received;
    uint64_t hello_sent;
    uint64_t hello_received;
    uint64_t lsp_sent;
    uint64_t lsp_received;
    uint64_t lsp_ack_sent;
    uint64_t lsp_ack_received;
    uint64_t routes_advertised;
    uint64_t routes_withdrawn;
    uint64_t neighbors_up;
    uint64_t neighbors_down;
    
    void reset() {
        packets_sent = 0;
        packets_received = 0;
        hello_sent = 0;
        hello_received = 0;
        lsp_sent = 0;
        lsp_received = 0;
        lsp_ack_sent = 0;
        lsp_ack_received = 0;
        routes_advertised = 0;
        routes_withdrawn = 0;
        neighbors_up = 0;
        neighbors_down = 0;
    }
};

// IS-IS Protocol class
class ISISProtocol {
public:
    ISISProtocol();
    ~ISISProtocol();
    
    // Protocol management
    bool start(const std::map<std::string, std::string>& config);
    bool stop();
    bool is_running() const;
    
    // Interface management
    bool add_interface(const std::string& interface, const std::map<std::string, std::string>& config);
    bool remove_interface(const std::string& interface);
    
    // Route management
    bool advertise_route(const std::string& prefix, const std::map<std::string, std::string>& attributes);
    bool withdraw_route(const std::string& prefix);
    
    // Information retrieval
    std::vector<RouteInfo> get_routes() const;
    std::vector<NeighborInfo> get_neighbors() const;
    std::map<std::string, uint64_t> get_statistics() const;
    
    // Callbacks
    void set_route_update_callback(std::function<void(const RouteInfo&, bool)> callback);
    void set_neighbor_update_callback(std::function<void(const NeighborInfo&, bool)> callback);
    
private:
    // IS-IS processing
    void isis_processing_loop();
    void process_isis_state_machine();
    void send_hello_packets();
    void process_incoming_messages();
    void check_dead_neighbors();
    
    // IS-IS state machine
    void maintain_isis_interface(const std::string& interface_name, ISISInterface& interface);
    
    // Interface management
    bool bring_interface_up(const std::string& interface);
    
    // IS-IS message handling
    bool send_hello_packet(const std::string& interface);
    
    // Utility functions
    std::string neighbor_state_to_string(ISISNeighborState state) const;
    
    // Configuration
    std::string system_id_;
    std::string area_id_;
    uint32_t hello_interval_;
    uint32_t hold_time_;
    uint32_t lsp_interval_;
    
    // State
    bool running_;
    ISISState state_;
    std::thread isis_thread_;
    
    // Data structures
    std::map<std::string, ISISInterface> interfaces_;
    std::map<std::string, ISISRoute> routes_;
    
    // Statistics
    mutable std::mutex stats_mutex_;
    ISISStats stats_;
    
    // Callbacks
    std::function<void(const RouteInfo&, bool)> route_update_callback_;
    std::function<void(const NeighborInfo&, bool)> neighbor_update_callback_;
};

} // namespace router_sim
