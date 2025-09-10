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

// OSPF interface states
enum class OSPFInterfaceState {
    DOWN,
    WAITING,
    DR_OTHER,
    DR,
    BDR
};

// OSPF neighbor states
enum class OSPFNeighborState {
    DOWN,
    ATTEMPT,
    INIT,
    TWO_WAY,
    EXSTART,
    EXCHANGE,
    LOADING,
    FULL
};

// OSPF states
enum class OSPFState {
    DOWN,
    INIT,
    TWO_WAY,
    EXSTART,
    EXCHANGE,
    LOADING,
    FULL
};

// OSPF neighbor information
struct OSPFNeighbor {
    std::string neighbor_id;
    OSPFNeighborState state;
    uint32_t priority;
    std::string dr;
    std::string bdr;
    std::chrono::steady_clock::time_point last_update;
    uint32_t hello_sent;
    uint32_t hello_received;
    uint32_t lsa_sent;
    uint32_t lsa_received;
    uint32_t lsa_ack_sent;
    uint32_t lsa_ack_received;
};

// OSPF interface information
struct OSPFInterface {
    std::string name;
    std::string area_id;
    uint32_t hello_interval;
    uint32_t dead_interval;
    uint32_t retransmit_interval;
    uint32_t priority;
    uint32_t cost;
    OSPFInterfaceState state;
    std::string dr;
    std::string bdr;
    std::chrono::steady_clock::time_point last_hello;
    std::map<std::string, OSPFNeighbor> neighbors;
};

// OSPF route information
struct OSPFRoute {
    std::string prefix;
    std::string area_id;
    std::string type;
    uint32_t cost;
    std::string next_hop;
    std::string advertising_router;
    std::chrono::steady_clock::time_point last_update;
    bool is_active;
};

// OSPF statistics
struct OSPFStats {
    uint64_t packets_sent;
    uint64_t packets_received;
    uint64_t hello_sent;
    uint64_t hello_received;
    uint64_t lsa_sent;
    uint64_t lsa_received;
    uint64_t lsa_ack_sent;
    uint64_t lsa_ack_received;
    uint64_t routes_advertised;
    uint64_t routes_withdrawn;
    uint64_t neighbors_up;
    uint64_t neighbors_down;
    
    void reset() {
        packets_sent = 0;
        packets_received = 0;
        hello_sent = 0;
        hello_received = 0;
        lsa_sent = 0;
        lsa_received = 0;
        lsa_ack_sent = 0;
        lsa_ack_received = 0;
        routes_advertised = 0;
        routes_withdrawn = 0;
        neighbors_up = 0;
        neighbors_down = 0;
    }
};

// OSPF Protocol class
class OSPFProtocol {
public:
    OSPFProtocol();
    ~OSPFProtocol();
    
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
    // OSPF processing
    void ospf_processing_loop();
    void process_ospf_state_machine();
    void send_hello_packets();
    void process_incoming_messages();
    void check_dead_neighbors();
    
    // OSPF state machine
    void maintain_ospf_interface(const std::string& interface_name, OSPFInterface& interface);
    
    // Interface management
    bool bring_interface_up(const std::string& interface);
    bool perform_dr_bdr_election(const std::string& interface);
    
    // OSPF message handling
    bool send_hello_packet(const std::string& interface);
    
    // Utility functions
    std::string neighbor_state_to_string(OSPFNeighborState state) const;
    
    // Configuration
    std::string router_id_;
    std::string area_id_;
    uint32_t hello_interval_;
    uint32_t dead_interval_;
    uint32_t retransmit_interval_;
    
    // State
    bool running_;
    OSPFState state_;
    std::thread ospf_thread_;
    
    // Data structures
    std::map<std::string, OSPFInterface> interfaces_;
    std::map<std::string, OSPFRoute> routes_;
    
    // Statistics
    mutable std::mutex stats_mutex_;
    OSPFStats stats_;
    
    // Callbacks
    std::function<void(const RouteInfo&, bool)> route_update_callback_;
    std::function<void(const NeighborInfo&, bool)> neighbor_update_callback_;
};

} // namespace router_sim
