#pragma once

#include <string>
#include <vector>
#include <map>
#include <cstdint>

namespace router_sim {
namespace testing {

struct PCAPDiffOptions {
    bool ignore_basic_stats{false};
    bool stop_on_first_difference{false};
    uint64_t max_packets{0}; // 0 means no limit
    double timestamp_tolerance_seconds{1.0};
    bool verbose{false};
};

struct PCAPFileInfo {
    // Basic statistics
    uint64_t packet_count{0};
    uint64_t total_bytes{0};
    uint64_t avg_packet_size{0};
    uint64_t min_packet_size{0};
    uint64_t max_packet_size{0};
    
    // File header information
    uint32_t magic_number{0};
    uint16_t version_major{0};
    uint16_t version_minor{0};
    int32_t thiszone{0};
    uint32_t sigfigs{0};
    uint32_t snaplen{0};
    uint32_t network{0};
    
    // Packet statistics
    uint64_t packets_received{0};
    uint64_t packets_dropped{0};
    uint64_t packets_if_dropped{0};
    
    // Protocol distribution
    uint64_t ipv4_packets{0};
    uint64_t ipv6_packets{0};
    uint64_t arp_packets{0};
    uint64_t other_packets{0};
    
    // IP protocol distribution
    uint64_t tcp_packets{0};
    uint64_t udp_packets{0};
    uint64_t icmp_packets{0};
    uint64_t icmpv6_packets{0};
    uint64_t other_ip_packets{0};
    
    // Application protocol distribution
    uint64_t http_packets{0};
    uint64_t https_packets{0};
    uint64_t dns_packets{0};
    uint64_t ssh_packets{0};
    uint64_t telnet_packets{0};
    uint64_t ftp_packets{0};
    uint64_t smtp_packets{0};
    uint64_t pop3_packets{0};
    uint64_t imap_packets{0};
    uint64_t imaps_packets{0};
    uint64_t pop3s_packets{0};
    uint64_t dhcp_packets{0};
    uint64_t ntp_packets{0};
    uint64_t snmp_packets{0};
    uint64_t snmp_trap_packets{0};
    uint64_t syslog_packets{0};
    uint64_t tftp_packets{0};
    uint64_t other_tcp_packets{0};
    uint64_t other_udp_packets{0};
    
    // Size distribution
    uint64_t size_64_or_less{0};
    uint64_t size_65_to_128{0};
    uint64_t size_129_to_256{0};
    uint64_t size_257_to_512{0};
    uint64_t size_513_to_1024{0};
    uint64_t size_1025_or_more{0};
    
    // TTL/Hop limit distribution
    uint64_t ttl_32_or_less{0};
    uint64_t ttl_33_to_64{0};
    uint64_t ttl_65_to_128{0};
    uint64_t ttl_129_or_more{0};
    uint64_t hop_limit_32_or_less{0};
    uint64_t hop_limit_33_to_64{0};
    uint64_t hop_limit_65_to_128{0};
    uint64_t hop_limit_129_or_more{0};
    
    // DSCP distribution
    uint64_t dscp_0_to_7{0};
    uint64_t dscp_8_to_15{0};
    uint64_t dscp_16_to_23{0};
    uint64_t dscp_24_to_31{0};
    uint64_t dscp_32_to_39{0};
    uint64_t dscp_40_to_47{0};
    uint64_t dscp_48_to_55{0};
    uint64_t dscp_56_to_63{0};
    
    // TCP flags distribution
    uint64_t tcp_syn_packets{0};
    uint64_t tcp_ack_packets{0};
    uint64_t tcp_rst_packets{0};
    uint64_t tcp_fin_packets{0};
    uint64_t tcp_psh_packets{0};
    uint64_t tcp_urg_packets{0};
};

class PCAPDiff {
public:
    PCAPDiff();
    ~PCAPDiff();
    
    // Main comparison function
    bool compare_files(const std::string& file1, const std::string& file2, 
                      const PCAPDiffOptions& options = PCAPDiffOptions{});
    
    // File analysis
    PCAPFileInfo get_file_info(const std::string& filename);
    void print_file_info(const PCAPFileInfo& info, const std::string& filename);
    
    // Detailed comparison
    bool compare_packets_detailed(const std::string& file1, const std::string& file2, 
                                 const PCAPDiffOptions& options = PCAPDiffOptions{});
    
    // Generate difference report
    std::string generate_diff_report(const std::string& file1, const std::string& file2, 
                                   const PCAPDiffOptions& options = PCAPDiffOptions{});
    
private:
    // Internal analysis methods
    PCAPFileInfo get_file_info(pcap_t* pcap);
    void analyze_packet(const u_char* packet, const pcap_pkthdr* header, PCAPFileInfo& info);
    void analyze_ipv4_packet(const u_char* packet, uint32_t length, PCAPFileInfo& info);
    void analyze_ipv6_packet(const u_char* packet, uint32_t length, PCAPFileInfo& info);
    void analyze_tcp_packet(const u_char* packet, uint32_t length, PCAPFileInfo& info);
    void analyze_udp_packet(const u_char* packet, uint32_t length, PCAPFileInfo& info);
    
    // Comparison methods
    bool compare_basic_stats(const PCAPFileInfo& info1, const PCAPFileInfo& info2, 
                           const PCAPDiffOptions& options);
    bool compare_packets(pcap_t* pcap1, pcap_t* pcap2, const PCAPDiffOptions& options);
    bool compare_packet_headers(const pcap_pkthdr* header1, const pcap_pkthdr* header2, 
                               uint64_t packet_number);
    bool compare_packet_data(const u_char* packet1, uint32_t len1, 
                           const u_char* packet2, uint32_t len2, 
                           uint64_t packet_number);
};

} // namespace testing
} // namespace router_sim
