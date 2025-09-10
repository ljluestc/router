#include "frr_integration.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>

namespace router_sim {

FRRClient::FRRClient() : connected_(false), socket_fd_(-1) {
}

FRRClient::~FRRClient() {
    disconnect();
}

bool FRRClient::connect() {
    if (connected_) {
        return true;
    }

    // Create Unix domain socket
    socket_fd_ = socket(AF_UNIX, SOCK_STREAM, 0);
    if (socket_fd_ < 0) {
        std::cerr << "Failed to create socket: " << strerror(errno) << std::endl;
        return false;
    }

    // Connect to FRR daemon
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, "/var/run/frr/frr.vty", sizeof(addr.sun_path) - 1);

    if (::connect(socket_fd_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "Failed to connect to FRR daemon: " << strerror(errno) << std::endl;
        close(socket_fd_);
        socket_fd_ = -1;
        return false;
    }

    connected_ = true;
    std::cout << "Connected to FRR daemon" << std::endl;
    return true;
}

bool FRRClient::disconnect() {
    if (!connected_) {
        return true;
    }

    if (socket_fd_ >= 0) {
        close(socket_fd_);
        socket_fd_ = -1;
    }

    connected_ = false;
    std::cout << "Disconnected from FRR daemon" << std::endl;
    return true;
}

bool FRRClient::is_connected() const {
    return connected_;
}

bool FRRClient::send_command(const std::string& command) {
    if (!connected_) {
        return false;
    }

    std::string full_command = command + "\n";
    ssize_t bytes_sent = send(socket_fd_, full_command.c_str(), full_command.length(), 0);
    
    if (bytes_sent < 0) {
        std::cerr << "Failed to send command: " << strerror(errno) << std::endl;
        return false;
    }

    return true;
}

std::string FRRClient::receive_response() {
    if (!connected_) {
        return "";
    }

    char buffer[4096];
    ssize_t bytes_received = recv(socket_fd_, buffer, sizeof(buffer) - 1, 0);
    
    if (bytes_received < 0) {
        std::cerr << "Failed to receive response: " << strerror(errno) << std::endl;
        return "";
    }

    buffer[bytes_received] = '\0';
    return std::string(buffer);
}

bool FRRClient::execute_command(const std::string& command, std::string& response) {
    if (!send_command(command)) {
        return false;
    }

    response = receive_response();
    return !response.empty();
}

// BGP Configuration
bool FRRClient::configure_bgp(uint32_t as_number, const std::string& router_id) {
    std::vector<std::string> commands = {
        "configure terminal",
        "router bgp " + std::to_string(as_number),
        "bgp router-id " + router_id,
        "end",
        "write memory"
    };

    for (const auto& cmd : commands) {
        std::string response;
        if (!execute_command(cmd, response)) {
            std::cerr << "Failed to execute BGP command: " << cmd << std::endl;
            return false;
        }
    }

    return true;
}

bool FRRClient::add_bgp_neighbor(const std::string& neighbor_ip, uint32_t remote_as) {
    std::vector<std::string> commands = {
        "configure terminal",
        "router bgp " + std::to_string(local_as_),
        "neighbor " + neighbor_ip + " remote-as " + std::to_string(remote_as),
        "neighbor " + neighbor_ip + " activate",
        "end",
        "write memory"
    };

    for (const auto& cmd : commands) {
        std::string response;
        if (!execute_command(cmd, response)) {
            std::cerr << "Failed to execute BGP neighbor command: " << cmd << std::endl;
            return false;
        }
    }

    return true;
}

bool FRRClient::remove_bgp_neighbor(const std::string& neighbor_ip) {
    std::vector<std::string> commands = {
        "configure terminal",
        "router bgp " + std::to_string(local_as_),
        "no neighbor " + neighbor_ip,
        "end",
        "write memory"
    };

    for (const auto& cmd : commands) {
        std::string response;
        if (!execute_command(cmd, response)) {
            std::cerr << "Failed to execute BGP neighbor removal command: " << cmd << std::endl;
            return false;
        }
    }

    return true;
}

std::vector<BGPNeighbor> FRRClient::get_bgp_neighbors() {
    std::vector<BGPNeighbor> neighbors;
    
    std::string response;
    if (!execute_command("show bgp neighbors", response)) {
        return neighbors;
    }

    // Parse FRR output to extract neighbor information
    std::istringstream iss(response);
    std::string line;
    BGPNeighbor current_neighbor;
    
    while (std::getline(iss, line)) {
        if (line.find("BGP neighbor is") != std::string::npos) {
            // Extract IP address
            size_t start = line.find("BGP neighbor is ") + 16;
            size_t end = line.find(",", start);
            if (end == std::string::npos) end = line.find(" ", start);
            if (end != std::string::npos) {
                current_neighbor.address = line.substr(start, end - start);
            }
        } else if (line.find("Remote AS") != std::string::npos) {
            // Extract AS number
            size_t start = line.find("Remote AS ") + 10;
            size_t end = line.find(",", start);
            if (end == std::string::npos) end = line.find(" ", start);
            if (end != std::string::npos) {
                current_neighbor.as_number = std::stoul(line.substr(start, end - start));
            }
        } else if (line.find("BGP state =") != std::string::npos) {
            // Extract state
            size_t start = line.find("BGP state = ") + 12;
            size_t end = line.find(",", start);
            if (end == std::string::npos) end = line.find(" ", start);
            if (end != std::string::npos) {
                current_neighbor.state = line.substr(start, end - start);
                neighbors.push_back(current_neighbor);
                current_neighbor = BGPNeighbor{};
            }
        }
    }

    return neighbors;
}

// OSPF Configuration
bool FRRClient::configure_ospf(const std::string& router_id, uint32_t area) {
    std::vector<std::string> commands = {
        "configure terminal",
        "router ospf",
        "ospf router-id " + router_id,
        "end",
        "write memory"
    };

    for (const auto& cmd : commands) {
        std::string response;
        if (!execute_command(cmd, response)) {
            std::cerr << "Failed to execute OSPF command: " << cmd << std::endl;
            return false;
        }
    }

    return true;
}

bool FRRClient::add_ospf_interface(const std::string& interface, uint32_t area, uint32_t cost) {
    std::vector<std::string> commands = {
        "configure terminal",
        "router ospf",
        "network " + interface + " area " + std::to_string(area),
        "interface " + interface,
        "ip ospf cost " + std::to_string(cost),
        "end",
        "write memory"
    };

    for (const auto& cmd : commands) {
        std::string response;
        if (!execute_command(cmd, response)) {
            std::cerr << "Failed to execute OSPF interface command: " << cmd << std::endl;
            return false;
        }
    }

    return true;
}

// ISIS Configuration
bool FRRClient::configure_isis(const std::string& system_id, uint8_t level) {
    std::vector<std::string> commands = {
        "configure terminal",
        "router isis",
        "net " + system_id,
        "is-type level-" + std::to_string(level),
        "end",
        "write memory"
    };

    for (const auto& cmd : commands) {
        std::string response;
        if (!execute_command(cmd, response)) {
            std::cerr << "Failed to execute ISIS command: " << cmd << std::endl;
            return false;
        }
    }

    return true;
}

bool FRRClient::add_isis_interface(const std::string& interface, uint8_t level) {
    std::vector<std::string> commands = {
        "configure terminal",
        "interface " + interface,
        "ip router isis",
        "isis circuit-type level-" + std::to_string(level),
        "end",
        "write memory"
    };

    for (const auto& cmd : commands) {
        std::string response;
        if (!execute_command(cmd, response)) {
            std::cerr << "Failed to execute ISIS interface command: " << cmd << std::endl;
            return false;
        }
    }

    return true;
}

// Route Management
std::vector<Route> FRRClient::get_routes() {
    std::vector<Route> routes;
    
    std::string response;
    if (!execute_command("show ip route", response)) {
        return routes;
    }

    // Parse FRR output to extract route information
    std::istringstream iss(response);
    std::string line;
    
    while (std::getline(iss, line)) {
        if (line.find("via") != std::string::npos || line.find("is directly connected") != std::string::npos) {
            Route route;
            
            // Extract destination
            size_t start = 0;
            size_t end = line.find(" ", start);
            if (end != std::string::npos) {
                route.destination = line.substr(start, end - start);
            }
            
            // Extract next hop
            size_t via_pos = line.find("via ");
            if (via_pos != std::string::npos) {
                start = via_pos + 4;
                end = line.find(" ", start);
                if (end == std::string::npos) end = line.find(",", start);
                if (end != std::string::npos) {
                    route.next_hop = line.substr(start, end - start);
                }
            }
            
            // Extract protocol
            if (line.find("B") != std::string::npos) {
                route.protocol = "BGP";
            } else if (line.find("O") != std::string::npos) {
                route.protocol = "OSPF";
            } else if (line.find("i") != std::string::npos) {
                route.protocol = "ISIS";
            } else if (line.find("C") != std::string::npos) {
                route.protocol = "Connected";
            }
            
            routes.push_back(route);
        }
    }

    return routes;
}

bool FRRClient::add_static_route(const std::string& destination, const std::string& next_hop, uint32_t distance) {
    std::vector<std::string> commands = {
        "configure terminal",
        "ip route " + destination + " " + next_hop + " " + std::to_string(distance),
        "end",
        "write memory"
    };

    for (const auto& cmd : commands) {
        std::string response;
        if (!execute_command(cmd, response)) {
            std::cerr << "Failed to execute static route command: " << cmd << std::endl;
            return false;
        }
    }

    return true;
}

bool FRRClient::remove_static_route(const std::string& destination, const std::string& next_hop) {
    std::vector<std::string> commands = {
        "configure terminal",
        "no ip route " + destination + " " + next_hop,
        "end",
        "write memory"
    };

    for (const auto& cmd : commands) {
        std::string response;
        if (!execute_command(cmd, response)) {
            std::cerr << "Failed to execute static route removal command: " << cmd << std::endl;
            return false;
        }
    }

    return true;
}

// Interface Management
std::vector<Interface> FRRClient::get_interfaces() {
    std::vector<Interface> interfaces;
    
    std::string response;
    if (!execute_command("show interface", response)) {
        return interfaces;
    }

    // Parse FRR output to extract interface information
    std::istringstream iss(response);
    std::string line;
    Interface current_interface;
    
    while (std::getline(iss, line)) {
        if (line.find("Interface") != std::string::npos && line.find("is") != std::string::npos) {
            // Extract interface name
            size_t start = line.find("Interface ") + 10;
            size_t end = line.find(" is", start);
            if (end != std::string::npos) {
                current_interface.name = line.substr(start, end - start);
            }
            
            // Extract status
            if (line.find("up") != std::string::npos) {
                current_interface.status = "up";
            } else {
                current_interface.status = "down";
            }
        } else if (line.find("Internet address is") != std::string::npos) {
            // Extract IP address
            size_t start = line.find("Internet address is ") + 20;
            size_t end = line.find("/", start);
            if (end == std::string::npos) end = line.find(" ", start);
            if (end != std::string::npos) {
                current_interface.ip_address = line.substr(start, end - start);
            }
        } else if (line.find("MTU") != std::string::npos) {
            // Extract MTU
            size_t start = line.find("MTU ") + 4;
            size_t end = line.find(" ", start);
            if (end != std::string::npos) {
                current_interface.mtu = std::stoul(line.substr(start, end - start));
            }
            
            interfaces.push_back(current_interface);
            current_interface = Interface{};
        }
    }

    return interfaces;
}

void FRRClient::set_local_as(uint32_t as_number) {
    local_as_ = as_number;
}

uint32_t FRRClient::get_local_as() const {
    return local_as_;
}

} // namespace router_sim