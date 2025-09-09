#include "network_impairments.h"
#include <iostream>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>
#include <fstream>

namespace router_sim {

// Enhanced NetEm Impairments Implementation
NetEmImpairments::NetEmImpairments() : enabled_(false), interface_("lo") {
}

NetEmImpairments::~NetEmImpairments() {
    cleanup();
}

bool NetEmImpairments::initialize(const std::string& interface) {
    interface_ = interface;
    
    // Check if tc (traffic control) is available
    if (system("which tc > /dev/null 2>&1") != 0) {
        std::cerr << "Error: tc (traffic control) not found. Please install iproute2." << std::endl;
        return false;
    }
    
    // Check if the interface exists
    std::ifstream iface_file("/sys/class/net/" + interface_ + "/operstate");
    if (!iface_file.good()) {
        std::cerr << "Error: Interface " << interface_ << " not found." << std::endl;
        return false;
    }
    
    std::cout << "NetEm impairments initialized for interface: " << interface_ << std::endl;
    return true;
}

bool NetEmImpairments::enable() {
    if (enabled_) {
        return true;
    }
    
    // Create root qdisc if it doesn't exist
    std::string cmd = "tc qdisc add dev " + interface_ + " root handle 1: htb default 30";
    if (system(cmd.c_str()) != 0) {
        // Root qdisc might already exist, try to replace it
        cmd = "tc qdisc replace dev " + interface_ + " root handle 1: htb default 30";
        if (system(cmd.c_str()) != 0) {
            std::cerr << "Failed to create root qdisc" << std::endl;
            return false;
        }
    }
    
    enabled_ = true;
    std::cout << "NetEm impairments enabled on interface: " << interface_ << std::endl;
    return true;
}

bool NetEmImpairments::disable() {
    if (!enabled_) {
        return true;
    }
    
    // Remove all qdiscs from the interface
    std::string cmd = "tc qdisc del dev " + interface_ + " root 2>/dev/null";
    system(cmd.c_str());
    
    enabled_ = false;
    std::cout << "NetEm impairments disabled on interface: " << interface_ << std::endl;
    return true;
}

bool NetEmImpairments::is_enabled() const {
    return enabled_;
}

bool NetEmImpairments::set_delay(const DelayConfig& config) {
    if (!enabled_) {
        std::cerr << "NetEm impairments not enabled" << std::endl;
        return false;
    }
    
    std::stringstream cmd;
    cmd << "tc qdisc add dev " << interface_ << " parent 1: handle 10: netem delay ";
    cmd << config.delay_ms << "ms";
    
    if (config.jitter_ms > 0) {
        cmd << " " << config.jitter_ms << "ms";
    }
    
    if (config.distribution != DelayDistribution::UNIFORM) {
        cmd << " distribution " << get_distribution_string(config.distribution);
    }
    
    if (config.correlation > 0.0) {
        cmd << " " << static_cast<int>(config.correlation * 100) << "%";
    }
    
    std::cout << "Setting delay: " << cmd.str() << std::endl;
    
    if (system(cmd.str().c_str()) != 0) {
        std::cerr << "Failed to set delay" << std::endl;
        return false;
    }
    
    delay_config_ = config;
    return true;
}

bool NetEmImpairments::set_loss(const LossConfig& config) {
    if (!enabled_) {
        std::cerr << "NetEm impairments not enabled" << std::endl;
        return false;
    }
    
    std::stringstream cmd;
    cmd << "tc qdisc add dev " << interface_ << " parent 1: handle 20: netem loss ";
    cmd << static_cast<int>(config.loss_percentage * 100) << "%";
    
    if (config.correlation > 0.0) {
        cmd << " " << static_cast<int>(config.correlation * 100) << "%";
    }
    
    if (config.random) {
        cmd << " random";
    }
    
    std::cout << "Setting loss: " << cmd.str() << std::endl;
    
    if (system(cmd.str().c_str()) != 0) {
        std::cerr << "Failed to set loss" << std::endl;
        return false;
    }
    
    loss_config_ = config;
    return true;
}

bool NetEmImpairments::set_duplicate(const DuplicateConfig& config) {
    if (!enabled_) {
        std::cerr << "NetEm impairments not enabled" << std::endl;
        return false;
    }
    
    std::stringstream cmd;
    cmd << "tc qdisc add dev " << interface_ << " parent 1: handle 30: netem duplicate ";
    cmd << static_cast<int>(config.duplicate_percentage * 100) << "%";
    
    if (config.correlation > 0.0) {
        cmd << " " << static_cast<int>(config.correlation * 100) << "%";
    }
    
    std::cout << "Setting duplicate: " << cmd.str() << std::endl;
    
    if (system(cmd.str().c_str()) != 0) {
        std::cerr << "Failed to set duplicate" << std::endl;
        return false;
    }
    
    duplicate_config_ = config;
    return true;
}

bool NetEmImpairments::set_corrupt(const CorruptConfig& config) {
    if (!enabled_) {
        std::cerr << "NetEm impairments not enabled" << std::endl;
        return false;
    }
    
    std::stringstream cmd;
    cmd << "tc qdisc add dev " << interface_ << " parent 1: handle 40: netem corrupt ";
    cmd << static_cast<int>(config.corrupt_percentage * 100) << "%";
    
    if (config.correlation > 0.0) {
        cmd << " " << static_cast<int>(config.correlation * 100) << "%";
    }
    
    std::cout << "Setting corrupt: " << cmd.str() << std::endl;
    
    if (system(cmd.str().c_str()) != 0) {
        std::cerr << "Failed to set corrupt" << std::endl;
        return false;
    }
    
    corrupt_config_ = config;
    return true;
}

bool NetEmImpairments::set_reorder(const ReorderConfig& config) {
    if (!enabled_) {
        std::cerr << "NetEm impairments not enabled" << std::endl;
        return false;
    }
    
    std::stringstream cmd;
    cmd << "tc qdisc add dev " << interface_ << " parent 1: handle 50: netem reorder ";
    cmd << static_cast<int>(config.reorder_percentage * 100) << "%";
    
    if (config.gap > 0) {
        cmd << " gap " << config.gap;
    }
    
    if (config.correlation > 0.0) {
        cmd << " " << static_cast<int>(config.correlation * 100) << "%";
    }
    
    std::cout << "Setting reorder: " << cmd.str() << std::endl;
    
    if (system(cmd.str().c_str()) != 0) {
        std::cerr << "Failed to set reorder" << std::endl;
        return false;
    }
    
    reorder_config_ = config;
    return true;
}

bool NetEmImpairments::set_rate_limit(const RateLimitConfig& config) {
    if (!enabled_) {
        std::cerr << "NetEm impairments not enabled" << std::endl;
        return false;
    }
    
    std::stringstream cmd;
    cmd << "tc qdisc add dev " << interface_ << " parent 1: handle 60: tbf ";
    cmd << "rate " << config.rate_kbps << "kbit ";
    cmd << "burst " << config.burst_kb << "kbit ";
    cmd << "latency " << config.latency_ms << "ms";
    
    std::cout << "Setting rate limit: " << cmd.str() << std::endl;
    
    if (system(cmd.str().c_str()) != 0) {
        std::cerr << "Failed to set rate limit" << std::endl;
        return false;
    }
    
    rate_limit_config_ = config;
    return true;
}

bool NetEmImpairments::set_bandwidth(const BandwidthConfig& config) {
    if (!enabled_) {
        std::cerr << "NetEm impairments not enabled" << std::endl;
        return false;
    }
    
    std::stringstream cmd;
    cmd << "tc qdisc add dev " << interface_ << " parent 1: handle 70: netem ";
    cmd << "rate " << config.bandwidth_kbps << "kbit";
    
    if (config.packet_overhead > 0) {
        cmd << " overhead " << config.packet_overhead;
    }
    
    if (config.cell_size > 0) {
        cmd << " cell " << config.cell_size;
    }
    
    std::cout << "Setting bandwidth: " << cmd.str() << std::endl;
    
    if (system(cmd.str().c_str()) != 0) {
        std::cerr << "Failed to set bandwidth" << std::endl;
        return false;
    }
    
    bandwidth_config_ = config;
    return true;
}

bool NetEmImpairments::apply_scenario(const ImpairmentScenario& scenario) {
    if (!enabled_) {
        if (!enable()) {
            return false;
        }
    }
    
    bool success = true;
    
    // Apply delay if configured
    if (scenario.delay.enabled) {
        if (!set_delay(scenario.delay)) {
            success = false;
        }
    }
    
    // Apply loss if configured
    if (scenario.loss.enabled) {
        if (!set_loss(scenario.loss)) {
            success = false;
        }
    }
    
    // Apply duplicate if configured
    if (scenario.duplicate.enabled) {
        if (!set_duplicate(scenario.duplicate)) {
            success = false;
        }
    }
    
    // Apply corrupt if configured
    if (scenario.corrupt.enabled) {
        if (!set_corrupt(scenario.corrupt)) {
            success = false;
        }
    }
    
    // Apply reorder if configured
    if (scenario.reorder.enabled) {
        if (!set_reorder(scenario.reorder)) {
            success = false;
        }
    }
    
    // Apply rate limit if configured
    if (scenario.rate_limit.enabled) {
        if (!set_rate_limit(scenario.rate_limit)) {
            success = false;
        }
    }
    
    // Apply bandwidth if configured
    if (scenario.bandwidth.enabled) {
        if (!set_bandwidth(scenario.bandwidth)) {
            success = false;
        }
    }
    
    if (success) {
        std::cout << "Applied impairment scenario: " << scenario.name << std::endl;
    } else {
        std::cerr << "Failed to apply some impairments in scenario: " << scenario.name << std::endl;
    }
    
    return success;
}

NetEmImpairments::Statistics NetEmImpairments::get_statistics() const {
    Statistics stats;
    stats.enabled = enabled_;
    stats.interface = interface_;
    stats.delay_config = delay_config_;
    stats.loss_config = loss_config_;
    stats.duplicate_config = duplicate_config_;
    stats.corrupt_config = corrupt_config_;
    stats.reorder_config = reorder_config_;
    stats.rate_limit_config = rate_limit_config_;
    stats.bandwidth_config = bandwidth_config_;
    
    // Get current qdisc statistics
    std::string cmd = "tc -s qdisc show dev " + interface_;
    FILE* pipe = popen(cmd.c_str(), "r");
    if (pipe) {
        char buffer[1024];
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            stats.qdisc_info += buffer;
        }
        pclose(pipe);
    }
    
    return stats;
}

void NetEmImpairments::cleanup() {
    if (enabled_) {
        disable();
    }
}

std::string NetEmImpairments::get_distribution_string(DelayDistribution dist) const {
    switch (dist) {
        case DelayDistribution::UNIFORM:
            return "uniform";
        case DelayDistribution::NORMAL:
            return "normal";
        case DelayDistribution::PARETO:
            return "pareto";
        case DelayDistribution::PARETONORMAL:
            return "paretonormal";
        default:
            return "uniform";
    }
}

// Network Impairment Manager
NetworkImpairmentManager::NetworkImpairmentManager() {
}

NetworkImpairmentManager::~NetworkImpairmentManager() {
    for (auto& pair : impairments_) {
        delete pair.second;
    }
}

bool NetworkImpairmentManager::add_interface(const std::string& interface) {
    if (impairments_.find(interface) != impairments_.end()) {
        return true; // Already exists
    }
    
    NetEmImpairments* imp = new NetEmImpairments();
    if (!imp->initialize(interface)) {
        delete imp;
        return false;
    }
    
    impairments_[interface] = imp;
    std::cout << "Added interface for impairments: " << interface << std::endl;
    return true;
}

bool NetworkImpairmentManager::remove_interface(const std::string& interface) {
    auto it = impairments_.find(interface);
    if (it == impairments_.end()) {
        return false;
    }
    
    delete it->second;
    impairments_.erase(it);
    std::cout << "Removed interface from impairments: " << interface << std::endl;
    return true;
}

NetEmImpairments* NetworkImpairmentManager::get_impairments(const std::string& interface) {
    auto it = impairments_.find(interface);
    return (it != impairments_.end()) ? it->second : nullptr;
}

bool NetworkImpairmentManager::apply_scenario_to_interface(const std::string& interface, const ImpairmentScenario& scenario) {
    NetEmImpairments* imp = get_impairments(interface);
    if (!imp) {
        std::cerr << "Interface not found: " << interface << std::endl;
        return false;
    }
    
    return imp->apply_scenario(scenario);
}

std::vector<std::string> NetworkImpairmentManager::get_interfaces() const {
    std::vector<std::string> interfaces;
    for (const auto& pair : impairments_) {
        interfaces.push_back(pair.first);
    }
    return interfaces;
}

NetworkImpairmentManager::GlobalStatistics NetworkImpairmentManager::get_global_statistics() const {
    GlobalStatistics stats;
    stats.total_interfaces = impairments_.size();
    stats.enabled_interfaces = 0;
    
    for (const auto& pair : impairments_) {
        if (pair.second->is_enabled()) {
            stats.enabled_interfaces++;
        }
    }
    
    return stats;
}

} // namespace router_sim
