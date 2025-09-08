#include "pcap_diff.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <iomanip>
#include <sstream>

namespace RouterSim {

PCAPDiff::PCAPDiff() {
}

PCAPDiff::~PCAPDiff() {
}

bool PCAPDiff::compare_files(const std::string& file1, const std::string& file2) {
    std::ifstream f1(file1, std::ios::binary);
    std::ifstream f2(file2, std::ios::binary);
    
    if (!f1.is_open() || !f2.is_open()) {
        std::cerr << "Failed to open one or both files" << std::endl;
        return false;
    }
    
    // Compare file sizes
    f1.seekg(0, std::ios::end);
    f2.seekg(0, std::ios::end);
    size_t size1 = f1.tellg();
    size_t size2 = f2.tellg();
    
    if (size1 != size2) {
        std::cout << "Files have different sizes: " << size1 << " vs " << size2 << std::endl;
        return false;
    }
    
    // Compare content byte by byte
    f1.seekg(0, std::ios::beg);
    f2.seekg(0, std::ios::beg);
    
    char byte1, byte2;
    size_t position = 0;
    size_t differences = 0;
    
    while (f1.get(byte1) && f2.get(byte2)) {
        if (byte1 != byte2) {
            differences++;
            if (differences <= 10) { // Show first 10 differences
                std::cout << "Difference at position " << position 
                         << ": 0x" << std::hex << std::setw(2) << std::setfill('0') 
                         << static_cast<int>(static_cast<unsigned char>(byte1))
                         << " vs 0x" << std::setw(2) << std::setfill('0')
                         << static_cast<int>(static_cast<unsigned char>(byte2))
                         << std::dec << std::endl;
            }
        }
        position++;
    }
    
    if (differences > 10) {
        std::cout << "... and " << (differences - 10) << " more differences" << std::endl;
    }
    
    std::cout << "Total differences: " << differences << std::endl;
    return differences == 0;
}

bool PCAPDiff::compare_pcap_files(const std::string& file1, const std::string& file2) {
    PCAPFile pcap1, pcap2;
    
    if (!load_pcap_file(file1, pcap1)) {
        std::cerr << "Failed to load PCAP file: " << file1 << std::endl;
        return false;
    }
    
    if (!load_pcap_file(file2, pcap2)) {
        std::cerr << "Failed to load PCAP file: " << file2 << std::endl;
        return false;
    }
    
    return compare_pcap_structures(pcap1, pcap2);
}

bool PCAPDiff::load_pcap_file(const std::string& filename, PCAPFile& pcap) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    // Read PCAP global header
    file.read(reinterpret_cast<char*>(&pcap.global_header), sizeof(pcap.global_header));
    if (file.gcount() != sizeof(pcap.global_header)) {
        return false;
    }
    
    // Read packet records
    while (file.good()) {
        PCAPRecord record;
        
        // Read packet header
        file.read(reinterpret_cast<char*>(&record.header), sizeof(record.header));
        if (file.gcount() != sizeof(record.header)) {
            break;
        }
        
        // Read packet data
        record.data.resize(record.header.caplen);
        file.read(reinterpret_cast<char*>(record.data.data()), record.header.caplen);
        if (file.gcount() != static_cast<std::streamsize>(record.header.caplen)) {
            break;
        }
        
        pcap.records.push_back(record);
    }
    
    return true;
}

bool PCAPDiff::compare_pcap_structures(const PCAPFile& pcap1, const PCAPFile& pcap2) {
    // Compare global headers
    if (memcmp(&pcap1.global_header, &pcap2.global_header, sizeof(PCAPGlobalHeader)) != 0) {
        std::cout << "PCAP global headers differ" << std::endl;
        return false;
    }
    
    // Compare number of packets
    if (pcap1.records.size() != pcap2.records.size()) {
        std::cout << "Different number of packets: " << pcap1.records.size() 
                 << " vs " << pcap2.records.size() << std::endl;
        return false;
    }
    
    // Compare each packet
    size_t differences = 0;
    for (size_t i = 0; i < pcap1.records.size(); ++i) {
        if (!compare_packet_records(pcap1.records[i], pcap2.records[i], i)) {
            differences++;
        }
    }
    
    std::cout << "Packet differences: " << differences << std::endl;
    return differences == 0;
}

bool PCAPDiff::compare_packet_records(const PCAPRecord& record1, 
                                     const PCAPRecord& record2, 
                                     size_t packet_index) {
    // Compare packet headers
    if (memcmp(&record1.header, &record2.header, sizeof(PCAPPacketHeader)) != 0) {
        std::cout << "Packet " << packet_index << " headers differ" << std::endl;
        return false;
    }
    
    // Compare packet data
    if (record1.data.size() != record2.data.size()) {
        std::cout << "Packet " << packet_index << " data sizes differ: " 
                 << record1.data.size() << " vs " << record2.data.size() << std::endl;
        return false;
    }
    
    if (record1.data != record2.data) {
        std::cout << "Packet " << packet_index << " data differs" << std::endl;
        return false;
    }
    
    return true;
}

PCAPDiff::Statistics PCAPDiff::analyze_pcap_file(const std::string& filename) {
    Statistics stats;
    PCAPFile pcap;
    
    if (!load_pcap_file(filename, pcap)) {
        return stats;
    }
    
    stats.total_packets = pcap.records.size();
    stats.total_bytes = 0;
    
    for (const auto& record : pcap.records) {
        stats.total_bytes += record.header.caplen;
        
        // Analyze packet types
        if (record.data.size() >= 14) { // Minimum Ethernet header size
            uint16_t ethertype = (static_cast<uint16_t>(record.data[12]) << 8) | 
                                 static_cast<uint16_t>(record.data[13]);
            
            switch (ethertype) {
                case 0x0800: // IPv4
                    stats.ipv4_packets++;
                    analyze_ipv4_packet(record.data, stats);
                    break;
                case 0x0806: // ARP
                    stats.arp_packets++;
                    break;
                case 0x86DD: // IPv6
                    stats.ipv6_packets++;
                    break;
                default:
                    stats.other_packets++;
                    break;
            }
        }
    }
    
    return stats;
}

void PCAPDiff::analyze_ipv4_packet(const std::vector<uint8_t>& data, Statistics& stats) {
    if (data.size() < 34) { // Minimum IP header size
        return;
    }
    
    uint8_t protocol = data[23]; // Protocol field in IP header
    
    switch (protocol) {
        case 1: // ICMP
            stats.icmp_packets++;
            break;
        case 6: // TCP
            stats.tcp_packets++;
            break;
        case 17: // UDP
            stats.udp_packets++;
            break;
        case 89: // OSPF
            stats.ospf_packets++;
            break;
        default:
            stats.other_ip_packets++;
            break;
    }
}

bool PCAPDiff::generate_pcap_report(const std::string& filename, 
                                   const Statistics& stats) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    file << "PCAP Analysis Report\n";
    file << "===================\n\n";
    
    file << "Total Packets: " << stats.total_packets << "\n";
    file << "Total Bytes: " << stats.total_bytes << "\n\n";
    
    file << "Packet Types:\n";
    file << "  IPv4: " << stats.ipv4_packets << "\n";
    file << "  IPv6: " << stats.ipv6_packets << "\n";
    file << "  ARP: " << stats.arp_packets << "\n";
    file << "  Other: " << stats.other_packets << "\n\n";
    
    file << "IP Protocols:\n";
    file << "  TCP: " << stats.tcp_packets << "\n";
    file << "  UDP: " << stats.udp_packets << "\n";
    file << "  ICMP: " << stats.icmp_packets << "\n";
    file << "  OSPF: " << stats.ospf_packets << "\n";
    file << "  Other: " << stats.other_ip_packets << "\n";
    
    file.close();
    return true;
}

// PCAPCapture implementation
PCAPCapture::PCAPCapture() 
    : capturing_(false), packet_count_(0), max_packets_(1000) {
}

PCAPCapture::~PCAPCapture() {
    stop_capture();
}

bool PCAPCapture::start_capture(const std::string& interface, 
                               const std::string& output_file) {
    if (capturing_) {
        return false;
    }
    
    interface_ = interface;
    output_file_ = output_file;
    packet_count_ = 0;
    
    // Start capture thread
    capturing_ = true;
    capture_thread_ = std::thread(&PCAPCapture::capture_loop, this);
    
    std::cout << "Started PCAP capture on interface " << interface 
              << " to file " << output_file << std::endl;
    return true;
}

bool PCAPCapture::stop_capture() {
    if (!capturing_) {
        return true;
    }
    
    capturing_ = false;
    
    if (capture_thread_.joinable()) {
        capture_thread_.join();
    }
    
    std::cout << "Stopped PCAP capture. Captured " << packet_count_ << " packets" << std::endl;
    return true;
}

void PCAPCapture::capture_loop() {
    // This is a simplified implementation
    // In a real implementation, you would use libpcap or similar
    
    while (capturing_ && packet_count_ < max_packets_) {
        // Simulate packet capture
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        
        // Generate dummy packet
        PCAPRecord record;
        record.header.ts_sec = time(nullptr);
        record.header.ts_usec = 0;
        record.header.caplen = 64;
        record.header.len = 64;
        
        record.data.resize(64);
        for (size_t i = 0; i < 64; ++i) {
            record.data[i] = rand() % 256;
        }
        
        // Write packet to file
        write_packet_to_file(record);
        packet_count_++;
    }
}

void PCAPCapture::write_packet_to_file(const PCAPRecord& record) {
    std::ofstream file(output_file_, std::ios::binary | std::ios::app);
    if (file.is_open()) {
        file.write(reinterpret_cast<const char*>(&record.header), sizeof(record.header));
        file.write(reinterpret_cast<const char*>(record.data.data()), record.data.size());
    }
}

void PCAPCapture::set_max_packets(size_t max_packets) {
    max_packets_ = max_packets;
}

size_t PCAPCapture::get_packet_count() const {
    return packet_count_;
}

bool PCAPCapture::is_capturing() const {
    return capturing_;
}

// PCAPReplay implementation
PCAPReplay::PCAPReplay() 
    : replaying_(false), speed_multiplier_(1.0) {
}

PCAPReplay::~PCAPReplay() {
    stop_replay();
}

bool PCAPReplay::start_replay(const std::string& pcap_file, 
                             const std::string& interface) {
    if (replaying_) {
        return false;
    }
    
    pcap_file_ = pcap_file;
    interface_ = interface;
    
    // Load PCAP file
    if (!load_pcap_file(pcap_file_, pcap_data_)) {
        std::cerr << "Failed to load PCAP file: " << pcap_file << std::endl;
        return false;
    }
    
    // Start replay thread
    replaying_ = true;
    replay_thread_ = std::thread(&PCAPReplay::replay_loop, this);
    
    std::cout << "Started PCAP replay from file " << pcap_file 
              << " to interface " << interface << std::endl;
    return true;
}

bool PCAPReplay::stop_replay() {
    if (!replaying_) {
        return true;
    }
    
    replaying_ = false;
    
    if (replay_thread_.joinable()) {
        replay_thread_.join();
    }
    
    std::cout << "Stopped PCAP replay" << std::endl;
    return true;
}

void PCAPReplay::replay_loop() {
    auto start_time = std::chrono::steady_clock::now();
    auto last_packet_time = start_time;
    
    for (size_t i = 0; i < pcap_data_.records.size() && replaying_; ++i) {
        const auto& record = pcap_data_.records[i];
        
        // Calculate delay between packets
        auto packet_time = std::chrono::steady_clock::now();
        if (i > 0) {
            auto delay = std::chrono::microseconds(
                static_cast<uint64_t>((record.header.ts_usec - 
                pcap_data_.records[i-1].header.ts_usec) / speed_multiplier_));
            
            std::this_thread::sleep_for(delay);
        }
        
        // Send packet to interface
        send_packet_to_interface(record);
        
        last_packet_time = packet_time;
    }
}

void PCAPReplay::send_packet_to_interface(const PCAPRecord& record) {
    // This is a simplified implementation
    // In a real implementation, you would use raw sockets or similar
    std::cout << "Replaying packet of size " << record.data.size() << " bytes" << std::endl;
}

void PCAPReplay::set_speed_multiplier(double multiplier) {
    speed_multiplier_ = multiplier;
}

bool PCAPReplay::is_replaying() const {
    return replaying_;
}

} // namespace RouterSim