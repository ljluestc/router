#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <thread>
#include <atomic>
#include <mutex>
#include <queue>

namespace RouterSim {

// Forward declarations
struct Packet;
struct RouteEntry;

// Packet protocol types
enum class ProtocolType {
    ETHERNET = 0x0800,
    IPV4 = 0x0800,
    IPV6 = 0x86DD,
    ARP = 0x0806,
    ICMP = 1,
    TCP = 6,
    UDP = 17,
    OSPF = 89,
    BGP = 179,
    ISIS = 124
};

// Packet processing result
enum class ProcessingResult {
    FORWARD,
    DROP,
    CONSUME,
    ERROR
};

// Packet processor class
class PacketProcessor {
public:
    PacketProcessor();
    ~PacketProcessor();

    // Initialization
    bool initialize();
    void cleanup();

    // Packet processing
    ProcessingResult process_packet(Packet& packet);
    bool forward_packet(const Packet& packet, const std::string& next_hop);
    bool drop_packet(const Packet& packet, const std::string& reason);

    // Protocol handlers
    bool register_protocol_handler(ProtocolType protocol, 
                                 std::function<ProcessingResult(Packet&)> handler);
    bool unregister_protocol_handler(ProtocolType protocol);

    // Routing integration
    void set_routing_table(std::shared_ptr<class RoutingTable> routing_table);
    std::string lookup_route(const std::string& destination) const;

    // Statistics
    std::map<std::string, uint64_t> get_processing_stats() const;
    std::map<ProtocolType, uint64_t> get_protocol_stats() const;
    void reset_statistics();

    // Control
    void start();
    void stop();
    bool is_running() const;

private:
    // Protocol handlers
    ProcessingResult handle_ethernet(Packet& packet);
    ProcessingResult handle_ipv4(Packet& packet);
    ProcessingResult handle_ipv6(Packet& packet);
    ProcessingResult handle_arp(Packet& packet);
    ProcessingResult handle_icmp(Packet& packet);
    ProcessingResult handle_tcp(Packet& packet);
    ProcessingResult handle_udp(Packet& packet);
    ProcessingResult handle_ospf(Packet& packet);
    ProcessingResult handle_bgp(Packet& packet);
    ProcessingResult handle_isis(Packet& packet);

    // Packet parsing
    bool parse_ethernet_header(Packet& packet);
    bool parse_ipv4_header(Packet& packet);
    bool parse_ipv6_header(Packet& packet);
    bool parse_arp_header(Packet& packet);
    bool parse_icmp_header(Packet& packet);
    bool parse_tcp_header(Packet& packet);
    bool parse_udp_header(Packet& packet);

    // Packet validation
    bool validate_packet(const Packet& packet) const;
    bool validate_checksum(const Packet& packet) const;
    bool validate_length(const Packet& packet) const;

    // Internal state
    std::map<ProtocolType, std::function<ProcessingResult(Packet&)>> protocol_handlers_;
    std::shared_ptr<class RoutingTable> routing_table_;
    std::atomic<bool> running_;
    mutable std::mutex handlers_mutex_;

    // Statistics
    std::map<std::string, uint64_t> processing_stats_;
    std::map<ProtocolType, uint64_t> protocol_stats_;
    mutable std::mutex stats_mutex_;
};

// Packet utilities
class PacketUtils {
public:
    // Packet creation
    static Packet create_ethernet_packet(const std::string& src_mac, const std::string& dst_mac,
                                       ProtocolType protocol, const std::vector<uint8_t>& payload);
    static Packet create_ipv4_packet(const std::string& src_ip, const std::string& dst_ip,
                                   uint8_t protocol, const std::vector<uint8_t>& payload);
    static Packet create_icmp_packet(const std::string& src_ip, const std::string& dst_ip,
                                   uint8_t type, uint8_t code, const std::vector<uint8_t>& payload);

    // Packet parsing
    static std::string extract_src_mac(const Packet& packet);
    static std::string extract_dst_mac(const Packet& packet);
    static std::string extract_src_ip(const Packet& packet);
    static std::string extract_dst_ip(const Packet& packet);
    static uint16_t extract_src_port(const Packet& packet);
    static uint16_t extract_dst_port(const Packet& packet);
    static ProtocolType extract_protocol(const Packet& packet);

    // Checksum calculation
    static uint16_t calculate_ipv4_checksum(const std::vector<uint8_t>& data);
    static uint16_t calculate_tcp_checksum(const std::vector<uint8_t>& data, 
                                         const std::string& src_ip, const std::string& dst_ip);
    static uint16_t calculate_udp_checksum(const std::vector<uint8_t>& data,
                                         const std::string& src_ip, const std::string& dst_ip);

    // Packet manipulation
    static bool set_ttl(Packet& packet, uint8_t ttl);
    static bool set_tos(Packet& packet, uint8_t tos);
    static bool set_dscp(Packet& packet, uint8_t dscp);
    static bool set_priority(Packet& packet, uint8_t priority);

    // Packet filtering
    static bool matches_filter(const Packet& packet, const std::string& filter);
    static bool is_broadcast(const Packet& packet);
    static bool is_multicast(const Packet& packet);
    static bool is_unicast(const Packet& packet);
};

} // namespace RouterSim
