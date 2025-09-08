#include "frr_integration.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

namespace RouterSim {

FRRIntegration::FRRIntegration() 
    : frr_running_(false)
    , bgp_enabled_(false)
    , ospf_enabled_(false)
    , isis_enabled_(false)
    , as_number_(65000)
    , router_id_("192.168.1.1")
{
    // Initialize FRR configuration paths
    frr_config_dir_ = "/etc/frr";
    frr_log_dir_ = "/var/log/frr";
    zebra_socket_ = "/var/run/frr/zserv.api";
}

FRRIntegration::~FRRIntegration() {
    stop();
}

bool FRRIntegration::initialize() {
    std::cout << "Initializing FRR integration..." << std::endl;
    
    // Check if FRR is installed
    if (!checkFRRInstallation()) {
        std::cerr << "FRR is not properly installed" << std::endl;
        return false;
    }
    
    // Create necessary directories
    createFRRDirectories();
    
    // Generate FRR configuration
    if (!generateFRRConfig()) {
        std::cerr << "Failed to generate FRR configuration" << std::endl;
        return false;
    }
    
    return true;
}

bool FRRIntegration::start() {
    if (frr_running_) {
        return true;
    }
    
    std::cout << "Starting FRR daemons..." << std::endl;
    
    // Start zebra daemon
    if (!startDaemon("zebra")) {
        std::cerr << "Failed to start zebra daemon" << std::endl;
        return false;
    }
    
    // Start BGP daemon if enabled
    if (bgp_enabled_ && !startDaemon("bgpd")) {
        std::cerr << "Failed to start bgpd daemon" << std::endl;
        return false;
    }
    
    // Start OSPF daemon if enabled
    if (ospf_enabled_ && !startDaemon("ospfd")) {
        std::cerr << "Failed to start ospfd daemon" << std::endl;
        return false;
    }
    
    // Start IS-IS daemon if enabled
    if (isis_enabled_ && !startDaemon("isisd")) {
        std::cerr << "Failed to start isisd daemon" << std::endl;
        return false;
    }
    
    // Wait for daemons to start
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // Connect to zebra socket
    if (!connectToZebra()) {
        std::cerr << "Failed to connect to zebra socket" << std::endl;
        return false;
    }
    
    frr_running_ = true;
    std::cout << "FRR daemons started successfully" << std::endl;
    
    return true;
}

void FRRIntegration::stop() {
    if (!frr_running_) {
        return;
    }
    
    std::cout << "Stopping FRR daemons..." << std::endl;
    
    // Stop daemons in reverse order
    if (isis_enabled_) {
        stopDaemon("isisd");
    }
    if (ospf_enabled_) {
        stopDaemon("ospfd");
    }
    if (bgp_enabled_) {
        stopDaemon("bgpd");
    }
    stopDaemon("zebra");
    
    frr_running_ = false;
    std::cout << "FRR daemons stopped" << std::endl;
}

bool FRRIntegration::configureBGP(const BGPConfig& config) {
    if (!frr_running_) {
        std::cerr << "FRR is not running" << std::endl;
        return false;
    }
    
    std::cout << "Configuring BGP..." << std::endl;
    
    // Enable BGP if not already enabled
    if (!bgp_enabled_) {
        bgp_enabled_ = true;
        if (!startDaemon("bgpd")) {
            std::cerr << "Failed to start bgpd daemon" << std::endl;
            return false;
        }
    }
    
    // Update BGP configuration
    as_number_ = config.as_number;
    router_id_ = config.router_id;
    
    // Configure BGP neighbors
    for (const auto& neighbor : config.neighbors) {
        if (!addBGPNeighbor(neighbor)) {
            std::cerr << "Failed to add BGP neighbor: " << neighbor.ip_address << std::endl;
            return false;
        }
    }
    
    // Configure BGP networks
    for (const auto& network : config.networks) {
        if (!addBGPNetwork(network)) {
            std::cerr << "Failed to add BGP network: " << network << std::endl;
            return false;
        }
    }
    
    std::cout << "BGP configuration completed" << std::endl;
    return true;
}

bool FRRIntegration::configureOSPF(const OSPFConfig& config) {
    if (!frr_running_) {
        std::cerr << "FRR is not running" << std::endl;
        return false;
    }
    
    std::cout << "Configuring OSPF..." << std::endl;
    
    // Enable OSPF if not already enabled
    if (!ospf_enabled_) {
        ospf_enabled_ = true;
        if (!startDaemon("ospfd")) {
            std::cerr << "Failed to start ospfd daemon" << std::endl;
            return false;
        }
    }
    
    // Configure OSPF areas
    for (const auto& area : config.areas) {
        if (!addOSPFArea(area)) {
            std::cerr << "Failed to add OSPF area: " << area.area_id << std::endl;
            return false;
        }
    }
    
    // Configure OSPF interfaces
    for (const auto& interface : config.interfaces) {
        if (!addOSPFInterface(interface)) {
            std::cerr << "Failed to add OSPF interface: " << interface.name << std::endl;
            return false;
        }
    }
    
    std::cout << "OSPF configuration completed" << std::endl;
    return true;
}

bool FRRIntegration::configureISIS(const ISISConfig& config) {
    if (!frr_running_) {
        std::cerr << "FRR is not running" << std::endl;
        return false;
    }
    
    std::cout << "Configuring IS-IS..." << std::endl;
    
    // Enable IS-IS if not already enabled
    if (!isis_enabled_) {
        isis_enabled_ = true;
        if (!startDaemon("isisd")) {
            std::cerr << "Failed to start isisd daemon" << std::endl;
            return false;
        }
    }
    
    // Configure IS-IS levels
    for (const auto& level : config.levels) {
        if (!addISISLevel(level)) {
            std::cerr << "Failed to add IS-IS level: " << static_cast<int>(level.level) << std::endl;
            return false;
        }
    }
    
    // Configure IS-IS interfaces
    for (const auto& interface : config.interfaces) {
        if (!addISISInterface(interface)) {
            std::cerr << "Failed to add IS-IS interface: " << interface.name << std::endl;
            return false;
        }
    }
    
    std::cout << "IS-IS configuration completed" << std::endl;
    return true;
}

std::vector<Route> FRRIntegration::getRoutes() const {
    std::vector<Route> routes;
    
    if (!frr_running_) {
        return routes;
    }
    
    // Execute 'show ip route' command via vtysh
    std::string command = "vtysh -c 'show ip route'";
    std::string output = executeCommand(command);
    
    // Parse the output to extract routes
    routes = parseRoutes(output);
    
    return routes;
}

std::vector<BGPNeighbor> FRRIntegration::getBGPNeighbors() const {
    std::vector<BGPNeighbor> neighbors;
    
    if (!frr_running_ || !bgp_enabled_) {
        return neighbors;
    }
    
    // Execute 'show ip bgp neighbors' command via vtysh
    std::string command = "vtysh -c 'show ip bgp neighbors'";
    std::string output = executeCommand(command);
    
    // Parse the output to extract BGP neighbors
    neighbors = parseBGPNeighbors(output);
    
    return neighbors;
}

std::vector<OSPFArea> FRRIntegration::getOSPFAreas() const {
    std::vector<OSPFArea> areas;
    
    if (!frr_running_ || !ospf_enabled_) {
        return areas;
    }
    
    // Execute 'show ip ospf area' command via vtysh
    std::string command = "vtysh -c 'show ip ospf area'";
    std::string output = executeCommand(command);
    
    // Parse the output to extract OSPF areas
    areas = parseOSPFAreas(output);
    
    return areas;
}

std::vector<ISISLevel> FRRIntegration::getISISLevels() const {
    std::vector<ISISLevel> levels;
    
    if (!frr_running_ || !isis_enabled_) {
        return levels;
    }
    
    // Execute 'show isis level' command via vtysh
    std::string command = "vtysh -c 'show isis level'";
    std::string output = executeCommand(command);
    
    // Parse the output to extract IS-IS levels
    levels = parseISISLevels(output);
    
    return levels;
}

bool FRRIntegration::checkFRRInstallation() const {
    // Check if FRR binaries exist
    std::vector<std::string> required_binaries = {
        "zebra", "bgpd", "ospfd", "isisd", "vtysh"
    };
    
    for (const auto& binary : required_binaries) {
        std::string command = "which " + binary;
        if (executeCommand(command).empty()) {
            std::cerr << "Required FRR binary not found: " << binary << std::endl;
            return false;
        }
    }
    
    return true;
}

void FRRIntegration::createFRRDirectories() const {
    // Create FRR configuration directory
    std::string command = "mkdir -p " + frr_config_dir_;
    executeCommand(command);
    
    // Create FRR log directory
    command = "mkdir -p " + frr_log_dir_;
    executeCommand(command);
    
    // Set proper permissions
    command = "chown -R frr:frr " + frr_config_dir_ + " " + frr_log_dir_;
    executeCommand(command);
}

bool FRRIntegration::generateFRRConfig() const {
    // Generate zebra configuration
    if (!generateZebraConfig()) {
        return false;
    }
    
    // Generate BGP configuration
    if (bgp_enabled_ && !generateBGPConfig()) {
        return false;
    }
    
    // Generate OSPF configuration
    if (ospf_enabled_ && !generateOSPFConfig()) {
        return false;
    }
    
    // Generate IS-IS configuration
    if (isis_enabled_ && !generateISISConfig()) {
        return false;
    }
    
    return true;
}

bool FRRIntegration::generateZebraConfig() const {
    std::string config_file = frr_config_dir_ + "/zebra.conf";
    std::ofstream file(config_file);
    
    if (!file.is_open()) {
        std::cerr << "Failed to open zebra config file: " << config_file << std::endl;
        return false;
    }
    
    file << "hostname RouterSim\n";
    file << "password zebra\n";
    file << "enable password zebra\n";
    file << "log file " << frr_log_dir_ << "/zebra.log\n";
    file << "log stdout\n";
    file << "!\n";
    file << "interface lo\n";
    file << " ip address " << router_id_ << "/32\n";
    file << "!\n";
    file << "ip forwarding\n";
    file << "!\n";
    file << "line vty\n";
    file << "!\n";
    
    file.close();
    return true;
}

bool FRRIntegration::generateBGPConfig() const {
    std::string config_file = frr_config_dir_ + "/bgpd.conf";
    std::ofstream file(config_file);
    
    if (!file.is_open()) {
        std::cerr << "Failed to open BGP config file: " << config_file << std::endl;
        return false;
    }
    
    file << "hostname RouterSim\n";
    file << "password zebra\n";
    file << "enable password zebra\n";
    file << "log file " << frr_log_dir_ << "/bgpd.log\n";
    file << "log stdout\n";
    file << "!\n";
    file << "router bgp " << as_number_ << "\n";
    file << " bgp router-id " << router_id_ << "\n";
    file << "!\n";
    file << "line vty\n";
    file << "!\n";
    
    file.close();
    return true;
}

bool FRRIntegration::generateOSPFConfig() const {
    std::string config_file = frr_config_dir_ + "/ospfd.conf";
    std::ofstream file(config_file);
    
    if (!file.is_open()) {
        std::cerr << "Failed to open OSPF config file: " << config_file << std::endl;
        return false;
    }
    
    file << "hostname RouterSim\n";
    file << "password zebra\n";
    file << "enable password zebra\n";
    file << "log file " << frr_log_dir_ << "/ospfd.log\n";
    file << "log stdout\n";
    file << "!\n";
    file << "router ospf\n";
    file << " router-id " << router_id_ << "\n";
    file << "!\n";
    file << "line vty\n";
    file << "!\n";
    
    file.close();
    return true;
}

bool FRRIntegration::generateISISConfig() const {
    std::string config_file = frr_config_dir_ + "/isisd.conf";
    std::ofstream file(config_file);
    
    if (!file.is_open()) {
        std::cerr << "Failed to open IS-IS config file: " << config_file << std::endl;
        return false;
    }
    
    file << "hostname RouterSim\n";
    file << "password zebra\n";
    file << "enable password zebra\n";
    file << "log file " << frr_log_dir_ << "/isisd.log\n";
    file << "log stdout\n";
    file << "!\n";
    file << "router isis\n";
    file << " net 49.0001.1921.6800.1001.00\n";
    file << " is-type level-2\n";
    file << "!\n";
    file << "line vty\n";
    file << "!\n";
    
    file.close();
    return true;
}

bool FRRIntegration::startDaemon(const std::string& daemon) const {
    std::string command = daemon + " -d -A 127.0.0.1 -f " + frr_config_dir_ + "/" + daemon + ".conf";
    std::string output = executeCommand(command);
    
    if (!output.empty() && output.find("error") != std::string::npos) {
        std::cerr << "Error starting " << daemon << ": " << output << std::endl;
        return false;
    }
    
    return true;
}

bool FRRIntegration::stopDaemon(const std::string& daemon) const {
    std::string command = "pkill -f " + daemon;
    executeCommand(command);
    return true;
}

bool FRRIntegration::connectToZebra() const {
    // This would typically involve connecting to the zebra socket
    // For now, we'll just check if the socket exists
    return access(zebra_socket_.c_str(), R_OK) == 0;
}

bool FRRIntegration::addBGPNeighbor(const BGPNeighbor& neighbor) const {
    std::string command = "vtysh -c 'configure terminal' -c 'router bgp " + 
                         std::to_string(as_number_) + "' -c 'neighbor " + 
                         neighbor.ip_address + " remote-as " + 
                         std::to_string(neighbor.as_number) + "' -c 'end'";
    
    std::string output = executeCommand(command);
    return output.empty() || output.find("error") == std::string::npos;
}

bool FRRIntegration::addBGPNetwork(const std::string& network) const {
    std::string command = "vtysh -c 'configure terminal' -c 'router bgp " + 
                         std::to_string(as_number_) + "' -c 'network " + 
                         network + "' -c 'end'";
    
    std::string output = executeCommand(command);
    return output.empty() || output.find("error") == std::string::npos;
}

bool FRRIntegration::addOSPFArea(const OSPFArea& area) const {
    std::string command = "vtysh -c 'configure terminal' -c 'router ospf' -c 'area " + 
                         area.area_id + " " + area.area_type + "' -c 'end'";
    
    std::string output = executeCommand(command);
    return output.empty() || output.find("error") == std::string::npos;
}

bool FRRIntegration::addOSPFInterface(const OSPFInterface& interface) const {
    std::string command = "vtysh -c 'configure terminal' -c 'interface " + 
                         interface.name + "' -c 'ip ospf area " + 
                         interface.area_id + "' -c 'end'";
    
    std::string output = executeCommand(command);
    return output.empty() || output.find("error") == std::string::npos;
}

bool FRRIntegration::addISISLevel(const ISISLevel& level) const {
    std::string command = "vtysh -c 'configure terminal' -c 'router isis' -c 'is-type level-" + 
                         std::to_string(level.level) + "' -c 'end'";
    
    std::string output = executeCommand(command);
    return output.empty() || output.find("error") == std::string::npos;
}

bool FRRIntegration::addISISInterface(const ISISInterface& interface) const {
    std::string command = "vtysh -c 'configure terminal' -c 'interface " + 
                         interface.name + "' -c 'ip router isis' -c 'end'";
    
    std::string output = executeCommand(command);
    return output.empty() || output.find("error") == std::string::npos;
}

std::string FRRIntegration::executeCommand(const std::string& command) const {
    char buffer[128];
    std::string result = "";
    
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        return "";
    }
    
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    
    pclose(pipe);
    return result;
}

std::vector<Route> FRRIntegration::parseRoutes(const std::string& output) const {
    std::vector<Route> routes;
    std::istringstream stream(output);
    std::string line;
    
    while (std::getline(stream, line)) {
        // Parse route lines (simplified)
        if (line.find("via") != std::string::npos) {
            Route route;
            // Parse route details (simplified implementation)
            routes.push_back(route);
        }
    }
    
    return routes;
}

std::vector<BGPNeighbor> FRRIntegration::parseBGPNeighbors(const std::string& output) const {
    std::vector<BGPNeighbor> neighbors;
    std::istringstream stream(output);
    std::string line;
    
    while (std::getline(stream, line)) {
        // Parse BGP neighbor lines (simplified)
        if (line.find("BGP neighbor is") != std::string::npos) {
            BGPNeighbor neighbor;
            // Parse neighbor details (simplified implementation)
            neighbors.push_back(neighbor);
        }
    }
    
    return neighbors;
}

std::vector<OSPFArea> FRRIntegration::parseOSPFAreas(const std::string& output) const {
    std::vector<OSPFArea> areas;
    std::istringstream stream(output);
    std::string line;
    
    while (std::getline(stream, line)) {
        // Parse OSPF area lines (simplified)
        if (line.find("Area") != std::string::npos) {
            OSPFArea area;
            // Parse area details (simplified implementation)
            areas.push_back(area);
        }
    }
    
    return areas;
}

std::vector<ISISLevel> FRRIntegration::parseISISLevels(const std::string& output) const {
    std::vector<ISISLevel> levels;
    std::istringstream stream(output);
    std::string line;
    
    while (std::getline(stream, line)) {
        // Parse IS-IS level lines (simplified)
        if (line.find("Level") != std::string::npos) {
            ISISLevel level;
            // Parse level details (simplified implementation)
            levels.push_back(level);
        }
    }
    
    return levels;
}

} // namespace RouterSim
