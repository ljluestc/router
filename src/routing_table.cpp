#include "routing_table.h"
#include <iostream>
#include <algorithm>
#include <fstream>
#include <sstream>

using namespace RouterSim;

RoutingTable::RoutingTable() {
}

RoutingTable::~RoutingTable() {
}

bool RoutingTable::add_route(const RouteEntry& route) {
    if (!is_valid_route(route)) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(routes_mutex_);
    
    // Add to network-based index
    routes_by_network_[route.network].push_back(route);
    
    // Add to protocol-based index
    routes_by_protocol_[route.protocol].push_back(route);
    
    // Add to interface-based index
    routes_by_interface_[route.interface].push_back(route);
    
    // Notify callback
    notify_route_change(route, true);
    
    return true;
}

bool RoutingTable::remove_route(const std::string& network) {
    std::lock_guard<std::mutex> lock(routes_mutex_);
    
    auto it = routes_by_network_.find(network);
    if (it == routes_by_network_.end()) {
        return false;
    }
    
    // Remove from all indices
    for (const auto& route : it->second) {
        // Remove from protocol index
        auto proto_it = routes_by_protocol_.find(route.protocol);
        if (proto_it != routes_by_protocol_.end()) {
            proto_it->second.erase(
                std::remove_if(proto_it->second.begin(), proto_it->second.end(),
                              [&route](const RouteEntry& r) { return r.network == route.network; }),
                proto_it->second.end()
            );
        }
        
        // Remove from interface index
        auto iface_it = routes_by_interface_.find(route.interface);
        if (iface_it != routes_by_interface_.end()) {
            iface_it->second.erase(
                std::remove_if(iface_it->second.begin(), iface_it->second.end(),
                              [&route](const RouteEntry& r) { return r.network == route.network; }),
                iface_it->second.end()
            );
        }
        
        // Notify callback
        notify_route_change(route, false);
    }
    
    routes_by_network_.erase(it);
    return true;
}

bool RoutingTable::update_route(const RouteEntry& route) {
    if (!is_valid_route(route)) {
        return false;
    }
    
    // Remove old route and add new one
    remove_route(route.network);
    return add_route(route);
}

bool RoutingTable::has_route(const std::string& network) const {
    std::lock_guard<std::mutex> lock(routes_mutex_);
    return routes_by_network_.find(network) != routes_by_network_.end();
}

std::vector<RouteEntry> RoutingTable::lookup_routes(const std::string& destination) const {
    std::lock_guard<std::mutex> lock(routes_mutex_);
    
    std::vector<RouteEntry> result;
    
    // Find all matching routes
    for (const auto& pair : routes_by_network_) {
        if (route_matches(pair.first, destination)) {
            for (const auto& route : pair.second) {
                if (route.is_active) {
                    result.push_back(route);
                }
            }
        }
    }
    
    return result;
}

RouteEntry RoutingTable::get_best_route(const std::string& destination) const {
    auto routes = lookup_routes(destination);
    if (routes.empty()) {
        return RouteEntry{}; // Empty route if not found
    }
    
    return select_best_route(routes);
}

std::string RoutingTable::get_next_hop(const std::string& destination) const {
    auto route = get_best_route(destination);
    return route.next_hop;
}

std::vector<RouteEntry> RoutingTable::get_all_routes() const {
    std::lock_guard<std::mutex> lock(routes_mutex_);
    
    std::vector<RouteEntry> result;
    for (const auto& pair : routes_by_network_) {
        for (const auto& route : pair.second) {
            result.push_back(route);
        }
    }
    
    return result;
}

std::vector<RouteEntry> RoutingTable::get_routes_by_protocol(const std::string& protocol) const {
    std::lock_guard<std::mutex> lock(routes_mutex_);
    
    auto it = routes_by_protocol_.find(protocol);
    if (it == routes_by_protocol_.end()) {
        return {};
    }
    
    return it->second;
}

std::vector<RouteEntry> RoutingTable::get_routes_by_interface(const std::string& interface) const {
    std::lock_guard<std::mutex> lock(routes_mutex_);
    
    auto it = routes_by_interface_.find(interface);
    if (it == routes_by_interface_.end()) {
        return {};
    }
    
    return it->second;
}

std::vector<RouteEntry> RoutingTable::get_active_routes() const {
    std::lock_guard<std::mutex> lock(routes_mutex_);
    
    std::vector<RouteEntry> result;
    for (const auto& pair : routes_by_network_) {
        for (const auto& route : pair.second) {
            if (route.is_active) {
                result.push_back(route);
            }
        }
    }
    
    return result;
}

std::vector<RouteEntry> RoutingTable::filter_routes(const std::function<bool(const RouteEntry&)>& filter) const {
    std::lock_guard<std::mutex> lock(routes_mutex_);
    
    std::vector<RouteEntry> result;
    for (const auto& pair : routes_by_network_) {
        for (const auto& route : pair.second) {
            if (filter(route)) {
                result.push_back(route);
            }
        }
    }
    
    return result;
}

size_t RoutingTable::get_route_count() const {
    std::lock_guard<std::mutex> lock(routes_mutex_);
    
    size_t count = 0;
    for (const auto& pair : routes_by_network_) {
        count += pair.second.size();
    }
    
    return count;
}

size_t RoutingTable::get_route_count_by_protocol(const std::string& protocol) const {
    std::lock_guard<std::mutex> lock(routes_mutex_);
    
    auto it = routes_by_protocol_.find(protocol);
    if (it == routes_by_protocol_.end()) {
        return 0;
    }
    
    return it->second.size();
}

std::map<std::string, size_t> RoutingTable::get_protocol_counts() const {
    std::lock_guard<std::mutex> lock(routes_mutex_);
    
    std::map<std::string, size_t> counts;
    for (const auto& pair : routes_by_protocol_) {
        counts[pair.first] = pair.second.size();
    }
    
    return counts;
}

void RoutingTable::clear_routes() {
    std::lock_guard<std::mutex> lock(routes_mutex_);
    
    routes_by_network_.clear();
    routes_by_protocol_.clear();
    routes_by_interface_.clear();
}

void RoutingTable::clear_routes_by_protocol(const std::string& protocol) {
    std::lock_guard<std::mutex> lock(routes_mutex_);
    
    auto it = routes_by_protocol_.find(protocol);
    if (it != routes_by_protocol_.end()) {
        for (const auto& route : it->second) {
            // Remove from network index
            auto net_it = routes_by_network_.find(route.network);
            if (net_it != routes_by_network_.end()) {
                net_it->second.erase(
                    std::remove_if(net_it->second.begin(), net_it->second.end(),
                                  [&route](const RouteEntry& r) { return r.protocol == route.protocol; }),
                    net_it->second.end()
                );
            }
            
            // Remove from interface index
            auto iface_it = routes_by_interface_.find(route.interface);
            if (iface_it != routes_by_interface_.end()) {
                iface_it->second.erase(
                    std::remove_if(iface_it->second.begin(), iface_it->second.end(),
                                  [&route](const RouteEntry& r) { return r.protocol == route.protocol; }),
                    iface_it->second.end()
                );
            }
        }
        
        routes_by_protocol_.erase(it);
    }
}

void RoutingTable::clear_routes_by_interface(const std::string& interface) {
    std::lock_guard<std::mutex> lock(routes_mutex_);
    
    auto it = routes_by_interface_.find(interface);
    if (it != routes_by_interface_.end()) {
        for (const auto& route : it->second) {
            // Remove from network index
            auto net_it = routes_by_network_.find(route.network);
            if (net_it != routes_by_network_.end()) {
                net_it->second.erase(
                    std::remove_if(net_it->second.begin(), net_it->second.end(),
                                  [&route](const RouteEntry& r) { return r.interface == route.interface; }),
                    net_it->second.end()
                );
            }
            
            // Remove from protocol index
            auto proto_it = routes_by_protocol_.find(route.protocol);
            if (proto_it != routes_by_protocol_.end()) {
                proto_it->second.erase(
                    std::remove_if(proto_it->second.begin(), proto_it->second.end(),
                                  [&route](const RouteEntry& r) { return r.interface == route.interface; }),
                    proto_it->second.end()
                );
            }
        }
        
        routes_by_interface_.erase(it);
    }
}

void RoutingTable::register_route_change_callback(std::function<void(const RouteEntry&, bool)> callback) {
    route_change_callback_ = callback;
}

void RoutingTable::unregister_route_change_callback() {
    route_change_callback_ = nullptr;
}

bool RoutingTable::save_to_file(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    auto routes = get_all_routes();
    for (const auto& route : routes) {
        file << RouteUtils::format_route(route) << "\n";
    }
    
    return true;
}

bool RoutingTable::load_from_file(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        auto route = RouteUtils::parse_route_string(line);
        if (!route.network.empty()) {
            add_route(route);
        }
    }
    
    return true;
}

bool RoutingTable::is_valid_route(const RouteEntry& route) const {
    return !route.network.empty() && !route.next_hop.empty() && !route.protocol.empty();
}

bool RoutingTable::is_route_active(const RouteEntry& route) const {
    return route.is_active;
}

void RoutingTable::notify_route_change(const RouteEntry& route, bool added) {
    if (route_change_callback_) {
        route_change_callback_(route, added);
    }
}

std::vector<RouteEntry> RoutingTable::find_routes(const std::string& network) const {
    std::lock_guard<std::mutex> lock(routes_mutex_);
    
    auto it = routes_by_network_.find(network);
    if (it == routes_by_network_.end()) {
        return {};
    }
    
    return it->second;
}

RouteEntry RoutingTable::select_best_route(const std::vector<RouteEntry>& routes) const {
    if (routes.empty()) {
        return RouteEntry{};
    }
    
    // Sort routes by priority (lower admin distance and metric = higher priority)
    std::vector<RouteEntry> sorted_routes = routes;
    std::sort(sorted_routes.begin(), sorted_routes.end(), 
              [](const RouteEntry& a, const RouteEntry& b) {
                  return compare_routes(a, b) < 0;
              });
    
    return sorted_routes[0];
}

bool RoutingTable::route_matches(const std::string& network, const std::string& destination) const {
    // Simple exact match - in real implementation, this would handle CIDR notation
    return network == destination;
}

int RoutingTable::calculate_route_priority(const RouteEntry& route) const {
    // Lower admin distance and metric = higher priority
    return route.admin_distance * 1000 + route.metric;
}

// RouteUtils implementation
bool RouteUtils::is_valid_network(const std::string& network) {
    // Simple validation - in real implementation, this would validate CIDR notation
    return !network.empty() && network.find('/') != std::string::npos;
}

bool RouteUtils::is_valid_next_hop(const std::string& next_hop) {
    // Simple IP validation
    std::regex ip_regex(R"(^(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3})$)");
    return std::regex_match(next_hop, ip_regex);
}

bool RouteUtils::is_valid_metric(uint32_t metric) {
    return metric <= 65535; // Maximum metric value
}

bool RouteUtils::is_valid_admin_distance(uint32_t admin_distance) {
    return admin_distance <= 255; // Maximum admin distance
}

bool RouteUtils::is_better_route(const RouteEntry& a, const RouteEntry& b) {
    return compare_routes(a, b) < 0;
}

int RouteUtils::compare_routes(const RouteEntry& a, const RouteEntry& b) {
    // Compare by admin distance first, then by metric
    if (a.admin_distance != b.admin_distance) {
        return a.admin_distance - b.admin_distance;
    }
    
    return a.metric - b.metric;
}

bool RouteUtils::is_subnet_of(const std::string& network, const std::string& subnet) {
    // Simplified implementation - in real code, this would parse CIDR notation
    return network.find(subnet) == 0;
}

std::string RouteUtils::get_network_address(const std::string& ip, const std::string& mask) {
    // Simplified implementation - in real code, this would calculate network address
    return ip;
}

std::string RouteUtils::get_broadcast_address(const std::string& ip, const std::string& mask) {
    // Simplified implementation - in real code, this would calculate broadcast address
    return ip;
}

bool RouteUtils::is_ip_in_network(const std::string& ip, const std::string& network) {
    // Simplified implementation - in real code, this would check if IP is in network
    return true;
}

std::string RouteUtils::format_route(const RouteEntry& route) {
    std::stringstream ss;
    ss << route.network << " " << route.next_hop << " " << route.interface 
       << " " << route.metric << " " << route.protocol << " " << route.admin_distance;
    return ss.str();
}

std::string RouteUtils::format_route_table(const std::vector<RouteEntry>& routes) {
    std::stringstream ss;
    
    ss << std::setw(20) << std::left << "Network"
       << std::setw(15) << "Next Hop"
       << std::setw(10) << "Interface"
       << std::setw(8) << "Metric"
       << std::setw(10) << "Protocol"
       << std::setw(8) << "AD" << "\n";
    ss << std::string(71, '-') << "\n";
    
    for (const auto& route : routes) {
        ss << std::setw(20) << std::left << route.network
           << std::setw(15) << route.next_hop
           << std::setw(10) << route.interface
           << std::setw(8) << route.metric
           << std::setw(10) << route.protocol
           << std::setw(8) << route.admin_distance << "\n";
    }
    
    return ss.str();
}

std::string RouteUtils::format_route_summary(const std::vector<RouteEntry>& routes) {
    std::stringstream ss;
    
    std::map<std::string, size_t> protocol_counts;
    for (const auto& route : routes) {
        protocol_counts[route.protocol]++;
    }
    
    ss << "Total routes: " << routes.size() << "\n";
    for (const auto& pair : protocol_counts) {
        ss << "  " << pair.first << ": " << pair.second << "\n";
    }
    
    return ss.str();
}

RouteEntry RouteUtils::parse_route_string(const std::string& route_str) {
    RouteEntry route;
    
    std::istringstream ss(route_str);
    std::string token;
    std::vector<std::string> tokens;
    
    while (ss >> token) {
        tokens.push_back(token);
    }
    
    if (tokens.size() >= 6) {
        route.network = tokens[0];
        route.next_hop = tokens[1];
        route.interface = tokens[2];
        route.metric = std::stoul(tokens[3]);
        route.protocol = tokens[4];
        route.admin_distance = std::stoul(tokens[5]);
        route.is_active = true;
    }
    
    return route;
}

std::vector<RouteEntry> RouteUtils::parse_route_table_string(const std::string& table_str) {
    std::vector<RouteEntry> routes;
    std::istringstream ss(table_str);
    std::string line;
    
    while (std::getline(ss, line)) {
        auto route = parse_route_string(line);
        if (!route.network.empty()) {
            routes.push_back(route);
        }
    }
    
    return routes;
}

// RouteManager implementation
RouteManager::RouteManager() {
}

RouteManager::~RouteManager() {
}

bool RouteManager::create_table(const std::string& name) {
    std::lock_guard<std::mutex> lock(tables_mutex_);
    
    if (tables_.find(name) != tables_.end()) {
        return false; // Table already exists
    }
    
    tables_[name] = std::make_shared<RoutingTable>();
    return true;
}

bool RouteManager::delete_table(const std::string& name) {
    std::lock_guard<std::mutex> lock(tables_mutex_);
    
    auto it = tables_.find(name);
    if (it == tables_.end()) {
        return false; // Table doesn't exist
    }
    
    tables_.erase(it);
    return true;
}

bool RouteManager::has_table(const std::string& name) const {
    std::lock_guard<std::mutex> lock(tables_mutex_);
    return tables_.find(name) != tables_.end();
}

std::vector<std::string> RouteManager::get_table_names() const {
    std::lock_guard<std::mutex> lock(tables_mutex_);
    
    std::vector<std::string> names;
    for (const auto& pair : tables_) {
        names.push_back(pair.first);
    }
    
    return names;
}

bool RouteManager::add_route(const std::string& table_name, const RouteEntry& route) {
    auto table = get_table(table_name);
    if (!table) {
        return false;
    }
    
    return table->add_route(route);
}

bool RouteManager::remove_route(const std::string& table_name, const std::string& network) {
    auto table = get_table(table_name);
    if (!table) {
        return false;
    }
    
    return table->remove_route(network);
}

bool RouteManager::update_route(const std::string& table_name, const RouteEntry& route) {
    auto table = get_table(table_name);
    if (!table) {
        return false;
    }
    
    return table->update_route(route);
}

std::vector<RouteEntry> RouteManager::lookup_routes(const std::string& table_name, 
                                                  const std::string& destination) const {
    auto table = get_table(table_name);
    if (!table) {
        return {};
    }
    
    return table->lookup_routes(destination);
}

RouteEntry RouteManager::get_best_route(const std::string& table_name, 
                                      const std::string& destination) const {
    auto table = get_table(table_name);
    if (!table) {
        return RouteEntry{};
    }
    
    return table->get_best_route(destination);
}

std::shared_ptr<RoutingTable> RouteManager::get_table(const std::string& name) const {
    std::lock_guard<std::mutex> lock(tables_mutex_);
    
    auto it = tables_.find(name);
    if (it == tables_.end()) {
        return nullptr;
    }
    
    return it->second;
}

bool RouteManager::clear_table(const std::string& name) {
    auto table = get_table(name);
    if (!table) {
        return false;
    }
    
    table->clear_routes();
    return true;
}

bool RouteManager::copy_table(const std::string& src_name, const std::string& dst_name) {
    auto src_table = get_table(src_name);
    if (!src_table) {
        return false;
    }
    
    auto dst_table = get_table(dst_name);
    if (!dst_table) {
        // Create destination table
        if (!create_table(dst_name)) {
            return false;
        }
        dst_table = get_table(dst_name);
    }
    
    auto routes = src_table->get_all_routes();
    for (const auto& route : routes) {
        dst_table->add_route(route);
    }
    
    return true;
}
