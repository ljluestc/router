#pragma once

#include "protocol_interface.h"
#include "traffic_shaping.h"
#include "network_impairments.h"
#include "config/yaml_config.h"
#include "analytics/clickhouse_client.h"
#include "frr_integration.h"

#include <memory>
#include <vector>
#include <map>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>
#include <functional>

namespace RouterSim {

// Forward declarations
class BGPProtocol;
class OSPFProtocol;
class ISISProtocol;
class TrafficShaper;
class NetemImpairments;
class ClickHouseClient;

// Router interface information
struct InterfaceInfo {
    std::string name;
    std::string ip_address;
    std::string netmask;
    uint32_t mtu;
    bool enabled;
    std::string status;
    uint64_t bytes_in;
    uint64_t bytes_out;
    uint64_t packets_in;
    uint64_t packets_out;
    std::chrono::steady_clock::time_point last_updated;
    
    InterfaceInfo() : mtu(1500), enabled(false), bytes_in(0), bytes_out(0), 
                     packets_in(0), packets_out(0) {
        last_updated = std::chrono::steady_clock::now();
    }
};

// Router statistics
struct RouterStatistics {
    uint64_t total_packets_processed;
    uint64_t total_bytes_processed;
    uint64_t packets_dropped;
    uint64_t bytes_dropped;
    uint64_t routing_updates;
    uint64_t neighbor_changes;
    std::chrono::steady_clock::time_point start_time;
    std::chrono::steady_clock::time_point last_update;
    
    void reset() {
        total_packets_processed = 0;
        total_bytes_processed = 0;
        packets_dropped = 0;
        bytes_dropped = 0;
        routing_updates = 0;
        neighbor_changes = 0;
        start_time = std::chrono::steady_clock::now();
        last_update = start_time;
    }
};

// Main router core class
class RouterCore {
public:
    RouterCore();
    ~RouterCore();

    // Core lifecycle
    bool initialize(const std::string& config_file = "");
    bool start();
    bool stop();
    bool is_running() const;
    bool is_initialized() const;
    void shutdown();

    // Configuration management
    bool load_config(const std::string& config_file);
    bool save_config(const std::string& config_file) const;
    std::string get_running_config() const;

    // Interface management
    bool add_interface(const InterfaceInfo& interface);
    bool remove_interface(const std::string& interface_name);
    bool update_interface(const InterfaceInfo& interface);
    std::vector<InterfaceInfo> get_interfaces() const;
    InterfaceInfo get_interface(const std::string& interface_name) const;

    // Protocol management
    bool enable_protocol(const std::string& protocol_name, 
                        const std::map<std::string, std::string>& config);
    bool disable_protocol(const std::string& protocol_name);
    bool is_protocol_enabled(const std::string& protocol_name) const;
    std::vector<std::string> get_enabled_protocols() const;

    // BGP protocol
    bool start_bgp(const std::map<std::string, std::string>& config);
    bool stop_bgp();
    bool add_bgp_neighbor(const std::string& address, uint32_t as_number);
    bool remove_bgp_neighbor(const std::string& address);
    std::vector<router_sim::NeighborInfo> get_bgp_neighbors() const;
    std::vector<router_sim::RouteInfo> get_bgp_routes() const;

    // OSPF protocol
    bool start_ospf(const std::map<std::string, std::string>& config);
    bool stop_ospf();
    std::vector<router_sim::NeighborInfo> get_ospf_neighbors() const;
    std::vector<router_sim::RouteInfo> get_ospf_routes() const;

    // IS-IS protocol
    bool start_isis(const std::map<std::string, std::string>& config);
    bool stop_isis();
    std::vector<router_sim::NeighborInfo> get_isis_neighbors() const;
    std::vector<router_sim::RouteInfo> get_isis_routes() const;

    // Route management
    bool advertise_route(const router_sim::RouteInfo& route);
    bool withdraw_route(const std::string& destination, uint8_t prefix_length);
    std::vector<router_sim::RouteInfo> get_all_routes() const;
    std::vector<router_sim::RouteInfo> get_routes_by_protocol(const std::string& protocol) const;

    // Traffic shaping
    bool enable_traffic_shaping(const std::string& algorithm, 
                               const std::map<std::string, std::string>& config);
    bool disable_traffic_shaping();
    bool is_traffic_shaping_enabled() const;
    TrafficShaper::Statistics get_traffic_shaping_stats() const;

    // Network impairments
    bool enable_impairments(const std::string& interface, 
                           const std::map<std::string, std::string>& config);
    bool disable_impairments(const std::string& interface);
    bool are_impairments_enabled(const std::string& interface) const;

    // Analytics
    bool enable_analytics(const std::map<std::string, std::string>& config);
    bool disable_analytics();
    bool is_analytics_enabled() const;

    // Statistics and monitoring
    RouterStatistics get_statistics() const;
    void reset_statistics();

    // Event callbacks
    void set_route_update_callback(router_sim::RouteUpdateCallback callback);
    void set_neighbor_update_callback(router_sim::NeighborUpdateCallback callback);
    void set_interface_update_callback(std::function<void(const InterfaceInfo&, bool)> callback);

    // Scenario management
    bool load_scenario(const std::string& scenario_file);
    bool run_scenario(const std::string& scenario_name);
    std::vector<std::string> get_available_scenarios() const;

private:
    // Internal state
    std::atomic<bool> initialized_;
    std::atomic<bool> running_;
    mutable std::mutex state_mutex_;

    // Configuration
    std::unique_ptr<YAMLConfig> config_;
    std::string config_file_;

    // Interfaces
    std::map<std::string, InterfaceInfo> interfaces_;
    mutable std::mutex interfaces_mutex_;

    // Protocols
    std::unique_ptr<BGPProtocol> bgp_protocol_;
    std::unique_ptr<OSPFProtocol> ospf_protocol_;
    std::unique_ptr<ISISProtocol> isis_protocol_;
    std::map<std::string, bool> protocol_states_;
    mutable std::mutex protocols_mutex_;

    // Traffic shaping
    std::unique_ptr<TrafficShaper> traffic_shaper_;
    bool traffic_shaping_enabled_;
    mutable std::mutex traffic_mutex_;

    // Network impairments
    std::unique_ptr<NetemImpairments> netem_impairments_;
    std::map<std::string, bool> impairment_states_;
    mutable std::mutex impairments_mutex_;

    // Analytics
    std::unique_ptr<ClickHouseClient> analytics_client_;
    bool analytics_enabled_;
    mutable std::mutex analytics_mutex_;

    // FRR integration
    std::unique_ptr<FRRIntegration> frr_integration_;

    // Statistics
    RouterStatistics statistics_;
    mutable std::mutex statistics_mutex_;

    // Event callbacks
    router_sim::RouteUpdateCallback route_update_callback_;
    router_sim::NeighborUpdateCallback neighbor_update_callback_;
    std::function<void(const InterfaceInfo&, bool)> interface_update_callback_;

    // Internal methods
    bool initialize_protocols();
    bool initialize_traffic_shaping();
    bool initialize_impairments();
    bool initialize_analytics();
    bool initialize_frr();

    void protocol_route_update_callback(const router_sim::RouteInfo& route, bool added);
    void protocol_neighbor_update_callback(const router_sim::NeighborInfo& neighbor, bool added);

    // Main processing loop
    void main_loop();
    std::thread main_thread_;

    // Packet processing
    bool process_packet(const Packet& packet);
    bool route_packet(const Packet& packet);
};

} // namespace RouterSim
