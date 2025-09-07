#pragma once

#include <memory>
#include <string>
#include <map>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <queue>
#include <condition_variable>
#include <chrono>

#include "frr_integration.h"
#include "traffic_shaping.h"
#include "netem_impairments.h"
#include "cli_interface.h"
#include "yaml_config.h"
#include "packet_processor.h"
#include "routing_table.h"
#include "statistics.h"

namespace RouterSim {

// Forward declarations
class RouterSimulator;

// Configuration structures
struct InterfaceConfig {
    std::string name;
    std::string ip_address;
    std::string subnet_mask;
    int mtu = 1500;
    int bandwidth_mbps = 1000;
    bool enabled = true;
    std::string description;
};

struct RouterConfig {
    std::string router_id;
    std::vector<InterfaceConfig> interfaces;
    BGPConfig bgp_config;
    OSPFConfig ospf_config;
    ISISConfig isis_config;
    std::string log_level = "info";
    int cli_port = 8080;
    bool daemon_mode = false;
};

struct PacketInfo {
    std::vector<uint8_t> data;
    std::string source_interface;
    std::string destination_interface;
    std::chrono::steady_clock::time_point timestamp;
    size_t size;
    uint16_t protocol;
    uint32_t source_ip;
    uint32_t destination_ip;
};

// Main router simulator class
class RouterSimulator {
public:
    RouterSimulator();
    ~RouterSimulator();

    // Core functionality
    bool initialize(const RouterConfig& config);
    bool start();
    void stop();
    bool is_running() const { return running_; }

    // Configuration management
    bool load_config_from_file(const std::string& config_file);
    bool save_config_to_file(const std::string& config_file);
    const RouterConfig& get_config() const { return config_; }

    // Interface management
    bool add_interface(const InterfaceConfig& config);
    bool remove_interface(const std::string& name);
    bool update_interface(const std::string& name, const InterfaceConfig& config);
    const std::map<std::string, InterfaceConfig>& get_interfaces() const { return interfaces_; }

    // Protocol management
    bool enable_bgp(const BGPConfig& config);
    bool disable_bgp();
    bool enable_ospf(const OSPFConfig& config);
    bool disable_ospf();
    bool enable_isis(const ISISConfig& config);
    bool disable_isis();

    // Traffic shaping
    bool configure_traffic_shaping(const std::string& interface, const ShapingConfig& config);
    bool apply_impairments(const std::string& interface, const ImpairmentConfig& config);

    // Scenario management
    bool load_scenario(const std::string& scenario_file);
    bool run_scenario(const std::string& scenario_file);

    // CLI interface
    void start_cli();
    void stop_cli();

    // Statistics and monitoring
    Statistics* get_statistics() { return statistics_.get(); }
    std::map<std::string, std::string> get_status() const;

    // Packet processing
    void process_packet(const PacketInfo& packet);
    void send_packet(const PacketInfo& packet);

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

    // Configuration
    RouterConfig config_;
    std::map<std::string, InterfaceConfig> interfaces_;

    // Threading and synchronization
    std::atomic<bool> running_;
    std::thread packet_processing_thread_;
    std::thread monitoring_thread_;
    std::mutex packet_queue_mutex_;
    std::queue<PacketInfo> packet_queue_;
    std::condition_variable packet_queue_cv_;

    // Internal methods
    void packet_processing_loop();
    void monitoring_loop();
    void process_packet_internal(const PacketInfo& packet);
};

} // namespace RouterSim
