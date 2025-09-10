#pragma once

#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <cstdint>
#include <cstdio>

namespace RouterSim {

// PCAP comparison structures
struct PacketInfo {
    uint64_t packet_number;
    std::chrono::system_clock::time_point timestamp;
    std::string src_ip;
    std::string dst_ip;
    uint8_t protocol;
    uint16_t src_port;
    uint16_t dst_port;
    uint32_t size;
    std::vector<uint8_t> data;
    uint8_t dscp;
    uint8_t ttl;
};

struct PcapStatistics {
    uint64_t total_packets;
    uint64_t total_bytes;
    std::map<uint8_t, uint64_t> protocol_counts;
    std::chrono::system_clock::time_point start_time;
    std::chrono::system_clock::time_point end_time;
};

struct PcapData {
    std::vector<PacketInfo> packets;
    PcapStatistics stats;
};

struct PcapDifference {
    std::string type;
    size_t packet_index = SIZE_MAX;
    std::string description;
};

struct PcapDiffOptions {
    bool compare_payload = false;
    bool compare_protocols = true;
    uint64_t timestamp_tolerance_us = 1000; // 1ms tolerance
    std::vector<uint8_t> ignore_protocols;
};

// PCAP capture structures
struct CaptureOptions {
    uint32_t packet_count = 0; // 0 = unlimited
    uint32_t timeout_seconds = 0; // 0 = no timeout
    std::string filter; // BPF filter
    bool promiscuous = true;
};

class PcapDiff {
public:
    PcapDiff();
    ~PcapDiff();

    // Core functionality
    bool initialize();
    bool compare_pcaps(const std::string& pcap1_path, const std::string& pcap2_path, 
                      const PcapDiffOptions& options = PcapDiffOptions{});
    bool compare_pcap_data(const PcapData& pcap1, const PcapData& pcap2, 
                          const PcapDiffOptions& options = PcapDiffOptions{});

    // Results
    std::vector<PcapDifference> get_differences() const;
    void print_differences() const;
    bool save_differences(const std::string& output_file) const;

private:
    bool initialized_;
    std::vector<PcapDifference> differences_;

    // Internal methods
    void compare_packet(const PacketInfo& packet1, const PacketInfo& packet2, 
                       size_t packet_index, const PcapDiffOptions& options);
    void compare_protocol_fields(const PacketInfo& packet1, const PacketInfo& packet2, 
                                size_t packet_index);
    void compare_statistics(const PcapStatistics& stats1, const PcapStatistics& stats2);
    bool read_pcap_file(const std::string& file_path, PcapData& pcap_data);
    void calculate_statistics(PcapData& pcap_data);
    std::vector<uint8_t> hex_string_to_bytes(const std::string& hex_string);
};

class PcapCapture {
public:
    PcapCapture();
    ~PcapCapture();

    // Core functionality
    bool initialize();
    bool start_capture(const std::string& interface, const std::string& output_file, 
                      const CaptureOptions& options = CaptureOptions{});
    bool stop_capture();
    bool is_capturing() const;

    // Information
    std::string get_output_file() const;

private:
    bool initialized_;
    bool capturing_;
    std::string output_file_;
    FILE* capture_process_;
};

} // namespace RouterSim
