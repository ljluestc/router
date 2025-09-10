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

struct ISISInterface {
    std::string name;
    uint32_t level;
    uint32_t metric;
    uint32_t hello_interval;
    uint32_t hold_time;
    std::string state;
    std::string network;
    uint32_t adjacencies_count;
    uint64_t hello_sent = 0;
    uint64_t hello_received = 0;
};

struct ISISRoute {
    std::string prefix;
    std::string system_id;
    std::string area_id;
    uint32_t metric;
    uint32_t level;
    bool is_valid;
    std::map<std::string, std::string> attributes;
    std::chrono::system_clock::time_point timestamp;
};

struct ISISNeighbor {
    std::string system_id;
    std::string state;
    uint32_t level;
    uint32_t metric;
    std::string interface;
    std::chrono::system_clock::time_point established_time;
    std::chrono::system_clock::time_point last_hello_received;
    uint64_t hello_sent = 0;
    uint64_t hello_received = 0;
};

struct ISISConfig {
    std::map<std::string, std::string> parameters;
    bool enabled;
    uint32_t update_interval_ms;
    std::string system_id;
    std::string area_id;
    uint32_t level;
    uint32_t hello_interval;
    uint32_t hold_time;
    uint32_t lsp_interval;
    uint32_t metric;
};

struct ISISStatistics {
    uint64_t hello_sent;
    uint64_t hello_received;
    uint64_t lsp_sent;
    uint64_t lsp_received;
    uint64_t psnp_sent;
    uint64_t psnp_received;
    uint64_t csnp_sent;
    uint64_t csnp_received;
    uint64_t adjacencies_formed;
    uint64_t adjacencies_lost;
};

using RouteUpdateCallback = std::function<void(const RouteInfo&, bool)>;
using NeighborCallback = std::function<void(const NeighborInfo&, bool)>;

class ISISProtocol {
public:
    ISISProtocol();
    ~ISISProtocol();

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
    void isis_main_loop();
    void hello_loop();
    void lsp_processing_loop();
    void spf_calculation_loop();

    // IS-IS operations
    bool send_hello_message(const std::string& interface);

    // Message processing
    void process_incoming_messages();
    void update_interface_states();
    void process_lsp_updates();
    void flood_lsps();
    void run_spf_calculation();

    // Configuration and state
    ISISConfig config_;
    std::string system_id_;
    std::string area_id_;
    std::atomic<bool> running_;
    mutable std::mutex config_mutex_;
    mutable std::mutex routes_mutex_;
    mutable std::mutex interfaces_mutex_;
    mutable std::mutex neighbors_mutex_;
    mutable std::mutex stats_mutex_;

    // Data structures
    std::map<std::string, ISISRoute> advertised_routes_;
    std::map<std::string, ISISRoute> learned_routes_;
    std::map<std::string, ISISInterface> interfaces_;
    std::map<std::string, ISISNeighbor> neighbors_;

    // Statistics
    ISISStatistics stats_;

    // Threads
    std::thread isis_thread_;
    std::thread hello_thread_;
    std::thread lsp_thread_;
    std::thread spf_thread_;

    // Callbacks
    RouteUpdateCallback route_update_callback_;
    NeighborCallback neighbor_update_callback_;
};

} // namespace router_sim
