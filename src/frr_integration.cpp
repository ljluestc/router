#include "frr_integration.h"
#include <iostream>
#include <cstdlib>
#include <sstream>
#include <fstream>

namespace RouterSim {

FRRIntegration::FRRIntegration() : is_initialized_(false) {}

FRRIntegration::~FRRIntegration() {
    shutdown();
}

bool FRRIntegration::initialize() {
    if (is_initialized_) {
        return true;
    }
    
    // Check if FRR is installed
    int result = std::system("which vtysh > /dev/null 2>&1");
    if (result != 0) {
        std::cerr << "FRR not found. Please install FRR routing suite." << std::endl;
        return false;
    }
    
    is_initialized_ = true;
    std::cout << "FRR integration initialized" << std::endl;
    return true;
}

void FRRIntegration::shutdown() {
    if (!is_initialized_) {
        return;
    }
    
    stop_bgp();
    stop_ospf();
    stop_isis();
    
    is_initialized_ = false;
    std::cout << "FRR integration shutdown" << std::endl;
}

bool FRRIntegration::start_bgp(const std::string& as_number, const std::string& router_id) {
    if (!is_initialized_) {
        return false;
    }
    
    bgp_as_number_ = as_number;
    
    std::ostringstream cmd;
    cmd << "vtysh -c \"configure terminal\" "
        << "-c \"router bgp " << as_number << "\" "
        << "-c \"bgp router-id " << router_id << "\" "
        << "-c \"no bgp default ipv4-unicast\" "
        << "-c \"end\" "
        << "-c \"write memory\"";
    
    int result = std::system(cmd.str().c_str());
    if (result == 0) {
        std::cout << "BGP started with AS " << as_number << " and router-id " << router_id << std::endl;
        return true;
    }
    
    return false;
}

bool FRRIntegration::stop_bgp() {
    if (bgp_as_number_.empty()) {
        return true;
    }
    
    std::ostringstream cmd;
    cmd << "vtysh -c \"configure terminal\" "
        << "-c \"no router bgp " << bgp_as_number_ << "\" "
        << "-c \"end\" "
        << "-c \"write memory\"";
    
    int result = std::system(cmd.str().c_str());
    bgp_as_number_.clear();
    
    return result == 0;
}

bool FRRIntegration::add_bgp_neighbor(const std::string& neighbor_ip, const std::string& remote_as) {
    if (bgp_as_number_.empty()) {
        return false;
    }
    
    std::ostringstream cmd;
    cmd << "vtysh -c \"configure terminal\" "
        << "-c \"router bgp " << bgp_as_number_ << "\" "
        << "-c \"neighbor " << neighbor_ip << " remote-as " << remote_as << "\" "
        << "-c \"neighbor " << neighbor_ip << " activate\" "
        << "-c \"end\" "
        << "-c \"write memory\"";
    
    int result = std::system(cmd.str().c_str());
    return result == 0;
}

bool FRRIntegration::remove_bgp_neighbor(const std::string& neighbor_ip) {
    if (bgp_as_number_.empty()) {
        return false;
    }
    
    std::ostringstream cmd;
    cmd << "vtysh -c \"configure terminal\" "
        << "-c \"router bgp " << bgp_as_number_ << "\" "
        << "-c \"no neighbor " << neighbor_ip << "\" "
        << "-c \"end\" "
        << "-c \"write memory\"";
    
    int result = std::system(cmd.str().c_str());
    return result == 0;
}

std::vector<std::string> FRRIntegration::get_bgp_neighbors() const {
    std::vector<std::string> neighbors;
    
    if (bgp_as_number_.empty()) {
        return neighbors;
    }
    
    std::ostringstream cmd;
    cmd << "vtysh -c \"show bgp neighbors\" 2>/dev/null";
    
    FILE* pipe = popen(cmd.str().c_str(), "r");
    if (!pipe) {
        return neighbors;
    }
    
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        std::string line(buffer);
        if (line.find("BGP neighbor is") != std::string::npos) {
            // Extract neighbor IP from line
            size_t pos = line.find("BGP neighbor is");
            if (pos != std::string::npos) {
                pos += 15; // Length of "BGP neighbor is"
                size_t end = line.find(" ", pos);
                if (end != std::string::npos) {
                    std::string neighbor = line.substr(pos, end - pos);
                    neighbors.push_back(neighbor);
                }
            }
        }
    }
    
    pclose(pipe);
    return neighbors;
}

bool FRRIntegration::start_ospf(const std::string& router_id, const std::string& area) {
    if (!is_initialized_) {
        return false;
    }
    
    ospf_router_id_ = router_id;
    
    std::ostringstream cmd;
    cmd << "vtysh -c \"configure terminal\" "
        << "-c \"router ospf\" "
        << "-c \"ospf router-id " << router_id << "\" "
        << "-c \"network " << area << " area 0\" "
        << "-c \"end\" "
        << "-c \"write memory\"";
    
    int result = std::system(cmd.str().c_str());
    if (result == 0) {
        std::cout << "OSPF started with router-id " << router_id << " and area " << area << std::endl;
        return true;
    }
    
    return false;
}

bool FRRIntegration::stop_ospf() {
    if (ospf_router_id_.empty()) {
        return true;
    }
    
    std::ostringstream cmd;
    cmd << "vtysh -c \"configure terminal\" "
        << "-c \"no router ospf\" "
        << "-c \"end\" "
        << "-c \"write memory\"";
    
    int result = std::system(cmd.str().c_str());
    ospf_router_id_.clear();
    
    return result == 0;
}

bool FRRIntegration::add_ospf_interface(const std::string& interface, const std::string& area) {
    std::ostringstream cmd;
    cmd << "vtysh -c \"configure terminal\" "
        << "-c \"interface " << interface << "\" "
        << "-c \"ip ospf area " << area << "\" "
        << "-c \"ip ospf hello-interval 10\" "
        << "-c \"ip ospf dead-interval 40\" "
        << "-c \"end\" "
        << "-c \"write memory\"";
    
    int result = std::system(cmd.str().c_str());
    return result == 0;
}

bool FRRIntegration::remove_ospf_interface(const std::string& interface) {
    std::ostringstream cmd;
    cmd << "vtysh -c \"configure terminal\" "
        << "-c \"interface " << interface << "\" "
        << "-c \"no ip ospf area\" "
        << "-c \"end\" "
        << "-c \"write memory\"";
    
    int result = std::system(cmd.str().c_str());
    return result == 0;
}

std::vector<std::string> FRRIntegration::get_ospf_interfaces() const {
    std::vector<std::string> interfaces;
    
    std::ostringstream cmd;
    cmd << "vtysh -c \"show ip ospf interface\" 2>/dev/null";
    
    FILE* pipe = popen(cmd.str().c_str(), "r");
    if (!pipe) {
        return interfaces;
    }
    
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        std::string line(buffer);
        if (line.find("is up") != std::string::npos) {
            // Extract interface name
            size_t pos = line.find(" ");
            if (pos != std::string::npos) {
                size_t end = line.find(" ", pos + 1);
                if (end != std::string::npos) {
                    std::string interface = line.substr(pos + 1, end - pos - 1);
                    interfaces.push_back(interface);
                }
            }
        }
    }
    
    pclose(pipe);
    return interfaces;
}

bool FRRIntegration::start_isis(const std::string& system_id, const std::string& area_id) {
    if (!is_initialized_) {
        return false;
    }
    
    isis_system_id_ = system_id;
    
    std::ostringstream cmd;
    cmd << "vtysh -c \"configure terminal\" "
        << "-c \"router isis\" "
        << "-c \"net " << area_id << "." << system_id << ".00\" "
        << "-c \"is-type level-2-only\" "
        << "-c \"end\" "
        << "-c \"write memory\"";
    
    int result = std::system(cmd.str().c_str());
    if (result == 0) {
        std::cout << "IS-IS started with system-id " << system_id << " and area-id " << area_id << std::endl;
        return true;
    }
    
    return false;
}

bool FRRIntegration::stop_isis() {
    if (isis_system_id_.empty()) {
        return true;
    }
    
    std::ostringstream cmd;
    cmd << "vtysh -c \"configure terminal\" "
        << "-c \"no router isis\" "
        << "-c \"end\" "
        << "-c \"write memory\"";
    
    int result = std::system(cmd.str().c_str());
    isis_system_id_.clear();
    
    return result == 0;
}

bool FRRIntegration::add_isis_interface(const std::string& interface, uint8_t level) {
    std::ostringstream cmd;
    cmd << "vtysh -c \"configure terminal\" "
        << "-c \"interface " << interface << "\" "
        << "-c \"ip router isis\" "
        << "-c \"isis circuit-type level-" << (int)level << "\" "
        << "-c \"end\" "
        << "-c \"write memory\"";
    
    int result = std::system(cmd.str().c_str());
    return result == 0;
}

bool FRRIntegration::remove_isis_interface(const std::string& interface) {
    std::ostringstream cmd;
    cmd << "vtysh -c \"configure terminal\" "
        << "-c \"interface " << interface << "\" "
        << "-c \"no ip router isis\" "
        << "-c \"end\" "
        << "-c \"write memory\"";
    
    int result = std::system(cmd.str().c_str());
    return result == 0;
}

std::vector<std::string> FRRIntegration::get_isis_interfaces() const {
    std::vector<std::string> interfaces;
    
    std::ostringstream cmd;
    cmd << "vtysh -c \"show isis interface\" 2>/dev/null";
    
    FILE* pipe = popen(cmd.str().c_str(), "r");
    if (!pipe) {
        return interfaces;
    }
    
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        std::string line(buffer);
        if (line.find("is up") != std::string::npos) {
            // Extract interface name
            size_t pos = line.find(" ");
            if (pos != std::string::npos) {
                size_t end = line.find(" ", pos + 1);
                if (end != std::string::npos) {
                    std::string interface = line.substr(pos + 1, end - pos - 1);
                    interfaces.push_back(interface);
                }
            }
        }
    }
    
    pclose(pipe);
    return interfaces;
}

void FRRIntegration::set_route_update_callback(std::function<void(const std::string&, const std::string&, const std::string&)> callback) {
    route_callback_ = callback;
}

} // namespace RouterSim
