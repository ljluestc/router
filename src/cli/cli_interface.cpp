#include "cli/cli_interface.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <chrono>

namespace RouterSim {

CLIInterface::CLIInterface() : running_(false), router_core_(nullptr) {
    initialize_commands();
}

CLIInterface::~CLIInterface() {
    stop();
}

bool CLIInterface::initialize(RouterCore* router_core) {
    if (!router_core) {
        return false;
    }
    
    router_core_ = router_core;
    return true;
}

bool CLIInterface::start() {
    if (running_) {
        return true;
    }
    
    running_ = true;
    std::cout << "Router Simulator CLI started. Type 'help' for available commands." << std::endl;
    return true;
}

bool CLIInterface::stop() {
    if (!running_) {
        return true;
    }
    
    running_ = false;
    std::cout << "Router Simulator CLI stopped." << std::endl;
    return true;
}

bool CLIInterface::is_running() const {
    return running_;
}

void CLIInterface::run() {
    if (!running_) {
        return;
    }
    
    std::string line;
    while (running_ && std::getline(std::cin, line)) {
        if (line.empty()) {
            continue;
        }
        
        // Parse and execute command
        std::vector<std::string> tokens = parse_command(line);
        if (!tokens.empty()) {
            execute_command(tokens);
        }
        
        if (running_) {
            std::cout << "router-sim> ";
        }
    }
}

void CLIInterface::initialize_commands() {
    // Core commands
    commands_["help"] = [this](const std::vector<std::string>& args) { cmd_help(args); };
    commands_["exit"] = [this](const std::vector<std::string>& args) { cmd_exit(args); };
    commands_["quit"] = [this](const std::vector<std::string>& args) { cmd_exit(args); };
    commands_["status"] = [this](const std::vector<std::string>& args) { cmd_status(args); };
    
    // Router core commands
    commands_["start"] = [this](const std::vector<std::string>& args) { cmd_start(args); };
    commands_["stop"] = [this](const std::vector<std::string>& args) { cmd_stop(args); };
    commands_["restart"] = [this](const std::vector<std::string>& args) { cmd_restart(args); };
    
    // Interface commands
    commands_["interface"] = [this](const std::vector<std::string>& args) { cmd_interface(args); };
    commands_["show interfaces"] = [this](const std::vector<std::string>& args) { cmd_show_interfaces(args); };
    
    // Protocol commands
    commands_["protocol"] = [this](const std::vector<std::string>& args) { cmd_protocol(args); };
    commands_["show protocols"] = [this](const std::vector<std::string>& args) { cmd_show_protocols(args); };
    commands_["show routes"] = [this](const std::vector<std::string>& args) { cmd_show_routes(args); };
    commands_["show neighbors"] = [this](const std::vector<std::string>& args) { cmd_show_neighbors(args); };
    
    // BGP commands
    commands_["bgp"] = [this](const std::vector<std::string>& args) { cmd_bgp(args); };
    commands_["show bgp"] = [this](const std::vector<std::string>& args) { cmd_show_bgp(args); };
    commands_["show bgp routes"] = [this](const std::vector<std::string>& args) { cmd_show_bgp_routes(args); };
    commands_["show bgp neighbors"] = [this](const std::vector<std::string>& args) { cmd_show_bgp_neighbors(args); };
    
    // OSPF commands
    commands_["ospf"] = [this](const std::vector<std::string>& args) { cmd_ospf(args); };
    commands_["show ospf"] = [this](const std::vector<std::string>& args) { cmd_show_ospf(args); };
    commands_["show ospf routes"] = [this](const std::vector<std::string>& args) { cmd_show_ospf_routes(args); };
    commands_["show ospf neighbors"] = [this](const std::vector<std::string>& args) { cmd_show_ospf_neighbors(args); };
    
    // IS-IS commands
    commands_["isis"] = [this](const std::vector<std::string>& args) { cmd_isis(args); };
    commands_["show isis"] = [this](const std::vector<std::string>& args) { cmd_show_isis(args); };
    commands_["show isis routes"] = [this](const std::vector<std::string>& args) { cmd_show_isis_routes(args); };
    commands_["show isis neighbors"] = [this](const std::vector<std::string>& args) { cmd_show_isis_neighbors(args); };
    
    // Traffic shaping commands
    commands_["traffic"] = [this](const std::vector<std::string>& args) { cmd_traffic(args); };
    commands_["show traffic"] = [this](const std::vector<std::string>& args) { cmd_show_traffic(args); };
    
    // Netem commands
    commands_["netem"] = [this](const std::vector<std::string>& args) { cmd_netem(args); };
    commands_["show netem"] = [this](const std::vector<std::string>& args) { cmd_show_netem(args); };
    
    // Configuration commands
    commands_["configure"] = [this](const std::vector<std::string>& args) { cmd_configure(args); };
    commands_["load config"] = [this](const std::vector<std::string>& args) { cmd_load_config(args); };
    commands_["save config"] = [this](const std::vector<std::string>& args) { cmd_save_config(args); };
    
    // Testing commands
    commands_["test"] = [this](const std::vector<std::string>& args) { cmd_test(args); };
    commands_["capture"] = [this](const std::vector<std::string>& args) { cmd_capture(args); };
    commands_["compare"] = [this](const std::vector<std::string>& args) { cmd_compare(args); };
}

std::vector<std::string> CLIInterface::parse_command(const std::string& line) {
    std::vector<std::string> tokens;
    std::istringstream iss(line);
    std::string token;
    
    while (iss >> token) {
        tokens.push_back(token);
    }
    
    return tokens;
}

void CLIInterface::execute_command(const std::vector<std::string>& tokens) {
    if (tokens.empty()) {
        return;
    }
    
    std::string command = tokens[0];
    
    // Handle multi-word commands
    if (tokens.size() > 1) {
        std::string multi_word = tokens[0] + " " + tokens[1];
        if (commands_.find(multi_word) != commands_.end()) {
            command = multi_word;
            tokens.erase(tokens.begin(), tokens.begin() + 2);
        } else {
            tokens.erase(tokens.begin());
        }
    } else {
        tokens.erase(tokens.begin());
    }
    
    auto it = commands_.find(command);
    if (it != commands_.end()) {
        it->second(tokens);
    } else {
        std::cout << "Unknown command: " << command << std::endl;
        std::cout << "Type 'help' for available commands." << std::endl;
    }
}

// Command implementations
void CLIInterface::cmd_help(const std::vector<std::string>& args) {
    std::cout << "Router Simulator CLI Commands:" << std::endl;
    std::cout << "===============================" << std::endl;
    std::cout << "Core Commands:" << std::endl;
    std::cout << "  help                    - Show this help message" << std::endl;
    std::cout << "  exit/quit               - Exit the CLI" << std::endl;
    std::cout << "  status                  - Show router status" << std::endl;
    std::cout << "  start                   - Start the router" << std::endl;
    std::cout << "  stop                    - Stop the router" << std::endl;
    std::cout << "  restart                 - Restart the router" << std::endl;
    std::cout << std::endl;
    std::cout << "Interface Commands:" << std::endl;
    std::cout << "  interface <name> <ip>   - Add interface" << std::endl;
    std::cout << "  show interfaces         - Show all interfaces" << std::endl;
    std::cout << std::endl;
    std::cout << "Protocol Commands:" << std::endl;
    std::cout << "  protocol <name> <cmd>   - Configure protocol" << std::endl;
    std::cout << "  show protocols          - Show protocol status" << std::endl;
    std::cout << "  show routes             - Show routing table" << std::endl;
    std::cout << "  show neighbors          - Show neighbors" << std::endl;
    std::cout << std::endl;
    std::cout << "BGP Commands:" << std::endl;
    std::cout << "  bgp <cmd>               - BGP configuration" << std::endl;
    std::cout << "  show bgp                - Show BGP status" << std::endl;
    std::cout << "  show bgp routes         - Show BGP routes" << std::endl;
    std::cout << "  show bgp neighbors      - Show BGP neighbors" << std::endl;
    std::cout << std::endl;
    std::cout << "OSPF Commands:" << std::endl;
    std::cout << "  ospf <cmd>              - OSPF configuration" << std::endl;
    std::cout << "  show ospf               - Show OSPF status" << std::endl;
    std::cout << "  show ospf routes        - Show OSPF routes" << std::endl;
    std::cout << "  show ospf neighbors     - Show OSPF neighbors" << std::endl;
    std::cout << std::endl;
    std::cout << "IS-IS Commands:" << std::endl;
    std::cout << "  isis <cmd>              - IS-IS configuration" << std::endl;
    std::cout << "  show isis               - Show IS-IS status" << std::endl;
    std::cout << "  show isis routes        - Show IS-IS routes" << std::endl;
    std::cout << "  show isis neighbors     - Show IS-IS neighbors" << std::endl;
    std::cout << std::endl;
    std::cout << "Traffic Shaping Commands:" << std::endl;
    std::cout << "  traffic <cmd>           - Traffic shaping configuration" << std::endl;
    std::cout << "  show traffic            - Show traffic statistics" << std::endl;
    std::cout << std::endl;
    std::cout << "Netem Commands:" << std::endl;
    std::cout << "  netem <cmd>             - Network impairments" << std::endl;
    std::cout << "  show netem              - Show impairments" << std::endl;
    std::cout << std::endl;
    std::cout << "Configuration Commands:" << std::endl;
    std::cout << "  configure <cmd>         - Configuration mode" << std::endl;
    std::cout << "  load config <file>      - Load configuration" << std::endl;
    std::cout << "  save config <file>      - Save configuration" << std::endl;
    std::cout << std::endl;
    std::cout << "Testing Commands:" << std::endl;
    std::cout << "  test <cmd>              - Run tests" << std::endl;
    std::cout << "  capture <interface>     - Start packet capture" << std::endl;
    std::cout << "  compare <file1> <file2> - Compare PCAP files" << std::endl;
}

void CLIInterface::cmd_exit(const std::vector<std::string>& args) {
    running_ = false;
    std::cout << "Goodbye!" << std::endl;
}

void CLIInterface::cmd_status(const std::vector<std::string>& args) {
    if (!router_core_) {
        std::cout << "Router core not initialized" << std::endl;
        return;
    }
    
    std::cout << "Router Status:" << std::endl;
    std::cout << "=============" << std::endl;
    std::cout << "Initialized: " << (router_core_->is_initialized() ? "Yes" : "No") << std::endl;
    std::cout << "Running: " << (router_core_->is_running() ? "Yes" : "No") << std::endl;
    
    if (router_core_->is_running()) {
        auto interfaces = router_core_->get_interfaces();
        std::cout << "Interfaces: " << interfaces.size() << std::endl;
        
        auto routes = router_core_->get_routes();
        std::cout << "Routes: " << routes.size() << std::endl;
    }
}

void CLIInterface::cmd_start(const std::vector<std::string>& args) {
    if (!router_core_) {
        std::cout << "Router core not initialized" << std::endl;
        return;
    }
    
    if (router_core_->start()) {
        std::cout << "Router started successfully" << std::endl;
    } else {
        std::cout << "Failed to start router" << std::endl;
    }
}

void CLIInterface::cmd_stop(const std::vector<std::string>& args) {
    if (!router_core_) {
        std::cout << "Router core not initialized" << std::endl;
        return;
    }
    
    if (router_core_->stop()) {
        std::cout << "Router stopped successfully" << std::endl;
    } else {
        std::cout << "Failed to stop router" << std::endl;
    }
}

void CLIInterface::cmd_restart(const std::vector<std::string>& args) {
    cmd_stop(args);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    cmd_start(args);
}

void CLIInterface::cmd_interface(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cout << "Usage: interface <name> <ip/mask>" << std::endl;
        return;
    }
    
    if (!router_core_) {
        std::cout << "Router core not initialized" << std::endl;
        return;
    }
    
    std::string name = args[0];
    std::string ip_mask = args[1];
    
    if (router_core_->add_interface(name, ip_mask)) {
        std::cout << "Interface " << name << " added with " << ip_mask << std::endl;
    } else {
        std::cout << "Failed to add interface " << name << std::endl;
    }
}

void CLIInterface::cmd_show_interfaces(const std::vector<std::string>& args) {
    if (!router_core_) {
        std::cout << "Router core not initialized" << std::endl;
        return;
    }
    
    auto interfaces = router_core_->get_interfaces();
    
    std::cout << "Interfaces:" << std::endl;
    std::cout << "===========" << std::endl;
    std::cout << std::setw(10) << "Name" << std::setw(20) << "IP Address" 
              << std::setw(10) << "Status" << std::endl;
    std::cout << std::string(40, '-') << std::endl;
    
    for (const auto& interface : interfaces) {
        std::cout << std::setw(10) << interface.name 
                  << std::setw(20) << interface.ip_address
                  << std::setw(10) << (interface.is_up ? "Up" : "Down") << std::endl;
    }
}

void CLIInterface::cmd_protocol(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cout << "Usage: protocol <name> <command>" << std::endl;
        return;
    }
    
    std::string protocol_name = args[0];
    std::vector<std::string> cmd_args(args.begin() + 1, args.end());
    
    if (protocol_name == "bgp") {
        cmd_bgp(cmd_args);
    } else if (protocol_name == "ospf") {
        cmd_ospf(cmd_args);
    } else if (protocol_name == "isis") {
        cmd_isis(cmd_args);
    } else {
        std::cout << "Unknown protocol: " << protocol_name << std::endl;
    }
}

void CLIInterface::cmd_show_protocols(const std::vector<std::string>& args) {
    if (!router_core_) {
        std::cout << "Router core not initialized" << std::endl;
        return;
    }
    
    std::cout << "Protocol Status:" << std::endl;
    std::cout << "===============" << std::endl;
    
    // This would need to be implemented in RouterCore
    std::cout << "BGP: Not implemented" << std::endl;
    std::cout << "OSPF: Not implemented" << std::endl;
    std::cout << "IS-IS: Not implemented" << std::endl;
}

void CLIInterface::cmd_show_routes(const std::vector<std::string>& args) {
    if (!router_core_) {
        std::cout << "Router core not initialized" << std::endl;
        return;
    }
    
    auto routes = router_core_->get_routes();
    
    std::cout << "Routing Table:" << std::endl;
    std::cout << "==============" << std::endl;
    std::cout << std::setw(20) << "Destination" << std::setw(15) << "Next Hop" 
              << std::setw(10) << "Protocol" << std::setw(8) << "Metric" << std::endl;
    std::cout << std::string(53, '-') << std::endl;
    
    for (const auto& route : routes) {
        std::cout << std::setw(20) << route.destination
                  << std::setw(15) << route.next_hop
                  << std::setw(10) << route.protocol
                  << std::setw(8) << route.metric << std::endl;
    }
}

void CLIInterface::cmd_show_neighbors(const std::vector<std::string>& args) {
    std::cout << "Neighbors:" << std::endl;
    std::cout << "==========" << std::endl;
    std::cout << "No neighbors configured" << std::endl;
}

// BGP commands
void CLIInterface::cmd_bgp(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cout << "Usage: bgp <command>" << std::endl;
        std::cout << "Commands: start, stop, neighbor, advertise" << std::endl;
        return;
    }
    
    std::string cmd = args[0];
    std::vector<std::string> cmd_args(args.begin() + 1, args.end());
    
    if (cmd == "start") {
        std::cout << "BGP started" << std::endl;
    } else if (cmd == "stop") {
        std::cout << "BGP stopped" << std::endl;
    } else if (cmd == "neighbor") {
        std::cout << "BGP neighbor configuration" << std::endl;
    } else if (cmd == "advertise") {
        std::cout << "BGP route advertisement" << std::endl;
    } else {
        std::cout << "Unknown BGP command: " << cmd << std::endl;
    }
}

void CLIInterface::cmd_show_bgp(const std::vector<std::string>& args) {
    std::cout << "BGP Status:" << std::endl;
    std::cout << "===========" << std::endl;
    std::cout << "Not implemented" << std::endl;
}

void CLIInterface::cmd_show_bgp_routes(const std::vector<std::string>& args) {
    std::cout << "BGP Routes:" << std::endl;
    std::cout << "===========" << std::endl;
    std::cout << "No BGP routes" << std::endl;
}

void CLIInterface::cmd_show_bgp_neighbors(const std::vector<std::string>& args) {
    std::cout << "BGP Neighbors:" << std::endl;
    std::cout << "==============" << std::endl;
    std::cout << "No BGP neighbors" << std::endl;
}

// OSPF commands
void CLIInterface::cmd_ospf(const std::vector<std::string>& args) {
    std::cout << "OSPF configuration" << std::endl;
}

void CLIInterface::cmd_show_ospf(const std::vector<std::string>& args) {
    std::cout << "OSPF Status:" << std::endl;
    std::cout << "============" << std::endl;
    std::cout << "Not implemented" << std::endl;
}

void CLIInterface::cmd_show_ospf_routes(const std::vector<std::string>& args) {
    std::cout << "OSPF Routes:" << std::endl;
    std::cout << "============" << std::endl;
    std::cout << "No OSPF routes" << std::endl;
}

void CLIInterface::cmd_show_ospf_neighbors(const std::vector<std::string>& args) {
    std::cout << "OSPF Neighbors:" << std::endl;
    std::cout << "===============" << std::endl;
    std::cout << "No OSPF neighbors" << std::endl;
}

// IS-IS commands
void CLIInterface::cmd_isis(const std::vector<std::string>& args) {
    std::cout << "IS-IS configuration" << std::endl;
}

void CLIInterface::cmd_show_isis(const std::vector<std::string>& args) {
    std::cout << "IS-IS Status:" << std::endl;
    std::cout << "=============" << std::endl;
    std::cout << "Not implemented" << std::endl;
}

void CLIInterface::cmd_show_isis_routes(const std::vector<std::string>& args) {
    std::cout << "IS-IS Routes:" << std::endl;
    std::cout << "=============" << std::endl;
    std::cout << "No IS-IS routes" << std::endl;
}

void CLIInterface::cmd_show_isis_neighbors(const std::vector<std::string>& args) {
    std::cout << "IS-IS Neighbors:" << std::endl;
    std::cout << "================" << std::endl;
    std::cout << "No IS-IS neighbors" << std::endl;
}

// Traffic shaping commands
void CLIInterface::cmd_traffic(const std::vector<std::string>& args) {
    std::cout << "Traffic shaping configuration" << std::endl;
}

void CLIInterface::cmd_show_traffic(const std::vector<std::string>& args) {
    std::cout << "Traffic Statistics:" << std::endl;
    std::cout << "===================" << std::endl;
    std::cout << "Not implemented" << std::endl;
}

// Netem commands
void CLIInterface::cmd_netem(const std::vector<std::string>& args) {
    std::cout << "Netem impairments configuration" << std::endl;
}

void CLIInterface::cmd_show_netem(const std::vector<std::string>& args) {
    std::cout << "Netem Impairments:" << std::endl;
    std::cout << "==================" << std::endl;
    std::cout << "No impairments active" << std::endl;
}

// Configuration commands
void CLIInterface::cmd_configure(const std::vector<std::string>& args) {
    std::cout << "Configuration mode" << std::endl;
}

void CLIInterface::cmd_load_config(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cout << "Usage: load config <file>" << std::endl;
        return;
    }
    
    std::cout << "Loading configuration from " << args[0] << std::endl;
}

void CLIInterface::cmd_save_config(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cout << "Usage: save config <file>" << std::endl;
        return;
    }
    
    std::cout << "Saving configuration to " << args[0] << std::endl;
}

// Testing commands
void CLIInterface::cmd_test(const std::vector<std::string>& args) {
    std::cout << "Running tests..." << std::endl;
}

void CLIInterface::cmd_capture(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cout << "Usage: capture <interface>" << std::endl;
        return;
    }
    
    std::cout << "Starting packet capture on " << args[0] << std::endl;
}

void CLIInterface::cmd_compare(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cout << "Usage: compare <file1> <file2>" << std::endl;
        return;
    }
    
    std::cout << "Comparing " << args[0] << " and " << args[1] << std::endl;
}

} // namespace RouterSim