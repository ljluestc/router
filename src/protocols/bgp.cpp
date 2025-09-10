#include "protocols/bgp.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <thread>
#include <random>

namespace router_sim {

BGPProtocol::BGPProtocol() : running_(false), next_hop_("0.0.0.0") {
    config_.local_as = 0;
    config_.router_id = "";
    config_.enable_graceful_restart = false;
    config_.hold_time = 180;
    config_.keepalive_interval = 60;
    config_.graceful_restart_time = 120;
    config_.stale_route_time = 360;
}

BGPProtocol::~BGPProtocol() {
    stop();
}

bool BGPProtocol::initialize(const ProtocolConfig& config) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    
    // Convert generic config to BGP-specific config
    config_.parameters = config.parameters;
    config_.enabled = config.enabled;
    config_.update_interval_ms = config.update_interval_ms;
    
    // Parse BGP-specific parameters
    auto it = config.parameters.find("local_as");
    if (it != config.parameters.end()) {
        config_.local_as = std::stoul(it->second);
    }
    
    it = config.parameters.find("router_id");
    if (it != config.parameters.end()) {
        config_.router_id = it->second;
    }
    
    it = config.parameters.find("neighbors");
    if (it != config.parameters.end()) {
        // Parse comma-separated neighbors
        std::stringstream ss(it->second);
        std::string neighbor;
        while (std::getline(ss, neighbor, ',')) {
            config_.neighbors.push_back(neighbor);
        }
    }
    
    it = config.parameters.find("hold_time");
    if (it != config.parameters.end()) {
        config_.hold_time = std::stoul(it->second);
    }
    
    it = config.parameters.find("keepalive_interval");
    if (it != config.parameters.end()) {
        config_.keepalive_interval = std::stoul(it->second);
    }
    
    it = config.parameters.find("next_hop");
    if (it != config.parameters.end()) {
        next_hop_ = it->second;
    }
    
    // Initialize statistics
    stats_.messages_sent = 0;
    stats_.messages_received = 0;
    stats_.routes_advertised = 0;
    stats_.routes_withdrawn = 0;
    stats_.routes_learned = 0;
    stats_.neighbors_established = 0;
    stats_.neighbors_failed = 0;
    
    std::cout << "BGP protocol initialized with AS " << config_.local_as 
              << " and router ID " << config_.router_id << "\n";
    return true;
}

bool BGPProtocol::start() {
    if (running_.load()) {
        return true;
    }

    std::cout << "Starting BGP protocol...\n";
    running_.store(true);

    // Start BGP threads
    bgp_thread_ = std::thread(&BGPProtocol::bgp_main_loop, this);
    neighbor_thread_ = std::thread(&BGPProtocol::neighbor_management_loop, this);
    route_thread_ = std::thread(&BGPProtocol::route_processing_loop, this);
    keepalive_thread_ = std::thread(&BGPProtocol::keepalive_loop, this);

    std::cout << "BGP protocol started\n";
    return true;
}

bool BGPProtocol::stop() {
    if (!running_.load()) {
        return true;
    }

    std::cout << "Stopping BGP protocol...\n";
    running_.store(false);

    // Wait for threads to finish
    if (bgp_thread_.joinable()) {
        bgp_thread_.join();
    }
    if (neighbor_thread_.joinable()) {
        neighbor_thread_.join();
    }
    if (route_thread_.joinable()) {
        route_thread_.join();
    }
    if (keepalive_thread_.joinable()) {
        keepalive_thread_.join();
    }

    std::cout << "BGP protocol stopped\n";
    return true;
}

bool BGPProtocol::is_running() const {
    return running_.load();
}

std::vector<std::string> BGPProtocol::get_advertised_routes() const {
    std::lock_guard<std::mutex> lock(routes_mutex_);
    
    std::vector<std::string> result;
    for (const auto& pair : advertised_routes_) {
        result.push_back(pair.first);
    }
    return result;
}

std::vector<std::string> BGPProtocol::get_learned_routes() const {
    std::lock_guard<std::mutex> lock(routes_mutex_);
    
    std::vector<std::string> result;
    for (const auto& pair : learned_routes_) {
        result.push_back(pair.first);
    }
    return result;
}

bool BGPProtocol::advertise_route(const std::string& prefix, uint8_t prefix_length, 
                                 uint32_t metric, const std::map<std::string, std::string>& attributes) {
    std::lock_guard<std::mutex> lock(routes_mutex_);
    
    BGPRoute route;
    route.prefix = prefix;
    route.prefix_length = prefix_length;
    route.metric = metric;
    route.next_hop = next_hop_;
    route.is_valid = true;
    route.attributes = attributes;
    route.timestamp = std::chrono::system_clock::now();
    
    std::string key = prefix + "/" + std::to_string(prefix_length);
    advertised_routes_[key] = route;
    
    stats_.routes_advertised++;
    
    std::cout << "BGP: Advertised route " << prefix << "/" 
              << static_cast<int>(prefix_length) << " with metric " << metric << "\n";
    
    // Notify callback
    if (route_update_callback_) {
        RouteInfo route_info;
        route_info.prefix = prefix;
        route_info.prefix_length = prefix_length;
        route_info.next_hop = next_hop_;
        route_info.metric = metric;
        route_info.protocol = "BGP";
        route_info.timestamp = route.timestamp;
        route_update_callback_(route_info, true);
    }
    
    return true;
}

bool BGPProtocol::withdraw_route(const std::string& prefix, uint8_t prefix_length) {
    std::lock_guard<std::mutex> lock(routes_mutex_);
    
    std::string key = prefix + "/" + std::to_string(prefix_length);
    auto it = advertised_routes_.find(key);
    if (it == advertised_routes_.end()) {
        return false;
    }

    advertised_routes_.erase(it);
    stats_.routes_withdrawn++;
    
    std::cout << "BGP: Withdrew route " << prefix << "/" 
              << static_cast<int>(prefix_length) << "\n";
    
    // Notify callback
    if (route_update_callback_) {
        RouteInfo route_info;
        route_info.prefix = prefix;
        route_info.prefix_length = prefix_length;
        route_info.protocol = "BGP";
        route_info.timestamp = std::chrono::system_clock::now();
        route_update_callback_(route_info, false);
    }
    
    return true;
}

bool BGPProtocol::add_neighbor(const std::string& address, const std::map<std::string, std::string>& config) {
    std::lock_guard<std::mutex> lock(neighbors_mutex_);
    
    BGPNeighbor neighbor;
    neighbor.address = address;
    neighbor.as_number = 0;
    neighbor.state = "Idle";
    neighbor.hold_time = config_.hold_time;
    neighbor.keepalive_interval = config_.keepalive_interval;
    neighbor.messages_sent = 0;
    neighbor.messages_received = 0;
    neighbor.last_error = "";
    neighbor.established_time = std::chrono::system_clock::now();
    
    // Parse neighbor-specific config
    auto it = config.find("as_number");
    if (it != config.end()) {
        neighbor.as_number = std::stoul(it->second);
    }
    
    it = config.find("hold_time");
    if (it != config.end()) {
        neighbor.hold_time = std::stoul(it->second);
    }
    
    it = config.find("keepalive_interval");
    if (it != config.end()) {
        neighbor.keepalive_interval = std::stoul(it->second);
    }
    
    it = config.find("password");
    if (it != config.end()) {
        neighbor.password = it->second;
    }

    neighbors_[address] = neighbor;
    config_.neighbors.push_back(address);
    config_.neighbor_as[address] = neighbor.as_number;
    
    std::cout << "BGP: Added neighbor " << address << " with AS " << neighbor.as_number << "\n";
    return true;
}

bool BGPProtocol::remove_neighbor(const std::string& address) {
    std::lock_guard<std::mutex> lock(neighbors_mutex_);
    
    auto it = neighbors_.find(address);
    if (it == neighbors_.end()) {
        return false;
    }

    neighbors_.erase(it);
    
    // Remove from config
    auto config_it = std::find(config_.neighbors.begin(), config_.neighbors.end(), address);
    if (config_it != config_.neighbors.end()) {
        config_.neighbors.erase(config_it);
    }
    config_.neighbor_as.erase(address);
    
    std::cout << "BGP: Removed neighbor " << address << "\n";
    return true;
}

std::vector<BGPNeighbor> BGPProtocol::get_neighbors() const {
    std::lock_guard<std::mutex> lock(neighbors_mutex_);
    
    std::vector<BGPNeighbor> result;
    for (const auto& pair : neighbors_) {
        result.push_back(pair.second);
    }
    return result;
}

BGPNeighbor BGPProtocol::get_neighbor(const std::string& address) const {
    std::lock_guard<std::mutex> lock(neighbors_mutex_);
    
    auto it = neighbors_.find(address);
    if (it == neighbors_.end()) {
        return BGPNeighbor{};
    }
    return it->second;
}

std::vector<RouteInfo> BGPProtocol::get_routes() const {
    std::lock_guard<std::mutex> lock(routes_mutex_);
    
    std::vector<RouteInfo> result;
    
    // Add advertised routes
    for (const auto& pair : advertised_routes_) {
        const auto& route = pair.second;
        RouteInfo route_info;
        route_info.prefix = route.prefix;
        route_info.prefix_length = route.prefix_length;
        route_info.next_hop = route.next_hop;
        route_info.metric = route.metric;
        route_info.protocol = "BGP";
        route_info.timestamp = route.timestamp;
        result.push_back(route_info);
    }
    
    // Add learned routes
    for (const auto& pair : learned_routes_) {
        const auto& route = pair.second;
        RouteInfo route_info;
        route_info.prefix = route.prefix;
        route_info.prefix_length = route.prefix_length;
        route_info.next_hop = route.next_hop;
        route_info.metric = route.metric;
        route_info.protocol = "BGP";
        route_info.timestamp = route.timestamp;
        result.push_back(route_info);
    }
    
    return result;
}

std::vector<NeighborInfo> BGPProtocol::get_neighbors() const {
    std::lock_guard<std::mutex> lock(neighbors_mutex_);
    
    std::vector<NeighborInfo> result;
    for (const auto& pair : neighbors_) {
        const auto& neighbor = pair.second;
        NeighborInfo neighbor_info;
        neighbor_info.address = neighbor.address;
        neighbor_info.state = neighbor.state;
        neighbor_info.protocol = "BGP";
        neighbor_info.established_time = neighbor.established_time;
        result.push_back(neighbor_info);
    }
    return result;
}

std::map<std::string, uint64_t> BGPProtocol::get_statistics() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    return {
        {"messages_sent", stats_.messages_sent},
        {"messages_received", stats_.messages_received},
        {"routes_advertised", stats_.routes_advertised},
        {"routes_withdrawn", stats_.routes_withdrawn},
        {"routes_learned", stats_.routes_learned},
        {"neighbors_established", stats_.neighbors_established},
        {"neighbors_failed", stats_.neighbors_failed}
    };
}

void BGPProtocol::bgp_main_loop() {
    std::cout << "BGP main loop started\n";
    
    while (running_.load()) {
        // Process incoming BGP messages
        process_incoming_messages();
        
        // Update neighbor states
        update_neighbor_states();
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cout << "BGP main loop stopped\n";
}

void BGPProtocol::neighbor_management_loop() {
    std::cout << "BGP neighbor management loop started\n";
    
    while (running_.load()) {
        std::lock_guard<std::mutex> lock(neighbors_mutex_);
        
        for (auto& pair : neighbors_) {
            auto& neighbor = pair.second;
            
            // Try to establish connection if in Idle state
            if (neighbor.state == "Idle") {
                if (establish_session(neighbor.address)) {
                    neighbor.state = "Connect";
                }
            }
            // Send keepalive if established
            else if (neighbor.state == "Established") {
                send_keepalive(neighbor.address);
            }
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    
    std::cout << "BGP neighbor management loop stopped\n";
}

void BGPProtocol::route_processing_loop() {
    std::cout << "BGP route processing loop started\n";
    
    while (running_.load()) {
        // Process route updates
        process_route_updates();
        
        // Apply route policies
        apply_route_policies();
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cout << "BGP route processing loop stopped\n";
}

void BGPProtocol::keepalive_loop() {
    std::cout << "BGP keepalive loop started\n";
    
    while (running_.load()) {
        std::lock_guard<std::mutex> lock(neighbors_mutex_);
        
        auto now = std::chrono::system_clock::now();
        for (auto& pair : neighbors_) {
            auto& neighbor = pair.second;
            
            if (neighbor.state == "Established") {
                auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                    now - neighbor.last_keepalive_sent);
                
                if (elapsed.count() >= neighbor.keepalive_interval) {
                    send_keepalive(neighbor.address);
                    neighbor.last_keepalive_sent = now;
                }
            }
        }
        
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    std::cout << "BGP keepalive loop stopped\n";
}

bool BGPProtocol::establish_session(const std::string& neighbor_address) {
    std::lock_guard<std::mutex> lock(neighbors_mutex_);
    
    auto it = neighbors_.find(neighbor_address);
    if (it == neighbors_.end()) {
        return false;
    }
    
    auto& neighbor = it->second;
    
    // Simulate connection establishment
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    
    if (dis(gen) > 0.1) { // 90% success rate
        neighbor.state = "Established";
        neighbor.established_time = std::chrono::system_clock::now();
        neighbor.last_keepalive_sent = neighbor.established_time;
        stats_.neighbors_established++;
        
        // Send OPEN message
        send_open_message(neighbor_address);
        
        std::cout << "BGP: Established session with " << neighbor_address << "\n";
        return true;
    } else {
        neighbor.state = "Active";
        neighbor.last_error = "Connection failed";
        stats_.neighbors_failed++;
        return false;
    }
}

bool BGPProtocol::send_open_message(const std::string& neighbor_address) {
    std::lock_guard<std::mutex> lock(neighbors_mutex_);
    
    auto it = neighbors_.find(neighbor_address);
    if (it == neighbors_.end()) {
        return false;
    }
    
    auto& neighbor = it->second;
    neighbor.messages_sent++;
    stats_.messages_sent++;
    
    std::cout << "BGP: Sent OPEN message to " << neighbor_address << "\n";
    return true;
}

bool BGPProtocol::send_keepalive(const std::string& neighbor_address) {
    std::lock_guard<std::mutex> lock(neighbors_mutex_);
    
    auto it = neighbors_.find(neighbor_address);
    if (it == neighbors_.end()) {
        return false;
    }
    
    auto& neighbor = it->second;
    neighbor.messages_sent++;
    stats_.messages_sent++;
    
    return true;
}

bool BGPProtocol::send_update_message(const std::string& neighbor_address, 
                                     const std::vector<BGPRoute>& routes) {
    std::lock_guard<std::mutex> lock(neighbors_mutex_);
    
    auto it = neighbors_.find(neighbor_address);
    if (it == neighbors_.end()) {
        return false;
    }
    
    auto& neighbor = it->second;
    neighbor.messages_sent++;
    stats_.messages_sent++;
    
    std::cout << "BGP: Sent UPDATE message to " << neighbor_address 
              << " with " << routes.size() << " routes\n";
    return true;
}

void BGPProtocol::process_incoming_messages() {
    // Simulate processing incoming BGP messages
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

void BGPProtocol::update_neighbor_states() {
    std::lock_guard<std::mutex> lock(neighbors_mutex_);
    
    auto now = std::chrono::system_clock::now();
    for (auto& pair : neighbors_) {
        auto& neighbor = pair.second;
        
        if (neighbor.state == "Established") {
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                now - neighbor.last_keepalive_received);
            
            if (elapsed.count() > neighbor.hold_time) {
                neighbor.state = "Idle";
                neighbor.last_error = "Hold timer expired";
                std::cout << "BGP: Hold timer expired for " << neighbor.address << "\n";
            }
        }
    }
}

void BGPProtocol::process_route_updates() {
    // Simulate route processing
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
}

void BGPProtocol::apply_route_policies() {
    // Simulate policy application
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
}

void BGPProtocol::set_route_update_callback(std::function<void(const RouteInfo&, bool)> callback) {
    route_update_callback_ = callback;
}

void BGPProtocol::set_neighbor_update_callback(std::function<void(const NeighborInfo&, bool)> callback) {
    neighbor_update_callback_ = callback;
}

} // namespace router_sim