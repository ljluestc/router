#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <thread>
#include <atomic>

namespace RouterSim {

// Forward declaration
class RouterSimulator;

// CLI command structure
struct CLICommand {
    std::string name;
    std::string description;
    std::vector<std::string> aliases;
    std::function<bool(const std::vector<std::string>&)> handler;
    std::string usage;
    std::string help_text;
};

// CLI interface class
class CLIInterface {
public:
    CLIInterface(RouterSimulator* router);
    ~CLIInterface();
    
    // Core functionality
    bool start();
    bool stop();
    bool is_running() const;
    
    // Command processing
    bool process_command(const std::string& input);
    bool execute_command(const std::string& command, const std::vector<std::string>& args);
    
    // Command registration
    bool register_command(const CLICommand& command);
    bool unregister_command(const std::string& name);
    
    // Help system
    std::string get_help(const std::string& command = "") const;
    std::vector<std::string> get_available_commands() const;
    std::string get_command_usage(const std::string& command) const;
    
    // Auto-completion
    std::vector<std::string> get_completions(const std::string& partial) const;
    std::vector<std::string> get_command_completions(const std::string& command, const std::string& partial) const;
    
    // Output formatting
    void print_help(const std::string& command = "") const;
    void print_error(const std::string& message) const;
    void print_success(const std::string& message) const;
    void print_info(const std::string& message) const;
    void print_warning(const std::string& message) const;
    
    // Interactive mode
    void start_interactive_mode();
    void stop_interactive_mode();
    bool is_interactive_mode() const;
    
    // Configuration
    void set_prompt(const std::string& prompt);
    std::string get_prompt() const;
    
    // History
    void add_to_history(const std::string& command);
    std::vector<std::string> get_history() const;
    void clear_history();
    
private:
    RouterSimulator* router_;
    std::map<std::string, CLICommand> commands_;
    std::atomic<bool> running_;
    std::atomic<bool> interactive_mode_;
    std::string prompt_;
    std::vector<std::string> history_;
    std::thread interactive_thread_;
    mutable std::mutex commands_mutex_;
    mutable std::mutex history_mutex_;
    
    // Internal methods
    void initialize_commands();
    void interactive_loop();
    std::vector<std::string> parse_command(const std::string& input) const;
    std::string trim(const std::string& str) const;
    std::vector<std::string> split(const std::string& str, char delimiter) const;
    
    // Command handlers
    bool handle_show_interfaces(const std::vector<std::string>& args);
    bool handle_show_routes(const std::vector<std::string>& args);
    bool handle_show_neighbors(const std::vector<std::string>& args);
    bool handle_show_protocols(const std::vector<std::string>& args);
    bool handle_show_statistics(const std::vector<std::string>& args);
    bool handle_configure_interface(const std::vector<std::string>& args);
    bool handle_configure_protocol(const std::vector<std::string>& args);
    bool handle_traffic_show(const std::vector<std::string>& args);
    bool handle_traffic_configure(const std::vector<std::string>& args);
    bool handle_impairment_show(const std::vector<std::string>& args);
    bool handle_impairment_configure(const std::vector<std::string>& args);
    bool handle_help(const std::vector<std::string>& args);
    bool handle_exit(const std::vector<std::string>& args);
    bool handle_clear(const std::vector<std::string>& args);
    bool handle_history(const std::vector<std::string>& args);
};

} // namespace RouterSim