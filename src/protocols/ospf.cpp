#include "protocols/ospf.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <thread>
#include <random>

namespace router_sim {

OSPFProtocol::OSPFProtocol() : running_(false), area_id_(0), router_id_("0.0.0.0") {
    config_.area_id = 0;
    config_.router_id = "0.0.0.0";
    config_.hello_interval = 10;
    config_.dead_interval = 40;
    config_.retransmit_interval = 5;
    config_.transit_delay = 1;
    config_.priority = 1;
    config_.cost = 1;
}

OSPFProtocol::~OSPFProtocol() {
    stop();
}

bool OSPFProtocol::initialize(const ProtocolConfig& config) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    
    config_.parameters = config.parameters;
    config_.enabled = config.enabled;
    config_.update_interval_ms = config.update_interval_ms;
    
    // Parse OSPF-specific parameters
    auto it = config.parameters.find("area_id");
    if (it != config.parameters.end()) {
        config_.area_id = std::stoul(it->second);
        area_id_ = config_.area_id;
    }
    
    it = config.parameters.find("router_id");
    if (it != config.parameters.end()) {
        config_.router_id = it->second;
        router_id_ = config_.router_id;
    }
    
    it = config.parameters.find("hello_interval");
    if (it != config.parameters.end()) {
        config_.hello_interval = std::stoul(it->second);
    }
    
    it = config.parameters.find("dead_interval");
    if (it != config.parameters.end()) {
        config_.dead_interval = std::stoul(it->second);
    }
    
    it = config.parameters.find("cost");
    if (it != config.parameters.end()) {
        config_.cost = std::stoul(it->second);
    }
    
    it = config.parameters.find("priority");
    if (it != config.parameters.end()) {
        config_.priority = std::stoul(it->second);
    }
    
    // Initialize statistics
    stats_.lsa_sent = 0;
    stats_.lsa_received = 0;
    stats_.hello_sent = 0;
    stats_.hello_received = 0;
    stats_.dd_sent = 0;
    stats_.dd_received = 0;
    stats_.lsr_sent = 0;
    stats_.lsr_received = 0;
    stats_.lsu_sent = 0;
    stats_.lsu_received = 0;
    stats_.lsack_sent = 0;
    stats_.lsack_received = 0;
    
    std::cout << "OSPF protocol initialized with area " << area_id_ 
              << " and router ID " << router_id_ << "\n";
    return true;
}

bool OSPFProtocol::start() {
    if (running_.load()) {
        return true;
    }

    std::cout << "Starting OSPF protocol...\n";
    running_.store(true);

    // Start OSPF threads
    ospf_thread_ = std::thread(&OSPFProtocol::ospf_main_loop, this);
    hello_thread_ = std::thread(&OSPFProtocol::hello_loop, this);
    lsa_thread_ = std::thread(&OSPFProtocol::lsa_processing_loop, this);
    spf_thread_ = std::thread(&OSPFProtocol::spf_calculation_loop, this);

    std::cout << "OSPF protocol started\n";
    return true;
}

bool OSPFProtocol::stop() {
    if (!running_.load()) {
        return true;
    }

    std::cout << "Stopping OSPF protocol...\n";
    running_.store(false);

    // Wait for threads to finish
    if (ospf_thread_.joinable()) {
        ospf_thread_.join();
    }
    if (hello_thread_.joinable()) {
        hello_thread_.join();
    }
    if (lsa_thread_.joinable()) {
        lsa_thread_.join();
    }
    if (spf_thread_.joinable()) {
        spf_thread_.join();
    }

    std::cout << "OSPF protocol stopped\n";
    return true;
}

bool OSPFProtocol::is_running() const {
    return running_.load();
}

bool OSPFProtocol::add_interface(const std::string& interface, const std::map<std::string, std::string>& config) {
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    OSPFInterface ospf_if;
    ospf_if.name = interface;
    ospf_if.area_id = area_id_;
    ospf_if.cost = config_.cost;
    ospf_if.hello_interval = config_.hello_interval;
    ospf_if.dead_interval = config_.dead_interval;
    ospf_if.priority = config_.priority;
    ospf_if.state = "Down";
    ospf_if.neighbors_count = 0;
    
    // Parse interface-specific config
    auto it = config.find("cost");
    if (it != config.end()) {
        ospf_if.cost = std::stoul(it->second);
    }
    
    it = config.find("hello_interval");
    if (it != config.end()) {
        ospf_if.hello_interval = std::stoul(it->second);
    }
    
    it = config.find("dead_interval");
    if (it != config.end()) {
        ospf_if.dead_interval = std::stoul(it->second);
    }
    
    it = config.find("priority");
    if (it != config.end()) {
        ospf_if.priority = std::stoul(it->second);
    }
    
    it = config.find("network");
    if (it != config.end()) {
        ospf_if.network = it->second;
    }

    interfaces_[interface] = ospf_if;
    
    std::cout << "OSPF: Added interface " << interface << " to area " << area_id_ << "\n";
    return true;
}

bool OSPFProtocol::remove_interface(const std::string& interface) {
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    auto it = interfaces_.find(interface);
    if (it == interfaces_.end()) {
        return false;
    }

    interfaces_.erase(it);
    
    std::cout << "OSPF: Removed interface " << interface << "\n";
    return true;
}

bool OSPFProtocol::advertise_route(const std::string& prefix, const std::map<std::string, std::string>& attributes) {
    std::lock_guard<std::mutex> lock(routes_mutex_);
    
    OSPFRoute route;
    route.prefix = prefix;
    route.area_id = area_id_;
    route.cost = config_.cost;
    route.type = "Intra-Area";
    route.is_valid = true;
    route.timestamp = std::chrono::system_clock::now();
    route.attributes = attributes;
    
    std::string key = prefix;
    advertised_routes_[key] = route;
    
    std::cout << "OSPF: Advertised route " << prefix << " in area " << area_id_ << "\n";
    
    // Notify callback
    if (route_update_callback_) {
        RouteInfo route_info;
        route_info.prefix = prefix;
        route_info.next_hop = router_id_;
        route_info.metric = config_.cost;
        route_info.protocol = "OSPF";
        route_info.timestamp = route.timestamp;
        route_update_callback_(route_info, true);
    }
    
    return true;
}

bool OSPFProtocol::withdraw_route(const std::string& prefix) {
    std::lock_guard<std::mutex> lock(routes_mutex_);
    
    auto it = advertised_routes_.find(prefix);
    if (it == advertised_routes_.end()) {
        return false;
    }

    advertised_routes_.erase(it);
    
    std::cout << "OSPF: Withdrew route " << prefix << "\n";
    
    // Notify callback
    if (route_update_callback_) {
        RouteInfo route_info;
        route_info.prefix = prefix;
        route_info.protocol = "OSPF";
        route_info.timestamp = std::chrono::system_clock::now();
        route_update_callback_(route_info, false);
    }
    
    return true;
}

std::vector<RouteInfo> OSPFProtocol::get_routes() const {
    std::lock_guard<std::mutex> lock(routes_mutex_);
    
    std::vector<RouteInfo> result;
    
    // Add advertised routes
    for (const auto& pair : advertised_routes_) {
        const auto& route = pair.second;
        RouteInfo route_info;
        route_info.prefix = route.prefix;
        route_info.next_hop = router_id_;
        route_info.metric = route.cost;
        route_info.protocol = "OSPF";
        route_info.timestamp = route.timestamp;
        result.push_back(route_info);
    }
    
    // Add learned routes
    for (const auto& pair : learned_routes_) {
        const auto& route = pair.second;
        RouteInfo route_info;
        route_info.prefix = route.prefix;
        route_info.next_hop = route.next_hop;
        route_info.metric = route.cost;
        route_info.protocol = "OSPF";
        route_info.timestamp = route.timestamp;
        result.push_back(route_info);
    }
    
    return result;
}

std::vector<NeighborInfo> OSPFProtocol::get_neighbors() const {
    std::lock_guard<std::mutex> lock(neighbors_mutex_);
    
    std::vector<NeighborInfo> result;
    for (const auto& pair : neighbors_) {
        const auto& neighbor = pair.second;
        NeighborInfo neighbor_info;
        neighbor_info.address = neighbor.router_id;
        neighbor_info.state = neighbor.state;
        neighbor_info.protocol = "OSPF";
        neighbor_info.established_time = neighbor.established_time;
        result.push_back(neighbor_info);
    }
    return result;
}

std::map<std::string, uint64_t> OSPFProtocol::get_statistics() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    return {
        {"lsa_sent", stats_.lsa_sent},
        {"lsa_received", stats_.lsa_received},
        {"hello_sent", stats_.hello_sent},
        {"hello_received", stats_.hello_received},
        {"dd_sent", stats_.dd_sent},
        {"dd_received", stats_.dd_received},
        {"lsr_sent", stats_.lsr_sent},
        {"lsr_received", stats_.lsr_received},
        {"lsu_sent", stats_.lsu_sent},
        {"lsu_received", stats_.lsu_received},
        {"lsack_sent", stats_.lsack_sent},
        {"lsack_received", stats_.lsack_received}
    };
}

void OSPFProtocol::ospf_main_loop() {
    std::cout << "OSPF main loop started\n";
    
    while (running_.load()) {
        // Process incoming OSPF messages
        process_incoming_messages();
        
        // Update interface states
        update_interface_states();
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cout << "OSPF main loop stopped\n";
}

void OSPFProtocol::hello_loop() {
    std::cout << "OSPF hello loop started\n";
    
    while (running_.load()) {
        std::lock_guard<std::mutex> lock(interfaces_mutex_);
        
        for (auto& pair : interfaces_) {
            auto& interface = pair.second;
            
            if (interface.state == "Up") {
                send_hello_message(interface.name);
                stats_.hello_sent++;
            }
        }
        
        std::this_thread::sleep_for(std::chrono::seconds(config_.hello_interval));
    }
    
    std::cout << "OSPF hello loop stopped\n";
}

void OSPFProtocol::lsa_processing_loop() {
    std::cout << "OSPF LSA processing loop started\n";
    
    while (running_.load()) {
        // Process LSA updates
        process_lsa_updates();
        
        // Flood LSAs
        flood_lsas();
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cout << "OSPF LSA processing loop stopped\n";
}

void OSPFProtocol::spf_calculation_loop() {
    std::cout << "OSPF SPF calculation loop started\n";
    
    while (running_.load()) {
        // Run SPF calculation
        run_spf_calculation();
        
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
    
    std::cout << "OSPF SPF calculation loop stopped\n";
}

void OSPFProtocol::process_incoming_messages() {
    // Simulate processing incoming OSPF messages
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

void OSPFProtocol::update_interface_states() {
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    for (auto& pair : interfaces_) {
        auto& interface = pair.second;
        
        if (interface.state == "Down") {
            interface.state = "Up";
            std::cout << "OSPF: Interface " << interface.name << " is now Up\n";
        }
    }
}

void OSPFProtocol::process_lsa_updates() {
    // Simulate LSA processing
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
}

void OSPFProtocol::flood_lsas() {
    // Simulate LSA flooding
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
}

void OSPFProtocol::run_spf_calculation() {
    // Simulate SPF calculation
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

bool OSPFProtocol::send_hello_message(const std::string& interface) {
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    auto it = interfaces_.find(interface);
    if (it == interfaces_.end()) {
        return false;
    }
    
    auto& interface_obj = it->second;
    interface_obj.hello_sent++;
    
    std::cout << "OSPF: Sent HELLO on interface " << interface << "\n";
    return true;
}

void OSPFProtocol::set_route_update_callback(std::function<void(const RouteInfo&, bool)> callback) {
    route_update_callback_ = callback;
}

void OSPFProtocol::set_neighbor_update_callback(std::function<void(const NeighborInfo&, bool)> callback) {
    neighbor_update_callback_ = callback;
}

} // namespace router_sim
