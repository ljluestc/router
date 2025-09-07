#include "netem_impairments.h"
#include <iostream>
#include <cstdlib>
#include <sstream>
#include <fstream>

namespace RouterSim {

NetemImpairments::NetemImpairments() : initialized_(false) {}

NetemImpairments::~NetemImpairments() {
    shutdown();
}

bool NetemImpairments::initialize() {
    if (initialized_) {
        return true;
    }
    
    // Check if tc and netem are available
    int result = std::system("which tc > /dev/null 2>&1");
    if (result != 0) {
        std::cerr << "tc (traffic control) not found. Please install iproute2." << std::endl;
        return false;
    }
    
    // Check if netem module is loaded
    result = std::system("lsmod | grep sch_netem > /dev/null 2>&1");
    if (result != 0) {
        std::cerr << "netem module not loaded. Please run: sudo modprobe sch_netem" << std::endl;
        return false;
    }
    
    initialized_ = true;
    std::cout << "Netem impairments initialized" << std::endl;
    return true;
}

void NetemImpairments::shutdown() {
    if (!initialized_) {
        return;
    }
    
    remove_all_impairments();
    initialized_ = false;
    std::cout << "Netem impairments shutdown" << std::endl;
}

bool NetemImpairments::apply_impairments(const std::string& interface, const NetemConfig& config) {
    if (!initialized_) {
        return false;
    }
    
    // Remove existing impairments first
    remove_impairments(interface);
    
    // Build netem command
    std::ostringstream cmd;
    cmd << "sudo tc qdisc add dev " << interface << " root netem";
    
    // Add delay
    if (config.delay_ms > 0) {
        cmd << " delay " << config.delay_ms << "ms";
        if (config.jitter_ms > 0) {
            cmd << " " << config.jitter_ms << "ms";
            if (config.delay_correlation > 0) {
                cmd << " " << config.delay_correlation;
            }
        }
    }
    
    // Add loss
    if (config.loss_percent > 0) {
        cmd << " loss " << config.loss_percent << "%";
        if (config.loss_correlation > 0) {
            cmd << " " << config.loss_correlation;
        }
    }
    
    // Add duplicate
    if (config.duplicate_percent > 0) {
        cmd << " duplicate " << config.duplicate_percent << "%";
    }
    
    // Add reorder
    if (config.reorder_percent > 0) {
        cmd << " reorder " << config.reorder_percent << "%";
        if (config.reorder_correlation > 0) {
            cmd << " " << config.reorder_correlation;
        }
    }
    
    // Add corruption
    if (config.corruption_percent > 0) {
        cmd << " corrupt " << config.corruption_percent << "%";
        if (config.corruption_correlation > 0) {
            cmd << " " << config.corruption_correlation;
        }
    }
    
    // Add rate limiting
    if (config.rate_bps > 0) {
        cmd << " rate " << config.rate_bps;
        if (config.packet_limit > 0) {
            cmd << " limit " << config.packet_limit;
        }
    }
    
    int result = std::system(cmd.str().c_str());
    if (result == 0) {
        active_impairments_[interface] = config;
        interface_stats_[interface] = ImpairmentStats{};
        std::cout << "Applied impairments to interface " << interface << std::endl;
        return true;
    }
    
    std::cerr << "Failed to apply impairments to interface " << interface << std::endl;
    return false;
}

bool NetemImpairments::remove_impairments(const std::string& interface) {
    if (!initialized_) {
        return false;
    }
    
    std::ostringstream cmd;
    cmd << "sudo tc qdisc del dev " << interface << " root 2>/dev/null";
    
    int result = std::system(cmd.str().c_str());
    
    // Remove from tracking even if command fails (interface might not have impairments)
    active_impairments_.erase(interface);
    interface_stats_.erase(interface);
    
    return true; // Always return true for removal
}

bool NetemImpairments::update_impairments(const std::string& interface, const NetemConfig& config) {
    return apply_impairments(interface, config);
}

bool NetemImpairments::apply_impairments_batch(const std::map<std::string, NetemConfig>& configs) {
    if (!initialized_) {
        return false;
    }
    
    bool all_success = true;
    
    for (const auto& pair : configs) {
        if (!apply_impairments(pair.first, pair.second)) {
            all_success = false;
        }
    }
    
    return all_success;
}

bool NetemImpairments::remove_all_impairments() {
    if (!initialized_) {
        return true;
    }
    
    bool all_success = true;
    
    for (const auto& pair : active_impairments_) {
        if (!remove_impairments(pair.first)) {
            all_success = false;
        }
    }
    
    return all_success;
}

bool NetemImpairments::is_impairment_active(const std::string& interface) const {
    return active_impairments_.find(interface) != active_impairments_.end();
}

std::vector<std::string> NetemImpairments::get_impaired_interfaces() const {
    std::vector<std::string> interfaces;
    
    for (const auto& pair : active_impairments_) {
        interfaces.push_back(pair.first);
    }
    
    return interfaces;
}

NetemConfig NetemImpairments::get_impairment_config(const std::string& interface) const {
    auto it = active_impairments_.find(interface);
    if (it != active_impairments_.end()) {
        return it->second;
    }
    
    return NetemConfig{}; // Return default config
}

NetemImpairments::ImpairmentStats NetemImpairments::get_interface_stats(const std::string& interface) const {
    auto it = interface_stats_.find(interface);
    if (it != interface_stats_.end()) {
        return it->second;
    }
    
    return ImpairmentStats{}; // Return default stats
}

bool NetemImpairments::execute_tc_command(const std::string& command) {
    int result = std::system(command.c_str());
    return result == 0;
}

bool NetemImpairments::setup_qdisc(const std::string& interface, const NetemConfig& config) {
    return apply_impairments(interface, config);
}

bool NetemImpairments::remove_qdisc(const std::string& interface) {
    return remove_impairments(interface);
}

} // namespace RouterSim
