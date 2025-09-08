#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include <thread>
#include <atomic>
#include <chrono>
#include <yaml-cpp/yaml.h>

namespace RouterSim {

// Forward declarations
class ProtocolInterface;
class TrafficShaper;
class FRRIntegration;
class ClickHouseClient;

// Interface configuration
struct InterfaceConfig {
    std::string name;
    std::string ip_address;
    std::string subnet_mask;
    int mtu;
    bool enabled;
    
    InterfaceConfig() : mtu(1500), enabled(true) {}
};

// Route information
struct Route {
    std::string destination;
    std::string next_hop;
    std::string interface;
    int metric;
    std::string protocol;
    bool is_active;
    std::chrono::steady_clock::time_point last_update;
    
    Route() : metric(0), is_active(false) {
        last_update = std::chrono::steady_clock::now();
    }
};

// Router statistics
struct Statistics {
    uint32_t interface_count;
    uint32_t route_count;
    uint32_t protocol_count;
    uint64_t packets_processed;
    uint64_t bytes_processed;
    std::chrono::steady_clock::time_point last_update;
    
    Statistics() : interface_count(0), route_count(0), protocol_count(0),
                   packets_processed(0), bytes_processed(0) {
        last_update = std::chrono::steady_clock::now();
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
    void stop();
    bool is_running() const;
    
    // Configuration management
    bool load_config(const std::string& config_file);
    bool save_config(const std::string& config_file) const;
    
    // Interface management
    bool add_interface(const std::string& name, const std::string& ip_address,
                      const std::string& subnet_mask, int mtu = 1500);
    bool remove_interface(const std::string& name);
    std::vector<InterfaceConfig> get_interfaces() const;
    
    // Route management
    bool add_route(const std::string& destination, const std::string& next_hop,
                  const std::string& interface = "", int metric = 0);
    bool remove_route(const std::string& destination);
    std::vector<Route> get_routes() const;
    
    // Protocol management
    bool enable_protocol(const std::string& protocol_name, const YAML::Node& config);
    bool disable_protocol(const std::string& protocol_name);
    std::vector<std::string> get_enabled_protocols() const;
    
    // Statistics
    Statistics get_statistics() const;
    
    // Signal handling
    static void signal_handler(int signal);
    
private:
    // State
    std::atomic<bool> running_;
    std::atomic<bool> initialized_;
    std::atomic<bool> config_loaded_;
    
    // Threading
    mutable std::mutex state_mutex_;
    mutable std::mutex interfaces_mutex_;
    mutable std::mutex routes_mutex_;
    mutable std::mutex protocols_mutex_;
    mutable std::mutex stats_mutex_;
    
    std::thread main_thread_;
    
    // Components
    std::map<std::string, InterfaceConfig> interfaces_;
    std::map<std::string, Route> routes_;
    std::map<std::string, std::unique_ptr<ProtocolInterface>> protocols_;
    
    std::unique_ptr<TrafficShaper> traffic_shaper_;
    std::unique_ptr<FRRIntegration> frr_integration_;
    std::unique_ptr<ClickHouseClient> analytics_engine_;
    
    // Statistics
    Statistics statistics_;
    
    // Internal methods
    void main_loop();
    void process_packets();
    void update_statistics();
    
    // Initialization methods
    bool initialize_protocols();
    bool initialize_traffic_shaping();
    bool initialize_frr_integration();
    bool initialize_analytics();
    
    // Configuration methods
    void configure_traffic_shaping(const YAML::Node& config);
};

} // namespace RouterSim
