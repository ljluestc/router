#include "netem_impairments.h"
#include <iostream>
#include <sstream>
#include <random>
#include <algorithm>
#include <cstdlib>
#include <fstream>

namespace RouterSim {

// NetemCommandGenerator implementation
std::string NetemCommandGenerator::generate_delay_command(const NetemConfig& config) {
    std::ostringstream cmd;
    cmd << "tc qdisc add dev " << config.interface 
        << " root netem delay " << config.value << "ms";
    
    if (config.variation > 0) {
        cmd << " " << config.variation << "ms";
    }
    
    if (config.correlation > 0) {
        cmd << " " << config.correlation << "%";
    }
    
    if (!config.distribution.empty()) {
        cmd << " distribution " << config.distribution;
    }
    
    return cmd.str();
}

std::string NetemCommandGenerator::generate_jitter_command(const NetemConfig& config) {
    std::ostringstream cmd;
    cmd << "tc qdisc add dev " << config.interface 
        << " root netem delay " << config.value << "ms " << config.variation << "ms";
    
    if (config.correlation > 0) {
        cmd << " " << config.correlation << "%";
    }
    
    return cmd.str();
}

std::string NetemCommandGenerator::generate_loss_command(const NetemConfig& config) {
    std::ostringstream cmd;
    cmd << "tc qdisc add dev " << config.interface 
        << " root netem loss " << config.value << "%";
    
    if (config.correlation > 0) {
        cmd << " " << config.correlation << "%";
    }
    
    return cmd.str();
}

std::string NetemCommandGenerator::generate_duplication_command(const NetemConfig& config) {
    std::ostringstream cmd;
    cmd << "tc qdisc add dev " << config.interface 
        << " root netem duplicate " << config.value << "%";
    
    if (config.correlation > 0) {
        cmd << " " << config.correlation << "%";
    }
    
    return cmd.str();
}

std::string NetemCommandGenerator::generate_reordering_command(const NetemConfig& config) {
    std::ostringstream cmd;
    cmd << "tc qdisc add dev " << config.interface 
        << " root netem delay " << config.value << "ms reorder " << config.variation << "%";
    
    if (config.correlation > 0) {
        cmd << " " << config.correlation << "%";
    }
    
    return cmd.str();
}

std::string NetemCommandGenerator::generate_corruption_command(const NetemConfig& config) {
    std::ostringstream cmd;
    cmd << "tc qdisc add dev " << config.interface 
        << " root netem corrupt " << config.value << "%";
    
    if (config.correlation > 0) {
        cmd << " " << config.correlation << "%";
    }
    
    return cmd.str();
}

std::string NetemCommandGenerator::generate_bandwidth_command(const NetemConfig& config) {
    std::ostringstream cmd;
    cmd << "tc qdisc add dev " << config.interface 
        << " root tbf rate " << config.value << "kbit burst " << config.variation << "kbit latency 50ms";
    
    return cmd.str();
}

std::string NetemCommandGenerator::generate_combined_command(const std::vector<NetemConfig>& configs) {
    if (configs.empty()) {
        return "";
    }
    
    std::ostringstream cmd;
    cmd << "tc qdisc add dev " << configs[0].interface << " root netem";
    
    for (const auto& config : configs) {
        switch (config.type) {
            case ImpairmentType::DELAY:
                cmd << " delay " << config.value << "ms";
                if (config.variation > 0) {
                    cmd << " " << config.variation << "ms";
                }
                break;
            case ImpairmentType::LOSS:
                cmd << " loss " << config.value << "%";
                break;
            case ImpairmentType::DUPLICATION:
                cmd << " duplicate " << config.value << "%";
                break;
            case ImpairmentType::REORDERING:
                cmd << " reorder " << config.value << "%";
                break;
            case ImpairmentType::CORRUPTION:
                cmd << " corrupt " << config.value << "%";
                break;
            default:
                break;
        }
        
        if (config.correlation > 0) {
            cmd << " " << config.correlation << "%";
        }
    }
    
    return cmd.str();
}

std::string NetemCommandGenerator::generate_clear_command(const std::string& interface) {
    return "tc qdisc del dev " + interface + " root";
}

} // namespace RouterSim