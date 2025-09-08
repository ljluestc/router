#include "yaml_config.h"
#include <fstream>
#include <iostream>
#include <yaml-cpp/yaml.h>

namespace RouterSim {

YAMLConfig::YAMLConfig() {
}

YAMLConfig::~YAMLConfig() {
}

bool YAMLConfig::load_scenario(const std::string& filename) {
    try {
        YAML::Node config = YAML::LoadFile(filename);
        
        if (!config.IsMap()) {
            std::cerr << "Invalid YAML file format" << std::endl;
            return false;
        }
        
        // Load router configuration
        if (config["router"]) {
            load_router_config(config["router"]);
        }
        
        // Load protocols
        if (config["protocols"]) {
            load_protocols_config(config["protocols"]);
        }
        
        // Load traffic shaping
        if (config["traffic_shaping"]) {
            load_traffic_shaping_config(config["traffic_shaping"]);
        }
        
        // Load network impairments
        if (config["impairments"]) {
            load_impairments_config(config["impairments"]);
        }
        
        // Load cloud networking
        if (config["cloud_networking"]) {
            load_cloud_networking_config(config["cloud_networking"]);
        }
        
        // Load test scenarios
        if (config["test_scenarios"]) {
            load_test_scenarios_config(config["test_scenarios"]);
        }
        
        std::cout << "✅ Scenario loaded successfully: " << filename << std::endl;
        return true;
        
    } catch (const YAML::Exception& e) {
        std::cerr << "YAML parsing error: " << e.what() << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Error loading scenario: " << e.what() << std::endl;
        return false;
    }
}

bool YAMLConfig::save_scenario(const std::string& filename) {
    try {
        YAML::Node config;
        
        // Save router configuration
        config["router"] = save_router_config();
        
        // Save protocols
        config["protocols"] = save_protocols_config();
        
        // Save traffic shaping
        config["traffic_shaping"] = save_traffic_shaping_config();
        
        // Save network impairments
        config["impairments"] = save_impairments_config();
        
        // Save cloud networking
        config["cloud_networking"] = save_cloud_networking_config();
        
        // Save test scenarios
        config["test_scenarios"] = save_test_scenarios_config();
        
        std::ofstream file(filename);
        file << config;
        file.close();
        
        std::cout << "✅ Scenario saved successfully: " << filename << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error saving scenario: " << e.what() << std::endl;
        return false;
    }
}

void YAMLConfig::load_router_config(const YAML::Node& node) {
    if (node["id"]) {
        router_config_.id = node["id"].as<std::string>();
    }
    
    if (node["name"]) {
        router_config_.name = node["name"].as<std::string>();
    }
    
    if (node["interfaces"]) {
        for (const auto& interface : node["interfaces"]) {
            InterfaceConfig if_config;
            
            if (interface["name"]) {
                if_config.name = interface["name"].as<std::string>();
            }
            
            if (interface["ip"]) {
                if_config.ip_address = interface["ip"].as<std::string>();
            }
            
            if (interface["mask"]) {
                if_config.subnet_mask = interface["mask"].as<std::string>();
            }
            
            if (interface["type"]) {
                std::string type = interface["type"].as<std::string>();
                if (type == "ethernet") {
                    if_config.type = InterfaceType::ETHERNET;
                } else if (type == "loopback") {
                    if_config.type = InterfaceType::LOOPBACK;
                } else if (type == "tunnel") {
                    if_config.type = InterfaceType::TUNNEL;
                }
            }
            
            router_config_.interfaces.push_back(if_config);
        }
    }
    
    std::cout << "Router configuration loaded: " << router_config_.name << std::endl;
}

void YAMLConfig::load_protocols_config(const YAML::Node& node) {
    // Load BGP configuration
    if (node["bgp"]) {
        const auto& bgp = node["bgp"];
        
        if (bgp["enabled"] && bgp["enabled"].as<bool>()) {
            bgp_config_.local_as = bgp["local_as"].as<std::string>();
            bgp_config_.router_id = bgp["router_id"].as<std::string>();
            
            if (bgp["neighbors"]) {
                for (const auto& neighbor : bgp["neighbors"]) {
                    std::string neighbor_ip = neighbor["ip"].as<std::string>();
                    std::string remote_as = neighbor["as"].as<std::string>();
                    
                    bgp_config_.neighbors.push_back(neighbor_ip);
                    bgp_config_.neighbor_configs[neighbor_ip] = remote_as;
                }
            }
            
            if (bgp["graceful_restart"]) {
                bgp_config_.enable_graceful_restart = bgp["graceful_restart"].as<bool>();
            }
            
            if (bgp["hold_time"]) {
                bgp_config_.hold_time = bgp["hold_time"].as<uint32_t>();
            }
            
            if (bgp["keepalive_interval"]) {
                bgp_config_.keepalive_interval = bgp["keepalive_interval"].as<uint32_t>();
            }
        }
    }
    
    // Load OSPF configuration
    if (node["ospf"]) {
        const auto& ospf = node["ospf"];
        
        if (ospf["enabled"] && ospf["enabled"].as<bool>()) {
            ospf_config_.router_id = ospf["router_id"].as<std::string>();
            
            if (ospf["areas"]) {
                for (const auto& area : ospf["areas"]) {
                    ospf_config_.areas.push_back(area.as<std::string>());
                }
            }
            
            if (ospf["hello_interval"]) {
                ospf_config_.hello_interval = ospf["hello_interval"].as<uint32_t>();
            }
            
            if (ospf["dead_interval"]) {
                ospf_config_.dead_interval = ospf["dead_interval"].as<uint32_t>();
            }
        }
    }
    
    // Load ISIS configuration
    if (node["isis"]) {
        const auto& isis = node["isis"];
        
        if (isis["enabled"] && isis["enabled"].as<bool>()) {
            isis_config_.system_id = isis["system_id"].as<std::string>();
            isis_config_.area_id = isis["area_id"].as<std::string>();
            
            if (isis["interfaces"]) {
                for (const auto& interface : isis["interfaces"]) {
                    isis_config_.interfaces.push_back(interface.as<std::string>());
                }
            }
            
            if (isis["level"]) {
                isis_config_.level = isis["level"].as<uint8_t>();
            }
            
            if (isis["hello_interval"]) {
                isis_config_.hello_interval = isis["hello_interval"].as<uint32_t>();
            }
            
            if (isis["hold_time"]) {
                isis_config_.hold_time = isis["hold_time"].as<uint32_t>();
            }
        }
    }
    
    std::cout << "Protocols configuration loaded" << std::endl;
}

void YAMLConfig::load_traffic_shaping_config(const YAML::Node& node) {
    if (node["token_bucket"]) {
        const auto& tb = node["token_bucket"];
        
        if (tb["rate_bps"]) {
            traffic_config_.token_bucket_rate = tb["rate_bps"].as<uint64_t>();
        }
        
        if (tb["burst_bytes"]) {
            traffic_config_.token_bucket_burst = tb["burst_bytes"].as<uint64_t>();
        }
    }
    
    if (node["wfq"]) {
        const auto& wfq = node["wfq"];
        
        if (wfq["total_bandwidth"]) {
            traffic_config_.wfq_total_bandwidth = wfq["total_bandwidth"].as<uint64_t>();
        }
        
        if (wfq["queues"]) {
            for (const auto& queue : wfq["queues"]) {
                WFQQueueConfig q_config;
                
                if (queue["name"]) {
                    q_config.name = queue["name"].as<std::string>();
                }
                
                if (queue["weight"]) {
                    q_config.weight = queue["weight"].as<uint32_t>();
                }
                
                if (queue["max_size"]) {
                    q_config.max_size = queue["max_size"].as<uint64_t>();
                }
                
                traffic_config_.wfq_queues.push_back(q_config);
            }
        }
    }
    
    std::cout << "Traffic shaping configuration loaded" << std::endl;
}

void YAMLConfig::load_impairments_config(const YAML::Node& node) {
    if (node["delay"]) {
        const auto& delay = node["delay"];
        
        if (delay["enabled"] && delay["enabled"].as<bool>()) {
            uint32_t delay_ms = delay["ms"].as<uint32_t>();
            uint32_t jitter_ms = 0;
            
            if (delay["jitter"]) {
                jitter_ms = delay["jitter"].as<uint32_t>();
            }
            
            impairments_config_.delay_ms = delay_ms;
            impairments_config_.jitter_ms = jitter_ms;
        }
    }
    
    if (node["loss"]) {
        const auto& loss = node["loss"];
        
        if (loss["enabled"] && loss["enabled"].as<bool>()) {
            impairments_config_.loss_percent = loss["percent"].as<double>();
        }
    }
    
    if (node["duplicate"]) {
        const auto& duplicate = node["duplicate"];
        
        if (duplicate["enabled"] && duplicate["enabled"].as<bool>()) {
            impairments_config_.duplicate_percent = duplicate["percent"].as<double>();
        }
    }
    
    if (node["reorder"]) {
        const auto& reorder = node["reorder"];
        
        if (reorder["enabled"] && reorder["enabled"].as<bool>()) {
            impairments_config_.reorder_percent = reorder["percent"].as<double>();
        }
    }
    
    if (node["corruption"]) {
        const auto& corruption = node["corruption"];
        
        if (corruption["enabled"] && corruption["enabled"].as<bool>()) {
            impairments_config_.corruption_percent = corruption["percent"].as<double>();
        }
    }
    
    if (node["rate_limit"]) {
        const auto& rate_limit = node["rate_limit"];
        
        if (rate_limit["enabled"] && rate_limit["enabled"].as<bool>()) {
            impairments_config_.rate_limit_bps = rate_limit["bps"].as<uint64_t>();
        }
    }
    
    std::cout << "Network impairments configuration loaded" << std::endl;
}

void YAMLConfig::load_cloud_networking_config(const YAML::Node& node) {
    if (node["vpc"]) {
        const auto& vpc = node["vpc"];
        
        if (vpc["enabled"] && vpc["enabled"].as<bool>()) {
            cloud_config_.vpc_enabled = true;
            cloud_config_.vpc_name = vpc["name"].as<std::string>();
            cloud_config_.vpc_cidr = vpc["cidr"].as<std::string>();
            cloud_config_.vpc_region = vpc["region"].as<std::string>();
        }
    }
    
    if (node["subnets"]) {
        for (const auto& subnet : node["subnets"]) {
            SubnetConfig s_config;
            
            if (subnet["name"]) {
                s_config.name = subnet["name"].as<std::string>();
            }
            
            if (subnet["cidr"]) {
                s_config.cidr = subnet["cidr"].as<std::string>();
            }
            
            if (subnet["az"]) {
                s_config.availability_zone = subnet["az"].as<std::string>();
            }
            
            cloud_config_.subnets.push_back(s_config);
        }
    }
    
    if (node["load_balancers"]) {
        for (const auto& lb : node["load_balancers"]) {
            LoadBalancerConfig lb_config;
            
            if (lb["name"]) {
                lb_config.name = lb["name"].as<std::string>();
            }
            
            if (lb["type"]) {
                lb_config.type = lb["type"].as<std::string>();
            }
            
            if (lb["subnet"]) {
                lb_config.subnet = lb["subnet"].as<std::string>();
            }
            
            cloud_config_.load_balancers.push_back(lb_config);
        }
    }
    
    if (node["nat_gateways"]) {
        for (const auto& nat : node["nat_gateways"]) {
            NATGatewayConfig nat_config;
            
            if (nat["name"]) {
                nat_config.name = nat["name"].as<std::string>();
            }
            
            if (nat["subnet"]) {
                nat_config.subnet = nat["subnet"].as<std::string>();
            }
            
            cloud_config_.nat_gateways.push_back(nat_config);
        }
    }
    
    std::cout << "Cloud networking configuration loaded" << std::endl;
}

void YAMLConfig::load_test_scenarios_config(const YAML::Node& node) {
    if (node["scenarios"]) {
        for (const auto& scenario : node["scenarios"]) {
            TestScenarioConfig t_config;
            
            if (scenario["name"]) {
                t_config.name = scenario["name"].as<std::string>();
            }
            
            if (scenario["description"]) {
                t_config.description = scenario["description"].as<std::string>();
            }
            
            if (scenario["duration"]) {
                t_config.duration_seconds = scenario["duration"].as<uint32_t>();
            }
            
            if (scenario["packets_per_second"]) {
                t_config.packets_per_second = scenario["packets_per_second"].as<uint32_t>();
            }
            
            if (scenario["packet_size"]) {
                t_config.packet_size = scenario["packet_size"].as<uint32_t>();
            }
            
            test_config_.scenarios.push_back(t_config);
        }
    }
    
    std::cout << "Test scenarios configuration loaded" << std::endl;
}

YAML::Node YAMLConfig::save_router_config() {
    YAML::Node node;
    
    node["id"] = router_config_.id;
    node["name"] = router_config_.name;
    
    YAML::Node interfaces;
    for (const auto& if_config : router_config_.interfaces) {
        YAML::Node interface;
        interface["name"] = if_config.name;
        interface["ip"] = if_config.ip_address;
        interface["mask"] = if_config.subnet_mask;
        
        std::string type_str;
        switch (if_config.type) {
            case InterfaceType::ETHERNET: type_str = "ethernet"; break;
            case InterfaceType::LOOPBACK: type_str = "loopback"; break;
            case InterfaceType::TUNNEL: type_str = "tunnel"; break;
        }
        interface["type"] = type_str;
        
        interfaces.push_back(interface);
    }
    node["interfaces"] = interfaces;
    
    return node;
}

YAML::Node YAMLConfig::save_protocols_config() {
    YAML::Node node;
    
    // BGP
    YAML::Node bgp;
    bgp["enabled"] = !bgp_config_.local_as.empty();
    bgp["local_as"] = bgp_config_.local_as;
    bgp["router_id"] = bgp_config_.router_id;
    bgp["graceful_restart"] = bgp_config_.enable_graceful_restart;
    bgp["hold_time"] = bgp_config_.hold_time;
    bgp["keepalive_interval"] = bgp_config_.keepalive_interval;
    
    YAML::Node neighbors;
    for (size_t i = 0; i < bgp_config_.neighbors.size(); ++i) {
        YAML::Node neighbor;
        neighbor["ip"] = bgp_config_.neighbors[i];
        neighbor["as"] = bgp_config_.neighbor_configs[bgp_config_.neighbors[i]];
        neighbors.push_back(neighbor);
    }
    bgp["neighbors"] = neighbors;
    node["bgp"] = bgp;
    
    // OSPF
    YAML::Node ospf;
    ospf["enabled"] = !ospf_config_.router_id.empty();
    ospf["router_id"] = ospf_config_.router_id;
    ospf["areas"] = ospf_config_.areas;
    ospf["hello_interval"] = ospf_config_.hello_interval;
    ospf["dead_interval"] = ospf_config_.dead_interval;
    node["ospf"] = ospf;
    
    // ISIS
    YAML::Node isis;
    isis["enabled"] = !isis_config_.system_id.empty();
    isis["system_id"] = isis_config_.system_id;
    isis["area_id"] = isis_config_.area_id;
    isis["interfaces"] = isis_config_.interfaces;
    isis["level"] = isis_config_.level;
    isis["hello_interval"] = isis_config_.hello_interval;
    isis["hold_time"] = isis_config_.hold_time;
    node["isis"] = isis;
    
    return node;
}

YAML::Node YAMLConfig::save_traffic_shaping_config() {
    YAML::Node node;
    
    // Token Bucket
    YAML::Node token_bucket;
    token_bucket["rate_bps"] = traffic_config_.token_bucket_rate;
    token_bucket["burst_bytes"] = traffic_config_.token_bucket_burst;
    node["token_bucket"] = token_bucket;
    
    // WFQ
    YAML::Node wfq;
    wfq["total_bandwidth"] = traffic_config_.wfq_total_bandwidth;
    
    YAML::Node queues;
    for (const auto& q_config : traffic_config_.wfq_queues) {
        YAML::Node queue;
        queue["name"] = q_config.name;
        queue["weight"] = q_config.weight;
        queue["max_size"] = q_config.max_size;
        queues.push_back(queue);
    }
    wfq["queues"] = queues;
    node["wfq"] = wfq;
    
    return node;
}

YAML::Node YAMLConfig::save_impairments_config() {
    YAML::Node node;
    
    // Delay
    YAML::Node delay;
    delay["enabled"] = impairments_config_.delay_ms > 0;
    delay["ms"] = impairments_config_.delay_ms;
    delay["jitter"] = impairments_config_.jitter_ms;
    node["delay"] = delay;
    
    // Loss
    YAML::Node loss;
    loss["enabled"] = impairments_config_.loss_percent > 0.0;
    loss["percent"] = impairments_config_.loss_percent;
    node["loss"] = loss;
    
    // Duplicate
    YAML::Node duplicate;
    duplicate["enabled"] = impairments_config_.duplicate_percent > 0.0;
    duplicate["percent"] = impairments_config_.duplicate_percent;
    node["duplicate"] = duplicate;
    
    // Reorder
    YAML::Node reorder;
    reorder["enabled"] = impairments_config_.reorder_percent > 0.0;
    reorder["percent"] = impairments_config_.reorder_percent;
    node["reorder"] = reorder;
    
    // Corruption
    YAML::Node corruption;
    corruption["enabled"] = impairments_config_.corruption_percent > 0.0;
    corruption["percent"] = impairments_config_.corruption_percent;
    node["corruption"] = corruption;
    
    // Rate Limit
    YAML::Node rate_limit;
    rate_limit["enabled"] = impairments_config_.rate_limit_bps > 0;
    rate_limit["bps"] = impairments_config_.rate_limit_bps;
    node["rate_limit"] = rate_limit;
    
    return node;
}

YAML::Node YAMLConfig::save_cloud_networking_config() {
    YAML::Node node;
    
    // VPC
    YAML::Node vpc;
    vpc["enabled"] = cloud_config_.vpc_enabled;
    vpc["name"] = cloud_config_.vpc_name;
    vpc["cidr"] = cloud_config_.vpc_cidr;
    vpc["region"] = cloud_config_.vpc_region;
    node["vpc"] = vpc;
    
    // Subnets
    YAML::Node subnets;
    for (const auto& s_config : cloud_config_.subnets) {
        YAML::Node subnet;
        subnet["name"] = s_config.name;
        subnet["cidr"] = s_config.cidr;
        subnet["az"] = s_config.availability_zone;
        subnets.push_back(subnet);
    }
    node["subnets"] = subnets;
    
    // Load Balancers
    YAML::Node load_balancers;
    for (const auto& lb_config : cloud_config_.load_balancers) {
        YAML::Node lb;
        lb["name"] = lb_config.name;
        lb["type"] = lb_config.type;
        lb["subnet"] = lb_config.subnet;
        load_balancers.push_back(lb);
    }
    node["load_balancers"] = load_balancers;
    
    // NAT Gateways
    YAML::Node nat_gateways;
    for (const auto& nat_config : cloud_config_.nat_gateways) {
        YAML::Node nat;
        nat["name"] = nat_config.name;
        nat["subnet"] = nat_config.subnet;
        nat_gateways.push_back(nat);
    }
    node["nat_gateways"] = nat_gateways;
    
    return node;
}

YAML::Node YAMLConfig::save_test_scenarios_config() {
    YAML::Node node;
    
    YAML::Node scenarios;
    for (const auto& t_config : test_config_.scenarios) {
        YAML::Node scenario;
        scenario["name"] = t_config.name;
        scenario["description"] = t_config.description;
        scenario["duration"] = t_config.duration_seconds;
        scenario["packets_per_second"] = t_config.packets_per_second;
        scenario["packet_size"] = t_config.packet_size;
        scenarios.push_back(scenario);
    }
    node["scenarios"] = scenarios;
    
    return node;
}

} // namespace RouterSim
