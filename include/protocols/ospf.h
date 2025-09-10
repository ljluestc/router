#pragma once

#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <atomic>
#include <thread>
#include <chrono>
#include <functional>
#include <memory>

namespace router_sim {

struct OSPFInterface {
    std::string name;
    uint32_t area_id;
    uint32_t cost;
    uint32_t hello_interval;
    uint32_t dead_interval;
    uint32_t retransmit_interval;
    uint32_t transit_delay;
    uint32_t priority;
    std::string state;
    std::string network;
    uint32_t neighbors_count;
    uint64_t hello_sent = 0;
    uint64_t hello_received = 0;
};

struct OSPFRoute {
    std::string prefix;
    uint32_t area_id;
    uint32_t cost;
    std::string type;
    bool is_valid;
    std::map<std::string, std::string> attributes;
    std::chrono::system_clock::time_point timestamp;
};

struct OSPFNeighbor {
    std::string router_id;
    std::string state;
    uint32_t priority;
    uint32_t cost;
    std::string interface;
    std::chrono::system_clock::time_point established_time;
    std::chrono::system_clock::time_point last_hello_received;
    uint64_t hello_sent = 0;
    uint64_t hello_received = 0;
};

struct OSPFConfig {
    std::map<std::string, std::string> parameters;
    bool enabled;
    uint32_t update_interval_ms;
    uint32_t area_id;
    std::string router_id;
    uint32_t hello_interval;
    uint32_t dead_interval;
    uint32_t retransmit_interval;
    uint32_t transit_delay;
    uint32_t priority;
    uint32_t cost;
};

struct OSPFStatistics {
    uint64_t lsa_sent;
    uint64_t lsa_received;
    uint64_t hello_sent;
    uint64_t hello_received;
    uint64_t dd_sent;
    uint64_t dd_received;
    uint64_t lsr_sent;
    uint64_t lsr_received;
    uint64_t lsu_sent;
    uint64_t lsu_received;
    uint64_t lsack_sent;
    uint64_t lsack_received;
};

using RouteUpdateCallback = std::function<void(const RouteInfo&, bool)>;
using NeighborCallback = std::function<void(const NeighborInfo&, bool)>;

class OSPFProtocol {
public:
    OSPFProtocol();
    ~OSPFProtocol();

    bool initialize(const ProtocolConfig& config);
    bool start();
    bool stop();
    bool is_running() const;

    // Interface management
    bool add_interface(const std::string& interface, const std::map<std::string, std::string>& config);
    bool remove_interface(const std::string& interface);

    // Route management
    bool advertise_route(const std::string& prefix, const std::map<std::string, std::string>& attributes = {});
    bool withdraw_route(const std::string& prefix);

    // Route and neighbor info
    std::vector<RouteInfo> get_routes() const;
    std::vector<NeighborInfo> get_neighbors() const;

    // Statistics
    std::map<std::string, uint64_t> get_statistics() const;

    // Configuration
    void update_configuration(const ProtocolConfig& config);
    ProtocolConfig get_configuration() const;

    // Callbacks
    void set_route_update_callback(RouteUpdateCallback callback);
    void set_neighbor_update_callback(NeighborCallback callback);

private:
    // Thread functions
    void ospf_main_loop();
    void hello_loop();
    void lsa_processing_loop();
    void spf_calculation_loop();

    // OSPF operations
    bool send_hello_message(const std::string& interface);

    // Message processing
    void process_incoming_messages();
    void update_interface_states();
    void process_lsa_updates();
    void flood_lsas();
    void run_spf_calculation();

    // Configuration and state
    OSPFConfig config_;
    uint32_t area_id_;
    std::string router_id_;
    std::atomic<bool> running_;
    mutable std::mutex config_mutex_;
    mutable std::mutex routes_mutex_;
    mutable std::mutex interfaces_mutex_;
    mutable std::mutex neighbors_mutex_;
    mutable std::mutex stats_mutex_;

    // Data structures
    std::map<std::string, OSPFRoute> advertised_routes_;
    std::map<std::string, OSPFRoute> learned_routes_;
    std::map<std::string, OSPFInterface> interfaces_;
    std::map<std::string, OSPFNeighbor> neighbors_;

    // Statistics
    OSPFStatistics stats_;

    // Threads
    std::thread ospf_thread_;
    std::thread hello_thread_;
    std::thread lsa_thread_;
    std::thread spf_thread_;

    // Callbacks
    RouteUpdateCallback route_update_callback_;
    NeighborCallback neighbor_update_callback_;
};

} // namespace router_sim
