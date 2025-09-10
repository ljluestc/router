#include "config/yaml_config.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <yaml-cpp/yaml.h>

namespace RouterSim {

YAMLConfig::YAMLConfig() : initialized_(false) {
}

YAMLConfig::~YAMLConfig() = default;

bool YAMLConfig::initialize() {
    if (initialized_) {
        return true;
    }

    initialized_ = true;
    return true;
}

bool YAMLConfig::load_config(const std::string& file_path) {
    if (!initialized_) {
        if (!initialize()) {
            return false;
        }
    }

    try {
        YAML::Node config = YAML::LoadFile(file_path);
        
        // Parse router configuration
        if (config["router"]) {
            parse_router_config(config["router"]);
        }
        
        // Parse interfaces
        if (config["interfaces"]) {
            parse_interfaces_config(config["interfaces"]);
        }
        
        // Parse protocols
        if (config["protocols"]) {
            parse_protocols_config(config["protocols"]);
        }
        
        // Parse traffic shaping
        if (config["traffic_shaping"]) {
            parse_traffic_shaping_config(config["traffic_shaping"]);
        }
        
        // Parse netem impairments
        if (config["netem"]) {
            parse_netem_config(config["netem"]);
        }
        
        // Parse scenarios
        if (config["scenarios"]) {
            parse_scenarios_config(config["scenarios"]);
        }
        
        return true;
    } catch (const YAML::Exception& e) {
        std::cerr << "YAML parsing error: " << e.what() << std::endl;
        return false;
    }
}

bool YAMLConfig::save_config(const std::string& file_path) const {
    if (!initialized_) {
        return false;
    }

    try {
        YAML::Node config;
        
        // Router configuration
        config["router"] = serialize_router_config();
        
        // Interfaces
        config["interfaces"] = serialize_interfaces_config();
        
        // Protocols
        config["protocols"] = serialize_protocols_config();
        
        // Traffic shaping
        config["traffic_shaping"] = serialize_traffic_shaping_config();
        
        // Netem impairments
        config["netem"] = serialize_netem_config();
        
        // Scenarios
        config["scenarios"] = serialize_scenarios_config();
        
        std::ofstream file(file_path);
        file << config;
        
        return true;
    } catch (const YAML::Exception& e) {
        std::cerr << "YAML serialization error: " << e.what() << std::endl;
        return false;
    }
}

bool YAMLConfig::load_scenario(const std::string& scenario_name) {
    if (!initialized_) {
        if (!initialize()) {
            return false;
        }
    }

    auto it = scenarios_.find(scenario_name);
    if (it == scenarios_.end()) {
        std::cerr << "Scenario not found: " << scenario_name << std::endl;
        return false;
    }

    const ScenarioConfig& scenario = it->second;
    
    // Apply scenario configuration
    if (scenario.router_config) {
        router_config_ = *scenario.router_config;
    }
    
    if (!scenario.interfaces.empty()) {
        interfaces_config_ = scenario.interfaces;
    }
    
    if (!scenario.protocols.empty()) {
        protocols_config_ = scenario.protocols;
    }
    
    if (scenario.traffic_shaping_config) {
        traffic_shaping_config_ = *scenario.traffic_shaping_config;
    }
    
    if (!scenario.netem_configs.empty()) {
        netem_configs_ = scenario.netem_configs;
    }
    
    return true;
}

std::vector<std::string> YAMLConfig::get_available_scenarios() const {
    std::vector<std::string> scenarios;
    for (const auto& [name, _] : scenarios_) {
        scenarios.push_back(name);
    }
    return scenarios;
}

RouterConfig YAMLConfig::get_router_config() const {
    return router_config_;
}

std::vector<InterfaceConfig> YAMLConfig::get_interfaces_config() const {
    return interfaces_config_;
}

std::map<std::string, ProtocolConfig> YAMLConfig::get_protocols_config() const {
    return protocols_config_;
}

TrafficShapingConfig YAMLConfig::get_traffic_shaping_config() const {
    return traffic_shaping_config_;
}

std::vector<NetemConfig> YAMLConfig::get_netem_configs() const {
    return netem_configs_;
}

void YAMLConfig::parse_router_config(const YAML::Node& node) {
    if (node["hostname"]) {
        router_config_.hostname = node["hostname"].as<std::string>();
    }
    
    if (node["router_id"]) {
        router_config_.router_id = node["router_id"].as<std::string>();
    }
    
    if (node["log_level"]) {
        router_config_.log_level = node["log_level"].as<std::string>();
    }
    
    if (node["enable_ipv6"]) {
        router_config_.enable_ipv6 = node["enable_ipv6"].as<bool>();
    }
    
    if (node["enable_mpls"]) {
        router_config_.enable_mpls = node["enable_mpls"].as<bool>();
    }
}

void YAMLConfig::parse_interfaces_config(const YAML::Node& node) {
    for (const auto& interface_node : node) {
        InterfaceConfig interface;
        
        if (interface_node["name"]) {
            interface.name = interface_node["name"].as<std::string>();
        }
        
        if (interface_node["ip_address"]) {
            interface.ip_address = interface_node["ip_address"].as<std::string>();
        }
        
        if (interface_node["subnet_mask"]) {
            interface.subnet_mask = interface_node["subnet_mask"].as<std::string>();
        }
        
        if (interface_node["mtu"]) {
            interface.mtu = interface_node["mtu"].as<uint32_t>();
        }
        
        if (interface_node["enabled"]) {
            interface.enabled = interface_node["enabled"].as<bool>();
        }
        
        if (interface_node["description"]) {
            interface.description = interface_node["description"].as<std::string>();
        }
        
        interfaces_config_.push_back(interface);
    }
}

void YAMLConfig::parse_protocols_config(const YAML::Node& node) {
    for (const auto& protocol_node : node) {
        std::string protocol_name = protocol_node.first.as<std::string>();
        ProtocolConfig protocol;
        
        const YAML::Node& config = protocol_node.second;
        
        if (config["enabled"]) {
            protocol.enabled = config["enabled"].as<bool>();
        }
        
        if (config["update_interval_ms"]) {
            protocol.update_interval_ms = config["update_interval_ms"].as<uint32_t>();
        }
        
        if (config["parameters"]) {
            for (const auto& param : config["parameters"]) {
                std::string key = param.first.as<std::string>();
                std::string value = param.second.as<std::string>();
                protocol.parameters[key] = value;
            }
        }
        
        protocols_config_[protocol_name] = protocol;
    }
}

void YAMLConfig::parse_traffic_shaping_config(const YAML::Node& node) {
    if (node["algorithm"]) {
        std::string algo = node["algorithm"].as<std::string>();
        if (algo == "token_bucket") {
            traffic_shaping_config_.algorithm = ShapingAlgorithm::TOKEN_BUCKET;
        } else if (algo == "weighted_fair_queue") {
            traffic_shaping_config_.algorithm = ShapingAlgorithm::WEIGHTED_FAIR_QUEUE;
        } else if (algo == "priority_queue") {
            traffic_shaping_config_.algorithm = ShapingAlgorithm::PRIORITY_QUEUE;
        } else if (algo == "rate_limiting") {
            traffic_shaping_config_.algorithm = ShapingAlgorithm::RATE_LIMITING;
        }
    }
    
    if (node["token_bucket"]) {
        const YAML::Node& tb = node["token_bucket"];
        if (tb["capacity"]) {
            traffic_shaping_config_.token_bucket_config.capacity = tb["capacity"].as<uint64_t>();
        }
        if (tb["rate"]) {
            traffic_shaping_config_.token_bucket_config.rate = tb["rate"].as<uint64_t>();
        }
        if (tb["burst_size"]) {
            traffic_shaping_config_.token_bucket_config.burst_size = tb["burst_size"].as<uint64_t>();
        }
        if (tb["allow_burst"]) {
            traffic_shaping_config_.token_bucket_config.allow_burst = tb["allow_burst"].as<bool>();
        }
    }
    
    if (node["wfq_classes"]) {
        for (const auto& class_node : node["wfq_classes"]) {
            WFQClass wfq_class;
            
            if (class_node["class_id"]) {
                wfq_class.class_id = class_node["class_id"].as<uint8_t>();
            }
            if (class_node["weight"]) {
                wfq_class.weight = class_node["weight"].as<uint32_t>();
            }
            if (class_node["min_bandwidth"]) {
                wfq_class.min_bandwidth = class_node["min_bandwidth"].as<uint64_t>();
            }
            if (class_node["max_bandwidth"]) {
                wfq_class.max_bandwidth = class_node["max_bandwidth"].as<uint64_t>();
            }
            if (class_node["name"]) {
                wfq_class.name = class_node["name"].as<std::string>();
            }
            if (class_node["is_active"]) {
                wfq_class.is_active = class_node["is_active"].as<bool>();
            }
            
            traffic_shaping_config_.wfq_classes.push_back(wfq_class);
        }
    }
}

void YAMLConfig::parse_netem_config(const YAML::Node& node) {
    for (const auto& netem_node : node) {
        NetemConfig netem;
        
        if (netem_node["interface"]) {
            netem.interface = netem_node["interface"].as<std::string>();
        }
        
        if (netem_node["delay"]) {
            const YAML::Node& delay = netem_node["delay"];
            if (delay["delay_ms"]) {
                netem.delay_config.delay_ms = delay["delay_ms"].as<uint32_t>();
            }
            if (delay["jitter_ms"]) {
                netem.delay_config.jitter_ms = delay["jitter_ms"].as<uint32_t>();
            }
            if (delay["distribution"]) {
                std::string dist = delay["distribution"].as<std::string>();
                if (dist == "uniform") {
                    netem.delay_config.distribution = DelayDistribution::UNIFORM;
                } else if (dist == "normal") {
                    netem.delay_config.distribution = DelayDistribution::NORMAL;
                } else if (dist == "pareto") {
                    netem.delay_config.distribution = DelayDistribution::PARETO;
                } else if (dist == "paretonormal") {
                    netem.delay_config.distribution = DelayDistribution::PARETONORMAL;
                }
            }
            netem.has_delay = true;
        }
        
        if (netem_node["loss"]) {
            const YAML::Node& loss = netem_node["loss"];
            if (loss["type"]) {
                std::string type = loss["type"].as<std::string>();
                if (type == "random") {
                    netem.loss_config.loss_type = LossType::RANDOM;
                } else if (type == "state") {
                    netem.loss_config.loss_type = LossType::STATE;
                } else if (type == "gemodel") {
                    netem.loss_config.loss_type = LossType::GEMODEL;
                }
            }
            if (loss["percentage"]) {
                netem.loss_config.loss_percentage = loss["percentage"].as<double>();
            }
            netem.has_loss = true;
        }
        
        if (netem_node["duplicate"]) {
            const YAML::Node& dup = netem_node["duplicate"];
            if (dup["percentage"]) {
                netem.duplicate_config.duplicate_percentage = dup["percentage"].as<double>();
            }
            netem.has_duplicate = true;
        }
        
        if (netem_node["corrupt"]) {
            const YAML::Node& corrupt = netem_node["corrupt"];
            if (corrupt["percentage"]) {
                netem.corrupt_config.corrupt_percentage = corrupt["percentage"].as<double>();
            }
            netem.has_corrupt = true;
        }
        
        if (netem_node["reorder"]) {
            const YAML::Node& reorder = netem_node["reorder"];
            if (reorder["type"]) {
                std::string type = reorder["type"].as<std::string>();
                if (type == "percentage") {
                    netem.reorder_config.reorder_type = ReorderType::PERCENTAGE;
                } else if (type == "gap") {
                    netem.reorder_config.reorder_type = ReorderType::GAP;
                }
            }
            if (reorder["percentage"]) {
                netem.reorder_config.reorder_percentage = reorder["percentage"].as<double>();
            }
            if (reorder["gap"]) {
                netem.reorder_config.gap = reorder["gap"].as<uint32_t>();
            }
            netem.has_reorder = true;
        }
        
        if (netem_node["rate_limit"]) {
            const YAML::Node& rate = netem_node["rate_limit"];
            if (rate["rate_kbps"]) {
                netem.rate_limit_config.rate_kbps = rate["rate_kbps"].as<uint32_t>();
            }
            if (rate["burst_kb"]) {
                netem.rate_limit_config.burst_kb = rate["burst_kb"].as<uint32_t>();
            }
            netem.has_rate_limit = true;
        }
        
        netem_configs_.push_back(netem);
    }
}

void YAMLConfig::parse_scenarios_config(const YAML::Node& node) {
    for (const auto& scenario_node : node) {
        std::string scenario_name = scenario_node.first.as<std::string>();
        ScenarioConfig scenario;
        
        const YAML::Node& config = scenario_node.second;
        
        if (config["description"]) {
            scenario.description = config["description"].as<std::string>();
        }
        
        if (config["router"]) {
            scenario.router_config = std::make_unique<RouterConfig>();
            parse_router_config(config["router"]);
            *scenario.router_config = router_config_;
        }
        
        if (config["interfaces"]) {
            parse_interfaces_config(config["interfaces"]);
            scenario.interfaces = interfaces_config_;
        }
        
        if (config["protocols"]) {
            parse_protocols_config(config["protocols"]);
            scenario.protocols = protocols_config_;
        }
        
        if (config["traffic_shaping"]) {
            scenario.traffic_shaping_config = std::make_unique<TrafficShapingConfig>();
            parse_traffic_shaping_config(config["traffic_shaping"]);
            *scenario.traffic_shaping_config = traffic_shaping_config_;
        }
        
        if (config["netem"]) {
            parse_netem_config(config["netem"]);
            scenario.netem_configs = netem_configs_;
        }
        
        scenarios_[scenario_name] = scenario;
    }
}

YAML::Node YAMLConfig::serialize_router_config() const {
    YAML::Node node;
    node["hostname"] = router_config_.hostname;
    node["router_id"] = router_config_.router_id;
    node["log_level"] = router_config_.log_level;
    node["enable_ipv6"] = router_config_.enable_ipv6;
    node["enable_mpls"] = router_config_.enable_mpls;
    return node;
}

YAML::Node YAMLConfig::serialize_interfaces_config() const {
    YAML::Node node;
    for (const auto& interface : interfaces_config_) {
        YAML::Node interface_node;
        interface_node["name"] = interface.name;
        interface_node["ip_address"] = interface.ip_address;
        interface_node["subnet_mask"] = interface.subnet_mask;
        interface_node["mtu"] = interface.mtu;
        interface_node["enabled"] = interface.enabled;
        interface_node["description"] = interface.description;
        node.push_back(interface_node);
    }
    return node;
}

YAML::Node YAMLConfig::serialize_protocols_config() const {
    YAML::Node node;
    for (const auto& [name, protocol] : protocols_config_) {
        YAML::Node protocol_node;
        protocol_node["enabled"] = protocol.enabled;
        protocol_node["update_interval_ms"] = protocol.update_interval_ms;
        
        YAML::Node params;
        for (const auto& [key, value] : protocol.parameters) {
            params[key] = value;
        }
        protocol_node["parameters"] = params;
        
        node[name] = protocol_node;
    }
    return node;
}

YAML::Node YAMLConfig::serialize_traffic_shaping_config() const {
    YAML::Node node;
    
    std::string algo;
    switch (traffic_shaping_config_.algorithm) {
        case ShapingAlgorithm::TOKEN_BUCKET:
            algo = "token_bucket";
            break;
        case ShapingAlgorithm::WEIGHTED_FAIR_QUEUE:
            algo = "weighted_fair_queue";
            break;
        case ShapingAlgorithm::PRIORITY_QUEUE:
            algo = "priority_queue";
            break;
        case ShapingAlgorithm::RATE_LIMITING:
            algo = "rate_limiting";
            break;
    }
    node["algorithm"] = algo;
    
    YAML::Node tb;
    tb["capacity"] = traffic_shaping_config_.token_bucket_config.capacity;
    tb["rate"] = traffic_shaping_config_.token_bucket_config.rate;
    tb["burst_size"] = traffic_shaping_config_.token_bucket_config.burst_size;
    tb["allow_burst"] = traffic_shaping_config_.token_bucket_config.allow_burst;
    node["token_bucket"] = tb;
    
    YAML::Node wfq_classes;
    for (const auto& wfq_class : traffic_shaping_config_.wfq_classes) {
        YAML::Node class_node;
        class_node["class_id"] = wfq_class.class_id;
        class_node["weight"] = wfq_class.weight;
        class_node["min_bandwidth"] = wfq_class.min_bandwidth;
        class_node["max_bandwidth"] = wfq_class.max_bandwidth;
        class_node["name"] = wfq_class.name;
        class_node["is_active"] = wfq_class.is_active;
        wfq_classes.push_back(class_node);
    }
    node["wfq_classes"] = wfq_classes;
    
    return node;
}

YAML::Node YAMLConfig::serialize_netem_config() const {
    YAML::Node node;
    for (const auto& netem : netem_configs_) {
        YAML::Node netem_node;
        netem_node["interface"] = netem.interface;
        
        if (netem.has_delay) {
            YAML::Node delay;
            delay["delay_ms"] = netem.delay_config.delay_ms;
            delay["jitter_ms"] = netem.delay_config.jitter_ms;
            
            std::string dist;
            switch (netem.delay_config.distribution) {
                case DelayDistribution::UNIFORM:
                    dist = "uniform";
                    break;
                case DelayDistribution::NORMAL:
                    dist = "normal";
                    break;
                case DelayDistribution::PARETO:
                    dist = "pareto";
                    break;
                case DelayDistribution::PARETONORMAL:
                    dist = "paretonormal";
                    break;
            }
            delay["distribution"] = dist;
            netem_node["delay"] = delay;
        }
        
        if (netem.has_loss) {
            YAML::Node loss;
            std::string type;
            switch (netem.loss_config.loss_type) {
                case LossType::RANDOM:
                    type = "random";
                    break;
                case LossType::STATE:
                    type = "state";
                    break;
                case LossType::GEMODEL:
                    type = "gemodel";
                    break;
            }
            loss["type"] = type;
            loss["percentage"] = netem.loss_config.loss_percentage;
            netem_node["loss"] = loss;
        }
        
        if (netem.has_duplicate) {
            YAML::Node duplicate;
            duplicate["percentage"] = netem.duplicate_config.duplicate_percentage;
            netem_node["duplicate"] = duplicate;
        }
        
        if (netem.has_corrupt) {
            YAML::Node corrupt;
            corrupt["percentage"] = netem.corrupt_config.corrupt_percentage;
            netem_node["corrupt"] = corrupt;
        }
        
        if (netem.has_reorder) {
            YAML::Node reorder;
            std::string type;
            switch (netem.reorder_config.reorder_type) {
                case ReorderType::PERCENTAGE:
                    type = "percentage";
                    break;
                case ReorderType::GAP:
                    type = "gap";
                    break;
            }
            reorder["type"] = type;
            reorder["percentage"] = netem.reorder_config.reorder_percentage;
            reorder["gap"] = netem.reorder_config.gap;
            netem_node["reorder"] = reorder;
        }
        
        if (netem.has_rate_limit) {
            YAML::Node rate_limit;
            rate_limit["rate_kbps"] = netem.rate_limit_config.rate_kbps;
            rate_limit["burst_kb"] = netem.rate_limit_config.burst_kb;
            netem_node["rate_limit"] = rate_limit;
        }
        
        node.push_back(netem_node);
    }
    return node;
}

YAML::Node YAMLConfig::serialize_scenarios_config() const {
    YAML::Node node;
    for (const auto& [name, scenario] : scenarios_) {
        YAML::Node scenario_node;
        scenario_node["description"] = scenario.description;
        
        if (scenario.router_config) {
            scenario_node["router"] = serialize_router_config();
        }
        
        if (!scenario.interfaces.empty()) {
            scenario_node["interfaces"] = serialize_interfaces_config();
        }
        
        if (!scenario.protocols.empty()) {
            scenario_node["protocols"] = serialize_protocols_config();
        }
        
        if (scenario.traffic_shaping_config) {
            scenario_node["traffic_shaping"] = serialize_traffic_shaping_config();
        }
        
        if (!scenario.netem_configs.empty()) {
            scenario_node["netem"] = serialize_netem_config();
        }
        
        node[name] = scenario_node;
    }
    return node;
}

} // namespace RouterSim
