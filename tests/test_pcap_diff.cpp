#include <gtest/gtest.h>
#include "pcap_diff.h"
#include <fstream>
#include <cstring>

using namespace RouterSim;

class PcapDiffTest : public ::testing::Test {
protected:
    void SetUp() override {
        diff = std::make_unique<PcapDiff>();
    }
    
    std::unique_ptr<PcapDiff> diff;
    
    // Helper function to create a simple PCAP file
    void create_test_pcap(const std::string& filename) {
        std::ofstream file(filename, std::ios::binary);
        
        // PCAP global header (24 bytes)
        uint8_t global_header[24] = {
            0xD4, 0xC3, 0xB2, 0xA1,  // Magic number
            0x02, 0x00, 0x04, 0x00,  // Version major, minor
            0x00, 0x00, 0x00, 0x00,  // Thiszone
            0x00, 0x00, 0x00, 0x00,  // Sigfigs
            0x00, 0x00, 0x00, 0x40,  // Snaplen (64)
            0x00, 0x00, 0x00, 0x01   // Network (Ethernet)
        };
        file.write(reinterpret_cast<char*>(global_header), 24);
        
        // Create a simple packet record
        uint32_t timestamp_sec = 1234567890;
        uint32_t timestamp_usec = 123456;
        uint32_t packet_len = 42;
        uint32_t orig_len = 42;
        
        // Packet record header (16 bytes)
        file.write(reinterpret_cast<char*>(&timestamp_sec), 4);
        file.write(reinterpret_cast<char*>(&timestamp_usec), 4);
        file.write(reinterpret_cast<char*>(&packet_len), 4);
        file.write(reinterpret_cast<char*>(&orig_len), 4);
        
        // Simple Ethernet + IP + UDP packet (42 bytes)
        uint8_t packet_data[42] = {
            // Ethernet header (14 bytes)
            0x00, 0x11, 0x22, 0x33, 0x44, 0x55,  // Dest MAC
            0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB,  // Src MAC
            0x08, 0x00,                           // EtherType (IPv4)
            
            // IP header (20 bytes)
            0x45, 0x00, 0x00, 0x1C,              // Version, IHL, TOS, Length
            0x00, 0x00, 0x40, 0x00,              // ID, Flags, Fragment Offset
            0x40, 0x11, 0x00, 0x00,              // TTL, Protocol (UDP), Checksum
            0xC0, 0xA8, 0x01, 0x01,              // Source IP (192.168.1.1)
            0xC0, 0xA8, 0x01, 0x02,              // Dest IP (192.168.1.2)
            
            // UDP header (8 bytes)
            0x13, 0x88, 0x13, 0x88,              // Source port, Dest port (5000)
            0x00, 0x08, 0x00, 0x00,              // Length, Checksum
            
            // Payload (0 bytes)
        };
        
        file.write(reinterpret_cast<char*>(packet_data), 42);
        file.close();
    }
};

TEST_F(PcapDiffTest, LoadFiles) {
    create_test_pcap("test1.pcap");
    create_test_pcap("test2.pcap");
    
    EXPECT_TRUE(diff->load_expected("test1.pcap"));
    EXPECT_TRUE(diff->load_actual("test2.pcap"));
    
    // Clean up
    std::remove("test1.pcap");
    std::remove("test2.pcap");
}

TEST_F(PcapDiffTest, CompareIdenticalFiles) {
    create_test_pcap("test1.pcap");
    create_test_pcap("test2.pcap");
    
    diff->load_expected("test1.pcap");
    diff->load_actual("test2.pcap");
    
    auto result = diff->compare();
    EXPECT_TRUE(result.identical);
    EXPECT_EQ(result.packets_expected, 1);
    EXPECT_EQ(result.packets_actual, 1);
    EXPECT_EQ(result.similarity_score, 1.0);
    
    // Clean up
    std::remove("test1.pcap");
    std::remove("test2.pcap");
}

TEST_F(PcapDiffTest, CompareDifferentFiles) {
    create_test_pcap("test1.pcap");
    
    // Create a different packet
    std::ofstream file("test2.pcap", std::ios::binary);
    
    // PCAP global header
    uint8_t global_header[24] = {
        0xD4, 0xC3, 0xB2, 0xA1, 0x02, 0x00, 0x04, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x01
    };
    file.write(reinterpret_cast<char*>(global_header), 24);
    
    // Different packet with different IP
    uint32_t timestamp_sec = 1234567890;
    uint32_t timestamp_usec = 123456;
    uint32_t packet_len = 42;
    uint32_t orig_len = 42;
    
    file.write(reinterpret_cast<char*>(&timestamp_sec), 4);
    file.write(reinterpret_cast<char*>(&timestamp_usec), 4);
    file.write(reinterpret_cast<char*>(&packet_len), 4);
    file.write(reinterpret_cast<char*>(&orig_len), 4);
    
    // Different IP addresses
    uint8_t packet_data[42] = {
        // Ethernet header
        0x00, 0x11, 0x22, 0x33, 0x44, 0x55,
        0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB,
        0x08, 0x00,
        
        // IP header with different IPs
        0x45, 0x00, 0x00, 0x1C,
        0x00, 0x00, 0x40, 0x00,
        0x40, 0x11, 0x00, 0x00,
        0xC0, 0xA8, 0x02, 0x01,  // Different source IP (192.168.2.1)
        0xC0, 0xA8, 0x02, 0x02,  // Different dest IP (192.168.2.2)
        
        // UDP header
        0x13, 0x88, 0x13, 0x88,
        0x00, 0x08, 0x00, 0x00,
    };
    
    file.write(reinterpret_cast<char*>(packet_data), 42);
    file.close();
    
    diff->load_expected("test1.pcap");
    diff->load_actual("test2.pcap");
    
    auto result = diff->compare();
    EXPECT_FALSE(result.identical);
    EXPECT_EQ(result.packets_expected, 1);
    EXPECT_EQ(result.packets_actual, 1);
    EXPECT_LT(result.similarity_score, 1.0);
    EXPECT_FALSE(result.differences.empty());
    
    // Clean up
    std::remove("test1.pcap");
    std::remove("test2.pcap");
}

TEST_F(PcapDiffTest, Statistics) {
    create_test_pcap("test1.pcap");
    create_test_pcap("test2.pcap");
    
    diff->load_expected("test1.pcap");
    diff->load_actual("test2.pcap");
    diff->compare();
    
    auto stats = diff->get_comparison_stats();
    EXPECT_EQ(stats.total_packets, 1);
    EXPECT_EQ(stats.matching_packets, 1);
    
    // Clean up
    std::remove("test1.pcap");
    std::remove("test2.pcap");
}
