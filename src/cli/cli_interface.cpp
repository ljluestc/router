#include "router_core.h"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <memory>

namespace RouterSim {

class CLIInterface {
public:
    CLIInterface() : router_core_(nullptr) {}
    
    void set_router_core(RouterCore* router) {
        router_core_ = router;
    }
    
    void start() {
        std::cout << "Router Simulator CLI - Type 'help' for commands" << std::endl;
        
        std::string line;
        while (std::getline(std::cin, line)) {
            if (line.empty()) {
                continue;
            }
            
            if (line == "quit" || line == "exit") {
                break;
            }
            
            process_command(line);
        }
    }
    
    void process_command(const std::string& command) {
        std::istringstream iss(command);
        std::vector<std::string> tokens;
        std::string token;
        
        while (iss >> token) {
            tokens.push_back(token);
        }
        
        if (tokens.empty()) {
            return;
        }
        
        std::string cmd = tokens[0];
        
        if (cmd == "help") {
            show_help();
        } else if (cmd == "status") {
            show_status();
        } else if (cmd == "interfaces") {
            show_interfaces();
        } else if (cmd == "routes") {
            show_routes();
        } else if (cmd == "protocols") {
            show_protocols();
        } else if (cmd == "statistics") {
            show_statistics();
        } else if (cmd == "add_interface") {
            add_interface(tokens);
        } else if (cmd == "remove_interface") {
            remove_interface(tokens);
        } else if (cmd == "add_route") {
            add_route(tokens);
        } else if (cmd == "remove_route") {
            remove_route(tokens);
        } else if (cmd == "start_protocol") {
            start_protocol(tokens);
        } else if (cmd == "stop_protocol") {
            stop_protocol(tokens);
        } else {
            std::cout << "Unknown command: " << cmd << std::endl;
            std::cout << "Type 'help' for available commands" << std::endl;
        }
    }
    
    void show_help() {
        std::cout << "Available commands:" << std::endl;
        std::cout << "  help                    - Show this help message" << std::endl;
        std::cout << "  status                  - Show router status" << std::endl;
        std::cout << "  interfaces              - Show all interfaces" << std::endl;
        std::cout << "  routes                  - Show routing table" << std::endl;
        std::cout << "  protocols               - Show running protocols" << std::endl;
        std::cout << "  statistics              - Show router statistics" << std::endl;
        std::cout << "  add_interface <name> <ip> <mask> - Add interface" << std::endl;
        std::cout << "  remove_interface <name>  - Remove interface" << std::endl;
        std::cout << "  add_route <dest> <mask> <next_hop> <interface> - Add route" << std::endl;
        std::cout << "  remove_route <dest> <mask> - Remove route" << std::endl;
        std::cout << "  start_protocol <name>    - Start protocol" << std::endl;
        std::cout << "  stop_protocol <name>     - Stop protocol" << std::endl;
        std::cout << "  quit/exit               - Exit CLI" << std::endl;
    }
    
    void show_status() {
        if (!router_core_) {
            std::cout << "Router core not initialized" << std::endl;
            return;
        }
        
        std::cout << "Router Status:" << std::endl;
        std::cout << "  Running: " << (router_core_->is_running() ? "Yes" : "No") << std::endl;
        
        auto interfaces = router_core_->get_interfaces();
        std::cout << "  Interfaces: " << interfaces.size() << std::endl;
        
        auto routes = router_core_->get_routes();
        std::cout << "  Routes: " << routes.size() << std::endl;
    }
    
    void show_interfaces() {
        if (!router_core_) {
            std::cout << "Router core not initialized" << std::endl;
            return;
        }
        
        auto interfaces = router_core_->get_interfaces();
        
        std::cout << "Interfaces:" << std::endl;
        std::cout << "  Name\t\tIP Address\t\tStatus" << std::endl;
        std::cout << "  ----\t\t----------\t\t------" << std::endl;
        
        for (const auto& iface : interfaces) {
            std::cout << "  " << iface.name << "\t\t" 
                      << ip_to_string(iface.ip_address) << "\t\t"
                      << (iface.is_up ? "Up" : "Down") << std::endl;
        }
    }
    
    void show_routes() {
        if (!router_core_) {
            std::cout << "Router core not initialized" << std::endl;
            return;
        }
        
        auto routes = router_core_->get_routes();
        
        std::cout << "Routing Table:" << std::endl;
        std::cout << "  Destination\t\tNext Hop\t\tInterface\tMetric" << std::endl;
        std::cout << "  -----------\t\t--------\t\t---------\t------" << std::endl;
        
        for (const auto& route : routes) {
            std::cout << "  " << ip_to_string(route.network) << "/" << cidr_to_string(route.mask)
                      << "\t" << ip_to_string(route.next_hop)
                      << "\t" << route.interface
                      << "\t" << route.metric << std::endl;
        }
    }
    
    void show_protocols() {
        std::cout << "Protocols:" << std::endl;
        std::cout << "  BGP: Running" << std::endl;
        std::cout << "  OSPF: Running" << std::endl;
        std::cout << "  ISIS: Stopped" << std::endl;
    }
    
    void show_statistics() {
        if (!router_core_) {
            std::cout << "Router core not initialized" << std::endl;
            return;
        }
        
        auto stats = router_core_->get_statistics();
        
        std::cout << "Router Statistics:" << std::endl;
        std::cout << "  Packets Received: " << stats.packets_received << std::endl;
        std::cout << "  Packets Sent: " << stats.packets_sent << std::endl;
        std::cout << "  Packets Dropped: " << stats.packets_dropped << std::endl;
        std::cout << "  Bytes Received: " << stats.bytes_received << std::endl;
        std::cout << "  Bytes Sent: " << stats.bytes_sent << std::endl;
        std::cout << "  Routing Updates: " << stats.routing_updates << std::endl;
    }
    
    void add_interface(const std::vector<std::string>& tokens) {
        if (tokens.size() < 4) {
            std::cout << "Usage: add_interface <name> <ip> <mask>" << std::endl;
            return;
        }
        
        if (!router_core_) {
            std::cout << "Router core not initialized" << std::endl;
            return;
        }
        
        Interface iface;
        iface.name = tokens[1];
        iface.ip_address = string_to_ip(tokens[2]);
        iface.subnet_mask = string_to_ip(tokens[3]);
        iface.is_up = true;
        iface.mtu = 1500;
        iface.bandwidth = 1000000000; // 1 Gbps
        
        if (router_core_->add_interface(iface)) {
            std::cout << "Interface " << iface.name << " added successfully" << std::endl;
        } else {
            std::cout << "Failed to add interface " << iface.name << std::endl;
        }
    }
    
    void remove_interface(const std::vector<std::string>& tokens) {
        if (tokens.size() < 2) {
            std::cout << "Usage: remove_interface <name>" << std::endl;
            return;
        }
        
        if (!router_core_) {
            std::cout << "Router core not initialized" << std::endl;
            return;
        }
        
        if (router_core_->remove_interface(tokens[1])) {
            std::cout << "Interface " << tokens[1] << " removed successfully" << std::endl;
        } else {
            std::cout << "Failed to remove interface " << tokens[1] << std::endl;
        }
    }
    
    void add_route(const std::vector<std::string>& tokens) {
        if (tokens.size() < 5) {
            std::cout << "Usage: add_route <dest> <mask> <next_hop> <interface>" << std::endl;
            return;
        }
        
        if (!router_core_) {
            std::cout << "Router core not initialized" << std::endl;
            return;
        }
        
        Route route;
        route.network = string_to_ip(tokens[1]);
        route.mask = string_to_ip(tokens[2]);
        route.next_hop = string_to_ip(tokens[3]);
        route.interface = tokens[4];
        route.metric = 1;
        route.protocol = "STATIC";
        route.admin_distance = 1;
        route.last_updated = std::chrono::steady_clock::now();
        
        if (router_core_->add_route(route)) {
            std::cout << "Route added successfully" << std::endl;
        } else {
            std::cout << "Failed to add route" << std::endl;
        }
    }
    
    void remove_route(const std::vector<std::string>& tokens) {
        if (tokens.size() < 3) {
            std::cout << "Usage: remove_route <dest> <mask>" << std::endl;
            return;
        }
        
        if (!router_core_) {
            std::cout << "Router core not initialized" << std::endl;
            return;
        }
        
        Route route;
        route.network = string_to_ip(tokens[1]);
        route.mask = string_to_ip(tokens[2]);
        
        if (router_core_->remove_route(route)) {
            std::cout << "Route removed successfully" << std::endl;
        } else {
            std::cout << "Failed to remove route" << std::endl;
        }
    }
    
    void start_protocol(const std::vector<std::string>& tokens) {
        if (tokens.size() < 2) {
            std::cout << "Usage: start_protocol <name>" << std::endl;
            return;
        }
        
        std::cout << "Starting protocol " << tokens[1] << "..." << std::endl;
        // In a real implementation, this would start the protocol
    }
    
    void stop_protocol(const std::vector<std::string>& tokens) {
        if (tokens.size() < 2) {
            std::cout << "Usage: stop_protocol <name>" << std::endl;
            return;
        }
        
        std::cout << "Stopping protocol " << tokens[1] << "..." << std::endl;
        // In a real implementation, this would stop the protocol
    }

private:
    RouterCore* router_core_;
    
    std::string ip_to_string(uint32_t ip) {
        return std::to_string((ip >> 24) & 0xFF) + "." +
               std::to_string((ip >> 16) & 0xFF) + "." +
               std::to_string((ip >> 8) & 0xFF) + "." +
               std::to_string(ip & 0xFF);
    }
    
    uint32_t string_to_ip(const std::string& ip_str) {
        std::istringstream iss(ip_str);
        std::string token;
        uint32_t ip = 0;
        
        for (int i = 0; i < 4; i++) {
            if (!std::getline(iss, token, '.')) {
                return 0;
            }
            ip = (ip << 8) | (std::stoi(token) & 0xFF);
        }
        
        return ip;
    }
    
    std::string cidr_to_string(uint32_t mask) {
        int bits = 0;
        uint32_t temp = mask;
        while (temp & 0x80000000) {
            bits++;
            temp <<= 1;
        }
        return std::to_string(bits);
    }
};

} // namespace RouterSim
