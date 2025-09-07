#include "router_core.h"
#include <iostream>
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace RouterSim {

RouterCore::RouterCore() : running_(false) {
    stats_ = {};
}

RouterCore::~RouterCore() {
    stop();
}

bool RouterCore::initialize() {
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    // Initialize default interfaces
    Interface lo = {"lo", "127.0.0.1", "255.0.0.0", true, 65536, 0, 0, 0, 0};
    interfaces_["lo"] = lo;
    
    return true;
}

void RouterCore::start() {
    if (running_) {
        return;
    }
    
    running_ = true;
    
    packet_thread_ = std::thread(&RouterCore::packet_processing_loop, this);
    route_thread_ = std::thread(&RouterCore::route_update_loop, this);
    interface_thread_ = std::thread(&RouterCore::interface_monitoring_loop, this);
    
    std::cout << "Router core started" << std::endl;
}

void RouterCore::stop() {
    if (!running_) {
        return;
    }
    
    running_ = false;
    
    if (packet_thread_.joinable()) {
        packet_thread_.join();
    }
    if (route_thread_.joinable()) {
        route_thread_.join();
    }
    if (interface_thread_.joinable()) {
        interface_thread_.join();
    }
    
    std::cout << "Router core stopped" << std::endl;
}

bool RouterCore::is_running() const {
    return running_;
}

bool RouterCore::add_interface(const std::string& name, const std::string& ip, const std::string& mask) {
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    Interface iface;
    iface.name = name;
    iface.ip_address = ip;
    iface.subnet_mask = mask;
    iface.is_up = false;
    iface.mtu = 1500;
    iface.bytes_sent = 0;
    iface.bytes_received = 0;
    iface.packets_sent = 0;
    iface.packets_received = 0;
    
    interfaces_[name] = iface;
    
    {
        std::lock_guard<std::mutex> stats_lock(stats_mutex_);
        stats_.interface_state_changes++;
    }
    
    return true;
}

bool RouterCore::remove_interface(const std::string& name) {
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    auto it = interfaces_.find(name);
    if (it == interfaces_.end()) {
        return false;
    }
    
    interfaces_.erase(it);
    
    {
        std::lock_guard<std::mutex> stats_lock(stats_mutex_);
        stats_.interface_state_changes++;
    }
    
    return true;
}

bool RouterCore::set_interface_up(const std::string& name, bool up) {
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    auto it = interfaces_.find(name);
    if (it == interfaces_.end()) {
        return false;
    }
    
    it->second.is_up = up;
    
    {
        std::lock_guard<std::mutex> stats_lock(stats_mutex_);
        stats_.interface_state_changes++;
    }
    
    return true;
}

std::vector<Interface> RouterCore::get_interfaces() const {
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    std::vector<Interface> result;
    for (const auto& pair : interfaces_) {
        result.push_back(pair.second);
    }
    
    return result;
}

bool RouterCore::add_route(const Route& route) {
    std::lock_guard<std::mutex> lock(routes_mutex_);
    
    // Check if route already exists
    auto it = std::find_if(routes_.begin(), routes_.end(),
        [&route](const Route& r) {
            return r.network == route.network && r.next_hop == route.next_hop;
        });
    
    if (it != routes_.end()) {
        *it = route; // Update existing route
    } else {
        routes_.push_back(route);
    }
    
    {
        std::lock_guard<std::mutex> stats_lock(stats_mutex_);
        stats_.routing_table_updates++;
    }
    
    return true;
}

bool RouterCore::remove_route(const std::string& network) {
    std::lock_guard<std::mutex> lock(routes_mutex_);
    
    auto it = std::find_if(routes_.begin(), routes_.end(),
        [&network](const Route& r) {
            return r.network == network;
        });
    
    if (it != routes_.end()) {
        routes_.erase(it);
        
        {
            std::lock_guard<std::mutex> stats_lock(stats_mutex_);
            stats_.routing_table_updates++;
        }
        
        return true;
    }
    
    return false;
}

std::vector<Route> RouterCore::get_routes() const {
    std::lock_guard<std::mutex> lock(routes_mutex_);
    return routes_;
}

Route* RouterCore::find_route(const std::string& destination) {
    std::lock_guard<std::mutex> lock(routes_mutex_);
    
    // Simple longest prefix match (simplified)
    Route* best_route = nullptr;
    size_t best_prefix_length = 0;
    
    for (auto& route : routes_) {
        if (!route.is_active) continue;
        
        // Check if destination matches this route's network
        if (destination.find(route.network) == 0) {
            if (route.network.length() > best_prefix_length) {
                best_prefix_length = route.network.length();
                best_route = &route;
            }
        }
    }
    
    return best_route;
}

void RouterCore::process_packet(const Packet& packet) {
    {
        std::lock_guard<std::mutex> lock(packet_queue_mutex_);
        packet_queue_.push(packet);
    }
    
    {
        std::lock_guard<std::mutex> stats_lock(stats_mutex_);
        stats_.total_packets_processed++;
        stats_.total_bytes_processed += packet.size;
    }
}

void RouterCore::send_packet(const Packet& packet, const std::string& interface) {
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    auto it = interfaces_.find(interface);
    if (it != interfaces_.end() && it->second.is_up) {
        it->second.bytes_sent += packet.size;
        it->second.packets_sent++;
        
        // In a real implementation, this would send the packet to the network
        std::cout << "Sending packet of " << packet.size << " bytes on interface " << interface << std::endl;
    }
}

RouterCore::Statistics RouterCore::get_statistics() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    return stats_;
}

void RouterCore::packet_processing_loop() {
    while (running_) {
        std::queue<Packet> local_queue;
        
        {
            std::lock_guard<std::mutex> lock(packet_queue_mutex_);
            if (!packet_queue_.empty()) {
                local_queue = std::move(packet_queue_);
            }
        }
        
        while (!local_queue.empty()) {
            const Packet& packet = local_queue.front();
            
            // Process packet (simplified)
            if (packet_handler_) {
                packet_handler_(packet);
            }
            
            local_queue.pop();
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void RouterCore::route_update_loop() {
    while (running_) {
        // Route update logic would go here
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void RouterCore::interface_monitoring_loop() {
    while (running_) {
        // Interface monitoring logic would go here
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}

} // namespace RouterSim
