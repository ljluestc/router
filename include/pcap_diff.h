#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

namespace RouterSim {

struct PacketInfo {
    uint64_t timestamp;
    uint32_t size;
    std::string source_ip;
    std::string dest_ip;
    uint16_t source_port;
    uint16_t dest_port;
    uint8_t protocol;
    std::vector<uint8_t> payload;
};

struct PcapDiffResult {
    bool identical;
    uint64_t packets_expected;
    uint64_t packets_actual;
    uint64_t bytes_expected;
    uint64_t bytes_actual;
    std::vector<std::string> differences;
    double similarity_score;
};

class PcapDiff {
public:
    PcapDiff();
    ~PcapDiff();

    // Load PCAP files
    bool load_expected(const std::string& filename);
    bool load_actual(const std::string& filename);
    
    // Compare packets
    PcapDiffResult compare();
    
    // Individual packet comparison
    bool compare_packets(const PacketInfo& expected, const PacketInfo& actual, 
                        std::string& difference);
    
    // Statistics
    struct ComparisonStats {
        uint64_t total_packets;
        uint64_t matching_packets;
        uint64_t size_differences;
        uint64_t timestamp_differences;
        uint64_t payload_differences;
        double average_size_diff;
        double average_timestamp_diff;
    };
    ComparisonStats get_comparison_stats() const;

private:
    bool parse_pcap_file(const std::string& filename, std::vector<PacketInfo>& packets);
    bool parse_ethernet_header(const uint8_t* data, size_t len, PacketInfo& packet);
    bool parse_ip_header(const uint8_t* data, size_t len, PacketInfo& packet);
    bool parse_tcp_header(const uint8_t* data, size_t len, PacketInfo& packet);
    bool parse_udp_header(const uint8_t* data, size_t len, PacketInfo& packet);
    
    std::vector<PacketInfo> expected_packets_;
    std::vector<PacketInfo> actual_packets_;
    ComparisonStats stats_;
};

} // namespace RouterSim
