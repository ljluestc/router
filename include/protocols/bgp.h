#pragma once

#include <string>
#include <map>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <functional>
#include <queue>

namespace router_sim {

// Forward declarations
struct RouteInfo;
struct NeighborInfo;

// BGP neighbor states
enum class BGPNeighborState {
    IDLE,
    CONNECT,
    ACTIVE,
    OPENSENT,
    OPENCONFIRM,
    ESTABLISHED
};

// BGP states
enum class BGPState {
    IDLE,
    CONNECT,
    ACTIVE,
    OPENSENT,
    OPENCONFIRM,
    ESTABLISHED
};

// BGP neighbor information
struct BGPNeighbor {
    std::string address;
    uint32_t as_number;
    BGPNeighborState state;
    uint32_t hold_time;
    uint32_t keepalive_time;
    uint32_t connect_retry_time;
    std::chrono::steady_clock::time_point last_update;
    std::chrono::steady_clock::time_point last_keepalive;
    uint32_t packets_sent;
    uint32_t packets_received;
    uint32_t updates_sent;
    uint32_t updates_received;
    uint32_t keepalives_sent;
    uint32_t keepalives_received;
    uint32_t notifications_sent;
    uint32_t notifications_received;
};

// BGP route information
struct BGPRoute {
    std::string prefix;
    std::string next_hop;
    std::string as_path;
    std::string origin;
    uint32_t local_pref;
    uint32_t med;
    std::string community;
    std::chrono::steady_clock::time_point last_update;
    bool is_active;
};

// BGP statistics
struct BGPStats {
    uint64_t packets_sent;
    uint64_t packets_received;
    uint64_t updates_sent;
    uint64_t updates_received;
    uint64_t keepalives_sent;
    uint64_t keepalives_received;
    uint64_t notifications_sent;
    uint64_t notifications_received;
    uint64_t routes_advertised;
    uint64_t routes_withdrawn;
    uint64_t neighbors_up;
    uint64_t neighbors_down;
    
    void reset() {
        packets_sent = 0;
        packets_received = 0;
        updates_sent = 0;
        updates_received = 0;
        keepalives_sent = 0;
        keepalives_received = 0;
        notifications_sent = 0;
        notifications_received = 0;
        routes_advertised = 0;
        routes_withdrawn = 0;
        neighbors_up = 0;
        neighbors_down = 0;
    }
};

// BGP Protocol class
class BGPProtocol {
public:
    BGPProtocol();
    ~BGPProtocol();
    
    // Protocol management
    bool start(const std::map<std::string, std::string>& config);
    bool stop();
    bool is_running() const;
    
    // Neighbor management
    bool add_neighbor(const std::string& address, const std::map<std::string, std::string>& config);
    bool remove_neighbor(const std::string& address);
    
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
    // BGP processing
    void bgp_processing_loop();
    void process_bgp_state_machine();
    void send_keepalives();
    void process_incoming_messages();
    
    // BGP state machine
    void maintain_bgp_session(const std::string& address, BGPNeighbor& neighbor);
    
    // TCP connection management
    bool establish_tcp_connection(const std::string& address);
    bool is_tcp_connected(const std::string& address);
    
    // BGP message handling
    bool send_open_message(const std::string& address);
    bool receive_keepalive(const std::string& address);
    bool send_keepalive_message(const std::string& address);
    
    // Utility functions
    std::string neighbor_state_to_string(BGPNeighborState state) const;
    
    // Configuration
    uint32_t as_number_;
    std::string router_id_;
    uint16_t listen_port_;
    
    // State
    bool running_;
    BGPState state_;
    std::thread bgp_thread_;
    
    // Data structures
    std::map<std::string, BGPNeighbor> neighbors_;
    std::map<std::string, BGPRoute> routes_;
    
    // Statistics
    mutable std::mutex stats_mutex_;
    BGPStats stats_;
    
    // Callbacks
    std::function<void(const RouteInfo&, bool)> route_update_callback_;
    std::function<void(const NeighborInfo&, bool)> neighbor_update_callback_;
};

} // namespace router_sim
