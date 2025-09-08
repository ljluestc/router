#include "yaml_config.h"
#include <iostream>
#include <fstream>
#include <yaml-cpp/yaml.h>

namespace RouterSim {

YAMLConfig::YAMLConfig() : loaded_(false) {
}

YAMLConfig::~YAMLConfig() = default;

bool YAMLConfig::load_config(const std::string& filename) {
    try {
        config_ = YAML::LoadFile(filename);
        loaded_ = true;
        return true;
    } catch (const YAML::Exception& e) {
        std::cerr << "Error loading YAML config: " << e.what() << std::endl;
        return false;
    }
}

bool YAMLConfig::save_config(const std::string& filename) const {
    try {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error opening file for writing: " << filename << std::endl;
            return false;
        }
        
        file << config_;
        file.close();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error saving YAML config: " << e.what() << std::endl;
        return false;
    }
}

bool YAMLConfig::is_loaded() const {
    return loaded_;
}

YAML::Node YAMLConfig::get_config() const {
    return config_;
}

// Scenario parser implementation
ScenarioParser::ScenarioParser() : loaded_(false) {
}

ScenarioParser::~ScenarioParser() = default;

bool ScenarioParser::load_scenario(const std::string& filename) {
    try {
        scenario_ = YAML::LoadFile(filename);
        loaded_ = true;
        return true;
    } catch (const YAML::Exception& e) {
        std::cerr << "Error loading scenario: " << e.what() << std::endl;
        return false;
    }
}

bool ScenarioParser::save_scenario(const std::string& filename) const {
    try {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error opening file for writing: " << filename << std::endl;
            return false;
        }
        
        file << scenario_;
        file.close();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error saving scenario: " << e.what() << std::endl;
        return false;
    }
}

bool ScenarioParser::is_loaded() const {
    return loaded_;
}

YAML::Node ScenarioParser::get_scenario() const {
    return scenario_;
}

Scenario ScenarioParser::parse_scenario() const {
    Scenario scenario;
    
    if (!loaded_) {
        return scenario;
    }
    
    try {
        // Parse basic scenario information
        if (scenario_["name"]) {
            scenario.name = scenario_["name"].as<std::string>();
        }
        
        if (scenario_["description"]) {
            scenario.description = scenario_["description"].as<std::string>();
        }
        
        // Parse interfaces
        if (scenario_["interfaces"]) {
            for (const auto& interface_node : scenario_["interfaces"]) {
                Interface interface;
                interface.name = interface_node["name"].as<std::string>();
                interface.ip_address = interface_node["ip_address"].as<std::string>();
                interface.subnet_mask = interface_node["subnet_mask"].as<std::string>();
                interface.is_up = interface_node["is_up"].as<bool>();
                interface.mtu = interface_node["mtu"] ? interface_node["mtu"].as<uint32_t>() : 1500;
                interface.bytes_sent = 0;
                interface.bytes_received = 0;
                interface.packets_sent = 0;
                interface.packets_received = 0;
                
                scenario.interfaces.push_back(interface);
            }
        }
        
        // Parse routes
        if (scenario_["routes"]) {
            for (const auto& route_node : scenario_["routes"]) {
                Route route;
                route.network = route_node["network"].as<std::string>();
                route.next_hop = route_node["next_hop"].as<std::string>();
                route.interface = route_node["interface"].as<std::string>();
                route.metric = route_node["metric"].as<uint32_t>();
                route.protocol = route_node["protocol"].as<std::string>();
                route.is_active = route_node["is_active"] ? route_node["is_active"].as<bool>() : true;
                
                scenario.routes.push_back(route);
            }
        }
        
        // Parse BGP configuration
        if (scenario_["bgp"]) {
            BGPConfig bgp_config;
            bgp_config.as_number = scenario_["bgp"]["as_number"].as<std::string>();
            bgp_config.router_id = scenario_["bgp"]["router_id"].as<std::string>();
            
            if (scenario_["bgp"]["neighbors"]) {
                for (const auto& neighbor_node : scenario_["bgp"]["neighbors"]) {
                    BGPNeighbor neighbor;
                    neighbor.ip = neighbor_node["ip"].as<std::string>();
                    neighbor.as = neighbor_node["as"].as<std::string>();
                    neighbor.description = neighbor_node["description"] ? neighbor_node["description"].as<std::string>() : "";
                    neighbor.hold_time = neighbor_node["hold_time"] ? neighbor_node["hold_time"].as<uint32_t>() : 180;
                    neighbor.keepalive_time = neighbor_node["keepalive_time"] ? neighbor_node["keepalive_time"].as<uint32_t>() : 60;
                    
                    bgp_config.neighbors.push_back(neighbor);
                }
            }
            
            scenario.bgp_config = bgp_config;
        }
        
        // Parse OSPF configuration
        if (scenario_["ospf"]) {
            OSPFConfig ospf_config;
            ospf_config.router_id = scenario_["ospf"]["router_id"].as<std::string>();
            ospf_config.area = scenario_["ospf"]["area"].as<std::string>();
            ospf_config.hello_interval = scenario_["ospf"]["hello_interval"] ? scenario_["ospf"]["hello_interval"].as<uint32_t>() : 10;
            ospf_config.dead_interval = scenario_["ospf"]["dead_interval"] ? scenario_["ospf"]["dead_interval"].as<uint32_t>() : 40;
            
            if (scenario_["ospf"]["interfaces"]) {
                for (const auto& interface_node : scenario_["ospf"]["interfaces"]) {
                    OSPFInterface interface;
                    interface.name = interface_node["name"].as<std::string>();
                    interface.area = interface_node["area"].as<std::string>();
                    interface.cost = interface_node["cost"] ? interface_node["cost"].as<uint32_t>() : 1;
                    interface.priority = interface_node["priority"] ? interface_node["priority"].as<uint8_t>() : 1;
                    
                    ospf_config.interfaces.push_back(interface);
                }
            }
            
            scenario.ospf_config = ospf_config;
        }
        
        // Parse IS-IS configuration
        if (scenario_["isis"]) {
            ISISConfig isis_config;
            isis_config.system_id = scenario_["isis"]["system_id"].as<std::string>();
            isis_config.area_id = scenario_["isis"]["area_id"].as<std::string>();
            isis_config.level = scenario_["isis"]["level"] ? scenario_["isis"]["level"].as<uint8_t>() : 2;
            
            if (scenario_["isis"]["interfaces"]) {
                for (const auto& interface_node : scenario_["isis"]["interfaces"]) {
                    ISISInterface interface;
                    interface.name = interface_node["name"].as<std::string>();
                    interface.level = interface_node["level"].as<uint8_t>();
                    interface.cost = interface_node["cost"] ? interface_node["cost"].as<uint32_t>() : 10;
                    
                    isis_config.interfaces.push_back(interface);
                }
            }
            
            scenario.isis_config = isis_config;
        }
        
        // Parse traffic shaping configuration
        if (scenario_["traffic_shaping"]) {
            TrafficShapingConfig ts_config;
            
            if (scenario_["traffic_shaping"]["token_bucket"]) {
                TokenBucketConfig tb_config;
                tb_config.capacity = scenario_["traffic_shaping"]["token_bucket"]["capacity"].as<uint64_t>();
                tb_config.rate = scenario_["traffic_shaping"]["token_bucket"]["rate"].as<uint64_t>();
                tb_config.burst_size = scenario_["traffic_shaping"]["token_bucket"]["burst_size"] ? 
                                     scenario_["traffic_shaping"]["token_bucket"]["burst_size"].as<uint64_t>() : tb_config.capacity / 2;
                tb_config.allow_burst = scenario_["traffic_shaping"]["token_bucket"]["allow_burst"] ? 
                                      scenario_["traffic_shaping"]["token_bucket"]["allow_burst"].as<bool>() : true;
                
                ts_config.token_bucket = tb_config;
            }
            
            if (scenario_["traffic_shaping"]["wfq_classes"]) {
                for (const auto& class_node : scenario_["traffic_shaping"]["wfq_classes"]) {
                    WFQClass wfq_class;
                    wfq_class.class_id = class_node["class_id"].as<uint8_t>();
                    wfq_class.weight = class_node["weight"].as<uint32_t>();
                    wfq_class.min_bandwidth = class_node["min_bandwidth"] ? class_node["min_bandwidth"].as<uint64_t>() : 0;
                    wfq_class.max_bandwidth = class_node["max_bandwidth"] ? class_node["max_bandwidth"].as<uint64_t>() : 0;
                    wfq_class.name = class_node["name"] ? class_node["name"].as<std::string>() : "";
                    wfq_class.is_active = class_node["is_active"] ? class_node["is_active"].as<bool>() : true;
                    
                    ts_config.wfq_classes.push_back(wfq_class);
                }
            }
            
            scenario.traffic_shaping_config = ts_config;
        }
        
        // Parse network impairments configuration
        if (scenario_["netem"]) {
            for (const auto& netem_node : scenario_["netem"]) {
                NetemConfig netem_config;
                netem_config.interface = netem_node["interface"].as<std::string>();
                netem_config.delay_ms = netem_node["delay_ms"] ? netem_node["delay_ms"].as<double>() : 0.0;
                netem_config.jitter_ms = netem_node["jitter_ms"] ? netem_node["jitter_ms"].as<double>() : 0.0;
                netem_config.loss_percent = netem_node["loss_percent"] ? netem_node["loss_percent"].as<double>() : 0.0;
                netem_config.duplication_percent = netem_node["duplication_percent"] ? netem_node["duplication_percent"].as<double>() : 0.0;
                netem_config.reordering_percent = netem_node["reordering_percent"] ? netem_node["reordering_percent"].as<double>() : 0.0;
                netem_config.corruption_percent = netem_node["corruption_percent"] ? netem_node["corruption_percent"].as<double>() : 0.0;
                netem_config.rate_bps = netem_node["rate_bps"] ? netem_node["rate_bps"].as<uint64_t>() : 0;
                
                scenario.netem_configs.push_back(netem_config);
            }
        }
        
        // Parse test cases
        if (scenario_["test_cases"]) {
            for (const auto& test_node : scenario_["test_cases"]) {
                TestCase test_case;
                test_case.name = test_node["name"].as<std::string>();
                test_case.description = test_node["description"] ? test_node["description"].as<std::string>() : "";
                test_case.expected_result = test_node["expected_result"].as<std::string>();
                
                if (test_node["commands"]) {
                    for (const auto& cmd_node : test_node["commands"]) {
                        test_case.commands.push_back(cmd_node.as<std::string>());
                    }
                }
                
                scenario.test_cases.push_back(test_case);
            }
        }
        
    } catch (const YAML::Exception& e) {
        std::cerr << "Error parsing scenario: " << e.what() << std::endl;
        return Scenario();
    }
    
    return scenario;
}

// Scenario executor implementation
ScenarioExecutor::ScenarioExecutor() : initialized_(false) {
}

ScenarioExecutor::~ScenarioExecutor() = default;

bool ScenarioExecutor::initialize() {
    if (initialized_) {
        return true;
    }
    
    // Initialize components
    router_core_ = std::make_unique<RouterCore>();
    frr_integration_ = std::make_unique<FRRIntegration>();
    traffic_shaper_ = std::make_unique<TrafficShapingManager>();
    netem_impairments_ = std::make_unique<NetemImpairments>();
    
    // Initialize components
    if (!router_core_->initialize()) {
        std::cerr << "Failed to initialize router core" << std::endl;
        return false;
    }
    
    if (!frr_integration_->initialize({})) {
        std::cerr << "Failed to initialize FRR integration" << std::endl;
        return false;
    }
    
    if (!traffic_shaper_->initialize()) {
        std::cerr << "Failed to initialize traffic shaper" << std::endl;
        return false;
    }
    
    if (!netem_impairments_->initialize()) {
        std::cerr << "Failed to initialize network impairments" << std::endl;
        return false;
    }
    
    initialized_ = true;
    return true;
}

bool ScenarioExecutor::execute_scenario(const Scenario& scenario) {
    if (!initialized_) {
        if (!initialize()) {
            return false;
        }
    }
    
    std::cout << "Executing scenario: " << scenario.name << std::endl;
    std::cout << "Description: " << scenario.description << std::endl;
    
    // Start components
    router_core_->start();
    frr_integration_->start();
    traffic_shaper_->start();
    netem_impairments_->start();
    
    // Configure interfaces
    for (const auto& interface : scenario.interfaces) {
        if (router_core_->add_interface(interface.name, interface.ip_address, interface.subnet_mask)) {
            router_core_->set_interface_up(interface.name, interface.is_up);
            std::cout << "Configured interface: " << interface.name << std::endl;
        }
    }
    
    // Configure routes
    for (const auto& route : scenario.routes) {
        if (router_core_->add_route(route)) {
            std::cout << "Added route: " << route.network << " -> " << route.next_hop << std::endl;
        }
    }
    
    // Configure BGP
    if (!scenario.bgp_config.as_number.empty()) {
        std::map<std::string, std::string> bgp_config;
        bgp_config["as_number"] = scenario.bgp_config.as_number;
        bgp_config["router_id"] = scenario.bgp_config.router_id;
        
        if (frr_integration_->start_protocol("bgp", bgp_config)) {
            std::cout << "Started BGP with AS " << scenario.bgp_config.as_number << std::endl;
            
            // Add BGP neighbors
            for (const auto& neighbor : scenario.bgp_config.neighbors) {
                std::map<std::string, std::string> neighbor_config;
                neighbor_config["remote_as"] = neighbor.as;
                neighbor_config["description"] = neighbor.description;
                neighbor_config["hold_time"] = std::to_string(neighbor.hold_time);
                neighbor_config["keepalive_time"] = std::to_string(neighbor.keepalive_time);
                
                if (frr_integration_->add_bgp_neighbor(neighbor.ip, neighbor_config)) {
                    std::cout << "Added BGP neighbor: " << neighbor.ip << " (AS " << neighbor.as << ")" << std::endl;
                }
            }
        }
    }
    
    // Configure OSPF
    if (!scenario.ospf_config.router_id.empty()) {
        std::map<std::string, std::string> ospf_config;
        ospf_config["router_id"] = scenario.ospf_config.router_id;
        ospf_config["area"] = scenario.ospf_config.area;
        ospf_config["hello_interval"] = std::to_string(scenario.ospf_config.hello_interval);
        ospf_config["dead_interval"] = std::to_string(scenario.ospf_config.dead_interval);
        
        if (frr_integration_->start_protocol("ospf", ospf_config)) {
            std::cout << "Started OSPF with router-id " << scenario.ospf_config.router_id << std::endl;
            
            // Configure OSPF interfaces
            for (const auto& interface : scenario.ospf_config.interfaces) {
                std::map<std::string, std::string> interface_config;
                interface_config["area"] = interface.area;
                interface_config["cost"] = std::to_string(interface.cost);
                interface_config["priority"] = std::to_string(interface.priority);
                
                if (frr_integration_->add_ospf_interface(interface.name, interface_config)) {
                    std::cout << "Added OSPF interface: " << interface.name << " (area " << interface.area << ")" << std::endl;
                }
            }
        }
    }
    
    // Configure IS-IS
    if (!scenario.isis_config.system_id.empty()) {
        std::map<std::string, std::string> isis_config;
        isis_config["system_id"] = scenario.isis_config.system_id;
        isis_config["area_id"] = scenario.isis_config.area_id;
        isis_config["level"] = std::to_string(scenario.isis_config.level);
        
        if (frr_integration_->start_protocol("isis", isis_config)) {
            std::cout << "Started IS-IS with system-id " << scenario.isis_config.system_id << std::endl;
            
            // Configure IS-IS interfaces
            for (const auto& interface : scenario.isis_config.interfaces) {
                std::map<std::string, std::string> interface_config;
                interface_config["level"] = std::to_string(interface.level);
                interface_config["cost"] = std::to_string(interface.cost);
                
                if (frr_integration_->add_isis_interface(interface.name, interface_config)) {
                    std::cout << "Added IS-IS interface: " << interface.name << " (level " << (int)interface.level << ")" << std::endl;
                }
            }
        }
    }
    
    // Configure traffic shaping
    if (!scenario.traffic_shaping_config.wfq_classes.empty()) {
        if (traffic_shaper_->configure_interface("default", ShapingAlgorithm::WEIGHTED_FAIR_QUEUE, {})) {
            std::cout << "Configured traffic shaping" << std::endl;
        }
    }
    
    // Configure network impairments
    for (const auto& netem_config : scenario.netem_configs) {
        CombinedImpairments impairments;
        impairments.delay.delay_ms = netem_config.delay_ms;
        impairments.delay.jitter_ms = netem_config.jitter_ms;
        impairments.loss.loss_percent = netem_config.loss_percent;
        impairments.duplication.duplication_percent = netem_config.duplication_percent;
        impairments.reordering.reordering_percent = netem_config.reordering_percent;
        impairments.corruption.corruption_percent = netem_config.corruption_percent;
        impairments.rate.rate_bps = netem_config.rate_bps;
        
        if (netem_impairments_->apply_combined_impairments(netem_config.interface, impairments)) {
            std::cout << "Applied network impairments to " << netem_config.interface << std::endl;
        }
    }
    
    // Execute test cases
    for (const auto& test_case : scenario.test_cases) {
        std::cout << "\nExecuting test case: " << test_case.name << std::endl;
        std::cout << "Description: " << test_case.description << std::endl;
        
        for (const auto& command : test_case.commands) {
            std::cout << "Executing command: " << command << std::endl;
            // Execute command (simplified implementation)
        }
        
        std::cout << "Expected result: " << test_case.expected_result << std::endl;
    }
    
    std::cout << "\nScenario execution completed" << std::endl;
    return true;
}

bool ScenarioExecutor::is_initialized() const {
    return initialized_;
}

} // namespace RouterSim
