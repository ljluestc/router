#include "network_impairments.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

namespace router_sim {

NetworkImpairments::NetworkImpairments() : enabled_(false) {
}

NetworkImpairments::~NetworkImpairments() {
    clear_all_impairments();
}

bool NetworkImpairments::add_delay(const std::string& interface, uint32_t delay_ms, uint32_t jitter_ms) {
    std::string command = "tc qdisc add dev " + interface + " root netem delay " + 
                         std::to_string(delay_ms) + "ms";
    
    if (jitter_ms > 0) {
        command += " " + std::to_string(jitter_ms) + "ms";
    }

    if (!execute_tc_command(command)) {
        std::cerr << "Failed to add delay impairment to interface " << interface << std::endl;
        return false;
    }

    ImpairmentConfig config;
    config.interface = interface;
    config.type = "delay";
    config.parameters["delay"] = std::to_string(delay_ms);
    config.parameters["jitter"] = std::to_string(jitter_ms);
    
    impairments_[interface] = config;
    std::cout << "Added delay impairment: " << delay_ms << "ms (+" << jitter_ms << "ms jitter) to " << interface << std::endl;
    return true;
}

bool NetworkImpairments::add_loss(const std::string& interface, double loss_percentage) {
    std::string command = "tc qdisc add dev " + interface + " root netem loss " + 
                         std::to_string(loss_percentage) + "%";

    if (!execute_tc_command(command)) {
        std::cerr << "Failed to add loss impairment to interface " << interface << std::endl;
        return false;
    }

    ImpairmentConfig config;
    config.interface = interface;
    config.type = "loss";
    config.parameters["loss"] = std::to_string(loss_percentage);
    
    impairments_[interface] = config;
    std::cout << "Added loss impairment: " << loss_percentage << "% to " << interface << std::endl;
    return true;
}

bool NetworkImpairments::add_duplicate(const std::string& interface, double duplicate_percentage) {
    std::string command = "tc qdisc add dev " + interface + " root netem duplicate " + 
                         std::to_string(duplicate_percentage) + "%";

    if (!execute_tc_command(command)) {
        std::cerr << "Failed to add duplicate impairment to interface " << interface << std::endl;
        return false;
    }

    ImpairmentConfig config;
    config.interface = interface;
    config.type = "duplicate";
    config.parameters["duplicate"] = std::to_string(duplicate_percentage);
    
    impairments_[interface] = config;
    std::cout << "Added duplicate impairment: " << duplicate_percentage << "% to " << interface << std::endl;
    return true;
}

bool NetworkImpairments::add_corruption(const std::string& interface, double corruption_percentage) {
    std::string command = "tc qdisc add dev " + interface + " root netem corrupt " + 
                         std::to_string(corruption_percentage) + "%";

    if (!execute_tc_command(command)) {
        std::cerr << "Failed to add corruption impairment to interface " << interface << std::endl;
        return false;
    }

    ImpairmentConfig config;
    config.interface = interface;
    config.type = "corrupt";
    config.parameters["corrupt"] = std::to_string(corruption_percentage);
    
    impairments_[interface] = config;
    std::cout << "Added corruption impairment: " << corruption_percentage << "% to " << interface << std::endl;
    return true;
}

bool NetworkImpairments::add_reorder(const std::string& interface, double reorder_percentage, uint32_t gap) {
    std::string command = "tc qdisc add dev " + interface + " root netem reorder " + 
                         std::to_string(reorder_percentage) + "% gap " + std::to_string(gap);
    
    if (!execute_tc_command(command)) {
        std::cerr << "Failed to add reorder impairment to interface " << interface << std::endl;
        return false;
    }

    ImpairmentConfig config;
    config.interface = interface;
    config.type = "reorder";
    config.parameters["reorder"] = std::to_string(reorder_percentage);
    config.parameters["gap"] = std::to_string(gap);
    
    impairments_[interface] = config;
    std::cout << "Added reorder impairment: " << reorder_percentage << "% gap " << gap << " to " << interface << std::endl;
    return true;
}

bool NetworkImpairments::add_rate_limit(const std::string& interface, uint64_t rate_kbps) {
    std::string command = "tc qdisc add dev " + interface + " root tbf rate " + 
                         std::to_string(rate_kbps) + "kbit latency 50ms burst 1540";
    
    if (!execute_tc_command(command)) {
        std::cerr << "Failed to add rate limit impairment to interface " << interface << std::endl;
        return false;
    }

    ImpairmentConfig config;
    config.interface = interface;
    config.type = "rate_limit";
    config.parameters["rate"] = std::to_string(rate_kbps);
    
    impairments_[interface] = config;
    std::cout << "Added rate limit impairment: " << rate_kbps << " kbps to " << interface << std::endl;
    return true;
}

bool NetworkImpairments::remove_impairment(const std::string& interface) {
    std::string command = "tc qdisc del dev " + interface + " root";

    if (!execute_tc_command(command)) {
        std::cerr << "Failed to remove impairment from interface " << interface << std::endl;
        return false;
    }

    impairments_.erase(interface);
    std::cout << "Removed impairment from " << interface << std::endl;
    return true;
}

bool NetworkImpairments::clear_all_impairments() {
    bool success = true;
    
    for (const auto& pair : impairments_) {
        if (!remove_impairment(pair.first)) {
            success = false;
        }
    }
    
    impairments_.clear();
    return success;
}

std::vector<ImpairmentConfig> NetworkImpairments::get_impairments() const {
    std::vector<ImpairmentConfig> result;
    for (const auto& pair : impairments_) {
        result.push_back(pair.second);
    }
    return result;
}

ImpairmentConfig NetworkImpairments::get_impairment(const std::string& interface) const {
    auto it = impairments_.find(interface);
    if (it != impairments_.end()) {
        return it->second;
    }
    return ImpairmentConfig{};
}

bool NetworkImpairments::is_impairment_active(const std::string& interface) const {
    return impairments_.find(interface) != impairments_.end();
}

std::vector<std::string> NetworkImpairments::get_available_interfaces() const {
    std::vector<std::string> interfaces;
    
    std::ifstream file("/proc/net/dev");
    std::string line;
    
    // Skip header lines
    std::getline(file, line);
    std::getline(file, line);
    
    while (std::getline(file, line)) {
        size_t colon_pos = line.find(':');
        if (colon_pos != std::string::npos) {
            std::string interface = line.substr(0, colon_pos);
            // Remove leading whitespace
            interface.erase(0, interface.find_first_not_of(" \t"));
            interfaces.push_back(interface);
        }
    }
    
    return interfaces;
}

bool NetworkImpairments::execute_tc_command(const std::string& command) {
    int result = system(command.c_str());
    return WEXITSTATUS(result) == 0;
}

std::string NetworkImpairments::get_tc_status(const std::string& interface) const {
    std::string command = "tc qdisc show dev " + interface;
    std::string output;
    
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        return "";
    }
    
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        output += buffer;
    }
    pclose(pipe);
    
    return output;
}

NetworkImpairments::Statistics NetworkImpairments::get_statistics() const {
    Statistics stats;
    stats.total_impairments = impairments_.size();
    stats.enabled = enabled_;
    
    for (const auto& pair : impairments_) {
        const auto& config = pair.second;
        if (config.type == "delay") {
            stats.delay_impairments++;
        } else if (config.type == "loss") {
            stats.loss_impairments++;
        } else if (config.type == "duplicate") {
            stats.duplicate_impairments++;
        } else if (config.type == "corrupt") {
            stats.corruption_impairments++;
        } else if (config.type == "reorder") {
            stats.reorder_impairments++;
        } else if (config.type == "rate_limit") {
            stats.rate_limit_impairments++;
        }
    }
    
    return stats;
}

void NetworkImpairments::set_enabled(bool enabled) {
    enabled_ = enabled;
    if (!enabled) {
        clear_all_impairments();
    }
}

bool NetworkImpairments::is_enabled() const {
    return enabled_;
}

// Advanced impairment combinations
bool NetworkImpairments::add_combined_impairment(const std::string& interface, 
                                                const ImpairmentProfile& profile) {
    std::string command = "tc qdisc add dev " + interface + " root netem";
    
    // Add delay
    if (profile.delay_ms > 0) {
        command += " delay " + std::to_string(profile.delay_ms) + "ms";
        if (profile.jitter_ms > 0) {
            command += " " + std::to_string(profile.jitter_ms) + "ms";
        }
    }
    
    // Add loss
    if (profile.loss_percentage > 0) {
        command += " loss " + std::to_string(profile.loss_percentage) + "%";
    }
    
    // Add duplicate
    if (profile.duplicate_percentage > 0) {
        command += " duplicate " + std::to_string(profile.duplicate_percentage) + "%";
    }
    
    // Add corruption
    if (profile.corruption_percentage > 0) {
        command += " corrupt " + std::to_string(profile.corruption_percentage) + "%";
    }
    
    // Add reorder
    if (profile.reorder_percentage > 0) {
        command += " reorder " + std::to_string(profile.reorder_percentage) + "% gap " + 
                  std::to_string(profile.reorder_gap);
    }

    if (!execute_tc_command(command)) {
        std::cerr << "Failed to add combined impairment to interface " << interface << std::endl;
        return false;
    }

    ImpairmentConfig config;
    config.interface = interface;
    config.type = "combined";
    config.parameters["delay"] = std::to_string(profile.delay_ms);
    config.parameters["jitter"] = std::to_string(profile.jitter_ms);
    config.parameters["loss"] = std::to_string(profile.loss_percentage);
    config.parameters["duplicate"] = std::to_string(profile.duplicate_percentage);
    config.parameters["corrupt"] = std::to_string(profile.corruption_percentage);
    config.parameters["reorder"] = std::to_string(profile.reorder_percentage);
    config.parameters["reorder_gap"] = std::to_string(profile.reorder_gap);
    
    impairments_[interface] = config;
    std::cout << "Added combined impairment to " << interface << std::endl;
    return true;
}

} // namespace router_sim