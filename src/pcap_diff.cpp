#include "pcap_diff.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cmath>

namespace RouterSim {

PcapDiff::PcapDiff() {
    stats_ = {};
}

PcapDiff::~PcapDiff() = default;

bool PcapDiff::load_expected(const std::string& filename) {
    return parse_pcap_file(filename, expected_packets_);
}

bool PcapDiff::load_actual(const std::string& filename) {
    return parse_pcap_file(filename, actual_packets_);
}

PcapDiffResult PcapDiff::compare() {
    PcapDiffResult result;
    result.identical = true;
    result.packets_expected = expected_packets_.size();
    result.packets_actual = actual_packets_.size();
    result.bytes_expected = 0;
    result.bytes_actual = 0;
    result.similarity_score = 0.0;
    
    // Calculate total bytes
    for (const auto& packet : expected_packets_) {
        result.bytes_expected += packet.size;
    }
    for (const auto& packet : actual_packets_) {
        result.bytes_actual += packet.size;
    }
    
    // Compare packet counts
    if (result.packets_expected != result.packets_actual) {
        result.identical = false;
        result.differences.push_back("Packet count mismatch: expected " + 
                                   std::to_string(result.packets_expected) + 
                                   ", got " + std::to_string(result.packets_actual));
    }
    
    // Compare individual packets
    size_t min_packets = std::min(expected_packets_.size(), actual_packets_.size());
    uint64_t matching_packets = 0;
    
    for (size_t i = 0; i < min_packets; ++i) {
        std::string difference;
        if (compare_packets(expected_packets_[i], actual_packets_[i], difference)) {
            matching_packets++;
        } else {
            result.identical = false;
            result.differences.push_back("Packet " + std::to_string(i) + ": " + difference);
        }
    }
    
    // Calculate similarity score
    if (result.packets_expected > 0) {
        result.similarity_score = (double)matching_packets / result.packets_expected;
    }
    
    // Update statistics
    stats_.total_packets = result.packets_expected;
    stats_.matching_packets = matching_packets;
    
    return result;
}

bool PcapDiff::compare_packets(const PacketInfo& expected, const PacketInfo& actual, 
                              std::string& difference) {
    // Compare size
    if (expected.size != actual.size) {
        difference = "Size mismatch: expected " + std::to_string(expected.size) + 
                    ", got " + std::to_string(actual.size);
        stats_.size_differences++;
        return false;
    }
    
    // Compare timestamp (with tolerance)
    uint64_t time_diff = std::abs((int64_t)expected.timestamp - (int64_t)actual.timestamp);
    if (time_diff > 1000) { // 1ms tolerance
        difference = "Timestamp mismatch: expected " + std::to_string(expected.timestamp) + 
                    ", got " + std::to_string(actual.timestamp);
        stats_.timestamp_differences++;
        return false;
    }
    
    // Compare IP addresses
    if (expected.source_ip != actual.source_ip) {
        difference = "Source IP mismatch: expected " + expected.source_ip + 
                    ", got " + actual.source_ip;
        return false;
    }
    
    if (expected.dest_ip != actual.dest_ip) {
        difference = "Dest IP mismatch: expected " + expected.dest_ip + 
                    ", got " + actual.dest_ip;
        return false;
    }
    
    // Compare ports
    if (expected.source_port != actual.source_port) {
        difference = "Source port mismatch: expected " + std::to_string(expected.source_port) + 
                    ", got " + std::to_string(actual.source_port);
        return false;
    }
    
    if (expected.dest_port != actual.dest_port) {
        difference = "Dest port mismatch: expected " + std::to_string(expected.dest_port) + 
                    ", got " + std::to_string(actual.dest_port);
        return false;
    }
    
    // Compare protocol
    if (expected.protocol != actual.protocol) {
        difference = "Protocol mismatch: expected " + std::to_string(expected.protocol) + 
                    ", got " + std::to_string(actual.protocol);
        return false;
    }
    
    // Compare payload
    if (expected.payload != actual.payload) {
        difference = "Payload mismatch";
        stats_.payload_differences++;
        return false;
    }
    
    return true;
}

PcapDiff::ComparisonStats PcapDiff::get_comparison_stats() const {
    return stats_;
}

bool PcapDiff::parse_pcap_file(const std::string& filename, std::vector<PacketInfo>& packets) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return false;
    }
    
    // Read PCAP global header (24 bytes)
    char global_header[24];
    file.read(global_header, 24);
    if (file.gcount() != 24) {
        std::cerr << "Failed to read PCAP global header" << std::endl;
        return false;
    }
    
    // Read packet records
    while (file.good()) {
        // Read packet record header (16 bytes)
        char record_header[16];
        file.read(record_header, 16);
        if (file.gcount() != 16) {
            break; // End of file
        }
        
        // Extract packet length
        uint32_t packet_len = *(uint32_t*)(record_header + 8);
        uint32_t orig_len = *(uint32_t*)(record_header + 12);
        
        // Read packet data
        std::vector<uint8_t> packet_data(packet_len);
        file.read(reinterpret_cast<char*>(packet_data.data()), packet_len);
        if (file.gcount() != packet_len) {
            std::cerr << "Failed to read packet data" << std::endl;
            break;
        }
        
        // Parse packet
        PacketInfo packet;
        packet.size = packet_len;
        packet.timestamp = *(uint32_t*)(record_header + 0) * 1000000ULL + 
                          *(uint32_t*)(record_header + 4);
        
        if (parse_ethernet_header(packet_data.data(), packet_data.size(), packet)) {
            packets.push_back(packet);
        }
    }
    
    return true;
}

bool PcapDiff::parse_ethernet_header(const uint8_t* data, size_t len, PacketInfo& packet) {
    if (len < 14) {
        return false;
    }
    
    // Skip Ethernet header (14 bytes)
    return parse_ip_header(data + 14, len - 14, packet);
}

bool PcapDiff::parse_ip_header(const uint8_t* data, size_t len, PacketInfo& packet) {
    if (len < 20) {
        return false;
    }
    
    // Extract IP version and header length
    uint8_t version = (data[0] >> 4) & 0x0F;
    if (version != 4) {
        return false; // Only support IPv4
    }
    
    uint8_t header_len = (data[0] & 0x0F) * 4;
    if (header_len < 20 || len < header_len) {
        return false;
    }
    
    // Extract protocol
    packet.protocol = data[9];
    
    // Extract source and destination IPs
    std::ostringstream src_ip, dst_ip;
    src_ip << (int)data[12] << "." << (int)data[13] << "." 
           << (int)data[14] << "." << (int)data[15];
    dst_ip << (int)data[16] << "." << (int)data[17] << "." 
           << (int)data[18] << "." << (int)data[19];
    
    packet.source_ip = src_ip.str();
    packet.dest_ip = dst_ip.str();
    
    // Parse transport layer
    if (packet.protocol == 6) { // TCP
        return parse_tcp_header(data + header_len, len - header_len, packet);
    } else if (packet.protocol == 17) { // UDP
        return parse_udp_header(data + header_len, len - header_len, packet);
    }
    
    return true; // Other protocols
}

bool PcapDiff::parse_tcp_header(const uint8_t* data, size_t len, PacketInfo& packet) {
    if (len < 4) {
        return false;
    }
    
    packet.source_port = (data[0] << 8) | data[1];
    packet.dest_port = (data[2] << 8) | data[3];
    
    // Extract payload
    uint8_t header_len = ((data[12] >> 4) & 0x0F) * 4;
    if (len > header_len) {
        packet.payload.assign(data + header_len, data + len);
    }
    
    return true;
}

bool PcapDiff::parse_udp_header(const uint8_t* data, size_t len, PacketInfo& packet) {
    if (len < 8) {
        return false;
    }
    
    packet.source_port = (data[0] << 8) | data[1];
    packet.dest_port = (data[2] << 8) | data[3];
    
    // Extract payload (skip 8-byte UDP header)
    if (len > 8) {
        packet.payload.assign(data + 8, data + len);
    }
    
    return true;
}

} // namespace RouterSim
