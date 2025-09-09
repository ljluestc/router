#include "frr_client.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include <cstring>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

namespace RouterSim {

FRRClient::FRRClient() : connected_(false), socket_fd_(-1) {
    // Initialize FRR client
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
        std::cerr << "Failed to create socket" << std::endl;
        return false;
    }

    // Connect to FRR daemon
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, "/var/run/frr/zserv.api", sizeof(addr.sun_path) - 1);

    if (::connect(socket_fd_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "Failed to connect to FRR daemon" << std::endl;
        close(socket_fd_);
        socket_fd_ = -1;
        return false;
    }

    connected_ = true;
    std::cout << "Connected to FRR daemon" << std::endl;
    return true;
}

void FRRClient::disconnect() {
    if (socket_fd_ >= 0) {
        close(socket_fd_);
        socket_fd_ = -1;
    }
    connected_ = false;
}

bool FRRClient::configureBGP(const BGPConfig& config) {
    if (!connected_) {
        if (!connect()) {
            return false;
        }
    }

    std::stringstream bgp_config;
    bgp_config << "configure terminal\n";
    bgp_config << "router bgp " << config.as_number << "\n";
    
    for (const auto& neighbor : config.neighbors) {
        bgp_config << "neighbor " << neighbor.address << " remote-as " << neighbor.remote_as << "\n";
        bgp_config << "neighbor " << neighbor.address << " update-source " << neighbor.source_interface << "\n";
        if (neighbor.password != "") {
            bgp_config << "neighbor " << neighbor.address << " password " << neighbor.password << "\n";
        }
    }
    
    bgp_config << "end\n";
    bgp_config << "write memory\n";

    return sendCommand(bgp_config.str());
}

bool FRRClient::configureOSPF(const OSPFConfig& config) {
    if (!connected_) {
        if (!connect()) {
            return false;
        }
    }

    std::stringstream ospf_config;
    ospf_config << "configure terminal\n";
    ospf_config << "router ospf\n";
    ospf_config << "router-id " << config.router_id << "\n";
    
    for (const auto& network : config.networks) {
        ospf_config << "network " << network.address << " area " << network.area << "\n";
    }
    
    ospf_config << "end\n";
    ospf_config << "write memory\n";

    return sendCommand(ospf_config.str());
}

bool FRRClient::configureISIS(const ISISConfig& config) {
    if (!connected_) {
        if (!connect()) {
            return false;
        }
    }

    std::stringstream isis_config;
    isis_config << "configure terminal\n";
    isis_config << "router isis " << config.tag << "\n";
    isis_config << "net " << config.net_id << "\n";
    isis_config << "is-type " << (config.is_type == ISISLevel::L1 ? "level-1" : 
                                  config.is_type == ISISLevel::L2 ? "level-2" : "level-1-2") << "\n";
    
    for (const auto& interface : config.interfaces) {
        isis_config << "interface " << interface.name << "\n";
        isis_config << "ip router isis " << config.tag << "\n";
        isis_config << "isis hello-interval " << interface.hello_interval << "\n";
        isis_config << "isis hello-multiplier " << interface.hello_multiplier << "\n";
    }
    
    isis_config << "end\n";
    isis_config << "write memory\n";

    return sendCommand(isis_config.str());
}

std::vector<Route> FRRClient::getRoutes() {
    std::vector<Route> routes;
    
    if (!connected_) {
        if (!connect()) {
            return routes;
        }
    }

    std::string output = sendCommand("show ip route\n");
    routes = parseRoutes(output);
    
    return routes;
}

std::vector<Interface> FRRClient::getInterfaces() {
    std::vector<Interface> interfaces;
    
    if (!connected_) {
        if (!connect()) {
            return interfaces;
        }
    }

    std::string output = sendCommand("show interface\n");
    interfaces = parseInterfaces(output);
    
    return interfaces;
}

std::string FRRClient::sendCommand(const std::string& command) {
    if (!connected_) {
        return "";
    }

    // Send command
    if (send(socket_fd_, command.c_str(), command.length(), 0) < 0) {
        std::cerr << "Failed to send command" << std::endl;
        return "";
    }

    // Read response
    char buffer[4096];
    std::string response;
    int bytes_received;
    
    while ((bytes_received = recv(socket_fd_, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_received] = '\0';
        response += buffer;
        
        // Check if we've received the complete response
        if (response.find("router-sim#") != std::string::npos) {
            break;
        }
    }

    return response;
}

std::vector<Route> FRRClient::parseRoutes(const std::string& output) {
    std::vector<Route> routes;
    std::istringstream stream(output);
    std::string line;
    
    while (std::getline(stream, line)) {
        if (line.find("via") != std::string::npos || line.find("is directly connected") != std::string::npos) {
            Route route;
            
            // Parse route line (simplified parsing)
            std::istringstream line_stream(line);
            std::string token;
            std::vector<std::string> tokens;
            
            while (line_stream >> token) {
                tokens.push_back(token);
            }
            
            if (tokens.size() >= 2) {
                route.destination = tokens[0];
                route.protocol = tokens[1];
                
                if (tokens.size() >= 4 && tokens[2] == "via") {
                    route.next_hop = tokens[3];
                }
                
                routes.push_back(route);
            }
        }
    }
    
    return routes;
}

std::vector<Interface> FRRClient::parseInterfaces(const std::string& output) {
    std::vector<Interface> interfaces;
    std::istringstream stream(output);
    std::string line;
    Interface current_interface;
    bool in_interface = false;
    
    while (std::getline(stream, line)) {
        if (line.find("Interface") == 0) {
            if (in_interface) {
                interfaces.push_back(current_interface);
            }
            current_interface = Interface();
            current_interface.name = line.substr(9); // Remove "Interface "
            in_interface = true;
        } else if (in_interface && line.find("  ") == 0) {
            // Parse interface details
            if (line.find("Internet address") != std::string::npos) {
                size_t pos = line.find("Internet address");
                if (pos != std::string::npos) {
                    current_interface.ip_address = line.substr(pos + 16);
                }
            } else if (line.find("UP") != std::string::npos) {
                current_interface.status = "UP";
            } else if (line.find("DOWN") != std::string::npos) {
                current_interface.status = "DOWN";
            }
        }
    }
    
    if (in_interface) {
        interfaces.push_back(current_interface);
    }
    
    return interfaces;
}

} // namespace RouterSim