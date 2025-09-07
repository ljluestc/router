#include "isis.h"
#include <iostream>
#include <sstream>
#include <algorithm>

namespace router_sim {

ISISProtocol::ISISProtocol() 
    : running_(false)
    , system_id_("0000.0000.0001")
    , level_(2)
    , area_id_("49.0001")
    , stats_{} {
}

ISISProtocol::~ISISProtocol() {
    stop();
}

bool ISISProtocol::initialize(const std::map<std::string, std::string>& config) {
    std::cout << "Initializing ISIS Protocol..." << std::endl;
    
    config_ = config;
    
    // Parse configuration
    auto system_id_it = config.find("system_id");
    if (system_id_it != config.end()) {
        system_id_ = system_id_it->second;
    }
    
    auto level_it = config.find("level");
    if (level_it != config.end()) {
        level_ = static_cast<uint8_t>(std::stoul(level_it->second));
    }
    
    auto area_id_it = config.find("area_id");
    if (area_id_it != config.end()) {
        area_id_ = area_id_it->second;
    }
    
    std::cout << "ISIS Protocol initialized - System ID: " << system_id_ 
              << ", Level: " << static_cast<int>(level_) 
              << ", Area: " << area_id_ << std::endl;
    return true;
}

bool ISISProtocol::start() {
    if (running_) {
        return true;
    }
    
    std::cout << "Starting ISIS Protocol..." << std::endl;
    running_ = true;
    
    // Start ISIS adjacency with each neighbor
    for (const auto& neighbor : neighbors_) {
        if (establish_adjacency(neighbor.second)) {
            stats_.neighbors_established++;
        }
    }
    
    std::cout << "ISIS Protocol started with " << stats_.neighbors_established 
              << " established adjacencies" << std::endl;
    return true;
}

bool ISISProtocol::stop() {
    if (!running_) {
        return true;
    }
    
    std::cout << "Stopping ISIS Protocol..." << std::endl;
    running_ = false;
    
    // Close all ISIS adjacencies
    for (auto& neighbor : neighbors_) {
        neighbor.second.state = "Down";
        if (neighbor_update_callback_) {
            neighbor_update_callback_(neighbor.second, false);
        }
    }
    
    stats_.neighbors_established = 0;
    std::cout << "ISIS Protocol stopped" << std::endl;
    return true;
}

bool ISISProtocol::is_running() const {
    return running_;
}

bool ISISProtocol::add_neighbor(const std::string& address, const std::map<std::string, std::string>& config) {
    std::cout << "Adding ISIS neighbor: " << address << std::endl;
    
    NeighborInfo neighbor;
    neighbor.address = address;
    neighbor.interface = config.count("interface") ? config.at("interface") : "";
    neighbor.state = "Down";
    neighbor.as_number = 0; // ISIS doesn't use AS numbers
    neighbor.messages_sent = 0;
    neighbor.messages_received = 0;
    neighbor.last_error = "";
    neighbor.last_seen = std::chrono::steady_clock::now();
    
    neighbors_[address] = neighbor;
    
    // Try to establish adjacency if protocol is running
    if (running_) {
        if (establish_adjacency(neighbor)) {
            stats_.neighbors_established++;
        }
    }
    
    return true;
}

bool ISISProtocol::remove_neighbor(const std::string& address) {
    std::cout << "Removing ISIS neighbor: " << address << std::endl;
    
    auto it = neighbors_.find(address);
    if (it != neighbors_.end()) {
        if (it->second.state == "Up") {
            stats_.neighbors_established--;
        }
        neighbors_.erase(it);
        return true;
    }
    
    return false;
}

std::vector<NeighborInfo> ISISProtocol::get_neighbors() const {
    std::vector<NeighborInfo> result;
    for (const auto& pair : neighbors_) {
        result.push_back(pair.second);
    }
    return result;
}

bool ISISProtocol::is_neighbor_established(const std::string& address) const {
    auto it = neighbors_.find(address);
    return it != neighbors_.end() && it->second.state == "Up";
}

bool ISISProtocol::advertise_route(const RouteInfo& route) {
    std::cout << "ISIS: Advertising route " << route.destination 
              << "/" << static_cast<int>(route.prefix_length) << std::endl;
    
    // Store the route
    std::string key = route.destination + "/" + std::to_string(route.prefix_length);
    routes_[key] = route;
    
    // Send LSP UPDATE to all established neighbors
    for (auto& neighbor : neighbors_) {
        if (neighbor.second.state == "Up") {
            send_lsp_update(neighbor.second, route);
            neighbor.second.messages_sent++;
        }
    }
    
    stats_.routes_advertised++;
    return true;
}

bool ISISProtocol::withdraw_route(const std::string& destination, uint8_t prefix_length) {
    std::cout << "ISIS: Withdrawing route " << destination 
              << "/" << static_cast<int>(prefix_length) << std::endl;
    
    // Remove the route
    std::string key = destination + "/" + std::to_string(prefix_length);
    auto it = routes_.find(key);
    if (it != routes_.end()) {
        routes_.erase(it);
        
        // Send LSP UPDATE to all established neighbors
        for (auto& neighbor : neighbors_) {
            if (neighbor.second.state == "Up") {
                RouteInfo withdraw_route;
                withdraw_route.destination = destination;
                withdraw_route.prefix_length = prefix_length;
                send_lsp_update(neighbor.second, withdraw_route);
                neighbor.second.messages_sent++;
            }
        }
        
        stats_.routes_withdrawn++;
        return true;
    }
    
    return false;
}

std::vector<RouteInfo> ISISProtocol::get_routes() const {
    std::vector<RouteInfo> result;
    for (const auto& pair : routes_) {
        result.push_back(pair.second);
    }
    return result;
}

bool ISISProtocol::update_config(const std::map<std::string, std::string>& config) {
    config_.insert(config.begin(), config.end());
    
    // Update system ID if specified
    auto system_id_it = config.find("system_id");
    if (system_id_it != config.end()) {
        system_id_ = system_id_it->second;
    }
    
    // Update level if specified
    auto level_it = config.find("level");
    if (level_it != config.end()) {
        level_ = static_cast<uint8_t>(std::stoul(level_it->second));
    }
    
    // Update area ID if specified
    auto area_id_it = config.find("area_id");
    if (area_id_it != config.end()) {
        area_id_ = area_id_it->second;
    }
    
    return true;
}

std::map<std::string, std::string> ISISProtocol::get_config() const {
    return config_;
}

ISISProtocol::ProtocolStatistics ISISProtocol::get_statistics() const {
    return stats_;
}

void ISISProtocol::set_route_update_callback(RouteUpdateCallback callback) {
    route_update_callback_ = callback;
}

void ISISProtocol::set_neighbor_update_callback(NeighborUpdateCallback callback) {
    neighbor_update_callback_ = callback;
}

bool ISISProtocol::establish_adjacency(NeighborInfo& neighbor) {
    std::cout << "ISIS: Establishing adjacency with " << neighbor.address << std::endl;
    
    // Simulate ISIS adjacency establishment
    neighbor.state = "Init";
    
    // Send HELLO message
    if (send_hello_message(neighbor)) {
        neighbor.state = "Up";
        neighbor.messages_sent++;
        
        // Simulate receiving HELLO message
        if (receive_hello_message(neighbor)) {
            neighbor.messages_received++;
            neighbor.last_seen = std::chrono::steady_clock::now();
            
            if (neighbor_update_callback_) {
                neighbor_update_callback_(neighbor, true);
            }
            
            std::cout << "ISIS: Adjacency established with " << neighbor.address << std::endl;
            return true;
        }
    }
    
    neighbor.state = "Down";
    neighbor.last_error = "Failed to establish adjacency";
    return false;
}

bool ISISProtocol::send_hello_message(const NeighborInfo& neighbor) {
    // Simulate sending ISIS HELLO message
    std::cout << "ISIS: Sending HELLO message to " << neighbor.address << std::endl;
    return true;
}

bool ISISProtocol::receive_hello_message(NeighborInfo& neighbor) {
    // Simulate receiving ISIS HELLO message
    std::cout << "ISIS: Received HELLO message from " << neighbor.address << std::endl;
    return true;
}

bool ISISProtocol::send_lsp_update(const NeighborInfo& neighbor, const RouteInfo& route) {
    // Simulate sending ISIS LSP UPDATE message
    std::cout << "ISIS: Sending LSP UPDATE to " << neighbor.address 
              << " for route " << route.destination << "/" << static_cast<int>(route.prefix_length) << std::endl;
    return true;
}

void ISISProtocol::process_lsp_update(const NeighborInfo& neighbor, const RouteInfo& route) {
    std::cout << "ISIS: Processing LSP UPDATE from " << neighbor.address 
              << " for route " << route.destination << "/" << static_cast<int>(route.prefix_length) << std::endl;
    
    // Store received route
    std::string key = route.destination + "/" + std::to_string(route.prefix_length);
    routes_[key] = route;
    
    stats_.routes_received++;
    
    if (route_update_callback_) {
        route_update_callback_(route, true);
    }
}

} // namespace router_sim
