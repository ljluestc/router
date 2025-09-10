#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace router_sim {

struct BGPNeighbor {
    std::string address;
    uint32_t as_number;
    std::string state;
    uint32_t hold_time;
    uint32_t keepalive_interval;
    uint64_t messages_sent;
    uint64_t messages_received;
    std::string last_error;
};

struct Route {
    std::string destination;
    std::string next_hop;
    std::string protocol;
    uint32_t metric;
    uint32_t distance;
};

struct Interface {
    std::string name;
    std::string status;
    std::string ip_address;
    uint32_t mtu;
    uint64_t bytes_in;
    uint64_t bytes_out;
    uint64_t packets_in;
    uint64_t packets_out;
};

class FRRClient {
public:
    FRRClient();
    ~FRRClient();

    // Connection management
    bool connect();
    bool disconnect();
    bool is_connected() const;

    // Command execution
    bool send_command(const std::string& command);
    std::string receive_response();
    bool execute_command(const std::string& command, std::string& response);

    // BGP Configuration
    bool configure_bgp(uint32_t as_number, const std::string& router_id);
    bool add_bgp_neighbor(const std::string& neighbor_ip, uint32_t remote_as);
    bool remove_bgp_neighbor(const std::string& neighbor_ip);
    std::vector<BGPNeighbor> get_bgp_neighbors();

    // OSPF Configuration
    bool configure_ospf(const std::string& router_id, uint32_t area);
    bool add_ospf_interface(const std::string& interface, uint32_t area, uint32_t cost);

    // ISIS Configuration
    bool configure_isis(const std::string& system_id, uint8_t level);
    bool add_isis_interface(const std::string& interface, uint8_t level);

    // Route Management
    std::vector<Route> get_routes();
    bool add_static_route(const std::string& destination, const std::string& next_hop, uint32_t distance = 1);
    bool remove_static_route(const std::string& destination, const std::string& next_hop);

    // Interface Management
    std::vector<Interface> get_interfaces();

    // Configuration
    void set_local_as(uint32_t as_number);
    uint32_t get_local_as() const;

private:
    bool connected_;
    int socket_fd_;
    uint32_t local_as_;
};

} // namespace router_sim