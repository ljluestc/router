#include "cli_interface.h"
#include <iostream>
#include <sstream>

namespace router_sim::cli {

CLIInterface::CLIInterface() 
    : output_stream_(&std::cout), input_stream_(&std::cin), running_(false) {
    initialize_default_commands();
}

bool CLIInterface::initialize() {
    context_ = CLIContext{};
    return true;
}

bool CLIInterface::start() {
    running_ = true;
    return true;
}

bool CLIInterface::stop() {
    running_ = false;
    return true;
}

bool CLIInterface::is_running() const {
    return running_;
}

bool CLIInterface::register_command(const Command& command) {
    commands_[command.name] = command;
    return true;
}

bool CLIInterface::execute_command(const std::string& input) {
    if (!running_) return false;
    
    Command command;
    if (!parse_command(input, command)) {
        *output_stream_ << "Error: Invalid command" << std::endl;
        return false;
    }
    
    auto it = commands_.find(command.name);
    if (it == commands_.end()) {
        *output_stream_ << "Command not found: " << command.name << std::endl;
        return false;
    }
    
    if (it->second.handler) {
        return it->second.handler(command);
    }
    
    return true;
}

void CLIInterface::show_help() {
    *output_stream_ << "Available commands:" << std::endl;
    for (const auto& pair : commands_) {
        *output_stream_ << "  " << pair.first << " - " << pair.second.description << std::endl;
    }
}

bool CLIInterface::parse_command(const std::string& input, Command& command) {
    std::vector<std::string> tokens = tokenize(input);
    if (tokens.empty()) return false;
    
    command.name = tokens[0];
    command.arguments.assign(tokens.begin() + 1, tokens.end());
    
    auto it = commands_.find(command.name);
    if (it != commands_.end()) {
        command.type = it->second.type;
        command.description = it->second.description;
        command.handler = it->second.handler;
    }
    
    return true;
}

std::vector<std::string> CLIInterface::tokenize(const std::string& input) {
    std::vector<std::string> tokens;
    std::istringstream stream(input);
    std::string token;
    
    while (stream >> token) {
        tokens.push_back(token);
    }
    
    return tokens;
}

void CLIInterface::initialize_default_commands() {
    Command help_cmd;
    help_cmd.type = CommandType::HELP;
    help_cmd.name = "help";
    help_cmd.description = "Show help information";
    help_cmd.handler = [this](const Command& cmd) {
        show_help();
        return true;
    };
    register_command(help_cmd);
    
    Command quit_cmd;
    quit_cmd.type = CommandType::QUIT;
    quit_cmd.name = "quit";
    quit_cmd.description = "Exit the CLI";
    quit_cmd.handler = [this](const Command& cmd) {
        stop();
        return true;
    };
    register_command(quit_cmd);
}

} // namespace router_sim::cli
