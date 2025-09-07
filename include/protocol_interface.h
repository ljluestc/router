#pragma once

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <chrono>

namespace router_sim {

// Forward declarations
struct NeighborInfo;
struct RouteInfo;
struct ProtocolStatistics;

// Callback types
using RouteUpdateCallback = std::function<void(const RouteInfo&, bool)>;
using NeighborUpdateCallback = std::function<void(const NeighborInfo&, bool)>;

// Protocol interface for all routing protocols
class ProtocolInterface {
public:
    virtual ~ProtocolInterface() = default;
    
    // Core protocol management
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