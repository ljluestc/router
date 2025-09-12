#pragma once

#include "traffic_shaping.h"
#include "netem/impairments.h"
#include <string>
#include <vector>
#include <map>
#include <memory>

namespace RouterSim {

// Configuration structures
struct RouterConfig {
    std::string hostname = "router-sim";
    std::string router_id = "1.1.1.1";
    std::string log_level = "info";
    bool enable_ipv6 = false;
    bool enable_mpls = false;
};

struct InterfaceConfig {
    std::string name;
    std::string ip_address;
    std::string subnet_mask;
    uint32_t mtu = 1500;
    bool enabled = true;
    std::string description;
};

struct ProtocolConfig {
    bool enabled = false;
    uint32_t update_interval_ms = 1000;
    std::map<std::string, std::string> parameters;
};

struct TrafficShapingConfig {
    ShapingAlgorithm algorithm = ShapingAlgorithm::TOKEN_BUCKET;
    TokenBucketConfig token_bucket_config;
    std::vector<WFQClass> wfq_classes;
};

struct NetemConfig {
    std::string interface;
    DelayConfig delay_config;
    LossConfig loss_config;
    DuplicateConfig duplicate_config;
    CorruptConfig corrupt_config;
    ReorderConfig reorder_config;
    RateLimitConfig rate_limit_config;
    bool has_delay = false;
    bool has_loss = false;
    bool has_duplicate = false;
    bool has_corrupt = false;
    bool has_reorder = false;
    bool has_rate_limit = false;
};

struct ScenarioConfig {
    std::string description;
    std::unique_ptr<RouterConfig> router_config;
    std::vector<InterfaceConfig> interfaces;
    std::map<std::string, ProtocolConfig> protocols;
    std::unique_ptr<TrafficShapingConfig> traffic_shaping_config;
    std::vector<NetemConfig> netem_configs;
};

class YAMLConfig {
public:
    YAMLConfig();
    ~YAMLConfig();

    // Core management
    bool initialize();
    bool load_config(const std::string& file_path);
    bool save_config(const std::string& file_path) const;

    // Scenario management
    bool load_scenario(const std::string& scenario_name);
    std::vector<std::string> get_available_scenarios() const;

    // Configuration access
    RouterConfig get_router_config() const;
    std::vector<InterfaceConfig> get_interfaces_config() const;
    std::map<std::string, ProtocolConfig> get_protocols_config() const;
    TrafficShapingConfig get_traffic_shaping_config() const;
    std::vector<NetemConfig> get_netem_configs() const;

private:
    // Internal state
    bool initialized_;
    RouterConfig router_config_;
    std::vector<InterfaceConfig> interfaces_config_;
    std::map<std::string, ProtocolConfig> protocols_config_;
    TrafficShapingConfig traffic_shaping_config_;
    std::vector<NetemConfig> netem_configs_;
    std::map<std::string, ScenarioConfig> scenarios_;

    // Parsing methods
    void parse_router_config(const YAML::Node& node);
    void parse_interfaces_config(const YAML::Node& node);
    void parse_protocols_config(const YAML::Node& node);
    void parse_traffic_shaping_config(const YAML::Node& node);
    void parse_netem_config(const YAML::Node& node);
    void parse_scenarios_config(const YAML::Node& node);

    // Serialization methods
    YAML::Node serialize_router_config() const;
    YAML::Node serialize_interfaces_config() const;
    YAML::Node serialize_protocols_config() const;
    YAML::Node serialize_traffic_shaping_config() const;
    YAML::Node serialize_netem_config() const;
    YAML::Node serialize_scenarios_config() const;
};

} // namespace RouterSim
