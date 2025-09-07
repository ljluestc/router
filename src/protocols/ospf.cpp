#include "ospf.h"
#include <iostream>
#include <sstream>
#include <algorithm>

namespace router_sim {

OSPFProtocol::OSPFProtocol() 
    : running_(false)
    , router_id_("")
    , area_id_("0.0.0.0")
    , stats_{} {
}

OSPFProtocol::~OSPFProtocol() {
    stop();
}

bool OSPFProtocol::initialize(const std::map<std::string, std::string>& config) {
    std::cout << "Initializing OSPF Protocol..." << std::endl;
    
    config_ = config;
    
    // Parse configuration
    auto router_id_it = config.find("router_id");
    if (router_id_it != config.end()) {
        router_id_ = router_id_it->second;
    } else {
        std::cerr << "OSPF: router_id not specified" << std::endl;
        return false;
    }
    
    auto area_id_it = config.find("area_id");
    if (area_id_it != config.end()) {
        area_id_ = area_id_it->second;
    }
    
    std::cout << "OSPF Protocol initialized - Router ID: " << router_id_ 
              << ", Area: " << area_id_ << std::endl;
    return true;
}

bool OSPFProtocol::start() {
    if (running_) {
        return true;
    }
    
    std::cout << "Starting OSPF Protocol..." << std::endl;
    running_ = true;
    
    // Start OSPF adjacency with each neighbor
    for (const auto& neighbor : neighbors_) {
        if (establish_adjacency(neighbor.second)) {
            stats_.neighbors_established++;
        }
    }
    
    std::cout << "OSPF Protocol started with " << stats_.neighbors_established 
              << " established adjacencies" << std::endl;
    return true;
}

bool OSPFProtocol::stop() {
    if (!running_) {
        return true;
    }
    
    std::cout << "Stopping OSPF Protocol..." << std::endl;
    running_ = false;
    
    // Close all OSPF adjacencies
    for (auto& neighbor : neighbors_) {
        neighbor.second.state = "Down";
        if (neighbor_update_callback_) {
            neighbor_update_callback_(neighbor.second, false);
        }
    }
    
    stats_.neighbors_established = 0;
    std::cout << "OSPF Protocol stopped" << std::endl;
    return true;
}

bool OSPFProtocol::is_running() const {
    return running_;
}

bool OSPFProtocol::add_neighbor(const std::string& address, const std::map<std::string, std::string>& config) {
    std::cout << "Adding OSPF neighbor: " << address << std::endl;
    
    NeighborInfo neighbor;
    neighbor.address = address;
    neighbor.interface = config.count("interface") ? config.at("interface") : "";
    neighbor.state = "Down";
    neighbor.as_number = 0; // OSPF doesn't use AS numbers
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

bool OSPFProtocol::remove_neighbor(const std::string& address) {
    std::cout << "Removing OSPF neighbor: " << address << std::endl;
    
    auto it = neighbors_.find(address);
    if (it != neighbors_.end()) {
        if (it->second.state == "Full") {
            stats_.neighbors_established--;
        }
        neighbors_.erase(it);
        return true;
    }
    
    return false;
}

std::vector<NeighborInfo> OSPFProtocol::get_neighbors() const {
    std::vector<NeighborInfo> result;
    for (const auto& pair : neighbors_) {
        result.push_back(pair.second);
    }
    return result;
}

bool OSPFProtocol::is_neighbor_established(const std::string& address) const {
    auto it = neighbors_.find(address);
    return it != neighbors_.end() && it->second.state == "Full";
}

bool OSPFProtocol::advertise_route(const RouteInfo& route) {
    std::cout << "OSPF: Advertising route " << route.destination 
              << "/" << static_cast<int>(route.prefix_length) << std::endl;
    
    // Store the route
    std::string key = route.destination + "/" + std::to_string(route.prefix_length);
    routes_[key] = route;
    
    // Send LSA UPDATE to all established neighbors
    for (auto& neighbor : neighbors_) {
        if (neighbor.second.state == "Full") {
            send_lsa_update(neighbor.second, route);
            neighbor.second.messages_sent++;
        }
    }
    
    stats_.routes_advertised++;
    return true;
}

bool OSPFProtocol::withdraw_route(const std::string& destination, uint8_t prefix_length) {
    std::cout << "OSPF: Withdrawing route " << destination 
              << "/" << static_cast<int>(prefix_length) << std::endl;
    
    // Remove the route
    std::string key = destination + "/" + std::to_string(prefix_length);
    auto it = routes_.find(key);
    if (it != routes_.end()) {
        routes_.erase(it);
        
        // Send LSA UPDATE to all established neighbors
        for (auto& neighbor : neighbors_) {
            if (neighbor.second.state == "Full") {
                RouteInfo withdraw_route;
                withdraw_route.destination = destination;
                withdraw_route.prefix_length = prefix_length;
                send_lsa_update(neighbor.second, withdraw_route);
                neighbor.second.messages_sent++;
            }
        }
        
        stats_.routes_withdrawn++;
        return true;
    }
    
    return false;
}

std::vector<RouteInfo> OSPFProtocol::get_routes() const {
    std::vector<RouteInfo> result;
    for (const auto& pair : routes_) {
        result.push_back(pair.second);
    }
    return result;
}

bool OSPFProtocol::update_config(const std::map<std::string, std::string>& config) {
    config_.insert(config.begin(), config.end());
    
    // Update router ID if specified
    auto router_id_it = config.find("router_id");
    if (router_id_it != config.end()) {
        router_id_ = router_id_it->second;
    }
    
    // Update area ID if specified
    auto area_id_it = config.find("area_id");
    if (area_id_it != config.end()) {
        area_id_ = area_id_it->second;
    }
    
    return true;
}

std::map<std::string, std::string> OSPFProtocol::get_config() const {
    return config_;
}

OSPFProtocol::ProtocolStatistics OSPFProtocol::get_statistics() const {
    return stats_;
}

void OSPFProtocol::set_route_update_callback(RouteUpdateCallback callback) {
    route_update_callback_ = callback;
}

void OSPFProtocol::set_neighbor_update_callback(NeighborUpdateCallback callback) {
    neighbor_update_callback_ = callback;
}

bool OSPFProtocol::establish_adjacency(NeighborInfo& neighbor) {
    std::cout << "OSPF: Establishing adjacency with " << neighbor.address << std::endl;
    
    // Simulate OSPF adjacency establishment
    neighbor.state = "Init";
    
    // Send HELLO message
    if (send_hello_message(neighbor)) {
        neighbor.state = "2-Way";
        neighbor.messages_sent++;
        
        // Simulate receiving HELLO message
        if (receive_hello_message(neighbor)) {
            neighbor.state = "ExStart";
            neighbor.messages_received++;
            
            // Simulate database exchange
            neighbor.state = "Exchange";
            
            // Simulate loading
            neighbor.state = "Loading";
            
            // Simulate full adjacency
            neighbor.state = "Full";
            neighbor.last_seen = std::chrono::steady_clock::now();
            
            if (neighbor_update_callback_) {
                neighbor_update_callback_(neighbor, true);
            }
            
            std::cout << "OSPF: Adjacency established with " << neighbor.address << std::endl;
            return true;
        }
    }
    
    neighbor.state = "Down";
    neighbor.last_error = "Failed to establish adjacency";
    return false;
}

bool OSPFProtocol::send_hello_message(const NeighborInfo& neighbor) {
    // Simulate sending OSPF HELLO message
    std::cout << "OSPF: Sending HELLO message to " << neighbor.address << std::endl;
    return true;
}

bool OSPFProtocol::receive_hello_message(NeighborInfo& neighbor) {
    // Simulate receiving OSPF HELLO message
    std::cout << "OSPF: Received HELLO message from " << neighbor.address << std::endl;
    return true;
}

bool OSPFProtocol::send_lsa_update(const NeighborInfo& neighbor, const RouteInfo& route) {
    // Simulate sending OSPF LSA UPDATE message
    std::cout << "OSPF: Sending LSA UPDATE to " << neighbor.address 
              << " for route " << route.destination << "/" << static_cast<int>(route.prefix_length) << std::endl;
    return true;
}

void OSPFProtocol::process_lsa_update(const NeighborInfo& neighbor, const RouteInfo& route) {
    std::cout << "OSPF: Processing LSA UPDATE from " << neighbor.address 
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
