#include "network_impairments.h"
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
#include <sys/ioctl.h>
#include <net/if.h>

namespace RouterSim {

NetworkImpairments::NetworkImpairments()
    : enabled_(false)
    , total_packets_processed_(0)
    , total_bytes_processed_(0)
    , packets_dropped_(0)
    , bytes_dropped_(0)
{
}

NetworkImpairments::~NetworkImpairments() {
    cleanup();
}

bool NetworkImpairments::initialize() {
    std::cout << "Initializing network impairments..." << std::endl;
    
    // Check if tc (traffic control) is available
    if (!checkTCAvailability()) {
        std::cerr << "tc (traffic control) is not available" << std::endl;
        return false;
    }
    
    // Check if netem module is loaded
    if (!checkNetemModule()) {
        std::cerr << "netem module is not loaded" << std::endl;
        return false;
    }
    
    enabled_ = true;
    std::cout << "Network impairments initialized successfully" << std::endl;
    
    return true;
}

bool NetworkImpairments::applyDelay(const std::string& interface, uint32_t delay_ms, uint32_t jitter_ms) {
    if (!enabled_) {
        std::cerr << "Network impairments not initialized" << std::endl;
        return false;
    }
    
    std::string command = "tc qdisc add dev " + interface + " root netem delay " + 
                         std::to_string(delay_ms) + "ms";
    
    if (jitter_ms > 0) {
        command += " " + std::to_string(jitter_ms) + "ms";
    }
    
    std::string output = executeCommand(command);
    
    if (!output.empty() && output.find("error") != std::string::npos) {
        std::cerr << "Failed to apply delay: " << output << std::endl;
        return false;
    }
    
    std::cout << "Applied delay: " << delay_ms << "ms (+" << jitter_ms << "ms jitter) to " << interface << std::endl;
    return true;
}

bool NetworkImpairments::applyLoss(const std::string& interface, double loss_percentage) {
    if (!enabled_) {
        std::cerr << "Network impairments not initialized" << std::endl;
        return false;
    }
    
    if (loss_percentage < 0.0 || loss_percentage > 100.0) {
        std::cerr << "Invalid loss percentage: " << loss_percentage << std::endl;
        return false;
    }
    
    std::string command = "tc qdisc add dev " + interface + " root netem loss " + 
                         std::to_string(loss_percentage) + "%";
    
    std::string output = executeCommand(command);
    
    if (!output.empty() && output.find("error") != std::string::npos) {
        std::cerr << "Failed to apply loss: " << output << std::endl;
        return false;
    }
    
    std::cout << "Applied loss: " << loss_percentage << "% to " << interface << std::endl;
    return true;
}

bool NetworkImpairments::applyBandwidth(const std::string& interface, uint64_t bandwidth_bps) {
    if (!enabled_) {
        std::cerr << "Network impairments not initialized" << std::endl;
        return false;
    }
    
    // Convert to appropriate units for tc
    std::string bandwidth_str;
    if (bandwidth_bps >= 1000000000) {
        bandwidth_str = std::to_string(bandwidth_bps / 1000000000) + "gbit";
    } else if (bandwidth_bps >= 1000000) {
        bandwidth_str = std::to_string(bandwidth_bps / 1000000) + "mbit";
    } else if (bandwidth_bps >= 1000) {
        bandwidth_str = std::to_string(bandwidth_bps / 1000) + "kbit";
    } else {
        bandwidth_str = std::to_string(bandwidth_bps) + "bit";
    }
    
    std::string command = "tc qdisc add dev " + interface + " root tbf rate " + bandwidth_str + 
                         " burst 32kbit latency 400ms";
    
    std::string output = executeCommand(command);
    
    if (!output.empty() && output.find("error") != std::string::npos) {
        std::cerr << "Failed to apply bandwidth limit: " << output << std::endl;
        return false;
    }
    
    std::cout << "Applied bandwidth limit: " << bandwidth_str << " to " << interface << std::endl;
    return true;
}

bool NetworkImpairments::applyDuplication(const std::string& interface, double duplication_percentage) {
    if (!enabled_) {
        std::cerr << "Network impairments not initialized" << std::endl;
        return false;
    }
    
    if (duplication_percentage < 0.0 || duplication_percentage > 100.0) {
        std::cerr << "Invalid duplication percentage: " << duplication_percentage << std::endl;
        return false;
    }
    
    std::string command = "tc qdisc add dev " + interface + " root netem duplicate " + 
                         std::to_string(duplication_percentage) + "%";
    
    std::string output = executeCommand(command);
    
    if (!output.empty() && output.find("error") != std::string::npos) {
        std::cerr << "Failed to apply duplication: " << output << std::endl;
        return false;
    }
    
    std::cout << "Applied duplication: " << duplication_percentage << "% to " << interface << std::endl;
    return true;
}

bool NetworkImpairments::applyReordering(const std::string& interface, double reorder_percentage, uint32_t gap) {
    if (!enabled_) {
        std::cerr << "Network impairments not initialized" << std::endl;
        return false;
    }
    
    if (reorder_percentage < 0.0 || reorder_percentage > 100.0) {
        std::cerr << "Invalid reorder percentage: " << reorder_percentage << std::endl;
        return false;
    }
    
    std::string command = "tc qdisc add dev " + interface + " root netem reorder " + 
                         std::to_string(reorder_percentage) + "% " + std::to_string(gap);
    
    std::string output = executeCommand(command);
    
    if (!output.empty() && output.find("error") != std::string::npos) {
        std::cerr << "Failed to apply reordering: " << output << std::endl;
        return false;
    }
    
    std::cout << "Applied reordering: " << reorder_percentage << "% (gap: " << gap << ") to " << interface << std::endl;
    return true;
}

bool NetworkImpairments::applyCorruption(const std::string& interface, double corruption_percentage) {
    if (!enabled_) {
        std::cerr << "Network impairments not initialized" << std::endl;
        return false;
    }
    
    if (corruption_percentage < 0.0 || corruption_percentage > 100.0) {
        std::cerr << "Invalid corruption percentage: " << corruption_percentage << std::endl;
        return false;
    }
    
    std::string command = "tc qdisc add dev " + interface + " root netem corrupt " + 
                         std::to_string(corruption_percentage) + "%";
    
    std::string output = executeCommand(command);
    
    if (!output.empty() && output.find("error") != std::string::npos) {
        std::cerr << "Failed to apply corruption: " << output << std::endl;
        return false;
    }
    
    std::cout << "Applied corruption: " << corruption_percentage << "% to " << interface << std::endl;
    return true;
}

bool NetworkImpairments::applyComplexImpairment(const std::string& interface, const ImpairmentConfig& config) {
    if (!enabled_) {
        std::cerr << "Network impairments not initialized" << std::endl;
        return false;
    }
    
    // Build complex netem command
    std::string command = "tc qdisc add dev " + interface + " root netem";
    
    if (config.delay_ms > 0) {
        command += " delay " + std::to_string(config.delay_ms) + "ms";
        if (config.jitter_ms > 0) {
            command += " " + std::to_string(config.jitter_ms) + "ms";
        }
    }
    
    if (config.loss_percentage > 0.0) {
        command += " loss " + std::to_string(config.loss_percentage) + "%";
    }
    
    if (config.duplication_percentage > 0.0) {
        command += " duplicate " + std::to_string(config.duplication_percentage) + "%";
    }
    
    if (config.reorder_percentage > 0.0) {
        command += " reorder " + std::to_string(config.reorder_percentage) + "% " + 
                  std::to_string(config.reorder_gap);
    }
    
    if (config.corruption_percentage > 0.0) {
        command += " corrupt " + std::to_string(config.corruption_percentage) + "%";
    }
    
    std::string output = executeCommand(command);
    
    if (!output.empty() && output.find("error") != std::string::npos) {
        std::cerr << "Failed to apply complex impairment: " << output << std::endl;
        return false;
    }
    
    std::cout << "Applied complex impairment to " << interface << std::endl;
    return true;
}

bool NetworkImpairments::clearImpairments(const std::string& interface) {
    if (!enabled_) {
        std::cerr << "Network impairments not initialized" << std::endl;
        return false;
    }
    
    std::string command = "tc qdisc del dev " + interface + " root";
    std::string output = executeCommand(command);
    
    // Don't treat "RTNETLINK answers: No such file or directory" as an error
    if (!output.empty() && output.find("error") != std::string::npos && 
        output.find("No such file or directory") == std::string::npos) {
        std::cerr << "Failed to clear impairments: " << output << std::endl;
        return false;
    }
    
    std::cout << "Cleared impairments on " << interface << std::endl;
    return true;
}

bool NetworkImpairments::clearAllImpairments() {
    if (!enabled_) {
        std::cerr << "Network impairments not initialized" << std::endl;
        return false;
    }
    
    // Get list of network interfaces
    std::vector<std::string> interfaces = getNetworkInterfaces();
    
    bool success = true;
    for (const auto& interface : interfaces) {
        if (!clearImpairments(interface)) {
            success = false;
        }
    }
    
    return success;
}

std::vector<std::string> NetworkImpairments::getNetworkInterfaces() const {
    std::vector<std::string> interfaces;
    
    // Read /proc/net/dev to get interface list
    std::ifstream file("/proc/net/dev");
    if (!file.is_open()) {
        return interfaces;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        // Skip header lines
        if (line.find("Inter-|") != std::string::npos || 
            line.find(" face |") != std::string::npos) {
            continue;
        }
        
        // Extract interface name
        size_t pos = line.find(':');
        if (pos != std::string::npos) {
            std::string interface = line.substr(0, pos);
            // Trim whitespace
            interface.erase(0, interface.find_first_not_of(" \t"));
            interface.erase(interface.find_last_not_of(" \t") + 1);
            
            if (!interface.empty() && interface != "lo") {
                interfaces.push_back(interface);
            }
        }
    }
    
    return interfaces;
}

std::string NetworkImpairments::getInterfaceStatus(const std::string& interface) const {
    std::string command = "tc qdisc show dev " + interface;
    return executeCommand(command);
}

NetworkImpairments::Statistics NetworkImpairments::getStatistics() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    Statistics stats;
    stats.enabled = enabled_;
    stats.total_packets_processed = total_packets_processed_;
    stats.total_bytes_processed = total_bytes_processed_;
    stats.packets_dropped = packets_dropped_;
    stats.bytes_dropped = bytes_dropped_;
    
    // Get interface statistics
    std::vector<std::string> interfaces = getNetworkInterfaces();
    for (const auto& interface : interfaces) {
        InterfaceStatistics if_stats;
        if_stats.interface_name = interface;
        if_stats.status = getInterfaceStatus(interface);
        stats.interface_stats.push_back(if_stats);
    }
    
    return stats;
}

void NetworkImpairments::reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    total_packets_processed_ = 0;
    total_bytes_processed_ = 0;
    packets_dropped_ = 0;
    bytes_dropped_ = 0;
}

void NetworkImpairments::cleanup() {
    if (enabled_) {
        clearAllImpairments();
        enabled_ = false;
    }
}

bool NetworkImpairments::checkTCAvailability() const {
    std::string command = "which tc";
    std::string output = executeCommand(command);
    return !output.empty();
}

bool NetworkImpairments::checkNetemModule() const {
    std::string command = "lsmod | grep sch_netem";
    std::string output = executeCommand(command);
    return !output.empty();
}

std::string NetworkImpairments::executeCommand(const std::string& command) const {
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

} // namespace RouterSim
