#include "cli_interface.h"
#include "router_core.h"
#include "frr_integration.h"
#include "traffic_shaping.h"
#include "network_impairments.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <chrono>
#include <thread>

namespace RouterSim {

CLIInterface::CLIInterface() : running_(false), router_core_(nullptr) {
    // Initialize command handlers
    initialize_commands();
}

CLIInterface::~CLIInterface() {
    stop();
}

bool CLIInterface::initialize(RouterCore* router_core) {
    router_core_ = router_core;
    
    if (!router_core_) {
        std::cerr << "Error: Router core not provided" << std::endl;
        return false;
    }
    
    std::cout << "Router Simulator CLI initialized" << std::endl;
    return true;
}

bool CLIInterface::start() {
    if (running_) {
        return true;
    }
    
    running_ = true;
    std::cout << "Router Simulator CLI started. Type 'help' for available commands." << std::endl;
    
    // Start CLI loop in a separate thread
    cli_thread_ = std::thread(&CLIInterface::cli_loop, this);
    
    return true;
}

bool CLIInterface::stop() {
    if (!running_) {
        return true;
    }
    
    running_ = false;
    
    if (cli_thread_.joinable()) {
        cli_thread_.join();
    }
    
    std::cout << "Router Simulator CLI stopped" << std::endl;
    return true;
}

void CLIInterface::initialize_commands() {
    // System commands
    commands_["help"] = [this](const std::vector<std::string>& args) { return cmd_help(args); };
    commands_["exit"] = [this](const std::vector<std::string>& args) { return cmd_exit(args); };
    commands_["quit"] = [this](const std::vector<std::string>& args) { return cmd_exit(args); };
    commands_["clear"] = [this](const std::vector<std::string>& args) { return cmd_clear(args); };
    commands_["status"] = [this](const std::vector<std::string>& args) { return cmd_status(args); };
    
    // Router commands
    commands_["show"] = [this](const std::vector<std::string>& args) { return cmd_show(args); };
    commands_["configure"] = [this](const std::vector<std::string>& args) { return cmd_configure(args); };
    commands_["start"] = [this](const std::vector<std::string>& args) { return cmd_start(args); };
    commands_["stop"] = [this](const std::vector<std::string>& args) { return cmd_stop(args); };
    commands_["restart"] = [this](const std::vector<std::string>& args) { return cmd_restart(args); };
    
    // Protocol commands
    commands_["bgp"] = [this](const std::vector<std::string>& args) { return cmd_bgp(args); };
    commands_["ospf"] = [this](const std::vector<std::string>& args) { return cmd_ospf(args); };
    commands_["isis"] = [this](const std::vector<std::string>& args) { return cmd_isis(args); };
    
    // Traffic shaping commands
    commands_["traffic"] = [this](const std::vector<std::string>& args) { return cmd_traffic(args); };
    commands_["qos"] = [this](const std::vector<std::string>& args) { return cmd_qos(args); };
    
    // Impairment commands
    commands_["impairment"] = [this](const std::vector<std::string>& args) { return cmd_impairment(args); };
    commands_["netem"] = [this](const std::vector<std::string>& args) { return cmd_netem(args); };
    
    // Scenario commands
    commands_["scenario"] = [this](const std::vector<std::string>& args) { return cmd_scenario(args); };
    commands_["load"] = [this](const std::vector<std::string>& args) { return cmd_load(args); };
    commands_["save"] = [this](const std::vector<std::string>& args) { return cmd_save(args); };
    
    // Cloud commands
    commands_["cloud"] = [this](const std::vector<std::string>& args) { return cmd_cloud(args); };
    commands_["cloudpods"] = [this](const std::vector<std::string>& args) { return cmd_cloudpods(args); };
    commands_["aviatrix"] = [this](const std::vector<std::string>& args) { return cmd_aviatrix(args); };
    
    // Analytics commands
    commands_["analytics"] = [this](const std::vector<std::string>& args) { return cmd_analytics(args); };
    commands_["metrics"] = [this](const std::vector<std::string>& args) { return cmd_metrics(args); };
    
    // Testing commands
    commands_["test"] = [this](const std::vector<std::string>& args) { return cmd_test(args); };
    commands_["pcap"] = [this](const std::vector<std::string>& args) { return cmd_pcap(args); };
}

void CLIInterface::cli_loop() {
    std::string line;
    
    while (running_) {
        std::cout << "router-sim> ";
        std::cout.flush();
        
        if (!std::getline(std::cin, line)) {
            break;
        }
        
        // Parse command
        std::vector<std::string> args = parse_command(line);
        
        if (args.empty()) {
            continue;
        }
        
        // Execute command
        std::string command = args[0];
        args.erase(args.begin());
        
        auto it = commands_.find(command);
        if (it != commands_.end()) {
            try {
                bool result = it->second(args);
                if (!result) {
                    std::cout << "Command failed" << std::endl;
                }
            } catch (const std::exception& e) {
                std::cout << "Error: " << e.what() << std::endl;
            }
        } else {
            std::cout << "Unknown command: " << command << std::endl;
            std::cout << "Type 'help' for available commands" << std::endl;
        }
    }
}

std::vector<std::string> CLIInterface::parse_command(const std::string& line) {
    std::vector<std::string> args;
    std::istringstream iss(line);
    std::string arg;
    
    while (iss >> arg) {
        args.push_back(arg);
    }
    
    return args;
}

// Command implementations
bool CLIInterface::cmd_help(const std::vector<std::string>& args) {
    std::cout << "\nRouter Simulator CLI - Available Commands:\n" << std::endl;
    
    std::cout << "System Commands:" << std::endl;
    std::cout << "  help                    Show this help message" << std::endl;
    std::cout << "  exit, quit              Exit the CLI" << std::endl;
    std::cout << "  clear                   Clear the screen" << std::endl;
    std::cout << "  status                  Show system status" << std::endl;
    
    std::cout << "\nRouter Commands:" << std::endl;
    std::cout << "  show <item>             Show router information" << std::endl;
    std::cout << "  configure <item>        Configure router settings" << std::endl;
    std::cout << "  start <service>         Start a service" << std::endl;
    std::cout << "  stop <service>          Stop a service" << std::endl;
    std::cout << "  restart <service>       Restart a service" << std::endl;
    
    std::cout << "\nProtocol Commands:" << std::endl;
    std::cout << "  bgp <action>            BGP protocol operations" << std::endl;
    std::cout << "  ospf <action>           OSPF protocol operations" << std::endl;
    std::cout << "  isis <action>           IS-IS protocol operations" << std::endl;
    
    std::cout << "\nTraffic Shaping Commands:" << std::endl;
    std::cout << "  traffic <action>        Traffic shaping operations" << std::endl;
    std::cout << "  qos <action>            QoS configuration" << std::endl;
    
    std::cout << "\nImpairment Commands:" << std::endl;
    std::cout << "  impairment <action>     Network impairment operations" << std::endl;
    std::cout << "  netem <action>          NetEm operations" << std::endl;
    
    std::cout << "\nScenario Commands:" << std::endl;
    std::cout << "  scenario <action>       Scenario management" << std::endl;
    std::cout << "  load <file>             Load scenario from file" << std::endl;
    std::cout << "  save <file>             Save scenario to file" << std::endl;
    
    std::cout << "\nCloud Commands:" << std::endl;
    std::cout << "  cloud <action>          Cloud operations" << std::endl;
    std::cout << "  cloudpods <action>      CloudPods operations" << std::endl;
    std::cout << "  aviatrix <action>       Aviatrix operations" << std::endl;
    
    std::cout << "\nAnalytics Commands:" << std::endl;
    std::cout << "  analytics <action>      Analytics operations" << std::endl;
    std::cout << "  metrics <action>        Metrics operations" << std::endl;
    
    std::cout << "\nTesting Commands:" << std::endl;
    std::cout << "  test <action>           Testing operations" << std::endl;
    std::cout << "  pcap <action>           PCAP operations" << std::endl;
    
    std::cout << "\nFor detailed help on a specific command, use: <command> help" << std::endl;
    
    return true;
}

bool CLIInterface::cmd_exit(const std::vector<std::string>& args) {
    running_ = false;
    return true;
}

bool CLIInterface::cmd_clear(const std::vector<std::string>& args) {
    std::cout << "\033[2J\033[1;1H";
    return true;
}

bool CLIInterface::cmd_status(const std::vector<std::string>& args) {
    if (!router_core_) {
        std::cout << "Router core not available" << std::endl;
        return false;
    }
    
    std::cout << "\nRouter Simulator Status:" << std::endl;
    std::cout << "========================" << std::endl;
    
    // System status
    std::cout << "System Status: " << (router_core_->is_running() ? "Running" : "Stopped") << std::endl;
    
    // Protocol status
    std::cout << "\nProtocol Status:" << std::endl;
    std::cout << "  BGP:  " << (router_core_->is_protocol_running("bgp") ? "Running" : "Stopped") << std::endl;
    std::cout << "  OSPF: " << (router_core_->is_protocol_running("ospf") ? "Running" : "Stopped") << std::endl;
    std::cout << "  IS-IS:" << (router_core_->is_protocol_running("isis") ? "Running" : "Stopped") << std::endl;
    
    // Traffic shaping status
    std::cout << "\nTraffic Shaping: " << (router_core_->is_traffic_shaping_enabled() ? "Enabled" : "Disabled") << std::endl;
    
    // Impairment status
    std::cout << "Network Impairments: " << (router_core_->has_active_impairments() ? "Active" : "None") << std::endl;
    
    // Statistics
    auto stats = router_core_->get_statistics();
    std::cout << "\nStatistics:" << std::endl;
    std::cout << "  Packets Processed: " << stats.packets_processed << std::endl;
    std::cout << "  Bytes Processed:   " << stats.bytes_processed << std::endl;
    std::cout << "  Packets Dropped:   " << stats.packets_dropped << std::endl;
    std::cout << "  Bytes Dropped:     " << stats.bytes_dropped << std::endl;
    
    return true;
}

bool CLIInterface::cmd_show(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cout << "Usage: show <item>" << std::endl;
        std::cout << "Available items: routes, neighbors, interfaces, protocols, traffic, impairments" << std::endl;
        return false;
    }
    
    std::string item = args[0];
    
    if (item == "routes") {
        return show_routes();
    } else if (item == "neighbors") {
        return show_neighbors();
    } else if (item == "interfaces") {
        return show_interfaces();
    } else if (item == "protocols") {
        return show_protocols();
    } else if (item == "traffic") {
        return show_traffic();
    } else if (item == "impairments") {
        return show_impairments();
    } else {
        std::cout << "Unknown item: " << item << std::endl;
        return false;
    }
}

bool CLIInterface::show_routes() {
    if (!router_core_) {
        std::cout << "Router core not available" << std::endl;
        return false;
    }
    
    auto routes = router_core_->get_routes();
    
    std::cout << "\nRouting Table:" << std::endl;
    std::cout << "==============" << std::endl;
    std::cout << std::setw(20) << "Destination" 
              << std::setw(15) << "Next Hop" 
              << std::setw(10) << "Metric" 
              << std::setw(10) << "Protocol" 
              << std::setw(15) << "Timestamp" << std::endl;
    std::cout << std::string(80, '-') << std::endl;
    
    for (const auto& route : routes) {
        std::cout << std::setw(20) << route.prefix
                  << std::setw(15) << route.next_hop
                  << std::setw(10) << route.metric
                  << std::setw(10) << route.protocol
                  << std::setw(15) << "N/A" << std::endl;
    }
    
    std::cout << "\nTotal routes: " << routes.size() << std::endl;
    return true;
}

bool CLIInterface::show_neighbors() {
    if (!router_core_) {
        std::cout << "Router core not available" << std::endl;
        return false;
    }
    
    auto neighbors = router_core_->get_neighbors();
    
    std::cout << "\nNeighbor Table:" << std::endl;
    std::cout << "===============" << std::endl;
    std::cout << std::setw(20) << "Address" 
              << std::setw(15) << "State" 
              << std::setw(10) << "Protocol" 
              << std::setw(15) << "Established" << std::endl;
    std::cout << std::string(70, '-') << std::endl;
    
    for (const auto& neighbor : neighbors) {
        std::cout << std::setw(20) << neighbor.address
                  << std::setw(15) << neighbor.state
                  << std::setw(10) << neighbor.protocol
                  << std::setw(15) << "N/A" << std::endl;
    }
    
    std::cout << "\nTotal neighbors: " << neighbors.size() << std::endl;
    return true;
}

bool CLIInterface::show_interfaces() {
    std::cout << "\nInterface Table:" << std::endl;
    std::cout << "================" << std::endl;
    std::cout << std::setw(15) << "Interface" 
              << std::setw(15) << "Status" 
              << std::setw(15) << "IP Address" 
              << std::setw(10) << "MTU" << std::endl;
    std::cout << std::string(60, '-') << std::endl;
    
    // This would be implemented to show actual interface information
    std::cout << std::setw(15) << "eth0"
              << std::setw(15) << "Up"
              << std::setw(15) << "192.168.1.1"
              << std::setw(10) << "1500" << std::endl;
    
    return true;
}

bool CLIInterface::show_protocols() {
    if (!router_core_) {
        std::cout << "Router core not available" << std::endl;
        return false;
    }
    
    std::cout << "\nProtocol Status:" << std::endl;
    std::cout << "================" << std::endl;
    
    std::vector<std::string> protocols = {"bgp", "ospf", "isis"};
    
    for (const auto& protocol : protocols) {
        bool running = router_core_->is_protocol_running(protocol);
        std::cout << std::setw(10) << protocol << ": " 
                  << (running ? "Running" : "Stopped") << std::endl;
    }
    
    return true;
}

bool CLIInterface::show_traffic() {
    if (!router_core_) {
        std::cout << "Router core not available" << std::endl;
        return false;
    }
    
    auto stats = router_core_->get_traffic_shaping_statistics();
    
    std::cout << "\nTraffic Shaping Statistics:" << std::endl;
    std::cout << "===========================" << std::endl;
    std::cout << "Enabled: " << (stats.enabled ? "Yes" : "No") << std::endl;
    std::cout << "Packets Processed: " << stats.total_packets_processed << std::endl;
    std::cout << "Bytes Processed: " << stats.total_bytes_processed << std::endl;
    std::cout << "Packets Dropped: " << stats.packets_dropped << std::endl;
    std::cout << "Bytes Dropped: " << stats.bytes_dropped << std::endl;
    
    return true;
}

bool CLIInterface::show_impairments() {
    if (!router_core_) {
        std::cout << "Router core not available" << std::endl;
        return false;
    }
    
    auto impairments = router_core_->get_active_impairments();
    
    std::cout << "\nActive Impairments:" << std::endl;
    std::cout << "===================" << std::endl;
    
    if (impairments.empty()) {
        std::cout << "No active impairments" << std::endl;
    } else {
        for (const auto& impairment : impairments) {
            std::cout << "Interface: " << impairment.interface << std::endl;
            std::cout << "Type: " << impairment.type << std::endl;
            std::cout << "Status: " << (impairment.enabled ? "Active" : "Inactive") << std::endl;
            std::cout << "---" << std::endl;
        }
    }
    
    return true;
}

bool CLIInterface::cmd_bgp(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cout << "Usage: bgp <action>" << std::endl;
        std::cout << "Actions: start, stop, restart, show, neighbors, routes" << std::endl;
        return false;
    }
    
    std::string action = args[0];
    
    if (action == "start") {
        return router_core_->start_protocol("bgp");
    } else if (action == "stop") {
        return router_core_->stop_protocol("bgp");
    } else if (action == "restart") {
        router_core_->stop_protocol("bgp");
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        return router_core_->start_protocol("bgp");
    } else if (action == "show") {
        return show_protocols();
    } else if (action == "neighbors") {
        return show_neighbors();
    } else if (action == "routes") {
        return show_routes();
    } else {
        std::cout << "Unknown BGP action: " << action << std::endl;
        return false;
    }
}

bool CLIInterface::cmd_ospf(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cout << "Usage: ospf <action>" << std::endl;
        std::cout << "Actions: start, stop, restart, show, neighbors, routes" << std::endl;
        return false;
    }
    
    std::string action = args[0];
    
    if (action == "start") {
        return router_core_->start_protocol("ospf");
    } else if (action == "stop") {
        return router_core_->stop_protocol("ospf");
    } else if (action == "restart") {
        router_core_->stop_protocol("ospf");
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        return router_core_->start_protocol("ospf");
    } else if (action == "show") {
        return show_protocols();
    } else if (action == "neighbors") {
        return show_neighbors();
    } else if (action == "routes") {
        return show_routes();
    } else {
        std::cout << "Unknown OSPF action: " << action << std::endl;
        return false;
    }
}

bool CLIInterface::cmd_isis(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cout << "Usage: isis <action>" << std::endl;
        std::cout << "Actions: start, stop, restart, show, neighbors, routes" << std::endl;
        return false;
    }
    
    std::string action = args[0];
    
    if (action == "start") {
        return router_core_->start_protocol("isis");
    } else if (action == "stop") {
        return router_core_->stop_protocol("isis");
    } else if (action == "restart") {
        router_core_->stop_protocol("isis");
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        return router_core_->start_protocol("isis");
    } else if (action == "show") {
        return show_protocols();
    } else if (action == "neighbors") {
        return show_neighbors();
    } else if (action == "routes") {
        return show_routes();
    } else {
        std::cout << "Unknown IS-IS action: " << action << std::endl;
        return false;
    }
}

bool CLIInterface::cmd_traffic(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cout << "Usage: traffic <action>" << std::endl;
        std::cout << "Actions: enable, disable, show, configure" << std::endl;
        return false;
    }
    
    std::string action = args[0];
    
    if (action == "enable") {
        return router_core_->enable_traffic_shaping();
    } else if (action == "disable") {
        return router_core_->disable_traffic_shaping();
    } else if (action == "show") {
        return show_traffic();
    } else if (action == "configure") {
        std::cout << "Traffic shaping configuration not implemented yet" << std::endl;
        return false;
    } else {
        std::cout << "Unknown traffic action: " << action << std::endl;
        return false;
    }
}

bool CLIInterface::cmd_impairment(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cout << "Usage: impairment <action>" << std::endl;
        std::cout << "Actions: add, remove, show, clear" << std::endl;
        return false;
    }
    
    std::string action = args[0];
    
    if (action == "add") {
        std::cout << "Adding impairment not implemented yet" << std::endl;
        return false;
    } else if (action == "remove") {
        std::cout << "Removing impairment not implemented yet" << std::endl;
        return false;
    } else if (action == "show") {
        return show_impairments();
    } else if (action == "clear") {
        return router_core_->clear_all_impairments();
    } else {
        std::cout << "Unknown impairment action: " << action << std::endl;
        return false;
    }
}

bool CLIInterface::cmd_scenario(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cout << "Usage: scenario <action>" << std::endl;
        std::cout << "Actions: list, load, save, run" << std::endl;
        return false;
    }
    
    std::string action = args[0];
    
    if (action == "list") {
        std::cout << "Available scenarios:" << std::endl;
        std::cout << "  - basic_ospf.yaml" << std::endl;
        std::cout << "  - bgp_peering.yaml" << std::endl;
        std::cout << "  - impairment_test.yaml" << std::endl;
        return true;
    } else if (action == "load") {
        if (args.size() < 2) {
            std::cout << "Usage: scenario load <filename>" << std::endl;
            return false;
        }
        return router_core_->load_scenario(args[1]);
    } else if (action == "save") {
        if (args.size() < 2) {
            std::cout << "Usage: scenario save <filename>" << std::endl;
            return false;
        }
        return router_core_->save_scenario(args[1]);
    } else if (action == "run") {
        if (args.size() < 2) {
            std::cout << "Usage: scenario run <filename>" << std::endl;
            return false;
        }
        return router_core_->run_scenario(args[1]);
    } else {
        std::cout << "Unknown scenario action: " << action << std::endl;
        return false;
    }
}

bool CLIInterface::cmd_load(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cout << "Usage: load <filename>" << std::endl;
        return false;
    }
    
    return router_core_->load_scenario(args[0]);
}

bool CLIInterface::cmd_save(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cout << "Usage: save <filename>" << std::endl;
        return false;
    }
    
    return router_core_->save_scenario(args[0]);
}

bool CLIInterface::cmd_cloud(const std::vector<std::string>& args) {
    std::cout << "Cloud operations not implemented yet" << std::endl;
    return false;
}

bool CLIInterface::cmd_cloudpods(const std::vector<std::string>& args) {
    std::cout << "CloudPods operations not implemented yet" << std::endl;
    return false;
}

bool CLIInterface::cmd_aviatrix(const std::vector<std::string>& args) {
    std::cout << "Aviatrix operations not implemented yet" << std::endl;
    return false;
}

bool CLIInterface::cmd_analytics(const std::vector<std::string>& args) {
    std::cout << "Analytics operations not implemented yet" << std::endl;
    return false;
}

bool CLIInterface::cmd_metrics(const std::vector<std::string>& args) {
    std::cout << "Metrics operations not implemented yet" << std::endl;
    return false;
}

bool CLIInterface::cmd_test(const std::vector<std::string>& args) {
    std::cout << "Testing operations not implemented yet" << std::endl;
    return false;
}

bool CLIInterface::cmd_pcap(const std::vector<std::string>& args) {
    std::cout << "PCAP operations not implemented yet" << std::endl;
    return false;
}

bool CLIInterface::cmd_configure(const std::vector<std::string>& args) {
    std::cout << "Configuration operations not implemented yet" << std::endl;
    return false;
}

bool CLIInterface::cmd_start(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cout << "Usage: start <service>" << std::endl;
        std::cout << "Services: bgp, ospf, isis, traffic, all" << std::endl;
        return false;
    }
    
    std::string service = args[0];
    
    if (service == "all") {
        bool result = true;
        result &= router_core_->start_protocol("bgp");
        result &= router_core_->start_protocol("ospf");
        result &= router_core_->start_protocol("isis");
        result &= router_core_->enable_traffic_shaping();
        return result;
    } else {
        return router_core_->start_protocol(service);
    }
}

bool CLIInterface::cmd_stop(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cout << "Usage: stop <service>" << std::endl;
        std::cout << "Services: bgp, ospf, isis, traffic, all" << std::endl;
        return false;
    }
    
    std::string service = args[0];
    
    if (service == "all") {
        bool result = true;
        result &= router_core_->stop_protocol("bgp");
        result &= router_core_->stop_protocol("ospf");
        result &= router_core_->stop_protocol("isis");
        result &= router_core_->disable_traffic_shaping();
        return result;
    } else {
        return router_core_->stop_protocol(service);
    }
}

bool CLIInterface::cmd_restart(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cout << "Usage: restart <service>" << std::endl;
        std::cout << "Services: bgp, ospf, isis, traffic, all" << std::endl;
        return false;
    }
    
    std::string service = args[0];
    
    if (service == "all") {
        bool result = true;
        result &= router_core_->stop_protocol("bgp");
        result &= router_core_->stop_protocol("ospf");
        result &= router_core_->stop_protocol("isis");
        result &= router_core_->disable_traffic_shaping();
        
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        
        result &= router_core_->start_protocol("bgp");
        result &= router_core_->start_protocol("ospf");
        result &= router_core_->start_protocol("isis");
        result &= router_core_->enable_traffic_shaping();
        return result;
    } else {
        router_core_->stop_protocol(service);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        return router_core_->start_protocol(service);
    }
}

bool CLIInterface::cmd_qos(const std::vector<std::string>& args) {
    std::cout << "QoS operations not implemented yet" << std::endl;
    return false;
}

bool CLIInterface::cmd_netem(const std::vector<std::string>& args) {
    std::cout << "NetEm operations not implemented yet" << std::endl;
    return false;
}

} // namespace RouterSim