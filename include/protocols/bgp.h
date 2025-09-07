#pragma once

#include "protocol_interface.h"
#include <map>
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <chrono>

namespace router_sim {

class BGPProtocol : public ProtocolInterface {
public:
    BGPProtocol();
    virtual ~BGPProtocol();
    
    // ProtocolInterface implementation
    bool initialize(const std::map<std::string, std::string>& config) override;
    bool start() override;
    bool stop() override;
    bool is_running() const override;
    
    bool add_neighbor(const std::string& address, const std::map<std::string, std::string>& config) override;
    bool remove_neighbor(const std::string& address) override;
    std::vector<NeighborInfo> get_neighbors() const override;
    bool is_neighbor_established(const std::string& address) const override;
    
    bool advertise_route(const RouteInfo& route) override;
    bool withdraw_route(const std::string& destination, uint8_t prefix_length) override;
    std::vector<RouteInfo> get_routes() const override;
    
    bool update_config(const std::map<std::string, std::string>& config) override;
    std::map<std::string, std::string> get_config() const override;
    
    ProtocolStatistics get_statistics() const override;
    
    void set_route_update_callback(RouteUpdateCallback callback) override;
    void set_neighbor_update_callback(NeighborUpdateCallback callback) override;
    
private:
    bool establish_session(NeighborInfo& neighbor);
    bool send_open_message(const NeighborInfo& neighbor);
    bool receive_open_message(NeighborInfo& neighbor);
    bool send_keepalive_message(const NeighborInfo& neighbor);
    bool send_update_message(const NeighborInfo& neighbor, const RouteInfo& route, bool is_advertisement);
    void process_update_message(const NeighborInfo& neighbor, const RouteInfo& route);
    
    uint32_t local_as_;
    std::string router_id_;
    std::map<std::string, NeighborInfo> neighbors_;
    std::map<std::string, RouteInfo> routes_;
    ProtocolStatistics stats_;
    mutable std::mutex neighbors_mutex_;
    mutable std::mutex routes_mutex_;
};

} // namespace router_sim
