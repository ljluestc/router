#include "netem/impairments.h"
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <chrono>
#include <random>
#include <thread>

namespace RouterSim {

NetemImpairments::NetemImpairments() : running_(false), initialized_(false) {
    stats_.reset();
}

NetemImpairments::~NetemImpairments() {
    stop();
}

bool NetemImpairments::initialize() {
    if (initialized_) {
        return true;
    }
    
    // Check if tc/netem is available
    int result = std::system("which tc > /dev/null 2>&1");
    if (result != 0) {
        std::cerr << "tc (traffic control) not found. Please install iproute2." << std::endl;
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
    clear_all_impairments();
    
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
    
    if (config.distribution != "uniform") {
        cmd << " distribution " << config.distribution;
    }
    
    int result = std::system(cmd.str().c_str());
    if (result == 0) {
        impairments_[interface].delay = config;
        std::cout << "Added delay impairment to " << interface 
                  << " (delay: " << config.delay_ms << "ms, jitter: " << config.jitter_ms << "ms)" << std::endl;
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
    
    if (config.loss_type == "random") {
        cmd << config.loss_percentage << "%";
    } else if (config.loss_type == "state") {
        cmd << "state " << config.p13 << "% " << config.p31 << "% " 
            << config.p32 << "% " << config.p23 << "% " << config.p14 << "%";
    } else if (config.loss_type == "gemodel") {
        cmd << "gemodel " << config.p << " " << config.r << " " 
            << config.h << " " << config.k << " " << config.one << " " << config.two;
    }
    
    int result = std::system(cmd.str().c_str());
    if (result == 0) {
        impairments_[interface].loss = config;
        std::cout << "Added loss impairment to " << interface 
                  << " (type: " << config.loss_type << ", percentage: " << config.loss_percentage << "%)" << std::endl;
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
        impairments_[interface].duplicate = config;
        std::cout << "Added duplicate impairment to " << interface 
                  << " (percentage: " << config.duplicate_percentage << "%)" << std::endl;
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
        impairments_[interface].corrupt = config;
        std::cout << "Added corrupt impairment to " << interface 
                  << " (percentage: " << config.corrupt_percentage << "%)" << std::endl;
        return true;
    }
    
    return false;
}

bool NetemImpairments::add_reorder(const std::string& interface, const ReorderConfig& config) {
    if (!running_) {
        return false;
    }
    
    std::ostringstream cmd;
    cmd << "tc qdisc add dev " << interface << " root netem reorder " 
        << config.reorder_percentage << "%";
    
    if (config.gap > 0) {
        cmd << " gap " << config.gap;
    }
    
    int result = std::system(cmd.str().c_str());
    if (result == 0) {
        impairments_[interface].reorder = config;
        std::cout << "Added reorder impairment to " << interface 
                  << " (percentage: " << config.reorder_percentage << "%, gap: " << config.gap << ")" << std::endl;
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
        << config.rate << " burst " << config.burst << " latency " << config.latency << "ms";
    
    int result = std::system(cmd.str().c_str());
    if (result == 0) {
        impairments_[interface].rate_limit = config;
        std::cout << "Added rate limit impairment to " << interface 
                  << " (rate: " << config.rate << ", burst: " << config.burst << ")" << std::endl;
        return true;
    }
    
    return false;
}

bool NetemImpairments::remove_impairment(const std::string& interface, const std::string& type) {
    if (!running_) {
        return false;
    }
    
    std::ostringstream cmd;
    cmd << "tc qdisc del dev " << interface << " root";
    
    int result = std::system(cmd.str().c_str());
    if (result == 0) {
        // Remove from our tracking
        auto it = impairments_.find(interface);
        if (it != impairments_.end()) {
            if (type == "delay") {
                it->second.delay.reset();
            } else if (type == "loss") {
                it->second.loss.reset();
            } else if (type == "duplicate") {
                it->second.duplicate.reset();
            } else if (type == "corrupt") {
                it->second.corrupt.reset();
            } else if (type == "reorder") {
                it->second.reorder.reset();
            } else if (type == "rate_limit") {
                it->second.rate_limit.reset();
            }
        }
        
        std::cout << "Removed " << type << " impairment from " << interface << std::endl;
        return true;
    }
    
    return false;
}

bool NetemImpairments::clear_interface_impairments(const std::string& interface) {
    if (!running_) {
        return false;
    }
    
    std::ostringstream cmd;
    cmd << "tc qdisc del dev " << interface << " root";
    
    int result = std::system(cmd.str().c_str());
    if (result == 0) {
        impairments_.erase(interface);
        std::cout << "Cleared all impairments from " << interface << std::endl;
        return true;
    }
    
    return false;
}

bool NetemImpairments::clear_all_impairments() {
    if (!running_) {
        return true;
    }
    
    for (const auto& [interface, _] : impairments_) {
        clear_interface_impairments(interface);
    }
    
    impairments_.clear();
    std::cout << "Cleared all impairments" << std::endl;
    return true;
}

std::vector<std::string> NetemImpairments::get_interfaces() const {
    std::vector<std::string> interfaces;
    for (const auto& [interface, _] : impairments_) {
        interfaces.push_back(interface);
    }
    return interfaces;
}

std::map<std::string, ImpairmentConfig> NetemImpairments::get_impairments() const {
    return impairments_;
}

ImpairmentConfig NetemImpairments::get_interface_impairments(const std::string& interface) const {
    auto it = impairments_.find(interface);
    if (it != impairments_.end()) {
        return it->second;
    }
    return ImpairmentConfig();
}

bool NetemImpairments::simulate_network_conditions(const std::string& scenario) {
    if (!running_) {
        return false;
    }
    
    if (scenario == "high_latency") {
        // Simulate high latency network
        DelayConfig delay_config;
        delay_config.delay_ms = 200;
        delay_config.jitter_ms = 50;
        delay_config.distribution = "normal";
        
        for (const auto& [interface, _] : impairments_) {
            add_delay(interface, delay_config);
        }
        
    } else if (scenario == "packet_loss") {
        // Simulate packet loss
        LossConfig loss_config;
        loss_config.loss_type = "random";
        loss_config.loss_percentage = 5.0;
        
        for (const auto& [interface, _] : impairments_) {
            add_loss(interface, loss_config);
        }
        
    } else if (scenario == "unreliable_network") {
        // Simulate unreliable network with multiple impairments
        DelayConfig delay_config;
        delay_config.delay_ms = 100;
        delay_config.jitter_ms = 25;
        
        LossConfig loss_config;
        loss_config.loss_type = "random";
        loss_config.loss_percentage = 2.0;
        
        DuplicateConfig duplicate_config;
        duplicate_config.duplicate_percentage = 1.0;
        
        for (const auto& [interface, _] : impairments_) {
            add_delay(interface, delay_config);
            add_loss(interface, loss_config);
            add_duplicate(interface, duplicate_config);
        }
        
    } else if (scenario == "bandwidth_limited") {
        // Simulate bandwidth-limited network
        RateLimitConfig rate_config;
        rate_config.rate = "1mbit";
        rate_config.burst = 100000;
        rate_config.latency = 50;
        
        for (const auto& [interface, _] : impairments_) {
            add_rate_limit(interface, rate_config);
        }
    }
    
    std::cout << "Applied network scenario: " << scenario << std::endl;
    return true;
}

std::map<std::string, uint64_t> NetemImpairments::get_statistics() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    return {
        {"impairments_applied", stats_.impairments_applied},
        {"impairments_removed", stats_.impairments_removed},
        {"interfaces_affected", stats_.interfaces_affected},
        {"scenarios_executed", stats_.scenarios_executed}
    };
}

void NetemImpairments::reset_statistics() {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    stats_.reset();
}

bool NetemImpairments::validate_interface(const std::string& interface) {
    // Check if interface exists
    std::ostringstream cmd;
    cmd << "ip link show " << interface << " > /dev/null 2>&1";
    
    int result = std::system(cmd.str().c_str());
    return result == 0;
}

bool NetemImpairments::apply_impairment_sequence(const std::string& interface, 
                                                const std::vector<ImpairmentStep>& sequence) {
    if (!running_) {
        return false;
    }
    
    for (const auto& step : sequence) {
        std::this_thread::sleep_for(std::chrono::milliseconds(step.delay_ms));
        
        switch (step.type) {
            case ImpairmentType::DELAY:
                if (!add_delay(interface, step.delay_config)) {
                    return false;
                }
                break;
                
            case ImpairmentType::LOSS:
                if (!add_loss(interface, step.loss_config)) {
                    return false;
                }
                break;
                
            case ImpairmentType::DUPLICATE:
                if (!add_duplicate(interface, step.duplicate_config)) {
                    return false;
                }
                break;
                
            case ImpairmentType::CORRUPT:
                if (!add_corrupt(interface, step.corrupt_config)) {
                    return false;
                }
                break;
                
            case ImpairmentType::REORDER:
                if (!add_reorder(interface, step.reorder_config)) {
                    return false;
                }
                break;
                
            case ImpairmentType::RATE_LIMIT:
                if (!add_rate_limit(interface, step.rate_limit_config)) {
                    return false;
                }
                break;
        }
    }
    
    return true;
}

} // namespace RouterSim