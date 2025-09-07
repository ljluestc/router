#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include <atomic>
#include <thread>
#include <functional>
#include <json/json.h>

namespace router_sim {

// FRR Configuration
struct FRRConfig {
    std::string zebra_socket_path = "/var/run/frr/zserv.api";
    std::string bgpd_socket_path = "/var/run/frr/bgpd.vty";
    std::string ospfd_socket_path = "/var/run/frr/ospfd.vty";
    std::string isisd_socket_path = "/var/run/frr/isisd.vty";
    bool enable_auto_reconnect = true;
    uint32_t reconnect_interval_ms = 5000;
    uint32_t connection_timeout_ms = 10000;
};

// FRR Route structure
struct FRRRoute {
    std::string destination;
    uint8_t prefix_length;
    std::string next_hop;
    std::string interface;
    uint32_t metric;
    std::string protocol;
    uint32_t distance;
    bool is_active;
};

// FRR Neighbor structure
struct FRRNeighbor {
    std::string address;
    std::string interface;
    std::string state;
    uint32_t as_number;
    uint64_t messages_sent;
    uint64_t messages_received;
    std::string last_error;
};

// FRR Statistics
struct FRRStatistics {
    uint64_t routes_learned = 0;
    uint64_t routes_advertised = 0;
    uint64_t neighbors_established = 0;
    uint64_t messages_sent = 0;
    uint64_t messages_received = 0;
    uint64_t connection_errors = 0;
};

// Callback types
using RouteUpdateCallback = std::function<void(const FRRRoute&, bool)>;
using NeighborUpdateCallback = std::function<void(const FRRNeighbor&, bool)>;
using ConnectionCallback = std::function<void(const std::string&, bool)>;

// Forward declarations
class ZMQClient;
class UnixSocketClient;

// Main FRR Client class
class FRRClient {
public:
    FRRClient();
    ~FRRClient();

    // Initialization and control
    bool initialize(const FRRConfig& config);
    bool start();
    bool stop();
    bool is_running() const;

    // Connection management
    bool connect_to_zebra();
    bool connect_to_bgpd();
    bool connect_to_ospfd();
    bool connect_to_isisd();
    bool disconnect_all();

    // Route management
    bool add_route(const FRRRoute& route);
    bool remove_route(const std::string& destination, uint8_t prefix_length);
    bool update_route(const FRRRoute& route);
    std::vector<FRRRoute> get_routes() const;
    std::vector<FRRRoute> get_routes_by_protocol(const std::string& protocol) const;

    // Neighbor management
    std::vector<FRRNeighbor> get_neighbors() const;
    std::vector<FRRNeighbor> get_neighbors_by_protocol(const std::string& protocol) const;
    bool is_neighbor_established(const std::string& address) const;

    // Protocol control
    bool enable_bgp(const std::map<std::string, std::string>& config);
    bool disable_bgp();
    bool enable_ospf(const std::map<std::string, std::string>& config);
    bool disable_ospf();
    bool enable_isis(const std::map<std::string, std::string>& config);
    bool disable_isis();

    // Statistics
    FRRStatistics get_statistics() const;

    // Callbacks
    void set_route_update_callback(RouteUpdateCallback callback);
    void set_neighbor_update_callback(NeighborUpdateCallback callback);
    void set_connection_callback(ConnectionCallback callback);

private:
    FRRConfig config_;
    std::atomic<bool> running_;
    std::thread connection_thread_;
    std::thread message_thread_;
    
    std::unique_ptr<ZMQClient> zmq_client_;
    std::unique_ptr<UnixSocketClient> unix_client_;
    
    mutable std::mutex config_mutex_;
    mutable std::mutex connections_mutex_;
    mutable std::mutex routes_mutex_;
    mutable std::mutex neighbors_mutex_;
    
    std::map<std::string, bool> connections_;
    std::map<std::string, FRRRoute> routes_;
    std::map<std::string, FRRNeighbor> neighbors_;
    FRRStatistics stats_;
    
    // Callbacks
    RouteUpdateCallback route_update_callback_;
    NeighborUpdateCallback neighbor_update_callback_;
    ConnectionCallback connection_callback_;
    
    // Internal methods
    void connection_monitor_loop();
    void message_processing_loop();
    
    bool send_zebra_message(const std::string& message);
    bool send_bgpd_message(const std::string& message);
    bool send_ospfd_message(const std::string& message);
    bool send_isisd_message(const std::string& message);
    
    void process_zebra_message(const std::string& message);
    void process_bgpd_message(const std::string& message);
    void process_ospfd_message(const std::string& message);
    void process_isisd_message(const std::string& message);
    
    bool parse_route_message(const std::string& message, FRRRoute& route);
    bool parse_neighbor_message(const std::string& message, FRRNeighbor& neighbor);
    std::string serialize_route(const FRRRoute& route) const;
    std::string serialize_neighbor(const FRRNeighbor& neighbor) const;
};

// ZMQ Client for FRR communication
class ZMQClient {
public:
    ZMQClient();
    ~ZMQClient();
    
    bool initialize(const FRRConfig& config);
    bool connect(const std::string& endpoint);
    bool disconnect();
    bool is_connected() const;
    bool send_message(const std::string& message);
    bool receive_message(std::string& message, uint32_t timeout_ms = 1000);

private:
    void* context_;
    void* socket_;
    bool connected_;
};

// Unix Socket Client for FRR communication
class UnixSocketClient {
public:
    UnixSocketClient();
    ~UnixSocketClient();
    
    bool initialize(const FRRConfig& config);
    bool connect(const std::string& socket_path);
    bool disconnect();
    bool is_connected() const;
    bool send_message(const std::string& message);
    bool receive_message(std::string& message, uint32_t timeout_ms = 1000);

private:
    int socket_fd_;
    bool connected_;
};

// FRR Integration wrapper
class FRRIntegration {
public:
    FRRIntegration();
    ~FRRIntegration();
    
    bool initialize(const struct RouterConfig& config);
    bool start();
    bool stop();
    bool is_running() const;
    
    // Protocol control
    bool enable_bgp(const std::map<std::string, std::string>& config);
    bool disable_bgp();
    bool enable_ospf(const std::map<std::string, std::string>& config);
    bool disable_ospf();
    bool enable_isis(const std::map<std::string, std::string>& config);
    bool disable_isis();
    
    // Route management
    bool add_route(const FRRRoute& route);
    bool remove_route(const std::string& destination, uint8_t prefix_length);
    std::vector<FRRRoute> get_routes() const;
    std::vector<FRRNeighbor> get_neighbors() const;
    
    // Statistics
    FRRStatistics get_statistics() const;

private:
    std::unique_ptr<FRRClient> client_;
    FRRConfig config_;
    bool initialized_;
};

} // namespace router_sim
