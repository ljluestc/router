#pragma once

#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <functional>

namespace router_sim {

// Forward declarations
struct RouteInfo;
struct NeighborInfo;
struct ProtocolStatistics;

// Callback types
using RouteUpdateCallback = std::function<void(const RouteInfo&, bool)>;
using NeighborUpdateCallback = std::function<void(const NeighborInfo&, bool)>;

// Protocol statistics structure
struct ProtocolStatistics {
    uint64_t messages_sent = 0;
    uint64_t messages_received = 0;
    uint64_t routes_advertised = 0;
    uint64_t routes_withdrawn = 0;
    uint64_t neighbor_up_count = 0;
    uint64_t neighbor_down_count = 0;
    uint64_t errors = 0;
    std::chrono::steady_clock::time_point last_update;
    
    void reset() {
        messages_sent = 0;
        messages_received = 0;
        routes_advertised = 0;
        routes_withdrawn = 0;
        neighbor_up_count = 0;
        neighbor_down_count = 0;
        errors = 0;
        last_update = std::chrono::steady_clock::now();
    }
};

// Route information structure
struct RouteInfo {
    std::string destination;
    uint8_t prefix_length;
    std::string next_hop;
    std::string protocol;
    uint32_t metric;
    uint32_t admin_distance;
    std::chrono::steady_clock::time_point last_updated;
    bool is_active;
    std::map<std::string, std::string> attributes;
    
    RouteInfo() : prefix_length(0), metric(0), admin_distance(0), is_active(false) {
        last_updated = std::chrono::steady_clock::now();
    }
    
    bool operator==(const RouteInfo& other) const {
        return destination == other.destination && prefix_length == other.prefix_length;
    }
};

// Neighbor information structure
struct NeighborInfo {
    std::string address;
    std::string protocol;
    std::string state;
    std::chrono::steady_clock::time_point last_hello;
    uint32_t hold_time;
    std::map<std::string, std::string> capabilities;
    std::map<std::string, std::string> attributes;
    
    NeighborInfo() : hold_time(0) {
        last_hello = std::chrono::steady_clock::now();
    }
    
    bool is_established() const {
        return state == "Established" || state == "Full";
    }
};

// Base protocol interface
class ProtocolInterface {
public:
    virtual ~ProtocolInterface() = default;
    
    // Core protocol operations
    virtual bool initialize(const std::map<std::string, std::string>& config) = 0;
    virtual bool start() = 0;
    virtual bool stop() = 0;
    virtual bool is_running() const = 0;
    
    // Neighbor management
    virtual bool add_neighbor(const std::string& address, const std::map<std::string, std::string>& config) = 0;
    virtual bool remove_neighbor(const std::string& address) = 0;
    virtual std::vector<NeighborInfo> get_neighbors() const = 0;
    virtual bool is_neighbor_established(const std::string& address) const = 0;
    
    // Route management
    virtual bool advertise_route(const RouteInfo& route) = 0;
    virtual bool withdraw_route(const std::string& destination, uint8_t prefix_length) = 0;
    virtual std::vector<RouteInfo> get_routes() const = 0;
    
    // Configuration
    virtual bool update_config(const std::map<std::string, std::string>& config) = 0;
    virtual std::map<std::string, std::string> get_config() const = 0;
    
    // Statistics
    virtual ProtocolStatistics get_statistics() const = 0;
    
    // Event callbacks
    virtual void set_route_update_callback(RouteUpdateCallback callback) = 0;
    virtual void set_neighbor_update_callback(NeighborUpdateCallback callback) = 0;
};

} // namespace router_sim
