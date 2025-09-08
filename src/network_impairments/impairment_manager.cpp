#include "network_impairments.h"
#include <iostream>

namespace router_sim {

NetworkImpairmentManager::NetworkImpairmentManager() {
    impairment_engine_ = std::make_unique<NetemImpairment>();
    load_preset_scenarios();
}

NetworkImpairmentManager::~NetworkImpairmentManager() {
    stop();
}

bool NetworkImpairmentManager::initialize(const std::map<std::string, std::string>& config) {
    if (!impairment_engine_) {
        return false;
    }
    
    return impairment_engine_->initialize(config);
}

bool NetworkImpairmentManager::start() {
    if (!impairment_engine_) {
        return false;
    }
    
    return impairment_engine_->start();
}

bool NetworkImpairmentManager::stop() {
    if (!impairment_engine_) {
        return true;
    }
    
    return impairment_engine_->stop();
}

bool NetworkImpairmentManager::is_running() const {
    if (!impairment_engine_) {
        return false;
    }
    
    return impairment_engine_->is_running();
}

bool NetworkImpairmentManager::add_impairment(const std::string& interface, const ImpairmentConfig& config) {
    if (!impairment_engine_) {
        return false;
    }
    
    // Ensure interface exists
    if (!add_interface(interface)) {
        return false;
    }
    
    return impairment_engine_->add_impairment(config);
}

bool NetworkImpairmentManager::remove_impairment(const std::string& interface, ImpairmentType type) {
    if (!impairment_engine_) {
        return false;
    }
    
    return impairment_engine_->remove_impairment(interface, type);
}

bool NetworkImpairmentManager::update_impairment(const std::string& interface, const ImpairmentConfig& config) {
    if (!impairment_engine_) {
        return false;
    }
    
    return impairment_engine_->update_impairment(config);
}

std::vector<ImpairmentConfig> NetworkImpairmentManager::get_impairments(const std::string& interface) const {
    if (!impairment_engine_) {
        return {};
    }
    
    return impairment_engine_->get_impairments(interface);
}

bool NetworkImpairmentManager::add_interface(const std::string& name) {
    if (!impairment_engine_) {
        return false;
    }
    
    return impairment_engine_->add_interface(name);
}

bool NetworkImpairmentManager::remove_interface(const std::string& name) {
    if (!impairment_engine_) {
        return false;
    }
    
    return impairment_engine_->remove_interface(name);
}

std::vector<NetworkInterface> NetworkImpairmentManager::get_interfaces() const {
    if (!impairment_engine_) {
        return {};
    }
    
    return impairment_engine_->get_interfaces();
}

NetworkInterface NetworkImpairmentManager::get_interface(const std::string& name) const {
    if (!impairment_engine_) {
        return NetworkInterface{};
    }
    
    return impairment_engine_->get_interface(name);
}

bool NetworkImpairmentManager::apply_scenario(const std::string& scenario_name) {
    auto it = scenarios_.find(scenario_name);
    if (it == scenarios_.end()) {
        std::cerr << "Scenario not found: " << scenario_name << std::endl;
        return false;
    }
    
    const auto& scenario_configs = it->second;
    bool all_success = true;
    
    for (const auto& config : scenario_configs) {
        if (!add_impairment(config.interface, config)) {
            std::cerr << "Failed to apply impairment for interface: " << config.interface << std::endl;
            all_success = false;
        }
    }
    
    return all_success;
}

bool NetworkImpairmentManager::remove_scenario(const std::string& scenario_name) {
    auto it = scenarios_.find(scenario_name);
    if (it == scenarios_.end()) {
        return false;
    }
    
    const auto& scenario_configs = it->second;
    bool all_success = true;
    
    for (const auto& config : scenario_configs) {
        if (!remove_impairment(config.interface, config.type)) {
            all_success = false;
        }
    }
    
    return all_success;
}

std::vector<std::string> NetworkImpairmentManager::get_available_scenarios() const {
    std::vector<std::string> scenario_names;
    for (const auto& pair : scenarios_) {
        scenario_names.push_back(pair.first);
    }
    return scenario_names;
}

std::map<std::string, ImpairmentStatistics> NetworkImpairmentManager::get_all_statistics() const {
    if (!impairment_engine_) {
        return {};
    }
    
    std::map<std::string, ImpairmentStatistics> stats;
    auto interfaces = get_interfaces();
    
    for (const auto& interface : interfaces) {
        stats[interface.name] = impairment_engine_->get_interface_statistics(interface.name);
    }
    
    return stats;
}

ImpairmentStatistics NetworkImpairmentManager::get_interface_statistics(const std::string& interface) const {
    if (!impairment_engine_) {
        return ImpairmentStatistics{};
    }
    
    return impairment_engine_->get_interface_statistics(interface);
}

void NetworkImpairmentManager::set_packet_processed_callback(std::function<void(const std::string&, size_t)> callback) {
    packet_processed_callback_ = callback;
    if (impairment_engine_) {
        impairment_engine_->set_packet_processed_callback(callback);
    }
}

void NetworkImpairmentManager::set_impairment_applied_callback(std::function<void(const std::string&, ImpairmentType)> callback) {
    impairment_applied_callback_ = callback;
    if (impairment_engine_) {
        impairment_engine_->set_impairment_applied_callback(callback);
    }
}

void NetworkImpairmentManager::load_preset_scenarios() {
    // High latency scenario
    scenarios_["high_latency"] = scenarios::high_latency("eth0");
    
    // Packet loss scenarios
    scenarios_["packet_loss_1%"] = scenarios::packet_loss("eth0", 1.0);
    scenarios_["packet_loss_5%"] = scenarios::packet_loss("eth0", 5.0);
    scenarios_["packet_loss_10%"] = scenarios::packet_loss("eth0", 10.0);
    
    // Jitter scenarios
    scenarios_["jitter_low"] = scenarios::jitter("eth0", 5.0);
    scenarios_["jitter_medium"] = scenarios::jitter("eth0", 20.0);
    scenarios_["jitter_high"] = scenarios::jitter("eth0", 50.0);
    
    // Bandwidth limitation scenarios
    scenarios_["bandwidth_1mbps"] = scenarios::bandwidth_limit("eth0", 1000000);
    scenarios_["bandwidth_10mbps"] = scenarios::bandwidth_limit("eth0", 10000000);
    scenarios_["bandwidth_100mbps"] = scenarios::bandwidth_limit("eth0", 100000000);
    
    // Network type scenarios
    scenarios_["mobile_network"] = scenarios::mobile_network("eth0");
    scenarios_["satellite_link"] = scenarios::satellite_link("eth0");
    scenarios_["congested_network"] = scenarios::congested_network("eth0");
}

} // namespace router_sim

// Preset Scenarios Implementation
namespace router_sim::scenarios {

std::vector<ImpairmentConfig> high_latency(const std::string& interface) {
    std::vector<ImpairmentConfig> configs;
    
    ImpairmentConfig config;
    config.type = ImpairmentType::DELAY;
    config.interface = interface;
    config.value = 100.0; // 100ms delay
    config.variation = 10.0; // 10ms jitter
    config.enabled = true;
    configs.push_back(config);
    
    return configs;
}

std::vector<ImpairmentConfig> packet_loss(const std::string& interface, double loss_percentage) {
    std::vector<ImpairmentConfig> configs;
    
    ImpairmentConfig config;
    config.type = ImpairmentType::LOSS;
    config.interface = interface;
    config.value = loss_percentage;
    config.correlation = 25; // 25% correlation
    config.enabled = true;
    configs.push_back(config);
    
    return configs;
}

std::vector<ImpairmentConfig> jitter(const std::string& interface, double jitter_ms) {
    std::vector<ImpairmentConfig> configs;
    
    ImpairmentConfig config;
    config.type = ImpairmentType::JITTER;
    config.interface = interface;
    config.value = 0.0; // Base delay
    config.variation = jitter_ms; // Jitter amount
    config.enabled = true;
    configs.push_back(config);
    
    return configs;
}

std::vector<ImpairmentConfig> bandwidth_limit(const std::string& interface, uint64_t bandwidth_bps) {
    std::vector<ImpairmentConfig> configs;
    
    ImpairmentConfig config;
    config.type = ImpairmentType::BANDWIDTH_LIMIT;
    config.interface = interface;
    config.value = static_cast<double>(bandwidth_bps);
    config.enabled = true;
    configs.push_back(config);
    
    return configs;
}

std::vector<ImpairmentConfig> mobile_network(const std::string& interface) {
    std::vector<ImpairmentConfig> configs;
    
    // Mobile network characteristics: variable delay, some packet loss, jitter
    ImpairmentConfig delay_config;
    delay_config.type = ImpairmentType::DELAY;
    delay_config.interface = interface;
    delay_config.value = 50.0; // 50ms base delay
    delay_config.variation = 20.0; // 20ms jitter
    delay_config.enabled = true;
    configs.push_back(delay_config);
    
    ImpairmentConfig loss_config;
    loss_config.type = ImpairmentType::LOSS;
    loss_config.interface = interface;
    loss_config.value = 2.0; // 2% packet loss
    loss_config.correlation = 30; // 30% correlation
    loss_config.enabled = true;
    configs.push_back(loss_config);
    
    return configs;
}

std::vector<ImpairmentConfig> satellite_link(const std::string& interface) {
    std::vector<ImpairmentConfig> configs;
    
    // Satellite link characteristics: high delay, some packet loss
    ImpairmentConfig delay_config;
    delay_config.type = ImpairmentType::DELAY;
    delay_config.interface = interface;
    delay_config.value = 500.0; // 500ms delay (typical satellite)
    delay_config.variation = 10.0; // 10ms jitter
    delay_config.enabled = true;
    configs.push_back(delay_config);
    
    ImpairmentConfig loss_config;
    loss_config.type = ImpairmentType::LOSS;
    loss_config.interface = interface;
    loss_config.value = 1.0; // 1% packet loss
    loss_config.correlation = 20; // 20% correlation
    loss_config.enabled = true;
    configs.push_back(loss_config);
    
    return configs;
}

std::vector<ImpairmentConfig> congested_network(const std::string& interface) {
    std::vector<ImpairmentConfig> configs;
    
    // Congested network characteristics: high packet loss, reordering, variable delay
    ImpairmentConfig delay_config;
    delay_config.type = ImpairmentType::DELAY;
    delay_config.interface = interface;
    delay_config.value = 100.0; // 100ms base delay
    delay_config.variation = 50.0; // 50ms jitter
    delay_config.enabled = true;
    configs.push_back(delay_config);
    
    ImpairmentConfig loss_config;
    loss_config.type = ImpairmentType::LOSS;
    loss_config.interface = interface;
    loss_config.value = 5.0; // 5% packet loss
    loss_config.correlation = 40; // 40% correlation
    loss_config.enabled = true;
    configs.push_back(loss_config);
    
    ImpairmentConfig reorder_config;
    reorder_config.type = ImpairmentType::REORDERING;
    reorder_config.interface = interface;
    reorder_config.value = 10.0; // 10% reordering
    reorder_config.enabled = true;
    configs.push_back(reorder_config);
    
    return configs;
}

} // namespace router_sim::scenarios
