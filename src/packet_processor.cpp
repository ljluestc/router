#include "packet_processor.h"
#include <iostream>
#include <cstring>

using namespace RouterSim;

PacketProcessor::PacketProcessor() : running_(false) {
}

PacketProcessor::~PacketProcessor() {
    cleanup();
}

bool PacketProcessor::initialize() {
    // Register default protocol handlers
    register_protocol_handler(ProtocolType::ETHERNET, 
                             [this](Packet& packet) { return handle_ethernet(packet); });
    register_protocol_handler(ProtocolType::IPV4, 
                             [this](Packet& packet) { return handle_ipv4(packet); });
    register_protocol_handler(ProtocolType::IPV6, 
                             [this](Packet& packet) { return handle_ipv6(packet); });
    register_protocol_handler(ProtocolType::ARP, 
                             [this](Packet& packet) { return handle_arp(packet); });
    register_protocol_handler(ProtocolType::ICMP, 
                             [this](Packet& packet) { return handle_icmp(packet); });
    register_protocol_handler(ProtocolType::TCP, 
                             [this](Packet& packet) { return handle_tcp(packet); });
    register_protocol_handler(ProtocolType::UDP, 
                             [this](Packet& packet) { return handle_udp(packet); });
    register_protocol_handler(ProtocolType::OSPF, 
                             [this](Packet& packet) { return handle_ospf(packet); });
    register_protocol_handler(ProtocolType::BGP, 
                             [this](Packet& packet) { return handle_bgp(packet); });
    register_protocol_handler(ProtocolType::ISIS, 
                             [this](Packet& packet) { return handle_isis(packet); });

    std::cout << "Packet processor initialized\n";
    return true;
}

void PacketProcessor::cleanup() {
    stop();
}

ProcessingResult PacketProcessor::process_packet(Packet& packet) {
    if (!validate_packet(packet)) {
        return ProcessingResult::DROP;
    }

    // Extract protocol type from packet
    ProtocolType protocol = extract_protocol(packet);
    
    auto it = protocol_handlers_.find(protocol);
    if (it != protocol_handlers_.end()) {
        return it->second(packet);
    }

    // Default: forward packet
    return ProcessingResult::FORWARD;
}

bool PacketProcessor::forward_packet(const Packet& packet, const std::string& next_hop) {
    // Implementation would forward packet to next hop
    std::cout << "Forwarding packet to " << next_hop << "\n";
    return true;
}

bool PacketProcessor::drop_packet(const Packet& packet, const std::string& reason) {
    std::cout << "Dropping packet: " << reason << "\n";
    return true;
}

bool PacketProcessor::register_protocol_handler(ProtocolType protocol, 
                                              std::function<ProcessingResult(Packet&)> handler) {
    std::lock_guard<std::mutex> lock(handlers_mutex_);
    protocol_handlers_[protocol] = handler;
    return true;
}

bool PacketProcessor::unregister_protocol_handler(ProtocolType protocol) {
    std::lock_guard<std::mutex> lock(handlers_mutex_);
    auto it = protocol_handlers_.find(protocol);
    if (it != protocol_handlers_.end()) {
        protocol_handlers_.erase(it);
        return true;
    }
    return false;
}

void PacketProcessor::set_routing_table(std::shared_ptr<RoutingTable> routing_table) {
    routing_table_ = routing_table;
}

std::string PacketProcessor::lookup_route(const std::string& destination) const {
    if (!routing_table_) {
        return "";
    }
    
    auto route = routing_table_->get_best_route(destination);
    return route.next_hop;
}

std::map<std::string, uint64_t> PacketProcessor::get_processing_stats() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    return processing_stats_;
}

std::map<ProtocolType, uint64_t> PacketProcessor::get_protocol_stats() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    return protocol_stats_;
}

void PacketProcessor::reset_statistics() {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    processing_stats_.clear();
    protocol_stats_.clear();
}

void PacketProcessor::start() {
    if (running_) {
        return;
    }
    
    running_ = true;
    std::cout << "Packet processor started\n";
}

void PacketProcessor::stop() {
    if (!running_) {
        return;
    }
    
    running_ = false;
    std::cout << "Packet processor stopped\n";
}

bool PacketProcessor::is_running() const {
    return running_;
}

// Protocol handlers
ProcessingResult PacketProcessor::handle_ethernet(Packet& packet) {
    if (!parse_ethernet_header(packet)) {
        return ProcessingResult::DROP;
    }
    
    processing_stats_["ethernet_packets"]++;
    return ProcessingResult::FORWARD;
}

ProcessingResult PacketProcessor::handle_ipv4(Packet& packet) {
    if (!parse_ipv4_header(packet)) {
        return ProcessingResult::DROP;
    }
    
    processing_stats_["ipv4_packets"]++;
    return ProcessingResult::FORWARD;
}

ProcessingResult PacketProcessor::handle_ipv6(Packet& packet) {
    if (!parse_ipv6_header(packet)) {
        return ProcessingResult::DROP;
    }
    
    processing_stats_["ipv6_packets"]++;
    return ProcessingResult::FORWARD;
}

ProcessingResult PacketProcessor::handle_arp(Packet& packet) {
    if (!parse_arp_header(packet)) {
        return ProcessingResult::DROP;
    }
    
    processing_stats_["arp_packets"]++;
    return ProcessingResult::CONSUME; // ARP is consumed locally
}

ProcessingResult PacketProcessor::handle_icmp(Packet& packet) {
    if (!parse_icmp_header(packet)) {
        return ProcessingResult::DROP;
    }
    
    processing_stats_["icmp_packets"]++;
    return ProcessingResult::CONSUME; // ICMP is consumed locally
}

ProcessingResult PacketProcessor::handle_tcp(Packet& packet) {
    if (!parse_tcp_header(packet)) {
        return ProcessingResult::DROP;
    }
    
    processing_stats_["tcp_packets"]++;
    return ProcessingResult::FORWARD;
}

ProcessingResult PacketProcessor::handle_udp(Packet& packet) {
    if (!parse_udp_header(packet)) {
        return ProcessingResult::DROP;
    }
    
    processing_stats_["udp_packets"]++;
    return ProcessingResult::FORWARD;
}

ProcessingResult PacketProcessor::handle_ospf(Packet& packet) {
    processing_stats_["ospf_packets"]++;
    return ProcessingResult::CONSUME; // OSPF is consumed by routing protocol
}

ProcessingResult PacketProcessor::handle_bgp(Packet& packet) {
    processing_stats_["bgp_packets"]++;
    return ProcessingResult::CONSUME; // BGP is consumed by routing protocol
}

ProcessingResult PacketProcessor::handle_isis(Packet& packet) {
    processing_stats_["isis_packets"]++;
    return ProcessingResult::CONSUME; // IS-IS is consumed by routing protocol
}

// Packet parsing methods
bool PacketProcessor::parse_ethernet_header(Packet& packet) {
    if (packet.data.size() < 14) { // Minimum Ethernet header size
        return false;
    }
    
    // Extract destination MAC (bytes 0-5)
    // Extract source MAC (bytes 6-11)
    // Extract EtherType (bytes 12-13)
    
    return true;
}

bool PacketProcessor::parse_ipv4_header(Packet& packet) {
    if (packet.data.size() < 20) { // Minimum IPv4 header size
        return false;
    }
    
    // Extract version (byte 0, bits 0-3)
    // Extract header length (byte 0, bits 4-7)
    // Extract TTL (byte 8)
    // Extract protocol (byte 9)
    // Extract source IP (bytes 12-15)
    // Extract destination IP (bytes 16-19)
    
    return true;
}

bool PacketProcessor::parse_ipv6_header(Packet& packet) {
    if (packet.data.size() < 40) { // IPv6 header size
        return false;
    }
    
    // Extract version (byte 0, bits 0-3)
    // Extract traffic class (byte 0, bits 4-7 and byte 1, bits 0-3)
    // Extract flow label (bytes 1-3, bits 4-31)
    // Extract payload length (bytes 4-5)
    // Extract next header (byte 6)
    // Extract hop limit (byte 7)
    // Extract source address (bytes 8-23)
    // Extract destination address (bytes 24-39)
    
    return true;
}

bool PacketProcessor::parse_arp_header(Packet& packet) {
    if (packet.data.size() < 28) { // ARP header size
        return false;
    }
    
    // Extract hardware type (bytes 0-1)
    // Extract protocol type (bytes 2-3)
    // Extract hardware address length (byte 4)
    // Extract protocol address length (byte 5)
    // Extract operation (bytes 6-7)
    // Extract sender hardware address (bytes 8-13)
    // Extract sender protocol address (bytes 14-17)
    // Extract target hardware address (bytes 18-23)
    // Extract target protocol address (bytes 24-27)
    
    return true;
}

bool PacketProcessor::parse_icmp_header(Packet& packet) {
    if (packet.data.size() < 8) { // Minimum ICMP header size
        return false;
    }
    
    // Extract type (byte 0)
    // Extract code (byte 1)
    // Extract checksum (bytes 2-3)
    // Extract identifier (bytes 4-5)
    // Extract sequence number (bytes 6-7)
    
    return true;
}

bool PacketProcessor::parse_tcp_header(Packet& packet) {
    if (packet.data.size() < 20) { // Minimum TCP header size
        return false;
    }
    
    // Extract source port (bytes 0-1)
    // Extract destination port (bytes 2-3)
    // Extract sequence number (bytes 4-7)
    // Extract acknowledgment number (bytes 8-11)
    // Extract header length (byte 12, bits 0-3)
    // Extract flags (byte 12, bits 4-7 and byte 13)
    // Extract window size (bytes 14-15)
    // Extract checksum (bytes 16-17)
    // Extract urgent pointer (bytes 18-19)
    
    return true;
}

bool PacketProcessor::parse_udp_header(Packet& packet) {
    if (packet.data.size() < 8) { // UDP header size
        return false;
    }
    
    // Extract source port (bytes 0-1)
    // Extract destination port (bytes 2-3)
    // Extract length (bytes 4-5)
    // Extract checksum (bytes 6-7)
    
    return true;
}

bool PacketProcessor::validate_packet(const Packet& packet) const {
    if (packet.data.empty()) {
        return false;
    }
    
    if (!validate_checksum(packet)) {
        return false;
    }
    
    if (!validate_length(packet)) {
        return false;
    }
    
    return true;
}

bool PacketProcessor::validate_checksum(const Packet& packet) const {
    // Simplified checksum validation
    // In real implementation, this would validate IP, TCP, UDP checksums
    return true;
}

bool PacketProcessor::validate_length(const Packet& packet) const {
    // Check if packet length matches declared length in headers
    return packet.data.size() == packet.size;
}

// PacketUtils implementation
Packet PacketUtils::create_ethernet_packet(const std::string& src_mac, const std::string& dst_mac,
                                         ProtocolType protocol, const std::vector<uint8_t>& payload) {
    Packet packet;
    
    // Create Ethernet header
    packet.data.resize(14 + payload.size());
    
    // Destination MAC (6 bytes)
    // Source MAC (6 bytes)
    // EtherType (2 bytes)
    uint16_t ethertype = static_cast<uint16_t>(protocol);
    packet.data[12] = (ethertype >> 8) & 0xFF;
    packet.data[13] = ethertype & 0xFF;
    
    // Copy payload
    std::copy(payload.begin(), payload.end(), packet.data.begin() + 14);
    
    packet.size = packet.data.size();
    packet.timestamp = std::chrono::steady_clock::now();
    
    return packet;
}

Packet PacketUtils::create_ipv4_packet(const std::string& src_ip, const std::string& dst_ip,
                                     uint8_t protocol, const std::vector<uint8_t>& payload) {
    Packet packet;
    
    // Create IPv4 header (20 bytes) + payload
    packet.data.resize(20 + payload.size());
    
    // Version (4) + Header Length (5) = 0x45
    packet.data[0] = 0x45;
    // Type of Service
    packet.data[1] = 0x00;
    // Total Length (will be set later)
    uint16_t total_length = 20 + payload.size();
    packet.data[2] = (total_length >> 8) & 0xFF;
    packet.data[3] = total_length & 0xFF;
    // Identification
    packet.data[4] = 0x00;
    packet.data[5] = 0x00;
    // Flags + Fragment Offset
    packet.data[6] = 0x00;
    packet.data[7] = 0x00;
    // TTL
    packet.data[8] = 64;
    // Protocol
    packet.data[9] = protocol;
    // Header Checksum (will be calculated later)
    packet.data[10] = 0x00;
    packet.data[11] = 0x00;
    
    // Source IP (simplified - would parse IP string)
    packet.data[12] = 192;
    packet.data[13] = 168;
    packet.data[14] = 1;
    packet.data[15] = 1;
    
    // Destination IP (simplified - would parse IP string)
    packet.data[16] = 192;
    packet.data[17] = 168;
    packet.data[18] = 1;
    packet.data[19] = 2;
    
    // Copy payload
    std::copy(payload.begin(), payload.end(), packet.data.begin() + 20);
    
    // Calculate checksum
    uint16_t checksum = calculate_ipv4_checksum(packet.data);
    packet.data[10] = (checksum >> 8) & 0xFF;
    packet.data[11] = checksum & 0xFF;
    
    packet.size = packet.data.size();
    packet.timestamp = std::chrono::steady_clock::now();
    
    return packet;
}

Packet PacketUtils::create_icmp_packet(const std::string& src_ip, const std::string& dst_ip,
                                     uint8_t type, uint8_t code, const std::vector<uint8_t>& payload) {
    Packet packet;
    
    // Create ICMP header (8 bytes) + payload
    packet.data.resize(8 + payload.size());
    
    // Type
    packet.data[0] = type;
    // Code
    packet.data[1] = code;
    // Checksum (will be calculated later)
    packet.data[2] = 0x00;
    packet.data[3] = 0x00;
    // Identifier
    packet.data[4] = 0x00;
    packet.data[5] = 0x00;
    // Sequence Number
    packet.data[6] = 0x00;
    packet.data[7] = 0x00;
    
    // Copy payload
    std::copy(payload.begin(), payload.end(), packet.data.begin() + 8);
    
    packet.size = packet.data.size();
    packet.timestamp = std::chrono::steady_clock::now();
    
    return packet;
}

std::string PacketUtils::extract_src_mac(const Packet& packet) {
    if (packet.data.size() < 14) {
        return "";
    }
    
    std::stringstream ss;
    for (int i = 6; i < 12; i++) {
        if (i > 6) ss << ":";
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(packet.data[i]);
    }
    return ss.str();
}

std::string PacketUtils::extract_dst_mac(const Packet& packet) {
    if (packet.data.size() < 14) {
        return "";
    }
    
    std::stringstream ss;
    for (int i = 0; i < 6; i++) {
        if (i > 0) ss << ":";
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(packet.data[i]);
    }
    return ss.str();
}

std::string PacketUtils::extract_src_ip(const Packet& packet) {
    if (packet.data.size() < 20) {
        return "";
    }
    
    std::stringstream ss;
    for (int i = 12; i < 16; i++) {
        if (i > 12) ss << ".";
        ss << static_cast<int>(packet.data[i]);
    }
    return ss.str();
}

std::string PacketUtils::extract_dst_ip(const Packet& packet) {
    if (packet.data.size() < 20) {
        return "";
    }
    
    std::stringstream ss;
    for (int i = 16; i < 20; i++) {
        if (i > 16) ss << ".";
        ss << static_cast<int>(packet.data[i]);
    }
    return ss.str();
}

uint16_t PacketUtils::extract_src_port(const Packet& packet) {
    if (packet.data.size() < 20) {
        return 0;
    }
    
    return (packet.data[20] << 8) | packet.data[21];
}

uint16_t PacketUtils::extract_dst_port(const Packet& packet) {
    if (packet.data.size() < 22) {
        return 0;
    }
    
    return (packet.data[22] << 8) | packet.data[23];
}

ProtocolType PacketUtils::extract_protocol(const Packet& packet) {
    if (packet.data.size() < 14) {
        return ProtocolType::ETHERNET;
    }
    
    uint16_t ethertype = (packet.data[12] << 8) | packet.data[13];
    return static_cast<ProtocolType>(ethertype);
}

uint16_t PacketUtils::calculate_ipv4_checksum(const std::vector<uint8_t>& data) {
    uint32_t sum = 0;
    
    // Sum all 16-bit words
    for (size_t i = 0; i < data.size(); i += 2) {
        if (i + 1 < data.size()) {
            sum += (data[i] << 8) | data[i + 1];
        } else {
            sum += data[i] << 8;
        }
    }
    
    // Add carry bits
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    
    // One's complement
    return ~sum;
}

uint16_t PacketUtils::calculate_tcp_checksum(const std::vector<uint8_t>& data, 
                                           const std::string& src_ip, const std::string& dst_ip) {
    // Simplified TCP checksum calculation
    return calculate_ipv4_checksum(data);
}

uint16_t PacketUtils::calculate_udp_checksum(const std::vector<uint8_t>& data,
                                           const std::string& src_ip, const std::string& dst_ip) {
    // Simplified UDP checksum calculation
    return calculate_ipv4_checksum(data);
}

bool PacketUtils::set_ttl(Packet& packet, uint8_t ttl) {
    if (packet.data.size() < 9) {
        return false;
    }
    
    packet.data[8] = ttl;
    return true;
}

bool PacketUtils::set_tos(Packet& packet, uint8_t tos) {
    if (packet.data.size() < 2) {
        return false;
    }
    
    packet.data[1] = tos;
    return true;
}

bool PacketUtils::set_dscp(Packet& packet, uint8_t dscp) {
    if (packet.data.size() < 2) {
        return false;
    }
    
    packet.data[1] = (packet.data[1] & 0x03) | (dscp << 2);
    return true;
}

bool PacketUtils::set_priority(Packet& packet, uint8_t priority) {
    packet.priority = priority;
    return true;
}

bool PacketUtils::matches_filter(const Packet& packet, const std::string& filter) {
    // Simplified filter matching
    return true;
}

bool PacketUtils::is_broadcast(const Packet& packet) {
    if (packet.data.size() < 6) {
        return false;
    }
    
    // Check if destination MAC is broadcast (FF:FF:FF:FF:FF:FF)
    for (int i = 0; i < 6; i++) {
        if (packet.data[i] != 0xFF) {
            return false;
        }
    }
    
    return true;
}

bool PacketUtils::is_multicast(const Packet& packet) {
    if (packet.data.size() < 1) {
        return false;
    }
    
    // Check if destination MAC is multicast (first bit is 1)
    return (packet.data[0] & 0x01) != 0;
}

bool PacketUtils::is_unicast(const Packet& packet) {
    return !is_broadcast(packet) && !is_multicast(packet);
}
