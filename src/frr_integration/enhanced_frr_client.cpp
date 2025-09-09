#include "frr_integration.h"
#include <iostream>
#include <chrono>
#include <sstream>
#include <json/json.h>
#include <zmq.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

namespace router_sim {

// Enhanced ZMQ Client Implementation
ZMQClient::ZMQClient() : context_(nullptr), socket_(nullptr), connected_(false), running_(false) {
}

ZMQClient::~ZMQClient() {
    stop();
    if (context_) {
        zmq_ctx_destroy(context_);
    }
}

bool ZMQClient::initialize(const FRRConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    config_ = config;
    
    context_ = zmq_ctx_new();
    if (!context_) {
        std::cerr << "Failed to create ZMQ context" << std::endl;
        return false;
    }
    
    socket_ = zmq_socket(context_, ZMQ_DEALER);
    if (!socket_) {
        std::cerr << "Failed to create ZMQ socket" << std::endl;
        return false;
    }
    
    // Set socket options
    int linger = 0;
    zmq_setsockopt(socket_, ZMQ_LINGER, &linger, sizeof(linger));
    
    return true;
}

bool ZMQClient::start() {
    if (running_.load()) {
        return true;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Connect to FRR daemons
    if (!connect_to_zebra()) {
        return false;
    }
    
    if (config_.enable_bgp && !connect_to_bgpd()) {
        return false;
    }
    
    if (config_.enable_ospf && !connect_to_ospfd()) {
        return false;
    }
    
    if (config_.enable_isis && !connect_to_isisd()) {
        return false;
    }
    
    running_.store(true);
    message_thread_ = std::thread(&ZMQClient::message_processing_loop, this);
    
    std::cout << "ZMQ FRR client started" << std::endl;
    return true;
}

bool ZMQClient::stop() {
    if (!running_.load()) {
        return true;
    }
    
    running_.store(false);
    connected_.store(false);
    
    if (message_thread_.joinable()) {
        message_thread_.join();
    }
    
    if (socket_) {
        zmq_close(socket_);
        socket_ = nullptr;
    }
    
    std::cout << "ZMQ FRR client stopped" << std::endl;
    return true;
}

bool ZMQClient::is_running() const {
    return running_.load();
}

bool ZMQClient::connect_to_zebra() {
    if (!socket_) return false;
    
    int rc = zmq_connect(socket_, config_.zebra_socket_path.c_str());
    if (rc != 0) {
        std::cerr << "Failed to connect to Zebra: " << zmq_strerror(errno) << std::endl;
        return false;
    }
    
    connected_.store(true);
    std::cout << "Connected to Zebra via ZMQ" << std::endl;
    return true;
}

bool ZMQClient::connect_to_bgpd() {
    // For now, simulate BGP connection
    std::cout << "Connected to BGPd via ZMQ" << std::endl;
    return true;
}

bool ZMQClient::connect_to_ospfd() {
    // For now, simulate OSPF connection
    std::cout << "Connected to OSPFd via ZMQ" << std::endl;
    return true;
}

bool ZMQClient::connect_to_isisd() {
    // For now, simulate ISIS connection
    std::cout << "Connected to ISISd via ZMQ" << std::endl;
    return true;
}

bool ZMQClient::disconnect_all() {
    connected_.store(false);
    std::cout << "Disconnected from all FRR daemons" << std::endl;
    return true;
}

bool ZMQClient::add_route(const FRRRoute& route) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::string key = route.destination + "/" + std::to_string(route.prefix_length);
    routes_[key] = route;
    routes_[key].timestamp = std::chrono::system_clock::now();
    routes_[key].is_active = true;
    
    stats_.total_routes++;
    if (route.protocol == "bgp") stats_.bgp_routes++;
    else if (route.protocol == "ospf") stats_.ospf_routes++;
    else if (route.protocol == "isis") stats_.isis_routes++;
    else if (route.protocol == "static") stats_.static_routes++;
    else if (route.protocol == "connected") stats_.connected_routes++;
    
    stats_.last_update = std::chrono::system_clock::now();
    
    // Send route to FRR
    std::string message = serialize_route(route);
    send_message(message);
    
    if (route_update_callback_) {
        route_update_callback_(route);
    }
    
    std::cout << "Added route " << route.destination << "/" 
              << static_cast<int>(route.prefix_length) << " via " << route.next_hop << std::endl;
    return true;
}

bool ZMQClient::remove_route(const std::string& destination, uint8_t prefix_length) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::string key = destination + "/" + std::to_string(prefix_length);
    auto it = routes_.find(key);
    if (it == routes_.end()) {
        return false;
    }
    
    std::string protocol = it->second.protocol;
    routes_.erase(it);
    
    stats_.total_routes--;
    if (protocol == "bgp") stats_.bgp_routes--;
    else if (protocol == "ospf") stats_.ospf_routes--;
    else if (protocol == "isis") stats_.isis_routes--;
    else if (protocol == "static") stats_.static_routes--;
    else if (protocol == "connected") stats_.connected_routes--;
    
    stats_.last_update = std::chrono::system_clock::now();
    
    std::cout << "Removed route " << destination << "/" 
              << static_cast<int>(prefix_length) << std::endl;
    return true;
}

bool ZMQClient::update_route(const FRRRoute& route) {
    return add_route(route); // Same as add for now
}

std::vector<FRRRoute> ZMQClient::get_routes() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<FRRRoute> result;
    for (const auto& pair : routes_) {
        result.push_back(pair.second);
    }
    return result;
}

std::vector<FRRRoute> ZMQClient::get_routes_by_protocol(const std::string& protocol) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<FRRRoute> result;
    for (const auto& pair : routes_) {
        if (pair.second.protocol == protocol) {
            result.push_back(pair.second);
        }
    }
    return result;
}

std::vector<FRRNeighbor> ZMQClient::get_neighbors() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<FRRNeighbor> result;
    for (const auto& pair : neighbors_) {
        result.push_back(pair.second);
    }
    return result;
}

std::vector<FRRNeighbor> ZMQClient::get_neighbors_by_protocol(const std::string& protocol) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<FRRNeighbor> result;
    for (const auto& pair : neighbors_) {
        if (pair.second.protocol == protocol) {
            result.push_back(pair.second);
        }
    }
    return result;
}

bool ZMQClient::is_neighbor_established(const std::string& address) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = neighbors_.find(address);
    return it != neighbors_.end() && it->second.is_established;
}

bool ZMQClient::enable_bgp(const std::map<std::string, std::string>& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    config_.bgp_config = config;
    config_.enable_bgp = true;
    
    // Add some sample BGP neighbors
    FRRNeighbor neighbor1;
    neighbor1.address = "192.168.1.1";
    neighbor1.protocol = "bgp";
    neighbor1.state = "Established";
    neighbor1.asn = 65001;
    neighbor1.is_established = true;
    neighbor1.last_seen = std::chrono::system_clock::now();
    neighbors_[neighbor1.address] = neighbor1;
    
    FRRNeighbor neighbor2;
    neighbor2.address = "192.168.2.1";
    neighbor2.protocol = "bgp";
    neighbor2.state = "Established";
    neighbor2.asn = 65002;
    neighbor2.is_established = true;
    neighbor2.last_seen = std::chrono::system_clock::now();
    neighbors_[neighbor2.address] = neighbor2;
    
    stats_.total_neighbors = neighbors_.size();
    stats_.established_neighbors = 2;
    
    std::cout << "Enabled BGP with " << config.size() << " configuration parameters" << std::endl;
    return true;
}

bool ZMQClient::disable_bgp() {
    std::lock_guard<std::mutex> lock(mutex_);
    config_.enable_bgp = false;
    
    // Remove BGP neighbors
    auto it = neighbors_.begin();
    while (it != neighbors_.end()) {
        if (it->second.protocol == "bgp") {
            it = neighbors_.erase(it);
        } else {
            ++it;
        }
    }
    
    stats_.total_neighbors = neighbors_.size();
    stats_.established_neighbors = 0;
    
    std::cout << "Disabled BGP" << std::endl;
    return true;
}

bool ZMQClient::enable_ospf(const std::map<std::string, std::string>& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    config_.ospf_config = config;
    config_.enable_ospf = true;
    
    std::cout << "Enabled OSPF with " << config.size() << " configuration parameters" << std::endl;
    return true;
}

bool ZMQClient::disable_ospf() {
    std::lock_guard<std::mutex> lock(mutex_);
    config_.enable_ospf = false;
    
    std::cout << "Disabled OSPF" << std::endl;
    return true;
}

bool ZMQClient::enable_isis(const std::map<std::string, std::string>& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    config_.isis_config = config;
    config_.enable_isis = true;
    
    std::cout << "Enabled ISIS with " << config.size() << " configuration parameters" << std::endl;
    return true;
}

bool ZMQClient::disable_isis() {
    std::lock_guard<std::mutex> lock(mutex_);
    config_.enable_isis = false;
    
    std::cout << "Disabled ISIS" << std::endl;
    return true;
}

FRRStatistics ZMQClient::get_statistics() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return stats_;
}

void ZMQClient::set_route_update_callback(RouteUpdateCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    route_update_callback_ = callback;
}

void ZMQClient::set_neighbor_update_callback(NeighborUpdateCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    neighbor_update_callback_ = callback;
}

void ZMQClient::set_connection_callback(ConnectionCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    connection_callback_ = callback;
}

void ZMQClient::message_processing_loop() {
    std::cout << "ZMQ message processing loop started" << std::endl;
    
    while (running_.load()) {
        std::string message;
        if (receive_message(message, 100)) {
            process_message(message);
        }
    }
    
    std::cout << "ZMQ message processing loop stopped" << std::endl;
}

bool ZMQClient::send_message(const std::string& message) {
    if (!socket_ || !connected_.load()) {
        return false;
    }
    
    int rc = zmq_send(socket_, message.c_str(), message.length(), 0);
    return rc == static_cast<int>(message.length());
}

bool ZMQClient::receive_message(std::string& message, uint32_t timeout_ms) {
    if (!socket_ || !connected_.load()) {
        return false;
    }
    
    zmq_pollitem_t items[1];
    items[0].socket = socket_;
    items[0].events = ZMQ_POLLIN;
    
    int rc = zmq_poll(items, 1, timeout_ms);
    if (rc > 0 && items[0].revents & ZMQ_POLLIN) {
        char buffer[1024];
        int size = zmq_recv(socket_, buffer, sizeof(buffer) - 1, 0);
        if (size > 0) {
            buffer[size] = '\0';
            message = std::string(buffer, size);
            return true;
        }
    }
    
    return false;
}

void ZMQClient::process_message(const std::string& message) {
    // Parse JSON message and update internal state
    Json::Value root;
    Json::Reader reader;
    
    if (reader.parse(message, root)) {
        if (root.isMember("type")) {
            std::string type = root["type"].asString();
            
            if (type == "route_update") {
                FRRRoute route;
                if (parse_route_message(message, route)) {
                    std::lock_guard<std::mutex> lock(mutex_);
                    std::string key = route.destination + "/" + std::to_string(route.prefix_length);
                    routes_[key] = route;
                    
                    if (route_update_callback_) {
                        route_update_callback_(route);
                    }
                }
            } else if (type == "neighbor_update") {
                FRRNeighbor neighbor;
                if (parse_neighbor_message(message, neighbor)) {
                    std::lock_guard<std::mutex> lock(mutex_);
                    neighbors_[neighbor.address] = neighbor;
                    
                    if (neighbor_update_callback_) {
                        neighbor_update_callback_(neighbor);
                    }
                }
            }
        }
    }
}

bool ZMQClient::parse_route_message(const std::string& message, FRRRoute& route) {
    Json::Value root;
    Json::Reader reader;
    
    if (!reader.parse(message, root)) {
        return false;
    }
    
    if (root.isMember("route")) {
        Json::Value routeData = root["route"];
        route.destination = routeData.get("destination", "").asString();
        route.prefix_length = routeData.get("prefix_length", 0).asUInt();
        route.next_hop = routeData.get("next_hop", "").asString();
        route.interface = routeData.get("interface", "").asString();
        route.protocol = routeData.get("protocol", "").asString();
        route.metric = routeData.get("metric", 0).asUInt();
        route.preference = routeData.get("preference", 0).asUInt();
        route.is_active = routeData.get("is_active", false).asBool();
        return true;
    }
    
    return false;
}

bool ZMQClient::parse_neighbor_message(const std::string& message, FRRNeighbor& neighbor) {
    Json::Value root;
    Json::Reader reader;
    
    if (!reader.parse(message, root)) {
        return false;
    }
    
    if (root.isMember("neighbor")) {
        Json::Value neighborData = root["neighbor"];
        neighbor.address = neighborData.get("address", "").asString();
        neighbor.protocol = neighborData.get("protocol", "").asString();
        neighbor.state = neighborData.get("state", "").asString();
        neighbor.asn = neighborData.get("asn", 0).asUInt();
        neighbor.description = neighborData.get("description", "").asString();
        neighbor.is_established = neighborData.get("is_established", false).asBool();
        return true;
    }
    
    return false;
}

std::string ZMQClient::serialize_route(const FRRRoute& route) const {
    Json::Value root;
    root["type"] = "route_update";
    
    Json::Value routeData;
    routeData["destination"] = route.destination;
    routeData["prefix_length"] = route.prefix_length;
    routeData["next_hop"] = route.next_hop;
    routeData["interface"] = route.interface;
    routeData["protocol"] = route.protocol;
    routeData["metric"] = route.metric;
    routeData["preference"] = route.preference;
    routeData["is_active"] = route.is_active;
    
    root["route"] = routeData;
    
    Json::StreamWriterBuilder builder;
    return Json::writeString(builder, root);
}

std::string ZMQClient::serialize_neighbor(const FRRNeighbor& neighbor) const {
    Json::Value root;
    root["type"] = "neighbor_update";
    
    Json::Value neighborData;
    neighborData["address"] = neighbor.address;
    neighborData["protocol"] = neighbor.protocol;
    neighborData["state"] = neighbor.state;
    neighborData["asn"] = neighbor.asn;
    neighborData["description"] = neighbor.description;
    neighborData["is_established"] = neighbor.is_established;
    
    root["neighbor"] = neighborData;
    
    Json::StreamWriterBuilder builder;
    return Json::writeString(builder, root);
}

} // namespace router_sim
