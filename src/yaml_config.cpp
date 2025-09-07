#include "yaml_config.h"
#include <yaml-cpp/yaml.h>
#include <iostream>
#include <fstream>

namespace RouterSim {

YAMLConfig::YAMLConfig() = default;

YAMLConfig::~YAMLConfig() = default;

bool YAMLConfig::load_scenario(const std::string& filename) {
    try {
        YAML::Node config = YAML::LoadFile(filename);
        
        // Parse scenario metadata
        if (config["name"]) {
            scenario_.name = config["name"].as<std::string>();
        }
        if (config["description"]) {
            scenario_.description = config["description"].as<std::string>();
        }
        
        // Parse interfaces
        if (config["interfaces"]) {
            scenario_.interfaces.clear();
            for (const auto& iface_node : config["interfaces"]) {
                InterfaceConfig iface;
                if (parse_interface(iface_node, iface)) {
                    scenario_.interfaces.push_back(iface);
                }
            }
        }
        
        // Parse routes
        if (config["routes"]) {
            scenario_.routes.clear();
            for (const auto& route_node : config["routes"]) {
                RouteConfig route;
                if (parse_route(route_node, route)) {
                    scenario_.routes.push_back(route);
                }
            }
        }
        
        // Parse BGP configuration
        if (config["bgp"]) {
            parse_bgp(config["bgp"], scenario_.bgp);
        }
        
        // Parse OSPF configuration
        if (config["ospf"]) {
            parse_ospf(config["ospf"], scenario_.ospf);
        }
        
        // Parse IS-IS configuration
        if (config["isis"]) {
            parse_isis(config["isis"], scenario_.isis);
        }
        
        // Parse traffic shaping configuration
        if (config["traffic_shaping"]) {
            parse_traffic_shaping(config["traffic_shaping"], scenario_.traffic_shaping);
        }
        
        // Parse netem configurations
        if (config["netem"]) {
            scenario_.netem_configs.clear();
            for (const auto& netem_node : config["netem"]) {
                NetemConfig netem;
                if (parse_netem(netem_node, netem)) {
                    scenario_.netem_configs.push_back(netem);
                }
            }
        }
        
        // Parse test commands
        if (config["test_commands"]) {
            scenario_.test_commands.clear();
            for (const auto& cmd : config["test_commands"]) {
                scenario_.test_commands.push_back(cmd.as<std::string>());
            }
        }
        
        return true;
    } catch (const YAML::Exception& e) {
        std::cerr << "YAML parsing error: " << e.what() << std::endl;
        return false;
    }
}

bool YAMLConfig::save_scenario(const std::string& filename, const ScenarioConfig& config) {
    try {
        YAML::Node root;
        
        root["name"] = config.name;
        root["description"] = config.description;
        
        // Save interfaces
        YAML::Node interfaces = YAML::Node(YAML::NodeType::Sequence);
        for (const auto& iface : config.interfaces) {
            interfaces.push_back(create_interface_node(iface));
        }
        root["interfaces"] = interfaces;
        
        // Save routes
        YAML::Node routes = YAML::Node(YAML::NodeType::Sequence);
        for (const auto& route : config.routes) {
            routes.push_back(create_route_node(route));
        }
        root["routes"] = routes;
        
        // Save BGP configuration
        root["bgp"] = create_bgp_node(config.bgp);
        
        // Save OSPF configuration
        root["ospf"] = create_ospf_node(config.ospf);
        
        // Save IS-IS configuration
        root["isis"] = create_isis_node(config.isis);
        
        // Save traffic shaping configuration
        root["traffic_shaping"] = create_traffic_shaping_node(config.traffic_shaping);
        
        // Save netem configurations
        YAML::Node netem_configs = YAML::Node(YAML::NodeType::Sequence);
        for (const auto& netem : config.netem_configs) {
            netem_configs.push_back(create_netem_node(netem));
        }
        root["netem"] = netem_configs;
        
        // Save test commands
        root["test_commands"] = config.test_commands;
        
        std::ofstream file(filename);
        file << root;
        
        return true;
    } catch (const YAML::Exception& e) {
        std::cerr << "YAML writing error: " << e.what() << std::endl;
        return false;
    }
}

ScenarioConfig YAMLConfig::get_scenario() const {
    return scenario_;
}

void YAMLConfig::set_scenario(const ScenarioConfig& config) {
    scenario_ = config;
}

bool YAMLConfig::validate_scenario(const ScenarioConfig& config) {
    validation_errors_.clear();
    
    // Validate interfaces
    for (const auto& iface : config.interfaces) {
        if (iface.name.empty()) {
            validation_errors_.push_back("Interface name cannot be empty");
        }
        if (iface.ip_address.empty()) {
            validation_errors_.push_back("Interface IP address cannot be empty");
        }
    }
    
    // Validate routes
    for (const auto& route : config.routes) {
        if (route.network.empty()) {
            validation_errors_.push_back("Route network cannot be empty");
        }
        if (route.next_hop.empty()) {
            validation_errors_.push_back("Route next hop cannot be empty");
        }
    }
    
    // Validate BGP configuration
    if (!config.bgp.as_number.empty() && config.bgp.router_id.empty()) {
        validation_errors_.push_back("BGP router ID is required when AS number is specified");
    }
    
    // Validate OSPF configuration
    if (!config.ospf.router_id.empty() && config.ospf.area.empty()) {
        validation_errors_.push_back("OSPF area is required when router ID is specified");
    }
    
    // Validate IS-IS configuration
    if (!config.isis.system_id.empty() && config.isis.area_id.empty()) {
        validation_errors_.push_back("IS-IS area ID is required when system ID is specified");
    }
    
    return validation_errors_.empty();
}

std::vector<std::string> YAMLConfig::get_validation_errors() const {
    return validation_errors_;
}

bool YAMLConfig::parse_interface(const YAML::Node& node, InterfaceConfig& config) {
    if (!node["name"] || !node["ip_address"] || !node["subnet_mask"]) {
        return false;
    }
    
    config.name = node["name"].as<std::string>();
    config.ip_address = node["ip_address"].as<std::string>();
    config.subnet_mask = node["subnet_mask"].as<std::string>();
    config.is_up = node["is_up"] ? node["is_up"].as<bool>() : false;
    config.mtu = node["mtu"] ? node["mtu"].as<uint32_t>() : 1500;
    
    return true;
}

bool YAMLConfig::parse_route(const YAML::Node& node, RouteConfig& config) {
    if (!node["network"] || !node["next_hop"]) {
        return false;
    }
    
    config.network = node["network"].as<std::string>();
    config.next_hop = node["next_hop"].as<std::string>();
    config.interface = node["interface"] ? node["interface"].as<std::string>() : "";
    config.metric = node["metric"] ? node["metric"].as<uint32_t>() : 1;
    config.protocol = node["protocol"] ? node["protocol"].as<std::string>() : "static";
    config.is_active = node["is_active"] ? node["is_active"].as<bool>() : true;
    
    return true;
}

bool YAMLConfig::parse_bgp(const YAML::Node& node, BGPConfig& config) {
    if (node["as_number"]) {
        config.as_number = node["as_number"].as<std::string>();
    }
    if (node["router_id"]) {
        config.router_id = node["router_id"].as<std::string>();
    }
    if (node["neighbors"]) {
        for (const auto& neighbor : node["neighbors"]) {
            if (neighbor["ip"] && neighbor["as"]) {
                config.neighbors.push_back({neighbor["ip"].as<std::string>(), 
                                          neighbor["as"].as<std::string>()});
            }
        }
    }
    return true;
}

bool YAMLConfig::parse_ospf(const YAML::Node& node, OSPFConfig& config) {
    if (node["router_id"]) {
        config.router_id = node["router_id"].as<std::string>();
    }
    if (node["area"]) {
        config.area = node["area"].as<std::string>();
    }
    if (node["interfaces"]) {
        for (const auto& iface : node["interfaces"]) {
            config.interfaces.push_back(iface.as<std::string>());
        }
    }
    return true;
}

bool YAMLConfig::parse_isis(const YAML::Node& node, ISISConfig& config) {
    if (node["system_id"]) {
        config.system_id = node["system_id"].as<std::string>();
    }
    if (node["area_id"]) {
        config.area_id = node["area_id"].as<std::string>();
    }
    if (node["interfaces"]) {
        for (const auto& iface : node["interfaces"]) {
            if (iface["name"] && iface["level"]) {
                config.interfaces.push_back({iface["name"].as<std::string>(), 
                                           iface["level"].as<uint8_t>()});
            }
        }
    }
    return true;
}

bool YAMLConfig::parse_traffic_shaping(const YAML::Node& node, TrafficShapingConfig& config) {
    if (node["token_bucket"]) {
        auto tb = node["token_bucket"];
        config.token_bucket_capacity = tb["capacity"] ? tb["capacity"].as<uint64_t>() : 1000000;
        config.token_bucket_rate = tb["rate"] ? tb["rate"].as<uint64_t>() : 100000;
    }
    if (node["wfq_classes"]) {
        for (const auto& wfq : node["wfq_classes"]) {
            if (wfq["class_id"] && wfq["weight"]) {
                config.wfq_classes[wfq["class_id"].as<uint32_t>()] = wfq["weight"].as<uint32_t>();
            }
        }
    }
    return true;
}

bool YAMLConfig::parse_netem(const YAML::Node& node, NetemConfig& config) {
    if (!node["interface"]) {
        return false;
    }
    
    config.interface = node["interface"].as<std::string>();
    config.delay_ms = node["delay_ms"] ? node["delay_ms"].as<double>() : 0.0;
    config.jitter_ms = node["jitter_ms"] ? node["jitter_ms"].as<double>() : 0.0;
    config.loss_percent = node["loss_percent"] ? node["loss_percent"].as<double>() : 0.0;
    config.duplicate_percent = node["duplicate_percent"] ? node["duplicate_percent"].as<double>() : 0.0;
    config.reorder_percent = node["reorder_percent"] ? node["reorder_percent"].as<double>() : 0.0;
    config.rate_bps = node["rate_bps"] ? node["rate_bps"].as<uint64_t>() : 0;
    
    return true;
}

YAML::Node YAMLConfig::create_interface_node(const InterfaceConfig& config) {
    YAML::Node node;
    node["name"] = config.name;
    node["ip_address"] = config.ip_address;
    node["subnet_mask"] = config.subnet_mask;
    node["is_up"] = config.is_up;
    node["mtu"] = config.mtu;
    return node;
}

YAML::Node YAMLConfig::create_route_node(const RouteConfig& config) {
    YAML::Node node;
    node["network"] = config.network;
    node["next_hop"] = config.next_hop;
    node["interface"] = config.interface;
    node["metric"] = config.metric;
    node["protocol"] = config.protocol;
    node["is_active"] = config.is_active;
    return node;
}

YAML::Node YAMLConfig::create_bgp_node(const BGPConfig& config) {
    YAML::Node node;
    if (!config.as_number.empty()) {
        node["as_number"] = config.as_number;
    }
    if (!config.router_id.empty()) {
        node["router_id"] = config.router_id;
    }
    if (!config.neighbors.empty()) {
        YAML::Node neighbors = YAML::Node(YAML::NodeType::Sequence);
        for (const auto& neighbor : config.neighbors) {
            YAML::Node neighbor_node;
            neighbor_node["ip"] = neighbor.first;
            neighbor_node["as"] = neighbor.second;
            neighbors.push_back(neighbor_node);
        }
        node["neighbors"] = neighbors;
    }
    return node;
}

YAML::Node YAMLConfig::create_ospf_node(const OSPFConfig& config) {
    YAML::Node node;
    if (!config.router_id.empty()) {
        node["router_id"] = config.router_id;
    }
    if (!config.area.empty()) {
        node["area"] = config.area;
    }
    if (!config.interfaces.empty()) {
        node["interfaces"] = config.interfaces;
    }
    return node;
}

YAML::Node YAMLConfig::create_isis_node(const ISISConfig& config) {
    YAML::Node node;
    if (!config.system_id.empty()) {
        node["system_id"] = config.system_id;
    }
    if (!config.area_id.empty()) {
        node["area_id"] = config.area_id;
    }
    if (!config.interfaces.empty()) {
        YAML::Node interfaces = YAML::Node(YAML::NodeType::Sequence);
        for (const auto& iface : config.interfaces) {
            YAML::Node iface_node;
            iface_node["name"] = iface.first;
            iface_node["level"] = iface.second;
            interfaces.push_back(iface_node);
        }
        node["interfaces"] = interfaces;
    }
    return node;
}

YAML::Node YAMLConfig::create_traffic_shaping_node(const TrafficShapingConfig& config) {
    YAML::Node node;
    YAML::Node token_bucket;
    token_bucket["capacity"] = config.token_bucket_capacity;
    token_bucket["rate"] = config.token_bucket_rate;
    node["token_bucket"] = token_bucket;
    
    if (!config.wfq_classes.empty()) {
        YAML::Node wfq_classes = YAML::Node(YAML::NodeType::Sequence);
        for (const auto& wfq : config.wfq_classes) {
            YAML::Node wfq_node;
            wfq_node["class_id"] = wfq.first;
            wfq_node["weight"] = wfq.second;
            wfq_classes.push_back(wfq_node);
        }
        node["wfq_classes"] = wfq_classes;
    }
    
    return node;
}

YAML::Node YAMLConfig::create_netem_node(const NetemConfig& config) {
    YAML::Node node;
    node["interface"] = config.interface;
    node["delay_ms"] = config.delay_ms;
    node["jitter_ms"] = config.jitter_ms;
    node["loss_percent"] = config.loss_percent;
    node["duplicate_percent"] = config.duplicate_percent;
    node["reorder_percent"] = config.reorder_percent;
    node["rate_bps"] = config.rate_bps;
    return node;
}

} // namespace RouterSim
