#include "frr_integration.h"
#include <iostream>
#include <sstream>

namespace router_sim {

FRRBGP::FRRBGP(std::shared_ptr<FRRControlPlane> control_plane)
    : control_plane_(control_plane), running_(false) {
}

bool FRRBGP::initialize(const std::map<std::string, std::string>& config) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    config_ = config;
    
    // Set up BGP-specific configuration
    if (config.find("as_number") == config.end()) {
        config_["as_number"] = "65001";
    }
    if (config.find("router_id") == config.end()) {
        config_["router_id"] = "1.1.1.1";
    }
    if (config.find("hold_time") == config.end()) {
        config_["hold_time"] = "180";
    }
    if (config.find("keepalive") == config.end()) {
        config_["keepalive"] = "60";
    }
    
    return true;
}

bool FRRBGP::start() {
    if (running_) {
        return true;
    }
    
    if (!control_plane_) {
        std::cerr << "FRR control plane not available" << std::endl;
        return false;
    }
    
    // Enable BGP protocol in FRR
    if (!control_plane_->enable_protocol(FRRProtocol::BGP)) {
        std::cerr << "Failed to enable BGP protocol" << std::endl;
        return false;
    }
    
    // Apply BGP configuration
    std::map<std::string, std::string> bgp_config;
    {
        std::lock_guard<std::mutex> lock(config_mutex_);
        bgp_config = config_;
    }
    
    // Send BGP configuration to FRR
    FRRMessage message;
    message.type = FRRMessageType::CONFIG_UPDATE;
    message.protocol = FRRProtocol::BGP;
    message.data = "configure_bgp";
    
    for (const auto& pair : bgp_config) {
        message.attributes[pair.first] = pair.second;
    }
    
    if (!control_plane_->send_message(message)) {
        std::cerr << "Failed to send BGP configuration" << std::endl;
        return false;
    }
    
    running_ = true;
    return true;
}

bool FRRBGP::stop() {
    if (!running_) {
        return true;
    }
    
    if (control_plane_) {
        control_plane_->disable_protocol(FRRProtocol::BGP);
    }
    
    running_ = false;
    return true;
}

bool FRRBGP::is_running() const {
    return running_;
}

bool FRRBGP::add_neighbor(const std::string& address, const std::map<std::string, std::string>& config) {
    if (!running_ || !control_plane_) {
        return false;
    }
    
    std::map<std::string, std::string> neighbor_config = config;
    neighbor_config["protocol"] = "bgp";
    
    return control_plane_->add_neighbor(address, FRRProtocol::BGP, neighbor_config);
}

bool FRRBGP::remove_neighbor(const std::string& address) {
    if (!running_ || !control_plane_) {
        return false;
    }
    
    return control_plane_->remove_neighbor(address, FRRProtocol::BGP);
}

std::vector<NeighborInfo> FRRBGP::get_neighbors() const {
    if (!control_plane_) {
        return {};
    }
    
    return control_plane_->get_neighbors(FRRProtocol::BGP);
}

bool FRRBGP::is_neighbor_established(const std::string& address) const {
    auto neighbors = get_neighbors();
    for (const auto& neighbor : neighbors) {
        if (neighbor.address == address) {
            return neighbor.is_established();
        }
    }
    return false;
}

bool FRRBGP::advertise_route(const RouteInfo& route) {
    if (!running_ || !control_plane_) {
        return false;
    }
    
    // Add BGP-specific attributes
    RouteInfo bgp_route = route;
    bgp_route.protocol = "bgp";
    
    return control_plane_->add_route(bgp_route);
}

bool FRRBGP::withdraw_route(const std::string& destination, uint8_t prefix_length) {
    if (!running_ || !control_plane_) {
        return false;
    }
    
    return control_plane_->remove_route(destination, prefix_length);
}

std::vector<RouteInfo> FRRBGP::get_routes() const {
    if (!control_plane_) {
        return {};
    }
    
    return control_plane_->get_routes(FRRProtocol::BGP);
}

bool FRRBGP::update_config(const std::map<std::string, std::string>& config) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    
    for (const auto& pair : config) {
        config_[pair.first] = pair.second;
    }
    
    // If running, apply the new configuration
    if (running_ && control_plane_) {
        FRRMessage message;
        message.type = FRRMessageType::CONFIG_UPDATE;
        message.protocol = FRRProtocol::BGP;
        message.data = "update_config";
        
        for (const auto& pair : config_) {
            message.attributes[pair.first] = pair.second;
        }
        
        return control_plane_->send_message(message);
    }
    
    return true;
}

std::map<std::string, std::string> FRRBGP::get_config() const {
    std::lock_guard<std::mutex> lock(config_mutex_);
    return config_;
}

ProtocolStatistics FRRBGP::get_statistics() const {
    ProtocolStatistics stats;
    
    if (control_plane_) {
        auto frr_stats = control_plane_->get_protocol_statistics(FRRProtocol::BGP);
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

void FRRBGP::set_route_update_callback(RouteUpdateCallback callback) {
    if (control_plane_) {
        control_plane_->set_route_update_callback(callback);
    }
}

void FRRBGP::set_neighbor_update_callback(NeighborUpdateCallback callback) {
    if (control_plane_) {
        control_plane_->set_neighbor_update_callback(callback);
    }
}

} // namespace router_sim
