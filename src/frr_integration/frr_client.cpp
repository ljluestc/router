#include "frr_integration.h"
#include <iostream>
#include <sstream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

namespace RouterSim {

// FRR Integration Implementation
FRRIntegration::FRRIntegration() 
    : running_(false)
    , status_(FRRStatus::DISCONNECTED) {
    
    statistics_["commands_executed"] = 0;
    statistics_["commands_failed"] = 0;
    statistics_["events_processed"] = 0;
    statistics_["daemons_started"] = 0;
    statistics_["daemons_stopped"] = 0;
}

FRRIntegration::~FRRIntegration() {
    stop();
}

bool FRRIntegration::initialize(const FRRConfig& config) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    config_ = config;
    
    // Create appropriate client based on configuration
    if (config.use_vtysh) {
        client_ = std::make_unique<VTYSHClient>();
    } else {
        client_ = std::make_unique<SocketClient>();
    }
    
    // Set up output callback for event processing
    client_->set_output_callback([this](const std::string& output) {
        process_frr_output(output);
    });
    
    return true;
}

void FRRIntegration::start() {
    if (running_) {
        return;
    }
    
    std::cout << "Starting FRR integration..." << std::endl;
    
    running_ = true;
    status_ = FRRStatus::CONNECTING;
    
    // Connect to FRR
    if (client_->connect(config_.hostname, config_.port)) {
        status_ = FRRStatus::CONNECTED;
        std::cout << "Connected to FRR at " << config_.hostname << ":" << config_.port << std::endl;
    } else {
        status_ = FRRStatus::ERROR;
        std::cerr << "Failed to connect to FRR" << std::endl;
        return;
    }
    
    // Start event processing thread
    event_thread_ = std::thread(&FRRIntegration::event_processing_loop, this);
    
    // Start configured daemons
    for (const auto& daemon_name : config_.daemons) {
        FRRDaemon daemon;
        if (daemon_name == "zebra") daemon = FRRDaemon::ZEBRA;
        else if (daemon_name == "bgpd") daemon = FRRDaemon::BGP;
        else if (daemon_name == "ospfd") daemon = FRRDaemon::OSPF;
        else if (daemon_name == "isisd") daemon = FRRDaemon::ISIS;
        else continue;
        
        start_daemon(daemon);
    }
    
    std::cout << "FRR integration started" << std::endl;
}

void FRRIntegration::stop() {
    if (!running_) {
        return;
    }
    
    std::cout << "Stopping FRR integration..." << std::endl;
    
    running_ = false;
    
    // Stop all daemons
    {
        std::lock_guard<std::mutex> lock(daemon_mutex_);
        for (auto& [daemon, status] : daemon_status_) {
            if (status) {
                stop_daemon(daemon);
            }
        }
    }
    
    // Stop event processing thread
    if (event_thread_.joinable()) {
        event_thread_.join();
    }
    
    // Disconnect from FRR
    if (client_) {
        client_->disconnect();
    }
    
    status_ = FRRStatus::DISCONNECTED;
    std::cout << "FRR integration stopped" << std::endl;
}

bool FRRIntegration::is_connected() const {
    return status_ == FRRStatus::CONNECTED && client_ && client_->is_connected();
}

FRRStatus FRRIntegration::get_status() const {
    return status_;
}

void FRRIntegration::set_config(const FRRConfig& config) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    config_ = config;
}

FRRConfig FRRIntegration::get_config() const {
    std::lock_guard<std::mutex> lock(config_mutex_);
    return config_;
}

FRRCommandResult FRRIntegration::execute_command(const std::string& command) {
    if (!is_connected()) {
        FRRCommandResult result;
        result.success = false;
        result.error = "Not connected to FRR";
        return result;
    }
    
    auto result = client_->execute_command(command);
    
    std::lock_guard<std::mutex> lock(statistics_mutex_);
    if (result.success) {
        statistics_["commands_executed"]++;
    } else {
        statistics_["commands_failed"]++;
    }
    
    return result;
}

FRRCommandResult FRRIntegration::execute_vtysh_command(const std::string& command) {
    std::string vtysh_cmd = build_vtysh_command(command);
    return execute_command(vtysh_cmd);
}

FRRCommandResult FRRIntegration::execute_daemon_command(FRRDaemon daemon, const std::string& command) {
    std::string daemon_cmd = build_daemon_command(daemon, command);
    return execute_command(daemon_cmd);
}

bool FRRIntegration::start_daemon(FRRDaemon daemon) {
    std::string daemon_name;
    switch (daemon) {
        case FRRDaemon::ZEBRA: daemon_name = "zebra"; break;
        case FRRDaemon::BGP: daemon_name = "bgpd"; break;
        case FRRDaemon::OSPF: daemon_name = "ospfd"; break;
        case FRRDaemon::ISIS: daemon_name = "isisd"; break;
        case FRRDaemon::STATIC: daemon_name = "staticd"; break;
    }
    
    auto result = execute_vtysh_command("service " + daemon_name + " start");
    
    if (result.success) {
        std::lock_guard<std::mutex> lock(daemon_mutex_);
        daemon_status_[daemon] = true;
        statistics_["daemons_started"]++;
        return true;
    }
    
    return false;
}

bool FRRIntegration::stop_daemon(FRRDaemon daemon) {
    std::string daemon_name;
    switch (daemon) {
        case FRRDaemon::ZEBRA: daemon_name = "zebra"; break;
        case FRRDaemon::BGP: daemon_name = "bgpd"; break;
        case FRRDaemon::OSPF: daemon_name = "ospfd"; break;
        case FRRDaemon::ISIS: daemon_name = "isisd"; break;
        case FRRDaemon::STATIC: daemon_name = "staticd"; break;
    }
    
    auto result = execute_vtysh_command("service " + daemon_name + " stop");
    
    if (result.success) {
        std::lock_guard<std::mutex> lock(daemon_mutex_);
        daemon_status_[daemon] = false;
        statistics_["daemons_stopped"]++;
        return true;
    }
    
    return false;
}

bool FRRIntegration::restart_daemon(FRRDaemon daemon) {
    return stop_daemon(daemon) && start_daemon(daemon);
}

bool FRRIntegration::is_daemon_running(FRRDaemon daemon) const {
    std::lock_guard<std::mutex> lock(daemon_mutex_);
    auto it = daemon_status_.find(daemon);
    return it != daemon_status_.end() && it->second;
}

bool FRRIntegration::load_config_file(const std::string& filename) {
    return execute_vtysh_command("copy " + filename + " running-config").success;
}

bool FRRIntegration::save_config_file(const std::string& filename) {
    return execute_vtysh_command("write " + filename).success;
}

bool FRRIntegration::apply_config() {
    return execute_vtysh_command("write").success;
}

bool FRRIntegration::reload_config() {
    return execute_vtysh_command("reload").success;
}

void FRRIntegration::set_event_callback(FRREventCallback callback) {
    event_callback_ = callback;
}

void FRRIntegration::remove_event_callback() {
    event_callback_ = nullptr;
}

std::map<std::string, uint64_t> FRRIntegration::get_statistics() const {
    std::lock_guard<std::mutex> lock(statistics_mutex_);
    return statistics_;
}

void FRRIntegration::reset_statistics() {
    std::lock_guard<std::mutex> lock(statistics_mutex_);
    for (auto& [key, value] : statistics_) {
        value = 0;
    }
}

// BGP specific methods
bool FRRIntegration::add_bgp_neighbor(const std::string& neighbor_ip, uint16_t as_number) {
    std::string command = "router bgp " + std::to_string(as_number) + "\nneighbor " + neighbor_ip + " remote-as " + std::to_string(as_number);
    return execute_vtysh_command(command).success;
}

bool FRRIntegration::remove_bgp_neighbor(const std::string& neighbor_ip) {
    std::string command = "no neighbor " + neighbor_ip;
    return execute_vtysh_command(command).success;
}

bool FRRIntegration::advertise_bgp_route(const std::string& prefix, const std::string& next_hop) {
    std::string command = "network " + prefix;
    return execute_vtysh_command(command).success;
}

bool FRRIntegration::withdraw_bgp_route(const std::string& prefix) {
    std::string command = "no network " + prefix;
    return execute_vtysh_command(command).success;
}

std::vector<std::string> FRRIntegration::get_bgp_routes() const {
    auto result = execute_vtysh_command("show ip bgp");
    std::vector<std::string> routes;
    
    if (result.success) {
        std::istringstream iss(result.output);
        std::string line;
        while (std::getline(iss, line)) {
            if (!line.empty() && line[0] != '*') {
                routes.push_back(line);
            }
        }
    }
    
    return routes;
}

std::vector<std::string> FRRIntegration::get_bgp_neighbors() const {
    auto result = execute_vtysh_command("show ip bgp neighbors");
    std::vector<std::string> neighbors;
    
    if (result.success) {
        std::istringstream iss(result.output);
        std::string line;
        while (std::getline(iss, line)) {
            if (!line.empty()) {
                neighbors.push_back(line);
            }
        }
    }
    
    return neighbors;
}

// OSPF specific methods
bool FRRIntegration::add_ospf_interface(const std::string& interface, uint32_t area_id) {
    std::string command = "interface " + interface + "\nip ospf area " + std::to_string(area_id);
    return execute_vtysh_command(command).success;
}

bool FRRIntegration::remove_ospf_interface(const std::string& interface) {
    std::string command = "interface " + interface + "\nno ip ospf area";
    return execute_vtysh_command(command).success;
}

bool FRRIntegration::set_ospf_router_id(const std::string& router_id) {
    std::string command = "router ospf\nospf router-id " + router_id;
    return execute_vtysh_command(command).success;
}

std::vector<std::string> FRRIntegration::get_ospf_routes() const {
    auto result = execute_vtysh_command("show ip ospf route");
    std::vector<std::string> routes;
    
    if (result.success) {
        std::istringstream iss(result.output);
        std::string line;
        while (std::getline(iss, line)) {
            if (!line.empty()) {
                routes.push_back(line);
            }
        }
    }
    
    return routes;
}

std::vector<std::string> FRRIntegration::get_ospf_interfaces() const {
    auto result = execute_vtysh_command("show ip ospf interface");
    std::vector<std::string> interfaces;
    
    if (result.success) {
        std::istringstream iss(result.output);
        std::string line;
        while (std::getline(iss, line)) {
            if (!line.empty()) {
                interfaces.push_back(line);
            }
        }
    }
    
    return interfaces;
}

// ISIS specific methods
bool FRRIntegration::set_isis_system_id(const std::string& system_id) {
    std::string command = "router isis\nnet " + system_id;
    return execute_vtysh_command(command).success;
}

bool FRRIntegration::add_isis_interface(const std::string& interface, uint8_t level) {
    std::string command = "interface " + interface + "\nip router isis";
    return execute_vtysh_command(command).success;
}

bool FRRIntegration::remove_isis_interface(const std::string& interface) {
    std::string command = "interface " + interface + "\nno ip router isis";
    return execute_vtysh_command(command).success;
}

std::vector<std::string> FRRIntegration::get_isis_routes() const {
    auto result = execute_vtysh_command("show ip route isis");
    std::vector<std::string> routes;
    
    if (result.success) {
        std::istringstream iss(result.output);
        std::string line;
        while (std::getline(iss, line)) {
            if (!line.empty()) {
                routes.push_back(line);
            }
        }
    }
    
    return routes;
}

std::vector<std::string> FRRIntegration::get_isis_interfaces() const {
    auto result = execute_vtysh_command("show isis interface");
    std::vector<std::string> interfaces;
    
    if (result.success) {
        std::istringstream iss(result.output);
        std::string line;
        while (std::getline(iss, line)) {
            if (!line.empty()) {
                interfaces.push_back(line);
            }
        }
    }
    
    return interfaces;
}

// Zebra specific methods
bool FRRIntegration::add_interface(const std::string& interface, const std::string& ip_address, const std::string& netmask) {
    std::string command = "interface " + interface + "\nip address " + ip_address + " " + netmask;
    return execute_vtysh_command(command).success;
}

bool FRRIntegration::remove_interface(const std::string& interface) {
    std::string command = "no interface " + interface;
    return execute_vtysh_command(command).success;
}

bool FRRIntegration::set_interface_up(const std::string& interface) {
    std::string command = "interface " + interface + "\nno shutdown";
    return execute_vtysh_command(command).success;
}

bool FRRIntegration::set_interface_down(const std::string& interface) {
    std::string command = "interface " + interface + "\nshutdown";
    return execute_vtysh_command(command).success;
}

std::vector<std::string> FRRIntegration::get_interfaces() const {
    auto result = execute_vtysh_command("show interface");
    std::vector<std::string> interfaces;
    
    if (result.success) {
        std::istringstream iss(result.output);
        std::string line;
        while (std::getline(iss, line)) {
            if (!line.empty()) {
                interfaces.push_back(line);
            }
        }
    }
    
    return interfaces;
}

void FRRIntegration::event_processing_loop() {
    std::cout << "FRR event processing loop started" << std::endl;
    
    while (running_) {
        // Event processing is handled by the output callback
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cout << "FRR event processing loop stopped" << std::endl;
}

void FRRIntegration::process_frr_output(const std::string& output) {
    std::istringstream iss(output);
    std::string line;
    
    while (std::getline(iss, line)) {
        FRREvent event = parse_frr_event(line);
        if (event.type != FRREventType::ERROR) {
            std::lock_guard<std::mutex> lock(statistics_mutex_);
            statistics_["events_processed"]++;
            
            if (event_callback_) {
                event_callback_(event);
            }
        }
    }
}

FRREvent FRRIntegration::parse_frr_event(const std::string& line) {
    FRREvent event;
    
    // Simple event parsing - in a real implementation, this would be more sophisticated
    if (line.find("route added") != std::string::npos) {
        event.type = FRREventType::ROUTE_ADDED;
    } else if (line.find("route removed") != std::string::npos) {
        event.type = FRREventType::ROUTE_REMOVED;
    } else if (line.find("neighbor up") != std::string::npos) {
        event.type = FRREventType::NEIGHBOR_UP;
    } else if (line.find("neighbor down") != std::string::npos) {
        event.type = FRREventType::NEIGHBOR_DOWN;
    } else if (line.find("interface up") != std::string::npos) {
        event.type = FRREventType::INTERFACE_UP;
    } else if (line.find("interface down") != std::string::npos) {
        event.type = FRREventType::INTERFACE_DOWN;
    } else {
        event.type = FRREventType::ERROR;
    }
    
    event.data = line;
    event.timestamp = std::chrono::steady_clock::now();
    
    return event;
}

std::string FRRIntegration::build_vtysh_command(const std::string& command) {
    return "vtysh -c \"" + command + "\"";
}

std::string FRRIntegration::build_daemon_command(FRRDaemon daemon, const std::string& command) {
    std::string daemon_name;
    switch (daemon) {
        case FRRDaemon::ZEBRA: daemon_name = "zebra"; break;
        case FRRDaemon::BGP: daemon_name = "bgpd"; break;
        case FRRDaemon::OSPF: daemon_name = "ospfd"; break;
        case FRRDaemon::ISIS: daemon_name = "isisd"; break;
        case FRRDaemon::STATIC: daemon_name = "staticd"; break;
    }
    
    return "vtysh -c \"router " + daemon_name + "\" -c \"" + command + "\"";
}

// VTYSH Client Implementation
VTYSHClient::VTYSHClient() : connected_(false) {}

VTYSHClient::~VTYSHClient() {
    disconnect();
}

bool VTYSHClient::connect(const std::string& hostname, uint16_t port) {
    std::lock_guard<std::mutex> lock(client_mutex_);
    
    // For VTYSH, we don't actually connect to a socket
    // We just verify that vtysh is available
    std::string test_cmd = "vtysh -c \"show version\"";
    std::string output, error;
    
    if (execute_vtysh(test_cmd, output, error)) {
        connected_ = true;
        return true;
    }
    
    return false;
}

void VTYSHClient::disconnect() {
    std::lock_guard<std::mutex> lock(client_mutex_);
    connected_ = false;
}

bool VTYSHClient::is_connected() const {
    return connected_;
}

FRRCommandResult VTYSHClient::execute_command(const std::string& command) {
    FRRCommandResult result;
    
    if (!connected_) {
        result.success = false;
        result.error = "Not connected";
        return result;
    }
    
    std::string output, error;
    if (execute_vtysh(command, output, error)) {
        result.success = true;
        result.output = output;
        result.exit_code = 0;
    } else {
        result.success = false;
        result.error = error;
        result.exit_code = 1;
    }
    
    return result;
}

void VTYSHClient::set_output_callback(std::function<void(const std::string&)> callback) {
    output_callback_ = callback;
}

bool VTYSHClient::execute_vtysh(const std::string& command, std::string& output, std::string& error) {
    std::string full_command = "vtysh -c \"" + command + "\" 2>&1";
    
    FILE* pipe = popen(full_command.c_str(), "r");
    if (!pipe) {
        error = "Failed to execute vtysh command";
        return false;
    }
    
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        output += buffer;
    }
    
    int exit_code = pclose(pipe);
    
    if (exit_code != 0) {
        error = "Command failed with exit code " + std::to_string(exit_code);
        return false;
    }
    
    if (output_callback_) {
        output_callback_(output);
    }
    
    return true;
}

// Socket Client Implementation
SocketClient::SocketClient() : socket_fd_(-1), connected_(false) {}

SocketClient::~SocketClient() {
    disconnect();
}

bool SocketClient::connect(const std::string& hostname, uint16_t port) {
    std::lock_guard<std::mutex> lock(socket_mutex_);
    
    socket_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd_ < 0) {
        return false;
    }
    
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, hostname.c_str(), &server_addr.sin_addr) <= 0) {
        close(socket_fd_);
        socket_fd_ = -1;
        return false;
    }
    
    if (::connect(socket_fd_, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        close(socket_fd_);
        socket_fd_ = -1;
        return false;
    }
    
    connected_ = true;
    return true;
}

void SocketClient::disconnect() {
    std::lock_guard<std::mutex> lock(socket_mutex_);
    
    if (socket_fd_ >= 0) {
        close(socket_fd_);
        socket_fd_ = -1;
    }
    
    connected_ = false;
}

bool SocketClient::is_connected() const {
    return connected_ && socket_fd_ >= 0;
}

FRRCommandResult SocketClient::execute_command(const std::string& command) {
    FRRCommandResult result;
    
    if (!is_connected()) {
        result.success = false;
        result.error = "Not connected";
        return result;
    }
    
    if (!send_command(command)) {
        result.success = false;
        result.error = "Failed to send command";
        return result;
    }
    
    std::string response;
    if (!receive_response(response)) {
        result.success = false;
        result.error = "Failed to receive response";
        return result;
    }
    
    result.success = true;
    result.output = response;
    result.exit_code = 0;
    
    if (output_callback_) {
        output_callback_(response);
    }
    
    return result;
}

void SocketClient::set_output_callback(std::function<void(const std::string&)> callback) {
    output_callback_ = callback;
}

bool SocketClient::send_command(const std::string& command) {
    std::lock_guard<std::mutex> lock(socket_mutex_);
    
    if (socket_fd_ < 0) {
        return false;
    }
    
    std::string cmd_with_newline = command + "\n";
    ssize_t bytes_sent = send(socket_fd_, cmd_with_newline.c_str(), cmd_with_newline.length(), 0);
    
    return bytes_sent == static_cast<ssize_t>(cmd_with_newline.length());
}

bool SocketClient::receive_response(std::string& response) {
    std::lock_guard<std::mutex> lock(socket_mutex_);
    
    if (socket_fd_ < 0) {
        return false;
    }
    
    char buffer[4096];
    ssize_t bytes_received = recv(socket_fd_, buffer, sizeof(buffer) - 1, 0);
    
    if (bytes_received <= 0) {
        return false;
    }
    
    buffer[bytes_received] = '\0';
    response = buffer;
    
    return true;
}

} // namespace RouterSim