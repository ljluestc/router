#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <thread>
#include <atomic>
#include <functional>

namespace RouterSim {

// Forward declarations
class RouterSimulator;
class FRRIntegration;
class TrafficShaper;
class NetemImpairments;

// CLI command structure
struct CLICommand {
    std::string name;
    std::string description;
    std::string syntax;
    std::function<bool(const std::vector<std::string>&)> handler;
    std::vector<std::string> aliases;
};

// CLI interface class
class CLIInterface {
public:
    CLIInterface(RouterSimulator* router);
    ~CLIInterface();

    // Control methods
    void start();
    void stop();
    bool is_running() const;

    // Command registration
    void register_command(const CLICommand& command);
    void unregister_command(const std::string& name);

    // Command execution
    bool execute_command(const std::string& input);
    std::vector<std::string> get_command_suggestions(const std::string& partial) const;

    // Help system
    void show_help(const std::string& command = "");
    void show_commands() const;

    // Configuration
    void set_prompt(const std::string& prompt);
    void enable_auto_completion(bool enable);

private:
    // Command parsing
    std::vector<std::string> parse_command(const std::string& input) const;
    bool validate_command(const std::vector<std::string>& tokens) const;

    // Built-in commands
    void register_builtin_commands();
    bool cmd_help(const std::vector<std::string>& args);
    bool cmd_quit(const std::vector<std::string>& args);
    bool cmd_show(const std::vector<std::string>& args);
    bool cmd_configure(const std::vector<std::string>& args);
    bool cmd_interface(const std::vector<std::string>& args);
    bool cmd_protocol(const std::vector<std::string>& args);
    bool cmd_traffic(const std::vector<std::string>& args);
    bool cmd_impairment(const std::vector<std::string>& args);
    bool cmd_statistics(const std::vector<std::string>& args);
    bool cmd_scenario(const std::vector<std::string>& args);

    // Show commands
    bool show_interfaces(const std::vector<std::string>& args);
    bool show_protocols(const std::vector<std::string>& args);
    bool show_routes(const std::vector<std::string>& args);
    bool show_neighbors(const std::vector<std::string>& args);
    bool show_statistics(const std::vector<std::string>& args);
    bool show_scenarios(const std::vector<std::string>& args);

    // Configuration commands
    bool configure_interface(const std::vector<std::string>& args);
    bool configure_protocol(const std::vector<std::string>& args);
    bool configure_traffic_shaping(const std::vector<std::string>& args);
    bool configure_impairments(const std::vector<std::string>& args);

    // Internal state
    RouterSimulator* router_;
    std::map<std::string, CLICommand> commands_;
    std::string prompt_;
    bool auto_completion_enabled_;
    std::atomic<bool> running_;
    std::thread cli_thread_;

    // Input handling
    void cli_loop();
    std::string read_input() const;
    void process_input(const std::string& input);
};

// CLI utilities
class CLIUtils {
public:
    // Text formatting
    static std::string format_table(const std::vector<std::vector<std::string>>& data, 
                                   const std::vector<std::string>& headers);
    static std::string format_json(const std::map<std::string, std::string>& data);
    static std::string format_yaml(const std::map<std::string, std::string>& data);

    // Input validation
    static bool is_valid_ip_address(const std::string& ip);
    static bool is_valid_subnet_mask(const std::string& mask);
    static bool is_valid_interface_name(const std::string& name);
    static bool is_valid_protocol_name(const std::string& protocol);

    // String utilities
    static std::vector<std::string> split_string(const std::string& str, char delimiter);
    static std::string trim_string(const std::string& str);
    static std::string to_lowercase(const std::string& str);
    static std::string to_uppercase(const std::string& str);
};

} // namespace RouterSim
