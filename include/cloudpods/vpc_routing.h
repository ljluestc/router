#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <cstdint>
#include <chrono>

namespace RouterSim {
namespace CloudPods {

// Forward declaration
namespace Analytics {
    class ClickHouseClient;
}

struct VPCRoutingConfig {
    std::string region;
    std::string cidr_block;
    bool enable_dns_hostnames;
    bool enable_dns_support;
    bool enable_ipv6;
    std::string tenancy;
};

struct SubnetInfo {
    std::string subnet_id;
    std::string cidr;
    std::string availability_zone;
    int64_t created_at;
};

struct NATGatewayInfo {
    std::string nat_id;
    std::string subnet_id;
    std::string elastic_ip;
    std::string state;
    int64_t created_at;
};

struct LoadBalancerInfo {
    std::string lb_id;
    std::string subnet_id;
    std::vector<std::string> target_groups;
    std::string state;
    int64_t created_at;
};

struct ServiceMeshRoute {
    std::string service_name;
    std::string service_ip;
    std::vector<std::string> endpoints;
    std::string state;
    int64_t created_at;
};

struct RouteEntry {
    std::string destination;
    std::string target;
    std::string target_id;
    std::string state;
    int64_t created_at;
};

struct VPCRoutingStats {
    std::string vpc_id;
    uint32_t subnet_count;
    uint32_t nat_gateway_count;
    uint32_t load_balancer_count;
    uint32_t service_mesh_routes;
    uint32_t total_routes;
    uint32_t active_routes;
};

class VPCRouting {
public:
    VPCRouting(const std::string& vpc_id, const VPCRoutingConfig& config);
    ~VPCRouting();

    void setAnalyticsClient(std::shared_ptr<Analytics::ClickHouseClient> client);

    // VPC Management
    bool addSubnet(const std::string& subnet_id, const std::string& cidr, 
                   const std::string& availability_zone);
    bool removeSubnet(const std::string& subnet_id);

    // NAT Gateway Management
    bool addNATGateway(const std::string& nat_id, const std::string& subnet_id,
                      const std::string& elastic_ip);
    bool removeNATGateway(const std::string& nat_id);

    // Load Balancer Management
    bool addLoadBalancer(const std::string& lb_id, const std::string& subnet_id,
                        const std::vector<std::string>& target_groups);
    bool removeLoadBalancer(const std::string& lb_id);

    // Service Mesh Management
    bool addServiceMeshRoute(const std::string& service_name, 
                           const std::string& service_ip,
                           const std::vector<std::string>& endpoints);
    bool removeServiceMeshRoute(const std::string& service_name);

    // Routing Operations
    std::string routePacket(const std::string& src_ip, const std::string& dst_ip,
                          const std::string& protocol, uint16_t port);

    // Statistics and Monitoring
    VPCRoutingStats getStats() const;
    std::vector<RouteEntry> getRoutes() const;
    std::vector<SubnetInfo> getSubnets() const;
    std::vector<NATGatewayInfo> getNATGateways() const;
    std::vector<LoadBalancerInfo> getLoadBalancers() const;
    std::vector<ServiceMeshRoute> getServiceMeshRoutes() const;

private:
    std::string vpc_id_;
    VPCRoutingConfig config_;
    std::map<std::string, SubnetInfo> subnets_;
    std::map<std::string, NATGatewayInfo> nat_gateways_;
    std::map<std::string, LoadBalancerInfo> load_balancers_;
    std::map<std::string, ServiceMeshRoute> service_mesh_routes_;
    std::map<std::string, RouteEntry> routing_table_;
    std::shared_ptr<Analytics::ClickHouseClient> analytics_client_;

    // Helper methods
    bool matchesCIDR(const std::string& ip, const std::string& cidr);
    int getPrefixLength(const std::string& cidr);
};

} // namespace CloudPods
} // namespace RouterSim
