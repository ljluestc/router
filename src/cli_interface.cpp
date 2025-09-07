#include "cli_interface.h"
#include "router_core.h"
#include "frr_integration.h"
#include "traffic_shaping.h"
#include "netem_impairments.h"
#include <iostream>
#include <sstream>
#include <iomanip>

namespace RouterSim {

CLIInterface::CLIInterface() : running_(false) {
    // Register default commands
    register_command("help", [this](const auto& args) { cmd_help(args); }, 
                    "Show available commands");
    register_command("quit", [this](const auto& args) { cmd_quit(args); }, 
                    "Exit the CLI");
    register_command("exit", [this](const auto& args) { cmd_quit(args); }, 
                    "Exit the CLI");
    register_command("show interfaces", [this](const auto& args) { cmd_show_interfaces(args); }, 
                    "Show interface information");
    register_command("show routes", [this](const auto& args) { cmd_show_routes(args); }, 
                    "Show routing table");
    register_command("show statistics", [this](const auto& args) { cmd_show_statistics(args); }, 
                    "Show router statistics");
    register_command("configure interface", [this](const auto& args) { cmd_configure_interface(args); }, 
                    "Configure interface");
    register_command("configure route", [this](const auto& args) { cmd_configure_route(args); }, 
                    "Configure route");
    register_command("start bgp", [this](const auto& args) { cmd_start_bgp(args); }, 
                    "Start BGP protocol");
    register_command("start ospf", [this](const auto& args) { cmd_start_ospf(args); }, 
                    "Start OSPF protocol");
    register_command("start isis", [this](const auto& args) { cmd_start_isis(args); }, 
                    "Start IS-IS protocol");
    register_command("configure traffic-shaping", [this](const auto& args) { cmd_configure_traffic_shaping(args); }, 
                    "Configure traffic shaping");
    register_command("configure netem", [this](const auto& args) { cmd_configure_netem(args); }, 
                    "Configure network impairments");
    register_command("load scenario", [this](const auto& args) { cmd_load_scenario(args); }, 
                    "Load scenario from YAML file");
}

CLIInterface::~CLIInterface() {
    stop();
}

void CLIInterface::set_router_core(std::shared_ptr<RouterCore> router) {
    router_ = router;
}

void CLIInterface::set_frr_integration(std::shared_ptr<FRRIntegration> frr) {
    frr_ = frr;
}

void CLIInterface::set_traffic_shaper(std::shared_ptr<TrafficShaper> shaper) {
    shaper_ = shaper;
}

void CLIInterface::set_netem_impairments(std::shared_ptr<NetemImpairments> netem) {
    netem_ = netem;
}

void CLIInterface::start() {
    if (running_) {
        return;
    }
    
    running_ = true;
    cli_thread_ = std::thread(&CLIInterface::command_loop, this);
    
    std::cout << "CLI interface started" << std::endl;
}

void CLIInterface::stop() {
    if (!running_) {
        return;
    }
    
    running_ = false;
    
    if (cli_thread_.joinable()) {
        cli_thread_.join();
    }
    
    std::cout << "CLI interface stopped" << std::endl;
}

bool CLIInterface::is_running() const {
    return running_;
}

void CLIInterface::register_command(const std::string& name, 
                                   std::function<void(const std::vector<std::string>&)> handler,
                                   const std::string& description) {
    commands_[name] = handler;
    descriptions_[name] = description;
}

void CLIInterface::command_loop() {
    std::string input;
    
    while (running_) {
        print_prompt();
        
        if (!std::getline(std::cin, input)) {
            break;
        }
        
        if (!input.empty()) {
            process_command(input);
        }
    }
}

void CLIInterface::process_command(const std::string& input) {
    auto args = parse_command(input);
    if (args.empty()) {
        return;
    }
    
    std::string command = args[0];
    args.erase(args.begin());
    
    auto it = commands_.find(command);
    if (it != commands_.end()) {
        it->second(args);
    } else {
        std::cout << "Unknown command: " << command << std::endl;
        std::cout << "Type 'help' for available commands" << std::endl;
    }
}

std::vector<std::string> CLIInterface::parse_command(const std::string& input) {
    std::vector<std::string> args;
    std::istringstream iss(input);
    std::string arg;
    
    while (iss >> arg) {
        args.push_back(arg);
    }
    
    return args;
}

void CLIInterface::print_help() {
    std::cout << "\nAvailable commands:" << std::endl;
    std::cout << "==================" << std::endl;
    
    for (const auto& pair : descriptions_) {
        std::cout << std::left << std::setw(25) << pair.first << " - " << pair.second << std::endl;
    }
    
    std::cout << std::endl;
}

void CLIInterface::print_prompt() {
    std::cout << "router> ";
    std::cout.flush();
}

void CLIInterface::cmd_help(const std::vector<std::string>& args) {
    print_help();
}

void CLIInterface::cmd_quit(const std::vector<std::string>& args) {
    std::cout << "Goodbye!" << std::endl;
    running_ = false;
}

void CLIInterface::cmd_show_interfaces(const std::vector<std::string>& args) {
    if (!router_) {
        std::cout << "Router core not available" << std::endl;
        return;
    }
    
    auto interfaces = router_->get_interfaces();
    
    std::cout << "\nInterface Status:" << std::endl;
    std::cout << "=================" << std::endl;
    std::cout << std::left << std::setw(10) << "Interface" 
              << std::setw(15) << "IP Address" 
              << std::setw(15) << "Subnet Mask" 
              << std::setw(8) << "Status" 
              << std::setw(8) << "MTU" << std::endl;
    std::cout << std::string(56, '-') << std::endl;
    
    for (const auto& iface : interfaces) {
        std::cout << std::left << std::setw(10) << iface.name
                  << std::setw(15) << iface.ip_address
                  << std::setw(15) << iface.subnet_mask
                  << std::setw(8) << (iface.is_up ? "UP" : "DOWN")
                  << std::setw(8) << iface.mtu << std::endl;
    }
    
    std::cout << std::endl;
}

void CLIInterface::cmd_show_routes(const std::vector<std::string>& args) {
    if (!router_) {
        std::cout << "Router core not available" << std::endl;
        return;
    }
    
    auto routes = router_->get_routes();
    
    std::cout << "\nRouting Table:" << std::endl;
    std::cout << "==============" << std::endl;
    std::cout << std::left << std::setw(20) << "Network" 
              << std::setw(15) << "Next Hop" 
              << std::setw(10) << "Interface" 
              << std::setw(8) << "Metric" 
              << std::setw(10) << "Protocol" << std::endl;
    std::cout << std::string(73, '-') << std::endl;
    
    for (const auto& route : routes) {
        std::cout << std::left << std::setw(20) << route.network
                  << std::setw(15) << route.next_hop
                  << std::setw(10) << route.interface
                  << std::setw(8) << route.metric
                  << std::setw(10) << route.protocol << std::endl;
    }
    
    std::cout << std::endl;
}

void CLIInterface::cmd_show_statistics(const std::vector<std::string>& args) {
    if (!router_) {
        std::cout << "Router core not available" << std::endl;
        return;
    }
    
    auto stats = router_->get_statistics();
    
    std::cout << "\nRouter Statistics:" << std::endl;
    std::cout << "==================" << std::endl;
    std::cout << "Total packets processed: " << stats.total_packets_processed << std::endl;
    std::cout << "Total bytes processed: " << stats.total_bytes_processed << std::endl;
    std::cout << "Routing table updates: " << stats.routing_table_updates << std::endl;
    std::cout << "Interface state changes: " << stats.interface_state_changes << std::endl;
    std::cout << std::endl;
}

void CLIInterface::cmd_configure_interface(const std::vector<std::string>& args) {
    if (args.size() < 3) {
        std::cout << "Usage: configure interface <name> <ip> <mask> [up|down]" << std::endl;
        return;
    }
    
    if (!router_) {
        std::cout << "Router core not available" << std::endl;
        return;
    }
    
    std::string name = args[0];
    std::string ip = args[1];
    std::string mask = args[2];
    bool up = (args.size() > 3 && args[3] == "up");
    
    if (router_->add_interface(name, ip, mask)) {
        router_->set_interface_up(name, up);
        std::cout << "Interface " << name << " configured successfully" << std::endl;
    } else {
        std::cout << "Failed to configure interface " << name << std::endl;
    }
}

void CLIInterface::cmd_configure_route(const std::vector<std::string>& args) {
    if (args.size() < 4) {
        std::cout << "Usage: configure route <network> <next_hop> <interface> <metric>" << std::endl;
        return;
    }
    
    if (!router_) {
        std::cout << "Router core not available" << std::endl;
        return;
    }
    
    Route route;
    route.network = args[0];
    route.next_hop = args[1];
    route.interface = args[2];
    route.metric = std::stoi(args[3]);
    route.protocol = "static";
    route.is_active = true;
    
    if (router_->add_route(route)) {
        std::cout << "Route configured successfully" << std::endl;
    } else {
        std::cout << "Failed to configure route" << std::endl;
    }
}

void CLIInterface::cmd_start_bgp(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cout << "Usage: start bgp <as_number> <router_id>" << std::endl;
        return;
    }
    
    if (!frr_) {
        std::cout << "FRR integration not available" << std::endl;
        return;
    }
    
    std::string as_number = args[0];
    std::string router_id = args[1];
    
    if (frr_->start_bgp(as_number, router_id)) {
        std::cout << "BGP started successfully" << std::endl;
    } else {
        std::cout << "Failed to start BGP" << std::endl;
    }
}

void CLIInterface::cmd_start_ospf(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cout << "Usage: start ospf <router_id> <area>" << std::endl;
        return;
    }
    
    if (!frr_) {
        std::cout << "FRR integration not available" << std::endl;
        return;
    }
    
    std::string router_id = args[0];
    std::string area = args[1];
    
    if (frr_->start_ospf(router_id, area)) {
        std::cout << "OSPF started successfully" << std::endl;
    } else {
        std::cout << "Failed to start OSPF" << std::endl;
    }
}

void CLIInterface::cmd_start_isis(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cout << "Usage: start isis <system_id> <area_id>" << std::endl;
        return;
    }
    
    if (!frr_) {
        std::cout << "FRR integration not available" << std::endl;
        return;
    }
    
    std::string system_id = args[0];
    std::string area_id = args[1];
    
    if (frr_->start_isis(system_id, area_id)) {
        std::cout << "IS-IS started successfully" << std::endl;
    } else {
        std::cout << "Failed to start IS-IS" << std::endl;
    }
}

void CLIInterface::cmd_configure_traffic_shaping(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cout << "Usage: configure traffic-shaping <capacity> <rate>" << std::endl;
        return;
    }
    
    if (!shaper_) {
        std::cout << "Traffic shaper not available" << std::endl;
        return;
    }
    
    uint64_t capacity = std::stoull(args[0]);
    uint64_t rate = std::stoull(args[1]);
    
    shaper_->set_token_bucket(capacity, rate);
    std::cout << "Traffic shaping configured: capacity=" << capacity << ", rate=" << rate << std::endl;
}

void CLIInterface::cmd_configure_netem(const std::vector<std::string>& args) {
    if (args.size() < 3) {
        std::cout << "Usage: configure netem <interface> <delay_ms> <loss_percent>" << std::endl;
        return;
    }
    
    if (!netem_) {
        std::cout << "Netem impairments not available" << std::endl;
        return;
    }
    
    std::string interface = args[0];
    double delay = std::stod(args[1]);
    double loss = std::stod(args[2]);
    
    NetemConfig config;
    config.interface = interface;
    config.delay_ms = delay;
    config.loss_percent = loss;
    
    if (netem_->apply_impairments(interface, config)) {
        std::cout << "Netem impairments applied to " << interface << std::endl;
    } else {
        std::cout << "Failed to apply netem impairments" << std::endl;
    }
}

void CLIInterface::cmd_load_scenario(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cout << "Usage: load scenario <filename>" << std::endl;
        return;
    }
    
    std::cout << "Loading scenario from " << args[0] << "..." << std::endl;
    // Implementation would load and apply YAML scenario
    std::cout << "Scenario loading not yet implemented" << std::endl;
}

} // namespace RouterSim
