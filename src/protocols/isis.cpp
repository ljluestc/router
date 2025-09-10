#include "protocols/isis.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <chrono>

namespace router_sim {

ISISProtocol::ISISProtocol() : running_(false), system_id_("0000.0000.0000"), area_id_("49.0001") {
    stats_.reset();
}

ISISProtocol::~ISISProtocol() {
    stop();
}

bool ISISProtocol::start(const std::map<std::string, std::string>& config) {
    if (running_) {
        return true;
    }
    
    // Parse configuration
    if (config.find("system_id") != config.end()) {
        system_id_ = config.at("system_id");
    }
    
    if (config.find("area_id") != config.end()) {
        area_id_ = config.at("area_id");
    }
    
    if (config.find("hello_interval") != config.end()) {
        hello_interval_ = std::stoul(config.at("hello_interval"));
    }
    
    if (config.find("hold_time") != config.end()) {
        hold_time_ = std::stoul(config.at("hold_time"));
    }
    
    if (config.find("lsp_interval") != config.end()) {
        lsp_interval_ = std::stoul(config.at("lsp_interval"));
    }
    
    // Initialize IS-IS state
    state_ = ISISState::DOWN;
    running_ = true;
    
    // Start IS-IS processing thread
    isis_thread_ = std::thread(&ISISProtocol::isis_processing_loop, this);
    
    std::cout << "IS-IS protocol started (System ID: " << system_id_ << ", Area: " << area_id_ << ")" << std::endl;
    return true;
}

bool ISISProtocol::stop() {
    if (!running_) {
        return true;
    }
    
    running_ = false;
    state_ = ISISState::DOWN;
    
    // Wait for processing thread to finish
    if (isis_thread_.joinable()) {
        isis_thread_.join();
    }
    
    // Clear all interfaces and routes
    interfaces_.clear();
    routes_.clear();
    
    std::cout << "IS-IS protocol stopped" << std::endl;
    return true;
}

bool ISISProtocol::is_running() const {
    return running_;
}

bool ISISProtocol::add_interface(const std::string& interface, const std::map<std::string, std::string>& config) {
    if (!running_) {
        return false;
    }
    
    ISISInterface isis_interface;
    isis_interface.name = interface;
    isis_interface.area_id = area_id_;
    isis_interface.hello_interval = hello_interval_;
    isis_interface.hold_time = hold_time_;
    isis_interface.lsp_interval = lsp_interval_;
    isis_interface.priority = 64; // Default priority
    isis_interface.cost = 10; // Default cost
    isis_interface.state = ISISInterfaceState::DOWN;
    isis_interface.last_hello = std::chrono::steady_clock::now();
    
    if (config.find("area_id") != config.end()) {
        isis_interface.area_id = config.at("area_id");
    }
    
    if (config.find("hello_interval") != config.end()) {
        isis_interface.hello_interval = std::stoul(config.at("hello_interval"));
    }
    
    if (config.find("hold_time") != config.end()) {
        isis_interface.hold_time = std::stoul(config.at("hold_time"));
    }
    
    if (config.find("lsp_interval") != config.end()) {
        isis_interface.lsp_interval = std::stoul(config.at("lsp_interval"));
    }
    
    if (config.find("priority") != config.end()) {
        isis_interface.priority = std::stoul(config.at("priority"));
    }
    
    if (config.find("cost") != config.end()) {
        isis_interface.cost = std::stoul(config.at("cost"));
    }
    
    interfaces_[interface] = isis_interface;
    
    std::cout << "IS-IS interface added: " << interface << " (Area: " << isis_interface.area_id << ")" << std::endl;
    return true;
}

bool ISISProtocol::remove_interface(const std::string& interface) {
    if (!running_) {
        return false;
    }
    
    auto it = interfaces_.find(interface);
    if (it == interfaces_.end()) {
        return false;
    }
    
    interfaces_.erase(it);
    
    std::cout << "IS-IS interface removed: " << interface << std::endl;
    return true;
}

bool ISISProtocol::advertise_route(const std::string& prefix, const std::map<std::string, std::string>& attributes) {
    if (!running_) {
        return false;
    }
    
    ISISRoute route;
    route.prefix = prefix;
    route.area_id = area_id_;
    route.type = attributes.at("type");
    route.cost = std::stoul(attributes.at("cost"));
    route.next_hop = attributes.at("next_hop");
    route.advertising_router = system_id_;
    route.last_update = std::chrono::steady_clock::now();
    route.is_active = true;
    
    routes_[prefix] = route;
    
    // Notify callback
    if (route_update_callback_) {
        RouteInfo info;
        info.prefix = prefix;
        info.next_hop = route.next_hop;
        info.protocol = "IS-IS";
        info.metric = route.cost;
        info.area_id = route.area_id;
        route_update_callback_(info, true);
    }
    
    std::cout << "IS-IS route advertised: " << prefix << " -> " << route.next_hop << " (Cost: " << route.cost << ")" << std::endl;
    return true;
}

bool ISISProtocol::withdraw_route(const std::string& prefix) {
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
        info.protocol = "IS-IS";
        info.metric = it->second.cost;
        route_update_callback_(info, false);
    }
    
    routes_.erase(it);
    
    std::cout << "IS-IS route withdrawn: " << prefix << std::endl;
    return true;
}

std::vector<RouteInfo> ISISProtocol::get_routes() const {
    std::vector<RouteInfo> route_list;
    
    for (const auto& [prefix, route] : routes_) {
        if (route.is_active) {
            RouteInfo info;
            info.prefix = prefix;
            info.next_hop = route.next_hop;
            info.protocol = "IS-IS";
            info.metric = route.cost;
            info.area_id = route.area_id;
            info.last_update = route.last_update;
            route_list.push_back(info);
        }
    }
    
    return route_list;
}

std::vector<NeighborInfo> ISISProtocol::get_neighbors() const {
    std::vector<NeighborInfo> neighbor_list;
    
    for (const auto& [interface_name, interface] : interfaces_) {
        for (const auto& [neighbor_id, neighbor] : interface.neighbors) {
            NeighborInfo info;
            info.address = neighbor_id;
            info.protocol = "IS-IS";
            info.state = neighbor_state_to_string(neighbor.state);
            info.interface = interface_name;
            info.area_id = interface.area_id;
            info.priority = neighbor.priority;
            info.last_update = neighbor.last_update;
            neighbor_list.push_back(info);
        }
    }
    
    return neighbor_list;
}

std::map<std::string, uint64_t> ISISProtocol::get_statistics() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    return {
        {"packets_sent", stats_.packets_sent},
        {"packets_received", stats_.packets_received},
        {"hello_sent", stats_.hello_sent},
        {"hello_received", stats_.hello_received},
        {"lsp_sent", stats_.lsp_sent},
        {"lsp_received", stats_.lsp_received},
        {"lsp_ack_sent", stats_.lsp_ack_sent},
        {"lsp_ack_received", stats_.lsp_ack_received},
        {"routes_advertised", stats_.routes_advertised},
        {"routes_withdrawn", stats_.routes_withdrawn},
        {"neighbors_up", stats_.neighbors_up},
        {"neighbors_down", stats_.neighbors_down}
    };
}

void ISISProtocol::set_route_update_callback(std::function<void(const RouteInfo&, bool)> callback) {
    route_update_callback_ = callback;
}

void ISISProtocol::set_neighbor_update_callback(std::function<void(const NeighborInfo&, bool)> callback) {
    neighbor_update_callback_ = callback;
}

void ISISProtocol::isis_processing_loop() {
    while (running_) {
        // Process IS-IS state machine
        process_isis_state_machine();
        
        // Send hello packets
        send_hello_packets();
        
        // Process incoming messages
        process_incoming_messages();
        
        // Check for dead neighbors
        check_dead_neighbors();
        
        // Sleep for a short time
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void ISISProtocol::process_isis_state_machine() {
    for (auto& [interface_name, interface] : interfaces_) {
        switch (interface.state) {
            case ISISInterfaceState::DOWN:
                // Interface is down, try to bring it up
                if (bring_interface_up(interface_name)) {
                    interface.state = ISISInterfaceState::UP;
                    interface.last_hello = std::chrono::steady_clock::now();
                }
                break;
                
            case ISISInterfaceState::UP:
                // Interface is up and running
                maintain_isis_interface(interface_name, interface);
                break;
        }
    }
}

void ISISProtocol::send_hello_packets() {
    auto now = std::chrono::steady_clock::now();
    
    for (auto& [interface_name, interface] : interfaces_) {
        if (interface.state != ISISInterfaceState::DOWN) {
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                now - interface.last_hello);
            
            if (elapsed.count() >= interface.hello_interval) {
                if (send_hello_packet(interface_name)) {
                    interface.last_hello = now;
                    
                    // Update statistics
                    {
                        std::lock_guard<std::mutex> lock(stats_mutex_);
                        stats_.hello_sent++;
                    }
                }
            }
        }
    }
}

void ISISProtocol::process_incoming_messages() {
    // This would process incoming IS-IS messages from the network
    // For simulation purposes, we'll just update statistics
}

void ISISProtocol::check_dead_neighbors() {
    auto now = std::chrono::steady_clock::now();
    
    for (auto& [interface_name, interface] : interfaces_) {
        for (auto& [neighbor_id, neighbor] : interface.neighbors) {
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                now - neighbor.last_update);
            
            if (elapsed.count() > interface.hold_time) {
                // Neighbor is dead
                neighbor.state = ISISNeighborState::DOWN;
                
                // Update statistics
                {
                    std::lock_guard<std::mutex> lock(stats_mutex_);
                    stats_.neighbors_down++;
                }
                
                // Notify callback
                if (neighbor_update_callback_) {
                    NeighborInfo info;
                    info.address = neighbor_id;
                    info.protocol = "IS-IS";
                    info.state = "DOWN";
                    info.interface = interface_name;
                    info.area_id = interface.area_id;
                    neighbor_update_callback_(info, false);
                }
            }
        }
    }
}

bool ISISProtocol::bring_interface_up(const std::string& interface) {
    // Simulate bringing interface up
    // In a real implementation, this would configure the network interface
    return true;
}

void ISISProtocol::maintain_isis_interface(const std::string& interface_name, ISISInterface& interface) {
    // Maintain IS-IS interface state
    // This would include sending LSPs, processing LSPs, etc.
}

bool ISISProtocol::send_hello_packet(const std::string& interface) {
    // Simulate sending IS-IS hello packet
    // In a real implementation, this would send actual IS-IS packets
    return true;
}

std::string ISISProtocol::neighbor_state_to_string(ISISNeighborState state) const {
    switch (state) {
        case ISISNeighborState::DOWN: return "DOWN";
        case ISISNeighborState::INIT: return "INIT";
        case ISISNeighborState::UP: return "UP";
        default: return "UNKNOWN";
    }
}

} // namespace router_sim