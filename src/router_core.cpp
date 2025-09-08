#include "router_core.h"
#include "protocols/bgp.h"
#include "protocols/ospf.h"
#include "protocols/isis.h"
#include "traffic_shaping/traffic_shaper.h"
#include "frr_integration/frr_control.h"
#include "analytics/clickhouse_client.h"
#include "config/yaml_config.h"

#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <signal.h>
#include <unistd.h>

namespace RouterSim {

RouterCore::RouterCore() 
    : running_(false), 
      initialized_(false),
      config_loaded_(false) {
    // Initialize signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
}

RouterCore::~RouterCore() {
    stop();
}

bool RouterCore::initialize(const std::string& config_file) {
    std::lock_guard<std::mutex> lock(state_mutex_);
    
    if (initialized_) {
        return true;
    }
    
    // Load configuration
    if (!config_file.empty()) {
        if (!load_config(config_file)) {
            std::cerr << "Failed to load configuration from " << config_file << std::endl;
            return false;
        }
        config_loaded_ = true;
    }
    
    // Initialize components
    if (!initialize_protocols()) {
        std::cerr << "Failed to initialize protocols" << std::endl;
        return false;
    }
    
    if (!initialize_traffic_shaping()) {
        std::cerr << "Failed to initialize traffic shaping" << std::endl;
        return false;
    }
    
    if (!initialize_frr_integration()) {
        std::cerr << "Failed to initialize FRR integration" << std::endl;
        return false;
    }
    
    if (!initialize_analytics()) {
        std::cerr << "Failed to initialize analytics" << std::endl;
        return false;
    }
    
    initialized_ = true;
    std::cout << "Router core initialized successfully" << std::endl;
    return true;
}

bool RouterCore::start() {
    std::lock_guard<std::mutex> lock(state_mutex_);
    
    if (!initialized_) {
        std::cerr << "Router core not initialized" << std::endl;
        return false;
    }
    
    if (running_) {
        return true;
    }
    
    std::cout << "Starting router core..." << std::endl;
    
    // Start protocols
    for (auto& protocol : protocols_) {
        if (protocol.second->is_enabled()) {
            if (!protocol.second->start()) {
                std::cerr << "Failed to start protocol " << protocol.first << std::endl;
                return false;
            }
        }
    }
    
    // Start traffic shaping
    if (traffic_shaper_) {
        traffic_shaper_->start();
    }
    
    // Start FRR integration
    if (frr_integration_) {
        frr_integration_->start();
    }
    
    // Start analytics
    if (analytics_engine_) {
        analytics_engine_->start();
    }
    
    // Start main processing loop
    running_ = true;
    main_thread_ = std::thread(&RouterCore::main_loop, this);
    
    std::cout << "Router core started successfully" << std::endl;
    return true;
}

void RouterCore::stop() {
    std::lock_guard<std::mutex> lock(state_mutex_);
    
    if (!running_) {
        return;
    }
    
    std::cout << "Stopping router core..." << std::endl;
    
    running_ = false;
    
    // Wait for main thread
    if (main_thread_.joinable()) {
        main_thread_.join();
    }
    
    // Stop components
    if (analytics_engine_) {
        analytics_engine_->stop();
    }
    
    if (frr_integration_) {
        frr_integration_->stop();
    }
    
    if (traffic_shaper_) {
        traffic_shaper_->stop();
    }
    
    for (auto& protocol : protocols_) {
        protocol.second->stop();
    }
    
    std::cout << "Router core stopped" << std::endl;
}

bool RouterCore::is_running() const {
    std::lock_guard<std::mutex> lock(state_mutex_);
    return running_;
}

bool RouterCore::load_config(const std::string& config_file) {
    std::ifstream file(config_file);
    if (!file.is_open()) {
        std::cerr << "Cannot open config file: " << config_file << std::endl;
        return false;
    }
    
    try {
        YAML::Node config = YAML::Load(file);
        
        // Load router configuration
        if (config["router"]) {
            auto router_config = config["router"];
            
            // Load interfaces
            if (router_config["interfaces"]) {
                for (const auto& iface : router_config["interfaces"]) {
                    InterfaceConfig iface_config;
                    iface_config.name = iface["name"].as<std::string>();
                    iface_config.ip_address = iface["ip_address"].as<std::string>();
                    iface_config.subnet_mask = iface["subnet_mask"].as<std::string>();
                    iface_config.mtu = iface["mtu"].as<int>(1500);
                    iface_config.enabled = iface["enabled"].as<bool>(true);
                    
                    interfaces_[iface_config.name] = iface_config;
                }
            }
            
            // Load protocols
            if (router_config["protocols"]) {
                for (const auto& proto : router_config["protocols"]) {
                    std::string proto_name = proto["type"].as<std::string>();
                    bool enabled = proto["enabled"].as<bool>(false);
                    
                    if (enabled) {
                        enable_protocol(proto_name, proto);
                    }
                }
            }
            
            // Load traffic shaping
            if (router_config["traffic_shaping"]) {
                auto ts_config = router_config["traffic_shaping"];
                if (ts_config["enabled"].as<bool>(false)) {
                    configure_traffic_shaping(ts_config);
                }
            }
        }
        
        config_loaded_ = true;
        std::cout << "Configuration loaded from " << config_file << std::endl;
        return true;
        
    } catch (const YAML::Exception& e) {
        std::cerr << "YAML parsing error: " << e.what() << std::endl;
        return false;
    }
}

bool RouterCore::save_config(const std::string& config_file) const {
    std::ofstream file(config_file);
    if (!file.is_open()) {
        std::cerr << "Cannot create config file: " << config_file << std::endl;
        return false;
    }
    
    YAML::Node config;
    
    // Router configuration
    config["router"]["interfaces"] = YAML::Node(YAML::NodeType::Sequence);
    for (const auto& iface : interfaces_) {
        YAML::Node iface_node;
        iface_node["name"] = iface.second.name;
        iface_node["ip_address"] = iface.second.ip_address;
        iface_node["subnet_mask"] = iface.second.subnet_mask;
        iface_node["mtu"] = iface.second.mtu;
        iface_node["enabled"] = iface.second.enabled;
        config["router"]["interfaces"].push_back(iface_node);
    }
    
    // Protocols
    config["router"]["protocols"] = YAML::Node(YAML::NodeType::Sequence);
    for (const auto& proto : protocols_) {
        if (proto.second->is_enabled()) {
            YAML::Node proto_node;
            proto_node["type"] = proto.first;
            proto_node["enabled"] = true;
            config["router"]["protocols"].push_back(proto_node);
        }
    }
    
    file << config;
    std::cout << "Configuration saved to " << config_file << std::endl;
    return true;
}

bool RouterCore::add_interface(const std::string& name, const std::string& ip_address, 
                               const std::string& subnet_mask, int mtu) {
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    InterfaceConfig config;
    config.name = name;
    config.ip_address = ip_address;
    config.subnet_mask = subnet_mask;
    config.mtu = mtu;
    config.enabled = true;
    
    interfaces_[name] = config;
    std::cout << "Added interface " << name << " with IP " << ip_address << std::endl;
    return true;
}

bool RouterCore::remove_interface(const std::string& name) {
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    auto it = interfaces_.find(name);
    if (it == interfaces_.end()) {
        return false;
    }
    
    interfaces_.erase(it);
    std::cout << "Removed interface " << name << std::endl;
    return true;
}

std::vector<InterfaceConfig> RouterCore::get_interfaces() const {
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    std::vector<InterfaceConfig> result;
    for (const auto& iface : interfaces_) {
        result.push_back(iface.second);
    }
    return result;
}

bool RouterCore::add_route(const std::string& destination, const std::string& next_hop, 
                          const std::string& interface, int metric) {
    std::lock_guard<std::mutex> lock(routes_mutex_);
    
    Route route;
    route.destination = destination;
    route.next_hop = next_hop;
    route.interface = interface;
    route.metric = metric;
    route.protocol = "static";
    route.is_active = true;
    
    std::string key = destination + "/" + next_hop;
    routes_[key] = route;
    
    std::cout << "Added route " << destination << " via " << next_hop << std::endl;
    return true;
}

bool RouterCore::remove_route(const std::string& destination) {
    std::lock_guard<std::mutex> lock(routes_mutex_);
    
    auto it = routes_.find(destination);
    if (it == routes_.end()) {
        return false;
    }
    
    routes_.erase(it);
    std::cout << "Removed route " << destination << std::endl;
    return true;
}

std::vector<Route> RouterCore::get_routes() const {
    std::lock_guard<std::mutex> lock(routes_mutex_);
    
    std::vector<Route> result;
    for (const auto& route : routes_) {
        result.push_back(route.second);
    }
    return result;
}

bool RouterCore::enable_protocol(const std::string& protocol_name, const YAML::Node& config) {
    std::lock_guard<std::mutex> lock(protocols_mutex_);
    
    if (protocols_.find(protocol_name) != protocols_.end()) {
        return false; // Already exists
    }
    
    std::unique_ptr<ProtocolInterface> protocol;
    
    if (protocol_name == "bgp") {
        protocol = std::make_unique<BGPProtocol>();
    } else if (protocol_name == "ospf") {
        protocol = std::make_unique<OSPFProtocol>();
    } else if (protocol_name == "isis") {
        protocol = std::make_unique<ISISProtocol>();
    } else {
        std::cerr << "Unknown protocol: " << protocol_name << std::endl;
        return false;
    }
    
    if (protocol) {
        protocols_[protocol_name] = std::move(protocol);
        std::cout << "Enabled protocol " << protocol_name << std::endl;
        return true;
    }
    
    return false;
}

bool RouterCore::disable_protocol(const std::string& protocol_name) {
    std::lock_guard<std::mutex> lock(protocols_mutex_);
    
    auto it = protocols_.find(protocol_name);
    if (it == protocols_.end()) {
        return false;
    }
    
    it->second->stop();
    protocols_.erase(it);
    std::cout << "Disabled protocol " << protocol_name << std::endl;
    return true;
}

std::vector<std::string> RouterCore::get_enabled_protocols() const {
    std::lock_guard<std::mutex> lock(protocols_mutex_);
    
    std::vector<std::string> result;
    for (const auto& proto : protocols_) {
        if (proto.second->is_enabled()) {
            result.push_back(proto.first);
        }
    }
    return result;
}

RouterCore::Statistics RouterCore::get_statistics() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    return statistics_;
}

void RouterCore::update_statistics() {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    
    // Update interface statistics
    statistics_.interface_count = interfaces_.size();
    
    // Update route statistics
    statistics_.route_count = routes_.size();
    
    // Update protocol statistics
    statistics_.protocol_count = protocols_.size();
    
    // Update timestamp
    statistics_.last_update = std::chrono::steady_clock::now();
}

void RouterCore::main_loop() {
    std::cout << "Router main loop started" << std::endl;
    
    while (running_) {
        // Update statistics
        update_statistics();
        
        // Process packets (placeholder)
        process_packets();
        
        // Sleep for a short time
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    std::cout << "Router main loop stopped" << std::endl;
}

void RouterCore::process_packets() {
    // Placeholder for packet processing
    // In a real implementation, this would process incoming packets
}

bool RouterCore::initialize_protocols() {
    // Initialize protocol handlers
    return true;
}

bool RouterCore::initialize_traffic_shaping() {
    traffic_shaper_ = std::make_unique<TrafficShaper>();
    return traffic_shaper_ != nullptr;
}

bool RouterCore::initialize_frr_integration() {
    if (ENABLE_FRR) {
        frr_integration_ = std::make_unique<FRRIntegration>();
        return frr_integration_ != nullptr;
    }
    return true;
}

bool RouterCore::initialize_analytics() {
    if (ENABLE_ANALYTICS) {
        analytics_engine_ = std::make_unique<ClickHouseClient>();
        return analytics_engine_ != nullptr;
    }
    return true;
}

void RouterCore::configure_traffic_shaping(const YAML::Node& config) {
    if (!traffic_shaper_) {
        return;
    }
    
    // Configure traffic shaping based on YAML config
    // This is a placeholder implementation
}

void RouterCore::signal_handler(int signal) {
    // Global signal handler
    // In a real implementation, this would signal the router to stop
}

} // namespace RouterSim
