#include "frr_integration.h"
#include <iostream>

namespace router_sim {

FRRISIS::FRRISIS(std::shared_ptr<FRRControlPlane> control_plane)
    : control_plane_(control_plane), running_(false) {
}

bool FRRISIS::initialize(const std::map<std::string, std::string>& config) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    config_ = config;
    
    // Set up ISIS-specific configuration
    if (config.find("system_id") == config.end()) {
        config_["system_id"] = "0000.0000.0001";
    }
    if (config.find("area_id") == config.end()) {
        config_["area_id"] = "49.0001";
    }
    if (config.find("net_id") == config.end()) {
        config_["net_id"] = "49.0001.0000.0000.0001.00";
    }
    if (config.find("level") == config.end()) {
        config_["level"] = "level-2";
    }
    if (config.find("metric") == config.end()) {
        config_["metric"] = "10";
    }
    
    return true;
}

bool FRRISIS::start() {
    if (running_) {
        return true;
    }
    
    if (!control_plane_) {
        std::cerr << "FRR control plane not available" << std::endl;
        return false;
    }
    
    // Enable ISIS protocol in FRR
    if (!control_plane_->enable_protocol(FRRProtocol::ISIS)) {
        std::cerr << "Failed to enable ISIS protocol" << std::endl;
        return false;
    }
    
    // Apply ISIS configuration
    std::map<std::string, std::string> isis_config;
    {
        std::lock_guard<std::mutex> lock(config_mutex_);
        isis_config = config_;
    }
    
    // Send ISIS configuration to FRR
    FRRMessage message;
    message.type = FRRMessageType::CONFIG_UPDATE;
    message.protocol = FRRProtocol::ISIS;
    message.data = "configure_isis";
    
    for (const auto& pair : isis_config) {
        message.attributes[pair.first] = pair.second;
    }
    
    if (!control_plane_->send_message(message)) {
        std::cerr << "Failed to send ISIS configuration" << std::endl;
        return false;
    }
    
    running_ = true;
    return true;
}

bool FRRISIS::stop() {
    if (!running_) {
        return true;
    }
    
    if (control_plane_) {
        control_plane_->disable_protocol(FRRProtocol::ISIS);
    }
    
    running_ = false;
    return true;
}

bool FRRISIS::is_running() const {
    return running_;
}

bool FRRISIS::add_neighbor(const std::string& address, const std::map<std::string, std::string>& config) {
    if (!running_ || !control_plane_) {
        return false;
    }
    
    std::map<std::string, std::string> neighbor_config = config;
    neighbor_config["protocol"] = "isis";
    
    return control_plane_->add_neighbor(address, FRRProtocol::ISIS, neighbor_config);
}

bool FRRISIS::remove_neighbor(const std::string& address) {
    if (!running_ || !control_plane_) {
        return false;
    }
    
    return control_plane_->remove_neighbor(address, FRRProtocol::ISIS);
}

std::vector<NeighborInfo> FRRISIS::get_neighbors() const {
    if (!control_plane_) {
        return {};
    }
    
    return control_plane_->get_neighbors(FRRProtocol::ISIS);
}

bool FRRISIS::is_neighbor_established(const std::string& address) const {
    auto neighbors = get_neighbors();
    for (const auto& neighbor : neighbors) {
        if (neighbor.address == address) {
            return neighbor.is_established();
        }
    }
    return false;
}

bool FRRISIS::advertise_route(const RouteInfo& route) {
    if (!running_ || !control_plane_) {
        return false;
    }
    
    // Add ISIS-specific attributes
    RouteInfo isis_route = route;
    isis_route.protocol = "isis";
    
    return control_plane_->add_route(isis_route);
}

bool FRRISIS::withdraw_route(const std::string& destination, uint8_t prefix_length) {
    if (!running_ || !control_plane_) {
        return false;
    }
    
    return control_plane_->remove_route(destination, prefix_length);
}

std::vector<RouteInfo> FRRISIS::get_routes() const {
    if (!control_plane_) {
        return {};
    }
    
    return control_plane_->get_routes(FRRProtocol::ISIS);
}

bool FRRISIS::update_config(const std::map<std::string, std::string>& config) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    
    for (const auto& pair : config) {
        config_[pair.first] = pair.second;
    }
    
    // If running, apply the new configuration
    if (running_ && control_plane_) {
        FRRMessage message;
        message.type = FRRMessageType::CONFIG_UPDATE;
        message.protocol = FRRProtocol::ISIS;
        message.data = "update_config";
        
        for (const auto& pair : config_) {
            message.attributes[pair.first] = pair.second;
        }
        
        return control_plane_->send_message(message);
    }
    
    return true;
}

std::map<std::string, std::string> FRRISIS::get_config() const {
    std::lock_guard<std::mutex> lock(config_mutex_);
    return config_;
}

ProtocolStatistics FRRISIS::get_statistics() const {
    ProtocolStatistics stats;
    
    if (control_plane_) {
        auto frr_stats = control_plane_->get_protocol_statistics(FRRProtocol::ISIS);
        stats.messages_sent = frr_stats.messages_sent;
        stats.messages_received = frr_stats.messages_received;
        stats.routes_advertised = frr_stats.routes_installed;
        stats.routes_withdrawn = frr_stats.routes_removed;
        stats.neighbor_up_count = frr_stats.neighbors_established;
        stats.neighbor_down_count = frr_stats.neighbors_lost;
        stats.errors = frr_stats.errors;
        stats.last_update = frr_stats.last_update;
    }
    
    return stats;
}

void FRRISIS::set_route_update_callback(RouteUpdateCallback callback) {
    if (control_plane_) {
        control_plane_->set_route_update_callback(callback);
    }
}

void FRRISIS::set_neighbor_update_callback(NeighborUpdateCallback callback) {
    if (control_plane_) {
        control_plane_->set_neighbor_update_callback(callback);
    }
}

} // namespace router_sim
