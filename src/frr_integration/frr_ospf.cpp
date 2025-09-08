#include "frr_integration.h"
#include <iostream>

namespace router_sim {

FRROSPF::FRROSPF(std::shared_ptr<FRRControlPlane> control_plane)
    : control_plane_(control_plane), running_(false) {
}

bool FRROSPF::initialize(const std::map<std::string, std::string>& config) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    config_ = config;
    
    // Set up OSPF-specific configuration
    if (config.find("router_id") == config.end()) {
        config_["router_id"] = "1.1.1.1";
    }
    if (config.find("area") == config.end()) {
        config_["area"] = "0.0.0.0";
    }
    if (config.find("hello_interval") == config.end()) {
        config_["hello_interval"] = "10";
    }
    if (config.find("dead_interval") == config.end()) {
        config_["dead_interval"] = "40";
    }
    if (config.find("cost") == config.end()) {
        config_["cost"] = "1";
    }
    
    return true;
}

bool FRROSPF::start() {
    if (running_) {
        return true;
    }
    
    if (!control_plane_) {
        std::cerr << "FRR control plane not available" << std::endl;
        return false;
    }
    
    // Enable OSPF protocol in FRR
    if (!control_plane_->enable_protocol(FRRProtocol::OSPF)) {
        std::cerr << "Failed to enable OSPF protocol" << std::endl;
        return false;
    }
    
    // Apply OSPF configuration
    std::map<std::string, std::string> ospf_config;
    {
        std::lock_guard<std::mutex> lock(config_mutex_);
        ospf_config = config_;
    }
    
    // Send OSPF configuration to FRR
    FRRMessage message;
    message.type = FRRMessageType::CONFIG_UPDATE;
    message.protocol = FRRProtocol::OSPF;
    message.data = "configure_ospf";
    
    for (const auto& pair : ospf_config) {
        message.attributes[pair.first] = pair.second;
    }
    
    if (!control_plane_->send_message(message)) {
        std::cerr << "Failed to send OSPF configuration" << std::endl;
        return false;
    }
    
    running_ = true;
    return true;
}

bool FRROSPF::stop() {
    if (!running_) {
        return true;
    }
    
    if (control_plane_) {
        control_plane_->disable_protocol(FRRProtocol::OSPF);
    }
    
    running_ = false;
    return true;
}

bool FRROSPF::is_running() const {
    return running_;
}

bool FRROSPF::add_neighbor(const std::string& address, const std::map<std::string, std::string>& config) {
    if (!running_ || !control_plane_) {
        return false;
    }
    
    std::map<std::string, std::string> neighbor_config = config;
    neighbor_config["protocol"] = "ospf";
    
    return control_plane_->add_neighbor(address, FRRProtocol::OSPF, neighbor_config);
}

bool FRROSPF::remove_neighbor(const std::string& address) {
    if (!running_ || !control_plane_) {
        return false;
    }
    
    return control_plane_->remove_neighbor(address, FRRProtocol::OSPF);
}

std::vector<NeighborInfo> FRROSPF::get_neighbors() const {
    if (!control_plane_) {
        return {};
    }
    
    return control_plane_->get_neighbors(FRRProtocol::OSPF);
}

bool FRROSPF::is_neighbor_established(const std::string& address) const {
    auto neighbors = get_neighbors();
    for (const auto& neighbor : neighbors) {
        if (neighbor.address == address) {
            return neighbor.is_established();
        }
    }
    return false;
}

bool FRROSPF::advertise_route(const RouteInfo& route) {
    if (!running_ || !control_plane_) {
        return false;
    }
    
    // Add OSPF-specific attributes
    RouteInfo ospf_route = route;
    ospf_route.protocol = "ospf";
    
    return control_plane_->add_route(ospf_route);
}

bool FRROSPF::withdraw_route(const std::string& destination, uint8_t prefix_length) {
    if (!running_ || !control_plane_) {
        return false;
    }
    
    return control_plane_->remove_route(destination, prefix_length);
}

std::vector<RouteInfo> FRROSPF::get_routes() const {
    if (!control_plane_) {
        return {};
    }
    
    return control_plane_->get_routes(FRRProtocol::OSPF);
}

bool FRROSPF::update_config(const std::map<std::string, std::string>& config) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    
    for (const auto& pair : config) {
        config_[pair.first] = pair.second;
    }
    
    // If running, apply the new configuration
    if (running_ && control_plane_) {
        FRRMessage message;
        message.type = FRRMessageType::CONFIG_UPDATE;
        message.protocol = FRRProtocol::OSPF;
        message.data = "update_config";
        
        for (const auto& pair : config_) {
            message.attributes[pair.first] = pair.second;
        }
        
        return control_plane_->send_message(message);
    }
    
    return true;
}

std::map<std::string, std::string> FRROSPF::get_config() const {
    std::lock_guard<std::mutex> lock(config_mutex_);
    return config_;
}

ProtocolStatistics FRROSPF::get_statistics() const {
    ProtocolStatistics stats;
    
    if (control_plane_) {
        auto frr_stats = control_plane_->get_protocol_statistics(FRRProtocol::OSPF);
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

void FRROSPF::set_route_update_callback(RouteUpdateCallback callback) {
    if (control_plane_) {
        control_plane_->set_route_update_callback(callback);
    }
}

void FRROSPF::set_neighbor_update_callback(NeighborUpdateCallback callback) {
    if (control_plane_) {
        control_plane_->set_neighbor_update_callback(callback);
    }
}

} // namespace router_sim
