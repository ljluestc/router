#include "frr_integration.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <thread>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>
#include <signal.h>
#include <sys/wait.h>

namespace RouterSim {

FRRIntegration::FRRIntegration() 
    : frr_running_(false), frr_pid_(-1) {
    // Initialize protocol states
    protocol_states_[FRRProtocol::BGP] = false;
    protocol_states_[FRRProtocol::OSPF] = false;
    protocol_states_[FRRProtocol::ISIS] = false;
    protocol_states_[FRRProtocol::STATIC] = false;
}

FRRIntegration::~FRRIntegration() {
    shutdown();
}

bool FRRIntegration::initialize() {
    std::cout << "Initializing FRR integration..." << std::endl;
    
    // Start FRR daemon
    if (!start_frr_daemon()) {
        std::cerr << "Failed to start FRR daemon" << std::endl;
        return false;
    }
    
    // Start monitoring thread
    frr_running_ = true;
    frr_monitor_thread_ = std::thread(&FRRIntegration::frr_monitor_loop, this);
    
    std::cout << "FRR integration initialized successfully" << std::endl;
    return true;
}

bool FRRIntegration::shutdown() {
    std::cout << "Shutting down FRR integration..." << std::endl;
    
    frr_running_ = false;
    
    // Stop all protocols
    for (auto& protocol : protocol_states_) {
        if (protocol.second) {
            stop_protocol(protocol.first);
        }
    }
    
    // Stop monitoring thread
    if (frr_monitor_thread_.joinable()) {
        frr_monitor_thread_.join();
    }
    
    // Stop FRR daemon
    stop_frr_daemon();
    
    std::cout << "FRR integration shutdown complete" << std::endl;
    return true;
}

bool FRRIntegration::configure_bgp(const BGPConfig& config) {
    std::lock_guard<std::mutex> lock(frr_mutex_);
    
    bgp_config_ = config;
    
    // Generate BGP configuration
    std::stringstream config_ss;
    config_ss << "router bgp " << config.local_as << "\n";
    config_ss << " bgp router-id " << config.router_id << "\n";
    
    if (config.enable_graceful_restart) {
        config_ss << " bgp graceful-restart\n";
    }
    
    for (const auto& neighbor : config.neighbors) {
        config_ss << " neighbor " << neighbor << " remote-as " 
                  << config.neighbor_configs.at(neighbor) << "\n";
        config_ss << " neighbor " << neighbor << " timers " 
                  << config.keepalive_interval << " " << config.hold_time << "\n";
    }
    
    config_ss << "!\n";
    
    // Write configuration to FRR
    if (!write_frr_config(config_ss.str())) {
        std::cerr << "Failed to write BGP configuration" << std::endl;
        return false;
    }
    
    std::cout << "BGP configured successfully" << std::endl;
    return true;
}

bool FRRIntegration::configure_ospf(const OSPFConfig& config) {
    std::lock_guard<std::mutex> lock(frr_mutex_);
    
    ospf_config_ = config;
    
    // Generate OSPF configuration
    std::stringstream config_ss;
    config_ss << "router ospf\n";
    config_ss << " ospf router-id " << config.router_id << "\n";
    
    for (const auto& area : config.areas) {
        config_ss << " network " << area << " area " << area << "\n";
    }
    
    config_ss << " timers throttle spf 10 100 5000\n";
    config_ss << "!\n";
    
    // Write configuration to FRR
    if (!write_frr_config(config_ss.str())) {
        std::cerr << "Failed to write OSPF configuration" << std::endl;
        return false;
    }
    
    std::cout << "OSPF configured successfully" << std::endl;
    return true;
}

bool FRRIntegration::configure_isis(const ISISConfig& config) {
    std::lock_guard<std::mutex> lock(frr_mutex_);
    
    isis_config_ = config;
    
    // Generate ISIS configuration
    std::stringstream config_ss;
    config_ss << "router isis " << config.area_id << "\n";
    config_ss << " net " << config.system_id << "\n";
    config_ss << " is-type level-" << static_cast<int>(config.level) << "\n";
    
    for (const auto& interface : config.interfaces) {
        config_ss << " interface " << interface << "\n";
        config_ss << "  isis hello-interval " << config.hello_interval << "\n";
        config_ss << "  isis hello-multiplier " << (config.hold_time / config.hello_interval) << "\n";
    }
    
    config_ss << "!\n";
    
    // Write configuration to FRR
    if (!write_frr_config(config_ss.str())) {
        std::cerr << "Failed to write ISIS configuration" << std::endl;
        return false;
    }
    
    std::cout << "ISIS configured successfully" << std::endl;
    return true;
}

bool FRRIntegration::start_protocol(FRRProtocol protocol) {
    std::lock_guard<std::mutex> lock(frr_mutex_);
    
    if (protocol_states_[protocol]) {
        std::cout << "Protocol already running" << std::endl;
        return true;
    }
    
    std::string command;
    switch (protocol) {
        case FRRProtocol::BGP:
            command = "router bgp " + bgp_config_.local_as;
            break;
        case FRRProtocol::OSPF:
            command = "router ospf";
            break;
        case FRRProtocol::ISIS:
            command = "router isis " + isis_config_.area_id;
            break;
        case FRRProtocol::STATIC:
            command = "ip route";
            break;
    }
    
    if (execute_frr_command(command).empty()) {
        std::cerr << "Failed to start protocol" << std::endl;
        return false;
    }
    
    protocol_states_[protocol] = true;
    std::cout << "Protocol started successfully" << std::endl;
    return true;
}

bool FRRIntegration::stop_protocol(FRRProtocol protocol) {
    std::lock_guard<std::mutex> lock(frr_mutex_);
    
    if (!protocol_states_[protocol]) {
        return true;
    }
    
    std::string command = "no ";
    switch (protocol) {
        case FRRProtocol::BGP:
            command += "router bgp " + bgp_config_.local_as;
            break;
        case FRRProtocol::OSPF:
            command += "router ospf";
            break;
        case FRRProtocol::ISIS:
            command += "router isis " + isis_config_.area_id;
            break;
        case FRRProtocol::STATIC:
            command += "ip route";
            break;
    }
    
    execute_frr_command(command);
    protocol_states_[protocol] = false;
    
    std::cout << "Protocol stopped" << std::endl;
    return true;
}

bool FRRIntegration::is_protocol_running(FRRProtocol protocol) const {
    std::lock_guard<std::mutex> lock(frr_mutex_);
    return protocol_states_.at(protocol);
}

bool FRRIntegration::add_route(const Route& route) {
    std::lock_guard<std::mutex> lock(routes_mutex_);
    
    std::string key = route.prefix + "/" + route.next_hop;
    routes_[key] = route;
    
    // Send route to FRR
    std::stringstream command;
    command << "ip route " << route.prefix << " " << route.next_hop;
    if (!route.interface.empty()) {
        command << " " << route.interface;
    }
    
    if (execute_frr_command(command.str()).empty()) {
        std::cerr << "Failed to add route to FRR" << std::endl;
        return false;
    }
    
    // Notify callback
    if (route_update_callback_) {
        route_update_callback_(route, true);
    }
    
    std::cout << "Route added: " << route.prefix << " via " << route.next_hop << std::endl;
    return true;
}

bool FRRIntegration::remove_route(const std::string& prefix) {
    std::lock_guard<std::mutex> lock(routes_mutex_);
    
    // Find and remove route
    for (auto it = routes_.begin(); it != routes_.end(); ++it) {
        if (it->second.prefix == prefix) {
            Route route = it->second;
            routes_.erase(it);
            
            // Remove from FRR
            std::string command = "no ip route " + prefix;
            execute_frr_command(command);
            
            // Notify callback
            if (route_update_callback_) {
                route_update_callback_(route, false);
            }
            
            std::cout << "Route removed: " << prefix << std::endl;
            return true;
        }
    }
    
    return false;
}

bool FRRIntegration::update_route(const Route& route) {
    return add_route(route); // FRR handles updates automatically
}

std::vector<Route> FRRIntegration::get_routes() const {
    std::lock_guard<std::mutex> lock(routes_mutex_);
    
    std::vector<Route> result;
    for (const auto& pair : routes_) {
        result.push_back(pair.second);
    }
    return result;
}

std::vector<Route> FRRIntegration::get_routes_by_protocol(FRRProtocol protocol) const {
    std::lock_guard<std::mutex> lock(routes_mutex_);
    
    std::vector<Route> result;
    for (const auto& pair : routes_) {
        if (pair.second.protocol == protocol) {
            result.push_back(pair.second);
        }
    }
    return result;
}

bool FRRIntegration::add_bgp_neighbor(const std::string& neighbor_ip, 
                                     const std::string& remote_as,
                                     const std::map<std::string, std::string>& options) {
    std::stringstream command;
    command << "router bgp " << bgp_config_.local_as << "\n";
    command << " neighbor " << neighbor_ip << " remote-as " << remote_as << "\n";
    
    for (const auto& option : options) {
        command << " neighbor " << neighbor_ip << " " << option.first 
                << " " << option.second << "\n";
    }
    
    if (execute_frr_command(command.str()).empty()) {
        std::cerr << "Failed to add BGP neighbor" << std::endl;
        return false;
    }
    
    std::cout << "BGP neighbor added: " << neighbor_ip << std::endl;
    return true;
}

bool FRRIntegration::remove_bgp_neighbor(const std::string& neighbor_ip) {
    std::stringstream command;
    command << "router bgp " << bgp_config_.local_as << "\n";
    command << " no neighbor " << neighbor_ip << "\n";
    
    execute_frr_command(command.str());
    std::cout << "BGP neighbor removed: " << neighbor_ip << std::endl;
    return true;
}

std::vector<std::string> FRRIntegration::get_bgp_neighbors() const {
    std::string output = execute_frr_command("show bgp neighbors");
    std::vector<std::string> neighbors;
    
    // Parse output to extract neighbor IPs
    std::istringstream iss(output);
    std::string line;
    while (std::getline(iss, line)) {
        if (line.find("BGP neighbor is") != std::string::npos) {
            // Extract IP address from line
            size_t pos = line.find("BGP neighbor is");
            if (pos != std::string::npos) {
                pos += 15; // Length of "BGP neighbor is"
                size_t end = line.find(",", pos);
                if (end != std::string::npos) {
                    neighbors.push_back(line.substr(pos, end - pos));
                }
            }
        }
    }
    
    return neighbors;
}

bool FRRIntegration::add_ospf_area(const std::string& area_id, 
                                  const std::string& area_type) {
    std::stringstream command;
    command << "router ospf\n";
    command << " area " << area_id << " " << area_type << "\n";
    
    if (execute_frr_command(command.str()).empty()) {
        std::cerr << "Failed to add OSPF area" << std::endl;
        return false;
    }
    
    std::cout << "OSPF area added: " << area_id << std::endl;
    return true;
}

bool FRRIntegration::remove_ospf_area(const std::string& area_id) {
    std::stringstream command;
    command << "router ospf\n";
    command << " no area " << area_id << "\n";
    
    execute_frr_command(command.str());
    std::cout << "OSPF area removed: " << area_id << std::endl;
    return true;
}

std::vector<std::string> FRRIntegration::get_ospf_areas() const {
    std::string output = execute_frr_command("show ip ospf area");
    std::vector<std::string> areas;
    
    // Parse output to extract area IDs
    std::istringstream iss(output);
    std::string line;
    while (std::getline(iss, line)) {
        if (line.find("Area") != std::string::npos) {
            // Extract area ID from line
            size_t pos = line.find("Area");
            if (pos != std::string::npos) {
                pos += 5; // Length of "Area "
                size_t end = line.find(" ", pos);
                if (end != std::string::npos) {
                    areas.push_back(line.substr(pos, end - pos));
                }
            }
        }
    }
    
    return areas;
}

bool FRRIntegration::add_interface_to_protocol(const std::string& interface, 
                                              FRRProtocol protocol,
                                              const std::map<std::string, std::string>& options) {
    std::stringstream command;
    
    switch (protocol) {
        case FRRProtocol::BGP:
            command << "router bgp " << bgp_config_.local_as << "\n";
            command << " network " << interface << "\n";
            break;
        case FRRProtocol::OSPF:
            command << "interface " << interface << "\n";
            command << " ip ospf area " << ospf_config_.areas[0] << "\n";
            break;
        case FRRProtocol::ISIS:
            command << "interface " << interface << "\n";
            command << " ip router isis " << isis_config_.area_id << "\n";
            break;
        default:
            return false;
    }
    
    for (const auto& option : options) {
        command << " " << option.first << " " << option.second << "\n";
    }
    
    if (execute_frr_command(command.str()).empty()) {
        std::cerr << "Failed to add interface to protocol" << std::endl;
        return false;
    }
    
    std::cout << "Interface " << interface << " added to protocol" << std::endl;
    return true;
}

bool FRRIntegration::remove_interface_from_protocol(const std::string& interface, 
                                                   FRRProtocol protocol) {
    std::stringstream command;
    
    switch (protocol) {
        case FRRProtocol::BGP:
            command << "router bgp " << bgp_config_.local_as << "\n";
            command << " no network " << interface << "\n";
            break;
        case FRRProtocol::OSPF:
            command << "interface " << interface << "\n";
            command << " no ip ospf area\n";
            break;
        case FRRProtocol::ISIS:
            command << "interface " << interface << "\n";
            command << " no ip router isis\n";
            break;
        default:
            return false;
    }
    
    execute_frr_command(command.str());
    std::cout << "Interface " << interface << " removed from protocol" << std::endl;
    return true;
}

FRRIntegration::ProtocolStats FRRIntegration::get_protocol_stats(FRRProtocol protocol) const {
    std::lock_guard<std::mutex> lock(frr_mutex_);
    return protocol_stats_.at(protocol);
}

std::map<FRRProtocol, FRRIntegration::ProtocolStats> FRRIntegration::get_all_protocol_stats() const {
    std::lock_guard<std::mutex> lock(frr_mutex_);
    return protocol_stats_;
}

void FRRIntegration::set_route_update_callback(RouteUpdateCallback callback) {
    route_update_callback_ = callback;
}

void FRRIntegration::set_neighbor_state_callback(NeighborStateCallback callback) {
    neighbor_state_callback_ = callback;
}

bool FRRIntegration::send_frr_command(const std::string& command) {
    return !execute_frr_command(command).empty();
}

std::string FRRIntegration::execute_frr_command(const std::string& command) {
    std::string full_command = "vtysh -c \"" + command + "\"";
    
    FILE* pipe = popen(full_command.c_str(), "r");
    if (!pipe) {
        return "";
    }
    
    std::string result;
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    
    pclose(pipe);
    return result;
}

bool FRRIntegration::save_configuration(const std::string& file_path) {
    std::ofstream file(file_path);
    if (!file.is_open()) {
        return false;
    }
    
    // Get current configuration from FRR
    std::string config = execute_frr_command("show running-config");
    file << config;
    
    file.close();
    std::cout << "Configuration saved to " << file_path << std::endl;
    return true;
}

bool FRRIntegration::load_configuration(const std::string& file_path) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        return false;
    }
    
    std::string config((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
    
    // Apply configuration to FRR
    std::string result = execute_frr_command("configure terminal");
    if (result.empty()) {
        return false;
    }
    
    // Parse and apply each line
    std::istringstream iss(config);
    std::string line;
    while (std::getline(iss, line)) {
        if (!line.empty() && line[0] != '!' && line[0] != '#') {
            execute_frr_command(line);
        }
    }
    
    execute_frr_command("end");
    execute_frr_command("write memory");
    
    std::cout << "Configuration loaded from " << file_path << std::endl;
    return true;
}

void FRRIntegration::enable_debug_logging(bool enable) {
    if (enable) {
        execute_frr_command("debug bgp");
        execute_frr_command("debug ospf");
        execute_frr_command("debug isis");
    } else {
        execute_frr_command("no debug bgp");
        execute_frr_command("no debug ospf");
        execute_frr_command("no debug isis");
    }
}

void FRRIntegration::set_log_level(const std::string& level) {
    std::string command = "log " + level;
    execute_frr_command(command);
}

void FRRIntegration::frr_monitor_loop() {
    std::cout << "FRR monitor loop started" << std::endl;
    
    while (frr_running_) {
        // Check FRR daemon status
        if (!is_frr_daemon_running()) {
            std::cerr << "FRR daemon not running, attempting restart..." << std::endl;
            start_frr_daemon();
        }
        
        // Update protocol statistics
        update_protocol_stats();
        
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    std::cout << "FRR monitor loop stopped" << std::endl;
}

bool FRRIntegration::start_frr_daemon() {
    if (is_frr_daemon_running()) {
        return true;
    }
    
    // Start FRR daemon
    frr_pid_ = fork();
    if (frr_pid_ == 0) {
        // Child process
        execlp("frr", "frr", "-d", nullptr);
        exit(1);
    } else if (frr_pid_ > 0) {
        // Parent process
        std::this_thread::sleep_for(std::chrono::seconds(2));
        return is_frr_daemon_running();
    }
    
    return false;
}

bool FRRIntegration::stop_frr_daemon() {
    if (frr_pid_ > 0) {
        kill(frr_pid_, SIGTERM);
        waitpid(frr_pid_, nullptr, 0);
        frr_pid_ = -1;
    }
    return true;
}

bool FRRIntegration::is_frr_daemon_running() const {
    if (frr_pid_ <= 0) {
        return false;
    }
    
    return kill(frr_pid_, 0) == 0;
}

void FRRIntegration::update_protocol_stats() {
    // Update BGP stats
    if (protocol_states_[FRRProtocol::BGP]) {
        std::string output = execute_frr_command("show bgp summary");
        // Parse output and update stats
        // This is a simplified implementation
        protocol_stats_[FRRProtocol::BGP].is_established = true;
    }
    
    // Update OSPF stats
    if (protocol_states_[FRRProtocol::OSPF]) {
        std::string output = execute_frr_command("show ip ospf neighbor");
        // Parse output and update stats
        protocol_stats_[FRRProtocol::OSPF].is_established = true;
    }
    
    // Update ISIS stats
    if (protocol_states_[FRRProtocol::ISIS]) {
        std::string output = execute_frr_command("show isis neighbor");
        // Parse output and update stats
        protocol_stats_[FRRProtocol::ISIS].is_established = true;
    }
}

bool FRRIntegration::write_frr_config(const std::string& config) {
    // Write configuration to FRR using vtysh
    std::string command = "configure terminal\n" + config + "end\nwrite memory";
    return !execute_frr_command(command).empty();
}

} // namespace RouterSim
