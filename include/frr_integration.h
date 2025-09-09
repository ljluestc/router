#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include <atomic>
#include <thread>
#include <functional>
#include <chrono>

namespace router_sim {

// Forward declarations
struct FRRRoute;
struct FRRNeighbor;
struct FRRConfig;

// Callback types
using RouteUpdateCallback = std::function<void(const FRRRoute&)>;
using NeighborUpdateCallback = std::function<void(const FRRNeighbor&)>;
using ConnectionCallback = std::function<void(const std::string&, bool)>;

// FRR Route structure
struct FRRRoute {
    std::string destination;
    uint8_t prefix_length;
    std::string next_hop;
    std::string interface;
    std::string protocol;
    uint32_t metric;
    uint32_t preference;
    std::chrono::system_clock::time_point timestamp;
    bool is_active;
    
    FRRRoute() : prefix_length(0), metric(0), preference(0), is_active(false) {}
};

// FRR Neighbor structure
struct FRRNeighbor {
    std::string address;
    std::string protocol;
    std::string state;
    uint32_t asn;
    std::string description;
    std::chrono::system_clock::time_point last_seen;
    uint32_t messages_sent;
    uint32_t messages_received;
    bool is_established;
    
    FRRNeighbor() : asn(0), messages_sent(0), messages_received(0), is_established(false) {}
};

// FRR Configuration
struct FRRConfig {
    std::string zebra_socket_path;
    std::string bgpd_socket_path;
    std::string ospfd_socket_path;
    std::string isisd_socket_path;
    uint32_t router_id;
    std::string hostname;
    bool enable_bgp;
    bool enable_ospf;
    bool enable_isis;
    std::map<std::string, std::string> bgp_config;
    std::map<std::string, std::string> ospf_config;
    std::map<std::string, std::string> isis_config;
    
    FRRConfig() : router_id(0), enable_bgp(false), enable_ospf(false), enable_isis(false) {}
};

// FRR Statistics
struct FRRStatistics {
    uint32_t total_routes;
    uint32_t bgp_routes;
    uint32_t ospf_routes;
    uint32_t isis_routes;
    uint32_t static_routes;
    uint32_t connected_routes;
    uint32_t total_neighbors;
    uint32_t established_neighbors;
    uint32_t messages_sent;
    uint32_t messages_received;
    std::chrono::system_clock::time_point last_update;
    
    FRRStatistics() : total_routes(0), bgp_routes(0), ospf_routes(0), isis_routes(0),
                     static_routes(0), connected_routes(0), total_neighbors(0),
                     established_neighbors(0), messages_sent(0), messages_received(0) {}
};

// Base FRR Client interface
class FRRClient {
public:
    virtual ~FRRClient() = default;
    
    // Lifecycle
    virtual bool initialize(const FRRConfig& config) = 0;
    virtual bool start() = 0;
    virtual bool stop() = 0;
    virtual bool is_running() const = 0;
    
    // Connection management
    virtual bool connect_to_zebra() = 0;
    virtual bool connect_to_bgpd() = 0;
    virtual bool connect_to_ospfd() = 0;
    virtual bool connect_to_isisd() = 0;
    virtual bool disconnect_all() = 0;
    
    // Route management
    virtual bool add_route(const FRRRoute& route) = 0;
    virtual bool remove_route(const std::string& destination, uint8_t prefix_length) = 0;
    virtual bool update_route(const FRRRoute& route) = 0;
    virtual std::vector<FRRRoute> get_routes() const = 0;
    virtual std::vector<FRRRoute> get_routes_by_protocol(const std::string& protocol) const = 0;
    
    // Neighbor management
    virtual std::vector<FRRNeighbor> get_neighbors() const = 0;
    virtual std::vector<FRRNeighbor> get_neighbors_by_protocol(const std::string& protocol) const = 0;
    virtual bool is_neighbor_established(const std::string& address) const = 0;
    
    // Protocol control
    virtual bool enable_bgp(const std::map<std::string, std::string>& config) = 0;
    virtual bool disable_bgp() = 0;
    virtual bool enable_ospf(const std::map<std::string, std::string>& config) = 0;
    virtual bool disable_ospf() = 0;
    virtual bool enable_isis(const std::map<std::string, std::string>& config) = 0;
    virtual bool disable_isis() = 0;
    
    // Statistics
    virtual FRRStatistics get_statistics() const = 0;
    
    // Callbacks
    virtual void set_route_update_callback(RouteUpdateCallback callback) = 0;
    virtual void set_neighbor_update_callback(NeighborUpdateCallback callback) = 0;
    virtual void set_connection_callback(ConnectionCallback callback) = 0;
};

// ZMQ-based FRR Client implementation
class ZMQClient : public FRRClient {
public:
    ZMQClient();
    virtual ~ZMQClient();
    
    bool initialize(const FRRConfig& config) override;
    bool start() override;
    bool stop() override;
    bool is_running() const override;
    
    bool connect_to_zebra() override;
    bool connect_to_bgpd() override;
    bool connect_to_ospfd() override;
    bool connect_to_isisd() override;
    bool disconnect_all() override;
    
    bool add_route(const FRRRoute& route) override;
    bool remove_route(const std::string& destination, uint8_t prefix_length) override;
    bool update_route(const FRRRoute& route) override;
    std::vector<FRRRoute> get_routes() const override;
    std::vector<FRRRoute> get_routes_by_protocol(const std::string& protocol) const override;
    
    std::vector<FRRNeighbor> get_neighbors() const override;
    std::vector<FRRNeighbor> get_neighbors_by_protocol(const std::string& protocol) const override;
    bool is_neighbor_established(const std::string& address) const override;
    
    bool enable_bgp(const std::map<std::string, std::string>& config) override;
    bool disable_bgp() override;
    bool enable_ospf(const std::map<std::string, std::string>& config) override;
    bool disable_ospf() override;
    bool enable_isis(const std::map<std::string, std::string>& config) override;
    bool disable_isis() override;
    
    FRRStatistics get_statistics() const override;
    
    void set_route_update_callback(RouteUpdateCallback callback) override;
    void set_neighbor_update_callback(NeighborUpdateCallback callback) override;
    void set_connection_callback(ConnectionCallback callback) override;

private:
    void* context_;
    void* socket_;
    std::string endpoint_;
    std::atomic<bool> connected_;
    std::atomic<bool> running_;
    std::thread message_thread_;
    mutable std::mutex mutex_;
    
    // Internal state
    std::map<std::string, FRRRoute> routes_;
    std::map<std::string, FRRNeighbor> neighbors_;
    FRRStatistics stats_;
    FRRConfig config_;
    
    // Callbacks
    RouteUpdateCallback route_update_callback_;
    NeighborUpdateCallback neighbor_update_callback_;
    ConnectionCallback connection_callback_;
    
    // Internal methods
    void message_processing_loop();
    bool send_message(const std::string& message);
    bool receive_message(std::string& message, uint32_t timeout_ms = 1000);
    void process_message(const std::string& message);
    bool parse_route_message(const std::string& message, FRRRoute& route);
    bool parse_neighbor_message(const std::string& message, FRRNeighbor& neighbor);
    std::string serialize_route(const FRRRoute& route) const;
    std::string serialize_neighbor(const FRRNeighbor& neighbor) const;
};

// Unix Socket-based FRR Client implementation
class UnixSocketClient : public FRRClient {
public:
    UnixSocketClient();
    virtual ~UnixSocketClient();
    
    bool initialize(const FRRConfig& config) override;
    bool start() override;
    bool stop() override;
    bool is_running() const override;
    
    bool connect_to_zebra() override;
    bool connect_to_bgpd() override;
    bool connect_to_ospfd() override;
    bool connect_to_isisd() override;
    bool disconnect_all() override;
    
    bool add_route(const FRRRoute& route) override;
    bool remove_route(const std::string& destination, uint8_t prefix_length) override;
    bool update_route(const FRRRoute& route) override;
    std::vector<FRRRoute> get_routes() const override;
    std::vector<FRRRoute> get_routes_by_protocol(const std::string& protocol) const override;
    
    std::vector<FRRNeighbor> get_neighbors() const override;
    std::vector<FRRNeighbor> get_neighbors_by_protocol(const std::string& protocol) const override;
    bool is_neighbor_established(const std::string& address) const override;
    
    bool enable_bgp(const std::map<std::string, std::string>& config) override;
    bool disable_bgp() override;
    bool enable_ospf(const std::map<std::string, std::string>& config) override;
    bool disable_ospf() override;
    bool enable_isis(const std::map<std::string, std::string>& config) override;
    bool disable_isis() override;
    
    FRRStatistics get_statistics() const override;
    
    void set_route_update_callback(RouteUpdateCallback callback) override;
    void set_neighbor_update_callback(NeighborUpdateCallback callback) override;
    void set_connection_callback(ConnectionCallback callback) override;

private:
    int socket_fd_;
    std::string socket_path_;
    std::atomic<bool> connected_;
    std::atomic<bool> running_;
    std::thread message_thread_;
    mutable std::mutex mutex_;
    
    // Internal state
    std::map<std::string, FRRRoute> routes_;
    std::map<std::string, FRRNeighbor> neighbors_;
    FRRStatistics stats_;
    FRRConfig config_;
    
    // Callbacks
    RouteUpdateCallback route_update_callback_;
    NeighborUpdateCallback neighbor_update_callback_;
    ConnectionCallback connection_callback_;
    
    // Internal methods
    void message_processing_loop();
    bool send_message(const std::string& message);
    bool receive_message(std::string& message, uint32_t timeout_ms = 1000);
    void process_message(const std::string& message);
    bool parse_route_message(const std::string& message, FRRRoute& route);
    bool parse_neighbor_message(const std::string& message, FRRNeighbor& neighbor);
    std::string serialize_route(const FRRRoute& route) const;
    std::string serialize_neighbor(const FRRNeighbor& neighbor) const;
};

// BGP Protocol Implementation
class BGPProtocol {
public:
    BGPProtocol();
    ~BGPProtocol();
    
    bool initialize(const std::map<std::string, std::string>& config);
    bool start();
    bool stop();
    bool is_running() const;
    
    // BGP-specific operations
    bool add_neighbor(const std::string& address, uint32_t asn, const std::map<std::string, std::string>& config);
    bool remove_neighbor(const std::string& address);
    bool advertise_route(const std::string& prefix, const std::string& next_hop, const std::map<std::string, std::string>& attributes);
    bool withdraw_route(const std::string& prefix);
    
    // Route filtering and policy
    bool add_route_filter(const std::string& prefix, const std::string& action);
    bool add_community_filter(const std::string& community, const std::string& action);
    bool add_as_path_filter(const std::string& as_path, const std::string& action);
    
    // Statistics
    struct BGPStatistics {
        uint32_t total_routes;
        uint32_t advertised_routes;
        uint32_t received_routes;
        uint32_t filtered_routes;
        uint32_t active_neighbors;
        uint32_t established_neighbors;
        uint32_t updates_sent;
        uint32_t updates_received;
        std::chrono::system_clock::time_point last_update;
    };
    
    BGPStatistics get_statistics() const;

private:
    std::atomic<bool> running_;
    std::map<std::string, std::string> config_;
    BGPStatistics stats_;
    mutable std::mutex mutex_;
};

// OSPF Protocol Implementation
class OSPFProtocol {
public:
    OSPFProtocol();
    ~OSPFProtocol();
    
    bool initialize(const std::map<std::string, std::string>& config);
    bool start();
    bool stop();
    bool is_running() const;
    
    // OSPF-specific operations
    bool add_interface(const std::string& interface, const std::string& area, const std::map<std::string, std::string>& config);
    bool remove_interface(const std::string& interface);
    bool add_area(const std::string& area_id, const std::string& area_type);
    bool add_virtual_link(const std::string& area_id, const std::string& router_id);
    
    // LSA management
    bool originate_lsa(const std::string& type, const std::map<std::string, std::string>& data);
    bool flood_lsa(const std::string& lsa_id);
    
    // Statistics
    struct OSPFStatistics {
        uint32_t total_lsas;
        uint32_t router_lsas;
        uint32_t network_lsas;
        uint32_t summary_lsas;
        uint32_t as_external_lsas;
        uint32_t active_interfaces;
        uint32_t total_areas;
        uint32_t ls_updates_sent;
        uint32_t ls_updates_received;
        std::chrono::system_clock::time_point last_update;
    };
    
    OSPFStatistics get_statistics() const;

private:
    std::atomic<bool> running_;
    std::map<std::string, std::string> config_;
    OSPFStatistics stats_;
    mutable std::mutex mutex_;
};

// ISIS Protocol Implementation
class ISISProtocol {
public:
    ISISProtocol();
    ~ISISProtocol();
    
    bool initialize(const std::map<std::string, std::string>& config);
    bool start();
    bool stop();
    bool is_running() const;
    
    // ISIS-specific operations
    bool add_interface(const std::string& interface, const std::string& level, const std::map<std::string, std::string>& config);
    bool remove_interface(const std::string& interface);
    bool add_adjacency(const std::string& neighbor_id, const std::string& level);
    bool remove_adjacency(const std::string& neighbor_id);
    
    // LSP management
    bool originate_lsp(const std::string& lsp_id, const std::map<std::string, std::string>& data);
    bool flood_lsp(const std::string& lsp_id);
    
    // Statistics
    struct ISISStatistics {
        uint32_t total_lsps;
        uint32_t level1_lsps;
        uint32_t level2_lsps;
        uint32_t active_interfaces;
        uint32_t active_adjacencies;
        uint32_t lsp_updates_sent;
        uint32_t lsp_updates_received;
        std::chrono::system_clock::time_point last_update;
    };
    
    ISISStatistics get_statistics() const;

private:
    std::atomic<bool> running_;
    std::map<std::string, std::string> config_;
    ISISStatistics stats_;
    mutable std::mutex mutex_;
};

} // namespace router_sim