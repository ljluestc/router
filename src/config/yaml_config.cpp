#include "config/yaml_config.h"
#include <iostream>
#include <fstream>
#include <yaml-cpp/yaml.h>

namespace RouterSim {

YAMLConfig::YAMLConfig() : initialized_(false) {
}

YAMLConfig::~YAMLConfig() = default;

bool YAMLConfig::initialize() {
    if (initialized_) {
        return true;
    }
    
    // Check if yaml-cpp is available
    try {
        YAML::Node test_node;
        test_node["test"] = "value";
        initialized_ = true;
        std::cout << "YAML configuration initialized successfully" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize YAML configuration: " << e.what() << std::endl;
        return false;
    }
}

bool YAMLConfig::load_from_file(const std::string& filename) {
    if (!initialized_) {
        if (!initialize()) {
            return false;
        }
    }
    
    try {
        config_node_ = YAML::LoadFile(filename);
        std::cout << "Configuration loaded from " << filename << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to load configuration from " << filename << ": " << e.what() << std::endl;
        return false;
    }
}

bool YAMLConfig::save_to_file(const std::string& filename) const {
    if (!initialized_) {
        return false;
    }
    
    try {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Failed to open file for writing: " << filename << std::endl;
            return false;
        }
        
        file << config_node_;
        file.close();
        
        std::cout << "Configuration saved to " << filename << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to save configuration to " << filename << ": " << e.what() << std::endl;
        return false;
    }
}

bool YAMLConfig::load_from_string(const std::string& yaml_string) {
    if (!initialized_) {
        if (!initialize()) {
            return false;
        }
    }
    
    try {
        config_node_ = YAML::Load(yaml_string);
        std::cout << "Configuration loaded from string" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to load configuration from string: " << e.what() << std::endl;
        return false;
    }
}

std::string YAMLConfig::save_to_string() const {
    if (!initialized_) {
        return "";
    }
    
    try {
        std::ostringstream oss;
        oss << config_node_;
        return oss.str();
    } catch (const std::exception& e) {
        std::cerr << "Failed to save configuration to string: " << e.what() << std::endl;
        return "";
    }
}

// Router configuration
RouterConfig YAMLConfig::get_router_config() const {
    RouterConfig config;
    
    if (config_node_["router"]) {
        auto router = config_node_["router"];
        
        if (router["hostname"]) {
            config.hostname = router["hostname"].as<std::string>();
        }
        
        if (router["router_id"]) {
            config.router_id = router["router_id"].as<std::string>();
        }
        
        if (router["as_number"]) {
            config.as_number = router["as_number"].as<uint32_t>();
        }
        
        if (router["interfaces"]) {
            auto interfaces = router["interfaces"];
            for (const auto& interface : interfaces) {
                InterfaceConfig if_config;
                
                if (interface["name"]) {
                    if_config.name = interface["name"].as<std::string>();
                }
                
                if (interface["ip_address"]) {
                    if_config.ip_address = interface["ip_address"].as<std::string>();
                }
                
                if (interface["subnet_mask"]) {
                    if_config.subnet_mask = interface["subnet_mask"].as<std::string>();
                }
                
                if (interface["mtu"]) {
                    if_config.mtu = interface["mtu"].as<uint32_t>();
                }
                
                if (interface["enabled"]) {
                    if_config.enabled = interface["enabled"].as<bool>();
                }
                
                config.interfaces.push_back(if_config);
            }
        }
    }
    
    return config;
}

// Protocol configuration
std::map<std::string, ProtocolConfig> YAMLConfig::get_protocol_configs() const {
    std::map<std::string, ProtocolConfig> configs;
    
    if (config_node_["protocols"]) {
        auto protocols = config_node_["protocols"];
        
        // BGP configuration
        if (protocols["bgp"]) {
            auto bgp = protocols["bgp"];
            ProtocolConfig bgp_config;
            bgp_config.name = "bgp";
            bgp_config.enabled = bgp["enabled"].as<bool>();
            
            if (bgp["as_number"]) {
                bgp_config.config["as_number"] = std::to_string(bgp["as_number"].as<uint32_t>());
            }
            
            if (bgp["router_id"]) {
                bgp_config.config["router_id"] = bgp["router_id"].as<std::string>();
            }
            
            if (bgp["neighbors"]) {
                auto neighbors = bgp["neighbors"];
                for (const auto& neighbor : neighbors) {
                    std::string address = neighbor["address"].as<std::string>();
                    std::map<std::string, std::string> neighbor_config;
                    
                    if (neighbor["as_number"]) {
                        neighbor_config["as_number"] = std::to_string(neighbor["as_number"].as<uint32_t>());
                    }
                    
                    if (neighbor["hold_time"]) {
                        neighbor_config["hold_time"] = std::to_string(neighbor["hold_time"].as<uint32_t>());
                    }
                    
                    if (neighbor["keepalive_time"]) {
                        neighbor_config["keepalive_time"] = std::to_string(neighbor["keepalive_time"].as<uint32_t>());
                    }
                    
                    bgp_config.neighbors[address] = neighbor_config;
                }
            }
            
            configs["bgp"] = bgp_config;
        }
        
        // OSPF configuration
        if (protocols["ospf"]) {
            auto ospf = protocols["ospf"];
            ProtocolConfig ospf_config;
            ospf_config.name = "ospf";
            ospf_config.enabled = ospf["enabled"].as<bool>();
            
            if (ospf["router_id"]) {
                ospf_config.config["router_id"] = ospf["router_id"].as<std::string>();
            }
            
            if (ospf["area_id"]) {
                ospf_config.config["area_id"] = ospf["area_id"].as<std::string>();
            }
            
            if (ospf["interfaces"]) {
                auto interfaces = ospf["interfaces"];
                for (const auto& interface : interfaces) {
                    std::string name = interface["name"].as<std::string>();
                    std::map<std::string, std::string> interface_config;
                    
                    if (interface["area_id"]) {
                        interface_config["area_id"] = interface["area_id"].as<std::string>();
                    }
                    
                    if (interface["cost"]) {
                        interface_config["cost"] = std::to_string(interface["cost"].as<uint32_t>());
                    }
                    
                    if (interface["priority"]) {
                        interface_config["priority"] = std::to_string(interface["priority"].as<uint32_t>());
                    }
                    
                    ospf_config.interfaces[name] = interface_config;
                }
            }
            
            configs["ospf"] = ospf_config;
        }
        
        // IS-IS configuration
        if (protocols["isis"]) {
            auto isis = protocols["isis"];
            ProtocolConfig isis_config;
            isis_config.name = "isis";
            isis_config.enabled = isis["enabled"].as<bool>();
            
            if (isis["system_id"]) {
                isis_config.config["system_id"] = isis["system_id"].as<std::string>();
            }
            
            if (isis["area_id"]) {
                isis_config.config["area_id"] = isis["area_id"].as<std::string>();
            }
            
            if (isis["interfaces"]) {
                auto interfaces = isis["interfaces"];
                for (const auto& interface : interfaces) {
                    std::string name = interface["name"].as<std::string>();
                    std::map<std::string, std::string> interface_config;
                    
                    if (interface["area_id"]) {
                        interface_config["area_id"] = interface["area_id"].as<std::string>();
                    }
                    
                    if (interface["cost"]) {
                        interface_config["cost"] = std::to_string(interface["cost"].as<uint32_t>());
                    }
                    
                    if (interface["priority"]) {
                        interface_config["priority"] = std::to_string(interface["priority"].as<uint32_t>());
                    }
                    
                    isis_config.interfaces[name] = interface_config;
                }
            }
            
            configs["isis"] = isis_config;
        }
    }
    
    return configs;
}

// Traffic shaping configuration
std::map<std::string, TrafficShapingConfig> YAMLConfig::get_traffic_shaping_configs() const {
    std::map<std::string, TrafficShapingConfig> configs;
    
    if (config_node_["traffic_shaping"]) {
        auto traffic_shaping = config_node_["traffic_shaping"];
        
        if (traffic_shaping["interfaces"]) {
            auto interfaces = traffic_shaping["interfaces"];
            for (const auto& interface : interfaces) {
                std::string name = interface["name"].as<std::string>();
                TrafficShapingConfig config;
                
                if (interface["algorithm"]) {
                    std::string algorithm = interface["algorithm"].as<std::string>();
                    if (algorithm == "token_bucket") {
                        config.algorithm = ShapingAlgorithm::TOKEN_BUCKET;
                    } else if (algorithm == "wfq") {
                        config.algorithm = ShapingAlgorithm::WEIGHTED_FAIR_QUEUE;
                    } else if (algorithm == "priority_queue") {
                        config.algorithm = ShapingAlgorithm::PRIORITY_QUEUE;
                    } else if (algorithm == "rate_limiting") {
                        config.algorithm = ShapingAlgorithm::RATE_LIMITING;
                    }
                }
                
                if (interface["token_bucket"]) {
                    auto tb = interface["token_bucket"];
                    TokenBucketConfig tb_config;
                    
                    if (tb["capacity"]) {
                        tb_config.capacity = tb["capacity"].as<uint64_t>();
                    }
                    
                    if (tb["rate"]) {
                        tb_config.rate = tb["rate"].as<uint64_t>();
                    }
                    
                    if (tb["burst_size"]) {
                        tb_config.burst_size = tb["burst_size"].as<uint64_t>();
                    }
                    
                    if (tb["allow_burst"]) {
                        tb_config.allow_burst = tb["allow_burst"].as<bool>();
                    }
                    
                    config.token_bucket_config = tb_config;
                }
                
                if (interface["wfq"]) {
                    auto wfq = interface["wfq"];
                    if (wfq["classes"]) {
                        auto classes = wfq["classes"];
                        for (const auto& wfq_class : classes) {
                            WFQClass class_config;
                            
                            if (wfq_class["class_id"]) {
                                class_config.class_id = wfq_class["class_id"].as<uint8_t>();
                            }
                            
                            if (wfq_class["weight"]) {
                                class_config.weight = wfq_class["weight"].as<uint32_t>();
                            }
                            
                            if (wfq_class["min_bandwidth"]) {
                                class_config.min_bandwidth = wfq_class["min_bandwidth"].as<uint64_t>();
                            }
                            
                            if (wfq_class["max_bandwidth"]) {
                                class_config.max_bandwidth = wfq_class["max_bandwidth"].as<uint64_t>();
                            }
                            
                            if (wfq_class["name"]) {
                                class_config.name = wfq_class["name"].as<std::string>();
                            }
                            
                            if (wfq_class["is_active"]) {
                                class_config.is_active = wfq_class["is_active"].as<bool>();
                            }
                            
                            config.wfq_classes.push_back(class_config);
                        }
                    }
                }
                
                configs[name] = config;
            }
        }
    }
    
    return configs;
}

// Impairment configuration
std::map<std::string, ImpairmentConfig> YAMLConfig::get_impairment_configs() const {
    std::map<std::string, ImpairmentConfig> configs;
    
    if (config_node_["impairments"]) {
        auto impairments = config_node_["impairments"];
        
        if (impairments["interfaces"]) {
            auto interfaces = impairments["interfaces"];
            for (const auto& interface : interfaces) {
                std::string name = interface["name"].as<std::string>();
                ImpairmentConfig config;
                
                if (interface["delay"]) {
                    auto delay = interface["delay"];
                    DelayConfig delay_config;
                    
                    if (delay["delay_ms"]) {
                        delay_config.delay_ms = delay["delay_ms"].as<uint32_t>();
                    }
                    
                    if (delay["jitter_ms"]) {
                        delay_config.jitter_ms = delay["jitter_ms"].as<uint32_t>();
                    }
                    
                    if (delay["distribution"]) {
                        delay_config.distribution = delay["distribution"].as<std::string>();
                    }
                    
                    config.delay = delay_config;
                }
                
                if (interface["loss"]) {
                    auto loss = interface["loss"];
                    LossConfig loss_config;
                    
                    if (loss["loss_type"]) {
                        loss_config.loss_type = loss["loss_type"].as<std::string>();
                    }
                    
                    if (loss["loss_percentage"]) {
                        loss_config.loss_percentage = loss["loss_percentage"].as<double>();
                    }
                    
                    config.loss = loss_config;
                }
                
                if (interface["duplicate"]) {
                    auto duplicate = interface["duplicate"];
                    DuplicateConfig duplicate_config;
                    
                    if (duplicate["duplicate_percentage"]) {
                        duplicate_config.duplicate_percentage = duplicate["duplicate_percentage"].as<double>();
                    }
                    
                    config.duplicate = duplicate_config;
                }
                
                if (interface["corrupt"]) {
                    auto corrupt = interface["corrupt"];
                    CorruptConfig corrupt_config;
                    
                    if (corrupt["corrupt_percentage"]) {
                        corrupt_config.corrupt_percentage = corrupt["corrupt_percentage"].as<double>();
                    }
                    
                    config.corrupt = corrupt_config;
                }
                
                if (interface["reorder"]) {
                    auto reorder = interface["reorder"];
                    ReorderConfig reorder_config;
                    
                    if (reorder["reorder_percentage"]) {
                        reorder_config.reorder_percentage = reorder["reorder_percentage"].as<double>();
                    }
                    
                    if (reorder["gap"]) {
                        reorder_config.gap = reorder["gap"].as<uint32_t>();
                    }
                    
                    config.reorder = reorder_config;
                }
                
                if (interface["rate_limit"]) {
                    auto rate_limit = interface["rate_limit"];
                    RateLimitConfig rate_limit_config;
                    
                    if (rate_limit["rate"]) {
                        rate_limit_config.rate = rate_limit["rate"].as<std::string>();
                    }
                    
                    if (rate_limit["burst"]) {
                        rate_limit_config.burst = rate_limit["burst"].as<uint32_t>();
                    }
                    
                    if (rate_limit["latency"]) {
                        rate_limit_config.latency = rate_limit["latency"].as<uint32_t>();
                    }
                    
                    config.rate_limit = rate_limit_config;
                }
                
                configs[name] = config;
            }
        }
    }
    
    return configs;
}

// Scenario configuration
std::vector<ScenarioConfig> YAMLConfig::get_scenario_configs() const {
    std::vector<ScenarioConfig> configs;
    
    if (config_node_["scenarios"]) {
        auto scenarios = config_node_["scenarios"];
        for (const auto& scenario : scenarios) {
            ScenarioConfig config;
            
            if (scenario["name"]) {
                config.name = scenario["name"].as<std::string>();
            }
            
            if (scenario["description"]) {
                config.description = scenario["description"].as<std::string>();
            }
            
            if (scenario["duration"]) {
                config.duration_seconds = scenario["duration"].as<uint32_t>();
            }
            
            if (scenario["steps"]) {
                auto steps = scenario["steps"];
                for (const auto& step : steps) {
                    ScenarioStep step_config;
                    
                    if (step["type"]) {
                        std::string type = step["type"].as<std::string>();
                        if (type == "delay") {
                            step_config.type = ImpairmentType::DELAY;
                        } else if (type == "loss") {
                            step_config.type = ImpairmentType::LOSS;
                        } else if (type == "duplicate") {
                            step_config.type = ImpairmentType::DUPLICATE;
                        } else if (type == "corrupt") {
                            step_config.type = ImpairmentType::CORRUPT;
                        } else if (type == "reorder") {
                            step_config.type = ImpairmentType::REORDER;
                        } else if (type == "rate_limit") {
                            step_config.type = ImpairmentType::RATE_LIMIT;
                        }
                    }
                    
                    if (step["delay_ms"]) {
                        step_config.delay_ms = step["delay_ms"].as<uint32_t>();
                    }
                    
                    if (step["interface"]) {
                        step_config.interface = step["interface"].as<std::string>();
                    }
                    
                    if (step["config"]) {
                        auto step_config_node = step["config"];
                        // Parse step-specific configuration
                        // This would be implemented based on the specific step type
                    }
                    
                    config.steps.push_back(step_config);
                }
            }
            
            configs.push_back(config);
        }
    }
    
    return configs;
}

// Setter methods
void YAMLConfig::set_router_config(const RouterConfig& config) {
    if (!initialized_) {
        initialize();
    }
    
    config_node_["router"]["hostname"] = config.hostname;
    config_node_["router"]["router_id"] = config.router_id;
    config_node_["router"]["as_number"] = config.as_number;
    
    YAML::Node interfaces = YAML::Node(YAML::NodeType::Sequence);
    for (const auto& interface : config.interfaces) {
        YAML::Node if_node;
        if_node["name"] = interface.name;
        if_node["ip_address"] = interface.ip_address;
        if_node["subnet_mask"] = interface.subnet_mask;
        if_node["mtu"] = interface.mtu;
        if_node["enabled"] = interface.enabled;
        interfaces.push_back(if_node);
    }
    config_node_["router"]["interfaces"] = interfaces;
}

void YAMLConfig::set_protocol_configs(const std::map<std::string, ProtocolConfig>& configs) {
    if (!initialized_) {
        initialize();
    }
    
    for (const auto& [name, config] : configs) {
        config_node_["protocols"][name]["enabled"] = config.enabled;
        
        for (const auto& [key, value] : config.config) {
            config_node_["protocols"][name][key] = value;
        }
        
        if (!config.neighbors.empty()) {
            YAML::Node neighbors = YAML::Node(YAML::NodeType::Sequence);
            for (const auto& [address, neighbor_config] : config.neighbors) {
                YAML::Node neighbor;
                neighbor["address"] = address;
                for (const auto& [key, value] : neighbor_config) {
                    neighbor[key] = value;
                }
                neighbors.push_back(neighbor);
            }
            config_node_["protocols"][name]["neighbors"] = neighbors;
        }
        
        if (!config.interfaces.empty()) {
            YAML::Node interfaces = YAML::Node(YAML::NodeType::Sequence);
            for (const auto& [name, interface_config] : config.interfaces) {
                YAML::Node if_node;
                if_node["name"] = name;
                for (const auto& [key, value] : interface_config) {
                    if_node[key] = value;
                }
                interfaces.push_back(if_node);
            }
            config_node_["protocols"][name]["interfaces"] = interfaces;
        }
    }
}

} // namespace RouterSim
