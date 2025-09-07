#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <yaml-cpp/yaml.h>

namespace RouterSim {

// Forward declarations
struct RouterConfig;
struct InterfaceConfig;
struct BGPConfig;
struct OSPFConfig;
struct ISISConfig;
struct ShapingConfig;
struct ImpairmentConfig;

// Scenario step types
enum class ScenarioStepType {
    CONFIGURE_INTERFACE,
    START_PROTOCOL,
    STOP_PROTOCOL,
    CONFIGURE_TRAFFIC_SHAPING,
    CONFIGURE_IMPAIRMENTS,
    SEND_PACKET,
    WAIT,
    VERIFY_ROUTE,
    VERIFY_NEIGHBOR,
    VERIFY_STATISTICS,
    CUSTOM_COMMAND
};

// Scenario step
struct ScenarioStep {
    ScenarioStepType type;
    std::string name;
    std::string description;
    std::map<std::string, std::string> parameters;
    uint32_t timeout_ms = 5000;
    bool required = true;
    std::vector<std::string> expected_results;
};

// Scenario definition
struct Scenario {
    std::string name;
    std::string description;
    std::string version;
    std::vector<ScenarioStep> steps;
    std::map<std::string, std::string> variables;
    uint32_t timeout_ms = 300000; // 5 minutes default
    bool enabled = true;
};

// Test case definition
struct TestCase {
    std::string name;
    std::string description;
    std::vector<Scenario> scenarios;
    std::map<std::string, std::string> global_variables;
    std::vector<std::string> tags;
    bool enabled = true;
};

// YAML configuration manager
class YamlConfig {
public:
    YamlConfig();
    ~YamlConfig() = default;

    // Configuration loading
    bool load_router_config(const std::string& filename, RouterConfig& config);
    bool save_router_config(const std::string& filename, const RouterConfig& config);

    // Scenario management
    bool load_scenarios(const std::string& filename);
    bool save_scenarios(const std::string& filename);
    std::vector<Scenario> get_scenarios() const;
    std::vector<Scenario> get_scenarios_by_tag(const std::string& tag) const;
    Scenario get_scenario(const std::string& name) const;

    // Test case management
    bool load_test_cases(const std::string& filename);
    bool save_test_cases(const std::string& filename);
    std::vector<TestCase> get_test_cases() const;
    TestCase get_test_case(const std::string& name) const;

    // Scenario execution
    bool execute_scenario(const std::string& scenario_name);
    bool execute_test_case(const std::string& test_case_name);
    std::vector<std::string> get_execution_log() const;

    // Validation
    bool validate_scenario(const Scenario& scenario) const;
    bool validate_test_case(const TestCase& test_case) const;

private:
    // Configuration parsing
    RouterConfig parse_router_config(const YAML::Node& node) const;
    InterfaceConfig parse_interface_config(const YAML::Node& node) const;
    BGPConfig parse_bgp_config(const YAML::Node& node) const;
    OSPFConfig parse_ospf_config(const YAML::Node& node) const;
    ISISConfig parse_isis_config(const YAML::Node& node) const;
    ShapingConfig parse_shaping_config(const YAML::Node& node) const;
    ImpairmentConfig parse_impairment_config(const YAML::Node& node) const;

    // Scenario parsing
    Scenario parse_scenario(const YAML::Node& node) const;
    ScenarioStep parse_scenario_step(const YAML::Node& node) const;
    TestCase parse_test_case(const YAML::Node& node) const;

    // Configuration generation
    YAML::Node generate_router_config(const RouterConfig& config) const;
    YAML::Node generate_interface_config(const InterfaceConfig& config) const;
    YAML::Node generate_bgp_config(const BGPConfig& config) const;
    YAML::Node generate_ospf_config(const OSPFConfig& config) const;
    YAML::Node generate_isis_config(const ISISConfig& config) const;
    YAML::Node generate_shaping_config(const ShapingConfig& config) const;
    YAML::Node generate_impairment_config(const ImpairmentConfig& config) const;

    // Scenario generation
    YAML::Node generate_scenario(const Scenario& scenario) const;
    YAML::Node generate_scenario_step(const ScenarioStep& step) const;
    YAML::Node generate_test_case(const TestCase& test_case) const;

    // Variable substitution
    std::string substitute_variables(const std::string& text, 
                                   const std::map<std::string, std::string>& variables) const;

    // Internal state
    std::vector<Scenario> scenarios_;
    std::vector<TestCase> test_cases_;
    std::vector<std::string> execution_log_;
    mutable std::mutex scenarios_mutex_;
    mutable std::mutex test_cases_mutex_;
    mutable std::mutex log_mutex_;
};

// YAML utilities
class YamlUtils {
public:
    // Validation helpers
    static bool is_valid_yaml(const std::string& filename);
    static bool has_required_fields(const YAML::Node& node, const std::vector<std::string>& fields);
    static bool is_valid_ip_address(const std::string& ip);
    static bool is_valid_subnet_mask(const std::string& mask);
    static bool is_valid_as_number(const std::string& as);

    // Conversion helpers
    static std::string node_to_string(const YAML::Node& node);
    static YAML::Node string_to_node(const std::string& str);
    static std::map<std::string, std::string> node_to_map(const YAML::Node& node);

    // File operations
    static bool file_exists(const std::string& filename);
    static bool create_directory(const std::string& path);
    static std::vector<std::string> list_yaml_files(const std::string& directory);
};

} // namespace RouterSim
