#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>

namespace RouterSim {

// Protocol types
enum class ProtocolType {
    BGP,
    OSPF,
    ISIS,
    STATIC,
    CONNECTED
};

// Route information
struct Route {
    std::string prefix;
    std::string next_hop;
    uint32_t metric;
    ProtocolType protocol;
    uint32_t admin_distance;
    std::string interface;
    bool active;
    
    Route() : metric(0), protocol(ProtocolType::STATIC), admin_distance(255), active(false) {}
};

// Neighbor information
struct Neighbor {
    std::string address;
    uint16_t as_number;
    std::string state;
    uint32_t uptime;
    uint32_t messages_sent;
    uint32_t messages_received;
    bool active;
    
    Neighbor() : as_number(0), uptime(0), messages_sent(0), messages_received(0), active(false) {}
};

// Interface information
struct Interface {
    std::string name;
    std::string ip_address;
    std::string netmask;
    bool up;
    uint64_t bytes_sent;
    uint64_t bytes_received;
    uint64_t packets_sent;
    uint64_t packets_received;
    uint32_t mtu;
    
    Interface() : up(false), bytes_sent(0), bytes_received(0), 
                  packets_sent(0), packets_received(0), mtu(1500) {}
};

// Protocol statistics
struct ProtocolStats {
    uint64_t packets_processed;
    uint64_t packets_dropped;
    uint64_t routes_advertised;
    uint64_t routes_withdrawn;
    uint64_t neighbors_up;
    uint64_t neighbors_down;
    std::chrono::steady_clock::time_point start_time;
    
    ProtocolStats() : packets_processed(0), packets_dropped(0), 
                      routes_advertised(0), routes_withdrawn(0),
                      neighbors_up(0), neighbors_down(0) {
        start_time = std::chrono::steady_clock::now();
    }
};

// Protocol event callbacks
using PacketCallback = std::function<void(const std::string&, const std::vector<uint8_t>&)>;
using RouteCallback = std::function<void(const std::string&, const Route&)>;
using NeighborCallback = std::function<void(const std::string&, const Neighbor&)>;

// Base protocol interface
class IProtocol {
public:
    virtual ~IProtocol() = default;
    
    // Lifecycle
    virtual bool initialize() = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual bool is_running() const = 0;
    
    // Protocol identification
    virtual ProtocolType get_type() const = 0;
    virtual std::string get_name() const = 0;
    virtual std::string get_version() const = 0;
    
    // Packet processing
    virtual void process_packet(const std::vector<uint8_t>& packet) = 0;
    virtual bool can_handle_packet(const std::vector<uint8_t>& packet) const = 0;
    
    // Route management
    virtual std::vector<Route> get_routes() const = 0;
    virtual bool add_route(const Route& route) = 0;
    virtual bool remove_route(const std::string& prefix) = 0;
    virtual bool update_route(const Route& route) = 0;
    
    // Neighbor management
    virtual std::vector<Neighbor> get_neighbors() const = 0;
    virtual bool add_neighbor(const Neighbor& neighbor) = 0;
    virtual bool remove_neighbor(const std::string& address) = 0;
    virtual bool update_neighbor(const Neighbor& neighbor) = 0;
    
    // Interface management
    virtual std::vector<Interface> get_interfaces() const = 0;
    virtual bool add_interface(const Interface& interface) = 0;
    virtual bool remove_interface(const std::string& name) = 0;
    virtual bool update_interface(const Interface& interface) = 0;
    
    // Statistics
    virtual ProtocolStats get_statistics() const = 0;
    virtual void reset_statistics() = 0;
    
    // Event callbacks
    virtual void set_packet_callback(PacketCallback callback) = 0;
    virtual void set_route_callback(RouteCallback callback) = 0;
    virtual void set_neighbor_callback(NeighborCallback callback) = 0;
    
    // Configuration
    virtual bool load_config(const std::string& config) = 0;
    virtual std::string save_config() const = 0;
};

// Protocol factory
class ProtocolFactory {
public:
    static std::unique_ptr<IProtocol> create_protocol(ProtocolType type);
    static std::vector<ProtocolType> get_supported_protocols();
    static std::string get_protocol_name(ProtocolType type);
};

} // namespace RouterSim