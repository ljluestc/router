#include "protocols/bgp.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <chrono>

namespace router_sim {

BGPProtocol::BGPProtocol() : running_(false), as_number_(0), router_id_("0.0.0.0") {
    stats_.reset();
}

BGPProtocol::~BGPProtocol() {
    stop();
}

bool BGPProtocol::start(const std::map<std::string, std::string>& config) {
    if (running_) {
        return true;
    }
    
    // Parse configuration
    if (config.find("as_number") != config.end()) {
        as_number_ = std::stoul(config.at("as_number"));
    }
    
    if (config.find("router_id") != config.end()) {
        router_id_ = config.at("router_id");
    }
    
    if (config.find("listen_port") != config.end()) {
        listen_port_ = std::stoul(config.at("listen_port"));
    }
    
    // Initialize BGP state
    state_ = BGPState::IDLE;
    running_ = true;
    
    // Start BGP processing thread
    bgp_thread_ = std::thread(&BGPProtocol::bgp_processing_loop, this);
    
    std::cout << "BGP protocol started (AS: " << as_number_ << ", Router ID: " << router_id_ << ")" << std::endl;
    return true;
}

bool BGPProtocol::stop() {
    if (!running_) {
        return true;
    }
    
    running_ = false;
    state_ = BGPState::IDLE;
    
    // Wait for processing thread to finish
    if (bgp_thread_.joinable()) {
        bgp_thread_.join();
    }
    
    // Clear all neighbors and routes
    neighbors_.clear();
    routes_.clear();
    
    std::cout << "BGP protocol stopped" << std::endl;
    return true;
}

bool BGPProtocol::is_running() const {
    return running_;
}

bool BGPProtocol::add_neighbor(const std::string& address, const std::map<std::string, std::string>& config) {
    if (!running_) {
        return false;
    }
    
    BGPNeighbor neighbor;
    neighbor.address = address;
    neighbor.as_number = std::stoul(config.at("as_number"));
    neighbor.state = BGPNeighborState::IDLE;
    neighbor.hold_time = 180; // Default hold time
    neighbor.keepalive_time = 60; // Default keepalive time
    neighbor.connect_retry_time = 120; // Default connect retry time
    neighbor.last_update = std::chrono::steady_clock::now();
    
    if (config.find("hold_time") != config.end()) {
        neighbor.hold_time = std::stoul(config.at("hold_time"));
    }
    
    if (config.find("keepalive_time") != config.end()) {
        neighbor.keepalive_time = std::stoul(config.at("keepalive_time"));
    }
    
    if (config.find("connect_retry_time") != config.end()) {
        neighbor.connect_retry_time = std::stoul(config.at("connect_retry_time"));
    }
    
    neighbors_[address] = neighbor;
    
    // Notify callback
    if (neighbor_update_callback_) {
        NeighborInfo info;
        info.address = address;
        info.protocol = "BGP";
        info.state = "IDLE";
        info.as_number = neighbor.as_number;
        neighbor_update_callback_(info, true);
    }
    
    std::cout << "BGP neighbor added: " << address << " (AS: " << neighbor.as_number << ")" << std::endl;
    return true;
}

bool BGPProtocol::remove_neighbor(const std::string& address) {
    if (!running_) {
        return false;
    }
    
    auto it = neighbors_.find(address);
    if (it == neighbors_.end()) {
        return false;
    }
    
    // Notify callback
    if (neighbor_update_callback_) {
        NeighborInfo info;
        info.address = address;
        info.protocol = "BGP";
        info.state = "DOWN";
        neighbor_update_callback_(info, false);
    }
    
    neighbors_.erase(it);
    
    std::cout << "BGP neighbor removed: " << address << std::endl;
    return true;
}

bool BGPProtocol::advertise_route(const std::string& prefix, const std::map<std::string, std::string>& attributes) {
    if (!running_) {
        return false;
    }
    
    BGPRoute route;
    route.prefix = prefix;
    route.next_hop = attributes.at("next_hop");
    route.as_path = attributes.at("as_path");
    route.origin = attributes.at("origin");
    route.local_pref = std::stoul(attributes.at("local_pref"));
    route.med = std::stoul(attributes.at("med"));
    route.community = attributes.at("community");
    route.last_update = std::chrono::steady_clock::now();
    route.is_active = true;
    
    routes_[prefix] = route;
    
    // Notify callback
    if (route_update_callback_) {
        RouteInfo info;
        info.prefix = prefix;
        info.next_hop = route.next_hop;
        info.protocol = "BGP";
        info.metric = route.local_pref;
        info.as_path = route.as_path;
        route_update_callback_(info, true);
    }
    
    std::cout << "BGP route advertised: " << prefix << " -> " << route.next_hop << std::endl;
    return true;
}

bool BGPProtocol::withdraw_route(const std::string& prefix) {
    if (!running_) {
        return false;
    }
    
    auto it = routes_.find(prefix);
    if (it == routes_.end()) {
        return false;
    }
    
    // Notify callback
    if (route_update_callback_) {
        RouteInfo info;
        info.prefix = prefix;
        info.next_hop = it->second.next_hop;
        info.protocol = "BGP";
        info.metric = it->second.local_pref;
        route_update_callback_(info, false);
    }
    
    routes_.erase(it);
    
    std::cout << "BGP route withdrawn: " << prefix << std::endl;
    return true;
}

std::vector<RouteInfo> BGPProtocol::get_routes() const {
    std::vector<RouteInfo> route_list;
    
    for (const auto& [prefix, route] : routes_) {
        if (route.is_active) {
            RouteInfo info;
            info.prefix = prefix;
            info.next_hop = route.next_hop;
            info.protocol = "BGP";
            info.metric = route.local_pref;
            info.as_path = route.as_path;
            info.last_update = route.last_update;
            route_list.push_back(info);
        }
    }
    
    return route_list;
}

std::vector<NeighborInfo> BGPProtocol::get_neighbors() const {
    std::vector<NeighborInfo> neighbor_list;
    
    for (const auto& [address, neighbor] : neighbors_) {
        NeighborInfo info;
        info.address = address;
        info.protocol = "BGP";
        info.state = neighbor_state_to_string(neighbor.state);
        info.as_number = neighbor.as_number;
        info.hold_time = neighbor.hold_time;
        info.keepalive_time = neighbor.keepalive_time;
        info.last_update = neighbor.last_update;
        neighbor_list.push_back(info);
    }
    
    return neighbor_list;
}

std::map<std::string, uint64_t> BGPProtocol::get_statistics() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    return {
        {"packets_sent", stats_.packets_sent},
        {"packets_received", stats_.packets_received},
        {"updates_sent", stats_.updates_sent},
        {"updates_received", stats_.updates_received},
        {"keepalives_sent", stats_.keepalives_sent},
        {"keepalives_received", stats_.keepalives_received},
        {"notifications_sent", stats_.notifications_sent},
        {"notifications_received", stats_.notifications_received},
        {"routes_advertised", stats_.routes_advertised},
        {"routes_withdrawn", stats_.routes_withdrawn},
        {"neighbors_up", stats_.neighbors_up},
        {"neighbors_down", stats_.neighbors_down}
    };
}

void BGPProtocol::set_route_update_callback(std::function<void(const RouteInfo&, bool)> callback) {
    route_update_callback_ = callback;
}

void BGPProtocol::set_neighbor_update_callback(std::function<void(const NeighborInfo&, bool)> callback) {
    neighbor_update_callback_ = callback;
}

void BGPProtocol::bgp_processing_loop() {
    while (running_) {
        // Process BGP state machine
        process_bgp_state_machine();
        
        // Send keepalives to established neighbors
        send_keepalives();
        
        // Process incoming messages
        process_incoming_messages();
        
        // Sleep for a short time
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void BGPProtocol::process_bgp_state_machine() {
    for (auto& [address, neighbor] : neighbors_) {
        switch (neighbor.state) {
            case BGPNeighborState::IDLE:
                // Try to establish TCP connection
                if (establish_tcp_connection(address)) {
                    neighbor.state = BGPNeighborState::CONNECT;
                    neighbor.last_update = std::chrono::steady_clock::now();
                }
                break;
                
            case BGPNeighborState::CONNECT:
                // Wait for TCP connection to be established
                if (is_tcp_connected(address)) {
                    neighbor.state = BGPNeighborState::OPENSENT;
                    neighbor.last_update = std::chrono::steady_clock::now();
                } else {
                    // Check for timeout
                    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                        std::chrono::steady_clock::now() - neighbor.last_update);
                    if (elapsed.count() > neighbor.connect_retry_time) {
                        neighbor.state = BGPNeighborState::IDLE;
                        neighbor.last_update = std::chrono::steady_clock::now();
                    }
                }
                break;
                
            case BGPNeighborState::OPENSENT:
                // Send OPEN message and wait for response
                if (send_open_message(address)) {
                    neighbor.state = BGPNeighborState::OPENCONFIRM;
                    neighbor.last_update = std::chrono::steady_clock::now();
                }
                break;
                
            case BGPNeighborState::OPENCONFIRM:
                // Wait for keepalive message
                if (receive_keepalive(address)) {
                    neighbor.state = BGPNeighborState::ESTABLISHED;
                    neighbor.last_update = std::chrono::steady_clock::now();
                    
                    // Update statistics
                    {
                        std::lock_guard<std::mutex> lock(stats_mutex_);
                        stats_.neighbors_up++;
                    }
                    
                    // Notify callback
                    if (neighbor_update_callback_) {
                        NeighborInfo info;
                        info.address = address;
                        info.protocol = "BGP";
                        info.state = "ESTABLISHED";
                        info.as_number = neighbor.as_number;
                        neighbor_update_callback_(info, true);
                    }
                }
                break;
                
            case BGPNeighborState::ESTABLISHED:
                // Maintain established session
                maintain_bgp_session(address, neighbor);
                break;
        }
    }
}

void BGPProtocol::send_keepalives() {
    auto now = std::chrono::steady_clock::now();
    
    for (auto& [address, neighbor] : neighbors_) {
        if (neighbor.state == BGPNeighborState::ESTABLISHED) {
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                now - neighbor.last_keepalive);
            
            if (elapsed.count() >= neighbor.keepalive_time) {
                if (send_keepalive_message(address)) {
                    neighbor.last_keepalive = now;
                    
                    // Update statistics
                    {
                        std::lock_guard<std::mutex> lock(stats_mutex_);
                        stats_.keepalives_sent++;
                    }
                }
            }
        }
    }
}

void BGPProtocol::process_incoming_messages() {
    // This would process incoming BGP messages from the network
    // For simulation purposes, we'll just update statistics
}

bool BGPProtocol::establish_tcp_connection(const std::string& address) {
    // Simulate TCP connection establishment
    // In a real implementation, this would create a TCP socket
    return true;
}

bool BGPProtocol::is_tcp_connected(const std::string& address) {
    // Simulate TCP connection check
    // In a real implementation, this would check socket state
    return true;
}

bool BGPProtocol::send_open_message(const std::string& address) {
    // Simulate sending BGP OPEN message
    // In a real implementation, this would send actual BGP packets
    return true;
}

bool BGPProtocol::receive_keepalive(const std::string& address) {
    // Simulate receiving BGP keepalive message
    // In a real implementation, this would receive actual BGP packets
    return true;
}

bool BGPProtocol::send_keepalive_message(const std::string& address) {
    // Simulate sending BGP keepalive message
    // In a real implementation, this would send actual BGP packets
    return true;
}

void BGPProtocol::maintain_bgp_session(const std::string& address, BGPNeighbor& neighbor) {
    // Check for hold timer expiration
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
        now - neighbor.last_update);
    
    if (elapsed.count() > neighbor.hold_time) {
        // Hold timer expired, close connection
        neighbor.state = BGPNeighborState::IDLE;
        neighbor.last_update = now;
        
        // Update statistics
        {
            std::lock_guard<std::mutex> lock(stats_mutex_);
            stats_.neighbors_down++;
        }
        
        // Notify callback
        if (neighbor_update_callback_) {
            NeighborInfo info;
            info.address = address;
            info.protocol = "BGP";
            info.state = "DOWN";
            info.as_number = neighbor.as_number;
            neighbor_update_callback_(info, false);
        }
    }
}

std::string BGPProtocol::neighbor_state_to_string(BGPNeighborState state) const {
    switch (state) {
        case BGPNeighborState::IDLE: return "IDLE";
        case BGPNeighborState::CONNECT: return "CONNECT";
        case BGPNeighborState::ACTIVE: return "ACTIVE";
        case BGPNeighborState::OPENSENT: return "OPENSENT";
        case BGPNeighborState::OPENCONFIRM: return "OPENCONFIRM";
        case BGPNeighborState::ESTABLISHED: return "ESTABLISHED";
        default: return "UNKNOWN";
    }
}

} // namespace router_sim