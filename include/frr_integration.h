#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <thread>
#include <atomic>
#include <mutex>
#include <functional>
#include <zmq.hpp>
#include "protocol_interface.h"

namespace router_sim {

// FRR Protocol Types
enum class FRRProtocol {
    BGP = 0,
    OSPF = 1,
    ISIS = 2
};

// FRR Message Types
enum class FRRMessageType {
    ROUTE_ADD = 0,
    ROUTE_DELETE = 1,
    NEIGHBOR_UP = 2,
    NEIGHBOR_DOWN = 3,
    CONFIG_UPDATE = 4,
    STATISTICS = 5
};

// FRR Message Structure
struct FRRMessage {
    FRRMessageType type;
    FRRProtocol protocol;
    std::string data;
    std::map<std::string, std::string> attributes;
    std::chrono::steady_clock::time_point timestamp;
    
    FRRMessage() : type(FRRMessageType::ROUTE_ADD), protocol(FRRProtocol::BGP) {
        timestamp = std::chrono::steady_clock::now();
    }
};

// FRR Configuration
struct FRRConfig {
    std::string host = "localhost";
    int port = 2605;
    std::string password = "";
    bool enable_bgp = true;
    bool enable_ospf = true;
    bool enable_isis = true;
    std::string log_level = "info";
    std::string config_file = "/etc/frr/frr.conf";
    std::map<std::string, std::string> custom_config;
};

// FRR Statistics
struct FRRStatistics {
    uint64_t messages_sent = 0;
    uint64_t messages_received = 0;
    uint64_t routes_installed = 0;
    uint64_t routes_removed = 0;
    uint64_t neighbors_established = 0;
    uint64_t neighbors_lost = 0;
    uint64_t errors = 0;
    std::chrono::steady_clock::time_point last_update;
    
    void reset() {
        messages_sent = 0;
        messages_received = 0;
        routes_installed = 0;
        routes_removed = 0;
        neighbors_established = 0;
        neighbors_lost = 0;
        errors = 0;
        last_update = std::chrono::steady_clock::now();
    }
};

// FRR Control Plane Interface
class FRRControlPlane {
public:
    FRRControlPlane();
    virtual ~FRRControlPlane();
    
    // Core operations
    bool initialize(const FRRConfig& config);
    bool start();
    bool stop();
    bool is_running() const;
    
    // Protocol management
    bool enable_protocol(FRRProtocol protocol);
    bool disable_protocol(FRRProtocol protocol);
    bool is_protocol_enabled(FRRProtocol protocol) const;
    
    // Configuration management
    bool load_config(const std::string& config_file);
    bool save_config(const std::string& config_file);
    bool apply_config();
    bool reload_config();
    
    // Route management
    bool add_route(const RouteInfo& route);
    bool remove_route(const std::string& destination, uint8_t prefix_length);
    bool update_route(const RouteInfo& route);
    std::vector<RouteInfo> get_routes(FRRProtocol protocol = FRRProtocol::BGP) const;
    
    // Neighbor management
    bool add_neighbor(const std::string& address, FRRProtocol protocol, 
                     const std::map<std::string, std::string>& config);
    bool remove_neighbor(const std::string& address, FRRProtocol protocol);
    std::vector<NeighborInfo> get_neighbors(FRRProtocol protocol = FRRProtocol::BGP) const;
    
    // Statistics
    FRRStatistics get_statistics() const;
    FRRStatistics get_protocol_statistics(FRRProtocol protocol) const;
    
    // Event callbacks
    void set_route_update_callback(std::function<void(const RouteInfo&, bool)> callback);
    void set_neighbor_update_callback(std::function<void(const NeighborInfo&, bool)> callback);
    void set_error_callback(std::function<void(const std::string&)> callback);
    
    // ZMQ communication
    bool send_message(const FRRMessage& message);
    bool receive_message(FRRMessage& message, int timeout_ms = 1000);
    
    // Debug and monitoring
    std::string get_status() const;
    std::vector<std::string> get_logs(int lines = 100) const;
    bool enable_debug(bool enable);

private:
    // Internal methods
    void message_loop();
    void process_message(const FRRMessage& message);
    bool connect_to_frr();
    void disconnect_from_frr();
    std::string serialize_message(const FRRMessage& message) const;
    bool deserialize_message(const std::string& data, FRRMessage& message) const;
    
    // ZMQ context and socket
    std::unique_ptr<zmq::context_t> zmq_context_;
    std::unique_ptr<zmq::socket_t> zmq_socket_;
    
    // Configuration and state
    FRRConfig config_;
    std::atomic<bool> running_;
    std::atomic<bool> connected_;
    std::map<FRRProtocol, bool> enabled_protocols_;
    
    // Statistics
    mutable std::mutex stats_mutex_;
    FRRStatistics statistics_;
    std::map<FRRProtocol, FRRStatistics> protocol_statistics_;
    
    // Callbacks
    std::function<void(const RouteInfo&, bool)> route_callback_;
    std::function<void(const NeighborInfo&, bool)> neighbor_callback_;
    std::function<void(const std::string&)> error_callback_;
    
    // Threading
    std::thread message_thread_;
    mutable std::mutex callback_mutex_;
};

// FRR Protocol Implementations
class FRRBGP : public ProtocolInterface {
public:
    FRRBGP(std::shared_ptr<FRRControlPlane> control_plane);
    virtual ~FRRBGP() = default;
    
    // ProtocolInterface implementation
    bool initialize(const std::map<std::string, std::string>& config) override;
    bool start() override;
    bool stop() override;
    bool is_running() const override;
    
    bool add_neighbor(const std::string& address, const std::map<std::string, std::string>& config) override;
    bool remove_neighbor(const std::string& address) override;
    std::vector<NeighborInfo> get_neighbors() const override;
    bool is_neighbor_established(const std::string& address) const override;
    
    bool advertise_route(const RouteInfo& route) override;
    bool withdraw_route(const std::string& destination, uint8_t prefix_length) override;
    std::vector<RouteInfo> get_routes() const override;
    
    bool update_config(const std::map<std::string, std::string>& config) override;
    std::map<std::string, std::string> get_config() const override;
    
    ProtocolStatistics get_statistics() const override;
    
    void set_route_update_callback(RouteUpdateCallback callback) override;
    void set_neighbor_update_callback(NeighborUpdateCallback callback) override;

private:
    std::shared_ptr<FRRControlPlane> control_plane_;
    std::map<std::string, std::string> config_;
    std::atomic<bool> running_;
    mutable std::mutex config_mutex_;
};

class FRROSPF : public ProtocolInterface {
public:
    FRROSPF(std::shared_ptr<FRRControlPlane> control_plane);
    virtual ~FRROSPF() = default;
    
    // ProtocolInterface implementation
    bool initialize(const std::map<std::string, std::string>& config) override;
    bool start() override;
    bool stop() override;
    bool is_running() const override;
    
    bool add_neighbor(const std::string& address, const std::map<std::string, std::string>& config) override;
    bool remove_neighbor(const std::string& address) override;
    std::vector<NeighborInfo> get_neighbors() const override;
    bool is_neighbor_established(const std::string& address) const override;
    
    bool advertise_route(const RouteInfo& route) override;
    bool withdraw_route(const std::string& destination, uint8_t prefix_length) override;
    std::vector<RouteInfo> get_routes() const override;
    
    bool update_config(const std::map<std::string, std::string>& config) override;
    std::map<std::string, std::string> get_config() const override;
    
    ProtocolStatistics get_statistics() const override;
    
    void set_route_update_callback(RouteUpdateCallback callback) override;
    void set_neighbor_update_callback(NeighborUpdateCallback callback) override;

private:
    std::shared_ptr<FRRControlPlane> control_plane_;
    std::map<std::string, std::string> config_;
    std::atomic<bool> running_;
    mutable std::mutex config_mutex_;
};

class FRRISIS : public ProtocolInterface {
public:
    FRRISIS(std::shared_ptr<FRRControlPlane> control_plane);
    virtual ~FRRISIS() = default;
    
    // ProtocolInterface implementation
    bool initialize(const std::map<std::string, std::string>& config) override;
    bool start() override;
    bool stop() override;
    bool is_running() const override;
    
    bool add_neighbor(const std::string& address, const std::map<std::string, std::string>& config) override;
    bool remove_neighbor(const std::string& address) override;
    std::vector<NeighborInfo> get_neighbors() const override;
    bool is_neighbor_established(const std::string& address) const override;
    
    bool advertise_route(const RouteInfo& route) override;
    bool withdraw_route(const std::string& destination, uint8_t prefix_length) override;
    std::vector<RouteInfo> get_routes() const override;
    
    bool update_config(const std::map<std::string, std::string>& config) override;
    std::map<std::string, std::string> get_config() const override;
    
    ProtocolStatistics get_statistics() const override;
    
    void set_route_update_callback(RouteUpdateCallback callback) override;
    void set_neighbor_update_callback(NeighborUpdateCallback callback) override;

private:
    std::shared_ptr<FRRControlPlane> control_plane_;
    std::map<std::string, std::string> config_;
    std::atomic<bool> running_;
    mutable std::mutex config_mutex_;
};

} // namespace router_sim