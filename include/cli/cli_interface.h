#pragma once

#include <string>
#include <vector>
#include <map>
#include <thread>
#include <memory>
#include <functional>

namespace RouterSim {

// Forward declarations
class RouterCore;
class FRRIntegration;
class TrafficShapingManager;
class NetemImpairments;

// CLI interface class
class CLIInterface {
public:
    CLIInterface();
    ~CLIInterface();
    
    // Lifecycle
    bool initialize();
    bool start();
    bool stop();
    bool is_running() const;
    
private:
    // CLI processing
    void cli_loop();
    void execute_command(const std::string& input);
    
    // Command initialization
    void initialize_commands();
    
    // Help and status
    void show_help();
    void show_router_status();
    void show_interfaces();
    void show_routes();
    void show_neighbors();
    
    // Command handlers
    void handle_protocol_command(const std::vector<std::string>& tokens);
    void handle_traffic_command(const std::vector<std::string>& tokens);
    void handle_impairment_command(const std::vector<std::string>& tokens);
    void handle_scenario_command(const std::vector<std::string>& tokens);
    void handle_config_command(const std::vector<std::string>& tokens);
    void handle_stats_command(const std::vector<std::string>& tokens);
    
    // State
    bool initialized_;
    bool running_;
    
    // Components
    std::unique_ptr<RouterCore> router_core_;
    std::unique_ptr<FRRIntegration> frr_integration_;
    std::unique_ptr<TrafficShapingManager> traffic_shaper_;
    std::unique_ptr<NetemImpairments> netem_impairments_;
    
    // Command handling
    std::map<std::string, std::function<void(const std::vector<std::string>&)>> command_handlers_;
    
    // Threading
    std::thread cli_thread_;
};

} // namespace RouterSim
