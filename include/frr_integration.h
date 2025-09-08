#pragma once

#include "router_sim.h"
#include <memory>
#include <vector>
#include <map>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>

namespace RouterSim {

// FRR integration class for BGP/OSPF/ISIS control plane
class FRRIntegration {
public:
    FRRIntegration();
    ~FRRIntegration();

    // Initialization
    bool initialize();
    void shutdown();

    // BGP functionality
    bool startBGP(uint32_t local_as, const std::string& router_id);
    bool stopBGP();
    bool addBGPNeighbor(const std::string& neighbor_ip, uint32_t remote_as, 
                       const std::string& interface = "");
    bool removeBGPNeighbor(const std::string& neighbor_ip);
    bool advertiseRoute(const std::string& network, uint8_t prefix_len, 
                      const std::string& next_hop = "");
    bool withdrawRoute(const std::string& network, uint8_t prefix_len);
    std::vector<BGPNeighbor> getBGPNeighbors() const;
    std::vector<Route> getBGPRoutes() const;

    // OSPF functionality
    bool startOSPF(uint32_t router_id);
    bool stopOSPF();
    bool addOSPFInterface(const std::string& interface, const std::string& area_id,
                         uint16_t cost = 1, const std::string& network_type = "broadcast");
    bool removeOSPFInterface(const std::string& interface);
    bool addOSPFArea(const std::string& area_id, const std::string& area_type = "normal");
    std::vector<OSPFArea> getOSPFAreas() const;
    std::vector<Route> getOSPFRoutes() const;

    // ISIS functionality
    bool startISIS(const std::string& system_id, uint8_t level = 3);
    bool stopISIS();
    bool addISISInterface(const std::string& interface, uint8_t level = 3,
                         uint16_t metric = 10);
    bool removeISISInterface(const std::string& interface);
    std::vector<ISISLevel> getISISLevels() const;
    std::vector<Route> getISISRoutes() const;

    // General routing
    std::vector<Route> getAllRoutes() const;
    std::vector<Route> getRoutesByProtocol(const std::string& protocol) const;
    bool addStaticRoute(const std::string& destination, uint8_t prefix_len,
                       const std::string& next_hop, const std::string& interface = "");
    bool removeStaticRoute(const std::string& destination, uint8_t prefix_len);

    // Event handling
    void registerRouteChangeCallback(std::function<void(const Route&, bool)> callback);
    void registerNeighborChangeCallback(std::function<void(const BGPNeighbor&, bool)> callback);

    // Statistics
    struct FRRStatistics {
        uint64_t bgp_updates_sent = 0;
        uint64_t bgp_updates_received = 0;
        uint64_t ospf_lsas_sent = 0;
        uint64_t ospf_lsas_received = 0;
        uint64_t isis_lsps_sent = 0;
        uint64_t isis_lsps_received = 0;
        uint32_t active_bgp_neighbors = 0;
        uint32_t active_ospf_interfaces = 0;
        uint32_t active_isis_interfaces = 0;
    };
    
    FRRStatistics getStatistics() const;

private:
    // FRR daemon management
    bool startFRRDaemon(const std::string& daemon);
    bool stopFRRDaemon(const std::string& daemon);
    bool isFRRDaemonRunning(const std::string& daemon) const;
    
    // Configuration management
    bool generateFRRConfig();
    bool applyFRRConfig();
    std::string generateBGPConfig() const;
    std::string generateOSPFConfig() const;
    std::string generateISISConfig() const;
    
    // Event processing
    void processFRREvents();
    void handleRouteChange(const std::string& route_data);
    void handleNeighborChange(const std::string& neighbor_data);
    
    // Internal state
    std::atomic<bool> initialized_;
    std::atomic<bool> bgp_running_;
    std::atomic<bool> ospf_running_;
    std::atomic<bool> isis_running_;
    
    // Configuration
    uint32_t local_as_;
    std::string router_id_;
    std::string system_id_;
    
    // Neighbors and areas
    std::map<std::string, BGPNeighbor> bgp_neighbors_;
    std::map<std::string, OSPFArea> ospf_areas_;
    std::map<uint8_t, ISISLevel> isis_levels_;
    
    // Routes
    std::vector<Route> routes_;
    
    // Callbacks
    std::function<void(const Route&, bool)> route_change_callback_;
    std::function<void(const BGPNeighbor&, bool)> neighbor_change_callback_;
    
    // Threading
    std::thread event_thread_;
    std::atomic<bool> event_thread_running_;
    mutable std::mutex state_mutex_;
    
    // Statistics
    mutable std::mutex stats_mutex_;
    FRRStatistics stats_;
    
    // FRR process management
    std::map<std::string, pid_t> daemon_pids_;
};

} // namespace RouterSim