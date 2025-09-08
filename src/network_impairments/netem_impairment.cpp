#include "network_impairments.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <unistd.h>
#include <sys/stat.h>

namespace router_sim {

NetemImpairment::NetemImpairment() 
    : running_(false), monitoring_running_(false) {
}

bool NetemImpairment::initialize(const std::map<std::string, std::string>& config) {
    // Check if tc command is available
    if (system("which tc > /dev/null 2>&1") != 0) {
        std::cerr << "tc command not found. Please install iproute2 package." << std::endl;
        return false;
    }
    
    // Check if we have root privileges
    if (geteuid() != 0) {
        std::cerr << "Root privileges required for tc operations." << std::endl;
        return false;
    }
    
    return true;
}

bool NetemImpairment::start() {
    if (running_) {
        return true;
    }
    
    running_ = true;
    monitoring_running_ = true;
    
    // Start monitoring thread
    monitoring_thread_ = std::thread(&NetemImpairment::monitoring_loop, this);
    
    return true;
}

bool NetemImpairment::stop() {
    if (!running_) {
        return true;
    }
    
    running_ = false;
    monitoring_running_ = false;
    
    if (monitoring_thread_.joinable()) {
        monitoring_thread_.join();
    }
    
    // Clean up all interfaces
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    for (const auto& pair : interfaces_) {
        teardown_interface(pair.first);
    }
    
    return true;
}

bool NetemImpairment::is_running() const {
    return running_;
}

bool NetemImpairment::add_impairment(const ImpairmentConfig& config) {
    if (!running_) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    // Check if interface exists
    auto it = interfaces_.find(config.interface);
    if (it == interfaces_.end()) {
        std::cerr << "Interface " << config.interface << " not found" << std::endl;
        return false;
    }
    
    // Add impairment to interface
    it->second.impairments.push_back(config);
    
    // Apply tc rule
    std::string tc_command = generate_tc_command(config);
    if (!execute_tc_command(tc_command)) {
        std::cerr << "Failed to apply tc rule: " << tc_command << std::endl;
        return false;
    }
    
    // Update statistics
    {
        std::lock_guard<std::mutex> stats_lock(stats_mutex_);
        if (impairment_applied_callback_) {
            impairment_applied_callback_(config.interface, config.type);
        }
    }
    
    return true;
}

bool NetemImpairment::remove_impairment(const std::string& interface, ImpairmentType type) {
    if (!running_) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    auto it = interfaces_.find(interface);
    if (it == interfaces_.end()) {
        return false;
    }
    
    // Remove impairment from interface
    auto& impairments = it->second.impairments;
    auto impairment_it = std::find_if(impairments.begin(), impairments.end(),
        [type](const ImpairmentConfig& config) { return config.type == type; });
    
    if (impairment_it != impairments.end()) {
        impairments.erase(impairment_it);
        
        // Rebuild tc rules for this interface
        remove_tc_rule(interface);
        
        for (const auto& impairment : impairments) {
            std::string tc_command = generate_tc_command(impairment);
            execute_tc_command(tc_command);
        }
        
        return true;
    }
    
    return false;
}

bool NetemImpairment::update_impairment(const ImpairmentConfig& config) {
    if (!running_) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    auto it = interfaces_.find(config.interface);
    if (it == interfaces_.end()) {
        return false;
    }
    
    // Find and update impairment
    auto& impairments = it->second.impairments;
    auto impairment_it = std::find_if(impairments.begin(), impairments.end(),
        [&config](const ImpairmentConfig& imp) { return imp.type == config.type; });
    
    if (impairment_it != impairments.end()) {
        *impairment_it = config;
        
        // Rebuild tc rules for this interface
        remove_tc_rule(config.interface);
        
        for (const auto& impairment : impairments) {
            std::string tc_command = generate_tc_command(impairment);
            execute_tc_command(tc_command);
        }
        
        return true;
    }
    
    return false;
}

std::vector<ImpairmentConfig> NetemImpairment::get_impairments(const std::string& interface) const {
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    auto it = interfaces_.find(interface);
    if (it != interfaces_.end()) {
        return it->second.impairments;
    }
    
    return {};
}

bool NetemImpairment::add_interface(const std::string& name) {
    if (!running_) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    // Check if interface already exists
    if (interfaces_.find(name) != interfaces_.end()) {
        return true;
    }
    
    // Create interface entry
    NetworkInterface interface;
    interface.name = name;
    interface.type = "ethernet";
    interface.status = "up";
    interfaces_[name] = interface;
    
    // Setup interface for tc operations
    if (!setup_interface(name)) {
        interfaces_.erase(name);
        return false;
    }
    
    return true;
}

bool NetemImpairment::remove_interface(const std::string& name) {
    if (!running_) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    auto it = interfaces_.find(name);
    if (it == interfaces_.end()) {
        return false;
    }
    
    // Teardown interface
    teardown_interface(name);
    interfaces_.erase(it);
    
    return true;
}

std::vector<NetworkInterface> NetemImpairment::get_interfaces() const {
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    std::vector<NetworkInterface> result;
    for (const auto& pair : interfaces_) {
        result.push_back(pair.second);
    }
    
    return result;
}

NetworkInterface NetemImpairment::get_interface(const std::string& name) const {
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    auto it = interfaces_.find(name);
    if (it != interfaces_.end()) {
        return it->second;
    }
    
    return NetworkInterface{};
}

ImpairmentStatistics NetemImpairment::get_statistics() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    return statistics_;
}

ImpairmentStatistics NetemImpairment::get_interface_statistics(const std::string& interface) const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    
    auto it = interface_statistics_.find(interface);
    if (it != interface_statistics_.end()) {
        return it->second;
    }
    
    return ImpairmentStatistics{};
}

void NetemImpairment::set_packet_processed_callback(std::function<void(const std::string&, size_t)> callback) {
    packet_processed_callback_ = callback;
}

void NetemImpairment::set_impairment_applied_callback(std::function<void(const std::string&, ImpairmentType)> callback) {
    impairment_applied_callback_ = callback;
}

bool NetemImpairment::apply_tc_rule(const std::string& interface, const std::string& rule) {
    std::string command = "tc qdisc add dev " + interface + " " + rule;
    return execute_tc_command(command);
}

bool NetemImpairment::remove_tc_rule(const std::string& interface) {
    std::string command = "tc qdisc del dev " + interface + " root";
    return execute_tc_command(command);
}

std::string NetemImpairment::generate_tc_command(const ImpairmentConfig& config) const {
    std::ostringstream oss;
    oss << "root netem";
    
    switch (config.type) {
        case ImpairmentType::DELAY:
            oss << " delay " << config.value << "ms";
            if (config.variation > 0) {
                oss << " " << config.variation << "ms";
            }
            break;
            
        case ImpairmentType::JITTER:
            oss << " delay " << config.value << "ms " << config.variation << "ms";
            break;
            
        case ImpairmentType::LOSS:
            oss << " loss " << config.value << "%";
            if (config.correlation > 0) {
                oss << " " << config.correlation << "%";
            }
            break;
            
        case ImpairmentType::DUPLICATION:
            oss << " duplicate " << config.value << "%";
            break;
            
        case ImpairmentType::REORDERING:
            oss << " reorder " << config.value << "%";
            break;
            
        case ImpairmentType::CORRUPTION:
            oss << " corrupt " << config.value << "%";
            break;
            
        case ImpairmentType::BANDWIDTH_LIMIT:
            oss << " rate " << config.value << "bps";
            break;
            
        case ImpairmentType::RATE_LIMIT:
            oss << " rate " << config.value << "bps";
            break;
            
        default:
            return "";
    }
    
    return oss.str();
}

bool NetemImpairment::execute_tc_command(const std::string& command) {
    int result = system(command.c_str());
    return result == 0;
}

void NetemImpairment::monitoring_loop() {
    while (monitoring_running_) {
        std::lock_guard<std::mutex> lock(interfaces_mutex_);
        
        for (auto& pair : interfaces_) {
            std::string stats = get_interface_stats(pair.first);
            if (!stats.empty()) {
                // Parse statistics and update interface
                // This would parse the tc output and update counters
                
                if (packet_processed_callback_) {
                    packet_processed_callback_(pair.first, 0); // Placeholder
                }
            }
        }
        
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

bool NetemImpairment::setup_interface(const std::string& interface) {
    // Bring interface up if it's down
    std::string up_command = "ip link set " + interface + " up";
    if (!execute_tc_command(up_command)) {
        std::cerr << "Failed to bring up interface: " << interface << std::endl;
        return false;
    }
    
    return true;
}

bool NetemImpairment::teardown_interface(const std::string& interface) {
    // Remove all tc rules
    remove_tc_rule(interface);
    return true;
}

std::string NetemImpairment::get_interface_stats(const std::string& interface) {
    std::string command = "tc -s qdisc show dev " + interface;
    
    // Execute command and capture output
    FILE* pipe = popen(command.c_str(), "r");
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

} // namespace router_sim
