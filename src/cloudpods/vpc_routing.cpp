#include "cloudpods/vpc_routing.h"
#include "analytics/clickhouse_client.h"
#include <iostream>
#include <algorithm>
#include <chrono>

namespace RouterSim {
namespace CloudPods {

VPCRouting::VPCRouting(const std::string& vpc_id, const VPCRoutingConfig& config)
    : vpc_id_(vpc_id), config_(config), analytics_client_(nullptr) {
    // Initialize VPC routing table
    routing_table_.clear();
    nat_gateways_.clear();
    load_balancers_.clear();
    service_mesh_routes_.clear();
}

VPCRouting::~VPCRouting() = default;

void VPCRouting::setAnalyticsClient(std::shared_ptr<Analytics::ClickHouseClient> client) {
    analytics_client_ = client;
}

bool VPCRouting::addSubnet(const std::string& subnet_id, const std::string& cidr, 
                          const std::string& availability_zone) {
    SubnetInfo subnet;
    subnet.subnet_id = subnet_id;
    subnet.cidr = cidr;
    subnet.availability_zone = availability_zone;
    subnet.created_at = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    subnets_[subnet_id] = subnet;
    
    // Add route to routing table
    RouteEntry route;
    route.destination = cidr;
    route.target = "local";
    route.target_id = subnet_id;
    route.state = "active";
    route.created_at = subnet.created_at;
    
    routing_table_[cidr] = route;
    
    std::cout << "Added subnet " << subnet_id << " with CIDR " << cidr 
              << " in AZ " << availability_zone << std::endl;
    
    return true;
}

bool VPCRouting::addNATGateway(const std::string& nat_id, const std::string& subnet_id,
                              const std::string& elastic_ip) {
    NATGatewayInfo nat;
    nat.nat_id = nat_id;
    nat.subnet_id = subnet_id;
    nat.elastic_ip = elastic_ip;
    nat.state = "available";
    nat.created_at = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    nat_gateways_[nat_id] = nat;
    
    // Add default route for internet access
    RouteEntry route;
    route.destination = "0.0.0.0/0";
    route.target = "nat-gateway";
    route.target_id = nat_id;
    route.state = "active";
    route.created_at = nat.created_at;
    
    routing_table_["0.0.0.0/0"] = route;
    
    std::cout << "Added NAT Gateway " << nat_id << " in subnet " << subnet_id 
              << " with EIP " << elastic_ip << std::endl;
    
    return true;
}

bool VPCRouting::addLoadBalancer(const std::string& lb_id, const std::string& subnet_id,
                                const std::vector<std::string>& target_groups) {
    LoadBalancerInfo lb;
    lb.lb_id = lb_id;
    lb.subnet_id = subnet_id;
    lb.target_groups = target_groups;
    lb.state = "active";
    lb.created_at = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    load_balancers_[lb_id] = lb;
    
    std::cout << "Added Load Balancer " << lb_id << " in subnet " << subnet_id 
              << " with " << target_groups.size() << " target groups" << std::endl;
    
    return true;
}

bool VPCRouting::addServiceMeshRoute(const std::string& service_name, 
                                   const std::string& service_ip,
                                   const std::vector<std::string>& endpoints) {
    ServiceMeshRoute route;
    route.service_name = service_name;
    route.service_ip = service_ip;
    route.endpoints = endpoints;
    route.state = "active";
    route.created_at = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    service_mesh_routes_[service_name] = route;
    
    // Add route to routing table
    RouteEntry rt_entry;
    rt_entry.destination = service_ip + "/32";
    rt_entry.target = "service-mesh";
    rt_entry.target_id = service_name;
    rt_entry.state = "active";
    rt_entry.created_at = route.created_at;
    
    routing_table_[service_ip + "/32"] = rt_entry;
    
    std::cout << "Added Service Mesh route for " << service_name 
              << " with IP " << service_ip << " and " << endpoints.size() 
              << " endpoints" << std::endl;
    
    return true;
}

std::string VPCRouting::routePacket(const std::string& src_ip, const std::string& dst_ip,
                                  const std::string& protocol, uint16_t port) {
    // Find the best matching route
    std::string best_route = "drop";
    int best_prefix_length = -1;
    
    for (const auto& [destination, route] : routing_table_) {
        if (route.state != "active") continue;
        
        // Check if destination matches (simplified CIDR matching)
        if (matchesCIDR(dst_ip, destination)) {
            int prefix_length = getPrefixLength(destination);
            if (prefix_length > best_prefix_length) {
                best_prefix_length = prefix_length;
                best_route = route.target;
            }
        }
    }
    
    // Log routing decision
    if (analytics_client_) {
        Analytics::RoutingEvent event;
        event.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        event.router_id = vpc_id_;
        event.protocol = "VPC_ROUTING";
        event.event_type = "PACKET_ROUTE";
        event.prefix = dst_ip;
        event.next_hop = best_route;
        event.metric = 0;
        event.success = (best_route != "drop") ? 1 : 0;
        
        analytics_client_->insertRoutingEvent(event);
    }
    
    return best_route;
}

bool VPCRouting::matchesCIDR(const std::string& ip, const std::string& cidr) {
    // Simplified CIDR matching - in real implementation, use proper IP address parsing
    size_t slash_pos = cidr.find('/');
    if (slash_pos == std::string::npos) {
        return ip == cidr;
    }
    
    std::string network = cidr.substr(0, slash_pos);
    return ip.substr(0, network.length()) == network;
}

int VPCRouting::getPrefixLength(const std::string& cidr) {
    size_t slash_pos = cidr.find('/');
    if (slash_pos == std::string::npos) {
        return 32; // Assume /32 if no prefix length specified
    }
    
    return std::stoi(cidr.substr(slash_pos + 1));
}

VPCRoutingStats VPCRouting::getStats() const {
    VPCRoutingStats stats;
    stats.vpc_id = vpc_id_;
    stats.subnet_count = subnets_.size();
    stats.nat_gateway_count = nat_gateways_.size();
    stats.load_balancer_count = load_balancers_.size();
    stats.service_mesh_routes = service_mesh_routes_.size();
    stats.total_routes = routing_table_.size();
    stats.active_routes = 0;
    
    for (const auto& [_, route] : routing_table_) {
        if (route.state == "active") {
            stats.active_routes++;
        }
    }
    
    return stats;
}

std::vector<RouteEntry> VPCRouting::getRoutes() const {
    std::vector<RouteEntry> routes;
    for (const auto& [_, route] : routing_table_) {
        routes.push_back(route);
    }
    return routes;
}

std::vector<SubnetInfo> VPCRouting::getSubnets() const {
    std::vector<SubnetInfo> subnets;
    for (const auto& [_, subnet] : subnets_) {
        subnets.push_back(subnet);
    }
    return subnets;
}

std::vector<NATGatewayInfo> VPCRouting::getNATGateways() const {
    std::vector<NATGatewayInfo> nats;
    for (const auto& [_, nat] : nat_gateways_) {
        nats.push_back(nat);
    }
    return nats;
}

std::vector<LoadBalancerInfo> VPCRouting::getLoadBalancers() const {
    std::vector<LoadBalancerInfo> lbs;
    for (const auto& [_, lb] : load_balancers_) {
        lbs.push_back(lb);
    }
    return lbs;
}

std::vector<ServiceMeshRoute> VPCRouting::getServiceMeshRoutes() const {
    std::vector<ServiceMeshRoute> routes;
    for (const auto& [_, route] : service_mesh_routes_) {
        routes.push_back(route);
    }
    return routes;
}

} // namespace CloudPods
} // namespace RouterSim
