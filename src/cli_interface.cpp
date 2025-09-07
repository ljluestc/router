#include "cli_interface.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <iomanip>

using namespace RouterSim;

CLIInterface::CLIInterface(RouterSimulator* router) 
    : router_(router), prompt_("router> "), auto_completion_enabled_(true), running_(false) {
    register_builtin_commands();
}

CLIInterface::~CLIInterface() {
    stop();
}

void CLIInterface::start() {
    if (running_) {
        return;
    }
    
    running_ = true;
    cli_thread_ = std::thread(&CLIInterface::cli_loop, this);
}

void CLIInterface::stop() {
    if (!running_) {
        return;
    }
    
    running_ = false;
    
    if (cli_thread_.joinable()) {
        cli_thread_.join();
    }
}

bool CLIInterface::is_running() const {
    return running_;
}

void CLIInterface::register_command(const CLICommand& command) {
    commands_[command.name] = command;
    
    // Register aliases
    for (const auto& alias : command.aliases) {
        commands_[alias] = command;
    }
}

void CLIInterface::unregister_command(const std::string& name) {
    commands_.erase(name);
}

bool CLIInterface::execute_command(const std::string& input) {
    auto tokens = parse_command(input);
    if (tokens.empty()) {
        return true;
    }
    
    std::string command_name = tokens[0];
    std::vector<std::string> args(tokens.begin() + 1, tokens.end());
    
    auto it = commands_.find(command_name);
    if (it == commands_.end()) {
        std::cout << "Unknown command: " << command_name << "\n";
        std::cout << "Type 'help' for available commands\n";
        return false;
    }
    
    return it->second.handler(args);
}

std::vector<std::string> CLIInterface::get_command_suggestions(const std::string& partial) const {
    std::vector<std::string> suggestions;
    
    for (const auto& pair : commands_) {
        if (pair.first.find(partial) == 0) {
            suggestions.push_back(pair.first);
        }
    }
    
    return suggestions;
}

void CLIInterface::show_help(const std::string& command) {
    if (command.empty()) {
        show_commands();
        return;
    }
    
    auto it = commands_.find(command);
    if (it == commands_.end()) {
        std::cout << "Command not found: " << command << "\n";
        return;
    }
    
    std::cout << "Command: " << it->second.name << "\n";
    std::cout << "Description: " << it->second.description << "\n";
    std::cout << "Syntax: " << it->second.syntax << "\n";
    
    if (!it->second.aliases.empty()) {
        std::cout << "Aliases: ";
        for (size_t i = 0; i < it->second.aliases.size(); i++) {
            if (i > 0) std::cout << ", ";
            std::cout << it->second.aliases[i];
        }
        std::cout << "\n";
    }
}

void CLIInterface::show_commands() const {
    std::cout << "Available commands:\n";
    std::cout << std::setw(20) << std::left << "Command" 
              << std::setw(50) << "Description" << "\n";
    std::cout << std::string(70, '-') << "\n";
    
    for (const auto& pair : commands_) {
        // Only show primary commands, not aliases
        if (pair.first == pair.second.name) {
            std::cout << std::setw(20) << std::left << pair.first
                      << std::setw(50) << pair.second.description << "\n";
        }
    }
}

void CLIInterface::set_prompt(const std::string& prompt) {
    prompt_ = prompt;
}

void CLIInterface::enable_auto_completion(bool enable) {
    auto_completion_enabled_ = enable;
}

std::vector<std::string> CLIInterface::parse_command(const std::string& input) const {
    std::vector<std::string> tokens;
    std::istringstream stream(input);
    std::string token;
    
    while (stream >> token) {
        tokens.push_back(token);
    }
    
    return tokens;
}

bool CLIInterface::validate_command(const std::vector<std::string>& tokens) const {
    if (tokens.empty()) {
        return false;
    }
    
    return commands_.find(tokens[0]) != commands_.end();
}

void CLIInterface::register_builtin_commands() {
    // Help command
    register_command({
        "help", "Show help information", "help [command]",
        [this](const std::vector<std::string>& args) { return cmd_help(args); },
        {"h", "?"}
    });
    
    // Quit command
    register_command({
        "quit", "Exit the CLI", "quit",
        [this](const std::vector<std::string>& args) { return cmd_quit(args); },
        {"exit", "q"}
    });
    
    // Show command
    register_command({
        "show", "Show system information", "show <type>",
        [this](const std::vector<std::string>& args) { return cmd_show(args); },
        {"sh", "display"}
    });
    
    // Configure command
    register_command({
        "configure", "Configure system settings", "configure <type> <options>",
        [this](const std::vector<std::string>& args) { return cmd_configure(args); },
        {"config", "conf"}
    });
    
    // Interface command
    register_command({
        "interface", "Interface management", "interface <name> <action>",
        [this](const std::vector<std::string>& args) { return cmd_interface(args); },
        {"int", "if"}
    });
    
    // Protocol command
    register_command({
        "protocol", "Protocol management", "protocol <name> <action>",
        [this](const std::vector<std::string>& args) { return cmd_protocol(args); },
        {"proto"}
    });
    
    // Traffic command
    register_command({
        "traffic", "Traffic shaping management", "traffic <action> <options>",
        [this](const std::vector<std::string>& args) { return cmd_traffic(args); },
        {"shaping"}
    });
    
    // Impairment command
    register_command({
        "impairment", "Network impairment management", "impairment <action> <options>",
        [this](const std::vector<std::string>& args) { return cmd_impairment(args); },
        {"netem"}
    });
    
    // Statistics command
    register_command({
        "statistics", "Show statistics", "statistics [type]",
        [this](const std::vector<std::string>& args) { return cmd_statistics(args); },
        {"stats", "stat"}
    });
    
    // Scenario command
    register_command({
        "scenario", "Scenario management", "scenario <action> <options>",
        [this](const std::vector<std::string>& args) { return cmd_scenario(args); },
        {"test"}
    });
}

bool CLIInterface::cmd_help(const std::vector<std::string>& args) {
    if (args.empty()) {
        show_commands();
    } else {
        show_help(args[0]);
    }
    return true;
}

bool CLIInterface::cmd_quit(const std::vector<std::string>& args) {
    std::cout << "Exiting CLI...\n";
    running_ = false;
    return true;
}

bool CLIInterface::cmd_show(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cout << "Usage: show <type>\n";
        std::cout << "Types: interfaces, protocols, routes, neighbors, statistics, scenarios\n";
        return false;
    }
    
    std::string type = args[0];
    std::transform(type.begin(), type.end(), type.begin(), ::tolower);
    
    if (type == "interfaces" || type == "interface") {
        return show_interfaces(args);
    } else if (type == "protocols" || type == "protocol") {
        return show_protocols(args);
    } else if (type == "routes" || type == "route") {
        return show_routes(args);
    } else if (type == "neighbors" || type == "neighbor") {
        return show_neighbors(args);
    } else if (type == "statistics" || type == "stats") {
        return show_statistics(args);
    } else if (type == "scenarios" || type == "scenario") {
        return show_scenarios(args);
    } else {
        std::cout << "Unknown show type: " << type << "\n";
        return false;
    }
}

bool CLIInterface::cmd_configure(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cout << "Usage: configure <type> <options>\n";
        std::cout << "Types: interface, protocol, traffic, impairment\n";
        return false;
    }
    
    std::string type = args[0];
    std::transform(type.begin(), type.end(), type.begin(), ::tolower);
    
    if (type == "interface") {
        return configure_interface(args);
    } else if (type == "protocol") {
        return configure_protocol(args);
    } else if (type == "traffic") {
        return configure_traffic_shaping(args);
    } else if (type == "impairment") {
        return configure_impairments(args);
    } else {
        std::cout << "Unknown configure type: " << type << "\n";
        return false;
    }
}

bool CLIInterface::cmd_interface(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cout << "Usage: interface <name> <action>\n";
        std::cout << "Actions: up, down, show, configure\n";
        return false;
    }
    
    std::string name = args[0];
    std::string action = args[1];
    std::transform(action.begin(), action.end(), action.begin(), ::tolower);
    
    if (action == "up") {
        // Bring interface up
        std::cout << "Interface " << name << " brought up\n";
        return true;
    } else if (action == "down") {
        // Bring interface down
        std::cout << "Interface " << name << " brought down\n";
        return true;
    } else if (action == "show") {
        // Show interface details
        return show_interfaces({name});
    } else if (action == "configure") {
        // Configure interface
        return configure_interface(args);
    } else {
        std::cout << "Unknown interface action: " << action << "\n";
        return false;
    }
}

bool CLIInterface::cmd_protocol(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cout << "Usage: protocol <name> <action>\n";
        std::cout << "Actions: start, stop, restart, show\n";
        return false;
    }
    
    std::string name = args[0];
    std::string action = args[1];
    std::transform(action.begin(), action.end(), action.begin(), ::tolower);
    
    if (action == "start") {
        if (name == "bgp") {
            std::cout << "Starting BGP...\n";
            return true;
        } else if (name == "ospf") {
            std::cout << "Starting OSPF...\n";
            return true;
        } else if (name == "isis") {
            std::cout << "Starting IS-IS...\n";
            return true;
        } else {
            std::cout << "Unknown protocol: " << name << "\n";
            return false;
        }
    } else if (action == "stop") {
        if (name == "bgp") {
            std::cout << "Stopping BGP...\n";
            return true;
        } else if (name == "ospf") {
            std::cout << "Stopping OSPF...\n";
            return true;
        } else if (name == "isis") {
            std::cout << "Stopping IS-IS...\n";
            return true;
        } else {
            std::cout << "Unknown protocol: " << name << "\n";
            return false;
        }
    } else if (action == "restart") {
        // Stop then start
        cmd_protocol({name, "stop"});
        return cmd_protocol({name, "start"});
    } else if (action == "show") {
        return show_protocols({name});
    } else {
        std::cout << "Unknown protocol action: " << action << "\n";
        return false;
    }
}

bool CLIInterface::cmd_traffic(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cout << "Usage: traffic <action> <options>\n";
        std::cout << "Actions: show, configure, reset\n";
        return false;
    }
    
    std::string action = args[0];
    std::transform(action.begin(), action.end(), action.begin(), ::tolower);
    
    if (action == "show") {
        std::cout << "Traffic shaping status:\n";
        // Show traffic shaping status
        return true;
    } else if (action == "configure") {
        return configure_traffic_shaping(args);
    } else if (action == "reset") {
        std::cout << "Traffic shaping statistics reset\n";
        return true;
    } else {
        std::cout << "Unknown traffic action: " << action << "\n";
        return false;
    }
}

bool CLIInterface::cmd_impairment(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cout << "Usage: impairment <action> <options>\n";
        std::cout << "Actions: show, configure, clear\n";
        return false;
    }
    
    std::string action = args[0];
    std::transform(action.begin(), action.end(), action.begin(), ::tolower);
    
    if (action == "show") {
        std::cout << "Network impairments status:\n";
        // Show impairment status
        return true;
    } else if (action == "configure") {
        return configure_impairments(args);
    } else if (action == "clear") {
        std::cout << "Network impairments cleared\n";
        return true;
    } else {
        std::cout << "Unknown impairment action: " << action << "\n";
        return false;
    }
}

bool CLIInterface::cmd_statistics(const std::vector<std::string>& args) {
    return show_statistics(args);
}

bool CLIInterface::cmd_scenario(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cout << "Usage: scenario <action> <options>\n";
        std::cout << "Actions: list, run, load\n";
        return false;
    }
    
    std::string action = args[0];
    std::transform(action.begin(), action.end(), action.begin(), ::tolower);
    
    if (action == "list") {
        return show_scenarios(args);
    } else if (action == "run") {
        if (args.size() < 2) {
            std::cout << "Usage: scenario run <name>\n";
            return false;
        }
        std::cout << "Running scenario: " << args[1] << "\n";
        return router_->run_scenario(args[1]);
    } else if (action == "load") {
        if (args.size() < 2) {
            std::cout << "Usage: scenario load <file>\n";
            return false;
        }
        std::cout << "Loading scenario from: " << args[1] << "\n";
        return router_->load_scenario(args[1]);
    } else {
        std::cout << "Unknown scenario action: " << action << "\n";
        return false;
    }
}

bool CLIInterface::show_interfaces(const std::vector<std::string>& args) {
    auto interfaces = router_->get_interfaces();
    
    std::cout << "Interfaces:\n";
    std::cout << std::setw(15) << std::left << "Name"
              << std::setw(20) << "IP Address"
              << std::setw(15) << "Status"
              << std::setw(10) << "Bandwidth" << "\n";
    std::cout << std::string(60, '-') << "\n";
    
    for (const auto& iface : interfaces) {
        std::cout << std::setw(15) << std::left << iface.name
                  << std::setw(20) << iface.ip_address
                  << std::setw(15) << (iface.is_up ? "UP" : "DOWN")
                  << std::setw(10) << (std::to_string(iface.bandwidth_mbps) + " Mbps") << "\n";
    }
    
    return true;
}

bool CLIInterface::show_protocols(const std::vector<std::string>& args) {
    std::cout << "Protocols:\n";
    std::cout << std::setw(10) << std::left << "Protocol"
              << std::setw(15) << "Status"
              << std::setw(20) << "Neighbors" << "\n";
    std::cout << std::string(45, '-') << "\n";
    
    std::cout << std::setw(10) << std::left << "BGP"
              << std::setw(15) << (router_->is_protocol_running("bgp") ? "Running" : "Stopped")
              << std::setw(20) << "0" << "\n";
    
    std::cout << std::setw(10) << std::left << "OSPF"
              << std::setw(15) << (router_->is_protocol_running("ospf") ? "Running" : "Stopped")
              << std::setw(20) << "0" << "\n";
    
    std::cout << std::setw(10) << std::left << "IS-IS"
              << std::setw(15) << (router_->is_protocol_running("isis") ? "Running" : "Stopped")
              << std::setw(20) << "0" << "\n";
    
    return true;
}

bool CLIInterface::show_routes(const std::vector<std::string>& args) {
    std::cout << "Routes:\n";
    std::cout << "No routes available\n";
    return true;
}

bool CLIInterface::show_neighbors(const std::vector<std::string>& args) {
    std::cout << "Neighbors:\n";
    std::cout << "No neighbors available\n";
    return true;
}

bool CLIInterface::show_statistics(const std::vector<std::string>& args) {
    std::cout << "Statistics:\n";
    std::cout << "No statistics available\n";
    return true;
}

bool CLIInterface::show_scenarios(const std::vector<std::string>& args) {
    auto scenarios = router_->list_scenarios();
    
    std::cout << "Available scenarios:\n";
    for (const auto& scenario : scenarios) {
        std::cout << "  " << scenario << "\n";
    }
    
    return true;
}

bool CLIInterface::configure_interface(const std::vector<std::string>& args) {
    if (args.size() < 3) {
        std::cout << "Usage: configure interface <name> <ip> <mask>\n";
        return false;
    }
    
    std::string name = args[0];
    std::string ip = args[1];
    std::string mask = args[2];
    
    InterfaceConfig config;
    config.name = name;
    config.ip_address = ip;
    config.subnet_mask = mask;
    config.bandwidth_mbps = 1000;
    config.is_up = true;
    
    if (router_->add_interface(config)) {
        std::cout << "Interface " << name << " configured successfully\n";
        return true;
    } else {
        std::cout << "Failed to configure interface " << name << "\n";
        return false;
    }
}

bool CLIInterface::configure_protocol(const std::vector<std::string>& args) {
    std::cout << "Protocol configuration not implemented yet\n";
    return true;
}

bool CLIInterface::configure_traffic_shaping(const std::vector<std::string>& args) {
    std::cout << "Traffic shaping configuration not implemented yet\n";
    return true;
}

bool CLIInterface::configure_impairments(const std::vector<std::string>& args) {
    std::cout << "Impairment configuration not implemented yet\n";
    return true;
}

void CLIInterface::cli_loop() {
    std::string input;
    
    while (running_) {
        std::cout << prompt_;
        std::getline(std::cin, input);
        
        if (input.empty()) {
            continue;
        }
        
        process_input(input);
    }
}

std::string CLIInterface::read_input() const {
    std::string input;
    std::getline(std::cin, input);
    return input;
}

void CLIInterface::process_input(const std::string& input) {
    execute_command(input);
}

// CLIUtils implementation
std::string CLIUtils::format_table(const std::vector<std::vector<std::string>>& data, 
                                  const std::vector<std::string>& headers) {
    std::stringstream result;
    
    // Calculate column widths
    std::vector<size_t> widths(headers.size(), 0);
    for (size_t i = 0; i < headers.size(); i++) {
        widths[i] = headers[i].length();
    }
    
    for (const auto& row : data) {
        for (size_t i = 0; i < row.size() && i < widths.size(); i++) {
            widths[i] = std::max(widths[i], row[i].length());
        }
    }
    
    // Print header
    for (size_t i = 0; i < headers.size(); i++) {
        result << std::setw(widths[i]) << std::left << headers[i];
        if (i < headers.size() - 1) result << "  ";
    }
    result << "\n";
    
    // Print separator
    for (size_t i = 0; i < headers.size(); i++) {
        result << std::string(widths[i], '-');
        if (i < headers.size() - 1) result << "  ";
    }
    result << "\n";
    
    // Print data
    for (const auto& row : data) {
        for (size_t i = 0; i < row.size() && i < widths.size(); i++) {
            result << std::setw(widths[i]) << std::left << row[i];
            if (i < headers.size() - 1) result << "  ";
        }
        result << "\n";
    }
    
    return result.str();
}

std::string CLIUtils::format_json(const std::map<std::string, std::string>& data) {
    std::stringstream result;
    result << "{\n";
    
    bool first = true;
    for (const auto& pair : data) {
        if (!first) result << ",\n";
        result << "  \"" << pair.first << "\": \"" << pair.second << "\"";
        first = false;
    }
    
    result << "\n}\n";
    return result.str();
}

std::string CLIUtils::format_yaml(const std::map<std::string, std::string>& data) {
    std::stringstream result;
    
    for (const auto& pair : data) {
        result << pair.first << ": " << pair.second << "\n";
    }
    
    return result.str();
}

bool CLIUtils::is_valid_ip_address(const std::string& ip) {
    // Simple IP validation
    std::regex ip_regex(R"(^(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3})$)");
    std::smatch matches;
    
    if (!std::regex_match(ip, matches, ip_regex)) {
        return false;
    }
    
    for (int i = 1; i <= 4; i++) {
        int octet = std::stoi(matches[i].str());
        if (octet < 0 || octet > 255) {
            return false;
        }
    }
    
    return true;
}

bool CLIUtils::is_valid_subnet_mask(const std::string& mask) {
    return is_valid_ip_address(mask); // Same validation for now
}

bool CLIUtils::is_valid_interface_name(const std::string& name) {
    return !name.empty() && name.length() <= 16;
}

bool CLIUtils::is_valid_protocol_name(const std::string& protocol) {
    std::string lower = protocol;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    return lower == "bgp" || lower == "ospf" || lower == "isis";
}

std::vector<std::string> CLIUtils::split_string(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::istringstream stream(str);
    std::string token;
    
    while (std::getline(stream, token, delimiter)) {
        tokens.push_back(token);
    }
    
    return tokens;
}

std::string CLIUtils::trim_string(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) {
        return "";
    }
    
    size_t end = str.find_last_not_of(" \t\n\r");
    return str.substr(start, end - start + 1);
}

std::string CLIUtils::to_lowercase(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

std::string CLIUtils::to_uppercase(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}
