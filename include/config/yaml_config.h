#pragma once

#include <string>
#include <vector>
#include <map>
#include <yaml-cpp/yaml.h>
#include "traffic_shaping.h"
#include "netem/impairments.h"

namespace RouterSim {

// Forward declarations
struct RouterConfig;
struct InterfaceConfig;
struct ProtocolConfig;
struct TrafficShapingConfig;
struct ScenarioConfig;
struct ScenarioStep;

// Interface configuration
struct InterfaceConfig {
    std::string name;
    std::string ip_address;
    std::string subnet_mask;
    uint32_t mtu;
    bool enabled;
    
    InterfaceConfig() : mtu(1500), enabled(true) {}
};

// Router configuration
struct RouterConfig {
    std::string hostname;
    std::string router_id;
    uint32_t as_number;
    std::vector<InterfaceConfig> interfaces;
    
    RouterConfig() : as_number(0) {}
};

// Protocol configuration
struct ProtocolConfig {
    std::string name;
    bool enabled;
    std::map<std::string, std::string> config;
    std::map<std::string, std::map<std::string, std::string>> neighbors;
    std::map<std::string, std::map<std::string, std::string>> interfaces;
    
    ProtocolConfig() : enabled(false) {}
};

// Traffic shaping configuration
struct TrafficShapingConfig {
    ShapingAlgorithm algorithm;
    TokenBucketConfig token_bucket_config;
    std::vector<WFQClass> wfq_classes;
    
    TrafficShapingConfig() : algorithm(ShapingAlgorithm::TOKEN_BUCKET) {}
};

// Scenario step
struct ScenarioStep {
    ImpairmentType type;
    uint32_t delay_ms;
    std::string interface;
    std::map<std::string, std::string> config;
    
    ScenarioStep() : type(ImpairmentType::DELAY), delay_ms(0) {}
};

// Scenario configuration
struct ScenarioConfig {
    std::string name;
    std::string description;
    uint32_t duration_seconds;
    std::vector<ScenarioStep> steps;
    
    ScenarioConfig() : duration_seconds(0) {}
};

// YAML configuration manager
class YAMLConfig {
public:
    YAMLConfig();
    ~YAMLConfig();
    
    // Initialization
    bool initialize();
    
    // File operations
    bool load_from_file(const std::string& filename);
    bool save_to_file(const std::string& filename) const;
    
    // String operations
    bool load_from_string(const std::string& yaml_string);
    std::string save_to_string() const;
    
    // Configuration getters
    RouterConfig get_router_config() const;
    std::map<std::string, ProtocolConfig> get_protocol_configs() const;
    std::map<std::string, TrafficShapingConfig> get_traffic_shaping_configs() const;
    std::map<std::string, ImpairmentConfig> get_impairment_configs() const;
    std::vector<ScenarioConfig> get_scenario_configs() const;
    
    // Configuration setters
    void set_router_config(const RouterConfig& config);
    void set_protocol_configs(const std::map<std::string, ProtocolConfig>& configs);
    void set_traffic_shaping_configs(const std::map<std::string, TrafficShapingConfig>& configs);
    void set_impairment_configs(const std::map<std::string, ImpairmentConfig>& configs);
    void set_scenario_configs(const std::vector<ScenarioConfig>& configs);
    
    // Utility methods
    bool has_key(const std::string& key) const;
    std::string get_string(const std::string& key, const std::string& default_value = "") const;
    int get_int(const std::string& key, int default_value = 0) const;
    bool get_bool(const std::string& key, bool default_value = false) const;
    
    void set_string(const std::string& key, const std::string& value);
    void set_int(const std::string& key, int value);
    void set_bool(const std::string& key, bool value);
    
private:
    bool initialized_;
    YAML::Node config_node_;
};

} // namespace RouterSim
