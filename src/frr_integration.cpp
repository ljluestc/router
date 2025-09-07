#include "frr_integration.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>
#include <regex>

using namespace RouterSim;

FRRIntegration::FRRIntegration() 
    : bgp_running_(false), ospf_running_(false), isis_running_(false), frr_running_(false),
      config_dir_("/tmp/frr_config"), pid_file_("/tmp/frr.pid"), log_file_("/tmp/frr.log") {
    // Create config directory
    system(("mkdir -p " + config_dir_).c_str());
}

FRRIntegration::~FRRIntegration() {
    cleanup();
}

bool FRRIntegration::initialize(const RouterConfig& config) {
    // Start FRR daemon
    if (!start_frr_daemon()) {
        std::cerr << "Failed to start FRR daemon\n";
        return false;
    }

    // Start monitoring thread
    monitoring_active_ = true;
    monitoring_thread_ = std::thread(&FRRIntegration::monitoring_loop, this);

    std::cout << "FRR integration initialized\n";
    return true;
}

void FRRIntegration::cleanup() {
    // Stop monitoring
    monitoring_active_ = false;
    if (monitoring_thread_.joinable()) {
        monitoring_thread_.join();
    }

    // Stop all protocols
    stop_bgp();
    stop_ospf();
    stop_isis();

    // Stop FRR daemon
    stop_frr_daemon();
}

bool FRRIntegration::start_bgp(const BGPConfig& config) {
    if (bgp_running_) {
        return true;
    }

    bgp_config_ = config;
    
    // Generate BGP configuration
    if (!generate_bgp_config(config)) {
        std::cerr << "Failed to generate BGP configuration\n";
        return false;
    }

    // Apply configuration
    std::string config_file = config_dir_ + "/bgpd.conf";
    if (!write_config_to_file(generate_bgp_config_string(config), config_file)) {
        std::cerr << "Failed to write BGP configuration\n";
        return false;
    }

    // Start BGP daemon
    std::string command = "bgpd -f " + config_file + " -d";
    if (system(command.c_str()) != 0) {
        std::cerr << "Failed to start BGP daemon\n";
        return false;
    }

    bgp_running_ = true;
    std::cout << "BGP started with AS " << config.as_number << "\n";
    return true;
}

bool FRRIntegration::stop_bgp() {
    if (!bgp_running_) {
        return true;
    }

    // Kill BGP daemon
    system("pkill -f bgpd");
    bgp_running_ = false;
    std::cout << "BGP stopped\n";
    return true;
}

bool FRRIntegration::start_ospf(const OSPFConfig& config) {
    if (ospf_running_) {
        return true;
    }

    ospf_config_ = config;
    
    // Generate OSPF configuration
    std::string config_file = config_dir_ + "/ospfd.conf";
    if (!write_config_to_file(generate_ospf_config_string(config), config_file)) {
        std::cerr << "Failed to write OSPF configuration\n";
        return false;
    }

    // Start OSPF daemon
    std::string command = "ospfd -f " + config_file + " -d";
    if (system(command.c_str()) != 0) {
        std::cerr << "Failed to start OSPF daemon\n";
        return false;
    }

    ospf_running_ = true;
    std::cout << "OSPF started in area " << config.area_id << "\n";
    return true;
}

bool FRRIntegration::stop_ospf() {
    if (!ospf_running_) {
        return true;
    }

    // Kill OSPF daemon
    system("pkill -f ospfd");
    ospf_running_ = false;
    std::cout << "OSPF stopped\n";
    return true;
}

bool FRRIntegration::start_isis(const ISISConfig& config) {
    if (isis_running_) {
        return true;
    }

    isis_config_ = config;
    
    // Generate IS-IS configuration
    std::string config_file = config_dir_ + "/isisd.conf";
    if (!write_config_to_file(generate_isis_config_string(config), config_file)) {
        std::cerr << "Failed to write IS-IS configuration\n";
        return false;
    }

    // Start IS-IS daemon
    std::string command = "isisd -f " + config_file + " -d";
    if (system(command.c_str()) != 0) {
        std::cerr << "Failed to start IS-IS daemon\n";
        return false;
    }

    isis_running_ = true;
    std::cout << "IS-IS started with system ID " << config.system_id << "\n";
    return true;
}

bool FRRIntegration::stop_isis() {
    if (!isis_running_) {
        return true;
    }

    // Kill IS-IS daemon
    system("pkill -f isisd");
    isis_running_ = false;
    std::cout << "IS-IS stopped\n";
    return true;
}

bool FRRIntegration::configure_interface(const std::string& interface, const InterfaceConfig& config) {
    // Create interface configuration command
    std::stringstream cmd;
    cmd << "vtysh -c 'configure terminal'";
    cmd << " -c 'interface " << interface << "'";
    cmd << " -c 'ip address " << config.ip_address << " " << config.subnet_mask << "'";
    cmd << " -c 'no shutdown'";
    cmd << " -c 'end'";

    return execute_vty_command(cmd.str());
}

bool FRRIntegration::unconfigure_interface(const std::string& interface) {
    std::stringstream cmd;
    cmd << "vtysh -c 'configure terminal'";
    cmd << " -c 'interface " << interface << "'";
    cmd << " -c 'shutdown'";
    cmd << " -c 'end'";

    return execute_vty_command(cmd.str());
}

std::vector<RouteEntry> FRRIntegration::get_routes() const {
    std::lock_guard<std::mutex> lock(route_mutex_);
    return routes_;
}

std::vector<RouteEntry> FRRIntegration::get_routes_by_protocol(const std::string& protocol) const {
    std::lock_guard<std::mutex> lock(route_mutex_);
    
    std::vector<RouteEntry> result;
    for (const auto& route : routes_) {
        if (route.protocol == protocol) {
            result.push_back(route);
        }
    }
    return result;
}

bool FRRIntegration::add_static_route(const std::string& network, const std::string& next_hop, 
                                     uint32_t metric, const std::string& interface) {
    std::stringstream cmd;
    cmd << "vtysh -c 'configure terminal'";
    cmd << " -c 'ip route " << network << " " << next_hop;
    if (!interface.empty()) {
        cmd << " " << interface;
    }
    cmd << "' -c 'end'";

    return execute_vty_command(cmd.str());
}

bool FRRIntegration::remove_static_route(const std::string& network) {
    std::stringstream cmd;
    cmd << "vtysh -c 'configure terminal'";
    cmd << " -c 'no ip route " << network << "'";
    cmd << " -c 'end'";

    return execute_vty_command(cmd.str());
}

std::vector<std::string> FRRIntegration::get_bgp_neighbors() const {
    return bgp_neighbors_;
}

std::vector<std::string> FRRIntegration::get_ospf_neighbors() const {
    return ospf_neighbors_;
}

std::vector<std::string> FRRIntegration::get_isis_neighbors() const {
    return isis_neighbors_;
}

bool FRRIntegration::is_bgp_running() const {
    return bgp_running_;
}

bool FRRIntegration::is_ospf_running() const {
    return ospf_running_;
}

bool FRRIntegration::is_isis_running() const {
    return isis_running_;
}

std::map<std::string, std::string> FRRIntegration::get_protocol_status() const {
    std::map<std::string, std::string> status;
    status["bgp"] = bgp_running_ ? "running" : "stopped";
    status["ospf"] = ospf_running_ ? "running" : "stopped";
    status["isis"] = isis_running_ ? "running" : "stopped";
    return status;
}

std::map<std::string, uint64_t> FRRIntegration::get_bgp_stats() const {
    std::map<std::string, uint64_t> stats;
    stats["neighbors"] = bgp_neighbors_.size();
    stats["routes"] = 0;
    
    std::lock_guard<std::mutex> lock(route_mutex_);
    for (const auto& route : routes_) {
        if (route.protocol == "bgp") {
            stats["routes"]++;
        }
    }
    
    return stats;
}

std::map<std::string, uint64_t> FRRIntegration::get_ospf_stats() const {
    std::map<std::string, uint64_t> stats;
    stats["neighbors"] = ospf_neighbors_.size();
    stats["routes"] = 0;
    
    std::lock_guard<std::mutex> lock(route_mutex_);
    for (const auto& route : routes_) {
        if (route.protocol == "ospf") {
            stats["routes"]++;
        }
    }
    
    return stats;
}

std::map<std::string, uint64_t> FRRIntegration::get_isis_stats() const {
    std::map<std::string, uint64_t> stats;
    stats["neighbors"] = isis_neighbors_.size();
    stats["routes"] = 0;
    
    std::lock_guard<std::mutex> lock(route_mutex_);
    for (const auto& route : routes_) {
        if (route.protocol == "isis") {
            stats["routes"]++;
        }
    }
    
    return stats;
}

void FRRIntegration::register_route_change_callback(std::function<void(const RouteEntry&)> callback) {
    route_change_callback_ = callback;
}

void FRRIntegration::register_neighbor_change_callback(std::function<void(const std::string&, bool)> callback) {
    neighbor_change_callback_ = callback;
}

bool FRRIntegration::start_frr_daemon() {
    if (frr_running_) {
        return true;
    }

    // Start zebra (core FRR daemon)
    std::string command = "zebra -f " + config_dir_ + "/zebra.conf -d";
    if (system(command.c_str()) != 0) {
        std::cerr << "Failed to start zebra daemon\n";
        return false;
    }

    frr_running_ = true;
    std::cout << "FRR daemon started\n";
    return true;
}

bool FRRIntegration::stop_frr_daemon() {
    if (!frr_running_) {
        return true;
    }

    // Kill all FRR daemons
    system("pkill -f zebra");
    system("pkill -f bgpd");
    system("pkill -f ospfd");
    system("pkill -f isisd");
    system("pkill -f vtysh");

    frr_running_ = false;
    std::cout << "FRR daemon stopped\n";
    return true;
}

bool FRRIntegration::is_frr_running() const {
    return frr_running_;
}

bool FRRIntegration::generate_bgp_config(const BGPConfig& config) {
    return true; // Implementation in generate_bgp_config_string
}

std::string FRRIntegration::generate_bgp_config_string(const BGPConfig& config) {
    std::stringstream config;
    config << "hostname bgpd\n";
    config << "password zebra\n";
    config << "enable password zebra\n";
    config << "log file " << log_file_ << "\n";
    config << "!\n";
    config << "router bgp " << config.as_number << "\n";
    config << " bgp router-id " << config.router_id << "\n";
    
    for (const auto& network : config.networks) {
        config << " network " << network << "\n";
    }
    
    for (const auto& neighbor : config.neighbors) {
        config << " neighbor " << neighbor << " remote-as " << config.as_number << "\n";
        config << " neighbor " << neighbor << " activate\n";
    }
    
    config << "!\n";
    config << "line vty\n";
    config << "!\n";
    
    return config.str();
}

std::string FRRIntegration::generate_ospf_config_string(const OSPFConfig& config) {
    std::stringstream config_str;
    config_str << "hostname ospfd\n";
    config_str << "password zebra\n";
    config_str << "enable password zebra\n";
    config_str << "log file " << log_file_ << "\n";
    config_str << "!\n";
    config_str << "router ospf\n";
    config_str << " router-id " << config.router_id << "\n";
    
    for (const auto& network : config.networks) {
        config_str << " network " << network << " area " << config.area_id << "\n";
    }
    
    config_str << "!\n";
    config_str << "line vty\n";
    config_str << "!\n";
    
    return config_str.str();
}

std::string FRRIntegration::generate_isis_config_string(const ISISConfig& config) {
    std::stringstream config_str;
    config_str << "hostname isisd\n";
    config_str << "password zebra\n";
    config_str << "enable password zebra\n";
    config_str << "log file " << log_file_ << "\n";
    config_str << "!\n";
    config_str << "router isis\n";
    config_str << " net " << config.area_id << "." << config.system_id << ".00\n";
    config_str << " is-type level-" << static_cast<int>(config.level) << "\n";
    
    for (const auto& network : config.networks) {
        config_str << " network " << network << "\n";
    }
    
    config_str << "!\n";
    config_str << "line vty\n";
    config_str << "!\n";
    
    return config_str.str();
}

bool FRRIntegration::write_config_to_file(const std::string& config, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    file << config;
    file.close();
    return true;
}

bool FRRIntegration::load_config_from_file(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        // Process configuration line
    }
    
    file.close();
    return true;
}

bool FRRIntegration::execute_vty_command(const std::string& command) {
    int result = system(command.c_str());
    return result == 0;
}

std::string FRRIntegration::get_vty_output(const std::string& command) {
    std::string output;
    FILE* pipe = popen(command.c_str(), "r");
    if (pipe) {
        char buffer[128];
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            output += buffer;
        }
        pclose(pipe);
    }
    return output;
}

std::vector<RouteEntry> FRRIntegration::parse_route_table(const std::string& output) {
    std::vector<RouteEntry> routes;
    std::istringstream stream(output);
    std::string line;
    
    while (std::getline(stream, line)) {
        // Parse route table line
        std::regex route_regex(R"(^(\S+)\s+(\S+)\s+(\S+)\s+(\d+)\s+(\S+)\s+(\S+))");
        std::smatch matches;
        
        if (std::regex_match(line, matches, route_regex)) {
            RouteEntry route;
            route.network = matches[1].str();
            route.next_hop = matches[2].str();
            route.interface = matches[3].str();
            route.metric = std::stoul(matches[4].str());
            route.protocol = matches[5].str();
            route.admin_distance = std::stoul(matches[6].str());
            route.is_active = true;
            routes.push_back(route);
        }
    }
    
    return routes;
}

std::vector<std::string> FRRIntegration::parse_neighbor_list(const std::string& output) {
    std::vector<std::string> neighbors;
    std::istringstream stream(output);
    std::string line;
    
    while (std::getline(stream, line)) {
        // Parse neighbor list line
        std::regex neighbor_regex(R"(^(\d+\.\d+\.\d+\.\d+))");
        std::smatch matches;
        
        if (std::regex_match(line, matches, neighbor_regex)) {
            neighbors.push_back(matches[1].str());
        }
    }
    
    return neighbors;
}

void FRRIntegration::monitoring_loop() {
    while (monitoring_active_) {
        // Update route table
        std::string command = "vtysh -c 'show ip route'";
        std::string output = get_vty_output(command);
        auto new_routes = parse_route_table(output);
        
        {
            std::lock_guard<std::mutex> lock(route_mutex_);
            routes_ = new_routes;
        }
        
        // Update neighbor lists
        if (bgp_running_) {
            command = "vtysh -c 'show bgp neighbors'";
            output = get_vty_output(command);
            bgp_neighbors_ = parse_neighbor_list(output);
        }
        
        if (ospf_running_) {
            command = "vtysh -c 'show ospf neighbor'";
            output = get_vty_output(command);
            ospf_neighbors_ = parse_neighbor_list(output);
        }
        
        if (isis_running_) {
            command = "vtysh -c 'show isis neighbor'";
            output = get_vty_output(command);
            isis_neighbors_ = parse_neighbor_list(output);
        }
        
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}
