#include "router_sim.h"
#include "frr_integration.h"
#include "traffic_shaping.h"
#include "netem_impairments.h"
#include "cli_interface.h"
#include "yaml_config.h"
#include "packet_processor.h"
#include "routing_table.h"
#include "statistics.h"
#include <iostream>
#include <fstream>
#include <yaml-cpp/yaml.h>
#include <thread>
#include <chrono>

namespace RouterSim {

RouterSimulator::RouterSimulator() 
    : running_(false) {
    // Initialize components
    frr_integration_ = std::make_unique<FRRIntegration>();
    traffic_shaper_ = std::make_unique<TrafficShaper>();
    netem_impairments_ = std::make_unique<NetemImpairments>();
    cli_interface_ = std::make_unique<CLIInterface>();
    yaml_config_ = std::make_unique<YamlConfig>();
    packet_processor_ = std::make_unique<PacketProcessor>();
    routing_table_ = std::make_unique<RoutingTable>();
    statistics_ = std::make_unique<Statistics>();
}

RouterSimulator::~RouterSimulator() {
    stop();
}

bool RouterSimulator::initialize(const RouterConfig& config) {
    std::cout << "Initializing Router Simulator..." << std::endl;
    
    config_ = config;
    
    // Initialize FRR integration
    if (config.enable_bgp || config.enable_ospf || config.enable_isis) {
        if (!frr_integration_->initialize(config)) {
            std::cerr << "Failed to initialize FRR integration" << std::endl;
            return false;
        }
    }
    
    // Initialize traffic shaper
    if (!traffic_shaper_->initialize()) {
        std::cerr << "Failed to initialize traffic shaper" << std::endl;
        return false;
    }
    
    // Initialize netem impairments
    if (!netem_impairments_->initialize()) {
        std::cerr << "Failed to initialize netem impairments" << std::endl;
        return false;
    }
    
    // Initialize packet processor
    if (!packet_processor_->initialize()) {
        std::cerr << "Failed to initialize packet processor" << std::endl;
        return false;
    }
    
    // Initialize routing table
    if (!routing_table_->initialize()) {
        std::cerr << "Failed to initialize routing table" << std::endl;
        return false;
    }
    
    // Initialize statistics
    if (!statistics_->initialize()) {
        std::cerr << "Failed to initialize statistics" << std::endl;
        return false;
    }
    
    // Initialize CLI interface
    if (!cli_interface_->initialize()) {
        std::cerr << "Failed to initialize CLI interface" << std::endl;
        return false;
    }
    
    // Initialize YAML config parser
    if (!yaml_config_->initialize()) {
        std::cerr << "Failed to initialize YAML config parser" << std::endl;
        return false;
    }
    
    std::cout << "Router Simulator initialized successfully" << std::endl;
    return true;
}

bool RouterSimulator::load_config_from_file(const std::string& config_file) {
    try {
        YAML::Node config = YAML::LoadFile(config_file);
        
        // Parse router configuration
        if (config["router"]) {
            auto router_node = config["router"];
            config_.router_id = router_node["id"].as<std::string>();
            config_.hostname = router_node["hostname"].as<std::string>();
            config_.as_number = router_node["as_number"].as<uint32_t>();
            config_.area_id = router_node["area_id"].as<std::string>();
            config_.system_id = router_node["system_id"].as<std::string>();
        }
        
        // Parse protocol configuration
        if (config["protocols"]) {
            auto protocols_node = config["protocols"];
            config_.enable_bgp = protocols_node["bgp"]["enabled"].as<bool>(false);
            config_.enable_ospf = protocols_node["ospf"]["enabled"].as<bool>(false);
            config_.enable_isis = protocols_node["isis"]["enabled"].as<bool>(false);
        }
        
        // Parse interfaces
        if (config["interfaces"]) {
            for (const auto& interface_node : config["interfaces"]) {
                InterfaceConfig iface_config;
                iface_config.name = interface_node["name"].as<std::string>();
                iface_config.ip_address = interface_node["ip_address"].as<std::string>();
                iface_config.subnet_mask = interface_node["subnet_mask"].as<std::string>();
                iface_config.bandwidth_mbps = interface_node["bandwidth_mbps"].as<uint32_t>(1000);
                iface_config.is_up = interface_node["is_up"].as<bool>(true);
                iface_config.description = interface_node["description"].as<std::string>("");
                
                interfaces_[iface_config.name] = iface_config;
            }
        }
        
        std::cout << "Configuration loaded from " << config_file << std::endl;
        return true;
        
    } catch (const YAML::Exception& e) {
        std::cerr << "Error parsing configuration file: " << e.what() << std::endl;
        return false;
    }
}

bool RouterSimulator::save_config_to_file(const std::string& config_file) {
    try {
        YAML::Node config;
        
        // Router configuration
        config["router"]["id"] = config_.router_id;
        config["router"]["hostname"] = config_.hostname;
        config["router"]["as_number"] = config_.as_number;
        config["router"]["area_id"] = config_.area_id;
        config["router"]["system_id"] = config_.system_id;
        
        // Protocol configuration
        config["protocols"]["bgp"]["enabled"] = config_.enable_bgp;
        config["protocols"]["ospf"]["enabled"] = config_.enable_ospf;
        config["protocols"]["isis"]["enabled"] = config_.enable_isis;
        
        // Interface configuration
        for (const auto& pair : interfaces_) {
            const auto& iface = pair.second;
            YAML::Node iface_node;
            iface_node["name"] = iface.name;
            iface_node["ip_address"] = iface.ip_address;
            iface_node["subnet_mask"] = iface.subnet_mask;
            iface_node["bandwidth_mbps"] = iface.bandwidth_mbps;
            iface_node["is_up"] = iface.is_up;
            iface_node["description"] = iface.description;
            
            config["interfaces"].push_back(iface_node);
        }
        
        std::ofstream file(config_file);
        file << config;
        file.close();
        
        std::cout << "Configuration saved to " << config_file << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error saving configuration file: " << e.what() << std::endl;
        return false;
    }
}

bool RouterSimulator::add_interface(const InterfaceConfig& config) {
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    interfaces_[config.name] = config;
    
    // Configure traffic shaping for the interface
    ShapingConfig shaping_config;
    shaping_config.rate_bps = config.bandwidth_mbps * 1000000; // Convert to bps
    shaping_config.burst_size = shaping_config.rate_bps / 10; // 100ms burst
    shaping_config.queue_size = 1000;
    
    traffic_shaper_->configure_interface(config.name, shaping_config);
    
    std::cout << "Added interface: " << config.name << std::endl;
    return true;
}

bool RouterSimulator::remove_interface(const std::string& name) {
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    auto it = interfaces_.find(name);
    if (it == interfaces_.end()) {
        return false;
    }
    
    // Clear traffic shaping for the interface
    traffic_shaper_->clear_interface(name);
    
    // Clear impairments for the interface
    netem_impairments_->clear_impairments(name);
    
    interfaces_.erase(it);
    
    std::cout << "Removed interface: " << name << std::endl;
    return true;
}

bool RouterSimulator::update_interface(const InterfaceConfig& config) {
    return add_interface(config); // Same as add for now
}

std::vector<InterfaceConfig> RouterSimulator::get_interfaces() const {
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    std::vector<InterfaceConfig> result;
    for (const auto& pair : interfaces_) {
        result.push_back(pair.second);
    }
    return result;
}

bool RouterSimulator::start_protocols() {
    std::cout << "Starting routing protocols..." << std::endl;
    
    if (config_.enable_bgp) {
        if (!frr_integration_->enable_bgp(config_.protocol_configs)) {
            std::cerr << "Failed to enable BGP" << std::endl;
            return false;
        }
        std::cout << "BGP enabled" << std::endl;
    }
    
    if (config_.enable_ospf) {
        if (!frr_integration_->enable_ospf(config_.protocol_configs)) {
            std::cerr << "Failed to enable OSPF" << std::endl;
            return false;
        }
        std::cout << "OSPF enabled" << std::endl;
    }
    
    if (config_.enable_isis) {
        if (!frr_integration_->enable_isis(config_.protocol_configs)) {
            std::cerr << "Failed to enable ISIS" << std::endl;
            return false;
        }
        std::cout << "ISIS enabled" << std::endl;
    }
    
    return true;
}

bool RouterSimulator::stop_protocols() {
    std::cout << "Stopping routing protocols..." << std::endl;
    
    if (config_.enable_bgp) {
        frr_integration_->disable_bgp();
    }
    
    if (config_.enable_ospf) {
        frr_integration_->disable_ospf();
    }
    
    if (config_.enable_isis) {
        frr_integration_->disable_isis();
    }
    
    return true;
}

bool RouterSimulator::is_protocol_running(const std::string& protocol) const {
    if (protocol == "bgp") return config_.enable_bgp;
    if (protocol == "ospf") return config_.enable_ospf;
    if (protocol == "isis") return config_.enable_isis;
    return false;
}

bool RouterSimulator::configure_traffic_shaping(const std::string& interface, const ShapingConfig& config) {
    return traffic_shaper_->configure_interface(interface, config);
}

bool RouterSimulator::enable_wfq(const std::string& interface, uint32_t num_queues) {
    return traffic_shaper_->enable_wfq(interface, num_queues);
}

bool RouterSimulator::configure_impairments(const std::string& interface, const ImpairmentConfig& config) {
    return netem_impairments_->configure_impairments(interface, config);
}

bool RouterSimulator::clear_impairments(const std::string& interface) {
    return netem_impairments_->clear_impairments(interface);
}

bool RouterSimulator::send_packet(const Packet& packet) {
    std::lock_guard<std::mutex> lock(packet_queue_mutex_);
    packet_queue_.push(packet);
    return true;
}

bool RouterSimulator::receive_packet(Packet& packet) {
    std::lock_guard<std::mutex> lock(packet_queue_mutex_);
    
    if (packet_queue_.empty()) {
        return false;
    }
    
    packet = packet_queue_.front();
    packet_queue_.pop();
    return true;
}

void RouterSimulator::process_packets() {
    packet_processing_loop();
}

std::map<std::string, uint64_t> RouterSimulator::get_interface_stats(const std::string& interface) const {
    return statistics_->get_interface_stats(interface);
}

std::map<std::string, uint64_t> RouterSimulator::get_protocol_stats(const std::string& protocol) const {
    return statistics_->get_protocol_stats(protocol);
}

void RouterSimulator::reset_statistics() {
    statistics_->reset();
}

void RouterSimulator::start_cli() {
    cli_interface_->start();
}

void RouterSimulator::stop_cli() {
    cli_interface_->stop();
}

bool RouterSimulator::load_scenario(const std::string& scenario_file) {
    return yaml_config_->load_scenario(scenario_file);
}

bool RouterSimulator::run_scenario(const std::string& scenario_name) {
    return yaml_config_->run_scenario(scenario_name);
}

std::vector<std::string> RouterSimulator::list_scenarios() const {
    return yaml_config_->list_scenarios();
}

void RouterSimulator::start() {
    if (running_.load()) {
        return;
    }
    
    std::cout << "Starting Router Simulator..." << std::endl;
    running_.store(true);
    
    // Start protocols
    start_protocols();
    
    // Start packet processing thread
    packet_processing_thread_ = std::thread(&RouterSimulator::packet_processing_loop, this);
    
    // Setup networking
    setup_networking();
    
    std::cout << "Router Simulator started" << std::endl;
}

void RouterSimulator::stop() {
    if (!running_.load()) {
        return;
    }
    
    std::cout << "Stopping Router Simulator..." << std::endl;
    running_.store(false);
    
    // Stop protocols
    stop_protocols();
    
    // Wait for packet processing thread
    if (packet_processing_thread_.joinable()) {
        packet_processing_thread_.join();
    }
    
    // Cleanup networking
    cleanup_networking();
    
    std::cout << "Router Simulator stopped" << std::endl;
}

bool RouterSimulator::is_running() const {
    return running_.load();
}

void RouterSimulator::packet_processing_loop() {
    while (running_.load()) {
        Packet packet;
        if (receive_packet(packet)) {
            // Process packet through traffic shaper
            traffic_shaper_->process_packet(packet);
            
            // Apply network impairments
            netem_impairments_->apply_impairments(packet);
            
            // Process packet
            packet_processor_->process_packet(packet);
            
            // Update statistics
            statistics_->update_packet_stats(packet);
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

bool RouterSimulator::setup_networking() {
    // Setup networking interfaces
    for (const auto& pair : interfaces_) {
        const auto& iface = pair.second;
        if (iface.is_up) {
            // Configure interface (simplified)
            std::cout << "Setting up interface: " << iface.name << std::endl;
        }
    }
    
    return true;
}

void RouterSimulator::cleanup_networking() {
    // Cleanup networking interfaces
    for (const auto& pair : interfaces_) {
        const auto& iface = pair.second;
        std::cout << "Cleaning up interface: " << iface.name << std::endl;
    }
}

} // namespace RouterSim
