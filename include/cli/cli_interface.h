#pragma once

#include "router_core.h"
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <memory>

namespace RouterSim {

class CLIInterface {
public:
    CLIInterface();
    ~CLIInterface();

    // Core management
    bool initialize(RouterCore* router_core);
    bool start();
    bool stop();
    bool is_running() const;

    // Main execution
    void run();

private:
    // Internal state
    bool running_;
    RouterCore* router_core_;
    std::map<std::string, std::function<void(const std::vector<std::string>&)>> commands_;

    // Command parsing and execution
    void initialize_commands();
    std::vector<std::string> parse_command(const std::string& line);
    void execute_command(const std::vector<std::string>& tokens);

    // Command implementations
    void cmd_help(const std::vector<std::string>& args);
    void cmd_exit(const std::vector<std::string>& args);
    void cmd_status(const std::vector<std::string>& args);
    void cmd_start(const std::vector<std::string>& args);
    void cmd_stop(const std::vector<std::string>& args);
    void cmd_restart(const std::vector<std::string>& args);

    // Interface commands
    void cmd_interface(const std::vector<std::string>& args);
    void cmd_show_interfaces(const std::vector<std::string>& args);

    // Protocol commands
    void cmd_protocol(const std::vector<std::string>& args);
    void cmd_show_protocols(const std::vector<std::string>& args);
    void cmd_show_routes(const std::vector<std::string>& args);
    void cmd_show_neighbors(const std::vector<std::string>& args);

    // BGP commands
    void cmd_bgp(const std::vector<std::string>& args);
    void cmd_show_bgp(const std::vector<std::string>& args);
    void cmd_show_bgp_routes(const std::vector<std::string>& args);
    void cmd_show_bgp_neighbors(const std::vector<std::string>& args);

    // OSPF commands
    void cmd_ospf(const std::vector<std::string>& args);
    void cmd_show_ospf(const std::vector<std::string>& args);
    void cmd_show_ospf_routes(const std::vector<std::string>& args);
    void cmd_show_ospf_neighbors(const std::vector<std::string>& args);

    // IS-IS commands
    void cmd_isis(const std::vector<std::string>& args);
    void cmd_show_isis(const std::vector<std::string>& args);
    void cmd_show_isis_routes(const std::vector<std::string>& args);
    void cmd_show_isis_neighbors(const std::vector<std::string>& args);

    // Traffic shaping commands
    void cmd_traffic(const std::vector<std::string>& args);
    void cmd_show_traffic(const std::vector<std::string>& args);

    // Netem commands
    void cmd_netem(const std::vector<std::string>& args);
    void cmd_show_netem(const std::vector<std::string>& args);

    // Configuration commands
    void cmd_configure(const std::vector<std::string>& args);
    void cmd_load_config(const std::vector<std::string>& args);
    void cmd_save_config(const std::vector<std::string>& args);

    // Testing commands
    void cmd_test(const std::vector<std::string>& args);
    void cmd_capture(const std::vector<std::string>& args);
    void cmd_compare(const std::vector<std::string>& args);
};

} // namespace RouterSim
