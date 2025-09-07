#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <yaml-cpp/yaml.h>

namespace RouterSim {

// Forward declarations
struct RouterConfig;
struct ShapingConfig;
struct ImpairmentConfig;

// YAML configuration manager
class YamlConfig {
public:
    YamlConfig();
    ~YamlConfig() = default;
    
    // Configuration loading/saving
    bool load_config(const std::string& filename, RouterConfig& config);
    bool save_config(const std::string& filename, const RouterConfig& config);
    
    // Scenario loading
    bool load_scenario(const std::string& filename, std::vector<YAML::Node>& steps);
    bool save_scenario(const std::string& filename, const std::vector<YAML::Node>& steps);
    
    // Validation
    bool validate_config(const RouterConfig& config) const;
    bool validate_scenario(const std::vector<YAML::Node>& steps) const;
    
    // Template processing
    std::string process_template(const std::string& template_content, 
                                const std::map<std::string, std::string>& variables) const;
    bool load_template(const std::string& filename, std::string& content) const;
    
    // Variable substitution
    void set_global_variable(const std::string& name, const std::string& value);
    std::string get_global_variable(const std::string& name) const;
    void clear_global_variables();
    
    // Configuration parsing helpers
    static RouterConfig parse_router_config(const YAML::Node& node);
    static ShapingConfig parse_shaping_config(const YAML::Node& node);
    static ImpairmentConfig parse_impairment_config(const YAML::Node& node);
    
    // Configuration serialization helpers
    static YAML::Node serialize_router_config(const RouterConfig& config);
    static YAML::Node serialize_shaping_config(const ShapingConfig& config);
    static YAML::Node serialize_impairment_config(const ImpairmentConfig& config);
    
    // Error handling
    std::string get_last_error() const;
    void clear_errors();
    
private:
    std::map<std::string, std::string> global_variables_;
    std::string last_error_;
    mutable std::mutex variables_mutex_;
    
    // Internal methods
    std::string substitute_variables(const std::string& content, 
                                   const std::map<std::string, std::string>& variables) const;
    bool validate_router_config(const RouterConfig& config) const;
    bool validate_shaping_config(const ShapingConfig& config) const;
    bool validate_impairment_config(const ImpairmentConfig& config) const;
    
    // YAML helpers
    static std::string node_to_string(const YAML::Node& node);
    static YAML::Node string_to_node(const std::string& str);
    static bool is_valid_yaml(const std::string& content);
};

// Scenario execution engine
class ScenarioExecutor {
public:
    ScenarioExecutor(RouterSimulator* router);
    ~ScenarioExecutor() = default;
    
    // Scenario execution
    bool execute_scenario(const std::vector<YAML::Node>& steps);
    bool execute_step(const YAML::Node& step);
    
    // Step handlers
    bool handle_configure_interface(const YAML::Node& step);
    bool handle_configure_protocol(const YAML::Node& step);
    bool handle_start_protocol(const YAML::Node& step);
    bool handle_stop_protocol(const YAML::Node& step);
    bool handle_configure_traffic_shaping(const YAML::Node& step);
    bool handle_configure_impairments(const YAML::Node& step);
    bool handle_send_packet(const YAML::Node& step);
    bool handle_wait(const YAML::Node& step);
    bool handle_assert(const YAML::Node& step);
    bool handle_loop(const YAML::Node& step);
    bool handle_condition(const YAML::Node& step);
    
    // Validation
    bool validate_scenario(const std::vector<YAML::Node>& steps) const;
    bool validate_step(const YAML::Node& step) const;
    
    // Results
    struct ExecutionResult {
        bool success = false;
        std::string error_message;
        std::vector<std::string> warnings;
        std::map<std::string, std::string> variables;
        std::chrono::steady_clock::time_point start_time;
        std::chrono::steady_clock::time_point end_time;
        uint64_t steps_executed = 0;
        uint64_t steps_failed = 0;
    };
    
    ExecutionResult get_last_result() const;
    void clear_results();
    
private:
    RouterSimulator* router_;
    ExecutionResult last_result_;
    std::map<std::string, std::string> scenario_variables_;
    mutable std::mutex result_mutex_;
    
    // Internal methods
    bool execute_loop(const YAML::Node& loop_node, int iterations);
    bool execute_condition(const YAML::Node& condition_node);
    bool evaluate_condition(const YAML::Node& condition) const;
    std::string substitute_variables(const std::string& content) const;
    void update_result(bool success, const std::string& error = "");
};

} // namespace RouterSim