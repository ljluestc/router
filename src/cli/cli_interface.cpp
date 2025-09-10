#include "cli/cli_interface.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <thread>
#include <chrono>

namespace RouterSim {

CLIInterface::CLIInterface() : running_(false), initialized_(false) {
    // Initialize command handlers
    initialize_commands();
}

CLIInterface::~CLIInterface() {
    stop();
}

bool CLIInterface::initialize() {
    if (initialized_) {
        return true;
    }
    
    // Initialize router core
    router_core_ = std::make_unique<RouterCore>();
    if (!router_core_->initialize()) {
        std::cerr << "Failed to initialize router core" << std::endl;
        return false;
    }
    
    // Initialize FRR integration
    frr_integration_ = std::make_unique<FRRIntegration>();
    
    // Initialize traffic shaper
    traffic_shaper_ = std::make_unique<TrafficShapingManager>();
    if (!traffic_shaper_->initialize()) {
        std::cerr << "Failed to initialize traffic shaper" << std::endl;
        return false;
    }
    
    // Initialize netem impairments
    netem_impairments_ = std::make_unique<NetemImpairments>();
    if (!netem_impairments_->initialize()) {
        std::cerr << "Failed to initialize netem impairments" << std::endl;
        return false;
    }
    
    initialized_ = true;
    std::cout << "CLI interface initialized successfully" << std::endl;
    return true;
}

bool CLIInterface::start() {
    if (!initialized_) {
        if (!initialize()) {
            return false;
        }
    }
    
    if (running_) {
        return true;
    }
    
    running_ = true;
    
    // Start components
    router_core_->start();
    traffic_shaper_->start();
    netem_impairments_->start();
    
    // Start CLI thread
    cli_thread_ = std::thread(&CLIInterface::cli_loop, this);
    
    std::cout << "CLI interface started" << std::endl;
    return true;
}

bool CLIInterface::stop() {
    if (!running_) {
        return true;
    }
    
    running_ = false;
    
    // Stop components
    if (router_core_) {
        router_core_->stop();
    }
    
    if (traffic_shaper_) {
        traffic_shaper_->stop();
    }
    
    if (netem_impairments_) {
        netem_impairments_->stop();
    }
    
    // Wait for CLI thread to finish
    if (cli_thread_.joinable()) {
        cli_thread_.join();
    }
    
    std::cout << "CLI interface stopped" << std::endl;
    return true;
}

bool CLIInterface::is_running() const {
    return running_;
}

void CLIInterface::cli_loop() {
    std::string input;
    
    while (running_) {
        std::cout << "router-sim> ";
        std::getline(std::cin, input);
        
        if (input.empty()) {
            continue;
        }
        
        if (input == "exit" || input == "quit") {
            break;
        }
        
        if (input == "help") {
            show_help();
            continue;
        }
        
        // Parse and execute command
        execute_command(input);
    }
}

void CLIInterface::execute_command(const std::string& input) {
    std::istringstream iss(input);
    std::vector<std::string> tokens;
    std::string token;
    
    while (iss >> token) {
        tokens.push_back(token);
    }
    
    if (tokens.empty()) {
        return;
    }
    
    std::string command = tokens[0];
    
    // Find command handler
    auto it = command_handlers_.find(command);
    if (it != command_handlers_.end()) {
        it->second(tokens);
    } else {
        std::cout << "Unknown command: " << command << std::endl;
        std::cout << "Type 'help' for available commands" << std::endl;
    }
}

void CLIInterface::initialize_commands() {
    // Router commands
    command_handlers_["status"] = [this](const std::vector<std::string>& tokens) {
        show_router_status();
    };
    
    command_handlers_["interfaces"] = [this](const std::vector<std::string>& tokens) {
        show_interfaces();
    };
    
    command_handlers_["routes"] = [this](const std::vector<std::string>& tokens) {
        show_routes();
    };
    
    command_handlers_["neighbors"] = [this](const std::vector<std::string>& tokens) {
        show_neighbors();
    };
    
    // Protocol commands
    command_handlers_["protocol"] = [this](const std::vector<std::string>& tokens) {
        handle_protocol_command(tokens);
    };
    
    // Traffic shaping commands
    command_handlers_["traffic"] = [this](const std::vector<std::string>& tokens) {
        handle_traffic_command(tokens);
    };
    
    // Impairment commands
    command_handlers_["impairment"] = [this](const std::vector<std::string>& tokens) {
        handle_impairment_command(tokens);
    };
    
    // Scenario commands
    command_handlers_["scenario"] = [this](const std::vector<std::string>& tokens) {
        handle_scenario_command(tokens);
    };
    
    // Configuration commands
    command_handlers_["config"] = [this](const std::vector<std::string>& tokens) {
        handle_config_command(tokens);
    };
    
    // Statistics commands
    command_handlers_["stats"] = [this](const std::vector<std::string>& tokens) {
        handle_stats_command(tokens);
    };
}

void CLIInterface::show_help() {
    std::cout << "\nRouter Simulator CLI Commands:\n";
    std::cout << "==============================\n\n";
    
    std::cout << "Router Commands:\n";
    std::cout << "  status                    - Show router status\n";
    std::cout << "  interfaces                - Show network interfaces\n";
    std::cout << "  routes                    - Show routing table\n";
    std::cout << "  neighbors                 - Show protocol neighbors\n\n";
    
    std::cout << "Protocol Commands:\n";
    std::cout << "  protocol start <name>     - Start routing protocol\n";
    std::cout << "  protocol stop <name>      - Stop routing protocol\n";
    std::cout << "  protocol status <name>    - Show protocol status\n";
    std::cout << "  protocol add-neighbor <protocol> <address> <config> - Add neighbor\n";
    std::cout << "  protocol add-route <protocol> <prefix> <config> - Add route\n\n";
    
    std::cout << "Traffic Shaping Commands:\n";
    std::cout << "  traffic show              - Show traffic shaping status\n";
    std::cout << "  traffic add-interface <name> - Add interface for shaping\n";
    std::cout << "  traffic configure <interface> <algorithm> <config> - Configure shaping\n";
    std::cout << "  traffic stats <interface> - Show traffic statistics\n\n";
    
    std::cout << "Impairment Commands:\n";
    std::cout << "  impairment add-delay <interface> <delay> <jitter> - Add delay\n";
    std::cout << "  impairment add-loss <interface> <percentage> - Add packet loss\n";
    std::cout << "  impairment add-duplicate <interface> <percentage> - Add duplication\n";
    std::cout << "  impairment clear <interface> - Clear impairments\n";
    std::cout << "  impairment show           - Show current impairments\n\n";
    
    std::cout << "Scenario Commands:\n";
    std::cout << "  scenario list             - List available scenarios\n";
    std::cout << "  scenario run <name>       - Run scenario\n";
    std::cout << "  scenario stop             - Stop current scenario\n";
    std::cout << "  scenario status           - Show scenario status\n\n";
    
    std::cout << "Configuration Commands:\n";
    std::cout << "  config load <file>        - Load configuration from file\n";
    std::cout << "  config save <file>        - Save configuration to file\n";
    std::cout << "  config show               - Show current configuration\n\n";
    
    std::cout << "Statistics Commands:\n";
    std::cout << "  stats show                - Show all statistics\n";
    std::cout << "  stats reset               - Reset all statistics\n";
    std::cout << "  stats export <file>       - Export statistics to file\n\n";
    
    std::cout << "General Commands:\n";
    std::cout << "  help                      - Show this help\n";
    std::cout << "  exit/quit                 - Exit the program\n\n";
}

void CLIInterface::show_router_status() {
    std::cout << "\nRouter Status:\n";
    std::cout << "==============\n";
    
    if (router_core_) {
        auto status = router_core_->get_status();
        std::cout << "Status: " << (status.is_running ? "Running" : "Stopped") << "\n";
        std::cout << "Uptime: " << status.uptime_seconds << " seconds\n";
        std::cout << "Interfaces: " << status.interface_count << "\n";
        std::cout << "Routes: " << status.route_count << "\n";
        std::cout << "Neighbors: " << status.neighbor_count << "\n";
    }
    
    std::cout << "\n";
}

void CLIInterface::show_interfaces() {
    std::cout << "\nNetwork Interfaces:\n";
    std::cout << "==================\n";
    
    if (router_core_) {
        auto interfaces = router_core_->get_interfaces();
        
        std::cout << std::setw(10) << "Interface" 
                  << std::setw(15) << "IP Address"
                  << std::setw(10) << "Status"
                  << std::setw(10) << "MTU"
                  << std::setw(15) << "RX Packets"
                  << std::setw(15) << "TX Packets" << "\n";
        std::cout << std::string(80, '-') << "\n";
        
        for (const auto& interface : interfaces) {
            std::cout << std::setw(10) << interface.name
                      << std::setw(15) << interface.ip_address
                      << std::setw(10) << (interface.is_up ? "UP" : "DOWN")
                      << std::setw(10) << interface.mtu
                      << std::setw(15) << interface.rx_packets
                      << std::setw(15) << interface.tx_packets << "\n";
        }
    }
    
    std::cout << "\n";
}

void CLIInterface::show_routes() {
    std::cout << "\nRouting Table:\n";
    std::cout << "==============\n";
    
    if (frr_integration_) {
        auto routes = frr_integration_->get_routes();
        
        std::cout << std::setw(20) << "Destination"
                  << std::setw(15) << "Next Hop"
                  << std::setw(10) << "Protocol"
                  << std::setw(10) << "Metric"
                  << std::setw(15) << "Interface" << "\n";
        std::cout << std::string(80, '-') << "\n";
        
        for (const auto& route : routes) {
            std::cout << std::setw(20) << route.prefix
                      << std::setw(15) << route.next_hop
                      << std::setw(10) << route.protocol
                      << std::setw(10) << route.metric
                      << std::setw(15) << "N/A" << "\n";
        }
    }
    
    std::cout << "\n";
}

void CLIInterface::show_neighbors() {
    std::cout << "\nProtocol Neighbors:\n";
    std::cout << "==================\n";
    
    if (frr_integration_) {
        auto neighbors = frr_integration_->get_neighbors();
        
        std::cout << std::setw(15) << "Address"
                  << std::setw(10) << "Protocol"
                  << std::setw(15) << "State"
                  << std::setw(10) << "AS Number"
                  << std::setw(15) << "Uptime" << "\n";
        std::cout << std::string(80, '-') << "\n";
        
        for (const auto& neighbor : neighbors) {
            std::cout << std::setw(15) << neighbor.address
                      << std::setw(10) << neighbor.protocol
                      << std::setw(15) << neighbor.state
                      << std::setw(10) << neighbor.as_number
                      << std::setw(15) << "N/A" << "\n";
        }
    }
    
    std::cout << "\n";
}

void CLIInterface::handle_protocol_command(const std::vector<std::string>& tokens) {
    if (tokens.size() < 2) {
        std::cout << "Usage: protocol <start|stop|status> <protocol_name>" << std::endl;
        return;
    }
    
    std::string action = tokens[1];
    
    if (action == "start") {
        if (tokens.size() < 3) {
            std::cout << "Usage: protocol start <protocol_name>" << std::endl;
            return;
        }
        
        std::string protocol = tokens[2];
        std::map<std::string, std::string> config;
        
        if (protocol == "bgp") {
            config["as_number"] = "65001";
            config["router_id"] = "1.1.1.1";
        } else if (protocol == "ospf") {
            config["router_id"] = "1.1.1.1";
            config["area_id"] = "0.0.0.0";
        } else if (protocol == "isis") {
            config["system_id"] = "0000.0000.0001";
            config["area_id"] = "49.0001";
        }
        
        if (frr_integration_->start_protocol(protocol, config)) {
            std::cout << "Protocol " << protocol << " started successfully" << std::endl;
        } else {
            std::cout << "Failed to start protocol " << protocol << std::endl;
        }
        
    } else if (action == "stop") {
        if (tokens.size() < 3) {
            std::cout << "Usage: protocol stop <protocol_name>" << std::endl;
            return;
        }
        
        std::string protocol = tokens[2];
        
        if (frr_integration_->stop_protocol(protocol)) {
            std::cout << "Protocol " << protocol << " stopped successfully" << std::endl;
        } else {
            std::cout << "Failed to stop protocol " << protocol << std::endl;
        }
        
    } else if (action == "status") {
        if (tokens.size() < 3) {
            std::cout << "Usage: protocol status <protocol_name>" << std::endl;
            return;
        }
        
        std::string protocol = tokens[2];
        
        if (frr_integration_->is_protocol_running(protocol)) {
            std::cout << "Protocol " << protocol << " is running" << std::endl;
        } else {
            std::cout << "Protocol " << protocol << " is not running" << std::endl;
        }
        
    } else {
        std::cout << "Unknown protocol action: " << action << std::endl;
    }
}

void CLIInterface::handle_traffic_command(const std::vector<std::string>& tokens) {
    if (tokens.size() < 2) {
        std::cout << "Usage: traffic <show|add-interface|configure|stats>" << std::endl;
        return;
    }
    
    std::string action = tokens[1];
    
    if (action == "show") {
        std::cout << "Traffic Shaping Status:\n";
        std::cout << "======================\n";
        
        if (traffic_shaper_) {
            auto stats = traffic_shaper_->get_global_statistics();
            std::cout << "Packets Processed: " << stats.packets_processed << "\n";
            std::cout << "Packets Dropped: " << stats.packets_dropped << "\n";
            std::cout << "Bytes Processed: " << stats.bytes_processed << "\n";
            std::cout << "Current Throughput: " << stats.current_throughput_bps << " bps\n";
        }
        
    } else if (action == "add-interface") {
        if (tokens.size() < 3) {
            std::cout << "Usage: traffic add-interface <interface_name>" << std::endl;
            return;
        }
        
        std::string interface = tokens[2];
        
        if (traffic_shaper_->add_interface(interface)) {
            std::cout << "Interface " << interface << " added for traffic shaping" << std::endl;
        } else {
            std::cout << "Failed to add interface " << interface << std::endl;
        }
        
    } else if (action == "configure") {
        if (tokens.size() < 5) {
            std::cout << "Usage: traffic configure <interface> <algorithm> <config>" << std::endl;
            return;
        }
        
        std::string interface = tokens[2];
        std::string algorithm = tokens[3];
        std::string config = tokens[4];
        
        // Parse algorithm
        ShapingAlgorithm alg;
        if (algorithm == "token-bucket") {
            alg = ShapingAlgorithm::TOKEN_BUCKET;
        } else if (algorithm == "wfq") {
            alg = ShapingAlgorithm::WEIGHTED_FAIR_QUEUE;
        } else {
            std::cout << "Unknown algorithm: " << algorithm << std::endl;
            return;
        }
        
        // Parse config (simplified)
        std::map<std::string, std::string> config_map;
        // This would parse the config string into a map
        
        if (traffic_shaper_->configure_interface(interface, alg, config_map)) {
            std::cout << "Interface " << interface << " configured successfully" << std::endl;
        } else {
            std::cout << "Failed to configure interface " << interface << std::endl;
        }
        
    } else if (action == "stats") {
        if (tokens.size() < 3) {
            std::cout << "Usage: traffic stats <interface>" << std::endl;
            return;
        }
        
        std::string interface = tokens[2];
        
        if (traffic_shaper_) {
            auto stats = traffic_shaper_->get_interface_statistics();
            auto it = stats.find(interface);
            if (it != stats.end()) {
                std::cout << "Traffic Statistics for " << interface << ":\n";
                std::cout << "Packets Processed: " << it->second.packets_processed << "\n";
                std::cout << "Packets Dropped: " << it->second.packets_dropped << "\n";
                std::cout << "Bytes Processed: " << it->second.bytes_processed << "\n";
                std::cout << "Current Throughput: " << it->second.current_throughput_bps << " bps\n";
            } else {
                std::cout << "Interface " << interface << " not found" << std::endl;
            }
        }
        
    } else {
        std::cout << "Unknown traffic action: " << action << std::endl;
    }
}

void CLIInterface::handle_impairment_command(const std::vector<std::string>& tokens) {
    if (tokens.size() < 2) {
        std::cout << "Usage: impairment <add-delay|add-loss|add-duplicate|clear|show>" << std::endl;
        return;
    }
    
    std::string action = tokens[1];
    
    if (action == "add-delay") {
        if (tokens.size() < 5) {
            std::cout << "Usage: impairment add-delay <interface> <delay_ms> <jitter_ms>" << std::endl;
            return;
        }
        
        std::string interface = tokens[2];
        uint32_t delay = std::stoul(tokens[3]);
        uint32_t jitter = std::stoul(tokens[4]);
        
        DelayConfig config;
        config.delay_ms = delay;
        config.jitter_ms = jitter;
        config.distribution = "uniform";
        
        if (netem_impairments_->add_delay(interface, config)) {
            std::cout << "Delay impairment added to " << interface << std::endl;
        } else {
            std::cout << "Failed to add delay impairment to " << interface << std::endl;
        }
        
    } else if (action == "add-loss") {
        if (tokens.size() < 4) {
            std::cout << "Usage: impairment add-loss <interface> <percentage>" << std::endl;
            return;
        }
        
        std::string interface = tokens[2];
        double percentage = std::stod(tokens[3]);
        
        LossConfig config;
        config.loss_type = "random";
        config.loss_percentage = percentage;
        
        if (netem_impairments_->add_loss(interface, config)) {
            std::cout << "Loss impairment added to " << interface << std::endl;
        } else {
            std::cout << "Failed to add loss impairment to " << interface << std::endl;
        }
        
    } else if (action == "add-duplicate") {
        if (tokens.size() < 4) {
            std::cout << "Usage: impairment add-duplicate <interface> <percentage>" << std::endl;
            return;
        }
        
        std::string interface = tokens[2];
        double percentage = std::stod(tokens[3]);
        
        DuplicateConfig config;
        config.duplicate_percentage = percentage;
        
        if (netem_impairments_->add_duplicate(interface, config)) {
            std::cout << "Duplicate impairment added to " << interface << std::endl;
        } else {
            std::cout << "Failed to add duplicate impairment to " << interface << std::endl;
        }
        
    } else if (action == "clear") {
        if (tokens.size() < 3) {
            std::cout << "Usage: impairment clear <interface>" << std::endl;
            return;
        }
        
        std::string interface = tokens[2];
        
        if (netem_impairments_->clear_interface_impairments(interface)) {
            std::cout << "Impairments cleared for " << interface << std::endl;
        } else {
            std::cout << "Failed to clear impairments for " << interface << std::endl;
        }
        
    } else if (action == "show") {
        std::cout << "Current Impairments:\n";
        std::cout << "===================\n";
        
        if (netem_impairments_) {
            auto interfaces = netem_impairments_->get_interfaces();
            for (const auto& interface : interfaces) {
                auto impairments = netem_impairments_->get_interface_impairments(interface);
                std::cout << "Interface " << interface << ":\n";
                
                if (impairments.delay.has_value()) {
                    std::cout << "  Delay: " << impairments.delay->delay_ms << "ms (+" 
                              << impairments.delay->jitter_ms << "ms jitter)\n";
                }
                
                if (impairments.loss.has_value()) {
                    std::cout << "  Loss: " << impairments.loss->loss_percentage << "%\n";
                }
                
                if (impairments.duplicate.has_value()) {
                    std::cout << "  Duplicate: " << impairments.duplicate->duplicate_percentage << "%\n";
                }
            }
        }
        
    } else {
        std::cout << "Unknown impairment action: " << action << std::endl;
    }
}

void CLIInterface::handle_scenario_command(const std::vector<std::string>& tokens) {
    if (tokens.size() < 2) {
        std::cout << "Usage: scenario <list|run|stop|status>" << std::endl;
        return;
    }
    
    std::string action = tokens[1];
    
    if (action == "list") {
        std::cout << "Available Scenarios:\n";
        std::cout << "===================\n";
        std::cout << "  high_latency      - Simulate high latency network\n";
        std::cout << "  packet_loss       - Simulate packet loss\n";
        std::cout << "  unreliable_network - Simulate unreliable network\n";
        std::cout << "  bandwidth_limited - Simulate bandwidth-limited network\n";
        
    } else if (action == "run") {
        if (tokens.size() < 3) {
            std::cout << "Usage: scenario run <scenario_name>" << std::endl;
            return;
        }
        
        std::string scenario = tokens[2];
        
        if (netem_impairments_->simulate_network_conditions(scenario)) {
            std::cout << "Scenario " << scenario << " started successfully" << std::endl;
        } else {
            std::cout << "Failed to start scenario " << scenario << std::endl;
        }
        
    } else if (action == "stop") {
        if (netem_impairments_->clear_all_impairments()) {
            std::cout << "Scenario stopped successfully" << std::endl;
        } else {
            std::cout << "Failed to stop scenario" << std::endl;
        }
        
    } else if (action == "status") {
        std::cout << "Scenario Status:\n";
        std::cout << "================\n";
        
        if (netem_impairments_) {
            auto interfaces = netem_impairments_->get_interfaces();
            if (interfaces.empty()) {
                std::cout << "No active scenarios\n";
            } else {
                std::cout << "Active scenarios on " << interfaces.size() << " interfaces\n";
            }
        }
        
    } else {
        std::cout << "Unknown scenario action: " << action << std::endl;
    }
}

void CLIInterface::handle_config_command(const std::vector<std::string>& tokens) {
    if (tokens.size() < 2) {
        std::cout << "Usage: config <load|save|show>" << std::endl;
        return;
    }
    
    std::string action = tokens[1];
    
    if (action == "load") {
        if (tokens.size() < 3) {
            std::cout << "Usage: config load <file>" << std::endl;
            return;
        }
        
        std::string file = tokens[2];
        std::cout << "Loading configuration from " << file << "..." << std::endl;
        // Implementation would load configuration from file
        
    } else if (action == "save") {
        if (tokens.size() < 3) {
            std::cout << "Usage: config save <file>" << std::endl;
            return;
        }
        
        std::string file = tokens[2];
        std::cout << "Saving configuration to " << file << "..." << std::endl;
        // Implementation would save configuration to file
        
    } else if (action == "show") {
        std::cout << "Current Configuration:\n";
        std::cout << "=====================\n";
        // Implementation would show current configuration
        
    } else {
        std::cout << "Unknown config action: " << action << std::endl;
    }
}

void CLIInterface::handle_stats_command(const std::vector<std::string>& tokens) {
    if (tokens.size() < 2) {
        std::cout << "Usage: stats <show|reset|export>" << std::endl;
        return;
    }
    
    std::string action = tokens[1];
    
    if (action == "show") {
        std::cout << "Statistics:\n";
        std::cout << "===========\n";
        
        if (router_core_) {
            auto status = router_core_->get_status();
            std::cout << "Router Uptime: " << status.uptime_seconds << " seconds\n";
            std::cout << "Interfaces: " << status.interface_count << "\n";
            std::cout << "Routes: " << status.route_count << "\n";
            std::cout << "Neighbors: " << status.neighbor_count << "\n";
        }
        
        if (traffic_shaper_) {
            auto stats = traffic_shaper_->get_global_statistics();
            std::cout << "Traffic Packets Processed: " << stats.packets_processed << "\n";
            std::cout << "Traffic Packets Dropped: " << stats.packets_dropped << "\n";
            std::cout << "Traffic Bytes Processed: " << stats.bytes_processed << "\n";
        }
        
    } else if (action == "reset") {
        std::cout << "Resetting statistics..." << std::endl;
        // Implementation would reset all statistics
        
    } else if (action == "export") {
        if (tokens.size() < 3) {
            std::cout << "Usage: stats export <file>" << std::endl;
            return;
        }
        
        std::string file = tokens[2];
        std::cout << "Exporting statistics to " << file << "..." << std::endl;
        // Implementation would export statistics to file
        
    } else {
        std::cout << "Unknown stats action: " << action << std::endl;
    }
}

} // namespace RouterSim