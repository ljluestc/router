#pragma once

#include "common_structures.h"
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <atomic>
#include <thread>

namespace RouterSim {

// Forward declarations
class RoutingTable;

// Packet processor class
class PacketProcessor {
public:
    PacketProcessor();
    ~PacketProcessor();
    
    // Core functionality
    bool initialize();
    bool start();
    bool stop();
    bool is_running() const;
    
    // Packet processing
    ProcessingResult process_packet(Packet& packet);
    bool forward_packet(const Packet& packet, const std::string& interface);
    bool drop_packet(const Packet& packet, const std::string& reason);
    
    // Routing
    std::string lookup_route(const std::string& destination) const;
    bool add_route(const router_sim::RouteInfo& route);
    bool remove_route(const std::string& destination, uint8_t prefix_length);
    std::vector<router_sim::RouteInfo> get_routes() const;
    
    // Interface management
    bool add_interface(const std::string& name, const std::string& ip, const std::string& mask);
    bool remove_interface(const std::string& name);
    bool is_interface_up(const std::string& name) const;
    std::vector<std::string> get_interfaces() const;
    
    // Statistics
    std::map<std::string, uint64_t> get_statistics() const;
    void reset_statistics();
    
    // Configuration
    void set_routing_table(std::shared_ptr<RoutingTable> routing_table);
    void set_max_packet_size(uint32_t size);
    void set_processing_timeout(uint32_t timeout_ms);
    
private:
    // Internal methods
    ProcessingResult parse_packet(const Packet& packet);
    ProcessingResult route_packet(const Packet& packet);
    bool validate_packet(const Packet& packet);
    void update_statistics(const Packet& packet, ProcessingResult result);
    
    // Packet parsing
    bool is_ip_packet(const Packet& packet) const;
    bool is_tcp_packet(const Packet& packet) const;
    bool is_udp_packet(const Packet& packet) const;
    bool is_icmp_packet(const Packet& packet) const;
    std::string extract_destination_ip(const Packet& packet) const;
    std::string extract_source_ip(const Packet& packet) const;
    
    // State
    std::atomic<bool> running_;
    std::atomic<bool> initialized_;
    
    // Components
    std::shared_ptr<RoutingTable> routing_table_;
    
    // Configuration
    uint32_t max_packet_size_;
    uint32_t processing_timeout_ms_;
    
    // Statistics
    uint64_t packets_processed_;
    uint64_t packets_forwarded_;
    uint64_t packets_dropped_;
    uint64_t packets_consumed_;
    uint64_t bytes_processed_;
    uint64_t routing_lookups_;
    uint64_t routing_misses_;
    mutable std::mutex stats_mutex_;
};

} // namespace RouterSim