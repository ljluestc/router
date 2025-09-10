#pragma once

#include <string>
#include <vector>
#include <map>
#include <chrono>

namespace router_sim {

// Route information structure
struct RouteInfo {
    std::string prefix;
    std::string next_hop;
    std::string protocol;
    uint32_t metric;
    std::string as_path;
    std::string area_id;
    std::chrono::steady_clock::time_point last_update;
    bool is_active;
    
    RouteInfo() : metric(0), is_active(true) {
        last_update = std::chrono::steady_clock::now();
    }
};

// Neighbor information structure
struct NeighborInfo {
    std::string address;
    std::string protocol;
    std::string state;
    std::string interface;
    std::string area_id;
    uint32_t as_number;
    uint32_t priority;
    uint32_t hold_time;
    uint32_t keepalive_time;
    std::chrono::steady_clock::time_point last_update;
    
    NeighborInfo() : as_number(0), priority(0), hold_time(0), keepalive_time(0) {
        last_update = std::chrono::steady_clock::now();
    }
};

// Protocol interface base class
class ProtocolInterface {
public:
    virtual ~ProtocolInterface() = default;
    
    // Protocol management
    virtual bool start(const std::map<std::string, std::string>& config) = 0;
    virtual bool stop() = 0;
    virtual bool is_running() const = 0;
    
    // Route management
    virtual bool advertise_route(const std::string& prefix, const std::map<std::string, std::string>& attributes) = 0;
    virtual bool withdraw_route(const std::string& prefix) = 0;
    
    // Information retrieval
    virtual std::vector<RouteInfo> get_routes() const = 0;
    virtual std::vector<NeighborInfo> get_neighbors() const = 0;
    virtual std::map<std::string, uint64_t> get_statistics() const = 0;
    
    // Callbacks
    virtual void set_route_update_callback(std::function<void(const RouteInfo&, bool)> callback) = 0;
    virtual void set_neighbor_update_callback(std::function<void(const NeighborInfo&, bool)> callback) = 0;
};

} // namespace router_sim