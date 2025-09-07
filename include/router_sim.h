#pragma once

#include <memory>
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <atomic>
#include <thread>
#include <functional>

namespace RouterSim {

// Forward declarations
class FRRIntegration;
class TrafficShaper;
class NetemImpairments;
class CLIInterface;
class YamlConfig;
class PacketProcessor;
class RoutingTable;
class Statistics;

// Configuration structures
struct RouterConfig {
    std::string router_id;
    std::string hostname;
    bool enable_bgp = false;
    bool enable_ospf = false;
    bool enable_isis = false;
    uint32_t as_number = 0;
    std::string area_id;
    std::string system_id;
    std::vector<std::string> interfaces;
    std::map<std::string, std::map<std::string, std::string>> interface_configs;
    std::map<std::string, std::map<std::string, std::string>> traffic_shaping;
    std::map<std::string, std::map<std::string, std::string>> impairments;
};

struct InterfaceInfo {
    std::string name;
    std::string ip_address;
    std::string subnet_mask;
    uint32_t bandwidth_mbps;
    bool is_up;
    std::string description;
    uint64_t packets_in;
    uint64_t packets_out;
    uint64_t bytes_in;
    uint64_t bytes_out;
    uint64_t errors_in;
    uint64_t errors_out;
};

struct RouteInfo {
    std::string destination;
    uint8_t prefix_length;
    std::string next_hop;
    std::string protocol;
    uint32_t metric;
    uint32_t admin_distance;
    std::chrono::steady_clock::time_point last_updated;
    bool is_active;
};

struct NeighborInfo {
    std::string address;
    std::string protocol;
    std::string state;
    std::chrono::steady_clock::time_point last_hello;
    uint32_t hold_time;
    std::map<std::string, std::string> capabilities;
};

enum class StatCategory {
    PACKET_PROCESSING,
    ROUTING,
    INTERFACE,
    PROTOCOL,
    TRAFFIC_SHAPING,
    IMPAIRMENTS
};

// Main router simulator class
class RouterSimulator {
public:
    RouterSimulator();
    ~RouterSimulator();
    
    // Core functionality
    bool initialize(const RouterConfig& config);
    bool start();
    bool stop();
    bool is_running() const;
    
    // Configuration
    bool load_config(const std::string& config_file);
    bool save_config(const std::string& config_file) const;
    RouterConfig get_config() const;
    
    // Interface management
    bool add_interface(const InterfaceInfo& interface);
    bool remove_interface(const std::string& name);
    bool update_interface(const std::string& name, const InterfaceInfo& interface);
    std::vector<InterfaceInfo> get_interfaces() const;
    InterfaceInfo get_interface(const std::string& name) const;
    
    // Route management
    std::vector<RouteInfo> get_routes() const;
    std::vector<RouteInfo> get_routes_by_protocol(const std::string& protocol) const;
    bool add_route(const RouteInfo& route);
    bool remove_route(const std::string& destination, uint8_t prefix_length);
    
    // Neighbor management
    std::vector<NeighborInfo> get_neighbors() const;
    std::vector<NeighborInfo> get_neighbors_by_protocol(const std::string& protocol) const;
    
    // Statistics
    std::map<std::string, uint64_t> get_statistics() const;
    std::map<std::string, uint64_t> get_statistics_by_category(StatCategory category) const;
    
    // Event callbacks
    void set_route_update_callback(std::function<void(const RouteInfo&, bool)> callback);
    void set_neighbor_update_callback(std::function<void(const NeighborInfo&, bool)> callback);
    void set_interface_update_callback(std::function<void(const InterfaceInfo&, bool)> callback);
    
    // Component access
    FRRIntegration* get_frr_integration() const;
    TrafficShaper* get_traffic_shaper() const;
    NetemImpairments* get_netem_impairments() const;
    CLIInterface* get_cli_interface() const;
    Statistics* get_statistics_collector() const;

private:
    // Core components
    std::unique_ptr<FRRIntegration> frr_integration_;
    std::unique_ptr<TrafficShaper> traffic_shaper_;
    std::unique_ptr<NetemImpairments> netem_impairments_;
    std::unique_ptr<CLIInterface> cli_interface_;
    std::unique_ptr<YamlConfig> yaml_config_;
    std::unique_ptr<PacketProcessor> packet_processor_;
    std::unique_ptr<RoutingTable> routing_table_;
    std::unique_ptr<Statistics> statistics_;
    
    // State
    RouterConfig config_;
    std::atomic<bool> running_;
    std::vector<InterfaceInfo> interfaces_;
    std::vector<RouteInfo> routes_;
    std::vector<NeighborInfo> neighbors_;
    
    // Threading
    std::thread main_thread_;
    std::mutex interfaces_mutex_;
    std::mutex routes_mutex_;
    std::mutex neighbors_mutex_;
    
    // Event callbacks
    std::function<void(const RouteInfo&, bool)> route_update_callback_;
    std::function<void(const NeighborInfo&, bool)> neighbor_update_callback_;
    std::function<void(const InterfaceInfo&, bool)> interface_update_callback_;
    
    // Internal methods
    void main_loop();
    bool initialize_components();
    void cleanup_components();
    void update_statistics();
};

} // namespace RouterSim
