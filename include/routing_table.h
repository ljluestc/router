#pragma once

#include "common_structures.h"
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <atomic>
#include <thread>

namespace RouterSim {

// Routing table entry
struct RoutingEntry {
    router_sim::RouteInfo route;
    std::chrono::steady_clock::time_point last_updated;
    uint32_t reference_count;
    bool is_active;
    
    RoutingEntry() : reference_count(0), is_active(false) {}
};

// Routing table class
class RoutingTable {
public:
    RoutingTable();
    ~RoutingTable();
    
    // Core functionality
    bool initialize();
    bool start();
    bool stop();
    bool is_running() const;
    
    // Route management
    bool add_route(const router_sim::RouteInfo& route);
    bool remove_route(const std::string& destination, uint8_t prefix_length);
    bool update_route(const router_sim::RouteInfo& route);
    bool has_route(const std::string& destination, uint8_t prefix_length) const;
    
    // Route lookup
    router_sim::RouteInfo* find_route(const std::string& destination) const;
    router_sim::RouteInfo* find_best_route(const std::string& destination) const;
    std::vector<router_sim::RouteInfo> get_routes() const;
    std::vector<router_sim::RouteInfo> get_routes_by_protocol(const std::string& protocol) const;
    std::vector<router_sim::RouteInfo> get_routes_by_interface(const std::string& interface) const;
    
    // Route filtering
    std::vector<router_sim::RouteInfo> get_active_routes() const;
    std::vector<router_sim::RouteInfo> get_routes_by_metric(uint32_t max_metric) const;
    std::vector<router_sim::RouteInfo> get_routes_by_admin_distance(uint32_t max_admin_distance) const;
    
    // Route management
    bool activate_route(const std::string& destination, uint8_t prefix_length);
    bool deactivate_route(const std::string& destination, uint8_t prefix_length);
    bool is_route_active(const std::string& destination, uint8_t prefix_length) const;
    
    // Route aging
    void age_routes();
    void remove_stale_routes(uint32_t max_age_seconds);
    void update_route_timestamp(const std::string& destination, uint8_t prefix_length);
    
    // Statistics
    std::map<std::string, uint64_t> get_statistics() const;
    void reset_statistics();
    
    // Configuration
    void set_max_routes(uint32_t max_routes);
    void set_route_aging_interval(uint32_t interval_seconds);
    void set_stale_route_timeout(uint32_t timeout_seconds);
    
    // Event callbacks
    void set_route_add_callback(std::function<void(const router_sim::RouteInfo&)> callback);
    void set_route_remove_callback(std::function<void(const router_sim::RouteInfo&)> callback);
    void set_route_update_callback(std::function<void(const router_sim::RouteInfo&)> callback);
    
private:
    // Internal methods
    void aging_loop();
    bool is_route_stale(const RoutingEntry& entry) const;
    void update_statistics(const router_sim::RouteInfo& route, bool added);
    
    // Route comparison
    bool is_better_route(const router_sim::RouteInfo& route1, const router_sim::RouteInfo& route2) const;
    uint32_t calculate_route_priority(const router_sim::RouteInfo& route) const;
    
    // State
    std::atomic<bool> running_;
    std::atomic<bool> initialized_;
    std::thread aging_thread_;
    
    // Route storage
    std::map<std::string, RoutingEntry> routes_;
    mutable std::mutex routes_mutex_;
    
    // Configuration
    uint32_t max_routes_;
    uint32_t route_aging_interval_seconds_;
    uint32_t stale_route_timeout_seconds_;
    
    // Statistics
    uint64_t total_routes_;
    uint64_t active_routes_;
    uint64_t routes_added_;
    uint64_t routes_removed_;
    uint64_t routes_updated_;
    uint64_t routes_aged_;
    uint64_t lookup_attempts_;
    uint64_t lookup_hits_;
    uint64_t lookup_misses_;
    mutable std::mutex stats_mutex_;
    
    // Event callbacks
    std::function<void(const router_sim::RouteInfo&)> route_add_callback_;
    std::function<void(const router_sim::RouteInfo&)> route_remove_callback_;
    std::function<void(const router_sim::RouteInfo&)> route_update_callback_;
};

} // namespace RouterSim