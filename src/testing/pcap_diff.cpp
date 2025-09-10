#include "testing/pcap_diff.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <cstring>

namespace RouterSim {

PcapDiff::PcapDiff() : initialized_(false) {
}

PcapDiff::~PcapDiff() = default;

bool PcapDiff::initialize() {
    if (initialized_) {
        return true;
    }

    // Check if required tools are available
    int result = std::system("which tcpdump > /dev/null 2>&1");
    if (result != 0) {
        std::cerr << "tcpdump not found. Please install tcpdump package." << std::endl;
        return false;
    }

    result = std::system("which tshark > /dev/null 2>&1");
    if (result != 0) {
        std::cerr << "tshark not found. Please install wireshark package." << std::endl;
        return false;
    }

    initialized_ = true;
    return true;
}

bool PcapDiff::compare_pcaps(const std::string& pcap1_path, const std::string& pcap2_path, 
                            const PcapDiffOptions& options) {
    if (!initialized_) {
        if (!initialize()) {
            return false;
        }
    }

    // Read both PCAP files
    PcapData pcap1, pcap2;
    if (!read_pcap_file(pcap1_path, pcap1)) {
        std::cerr << "Failed to read PCAP file: " << pcap1_path << std::endl;
        return false;
    }

    if (!read_pcap_file(pcap2_path, pcap2)) {
        std::cerr << "Failed to read PCAP file: " << pcap2_path << std::endl;
        return false;
    }

    // Compare the data
    return compare_pcap_data(pcap1, pcap2, options);
}

bool PcapDiff::compare_pcap_data(const PcapData& pcap1, const PcapData& pcap2, 
                                const PcapDiffOptions& options) {
    differences_.clear();

    // Compare packet counts
    if (pcap1.packets.size() != pcap2.packets.size()) {
        PcapDifference diff;
        diff.type = "Packet Count Mismatch";
        diff.description = "Different number of packets: " + 
                          std::to_string(pcap1.packets.size()) + " vs " + 
                          std::to_string(pcap2.packets.size());
        differences_.push_back(diff);
    }

    // Compare packets
    size_t min_packets = std::min(pcap1.packets.size(), pcap2.packets.size());
    for (size_t i = 0; i < min_packets; ++i) {
        compare_packet(pcap1.packets[i], pcap2.packets[i], i, options);
    }

    // Compare statistics
    compare_statistics(pcap1.stats, pcap2.stats);

    return differences_.empty();
}

void PcapDiff::compare_packet(const PacketInfo& packet1, const PacketInfo& packet2, 
                             size_t packet_index, const PcapDiffOptions& options) {
    // Compare packet size
    if (packet1.size != packet2.size) {
        PcapDifference diff;
        diff.type = "Packet Size Mismatch";
        diff.packet_index = packet_index;
        diff.description = "Different packet sizes: " + std::to_string(packet1.size) + 
                          " vs " + std::to_string(packet2.size);
        differences_.push_back(diff);
    }

    // Compare timestamps (with tolerance)
    auto time_diff = std::abs(std::chrono::duration_cast<std::chrono::microseconds>(
        packet1.timestamp - packet2.timestamp).count());
    if (time_diff > options.timestamp_tolerance_us) {
        PcapDifference diff;
        diff.type = "Timestamp Mismatch";
        diff.packet_index = packet_index;
        diff.description = "Different timestamps: " + 
                          std::to_string(time_diff) + " us difference";
        differences_.push_back(diff);
    }

    // Compare packet data (if enabled)
    if (options.compare_payload && packet1.data.size() == packet2.data.size()) {
        if (memcmp(packet1.data.data(), packet2.data.data(), packet1.data.size()) != 0) {
            PcapDifference diff;
            diff.type = "Payload Mismatch";
            diff.packet_index = packet_index;
            diff.description = "Different packet payloads";
            differences_.push_back(diff);
        }
    }

    // Compare protocol-specific fields
    if (options.compare_protocols) {
        compare_protocol_fields(packet1, packet2, packet_index);
    }
}

void PcapDiff::compare_protocol_fields(const PacketInfo& packet1, const PacketInfo& packet2, 
                                      size_t packet_index) {
    // Compare IP fields
    if (packet1.src_ip != packet2.src_ip) {
        PcapDifference diff;
        diff.type = "Source IP Mismatch";
        diff.packet_index = packet_index;
        diff.description = "Different source IPs: " + packet1.src_ip + " vs " + packet2.src_ip;
        differences_.push_back(diff);
    }

    if (packet1.dst_ip != packet2.dst_ip) {
        PcapDifference diff;
        diff.type = "Destination IP Mismatch";
        diff.packet_index = packet_index;
        diff.description = "Different destination IPs: " + packet1.dst_ip + " vs " + packet2.dst_ip;
        differences_.push_back(diff);
    }

    if (packet1.protocol != packet2.protocol) {
        PcapDifference diff;
        diff.type = "Protocol Mismatch";
        diff.packet_index = packet_index;
        diff.description = "Different protocols: " + std::to_string(packet1.protocol) + 
                          " vs " + std::to_string(packet2.protocol);
        differences_.push_back(diff);
    }

    // Compare TCP/UDP fields
    if (packet1.src_port != packet2.src_port) {
        PcapDifference diff;
        diff.type = "Source Port Mismatch";
        diff.packet_index = packet_index;
        diff.description = "Different source ports: " + std::to_string(packet1.src_port) + 
                          " vs " + std::to_string(packet2.src_port);
        differences_.push_back(diff);
    }

    if (packet1.dst_port != packet2.dst_port) {
        PcapDifference diff;
        diff.type = "Destination Port Mismatch";
        diff.packet_index = packet_index;
        diff.description = "Different destination ports: " + std::to_string(packet1.dst_port) + 
                          " vs " + std::to_string(packet2.dst_port);
        differences_.push_back(diff);
    }
}

void PcapDiff::compare_statistics(const PcapStatistics& stats1, const PcapStatistics& stats2) {
    // Compare total packets
    if (stats1.total_packets != stats2.total_packets) {
        PcapDifference diff;
        diff.type = "Statistics Mismatch";
        diff.description = "Different total packet counts: " + 
                          std::to_string(stats1.total_packets) + " vs " + 
                          std::to_string(stats2.total_packets);
        differences_.push_back(diff);
    }

    // Compare total bytes
    if (stats1.total_bytes != stats2.total_bytes) {
        PcapDifference diff;
        diff.type = "Statistics Mismatch";
        diff.description = "Different total byte counts: " + 
                          std::to_string(stats1.total_bytes) + " vs " + 
                          std::to_string(stats2.total_bytes);
        differences_.push_back(diff);
    }

    // Compare protocol distributions
    for (const auto& [protocol, count1] : stats1.protocol_counts) {
        auto it = stats2.protocol_counts.find(protocol);
        if (it == stats2.protocol_counts.end()) {
            PcapDifference diff;
            diff.type = "Protocol Distribution Mismatch";
            diff.description = "Protocol " + std::to_string(protocol) + " missing in second file";
            differences_.push_back(diff);
        } else if (it->second != count1) {
            PcapDifference diff;
            diff.type = "Protocol Distribution Mismatch";
            diff.description = "Different counts for protocol " + std::to_string(protocol) + 
                              ": " + std::to_string(count1) + " vs " + std::to_string(it->second);
            differences_.push_back(diff);
        }
    }
}

bool PcapDiff::read_pcap_file(const std::string& file_path, PcapData& pcap_data) {
    // Use tshark to parse PCAP file
    std::ostringstream cmd;
    cmd << "tshark -r " << file_path << " -T fields -e frame.number -e frame.time_epoch "
        << "-e ip.src -e ip.dst -e ip.proto -e tcp.srcport -e tcp.dstport "
        << "-e udp.srcport -e udp.dstport -e frame.len -e frame.data";

    FILE* pipe = popen(cmd.str().c_str(), "r");
    if (!pipe) {
        return false;
    }

    char buffer[4096];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        std::string line(buffer);
        std::istringstream iss(line);
        std::string token;
        std::vector<std::string> tokens;

        while (std::getline(iss, token, '\t')) {
            tokens.push_back(token);
        }

        if (tokens.size() >= 8) {
            PacketInfo packet;
            packet.packet_number = std::stoul(tokens[0]);
            
            // Parse timestamp
            double epoch_time = std::stod(tokens[1]);
            auto time_point = std::chrono::system_clock::from_time_t(static_cast<time_t>(epoch_time));
            packet.timestamp = time_point;

            packet.src_ip = tokens[2];
            packet.dst_ip = tokens[3];
            packet.protocol = std::stoi(tokens[4]);
            
            // Parse ports (handle both TCP and UDP)
            if (!tokens[5].empty()) {
                packet.src_port = std::stoi(tokens[5]);
            }
            if (!tokens[6].empty()) {
                packet.dst_port = std::stoi(tokens[6]);
            }
            
            packet.size = std::stoul(tokens[7]);
            
            // Parse payload data
            if (tokens.size() > 8 && !tokens[8].empty()) {
                packet.data = hex_string_to_bytes(tokens[8]);
            }

            pcap_data.packets.push_back(packet);
        }
    }

    pclose(pipe);

    // Calculate statistics
    calculate_statistics(pcap_data);

    return true;
}

void PcapDiff::calculate_statistics(PcapData& pcap_data) {
    pcap_data.stats.total_packets = pcap_data.packets.size();
    pcap_data.stats.total_bytes = 0;

    for (const auto& packet : pcap_data.packets) {
        pcap_data.stats.total_bytes += packet.size;
        pcap_data.stats.protocol_counts[packet.protocol]++;
    }
}

std::vector<uint8_t> PcapDiff::hex_string_to_bytes(const std::string& hex_string) {
    std::vector<uint8_t> bytes;
    
    for (size_t i = 0; i < hex_string.length(); i += 2) {
        std::string byte_string = hex_string.substr(i, 2);
        if (byte_string.length() == 2) {
            uint8_t byte = static_cast<uint8_t>(std::stoul(byte_string, nullptr, 16));
            bytes.push_back(byte);
        }
    }
    
    return bytes;
}

std::vector<PcapDifference> PcapDiff::get_differences() const {
    return differences_;
}

void PcapDiff::print_differences() const {
    if (differences_.empty()) {
        std::cout << "No differences found between PCAP files." << std::endl;
        return;
    }

    std::cout << "Found " << differences_.size() << " differences:" << std::endl;
    for (const auto& diff : differences_) {
        std::cout << "  [" << diff.type << "]";
        if (diff.packet_index != SIZE_MAX) {
            std::cout << " Packet " << diff.packet_index;
        }
        std::cout << ": " << diff.description << std::endl;
    }
}

bool PcapDiff::save_differences(const std::string& output_file) const {
    std::ofstream file(output_file);
    if (!file.is_open()) {
        return false;
    }

    file << "PCAP Diff Report" << std::endl;
    file << "================" << std::endl;
    file << "Total differences: " << differences_.size() << std::endl;
    file << std::endl;

    for (const auto& diff : differences_) {
        file << "Type: " << diff.type << std::endl;
        if (diff.packet_index != SIZE_MAX) {
            file << "Packet: " << diff.packet_index << std::endl;
        }
        file << "Description: " << diff.description << std::endl;
        file << std::endl;
    }

    return true;
}

// PcapCapture implementation
PcapCapture::PcapCapture() : initialized_(false), capturing_(false) {
}

PcapCapture::~PcapCapture() {
    stop_capture();
}

bool PcapCapture::initialize() {
    if (initialized_) {
        return true;
    }

    // Check if tcpdump is available
    int result = std::system("which tcpdump > /dev/null 2>&1");
    if (result != 0) {
        std::cerr << "tcpdump not found. Please install tcpdump package." << std::endl;
        return false;
    }

    initialized_ = true;
    return true;
}

bool PcapCapture::start_capture(const std::string& interface, const std::string& output_file, 
                               const CaptureOptions& options) {
    if (!initialized_) {
        if (!initialize()) {
            return false;
        }
    }

    if (capturing_) {
        return false;
    }

    // Build tcpdump command
    std::ostringstream cmd;
    cmd << "tcpdump -i " << interface << " -w " << output_file;
    
    if (options.packet_count > 0) {
        cmd << " -c " << options.packet_count;
    }
    
    if (options.timeout_seconds > 0) {
        cmd << " -G " << options.timeout_seconds;
    }
    
    if (!options.filter.empty()) {
        cmd << " " << options.filter;
    }

    // Start capture in background
    capture_process_ = popen(cmd.str().c_str(), "r");
    if (!capture_process_) {
        return false;
    }

    capturing_ = true;
    output_file_ = output_file;
    
    std::cout << "Started packet capture on interface " << interface 
              << " to file " << output_file << std::endl;
    
    return true;
}

bool PcapCapture::stop_capture() {
    if (!capturing_) {
        return true;
    }

    if (capture_process_) {
        pclose(capture_process_);
        capture_process_ = nullptr;
    }

    capturing_ = false;
    std::cout << "Stopped packet capture" << std::endl;
    
    return true;
}

bool PcapCapture::is_capturing() const {
    return capturing_;
}

std::string PcapCapture::get_output_file() const {
    return output_file_;
}

} // namespace RouterSim