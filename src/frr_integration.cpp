#include "frr_integration.h"
#include "protocols/bgp.h"
#include "protocols/ospf.h"
#include "protocols/isis.h"
#include "protocol_interface.h"
#include <iostream>
#include <cstdlib>
#include <sstream>
#include <fstream>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <thread>
#include <chrono>

namespace RouterSim {

FRRIntegration::FRRIntegration() : running_(false), daemon_running_(false), vty_socket_(-1) {
    // Initialize protocol handlers
    bgp_protocol_ = std::make_unique<router_sim::BGPProtocol>();
    ospf_protocol_ = std::make_unique<router_sim::OSPFProtocol>();
    isis_protocol_ = std::make_unique<router_sim::ISISProtocol>();
}

FRRIntegration::~FRRIntegration() {
    stop();
}

bool FRRIntegration::initialize(const FRRConfig& config) {
    if (running_) {
        return true;
    }
    
    config_ = config;
    
    // Check if FRR is installed
    int result = std::system("which vtysh > /dev/null 2>&1");
    if (result != 0) {
        std::cerr << "FRR not found. Please install FRR routing suite." << std::endl;
        return false;
    }
    
    // Initialize FRR daemon
    if (!initialize_frr_daemon()) {
        std::cerr << "Failed to initialize FRR daemon" << std::endl;
        return false;
    }
    
    // Load configuration
    if (!load_frr_config()) {
        std::cerr << "Failed to load FRR configuration" << std::endl;
        return false;
    }
    
    running_ = true;
    std::cout << "FRR integration initialized successfully" << std::endl;
    return true;
}

bool FRRIntegration::start() {
    if (!running_) {
        return false;
    }
    
    // Start FRR daemon
    if (!start_frr_daemon()) {
        std::cerr << "Failed to start FRR daemon" << std::endl;
        return false;
    }
    
    // Connect to VTY
    if (!connect_to_vty()) {
        std::cerr << "Failed to connect to VTY" << std::endl;
        return false;
    }
    
    std::cout << "FRR integration started successfully" << std::endl;
    return true;
}

bool FRRIntegration::stop() {
    if (!running_) {
        return true;
    }
    
    // Disconnect from VTY
    disconnect_from_vty();
    
    // Stop FRR daemon
    stop_frr_daemon();
    
    running_ = false;
    std::cout << "FRR integration stopped" << std::endl;
    return true;
}

bool FRRIntegration::is_running() const {
    return running_ && daemon_running_;
}

// Protocol management
bool FRRIntegration::start_protocol(const std::string& protocol, const std::map<std::string, std::string>& config) {
    if (!running_) {
        return false;
    }
    
    if (protocol == "bgp") {
        return bgp_protocol_->start(config);
    } else if (protocol == "ospf") {
        return ospf_protocol_->start(config);
    } else if (protocol == "isis") {
        return isis_protocol_->start(config);
    }
    
    return false;
}

bool FRRIntegration::stop_protocol(const std::string& protocol) {
    if (!running_) {
        return false;
    }
    
    if (protocol == "bgp") {
        return bgp_protocol_->stop();
    } else if (protocol == "ospf") {
        return ospf_protocol_->stop();
    } else if (protocol == "isis") {
        return isis_protocol_->stop();
    }
    
    return false;
}

bool FRRIntegration::is_protocol_running(const std::string& protocol) const {
    if (protocol == "bgp") {
        return bgp_protocol_->is_running();
    } else if (protocol == "ospf") {
        return ospf_protocol_->is_running();
    } else if (protocol == "isis") {
        return isis_protocol_->is_running();
    }
    
    return false;
}

// BGP operations
bool FRRIntegration::add_bgp_neighbor(const std::string& address, const std::map<std::string, std::string>& config) {
    if (!running_ || !bgp_protocol_->is_running()) {
        return false;
    }
    
    return bgp_protocol_->add_neighbor(address, config);
}

bool FRRIntegration::remove_bgp_neighbor(const std::string& address) {
    if (!running_ || !bgp_protocol_->is_running()) {
        return false;
    }
    
    return bgp_protocol_->remove_neighbor(address);
}

bool FRRIntegration::advertise_bgp_route(const std::string& prefix, const std::map<std::string, std::string>& attributes) {
    if (!running_ || !bgp_protocol_->is_running()) {
        return false;
    }
    
    return bgp_protocol_->advertise_route(prefix, attributes);
}

bool FRRIntegration::withdraw_bgp_route(const std::string& prefix) {
    if (!running_ || !bgp_protocol_->is_running()) {
        return false;
    }
    
    return bgp_protocol_->withdraw_route(prefix);
}

// OSPF operations
bool FRRIntegration::add_ospf_interface(const std::string& interface, const std::map<std::string, std::string>& config) {
    if (!running_ || !ospf_protocol_->is_running()) {
        return false;
    }
    
    return ospf_protocol_->add_interface(interface, config);
}

bool FRRIntegration::remove_ospf_interface(const std::string& interface) {
    if (!running_ || !ospf_protocol_->is_running()) {
        return false;
    }
    
    return ospf_protocol_->remove_interface(interface);
}

bool FRRIntegration::advertise_ospf_route(const std::string& prefix, const std::map<std::string, std::string>& attributes) {
    if (!running_ || !ospf_protocol_->is_running()) {
        return false;
    }
    
    return ospf_protocol_->advertise_route(prefix, attributes);
}

bool FRRIntegration::withdraw_ospf_route(const std::string& prefix) {
    if (!running_ || !ospf_protocol_->is_running()) {
        return false;
    }
    
    return ospf_protocol_->withdraw_route(prefix);
}

// IS-IS operations
bool FRRIntegration::add_isis_interface(const std::string& interface, const std::map<std::string, std::string>& config) {
    if (!running_ || !isis_protocol_->is_running()) {
        return false;
    }
    
    return isis_protocol_->add_interface(interface, config);
}

bool FRRIntegration::remove_isis_interface(const std::string& interface) {
    if (!running_ || !isis_protocol_->is_running()) {
        return false;
    }
    
    return isis_protocol_->remove_interface(interface);
}

bool FRRIntegration::advertise_isis_route(const std::string& prefix, const std::map<std::string, std::string>& attributes) {
    if (!running_ || !isis_protocol_->is_running()) {
        return false;
    }
    
    return isis_protocol_->advertise_route(prefix, attributes);
}

bool FRRIntegration::withdraw_isis_route(const std::string& prefix) {
    if (!running_ || !isis_protocol_->is_running()) {
        return false;
    }
    
    return isis_protocol_->withdraw_route(prefix);
}

// Route management
std::vector<RouteInfo> FRRIntegration::get_routes() const {
    std::vector<RouteInfo> routes;
    
    if (bgp_protocol_->is_running()) {
        auto bgp_routes = bgp_protocol_->get_routes();
        routes.insert(routes.end(), bgp_routes.begin(), bgp_routes.end());
    }
    
    if (ospf_protocol_->is_running()) {
        auto ospf_routes = ospf_protocol_->get_routes();
        routes.insert(routes.end(), ospf_routes.begin(), ospf_routes.end());
    }
    
    if (isis_protocol_->is_running()) {
        auto isis_routes = isis_protocol_->get_routes();
        routes.insert(routes.end(), isis_routes.begin(), isis_routes.end());
    }
    
    return routes;
}

std::vector<RouteInfo> FRRIntegration::get_routes_by_protocol(const std::string& protocol) const {
    if (protocol == "bgp") {
        return bgp_protocol_->get_routes();
    } else if (protocol == "ospf") {
        return ospf_protocol_->get_routes();
    } else if (protocol == "isis") {
        return isis_protocol_->get_routes();
    }
    
    return {};
}

std::vector<RouteInfo> FRRIntegration::get_bgp_routes() const {
    return bgp_protocol_->get_routes();
}

std::vector<RouteInfo> FRRIntegration::get_ospf_routes() const {
    return ospf_protocol_->get_routes();
}

std::vector<RouteInfo> FRRIntegration::get_isis_routes() const {
    return isis_protocol_->get_routes();
}

// Neighbor management
std::vector<NeighborInfo> FRRIntegration::get_neighbors() const {
    std::vector<NeighborInfo> neighbors;
    
    if (bgp_protocol_->is_running()) {
        auto bgp_neighbors = bgp_protocol_->get_neighbors();
        neighbors.insert(neighbors.end(), bgp_neighbors.begin(), bgp_neighbors.end());
    }
    
    if (ospf_protocol_->is_running()) {
        auto ospf_neighbors = ospf_protocol_->get_neighbors();
        neighbors.insert(neighbors.end(), ospf_neighbors.begin(), ospf_neighbors.end());
    }
    
    if (isis_protocol_->is_running()) {
        auto isis_neighbors = isis_protocol_->get_neighbors();
        neighbors.insert(neighbors.end(), isis_neighbors.begin(), isis_neighbors.end());
    }
    
    return neighbors;
}

std::vector<NeighborInfo> FRRIntegration::get_neighbors_by_protocol(const std::string& protocol) const {
    if (protocol == "bgp") {
        return bgp_protocol_->get_neighbors();
    } else if (protocol == "ospf") {
        return ospf_protocol_->get_neighbors();
    } else if (protocol == "isis") {
        return isis_protocol_->get_neighbors();
    }
    
    return {};
}

std::vector<NeighborInfo> FRRIntegration::get_bgp_neighbors() const {
    return bgp_protocol_->get_neighbors();
}

std::vector<NeighborInfo> FRRIntegration::get_ospf_neighbors() const {
    return ospf_protocol_->get_neighbors();
}

std::vector<NeighborInfo> FRRIntegration::get_isis_neighbors() const {
    return isis_protocol_->get_neighbors();
}

// Statistics
std::map<std::string, uint64_t> FRRIntegration::get_protocol_statistics(const std::string& protocol) const {
    if (protocol == "bgp") {
        return bgp_protocol_->get_statistics();
    } else if (protocol == "ospf") {
        return ospf_protocol_->get_statistics();
    } else if (protocol == "isis") {
        return isis_protocol_->get_statistics();
    }
    
    return {};
}

std::map<std::string, uint64_t> FRRIntegration::get_bgp_statistics() const {
    return bgp_protocol_->get_statistics();
}

std::map<std::string, uint64_t> FRRIntegration::get_ospf_statistics() const {
    return ospf_protocol_->get_statistics();
}

std::map<std::string, uint64_t> FRRIntegration::get_isis_statistics() const {
    return isis_protocol_->get_statistics();
}

// Configuration
bool FRRIntegration::update_config(const std::map<std::string, std::string>& config) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    
    for (const auto& [key, value] : config) {
        config_.global_config[key] = value;
    }
    
    return save_frr_config();
}

std::map<std::string, std::string> FRRIntegration::get_config() const {
    std::lock_guard<std::mutex> lock(config_mutex_);
    return config_.global_config;
}

bool FRRIntegration::save_config() const {
    return save_frr_config();
}

bool FRRIntegration::load_config() {
    return load_frr_config();
}

// Event callbacks
void FRRIntegration::set_route_update_callback(std::function<void(const RouteInfo&, bool)> callback) {
    route_update_callback_ = callback;
    
    // Set callbacks for individual protocols
    bgp_protocol_->set_route_update_callback(callback);
    ospf_protocol_->set_route_update_callback(callback);
    isis_protocol_->set_route_update_callback(callback);
}

void FRRIntegration::set_neighbor_update_callback(std::function<void(const NeighborInfo&, bool)> callback) {
    neighbor_update_callback_ = callback;
    
    // Set callbacks for individual protocols
    bgp_protocol_->set_neighbor_update_callback(callback);
    ospf_protocol_->set_neighbor_update_callback(callback);
    isis_protocol_->set_neighbor_update_callback(callback);
}

// VTY interface
bool FRRIntegration::execute_vty_command(const std::string& command) {
    if (!running_ || vty_socket_ == -1) {
        return false;
    }
    
    return send_vty_command(command);
}

std::string FRRIntegration::get_vty_output(const std::string& command) {
    if (!running_ || vty_socket_ == -1) {
        return "";
    }
    
    if (send_vty_command(command)) {
        return receive_vty_output();
    }
    
    return "";
}

// Logging
void FRRIntegration::set_log_level(const std::string& level) {
    std::lock_guard<std::mutex> lock(logs_mutex_);
    // Implementation for setting log level
}

std::vector<std::string> FRRIntegration::get_logs() const {
    std::lock_guard<std::mutex> lock(logs_mutex_);
    return logs_;
}

void FRRIntegration::clear_logs() {
    std::lock_guard<std::mutex> lock(logs_mutex_);
    logs_.clear();
}

// Private methods
bool FRRIntegration::initialize_frr_daemon() {
    // Check if FRR daemon is already running
    if (is_frr_daemon_running()) {
        return true;
    }
    
    // Start FRR daemon
    std::ostringstream cmd;
    cmd << "systemctl start frr";
    
    int result = std::system(cmd.str().c_str());
    return result == 0;
}

bool FRRIntegration::start_frr_daemon() {
    if (is_frr_daemon_running()) {
        daemon_running_ = true;
        return true;
    }
    
    // Start FRR daemon in background
    frr_daemon_thread_ = std::thread([this]() {
        std::ostringstream cmd;
        cmd << "frr -d -f " << config_.config_file;
        
        int result = std::system(cmd.str().c_str());
        daemon_running_ = (result == 0);
    });
    
    // Wait for daemon to start
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    return daemon_running_;
}

bool FRRIntegration::stop_frr_daemon() {
    if (!daemon_running_) {
        return true;
    }
    
    std::ostringstream cmd;
    cmd << "systemctl stop frr";
    
    int result = std::system(cmd.str().c_str());
    
    if (frr_daemon_thread_.joinable()) {
        frr_daemon_thread_.join();
    }
    
    daemon_running_ = false;
    return result == 0;
}

bool FRRIntegration::is_frr_daemon_running() const {
    int result = std::system("systemctl is-active --quiet frr");
    return result == 0;
}

bool FRRIntegration::load_frr_config() {
    std::ifstream file(config_.config_file);
    if (!file.is_open()) {
        // Create default configuration
        return create_default_config();
    }
    
    // Parse existing configuration
    std::string line;
    while (std::getline(file, line)) {
        // Parse configuration lines
        // This is a simplified implementation
    }
    
    return true;
}

bool FRRIntegration::save_frr_config() const {
    std::ofstream file(config_.config_file);
    if (!file.is_open()) {
        return false;
    }
    
    // Write configuration
    file << "frr version 8.1\n";
    file << "frr defaults traditional\n";
    file << "hostname router-sim\n";
    file << "log syslog informational\n";
    
    // Write global configuration
    for (const auto& [key, value] : config_.global_config) {
        file << key << " " << value << "\n";
    }
    
    return true;
}

bool FRRIntegration::create_default_config() const {
    std::ofstream file(config_.config_file);
    if (!file.is_open()) {
        return false;
    }
    
    file << "frr version 8.1\n";
    file << "frr defaults traditional\n";
    file << "hostname router-sim\n";
    file << "log syslog informational\n";
    file << "!\n";
    file << "line vty\n";
    file << "!\n";
    
    return true;
}

bool FRRIntegration::validate_frr_config() const {
    // Validate configuration syntax
    std::ostringstream cmd;
    cmd << "vtysh -c \"show running-config\" > /dev/null 2>&1";
    
    int result = std::system(cmd.str().c_str());
    return result == 0;
}

void FRRIntegration::on_route_update(const RouteInfo& route, bool is_advertisement) {
    if (route_update_callback_) {
        route_update_callback_(route, is_advertisement);
    }
}

void FRRIntegration::on_neighbor_update(const NeighborInfo& neighbor, bool is_up) {
    if (neighbor_update_callback_) {
        neighbor_update_callback_(neighbor, is_up);
    }
}

bool FRRIntegration::connect_to_vty() {
    vty_socket_ = socket(AF_UNIX, SOCK_STREAM, 0);
    if (vty_socket_ == -1) {
        return false;
    }
    
    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, "/var/run/frr/vtysh");
    
    if (connect(vty_socket_, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        close(vty_socket_);
        vty_socket_ = -1;
        return false;
    }
    
    return true;
}

void FRRIntegration::disconnect_from_vty() {
    if (vty_socket_ != -1) {
        close(vty_socket_);
        vty_socket_ = -1;
    }
}

bool FRRIntegration::send_vty_command(const std::string& command) {
    if (vty_socket_ == -1) {
        return false;
    }
    
    std::string full_command = command + "\n";
    ssize_t sent = send(vty_socket_, full_command.c_str(), full_command.length(), 0);
    return sent == static_cast<ssize_t>(full_command.length());
}

std::string FRRIntegration::receive_vty_output() {
    if (vty_socket_ == -1) {
        return "";
    }
    
    char buffer[4096];
    std::string output;
    
    // Set socket to non-blocking
    int flags = fcntl(vty_socket_, F_GETFL, 0);
    fcntl(vty_socket_, F_SETFL, flags | O_NONBLOCK);
    
    while (true) {
        ssize_t received = recv(vty_socket_, buffer, sizeof(buffer) - 1, 0);
        if (received <= 0) {
            break;
        }
        
        buffer[received] = '\0';
        output += buffer;
        
        // Check for end of output
        if (output.find("router-sim#") != std::string::npos) {
            break;
        }
    }
    
    // Restore blocking mode
    fcntl(vty_socket_, F_SETFL, flags);
    
    return output;
}

void FRRIntegration::log_message(const std::string& level, const std::string& message) {
    std::lock_guard<std::mutex> lock(logs_mutex_);
    
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto tm = *std::localtime(&time_t);
    
    std::ostringstream log_entry;
    log_entry << "[" << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << "] "
              << "[" << level << "] " << message;
    
    logs_.push_back(log_entry.str());
    
    // Keep only last 1000 log entries
    if (logs_.size() > 1000) {
        logs_.erase(logs_.begin());
    }
}

void FRRIntegration::parse_frr_logs() {
    std::ifstream log_file(config_.log_file);
    if (!log_file.is_open()) {
        return;
    }
    
    std::string line;
    while (std::getline(log_file, line)) {
        // Parse log entries and add to logs_
        log_message("INFO", line);
    }
}

} // namespace RouterSim
