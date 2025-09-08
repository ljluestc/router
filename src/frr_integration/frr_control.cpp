#include "frr_integration.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <json/json.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

namespace router_sim {

FRRControlPlane::FRRControlPlane() 
    : running_(false), connected_(false) {
    zmq_context_ = std::make_unique<zmq::context_t>(1);
    zmq_socket_ = std::make_unique<zmq::socket_t>(*zmq_context_, ZMQ_REQ);
    
    // Initialize protocol states
    enabled_protocols_[FRRProtocol::BGP] = false;
    enabled_protocols_[FRRProtocol::OSPF] = false;
    enabled_protocols_[FRRProtocol::ISIS] = false;
}

FRRControlPlane::~FRRControlPlane() {
    stop();
}

bool FRRControlPlane::initialize(const FRRConfig& config) {
    config_ = config;
    
    try {
        // Connect to FRR daemon
        std::string endpoint = "tcp://" + config_.host + ":" + std::to_string(config_.port);
        zmq_socket_->connect(endpoint.c_str());
        
        // Set socket timeout
        zmq_socket_->setsockopt(ZMQ_RCVTIMEO, 1000);
        zmq_socket_->setsockopt(ZMQ_SNDTIMEO, 1000);
        
        connected_ = true;
        
        // Load configuration if specified
        if (!config_.config_file.empty()) {
            load_config(config_.config_file);
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize FRR control plane: " << e.what() << std::endl;
        return false;
    }
}

bool FRRControlPlane::start() {
    if (running_) {
        return true;
    }
    
    try {
        running_ = true;
        message_thread_ = std::thread(&FRRControlPlane::message_loop, this);
        
        // Enable configured protocols
        if (config_.enable_bgp) {
            enable_protocol(FRRProtocol::BGP);
        }
        if (config_.enable_ospf) {
            enable_protocol(FRRProtocol::OSPF);
        }
        if (config_.enable_isis) {
            enable_protocol(FRRProtocol::ISIS);
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to start FRR control plane: " << e.what() << std::endl;
        running_ = false;
        return false;
    }
}

bool FRRControlPlane::stop() {
    if (!running_) {
        return true;
    }
    
    running_ = false;
    
    if (message_thread_.joinable()) {
        message_thread_.join();
    }
    
    disconnect_from_frr();
    return true;
}

bool FRRControlPlane::is_running() const {
    return running_;
}

bool FRRControlPlane::enable_protocol(FRRProtocol protocol) {
    if (!connected_) {
        return false;
    }
    
    FRRMessage message;
    message.type = FRRMessageType::CONFIG_UPDATE;
    message.protocol = protocol;
    message.data = "enable";
    
    if (send_message(message)) {
        enabled_protocols_[protocol] = true;
        return true;
    }
    
    return false;
}

bool FRRControlPlane::disable_protocol(FRRProtocol protocol) {
    if (!connected_) {
        return false;
    }
    
    FRRMessage message;
    message.type = FRRMessageType::CONFIG_UPDATE;
    message.protocol = protocol;
    message.data = "disable";
    
    if (send_message(message)) {
        enabled_protocols_[protocol] = false;
        return true;
    }
    
    return false;
}

bool FRRControlPlane::is_protocol_enabled(FRRProtocol protocol) const {
    auto it = enabled_protocols_.find(protocol);
    return it != enabled_protocols_.end() && it->second;
}

bool FRRControlPlane::load_config(const std::string& config_file) {
    std::ifstream file(config_file);
    if (!file.is_open()) {
        std::cerr << "Failed to open config file: " << config_file << std::endl;
        return false;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string config_content = buffer.str();
    
    FRRMessage message;
    message.type = FRRMessageType::CONFIG_UPDATE;
    message.data = config_content;
    
    return send_message(message);
}

bool FRRControlPlane::save_config(const std::string& config_file) {
    FRRMessage message;
    message.type = FRRMessageType::CONFIG_UPDATE;
    message.data = "save";
    message.attributes["file"] = config_file;
    
    return send_message(message);
}

bool FRRControlPlane::apply_config() {
    FRRMessage message;
    message.type = FRRMessageType::CONFIG_UPDATE;
    message.data = "apply";
    
    return send_message(message);
}

bool FRRControlPlane::reload_config() {
    FRRMessage message;
    message.type = FRRMessageType::CONFIG_UPDATE;
    message.data = "reload";
    
    return send_message(message);
}

bool FRRControlPlane::add_route(const RouteInfo& route) {
    if (!connected_) {
        return false;
    }
    
    FRRMessage message;
    message.type = FRRMessageType::ROUTE_ADD;
    message.protocol = FRRProtocol::BGP; // Default to BGP
    message.data = route.destination + "/" + std::to_string(route.prefix_length);
    message.attributes["next_hop"] = route.next_hop;
    message.attributes["metric"] = std::to_string(route.metric);
    message.attributes["admin_distance"] = std::to_string(route.admin_distance);
    
    return send_message(message);
}

bool FRRControlPlane::remove_route(const std::string& destination, uint8_t prefix_length) {
    if (!connected_) {
        return false;
    }
    
    FRRMessage message;
    message.type = FRRMessageType::ROUTE_DELETE;
    message.protocol = FRRProtocol::BGP; // Default to BGP
    message.data = destination + "/" + std::to_string(prefix_length);
    
    return send_message(message);
}

bool FRRControlPlane::update_route(const RouteInfo& route) {
    // Remove old route and add new one
    if (remove_route(route.destination, route.prefix_length)) {
        return add_route(route);
    }
    return false;
}

std::vector<RouteInfo> FRRControlPlane::get_routes(FRRProtocol protocol) const {
    std::vector<RouteInfo> routes;
    
    if (!connected_) {
        return routes;
    }
    
    FRRMessage message;
    message.type = FRRMessageType::STATISTICS;
    message.protocol = protocol;
    message.data = "routes";
    
    if (const_cast<FRRControlPlane*>(this)->send_message(message)) {
        FRRMessage response;
        if (const_cast<FRRControlPlane*>(this)->receive_message(response, 5000)) {
            // Parse response and populate routes
            // This would parse the JSON response from FRR
        }
    }
    
    return routes;
}

bool FRRControlPlane::add_neighbor(const std::string& address, FRRProtocol protocol, 
                                  const std::map<std::string, std::string>& config) {
    if (!connected_) {
        return false;
    }
    
    FRRMessage message;
    message.type = FRRMessageType::CONFIG_UPDATE;
    message.protocol = protocol;
    message.data = "add_neighbor";
    message.attributes["address"] = address;
    
    for (const auto& pair : config) {
        message.attributes[pair.first] = pair.second;
    }
    
    return send_message(message);
}

bool FRRControlPlane::remove_neighbor(const std::string& address, FRRProtocol protocol) {
    if (!connected_) {
        return false;
    }
    
    FRRMessage message;
    message.type = FRRMessageType::CONFIG_UPDATE;
    message.protocol = protocol;
    message.data = "remove_neighbor";
    message.attributes["address"] = address;
    
    return send_message(message);
}

std::vector<NeighborInfo> FRRControlPlane::get_neighbors(FRRProtocol protocol) const {
    std::vector<NeighborInfo> neighbors;
    
    if (!connected_) {
        return neighbors;
    }
    
    FRRMessage message;
    message.type = FRRMessageType::STATISTICS;
    message.protocol = protocol;
    message.data = "neighbors";
    
    if (const_cast<FRRControlPlane*>(this)->send_message(message)) {
        FRRMessage response;
        if (const_cast<FRRControlPlane*>(this)->receive_message(response, 5000)) {
            // Parse response and populate neighbors
            // This would parse the JSON response from FRR
        }
    }
    
    return neighbors;
}

FRRStatistics FRRControlPlane::get_statistics() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    return statistics_;
}

FRRStatistics FRRControlPlane::get_protocol_statistics(FRRProtocol protocol) const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    auto it = protocol_statistics_.find(protocol);
    if (it != protocol_statistics_.end()) {
        return it->second;
    }
    return FRRStatistics{};
}

void FRRControlPlane::set_route_update_callback(std::function<void(const RouteInfo&, bool)> callback) {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    route_callback_ = callback;
}

void FRRControlPlane::set_neighbor_update_callback(std::function<void(const NeighborInfo&, bool)> callback) {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    neighbor_callback_ = callback;
}

void FRRControlPlane::set_error_callback(std::function<void(const std::string&)> callback) {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    error_callback_ = callback;
}

bool FRRControlPlane::send_message(const FRRMessage& message) {
    if (!connected_) {
        return false;
    }
    
    try {
        std::string serialized = serialize_message(message);
        zmq::message_t zmq_msg(serialized.begin(), serialized.end());
        zmq_socket_->send(zmq_msg, zmq::send_flags::none);
        
        {
            std::lock_guard<std::mutex> lock(stats_mutex_);
            statistics_.messages_sent++;
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to send message: " << e.what() << std::endl;
        return false;
    }
}

bool FRRControlPlane::receive_message(FRRMessage& message, int timeout_ms) {
    if (!connected_) {
        return false;
    }
    
    try {
        zmq::message_t zmq_msg;
        auto result = zmq_socket_->recv(zmq_msg, zmq::recv_flags::none);
        
        if (result) {
            std::string data(static_cast<char*>(zmq_msg.data()), zmq_msg.size());
            bool success = deserialize_message(data, message);
            
            if (success) {
                std::lock_guard<std::mutex> lock(stats_mutex_);
                statistics_.messages_received++;
            }
            
            return success;
        }
        
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Failed to receive message: " << e.what() << std::endl;
        return false;
    }
}

void FRRControlPlane::message_loop() {
    while (running_) {
        FRRMessage message;
        if (receive_message(message, 1000)) {
            process_message(message);
        }
    }
}

void FRRControlPlane::process_message(const FRRMessage& message) {
    switch (message.type) {
        case FRRMessageType::ROUTE_ADD:
        case FRRMessageType::ROUTE_DELETE: {
            // Parse route information and call callback
            std::lock_guard<std::mutex> lock(callback_mutex_);
            if (route_callback_) {
                RouteInfo route;
                // Parse route from message data
                route_callback_(route, message.type == FRRMessageType::ROUTE_ADD);
            }
            break;
        }
        case FRRMessageType::NEIGHBOR_UP:
        case FRRMessageType::NEIGHBOR_DOWN: {
            // Parse neighbor information and call callback
            std::lock_guard<std::mutex> lock(callback_mutex_);
            if (neighbor_callback_) {
                NeighborInfo neighbor;
                // Parse neighbor from message data
                neighbor_callback_(neighbor, message.type == FRRMessageType::NEIGHBOR_UP);
            }
            break;
        }
        default:
            break;
    }
}

bool FRRControlPlane::connect_to_frr() {
    // Implementation for connecting to FRR daemon
    return true;
}

void FRRControlPlane::disconnect_from_frr() {
    if (connected_) {
        zmq_socket_->close();
        connected_ = false;
    }
}

std::string FRRControlPlane::serialize_message(const FRRMessage& message) const {
    Json::Value root;
    root["type"] = static_cast<int>(message.type);
    root["protocol"] = static_cast<int>(message.protocol);
    root["data"] = message.data;
    root["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
        message.timestamp.time_since_epoch()).count();
    
    Json::Value attributes(Json::objectValue);
    for (const auto& pair : message.attributes) {
        attributes[pair.first] = pair.second;
    }
    root["attributes"] = attributes;
    
    Json::StreamWriterBuilder builder;
    return Json::writeString(builder, root);
}

bool FRRControlPlane::deserialize_message(const std::string& data, FRRMessage& message) const {
    Json::Value root;
    Json::CharReaderBuilder builder;
    std::string errors;
    
    std::istringstream stream(data);
    if (!Json::parseFromStream(builder, stream, &root, &errors)) {
        std::cerr << "Failed to parse JSON: " << errors << std::endl;
        return false;
    }
    
    message.type = static_cast<FRRMessageType>(root.get("type", 0).asInt());
    message.protocol = static_cast<FRRProtocol>(root.get("protocol", 0).asInt());
    message.data = root.get("data", "").asString();
    
    auto timestamp_ms = root.get("timestamp", 0).asInt64();
    message.timestamp = std::chrono::steady_clock::time_point(
        std::chrono::milliseconds(timestamp_ms));
    
    if (root.isMember("attributes")) {
        const Json::Value& attrs = root["attributes"];
        for (const auto& key : attrs.getMemberNames()) {
            message.attributes[key] = attrs[key].asString();
        }
    }
    
    return true;
}

std::string FRRControlPlane::get_status() const {
    std::ostringstream oss;
    oss << "FRR Control Plane Status:\n";
    oss << "  Running: " << (running_ ? "Yes" : "No") << "\n";
    oss << "  Connected: " << (connected_ ? "Yes" : "No") << "\n";
    oss << "  BGP Enabled: " << (enabled_protocols_.at(FRRProtocol::BGP) ? "Yes" : "No") << "\n";
    oss << "  OSPF Enabled: " << (enabled_protocols_.at(FRRProtocol::OSPF) ? "Yes" : "No") << "\n";
    oss << "  ISIS Enabled: " << (enabled_protocols_.at(FRRProtocol::ISIS) ? "Yes" : "No") << "\n";
    
    auto stats = get_statistics();
    oss << "  Messages Sent: " << stats.messages_sent << "\n";
    oss << "  Messages Received: " << stats.messages_received << "\n";
    oss << "  Routes Installed: " << stats.routes_installed << "\n";
    oss << "  Routes Removed: " << stats.routes_removed << "\n";
    
    return oss.str();
}

std::vector<std::string> FRRControlPlane::get_logs(int lines) const {
    std::vector<std::string> logs;
    // Implementation to get FRR logs
    return logs;
}

bool FRRControlPlane::enable_debug(bool enable) {
    FRRMessage message;
    message.type = FRRMessageType::CONFIG_UPDATE;
    message.data = enable ? "debug_on" : "debug_off";
    
    return send_message(message);
}

} // namespace router_sim
