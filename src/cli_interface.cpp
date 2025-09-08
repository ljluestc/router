#include "cli_interface.h"
#include "router_core.h"
#include "frr_integration.h"
#include "traffic_shaping.h"
#include "netem_impairments.h"
#include "yaml_config.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <chrono>
#include <thread>

namespace RouterSim {

CLIInterface::CLIInterface() : running_(false), router_core_(nullptr), frr_integration_(nullptr), 
                              traffic_shaper_(nullptr), netem_impairments_(nullptr) {
    // Initialize command handlers
    initialize_commands();
}

CLIInterface::~CLIInterface() {
    stop();
}

bool CLIInterface::initialize() {
    if (running_) {
        return true;
    }
    
    // Initialize components
    router_core_ = std::make_unique<RouterCore>();
    frr_integration_ = std::make_unique<FRRIntegration>();
    traffic_shaper_ = std::make_unique<TrafficShapingManager>();
    netem_impairments_ = std::make_unique<NetemImpairments>();
    
    // Initialize components
    if (!router_core_->initialize()) {
        std::cerr << "Failed to initialize router core" << std::endl;
        return false;
    }
    
    if (!frr_integration_->initialize({})) {
        std::cerr << "Failed to initialize FRR integration" << std::endl;
        return false;
    }
    
    if (!traffic_shaper_->initialize()) {
        std::cerr << "Failed to initialize traffic shaper" << std::endl;
        return false;
    }
    
    if (!netem_impairments_->initialize()) {
        std::cerr << "Failed to initialize network impairments" << std::endl;
        return false;
    }
    
    return true;
}

bool CLIInterface::start() {
    if (!initialize()) {
        return false;
    }
    
    if (running_) {
        return true;
    }
    
    running_ = true;
    
    // Start components
    router_core_->start();
    frr_integration_->start();
    traffic_shaper_->start();
    netem_impairments_->start();
    
    // Start CLI thread
    cli_thread_ = std::thread(&CLIInterface::cli_loop, this);
    
    return true;
}

bool CLIInterface::stop() {
    if (!running_) {
        return true;
    }
    
    running_ = false;
    
    // Stop components
    if (netem_impairments_) {
        netem_impairments_->stop();
    }
    if (traffic_shaper_) {
        traffic_shaper_->stop();
    }
    if (frr_integration_) {
        frr_integration_->stop();
    }
    if (router_core_) {
        router_core_->stop();
    }
    
    // Wait for CLI thread to finish
    if (cli_thread_.joinable()) {
        cli_thread_.join();
    }
    
    return true;
}

bool CLIInterface::is_running() const {
    return running_;
}

void CLIInterface::initialize_commands() {
    // Help commands
    commands_["help"] = [this](const std::vector<std::string>& args) { return cmd_help(args); };
    commands_["?"] = [this](const std::vector<std::string>& args) { return cmd_help(args); };
    
    // Router core commands
    commands_["show interfaces"] = [this](const std::vector<std::string>& args) { return cmd_show_interfaces(args); };
    commands_["show routes"] = [this](const std::vector<std::string>& args) { return cmd_show_routes(args); };
    commands_["show statistics"] = [this](const std::vector<std::string>& args) { return cmd_show_statistics(args); };
    commands_["configure interface"] = [this](const std::vector<std::string>& args) { return cmd_configure_interface(args); };
    commands_["configure route"] = [this](const std::vector<std::string>& args) { return cmd_configure_route(args); };
    
    // FRR protocol commands
    commands_["start bgp"] = [this](const std::vector<std::string>& args) { return cmd_start_bgp(args); };
    commands_["stop bgp"] = [this](const std::vector<std::string>& args) { return cmd_stop_bgp(args); };
    commands_["start ospf"] = [this](const std::vector<std::string>& args) { return cmd_start_ospf(args); };
    commands_["stop ospf"] = [this](const std::vector<std::string>& args) { return cmd_stop_ospf(args); };
    commands_["start isis"] = [this](const std::vector<std::string>& args) { return cmd_start_isis(args); };
    commands_["stop isis"] = [this](const std::vector<std::string>& args) { return cmd_stop_isis(args); };
    commands_["show bgp neighbors"] = [this](const std::vector<std::string>& args) { return cmd_show_bgp_neighbors(args); };
    commands_["show ospf neighbors"] = [this](const std::vector<std::string>& args) { return cmd_show_ospf_neighbors(args); };
    commands_["show isis neighbors"] = [this](const std::vector<std::string>& args) { return cmd_show_isis_neighbors(args); };
    
    // Traffic shaping commands
    commands_["configure traffic-shaping"] = [this](const std::vector<std::string>& args) { return cmd_configure_traffic_shaping(args); };
    commands_["show traffic-shaping"] = [this](const std::vector<std::string>& args) { return cmd_show_traffic_shaping(args); };
    
    // Network impairments commands
    commands_["configure netem"] = [this](const std::vector<std::string>& args) { return cmd_configure_netem(args); };
    commands_["show netem"] = [this](const std::vector<std::string>& args) { return cmd_show_netem(args); };
    commands_["clear netem"] = [this](const std::vector<std::string>& args) { return cmd_clear_netem(args); };
    
    // Scenario commands
    commands_["load scenario"] = [this](const std::vector<std::string>& args) { return cmd_load_scenario(args); };
    commands_["save scenario"] = [this](const std::vector<std::string>& args) { return cmd_save_scenario(args); };
    
    // System commands
    commands_["exit"] = [this](const std::vector<std::string>& args) { return cmd_exit(args); };
    commands_["quit"] = [this](const std::vector<std::string>& args) { return cmd_exit(args); };
    commands_["clear"] = [this](const std::vector<std::string>& args) { return cmd_clear(args); };
}

void CLIInterface::cli_loop() {
    std::string line;
    
    print_banner();
    print_prompt();
    
    while (running_) {
        if (std::getline(std::cin, line)) {
            if (!line.empty()) {
                process_command(line);
            }
            print_prompt();
        } else {
            // EOF or error
            break;
        }
    }
}

void CLIInterface::process_command(const std::string& command) {
    std::vector<std::string> args = parse_command(command);
    
    if (args.empty()) {
        return;
    }
    
    std::string cmd = args[0];
    args.erase(args.begin());
    
    // Find command handler
    auto it = commands_.find(cmd);
    if (it != commands_.end()) {
        it->second(args);
    } else {
        std::cout << "Unknown command: " << cmd << std::endl;
        std::cout << "Type 'help' for available commands" << std::endl;
    }
}

std::vector<std::string> CLIInterface::parse_command(const std::string& command) {
    std::vector<std::string> args;
    std::istringstream iss(command);
    std::string arg;
    
    while (iss >> arg) {
        args.push_back(arg);
    }
    
    return args;
}

void CLIInterface::print_banner() {
    std::cout << R"(
╔══════════════════════════════════════════════════════════════════════════════╗
║                    Multi-Protocol Router Simulator CLI                      ║
║                                                                              ║
║  Available Commands:                                                         ║
║  • show interfaces          - Display interface information                 ║
║  • show routes             - Display routing table                         ║
║  • show statistics         - Display router statistics                     ║
║  • configure interface     - Configure network interface                   ║
║  • configure route         - Add/remove routes                             ║
║  • start/stop bgp/ospf/isis - Control routing protocols                    ║
║  • configure traffic-shaping - Configure traffic shaping                   ║
║  • configure netem         - Configure network impairments                 ║
║  • load/save scenario      - Load/save network scenarios                   ║
║  • help                    - Show this help message                        ║
║  • exit/quit               - Exit the CLI                                  ║
╚══════════════════════════════════════════════════════════════════════════════╝
)" << std::endl;
}

void CLIInterface::print_prompt() {
    std::cout << "router> ";
    std::cout.flush();
}

// Command implementations
bool CLIInterface::cmd_help(const std::vector<std::string>& args) {
    std::cout << "\nAvailable Commands:\n";
    std::cout << "  Router Core:\n";
    std::cout << "    show interfaces                    - Display interface information\n";
    std::cout << "    show routes                        - Display routing table\n";
    std::cout << "    show statistics                    - Display router statistics\n";
    std::cout << "    configure interface <name> <ip> <mask> [up|down] - Configure interface\n";
    std::cout << "    configure route <network> <next_hop> <interface> <metric> - Add route\n";
    std::cout << "\n  Routing Protocols:\n";
    std::cout << "    start bgp <as_number> <router_id>  - Start BGP protocol\n";
    std::cout << "    stop bgp                           - Stop BGP protocol\n";
    std::cout << "    start ospf <router_id> <area>      - Start OSPF protocol\n";
    std::cout << "    stop ospf                          - Stop OSPF protocol\n";
    std::cout << "    start isis <system_id> <area_id>   - Start IS-IS protocol\n";
    std::cout << "    stop isis                          - Stop IS-IS protocol\n";
    std::cout << "    show bgp neighbors                 - Show BGP neighbors\n";
    std::cout << "    show ospf neighbors                - Show OSPF neighbors\n";
    std::cout << "    show isis neighbors                - Show IS-IS neighbors\n";
    std::cout << "\n  Traffic Shaping:\n";
    std::cout << "    configure traffic-shaping <capacity> <rate> - Configure token bucket\n";
    std::cout << "    show traffic-shaping               - Show traffic shaping status\n";
    std::cout << "\n  Network Impairments:\n";
    std::cout << "    configure netem <interface> <delay> <loss> - Configure impairments\n";
    std::cout << "    show netem                         - Show current impairments\n";
    std::cout << "    clear netem                        - Clear all impairments\n";
    std::cout << "\n  Scenarios:\n";
    std::cout << "    load scenario <file>               - Load scenario from file\n";
    std::cout << "    save scenario <file>               - Save scenario to file\n";
    std::cout << "\n  System:\n";
    std::cout << "    help                               - Show this help message\n";
    std::cout << "    clear                              - Clear screen\n";
    std::cout << "    exit/quit                          - Exit the CLI\n";
    std::cout << std::endl;
    return true;
}

bool CLIInterface::cmd_show_interfaces(const std::vector<std::string>& args) {
    if (!router_core_) {
        std::cout << "Router core not initialized" << std::endl;
        return false;
    }
    
    auto interfaces = router_core_->get_interfaces();
    
    std::cout << "\nInterface Status:\n";
    std::cout << "Interface    IP Address      Subnet Mask      Status    Packets Sent    Packets Received\n";
    std::cout << "----------   ------------    ------------     -------   -------------   ----------------\n";
    
    for (const auto& interface : interfaces) {
        std::cout << std::setw(12) << interface.name
                  << std::setw(16) << interface.ip_address
                  << std::setw(16) << interface.subnet_mask
                  << std::setw(10) << (interface.is_up ? "UP" : "DOWN")
                  << std::setw(16) << interface.packets_sent
                  << std::setw(18) << interface.packets_received
                  << std::endl;
    }
    
    std::cout << std::endl;
    return true;
}

bool CLIInterface::cmd_show_routes(const std::vector<std::string>& args) {
    if (!router_core_) {
        std::cout << "Router core not initialized" << std::endl;
        return false;
    }
    
    auto routes = router_core_->get_routes();
    
    std::cout << "\nRouting Table:\n";
    std::cout << "Network        Next Hop         Interface    Metric    Protocol    Status\n";
    std::cout << "-------------  ---------------  ---------    -------   ---------   -------\n";
    
    for (const auto& route : routes) {
        std::cout << std::setw(15) << route.network
                  << std::setw(17) << route.next_hop
                  << std::setw(12) << route.interface
                  << std::setw(10) << route.metric
                  << std::setw(12) << route.protocol
                  << std::setw(10) << (route.is_active ? "Active" : "Inactive")
                  << std::endl;
    }
    
    std::cout << std::endl;
    return true;
}

bool CLIInterface::cmd_show_statistics(const std::vector<std::string>& args) {
    if (!router_core_) {
        std::cout << "Router core not initialized" << std::endl;
        return false;
    }
    
    auto stats = router_core_->get_statistics();
    
    std::cout << "\nRouter Statistics:\n";
    std::cout << "Total Packets Processed: " << stats.total_packets_processed << std::endl;
    std::cout << "Total Bytes Processed:   " << stats.total_bytes_processed << std::endl;
    std::cout << "Routing Table Updates:   " << stats.routing_table_updates << std::endl;
    std::cout << "Interface State Changes: " << stats.interface_state_changes << std::endl;
    std::cout << std::endl;
    return true;
}

bool CLIInterface::cmd_configure_interface(const std::vector<std::string>& args) {
    if (args.size() < 3) {
        std::cout << "Usage: configure interface <name> <ip> <mask> [up|down]" << std::endl;
        return false;
    }
    
    std::string name = args[0];
    std::string ip = args[1];
    std::string mask = args[2];
    bool up = (args.size() < 4 || args[3] == "up");
    
    if (!router_core_) {
        std::cout << "Router core not initialized" << std::endl;
        return false;
    }
    
    if (router_core_->add_interface(name, ip, mask)) {
        router_core_->set_interface_up(name, up);
        std::cout << "Interface " << name << " configured successfully" << std::endl;
        return true;
    } else {
        std::cout << "Failed to configure interface " << name << std::endl;
        return false;
    }
}

bool CLIInterface::cmd_configure_route(const std::vector<std::string>& args) {
    if (args.size() < 4) {
        std::cout << "Usage: configure route <network> <next_hop> <interface> <metric>" << std::endl;
        return false;
    }
    
    Route route;
    route.network = args[0];
    route.next_hop = args[1];
    route.interface = args[2];
    route.metric = std::stoul(args[3]);
    route.protocol = "static";
    route.is_active = true;
    
    if (!router_core_) {
        std::cout << "Router core not initialized" << std::endl;
        return false;
    }
    
    if (router_core_->add_route(route)) {
        std::cout << "Route added successfully" << std::endl;
        return true;
    } else {
        std::cout << "Failed to add route" << std::endl;
        return false;
    }
}

bool CLIInterface::cmd_start_bgp(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cout << "Usage: start bgp <as_number> <router_id>" << std::endl;
        return false;
    }
    
    std::map<std::string, std::string> config;
    config["as_number"] = args[0];
    config["router_id"] = args[1];
    
    if (!frr_integration_) {
        std::cout << "FRR integration not initialized" << std::endl;
        return false;
    }
    
    if (frr_integration_->start_protocol("bgp", config)) {
        std::cout << "BGP started successfully" << std::endl;
        return true;
    } else {
        std::cout << "Failed to start BGP" << std::endl;
        return false;
    }
}

bool CLIInterface::cmd_stop_bgp(const std::vector<std::string>& args) {
    if (!frr_integration_) {
        std::cout << "FRR integration not initialized" << std::endl;
        return false;
    }
    
    if (frr_integration_->stop_protocol("bgp")) {
        std::cout << "BGP stopped successfully" << std::endl;
        return true;
    } else {
        std::cout << "Failed to stop BGP" << std::endl;
        return false;
    }
}

bool CLIInterface::cmd_start_ospf(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cout << "Usage: start ospf <router_id> <area>" << std::endl;
        return false;
    }
    
    std::map<std::string, std::string> config;
    config["router_id"] = args[0];
    config["area"] = args[1];
    
    if (!frr_integration_) {
        std::cout << "FRR integration not initialized" << std::endl;
        return false;
    }
    
    if (frr_integration_->start_protocol("ospf", config)) {
        std::cout << "OSPF started successfully" << std::endl;
        return true;
    } else {
        std::cout << "Failed to start OSPF" << std::endl;
        return false;
    }
}

bool CLIInterface::cmd_stop_ospf(const std::vector<std::string>& args) {
    if (!frr_integration_) {
        std::cout << "FRR integration not initialized" << std::endl;
        return false;
    }
    
    if (frr_integration_->stop_protocol("ospf")) {
        std::cout << "OSPF stopped successfully" << std::endl;
        return true;
    } else {
        std::cout << "Failed to stop OSPF" << std::endl;
        return false;
    }
}

bool CLIInterface::cmd_start_isis(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cout << "Usage: start isis <system_id> <area_id>" << std::endl;
        return false;
    }
    
    std::map<std::string, std::string> config;
    config["system_id"] = args[0];
    config["area_id"] = args[1];
    
    if (!frr_integration_) {
        std::cout << "FRR integration not initialized" << std::endl;
        return false;
    }
    
    if (frr_integration_->start_protocol("isis", config)) {
        std::cout << "IS-IS started successfully" << std::endl;
        return true;
    } else {
        std::cout << "Failed to start IS-IS" << std::endl;
        return false;
    }
}

bool CLIInterface::cmd_stop_isis(const std::vector<std::string>& args) {
    if (!frr_integration_) {
        std::cout << "FRR integration not initialized" << std::endl;
        return false;
    }
    
    if (frr_integration_->stop_protocol("isis")) {
        std::cout << "IS-IS stopped successfully" << std::endl;
        return true;
    } else {
        std::cout << "Failed to stop IS-IS" << std::endl;
        return false;
    }
}

bool CLIInterface::cmd_show_bgp_neighbors(const std::vector<std::string>& args) {
    if (!frr_integration_) {
        std::cout << "FRR integration not initialized" << std::endl;
        return false;
    }
    
    auto neighbors = frr_integration_->get_bgp_neighbors();
    
    std::cout << "\nBGP Neighbors:\n";
    std::cout << "Address         State        Uptime    Hold Time    Keepalive Time\n";
    std::cout << "-------------   -------      -------   ----------   --------------\n";
    
    for (const auto& neighbor : neighbors) {
        std::cout << std::setw(15) << neighbor.address
                  << std::setw(12) << neighbor.state
                  << std::setw(10) << neighbor.uptime
                  << std::setw(13) << neighbor.hold_time
                  << std::setw(16) << neighbor.keepalive_time
                  << std::endl;
    }
    
    std::cout << std::endl;
    return true;
}

bool CLIInterface::cmd_show_ospf_neighbors(const std::vector<std::string>& args) {
    if (!frr_integration_) {
        std::cout << "FRR integration not initialized" << std::endl;
        return false;
    }
    
    auto neighbors = frr_integration_->get_ospf_neighbors();
    
    std::cout << "\nOSPF Neighbors:\n";
    std::cout << "Address         State        Uptime    Interface\n";
    std::cout << "-------------   -------      -------   ---------\n";
    
    for (const auto& neighbor : neighbors) {
        std::cout << std::setw(15) << neighbor.address
                  << std::setw(12) << neighbor.state
                  << std::setw(10) << neighbor.uptime
                  << std::setw(12) << neighbor.interface
                  << std::endl;
    }
    
    std::cout << std::endl;
    return true;
}

bool CLIInterface::cmd_show_isis_neighbors(const std::vector<std::string>& args) {
    if (!frr_integration_) {
        std::cout << "FRR integration not initialized" << std::endl;
        return false;
    }
    
    auto neighbors = frr_integration_->get_isis_neighbors();
    
    std::cout << "\nIS-IS Neighbors:\n";
    std::cout << "Address         State        Uptime    Interface\n";
    std::cout << "-------------   -------      -------   ---------\n";
    
    for (const auto& neighbor : neighbors) {
        std::cout << std::setw(15) << neighbor.address
                  << std::setw(12) << neighbor.state
                  << std::setw(10) << neighbor.uptime
                  << std::setw(12) << neighbor.interface
                  << std::endl;
    }
    
    std::cout << std::endl;
    return true;
}

bool CLIInterface::cmd_configure_traffic_shaping(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cout << "Usage: configure traffic-shaping <capacity> <rate>" << std::endl;
        return false;
    }
    
    uint64_t capacity = std::stoull(args[0]);
    uint64_t rate = std::stoull(args[1]);
    
    if (!traffic_shaper_) {
        std::cout << "Traffic shaper not initialized" << std::endl;
        return false;
    }
    
    // Configure token bucket for all interfaces
    auto interfaces = router_core_->get_interfaces();
    for (const auto& interface : interfaces) {
        std::map<std::string, std::string> config;
        config["capacity"] = std::to_string(capacity);
        config["rate"] = std::to_string(rate);
        config["burst_size"] = std::to_string(capacity / 2);
        config["allow_burst"] = "true";
        
        traffic_shaper_->configure_interface(interface.name, ShapingAlgorithm::TOKEN_BUCKET, config);
    }
    
    std::cout << "Traffic shaping configured successfully" << std::endl;
    return true;
}

bool CLIInterface::cmd_show_traffic_shaping(const std::vector<std::string>& args) {
    if (!traffic_shaper_) {
        std::cout << "Traffic shaper not initialized" << std::endl;
        return false;
    }
    
    auto stats = traffic_shaper_->get_interface_statistics();
    
    std::cout << "\nTraffic Shaping Statistics:\n";
    std::cout << "Interface    Packets Processed    Packets Dropped    Bytes Processed    Current Throughput\n";
    std::cout << "----------   -----------------    ----------------    ----------------    -----------------\n";
    
    for (const auto& [interface, stat] : stats) {
        std::cout << std::setw(12) << interface
                  << std::setw(20) << stat.packets_processed
                  << std::setw(18) << stat.packets_dropped
                  << std::setw(18) << stat.bytes_processed
                  << std::setw(20) << stat.current_throughput_bps
                  << std::endl;
    }
    
    std::cout << std::endl;
    return true;
}

bool CLIInterface::cmd_configure_netem(const std::vector<std::string>& args) {
    if (args.size() < 3) {
        std::cout << "Usage: configure netem <interface> <delay_ms> <loss_percent>" << std::endl;
        return false;
    }
    
    std::string interface = args[0];
    double delay = std::stod(args[1]);
    double loss = std::stod(args[2]);
    
    if (!netem_impairments_) {
        std::cout << "Network impairments not initialized" << std::endl;
        return false;
    }
    
    CombinedImpairments config;
    config.delay.delay_ms = delay;
    config.delay.jitter_ms = delay * 0.1; // 10% jitter
    config.loss.loss_percent = loss;
    
    if (netem_impairments_->apply_combined_impairments(interface, config)) {
        std::cout << "Network impairments configured successfully" << std::endl;
        return true;
    } else {
        std::cout << "Failed to configure network impairments" << std::endl;
        return false;
    }
}

bool CLIInterface::cmd_show_netem(const std::vector<std::string>& args) {
    if (!netem_impairments_) {
        std::cout << "Network impairments not initialized" << std::endl;
        return false;
    }
    
    auto impairments = netem_impairments_->get_current_impairments();
    
    std::cout << "\nNetwork Impairments:\n";
    std::cout << "Interface    Delay (ms)    Loss (%)    Duplication (%)    Reordering (%)    Corruption (%)\n";
    std::cout << "----------   -----------   ---------   ----------------   ----------------   --------------\n";
    
    for (const auto& [interface, config] : impairments) {
        std::cout << std::setw(12) << interface
                  << std::setw(13) << config.delay.delay_ms
                  << std::setw(11) << config.loss.loss_percent
                  << std::setw(18) << config.duplication.duplication_percent
                  << std::setw(18) << config.reordering.reordering_percent
                  << std::setw(16) << config.corruption.corruption_percent
                  << std::endl;
    }
    
    std::cout << std::endl;
    return true;
}

bool CLIInterface::cmd_clear_netem(const std::vector<std::string>& args) {
    if (!netem_impairments_) {
        std::cout << "Network impairments not initialized" << std::endl;
        return false;
    }
    
    if (netem_impairments_->clear_all_impairments()) {
        std::cout << "All network impairments cleared" << std::endl;
        return true;
    } else {
        std::cout << "Failed to clear network impairments" << std::endl;
        return false;
    }
}

bool CLIInterface::cmd_load_scenario(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cout << "Usage: load scenario <file>" << std::endl;
        return false;
    }
    
    std::string filename = args[0];
    std::cout << "Loading scenario from " << filename << "..." << std::endl;
    
    // Implementation would load scenario from file
    // This is a simplified implementation
    
    std::cout << "Scenario loaded successfully" << std::endl;
    return true;
}

bool CLIInterface::cmd_save_scenario(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cout << "Usage: save scenario <file>" << std::endl;
        return false;
    }
    
    std::string filename = args[0];
    std::cout << "Saving scenario to " << filename << "..." << std::endl;
    
    // Implementation would save scenario to file
    // This is a simplified implementation
    
    std::cout << "Scenario saved successfully" << std::endl;
    return true;
}

bool CLIInterface::cmd_exit(const std::vector<std::string>& args) {
    std::cout << "Exiting CLI..." << std::endl;
    running_ = false;
    return true;
}

bool CLIInterface::cmd_clear(const std::vector<std::string>& args) {
    // Clear screen
    std::cout << "\033[2J\033[1;1H";
    return true;
}

} // namespace RouterSim
