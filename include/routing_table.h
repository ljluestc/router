#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include <functional>

namespace RouterSim {

// Forward declarations
struct RouteEntry;

// Route comparison for priority
struct RouteComparator {
    bool operator()(const RouteEntry& a, const RouteEntry& b) const;
};

// Routing table class
class RoutingTable {
public:
    RoutingTable();
    ~RoutingTable() = default;

    // Route management
    bool add_route(const RouteEntry& route);
    bool remove_route(const std::string& network);
    bool update_route(const RouteEntry& route);
    bool has_route(const std::string& network) const;

    // Route lookup
    std::vector<RouteEntry> lookup_routes(const std::string& destination) const;
    RouteEntry get_best_route(const std::string& destination) const;
    std::string get_next_hop(const std::string& destination) const;

    // Route queries
    std::vector<RouteEntry> get_all_routes() const;
    std::vector<RouteEntry> get_routes_by_protocol(const std::string& protocol) const;
    std::vector<RouteEntry> get_routes_by_interface(const std::string& interface) const;
    std::vector<RouteEntry> get_active_routes() const;

    // Route filtering
    std::vector<RouteEntry> filter_routes(const std::function<bool(const RouteEntry&)>& filter) const;

    // Route statistics
    size_t get_route_count() const;
    size_t get_route_count_by_protocol(const std::string& protocol) const;
    std::map<std::string, size_t> get_protocol_counts() const;

    // Route management
    void clear_routes();
    void clear_routes_by_protocol(const std::string& protocol);
    void clear_routes_by_interface(const std::string& interface);

    // Event handling
    void register_route_change_callback(std::function<void(const RouteEntry&, bool)> callback);
    void unregister_route_change_callback();

    // Persistence
    bool save_to_file(const std::string& filename) const;
    bool load_from_file(const std::string& filename);

    // Validation
    bool is_valid_route(const RouteEntry& route) const;
    bool is_route_active(const RouteEntry& route) const;

private:
    // Route storage
    std::map<std::string, std::vector<RouteEntry>> routes_by_network_;
    std::map<std::string, std::vector<RouteEntry>> routes_by_protocol_;
    std::map<std::string, std::vector<RouteEntry>> routes_by_interface_;
    
    mutable std::mutex routes_mutex_;

    // Event handling
    std::function<void(const RouteEntry&, bool)> route_change_callback_;

    // Internal methods
    void notify_route_change(const RouteEntry& route, bool added);
    std::vector<RouteEntry> find_routes(const std::string& network) const;
    RouteEntry select_best_route(const std::vector<RouteEntry>& routes) const;
    bool route_matches(const RouteEntry& route, const std::string& destination) const;
    int calculate_route_priority(const RouteEntry& route) const;
};

// Route utilities
class RouteUtils {
public:
    // Route validation
    static bool is_valid_network(const std::string& network);
    static bool is_valid_next_hop(const std::string& next_hop);
    static bool is_valid_metric(uint32_t metric);
    static bool is_valid_admin_distance(uint32_t admin_distance);

    // Route comparison
    static bool is_better_route(const RouteEntry& a, const RouteEntry& b);
    static int compare_routes(const RouteEntry& a, const RouteEntry& b);

    // Network utilities
    static bool is_subnet_of(const std::string& network, const std::string& subnet);
    static std::string get_network_address(const std::string& ip, const std::string& mask);
    static std::string get_broadcast_address(const std::string& ip, const std::string& mask);
    static bool is_ip_in_network(const std::string& ip, const std::string& network);

    // Route formatting
    static std::string format_route(const RouteEntry& route);
    static std::string format_route_table(const std::vector<RouteEntry>& routes);
    static std::string format_route_summary(const std::vector<RouteEntry>& routes);

    // Route parsing
    static RouteEntry parse_route_string(const std::string& route_str);
    static std::vector<RouteEntry> parse_route_table_string(const std::string& table_str);
};

// Route manager for multiple routing tables
class RouteManager {
public:
    RouteManager();
    ~RouteManager() = default;

    // Table management
    bool create_table(const std::string& name);
    bool delete_table(const std::string& name);
    bool has_table(const std::string& name) const;
    std::vector<std::string> get_table_names() const;

    // Route operations
    bool add_route(const std::string& table_name, const RouteEntry& route);
    bool remove_route(const std::string& table_name, const std::string& network);
    bool update_route(const std::string& table_name, const RouteEntry& route);

    // Route lookup
    std::vector<RouteEntry> lookup_routes(const std::string& table_name, 
                                        const std::string& destination) const;
    RouteEntry get_best_route(const std::string& table_name, 
                            const std::string& destination) const;

    // Table operations
    std::shared_ptr<RoutingTable> get_table(const std::string& name) const;
    bool clear_table(const std::string& name);
    bool copy_table(const std::string& src_name, const std::string& dst_name);

private:
    std::map<std::string, std::shared_ptr<RoutingTable>> tables_;
    mutable std::mutex tables_mutex_;
};

} // namespace RouterSim
