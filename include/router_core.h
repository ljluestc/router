#pragma once

#include <memory>
#include <vector>
#include <map>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <queue>
#include <functional>

namespace RouterSim {

struct Packet {
    std::vector<uint8_t> data;
    std::string source_interface;
    std::string dest_interface;
    uint64_t timestamp;
    uint32_t size;
};

struct Interface {
    std::string name;
    std::string ip_address;
    std::string subnet_mask;
    bool is_up;
    uint32_t mtu;
    uint64_t bytes_sent;
    uint64_t bytes_received;
    uint64_t packets_sent;
    uint64_t packets_received;
};

struct Route {
    std::string network;
    std::string next_hop;
    std::string interface;
    uint32_t metric;
    std::string protocol;
    bool is_active;
};

class RouterCore {
public:
    RouterCore();
    ~RouterCore();

    // Core functionality
    bool initialize();
    void start();
    void stop();
    bool is_running() const;

    // Interface management
    bool add_interface(const std::string& name, const std::string& ip, const std::string& mask);
    bool remove_interface(const std::string& name);
    bool set_interface_up(const std::string& name, bool up);
    std::vector<Interface> get_interfaces() const;

    // Route management
    bool add_route(const Route& route);
    bool remove_route(const std::string& network);
    std::vector<Route> get_routes() const;
    Route* find_route(const std::string& destination);

    // Packet processing
    void process_packet(const Packet& packet);
    void send_packet(const Packet& packet, const std::string& interface);

    // Statistics
    struct Statistics {
        uint64_t total_packets_processed;
        uint64_t total_bytes_processed;
        uint64_t routing_table_updates;
        uint64_t interface_state_changes;
    };
    Statistics get_statistics() const;

private:
    void packet_processing_loop();
    void route_update_loop();
    void interface_monitoring_loop();

    std::atomic<bool> running_;
    std::thread packet_thread_;
    std::thread route_thread_;
    std::thread interface_thread_;
    
    mutable std::mutex interfaces_mutex_;
    mutable std::mutex routes_mutex_;
    mutable std::mutex stats_mutex_;
    
    std::map<std::string, Interface> interfaces_;
    std::vector<Route> routes_;
    std::queue<Packet> packet_queue_;
    std::mutex packet_queue_mutex_;
    
    Statistics stats_;
    
    // Callbacks for protocol handlers
    std::function<void(const Packet&)> packet_handler_;
};

} // namespace RouterSim
