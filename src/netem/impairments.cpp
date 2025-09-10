#include "netem/impairments.h"
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <chrono>
#include <random>
#include <algorithm>

namespace RouterSim {

NetemImpairments::NetemImpairments() : initialized_(false), running_(false) {
    // Initialize random number generator
    rng_.seed(std::chrono::steady_clock::now().time_since_epoch().count());
}

NetemImpairments::~NetemImpairments() {
    cleanup();
}

bool NetemImpairments::initialize() {
    if (initialized_) {
        return true;
    }

    // Check if tc/netem is available
    int result = std::system("which tc > /dev/null 2>&1");
    if (result != 0) {
        std::cerr << "tc (traffic control) not found. Please install iproute2 package." << std::endl;
        return false;
    }

    initialized_ = true;
    std::cout << "Netem impairments initialized successfully" << std::endl;
    return true;
}

bool NetemImpairments::start() {
    if (!initialized_) {
        if (!initialize()) {
            return false;
        }
    }

    if (running_) {
        return true;
    }

    running_ = true;
    std::cout << "Netem impairments started" << std::endl;
    return true;
}

bool NetemImpairments::stop() {
    if (!running_) {
        return true;
    }

    // Remove all impairments
    cleanup();
    
    running_ = false;
    std::cout << "Netem impairments stopped" << std::endl;
    return true;
}

bool NetemImpairments::is_running() const {
    return running_;
}

bool NetemImpairments::add_delay(const std::string& interface, const DelayConfig& config) {
    if (!running_) {
        return false;
    }

    std::ostringstream cmd;
    cmd << "tc qdisc add dev " << interface << " root netem delay " 
        << config.delay_ms << "ms";
    
    if (config.jitter_ms > 0) {
        cmd << " " << config.jitter_ms << "ms";
    }
    
    if (config.distribution != DelayDistribution::UNIFORM) {
        cmd << " distribution " << get_distribution_string(config.distribution);
    }

    int result = std::system(cmd.str().c_str());
    if (result == 0) {
        active_impairments_[interface].delay = config;
        std::cout << "Added delay impairment to interface " << interface << std::endl;
        return true;
    }

    return false;
}

bool NetemImpairments::add_loss(const std::string& interface, const LossConfig& config) {
    if (!running_) {
        return false;
    }

    std::ostringstream cmd;
    cmd << "tc qdisc add dev " << interface << " root netem loss ";
    
    if (config.loss_type == LossType::RANDOM) {
        cmd << config.loss_percentage << "%";
    } else if (config.loss_type == LossType::STATE) {
        cmd << "state " << config.p13 << "% " << config.p31 << "% " 
            << config.p32 << "% " << config.p23 << "% " << config.p14 << "%";
    } else if (config.loss_type == LossType::GEMODEL) {
        cmd << "gemodel " << config.p << " " << config.r << " " 
            << config.h << " " << config.k << " " << config.1_minus_h;
    }

    int result = std::system(cmd.str().c_str());
    if (result == 0) {
        active_impairments_[interface].loss = config;
        std::cout << "Added loss impairment to interface " << interface << std::endl;
        return true;
    }

    return false;
}

bool NetemImpairments::add_duplicate(const std::string& interface, const DuplicateConfig& config) {
    if (!running_) {
        return false;
    }

    std::ostringstream cmd;
    cmd << "tc qdisc add dev " << interface << " root netem duplicate " 
        << config.duplicate_percentage << "%";

    int result = std::system(cmd.str().c_str());
    if (result == 0) {
        active_impairments_[interface].duplicate = config;
        std::cout << "Added duplicate impairment to interface " << interface << std::endl;
        return true;
    }

    return false;
}

bool NetemImpairments::add_corrupt(const std::string& interface, const CorruptConfig& config) {
    if (!running_) {
        return false;
    }

    std::ostringstream cmd;
    cmd << "tc qdisc add dev " << interface << " root netem corrupt " 
        << config.corrupt_percentage << "%";

    int result = std::system(cmd.str().c_str());
    if (result == 0) {
        active_impairments_[interface].corrupt = config;
        std::cout << "Added corrupt impairment to interface " << interface << std::endl;
        return true;
    }

    return false;
}

bool NetemImpairments::add_reorder(const std::string& interface, const ReorderConfig& config) {
    if (!running_) {
        return false;
    }

    std::ostringstream cmd;
    cmd << "tc qdisc add dev " << interface << " root netem reorder ";
    
    if (config.reorder_type == ReorderType::PERCENTAGE) {
        cmd << config.reorder_percentage << "%";
    } else if (config.reorder_type == ReorderType::GAP) {
        cmd << "gap " << config.gap;
    }

    int result = std::system(cmd.str().c_str());
    if (result == 0) {
        active_impairments_[interface].reorder = config;
        std::cout << "Added reorder impairment to interface " << interface << std::endl;
        return true;
    }

    return false;
}

bool NetemImpairments::add_rate_limit(const std::string& interface, const RateLimitConfig& config) {
    if (!running_) {
        return false;
    }

    std::ostringstream cmd;
    cmd << "tc qdisc add dev " << interface << " root tbf rate " 
        << config.rate_kbps << "kbit burst " << config.burst_kb << "kbit";

    int result = std::system(cmd.str().c_str());
    if (result == 0) {
        active_impairments_[interface].rate_limit = config;
        std::cout << "Added rate limit impairment to interface " << interface << std::endl;
        return true;
    }

    return false;
}

bool NetemImpairments::remove_impairment(const std::string& interface) {
    if (!running_) {
        return false;
    }

    std::ostringstream cmd;
    cmd << "tc qdisc del dev " << interface << " root";

    int result = std::system(cmd.str().c_str());
    if (result == 0) {
        active_impairments_.erase(interface);
        std::cout << "Removed impairments from interface " << interface << std::endl;
        return true;
    }

    return false;
}

bool NetemImpairments::clear_all_impairments() {
    if (!running_) {
        return false;
    }

    bool success = true;
    for (const auto& [interface, _] : active_impairments_) {
        if (!remove_impairment(interface)) {
            success = false;
        }
    }

    return success;
}

std::vector<std::string> NetemImpairments::get_impaired_interfaces() const {
    std::vector<std::string> interfaces;
    for (const auto& [interface, _] : active_impairments_) {
        interfaces.push_back(interface);
    }
    return interfaces;
}

ImpairmentInfo NetemImpairments::get_interface_impairments(const std::string& interface) const {
    auto it = active_impairments_.find(interface);
    if (it != active_impairments_.end()) {
        return it->second;
    }
    return ImpairmentInfo{};
}

bool NetemImpairments::apply_scenario(const std::string& scenario_name) {
    // Predefined scenarios
    if (scenario_name == "high_latency") {
        return apply_high_latency_scenario();
    } else if (scenario_name == "packet_loss") {
        return apply_packet_loss_scenario();
    } else if (scenario_name == "unreliable_network") {
        return apply_unreliable_network_scenario();
    } else if (scenario_name == "congested_network") {
        return apply_congested_network_scenario();
    }

    return false;
}

bool NetemImpairments::apply_high_latency_scenario() {
    // Apply high latency to all interfaces
    DelayConfig delay_config;
    delay_config.delay_ms = 100;
    delay_config.jitter_ms = 10;
    delay_config.distribution = DelayDistribution::NORMAL;

    for (const auto& interface : get_available_interfaces()) {
        add_delay(interface, delay_config);
    }

    return true;
}

bool NetemImpairments::apply_packet_loss_scenario() {
    // Apply packet loss to all interfaces
    LossConfig loss_config;
    loss_config.loss_type = LossType::RANDOM;
    loss_config.loss_percentage = 5.0;

    for (const auto& interface : get_available_interfaces()) {
        add_loss(interface, loss_config);
    }

    return true;
}

bool NetemImpairments::apply_unreliable_network_scenario() {
    // Apply multiple impairments for unreliable network
    for (const auto& interface : get_available_interfaces()) {
        // Add delay
        DelayConfig delay_config;
        delay_config.delay_ms = 50;
        delay_config.jitter_ms = 5;
        add_delay(interface, delay_config);

        // Add loss
        LossConfig loss_config;
        loss_config.loss_type = LossType::RANDOM;
        loss_config.loss_percentage = 2.0;
        add_loss(interface, loss_config);

        // Add duplicates
        DuplicateConfig dup_config;
        dup_config.duplicate_percentage = 1.0;
        add_duplicate(interface, dup_config);
    }

    return true;
}

bool NetemImpairments::apply_congested_network_scenario() {
    // Apply rate limiting for congested network
    RateLimitConfig rate_config;
    rate_config.rate_kbps = 1000; // 1 Mbps
    rate_config.burst_kb = 100;

    for (const auto& interface : get_available_interfaces()) {
        add_rate_limit(interface, rate_config);
    }

    return true;
}

std::vector<std::string> NetemImpairments::get_available_interfaces() const {
    std::vector<std::string> interfaces;
    
    // Get list of network interfaces
    std::ostringstream cmd;
    cmd << "ip link show | grep -E '^[0-9]+:' | cut -d: -f2 | tr -d ' '";
    
    FILE* pipe = popen(cmd.str().c_str(), "r");
    if (pipe) {
        char buffer[128];
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            std::string interface(buffer);
            interface.erase(interface.find_last_not_of(" \n\r\t") + 1);
            if (!interface.empty() && interface != "lo") {
                interfaces.push_back(interface);
            }
        }
        pclose(pipe);
    }

    return interfaces;
}

void NetemImpairments::cleanup() {
    // Remove all impairments
    for (const auto& [interface, _] : active_impairments_) {
        std::ostringstream cmd;
        cmd << "tc qdisc del dev " << interface << " root 2>/dev/null";
        std::system(cmd.str().c_str());
    }
    
    active_impairments_.clear();
}

std::string NetemImpairments::get_distribution_string(DelayDistribution distribution) const {
    switch (distribution) {
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

// NetemManager implementation
NetemManager::NetemManager() : initialized_(false), running_(false) {
}

NetemManager::~NetemManager() {
    stop();
}

bool NetemManager::initialize() {
    if (initialized_) {
        return true;
    }

    initialized_ = true;
    return true;
}

bool NetemManager::start() {
    if (!initialized_) {
        if (!initialize()) {
            return false;
        }
    }

    if (running_) {
        return true;
    }

    running_ = true;
    return true;
}

bool NetemManager::stop() {
    if (!running_) {
        return true;
    }

    // Stop all impairment instances
    for (auto& [name, impairment] : impairments_) {
        impairment->stop();
    }

    running_ = false;
    return true;
}

bool NetemManager::add_impairment(const std::string& name, std::unique_ptr<NetemImpairments> impairment) {
    if (!running_) {
        return false;
    }

    if (impairments_.find(name) != impairments_.end()) {
        return false; // Name already exists
    }

    impairments_[name] = std::move(impairment);
    return true;
}

bool NetemManager::remove_impairment(const std::string& name) {
    auto it = impairments_.find(name);
    if (it == impairments_.end()) {
        return false;
    }

    it->second->stop();
    impairments_.erase(it);
    return true;
}

NetemImpairments* NetemManager::get_impairment(const std::string& name) const {
    auto it = impairments_.find(name);
    if (it == impairments_.end()) {
        return nullptr;
    }
    return it->second.get();
}

std::vector<std::string> NetemManager::get_impairment_names() const {
    std::vector<std::string> names;
    for (const auto& [name, _] : impairments_) {
        names.push_back(name);
    }
    return names;
}

} // namespace RouterSim
