#pragma once

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <mutex>
#include <chrono>
#include <set>

namespace router_sim {

// Cloud networking concepts
struct VPC {
    std::string id;
    std::string name;
    std::string cidr_block;
    std::string region;
    std::string state;
    std::map<std::string, std::string> tags;
    std::vector<std::string> subnets;
    std::vector<std::string> route_tables;
    std::vector<std::string> security_groups;
    std::chrono::steady_clock::time_point created_at;
};

struct Subnet {
    std::string id;
    std::string vpc_id;
    std::string cidr_block;
    std::string availability_zone;
    std::string state;
    bool map_public_ip_on_launch;
    std::map<std::string, std::string> tags;
    std::vector<std::string> instances;
    std::chrono::steady_clock::time_point created_at;
};

struct RouteTable {
    std::string id;
    std::string vpc_id;
    bool main;
    std::vector<Route> routes;
    std::vector<std::string> associations;
    std::map<std::string, std::string> tags;
    std::chrono::steady_clock::time_point created_at;
};

struct SecurityGroup {
    std::string id;
    std::string vpc_id;
    std::string name;
    std::string description;
    std::vector<SecurityGroupRule> rules;
    std::map<std::string, std::string> tags;
    std::chrono::steady_clock::time_point created_at;
};

struct SecurityGroupRule {
    std::string id;
    std::string security_group_id;
    std::string type; // "ingress" or "egress"
    std::string protocol; // "tcp", "udp", "icmp", etc.
    uint16_t from_port;
    uint16_t to_port;
    std::string cidr_blocks;
    std::string source_security_group_id;
    std::string description;
};

struct Instance {
    std::string id;
    std::string vpc_id;
    std::string subnet_id;
    std::string instance_type;
    std::string state;
    std::string public_ip;
    std::string private_ip;
    std::vector<std::string> security_groups;
    std::map<std::string, std::string> tags;
    std::chrono::steady_clock::time_point launched_at;
};

struct NATGateway {
    std::string id;
    std::string subnet_id;
    std::string state;
    std::string public_ip;
    std::string private_ip;
    std::string vpc_id;
    std::map<std::string, std::string> tags;
    std::chrono::steady_clock::time_point created_at;
};

struct LoadBalancer {
    std::string id;
    std::string name;
    std::string vpc_id;
    std::string state;
    std::string type; // "application" or "network"
    std::vector<std::string> subnets;
    std::vector<std::string> security_groups;
    std::vector<std::string> target_groups;
    std::map<std::string, std::string> tags;
    std::chrono::steady_clock::time_point created_at;
};

struct TargetGroup {
    std::string id;
    std::string name;
    std::string vpc_id;
    std::string protocol;
    uint16_t port;
    std::string target_type; // "instance" or "ip"
    std::vector<std::string> targets;
    std::map<std::string, std::string> health_check;
    std::map<std::string, std::string> tags;
    std::chrono::steady_clock::time_point created_at;
};

// Multi-tenancy support
struct Tenant {
    std::string id;
    std::string name;
    std::string description;
    std::set<std::string> vpcs;
    std::set<std::string> users;
    std::map<std::string, std::string> quotas;
    std::map<std::string, std::string> tags;
    std::chrono::steady_clock::time_point created_at;
};

class CloudNetworkingManager {
public:
    CloudNetworkingManager();
    ~CloudNetworkingManager();
    
    bool initialize();
    bool start();
    bool stop();
    bool is_running() const;
    
    // VPC management
    bool create_vpc(const std::string& name, const std::string& cidr_block, 
                   const std::string& region, VPC& vpc);
    bool delete_vpc(const std::string& vpc_id);
    bool get_vpc(const std::string& vpc_id, VPC& vpc) const;
    std::vector<VPC> list_vpcs() const;
    
    // Subnet management
    bool create_subnet(const std::string& vpc_id, const std::string& cidr_block,
                      const std::string& availability_zone, Subnet& subnet);
    bool delete_subnet(const std::string& subnet_id);
    bool get_subnet(const std::string& subnet_id, Subnet& subnet) const;
    std::vector<Subnet> list_subnets() const;
    
    // Route table management
    bool create_route_table(const std::string& vpc_id, RouteTable& route_table);
    bool delete_route_table(const std::string& route_table_id);
    bool associate_route_table(const std::string& route_table_id, const std::string& subnet_id);
    bool disassociate_route_table(const std::string& association_id);
    bool get_route_table(const std::string& route_table_id, RouteTable& route_table) const;
    std::vector<RouteTable> list_route_tables() const;
    
    // Security group management
    bool create_security_group(const std::string& vpc_id, const std::string& name,
                              const std::string& description, SecurityGroup& security_group);
    bool delete_security_group(const std::string& security_group_id);
    bool add_security_group_rule(const std::string& security_group_id, const SecurityGroupRule& rule);
    bool remove_security_group_rule(const std::string& security_group_id, const std::string& rule_id);
    bool get_security_group(const std::string& security_group_id, SecurityGroup& security_group) const;
    std::vector<SecurityGroup> list_security_groups() const;
    
    // Instance management
    bool launch_instance(const std::string& vpc_id, const std::string& subnet_id,
                        const std::string& instance_type, Instance& instance);
    bool terminate_instance(const std::string& instance_id);
    bool get_instance(const std::string& instance_id, Instance& instance) const;
    std::vector<Instance> list_instances() const;
    
    // NAT Gateway management
    bool create_nat_gateway(const std::string& subnet_id, NATGateway& nat_gateway);
    bool delete_nat_gateway(const std::string& nat_gateway_id);
    bool get_nat_gateway(const std::string& nat_gateway_id, NATGateway& nat_gateway) const;
    std::vector<NATGateway> list_nat_gateways() const;
    
    // Load balancer management
    bool create_load_balancer(const std::string& name, const std::string& vpc_id,
                             const std::vector<std::string>& subnets, LoadBalancer& load_balancer);
    bool delete_load_balancer(const std::string& load_balancer_id);
    bool get_load_balancer(const std::string& load_balancer_id, LoadBalancer& load_balancer) const;
    std::vector<LoadBalancer> list_load_balancers() const;
    
    // Target group management
    bool create_target_group(const std::string& name, const std::string& vpc_id,
                            const std::string& protocol, uint16_t port, TargetGroup& target_group);
    bool delete_target_group(const std::string& target_group_id);
    bool register_target(const std::string& target_group_id, const std::string& target_id);
    bool deregister_target(const std::string& target_group_id, const std::string& target_id);
    bool get_target_group(const std::string& target_group_id, TargetGroup& target_group) const;
    std::vector<TargetGroup> list_target_groups() const;
    
    // Multi-tenancy
    bool create_tenant(const std::string& name, const std::string& description, Tenant& tenant);
    bool delete_tenant(const std::string& tenant_id);
    bool add_tenant_vpc(const std::string& tenant_id, const std::string& vpc_id);
    bool remove_tenant_vpc(const std::string& tenant_id, const std::string& vpc_id);
    bool get_tenant(const std::string& tenant_id, Tenant& tenant) const;
    std::vector<Tenant> list_tenants() const;
    
    // Statistics
    struct CloudStatistics {
        uint64_t vpcs_created;
        uint64_t subnets_created;
        uint64_t instances_launched;
        uint64_t nat_gateways_created;
        uint64_t load_balancers_created;
        uint64_t tenants_created;
        std::map<std::string, uint64_t> region_stats;
        std::map<std::string, uint64_t> instance_type_stats;
    };
    CloudStatistics get_statistics() const;
    
private:
    std::string generate_id(const std::string& prefix);
    bool validate_cidr_block(const std::string& cidr_block);
    bool is_ip_in_cidr(const std::string& ip, const std::string& cidr_block);
    bool check_tenant_quota(const std::string& tenant_id, const std::string& resource_type);
    
    std::atomic<bool> running_;
    mutable std::mutex vpcs_mutex_;
    mutable std::mutex subnets_mutex_;
    mutable std::mutex route_tables_mutex_;
    mutable std::mutex security_groups_mutex_;
    mutable std::mutex instances_mutex_;
    mutable std::mutex nat_gateways_mutex_;
    mutable std::mutex load_balancers_mutex_;
    mutable std::mutex target_groups_mutex_;
    mutable std::mutex tenants_mutex_;
    
    std::map<std::string, VPC> vpcs_;
    std::map<std::string, Subnet> subnets_;
    std::map<std::string, RouteTable> route_tables_;
    std::map<std::string, SecurityGroup> security_groups_;
    std::map<std::string, Instance> instances_;
    std::map<std::string, NATGateway> nat_gateways_;
    std::map<std::string, LoadBalancer> load_balancers_;
    std::map<std::string, TargetGroup> target_groups_;
    std::map<std::string, Tenant> tenants_;
    
    CloudStatistics stats_;
};

} // namespace router_sim
