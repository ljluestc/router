#include "impairments.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <sys/wait.h>
#include <unistd.h>

namespace RouterSim {

NetworkImpairment::NetworkImpairment() : enabled_(false) {
}

NetworkImpairment::~NetworkImpairment() {
    disable();
}

bool NetworkImpairment::enable(const std::string& interface, const ImpairmentConfig& config) {
    if (enabled_) {
        disable();
    }
    
    interface_ = interface;
    config_ = config;
    
    // Apply tc/netem rules
    if (!applyTCRules()) {
        return false;
    }
    
    enabled_ = true;
    return true;
}

void NetworkImpairment::disable() {
    if (!enabled_) {
        return;
    }
    
    // Remove tc rules
    removeTCRules();
    
    enabled_ = false;
}

bool NetworkImpairment::isEnabled() const {
    return enabled_;
}

const ImpairmentConfig& NetworkImpairment::getConfig() const {
    return config_;
}

std::string NetworkImpairment::getInterface() const {
    return interface_;
}

bool NetworkImpairment::applyTCRules() {
    // First, clear existing rules
    removeTCRules();
    
    // Build tc command
    std::stringstream cmd;
    cmd << "tc qdisc add dev " << interface_ << " root netem";
    
    // Add delay
    if (config_.delay_ms > 0) {
        cmd << " delay " << config_.delay_ms << "ms";
        if (config_.delay_jitter_ms > 0) {
            cmd << " " << config_.delay_jitter_ms << "ms";
        }
        if (config_.delay_correlation > 0) {
            cmd << " " << config_.delay_correlation << "%";
        }
    }
    
    // Add packet loss
    if (config_.loss_percent > 0) {
        cmd << " loss " << config_.loss_percent << "%";
        if (config_.loss_correlation > 0) {
            cmd << " " << config_.loss_correlation << "%";
        }
    }
    
    // Add duplicate packets
    if (config_.duplicate_percent > 0) {
        cmd << " duplicate " << config_.duplicate_percent << "%";
    }
    
    // Add reordering
    if (config_.reorder_percent > 0) {
        cmd << " reorder " << config_.reorder_percent << "%";
        if (config_.reorder_correlation > 0) {
            cmd << " " << config_.reorder_correlation << "%";
        }
    }
    
    // Add corruption
    if (config_.corrupt_percent > 0) {
        cmd << " corrupt " << config_.corrupt_percent << "%";
    }
    
    // Add bandwidth limiting
    if (config_.bandwidth_kbps > 0) {
        cmd << " rate " << config_.bandwidth_kbps << "kbit";
    }
    
    // Execute tc command
    if (!executeCommand(cmd.str())) {
        std::cerr << "Failed to apply tc rules: " << cmd.str() << std::endl;
        return false;
    }
    
    std::cout << "Applied tc rules: " << cmd.str() << std::endl;
    return true;
}

void NetworkImpairment::removeTCRules() {
    std::stringstream cmd;
    cmd << "tc qdisc del dev " << interface_ << " root";
    
    // Execute command (ignore errors as rules might not exist)
    executeCommand(cmd.str());
}

bool NetworkImpairment::executeCommand(const std::string& command) {
    int result = system(command.c_str());
    return WEXITSTATUS(result) == 0;
}

// ImpairmentManager implementation
ImpairmentManager::ImpairmentManager() {
}

ImpairmentManager::~ImpairmentManager() {
    // Disable all impairments
    for (auto& pair : impairments_) {
        pair.second->disable();
    }
}

bool ImpairmentManager::addImpairment(const std::string& name, const std::string& interface, const ImpairmentConfig& config) {
    if (impairments_.find(name) != impairments_.end()) {
        return false; // Already exists
    }
    
    auto impairment = std::make_unique<NetworkImpairment>();
    if (!impairment->enable(interface, config)) {
        return false;
    }
    
    impairments_[name] = std::move(impairment);
    return true;
}

bool ImpairmentManager::removeImpairment(const std::string& name) {
    auto it = impairments_.find(name);
    if (it == impairments_.end()) {
        return false;
    }
    
    it->second->disable();
    impairments_.erase(it);
    return true;
}

bool ImpairmentManager::updateImpairment(const std::string& name, const ImpairmentConfig& config) {
    auto it = impairments_.find(name);
    if (it == impairments_.end()) {
        return false;
    }
    
    return it->second->enable(it->second->getInterface(), config);
}

bool ImpairmentManager::enableImpairment(const std::string& name) {
    auto it = impairments_.find(name);
    if (it == impairments_.end()) {
        return false;
    }
    
    return it->second->enable(it->second->getInterface(), it->second->getConfig());
}

bool ImpairmentManager::disableImpairment(const std::string& name) {
    auto it = impairments_.find(name);
    if (it == impairments_.end()) {
        return false;
    }
    
    it->second->disable();
    return true;
}

std::vector<std::string> ImpairmentManager::getImpairmentNames() const {
    std::vector<std::string> names;
    for (const auto& pair : impairments_) {
        names.push_back(pair.first);
    }
    return names;
}

bool ImpairmentManager::isImpairmentEnabled(const std::string& name) const {
    auto it = impairments_.find(name);
    if (it == impairments_.end()) {
        return false;
    }
    
    return it->second->isEnabled();
}

ImpairmentConfig ImpairmentManager::getImpairmentConfig(const std::string& name) const {
    auto it = impairments_.find(name);
    if (it == impairments_.end()) {
        return ImpairmentConfig();
    }
    
    return it->second->getConfig();
}

std::map<std::string, ImpairmentConfig> ImpairmentManager::getAllImpairments() const {
    std::map<std::string, ImpairmentConfig> configs;
    for (const auto& pair : impairments_) {
        configs[pair.first] = pair.second->getConfig();
    }
    return configs;
}

bool ImpairmentManager::loadConfiguration(const std::string& config_file) {
    // TODO: Implement YAML configuration loading
    return true;
}

bool ImpairmentManager::saveConfiguration(const std::string& config_file) {
    // TODO: Implement YAML configuration saving
    return true;
}

// Scenario-based impairment testing
bool ImpairmentManager::loadScenario(const std::string& scenario_file) {
    // TODO: Implement scenario loading from YAML
    return true;
}

bool ImpairmentManager::runScenario(const std::string& scenario_name) {
    // TODO: Implement scenario execution
    return true;
}

std::vector<std::string> ImpairmentManager::getAvailableScenarios() const {
    // TODO: Return available scenarios
    return {};
}

} // namespace RouterSim