#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <iostream>
#include <sstream>

namespace router_sim::cli {

// Command Types
enum class CommandType {
    HELP = 0,
    STATUS = 1,
    CONFIGURE = 2,
    START = 3,
    STOP = 4,
    RESTART = 5,
    SHOW = 6,
    CLEAR = 7,
    TEST = 8,
    SCENARIO = 9,
    QUIT = 10
};

// Command Structure
struct Command {
    CommandType type;
    std::string name;
    std::string description;
    std::vector<std::string> arguments;
    std::map<std::string, std::string> options;
    std::function<bool(const Command&)> handler;
    
    Command() : type(CommandType::HELP) {}
};

// CLI Context
struct CLIContext {
    std::string current_mode;
    std::map<std::string, std::string> variables;
    std::vector<std::string> history;
    bool interactive_mode;
    std::string prompt;
    
    CLIContext() : interactive_mode(true), prompt("router-sim> ") {}
};

// Base CLI Interface
class CLIInterface {
public:
    CLIInterface();
    virtual ~CLIInterface() = default;
    
    // Core operations
    bool initialize();
    bool start();
    bool stop();
    bool is_running() const;
    
    // Command management
    bool register_command(const Command& command);
    bool unregister_command(const std::string& command_name);
    std::vector<Command> get_commands() const;
    
    // Command execution
    bool execute_command(const std::string& input);
    bool execute_command(const Command& command);
    
    // Input/Output
    void set_output_stream(std::ostream* stream);
    void set_input_stream(std::istream* stream);
    
    // Context management
    void set_context(const CLIContext& context);
    CLIContext get_context() const;
    
    // History management
    void add_to_history(const std::string& command);
    std::vector<std::string> get_history() const;
    void clear_history();
    
    // Auto-completion
    std::vector<std::string> get_completions(const std::string& partial) const;
    
    // Help system
    void show_help();
    void show_help(const std::string& command_name);
    void show_usage();

private:
    bool parse_command(const std::string& input, Command& command);
    std::vector<std::string> tokenize(const std::string& input);
    bool validate_command(const Command& command);
    void print_prompt();
    
    std::map<std::string, Command> commands_;
    CLIContext context_;
    std::ostream* output_stream_;
    std::istream* input_stream_;
    std::atomic<bool> running_;
    
    void initialize_default_commands();
};

// Router-specific CLI Commands
class RouterCLI {
public:
    RouterCLI();
    virtual ~RouterCLI() = default;
    
    // Core operations
    bool initialize();
    bool start();
    bool stop();
    
    // Command handlers
    bool handle_help(const Command& command);
    bool handle_status(const Command& command);
    bool handle_configure(const Command& command);
    bool handle_start(const Command& command);
    bool handle_stop(const Command& command);
    bool handle_restart(const Command& command);
    bool handle_show(const Command& command);
    bool handle_clear(const Command& command);
    bool handle_test(const Command& command);
    bool handle_scenario(const Command& command);
    bool handle_quit(const Command& command);
    
    // Show commands
    bool show_interfaces();
    bool show_routes();
    bool show_neighbors();
    bool show_protocols();
    bool show_statistics();
    bool show_traffic_shaping();
    bool show_impairments();
    
    // Configuration commands
    bool configure_interface(const std::string& interface, const std::map<std::string, std::string>& config);
    bool configure_protocol(const std::string& protocol, const std::map<std::string, std::string>& config);
    bool configure_traffic_shaping(const std::map<std::string, std::string>& config);
    bool configure_impairments(const std::map<std::string, std::string>& config);
    
    // Test commands
    bool run_test(const std::string& test_name);
    bool run_all_tests();
    bool run_scenario(const std::string& scenario_name);
    
    // Scenario management
    bool load_scenario(const std::string& scenario_file);
    bool save_scenario(const std::string& scenario_file);
    std::vector<std::string> get_available_scenarios() const;

private:
    std::unique_ptr<CLIInterface> cli_interface_;
    std::map<std::string, std::function<bool(const Command&)>> command_handlers_;
    
    void register_router_commands();
    bool execute_show_command(const std::string& subcommand);
    bool execute_configure_command(const std::string& subcommand, const std::vector<std::string>& args);
};

// YAML Configuration Parser
class YAMLConfigParser {
public:
    YAMLConfigParser();
    virtual ~YAMLConfigParser() = default;
    
    // Configuration loading
    bool load_config(const std::string& filename);
    bool save_config(const std::string& filename);
    
    // Configuration access
    std::map<std::string, std::string> get_global_config() const;
    std::map<std::string, std::map<std::string, std::string>> get_interfaces() const;
    std::map<std::string, std::map<std::string, std::string>> get_protocols() const;
    std::map<std::string, std::map<std::string, std::string>> get_traffic_shaping() const;
    std::map<std::string, std::map<std::string, std::string>> get_impairments() const;
    std::map<std::string, std::map<std::string, std::string>> get_scenarios() const;
    
    // Configuration modification
    bool set_global_config(const std::map<std::string, std::string>& config);
    bool set_interface_config(const std::string& interface, const std::map<std::string, std::string>& config);
    bool set_protocol_config(const std::string& protocol, const std::map<std::string, std::string>& config);
    bool set_traffic_shaping_config(const std::map<std::string, std::string>& config);
    bool set_impairment_config(const std::string& interface, const std::map<std::string, std::string>& config);
    bool set_scenario_config(const std::string& scenario, const std::map<std::string, std::string>& config);
    
    // Validation
    bool validate_config() const;
    std::vector<std::string> get_validation_errors() const;

private:
    std::map<std::string, std::string> global_config_;
    std::map<std::string, std::map<std::string, std::string>> interfaces_;
    std::map<std::string, std::map<std::string, std::string>> protocols_;
    std::map<std::string, std::map<std::string, std::string>> traffic_shaping_;
    std::map<std::string, std::map<std::string, std::string>> impairments_;
    std::map<std::string, std::map<std::string, std::string>> scenarios_;
    
    std::vector<std::string> validation_errors_;
    
    bool parse_yaml_file(const std::string& filename);
    bool write_yaml_file(const std::string& filename);
    bool validate_interface_config(const std::string& interface, const std::map<std::string, std::string>& config) const;
    bool validate_protocol_config(const std::string& protocol, const std::map<std::string, std::string>& config) const;
    bool validate_traffic_shaping_config(const std::map<std::string, std::string>& config) const;
    bool validate_impairment_config(const std::string& interface, const std::map<std::string, std::string>& config) const;
};

// Scenario Manager
class ScenarioManager {
public:
    ScenarioManager();
    virtual ~ScenarioManager() = default;
    
    // Scenario management
    bool load_scenario(const std::string& filename);
    bool save_scenario(const std::string& filename);
    bool run_scenario(const std::string& scenario_name);
    bool stop_scenario(const std::string& scenario_name);
    
    // Scenario information
    std::vector<std::string> get_available_scenarios() const;
    std::string get_scenario_description(const std::string& scenario_name) const;
    std::map<std::string, std::string> get_scenario_config(const std::string& scenario_name) const;
    
    // Scenario creation
    bool create_scenario(const std::string& name, const std::string& description,
                        const std::map<std::string, std::string>& config);
    bool delete_scenario(const std::string& scenario_name);
    bool modify_scenario(const std::string& scenario_name, const std::map<std::string, std::string>& config);

private:
    std::map<std::string, std::map<std::string, std::string>> scenarios_;
    std::map<std::string, std::string> scenario_descriptions_;
    std::unique_ptr<YAMLConfigParser> config_parser_;
    
    bool load_scenario_from_file(const std::string& filename);
    bool save_scenario_to_file(const std::string& filename, const std::string& scenario_name);
};

} // namespace router_sim::cli
