#include "protocols/ospf.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <chrono>

namespace router_sim {

OSPFProtocol::OSPFProtocol() : running_(false), router_id_("0.0.0.0"), area_id_("0.0.0.0") {
    stats_.reset();
}

OSPFProtocol::~OSPFProtocol() {
    stop();
}

bool OSPFProtocol::start(const std::map<std::string, std::string>& config) {
    if (running_) {
        return true;
    }
    
    // Parse configuration
    if (config.find("router_id") != config.end()) {
        router_id_ = config.at("router_id");
    }
    
    if (config.find("area_id") != config.end()) {
        area_id_ = config.at("area_id");
    }
    
    if (config.find("hello_interval") != config.end()) {
        hello_interval_ = std::stoul(config.at("hello_interval"));
    }
    
    if (config.find("dead_interval") != config.end()) {
        dead_interval_ = std::stoul(config.at("dead_interval"));
    }
    
    if (config.find("retransmit_interval") != config.end()) {
        retransmit_interval_ = std::stoul(config.at("retransmit_interval"));
    }
    
    // Initialize OSPF state
    state_ = OSPFState::DOWN;
    running_ = true;
    
    // Start OSPF processing thread
    ospf_thread_ = std::thread(&OSPFProtocol::ospf_processing_loop, this);
    
    std::cout << "OSPF protocol started (Router ID: " << router_id_ << ", Area: " << area_id_ << ")" << std::endl;
    return true;
}

bool OSPFProtocol::stop() {
    if (!running_) {
        return true;
    }
    
    running_ = false;
    state_ = OSPFState::DOWN;
    
    // Wait for processing thread to finish
    if (ospf_thread_.joinable()) {
        ospf_thread_.join();
    }
    
    // Clear all interfaces and routes
    interfaces_.clear();
    routes_.clear();
    
    std::cout << "OSPF protocol stopped" << std::endl;
    return true;
}

bool OSPFProtocol::is_running() const {
    return running_;
}

bool OSPFProtocol::add_interface(const std::string& interface, const std::map<std::string, std::string>& config) {
    if (!running_) {
        return false;
    }
    
    OSPFInterface ospf_interface;
    ospf_interface.name = interface;
    ospf_interface.area_id = area_id_;
    ospf_interface.hello_interval = hello_interval_;
    ospf_interface.dead_interval = dead_interval_;
    ospf_interface.retransmit_interval = retransmit_interval_;
    ospf_interface.priority = 1; // Default priority
    ospf_interface.cost = 1; // Default cost
    ospf_interface.state = OSPFInterfaceState::DOWN;
    ospf_interface.last_hello = std::chrono::steady_clock::now();
    
    if (config.find("area_id") != config.end()) {
        ospf_interface.area_id = config.at("area_id");
    }
    
    if (config.find("hello_interval") != config.end()) {
        ospf_interface.hello_interval = std::stoul(config.at("hello_interval"));
    }
    
    if (config.find("dead_interval") != config.end()) {
        ospf_interface.dead_interval = std::stoul(config.at("dead_interval"));
    }
    
    if (config.find("retransmit_interval") != config.end()) {
        ospf_interface.retransmit_interval = std::stoul(config.at("retransmit_interval"));
    }
    
    if (config.find("priority") != config.end()) {
        ospf_interface.priority = std::stoul(config.at("priority"));
    }
    
    if (config.find("cost") != config.end()) {
        ospf_interface.cost = std::stoul(config.at("cost"));
    }
    
    interfaces_[interface] = ospf_interface;
    
    std::cout << "OSPF interface added: " << interface << " (Area: " << ospf_interface.area_id << ")" << std::endl;
    return true;
}

bool OSPFProtocol::remove_interface(const std::string& interface) {
    if (!running_) {
        return false;
    }
    
    auto it = interfaces_.find(interface);
    if (it == interfaces_.end()) {
        return false;
    }
    
    interfaces_.erase(it);
    
    std::cout << "OSPF interface removed: " << interface << std::endl;
    return true;
}

bool OSPFProtocol::advertise_route(const std::string& prefix, const std::map<std::string, std::string>& attributes) {
    if (!running_) {
        return false;
    }
    
    OSPFRoute route;
    route.prefix = prefix;
    route.area_id = area_id_;
    route.type = attributes.at("type");
    route.cost = std::stoul(attributes.at("cost"));
    route.next_hop = attributes.at("next_hop");
    route.advertising_router = router_id_;
    route.last_update = std::chrono::steady_clock::now();
    route.is_active = true;
    
    routes_[prefix] = route;
    
    // Notify callback
    if (route_update_callback_) {
        RouteInfo info;
        info.prefix = prefix;
        info.next_hop = route.next_hop;
        info.protocol = "OSPF";
        info.metric = route.cost;
        info.area_id = route.area_id;
        route_update_callback_(info, true);
    }
    
    std::cout << "OSPF route advertised: " << prefix << " -> " << route.next_hop << " (Cost: " << route.cost << ")" << std::endl;
    return true;
}

bool OSPFProtocol::withdraw_route(const std::string& prefix) {
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
        info.protocol = "OSPF";
        info.metric = it->second.cost;
        route_update_callback_(info, false);
    }
    
    routes_.erase(it);
    
    std::cout << "OSPF route withdrawn: " << prefix << std::endl;
    return true;
}

std::vector<RouteInfo> OSPFProtocol::get_routes() const {
    std::vector<RouteInfo> route_list;
    
    for (const auto& [prefix, route] : routes_) {
        if (route.is_active) {
            RouteInfo info;
            info.prefix = prefix;
            info.next_hop = route.next_hop;
            info.protocol = "OSPF";
            info.metric = route.cost;
            info.area_id = route.area_id;
            info.last_update = route.last_update;
            route_list.push_back(info);
        }
    }
    
    return route_list;
}

std::vector<NeighborInfo> OSPFProtocol::get_neighbors() const {
    std::vector<NeighborInfo> neighbor_list;
    
    for (const auto& [interface_name, interface] : interfaces_) {
        for (const auto& [neighbor_id, neighbor] : interface.neighbors) {
            NeighborInfo info;
            info.address = neighbor_id;
            info.protocol = "OSPF";
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

std::map<std::string, uint64_t> OSPFProtocol::get_statistics() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    return {
        {"packets_sent", stats_.packets_sent},
        {"packets_received", stats_.packets_received},
        {"hello_sent", stats_.hello_sent},
        {"hello_received", stats_.hello_received},
        {"lsa_sent", stats_.lsa_sent},
        {"lsa_received", stats_.lsa_received},
        {"lsa_ack_sent", stats_.lsa_ack_sent},
        {"lsa_ack_received", stats_.lsa_ack_received},
        {"routes_advertised", stats_.routes_advertised},
        {"routes_withdrawn", stats_.routes_withdrawn},
        {"neighbors_up", stats_.neighbors_up},
        {"neighbors_down", stats_.neighbors_down}
    };
}

void OSPFProtocol::set_route_update_callback(std::function<void(const RouteInfo&, bool)> callback) {
    route_update_callback_ = callback;
}

void OSPFProtocol::set_neighbor_update_callback(std::function<void(const NeighborInfo&, bool)> callback) {
    neighbor_update_callback_ = callback;
}

void OSPFProtocol::ospf_processing_loop() {
    while (running_) {
        // Process OSPF state machine
        process_ospf_state_machine();
        
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

void OSPFProtocol::process_ospf_state_machine() {
    for (auto& [interface_name, interface] : interfaces_) {
        switch (interface.state) {
            case OSPFInterfaceState::DOWN:
                // Interface is down, try to bring it up
                if (bring_interface_up(interface_name)) {
                    interface.state = OSPFInterfaceState::WAITING;
                    interface.last_hello = std::chrono::steady_clock::now();
                }
                break;
                
            case OSPFInterfaceState::WAITING:
                // Wait for DR/BDR election
                if (perform_dr_bdr_election(interface_name)) {
                    interface.state = OSPFInterfaceState::DR_OTHER;
                    interface.last_hello = std::chrono::steady_clock::now();
                }
                break;
                
            case OSPFInterfaceState::DR_OTHER:
            case OSPFInterfaceState::DR:
            case OSPFInterfaceState::BDR:
                // Interface is up and running
                maintain_ospf_interface(interface_name, interface);
                break;
        }
    }
}

void OSPFProtocol::send_hello_packets() {
    auto now = std::chrono::steady_clock::now();
    
    for (auto& [interface_name, interface] : interfaces_) {
        if (interface.state != OSPFInterfaceState::DOWN) {
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

void OSPFProtocol::process_incoming_messages() {
    // This would process incoming OSPF messages from the network
    // For simulation purposes, we'll just update statistics
}

void OSPFProtocol::check_dead_neighbors() {
    auto now = std::chrono::steady_clock::now();
    
    for (auto& [interface_name, interface] : interfaces_) {
        for (auto& [neighbor_id, neighbor] : interface.neighbors) {
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                now - neighbor.last_update);
            
            if (elapsed.count() > interface.dead_interval) {
                // Neighbor is dead
                neighbor.state = OSPFNeighborState::DOWN;
                
                // Update statistics
                {
                    std::lock_guard<std::mutex> lock(stats_mutex_);
                    stats_.neighbors_down++;
                }
                
                // Notify callback
                if (neighbor_update_callback_) {
                    NeighborInfo info;
                    info.address = neighbor_id;
                    info.protocol = "OSPF";
                    info.state = "DOWN";
                    info.interface = interface_name;
                    info.area_id = interface.area_id;
                    neighbor_update_callback_(info, false);
                }
            }
        }
    }
}

bool OSPFProtocol::bring_interface_up(const std::string& interface) {
    // Simulate bringing interface up
    // In a real implementation, this would configure the network interface
    return true;
}

bool OSPFProtocol::perform_dr_bdr_election(const std::string& interface) {
    // Simulate DR/BDR election
    // In a real implementation, this would perform actual OSPF DR/BDR election
    return true;
}

void OSPFProtocol::maintain_ospf_interface(const std::string& interface_name, OSPFInterface& interface) {
    // Maintain OSPF interface state
    // This would include sending LSAs, processing LSAs, etc.
}

bool OSPFProtocol::send_hello_packet(const std::string& interface) {
    // Simulate sending OSPF hello packet
    // In a real implementation, this would send actual OSPF packets
    return true;
}

std::string OSPFProtocol::neighbor_state_to_string(OSPFNeighborState state) const {
    switch (state) {
        case OSPFNeighborState::DOWN: return "DOWN";
        case OSPFNeighborState::ATTEMPT: return "ATTEMPT";
        case OSPFNeighborState::INIT: return "INIT";
        case OSPFNeighborState::TWO_WAY: return "2-WAY";
        case OSPFNeighborState::EXSTART: return "EXSTART";
        case OSPFNeighborState::EXCHANGE: return "EXCHANGE";
        case OSPFNeighborState::LOADING: return "LOADING";
        case OSPFNeighborState::FULL: return "FULL";
        default: return "UNKNOWN";
    }
}

} // namespace router_sim
