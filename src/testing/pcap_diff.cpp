#include "testing/pcap_diff.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <chrono>

namespace router_sim {

PcapReader::PcapReader() : file_(nullptr), is_open_(false), packet_count_(0) {
}

PcapReader::~PcapReader() {
    close();
}

bool PcapReader::open(const std::string& filename) {
    close();
    
    file_ = fopen(filename.c_str(), "rb");
    if (!file_) {
        return false;
    }
    
    // Read PCAP header
    uint8_t header[24];
    if (fread(header, 1, 24, file_) != 24) {
        fclose(file_);
        file_ = nullptr;
        return false;
    }
    
    filename_ = filename;
    is_open_ = true;
    packet_count_ = 0;
    
    return true;
}

void PcapReader::close() {
    if (file_) {
        fclose(file_);
        file_ = nullptr;
    }
    is_open_ = false;
}

bool PcapReader::is_open() const {
    return is_open_;
}

bool PcapReader::read_packet(PacketInfo& packet) {
    if (!is_open_) {
        return false;
    }
    
    // Read packet header
    uint8_t header[16];
    if (fread(header, 1, 16, file_) != 16) {
        return false;
    }
    
    uint32_t packet_length;
    std::chrono::microseconds timestamp;
    if (!parse_packet_header(header, packet_length, timestamp)) {
        return false;
    }
    
    // Read packet data
    std::vector<uint8_t> data(packet_length);
    if (fread(data.data(), 1, packet_length, file_) != packet_length) {
        return false;
    }
    
    packet.timestamp = timestamp;
    packet.length = packet_length;
    packet.data = std::move(data);
    
    // Parse packet headers
    uint32_t offset = 0;
    if (!parse_ethernet_header(packet.data.data(), offset, packet.source_ip, 
                              packet.dest_ip, packet.protocol)) {
        return false;
    }
    
    packet_count_++;
    return true;
}

std::vector<PacketInfo> PcapReader::read_all_packets() {
    std::vector<PacketInfo> packets;
    PacketInfo packet;
    
    while (read_packet(packet)) {
        packets.push_back(packet);
    }
    
    return packets;
}

uint64_t PcapReader::get_packet_count() const {
    return packet_count_;
}

std::chrono::microseconds PcapReader::get_duration() const {
    return end_time_ - start_time_;
}

bool PcapReader::set_filter(const std::string& filter_expression) {
    filter_expression_ = filter_expression;
    has_filters_ = true;
    return true;
}

bool PcapReader::set_time_range(std::chrono::microseconds start, std::chrono::microseconds end) {
    filter_start_ = start;
    filter_end_ = end;
    has_filters_ = true;
    return true;
}

bool PcapReader::set_protocol_filter(uint8_t protocol) {
    filter_protocol_ = protocol;
    has_filters_ = true;
    return true;
}

bool PcapReader::set_ip_filter(const std::string& source_ip, const std::string& dest_ip) {
    filter_source_ip_ = source_ip;
    filter_dest_ip_ = dest_ip;
    has_filters_ = true;
    return true;
}

bool PcapReader::parse_packet_header(const uint8_t* data, uint32_t& packet_length, 
                                    std::chrono::microseconds& timestamp) {
    // Parse PCAP packet header
    uint32_t ts_sec = *reinterpret_cast<const uint32_t*>(data);
    uint32_t ts_usec = *reinterpret_cast<const uint32_t*>(data + 4);
    uint32_t caplen = *reinterpret_cast<const uint32_t*>(data + 8);
    uint32_t len = *reinterpret_cast<const uint32_t*>(data + 12);
    
    packet_length = caplen;
    timestamp = std::chrono::microseconds(ts_sec * 1000000ULL + ts_usec);
    
    return true;
}

bool PcapReader::parse_ethernet_header(const uint8_t* data, uint32_t& offset, 
                                      std::string& source_ip, std::string& dest_ip, 
                                      uint8_t& protocol) {
    // Skip Ethernet header (14 bytes)
    offset += 14;
    
    // Parse IP header
    return parse_ip_header(data, offset, source_ip, dest_ip, protocol);
}

bool PcapReader::parse_ip_header(const uint8_t* data, uint32_t& offset, 
                                std::string& source_ip, std::string& dest_ip, 
                                uint8_t& protocol) {
    // Parse IP header
    uint8_t version = (data[offset] >> 4) & 0x0F;
    if (version != 4) {
        return false; // Only IPv4 supported
    }
    
    uint8_t ihl = data[offset] & 0x0F;
    protocol = data[offset + 9];
    
    // Extract IP addresses
    source_ip = std::to_string(data[offset + 12]) + "." +
                std::to_string(data[offset + 13]) + "." +
                std::to_string(data[offset + 14]) + "." +
                std::to_string(data[offset + 15]);
    
    dest_ip = std::to_string(data[offset + 16]) + "." +
              std::to_string(data[offset + 17]) + "." +
              std::to_string(data[offset + 18]) + "." +
              std::to_string(data[offset + 19]);
    
    offset += ihl * 4;
    
    // Parse transport layer
    if (protocol == 6) { // TCP
        return parse_tcp_header(data, offset, source_ip, dest_ip);
    } else if (protocol == 17) { // UDP
        return parse_udp_header(data, offset, source_ip, dest_ip);
    }
    
    return true;
}

bool PcapReader::parse_tcp_header(const uint8_t* data, uint32_t& offset, 
                                 uint16_t& source_port, uint16_t& dest_port) {
    source_port = (data[offset] << 8) | data[offset + 1];
    dest_port = (data[offset + 2] << 8) | data[offset + 3];
    offset += 20; // TCP header length
    return true;
}

bool PcapReader::parse_udp_header(const uint8_t* data, uint32_t& offset, 
                                 uint16_t& source_port, uint16_t& dest_port) {
    source_port = (data[offset] << 8) | data[offset + 1];
    dest_port = (data[offset + 2] << 8) | data[offset + 3];
    offset += 8; // UDP header length
    return true;
}

// PcapDiff implementation
PcapDiff::PcapDiff() {
    config_.time_tolerance_ms = 10;
    config_.ignore_timestamps = false;
    config_.ignore_checksums = false;
    config_.ignore_ttl = false;
    config_.ignore_tos = false;
    config_.ignore_id = false;
    config_.ignore_fragments = false;
    config_.byte_tolerance = 0;
    config_.packet_order_tolerance = 0;
    config_.time_synchronization = false;
    config_.correlation_window_ms = 1000;
}

PcapDiff::~PcapDiff() {
}

PcapDiffResult PcapDiff::compare_files(const std::string& expected_file, 
                                      const std::string& actual_file) {
    PcapReader expected_reader, actual_reader;
    
    if (!expected_reader.open(expected_file)) {
        PcapDiffResult result;
        result.identical = false;
        result.differences.push_back("Failed to open expected file: " + expected_file);
        return result;
    }
    
    if (!actual_reader.open(actual_file)) {
        PcapDiffResult result;
        result.identical = false;
        result.differences.push_back("Failed to open actual file: " + actual_file);
        return result;
    }
    
    std::vector<PacketInfo> expected_packets = expected_reader.read_all_packets();
    std::vector<PacketInfo> actual_packets = actual_reader.read_all_packets();
    
    return compare_packets(expected_packets, actual_packets);
}

PcapDiffResult PcapDiff::compare_packets(const std::vector<PacketInfo>& expected, 
                                        const std::vector<PacketInfo>& actual) {
    PcapDiffResult result;
    result.total_packets_expected = expected.size();
    result.total_packets_actual = actual.size();
    result.matching_packets = 0;
    result.missing_packets = 0;
    result.extra_packets = 0;
    result.different_packets = 0;
    
    // Simple comparison for now
    size_t min_size = std::min(expected.size(), actual.size());
    
    for (size_t i = 0; i < min_size; ++i) {
        if (compare_packet_data(expected[i], actual[i], result.differences)) {
            result.matching_packets++;
        } else {
            result.different_packets++;
        }
    }
    
    if (expected.size() > actual.size()) {
        result.missing_packets = expected.size() - actual.size();
    } else if (actual.size() > expected.size()) {
        result.extra_packets = actual.size() - expected.size();
    }
    
    result.identical = (result.missing_packets == 0 && result.extra_packets == 0 && 
                      result.different_packets == 0);
    
    result.similarity_percentage = (double)result.matching_packets / 
                                  std::max(expected.size(), actual.size()) * 100.0;
    
    return result;
}

void PcapDiff::set_tolerance_ms(uint32_t tolerance_ms) {
    config_.time_tolerance_ms = tolerance_ms;
}

void PcapDiff::set_ignore_timestamps(bool ignore) {
    config_.ignore_timestamps = ignore;
}

void PcapDiff::set_ignore_checksums(bool ignore) {
    config_.ignore_checksums = ignore;
}

void PcapDiff::set_ignore_ttl(bool ignore) {
    config_.ignore_ttl = ignore;
}

void PcapDiff::set_ignore_tos(bool ignore) {
    config_.ignore_tos = ignore;
}

void PcapDiff::set_ignore_id(bool ignore) {
    config_.ignore_id = ignore;
}

void PcapDiff::set_ignore_fragments(bool ignore) {
    config_.ignore_fragments = ignore;
}

void PcapDiff::set_byte_tolerance(uint32_t tolerance_bytes) {
    config_.byte_tolerance = tolerance_bytes;
}

void PcapDiff::set_packet_order_tolerance(uint32_t tolerance_packets) {
    config_.packet_order_tolerance = tolerance_packets;
}

void PcapDiff::set_time_synchronization(bool enable) {
    config_.time_synchronization = enable;
}

void PcapDiff::set_correlation_window_ms(uint32_t window_ms) {
    config_.correlation_window_ms = window_ms;
}

bool PcapDiff::compare_packet_data(const PacketInfo& expected, const PacketInfo& actual, 
                                  std::string& difference) {
    // Compare timestamps
    if (!config_.ignore_timestamps) {
        if (!compare_timestamps(expected, actual, difference)) {
            return false;
        }
    }
    
    // Compare headers
    if (!compare_headers(expected, actual, difference)) {
        return false;
    }
    
    // Compare payloads
    if (!compare_payloads(expected, actual, difference)) {
        return false;
    }
    
    return true;
}

bool PcapDiff::compare_timestamps(const PacketInfo& expected, const PacketInfo& actual, 
                                 std::string& difference) {
    auto time_diff = std::abs((expected.timestamp - actual.timestamp).count());
    if (time_diff > config_.time_tolerance_ms * 1000) {
        difference = "Timestamp mismatch: expected " + 
                    std::to_string(expected.timestamp.count()) + 
                    ", actual " + std::to_string(actual.timestamp.count());
        return false;
    }
    return true;
}

bool PcapDiff::compare_headers(const PacketInfo& expected, const PacketInfo& actual, 
                              std::string& difference) {
    // TODO: Implement header comparison
    return true;
}

bool PcapDiff::compare_payloads(const PacketInfo& expected, const PacketInfo& actual, 
                               std::string& difference) {
    if (expected.data.size() != actual.data.size()) {
        difference = "Payload size mismatch: expected " + 
                    std::to_string(expected.data.size()) + 
                    ", actual " + std::to_string(actual.data.size());
        return false;
    }
    
    for (size_t i = 0; i < expected.data.size(); ++i) {
        if (expected.data[i] != actual.data[i]) {
            difference = "Payload byte mismatch at offset " + std::to_string(i) + 
                        ": expected " + std::to_string(expected.data[i]) + 
                        ", actual " + std::to_string(actual.data[i]);
            return false;
        }
    }
    
    return true;
}

std::vector<PacketInfo> PcapDiff::synchronize_packets(const std::vector<PacketInfo>& expected, 
                                                     const std::vector<PacketInfo>& actual) {
    // TODO: Implement packet synchronization
    return actual;
}

std::vector<PacketInfo> PcapDiff::correlate_packets(const std::vector<PacketInfo>& expected, 
                                                   const std::vector<PacketInfo>& actual) {
    // TODO: Implement packet correlation
    return actual;
}

// PacketAnalyzer implementation
std::string PacketAnalyzer::get_protocol_name(uint8_t protocol) {
    switch (protocol) {
        case 1: return "ICMP";
        case 6: return "TCP";
        case 17: return "UDP";
        case 47: return "GRE";
        case 50: return "ESP";
        case 51: return "AH";
        case 89: return "OSPF";
        case 132: return "SCTP";
        default: return "Unknown";
    }
}

std::string PacketAnalyzer::format_timestamp(std::chrono::microseconds timestamp) {
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(timestamp);
    auto microseconds = timestamp - seconds;
    
    std::time_t time_t = seconds.count();
    std::tm* tm = std::localtime(&time_t);
    
    char buffer[64];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm);
    
    return std::string(buffer) + "." + std::to_string(microseconds.count());
}

std::string PacketAnalyzer::format_packet_summary(const PacketInfo& packet) {
    return format_timestamp(packet.timestamp) + " " +
           packet.source_ip + ":" + std::to_string(packet.source_port) + " -> " +
           packet.dest_ip + ":" + std::to_string(packet.dest_port) + " " +
           get_protocol_name(packet.protocol) + " (" + 
           std::to_string(packet.length) + " bytes)";
}

std::map<std::string, uint64_t> PacketAnalyzer::get_protocol_statistics(const std::vector<PacketInfo>& packets) {
    std::map<std::string, uint64_t> stats;
    
    for (const auto& packet : packets) {
        std::string protocol = get_protocol_name(packet.protocol);
        stats[protocol]++;
    }
    
    return stats;
}

std::map<std::string, uint64_t> PacketAnalyzer::get_flow_statistics(const std::vector<PacketInfo>& packets) {
    std::map<std::string, uint64_t> stats;
    
    for (const auto& packet : packets) {
        std::string flow = packet.source_ip + ":" + std::to_string(packet.source_port) + 
                          "->" + packet.dest_ip + ":" + std::to_string(packet.dest_port);
        stats[flow]++;
    }
    
    return stats;
}

std::vector<PacketInfo> PacketAnalyzer::filter_packets(const std::vector<PacketInfo>& packets, 
                                                      const std::string& filter_expression) {
    // TODO: Implement packet filtering
    return packets;
}

} // namespace router_sim
