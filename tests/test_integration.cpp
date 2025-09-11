#include <gtest/gtest.h>
#include "router_core.h"
#include "protocols/bgp.h"
#include "protocols/ospf.h"
#include "protocols/isis.h"
#include "traffic_shaping.h"
#include "traffic_shaping/wfq.h"
#include "traffic_shaping/drr.h"
#include "netem/impairments.h"
#include "testing/pcap_diff.h"
#include <chrono>
#include <thread>

using namespace RouterSim;
using namespace router_sim;

class RouterIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        router_core_ = std::make_unique<RouterCore>();
        ASSERT_TRUE(router_core_->initialize());
    }

    void TearDown() override {
        if (router_core_) {
            router_core_->shutdown();
        }
    }

    std::unique_ptr<RouterCore> router_core_;
};

TEST_F(RouterIntegrationTest, FullRouterSimulation) {
    // Test complete router simulation with multiple protocols
    EXPECT_TRUE(router_core_->is_initialized());
    
    // Add interfaces
    EXPECT_TRUE(router_core_->add_interface("eth0", "192.168.1.1/24"));
    EXPECT_TRUE(router_core_->add_interface("eth1", "10.0.0.1/24"));
    
    // Configure BGP
    std::map<std::string, std::string> bgp_config;
    bgp_config["local_as"] = "65001";
    bgp_config["router_id"] = "192.168.1.1";
    bgp_config["neighbors"] = "192.168.1.2,10.0.0.2";
    
    EXPECT_TRUE(router_core_->configure_protocol(Protocol::BGP, bgp_config));
    
    // Configure OSPF
    std::map<std::string, std::string> ospf_config;
    ospf_config["router_id"] = "192.168.1.1";
    ospf_config["area"] = "0";
    
    EXPECT_TRUE(router_core_->configure_protocol(Protocol::OSPF, ospf_config));
    
    // Add static routes
    Route route1;
    route1.destination = "0.0.0.0/0";
    route1.next_hop = "192.168.1.254";
    route1.protocol = Protocol::STATIC;
    route1.metric = 1;
    
    Route route2;
    route2.destination = "172.16.0.0/16";
    route2.next_hop = "10.0.0.254";
    route2.protocol = Protocol::STATIC;
    route2.metric = 2;
    
    EXPECT_TRUE(router_core_->add_route(route1));
    EXPECT_TRUE(router_core_->add_route(route2));
    
    // Start router
    EXPECT_TRUE(router_core_->start());
    EXPECT_TRUE(router_core_->is_running());
    
    // Verify routes exist
    auto routes = router_core_->get_routes();
    EXPECT_GE(routes.size(), 2);
    
    // Stop router
    EXPECT_TRUE(router_core_->stop());
    EXPECT_FALSE(router_core_->is_running());
}

TEST_F(RouterIntegrationTest, TrafficShapingIntegration) {
    // Test traffic shaping with multiple algorithms
    auto traffic_shaper = std::make_unique<TrafficShaper>();
    ASSERT_TRUE(traffic_shaper->initialize());
    
    // Configure token bucket
    TokenBucketConfig tb_config;
    tb_config.capacity = 10000;
    tb_config.rate = 1000;
    tb_config.burst_size = 5000;
    tb_config.allow_burst = true;
    
    EXPECT_TRUE(traffic_shaper->configure_token_bucket(tb_config));
    traffic_shaper->set_shaping_algorithm(ShapingAlgorithm::TOKEN_BUCKET);
    
    // Configure WFQ
    std::vector<WFQClass> wfq_classes;
    
    WFQClass high_priority;
    high_priority.class_id = 1;
    high_priority.weight = 10;
    high_priority.min_bandwidth = 1000000;
    high_priority.max_bandwidth = 10000000;
    high_priority.name = "High Priority";
    high_priority.is_active = true;
    wfq_classes.push_back(high_priority);
    
    WFQClass low_priority;
    low_priority.class_id = 2;
    low_priority.weight = 1;
    low_priority.min_bandwidth = 100000;
    low_priority.max_bandwidth = 1000000;
    low_priority.name = "Low Priority";
    low_priority.is_active = true;
    wfq_classes.push_back(low_priority);
    
    EXPECT_TRUE(traffic_shaper->configure_wfq(wfq_classes));
    
    // Configure DRR
    std::vector<DRRClass> drr_classes;
    
    DRRClass drr_class1;
    drr_class1.class_id = 1;
    drr_class1.quantum = 1000;
    drr_class1.deficit = 1000;
    drr_class1.min_bandwidth = 1000000;
    drr_class1.max_bandwidth = 10000000;
    drr_class1.name = "DRR Class 1";
    drr_class1.is_active = true;
    drr_classes.push_back(drr_class1);
    
    EXPECT_TRUE(traffic_shaper->configure_drr(drr_classes));
    
    // Start traffic shaper
    EXPECT_TRUE(traffic_shaper->start());
    
    // Test packet processing
    PacketInfo packet;
    packet.size = 100;
    packet.src_ip = "192.168.1.1";
    packet.dst_ip = "192.168.1.2";
    packet.protocol = 6;
    packet.dscp = 48; // High priority
    
    EXPECT_TRUE(traffic_shaper->process_packet(packet));
    
    // Test statistics
    auto stats = traffic_shaper->get_statistics();
    EXPECT_GT(stats.packets_processed, 0);
    
    traffic_shaper->stop();
}

TEST_F(RouterIntegrationTest, NetworkImpairmentsIntegration) {
    // Test network impairments integration
    auto netem = std::make_unique<NetemImpairments>();
    ASSERT_TRUE(netem->initialize());
    
    // Configure delay impairment
    DelayConfig delay_config;
    delay_config.delay_ms = 100;
    delay_config.jitter_ms = 10;
    delay_config.distribution = DelayDistribution::NORMAL;
    
    // Note: This test requires root privileges and a real interface
    // In a real test environment, you would use a test interface
    // EXPECT_TRUE(netem->add_delay("lo", delay_config));
    
    // Configure loss impairment
    LossConfig loss_config;
    loss_config.loss_type = LossType::RANDOM;
    loss_config.loss_percentage = 1.0;
    
    // Note: This test requires root privileges and a real interface
    // EXPECT_TRUE(netem->add_loss("lo", loss_config));
    
    // Test statistics
    auto stats = netem->get_statistics();
    EXPECT_TRUE(netem->is_running());
    
    netem->stop();
}

TEST_F(RouterIntegrationTest, PcapDiffIntegration) {
    // Test pcap diffing integration
    auto pcap_diff = std::make_unique<PcapDiff>();
    ASSERT_TRUE(pcap_diff->initialize());
    
    // Create test packet data
    PcapData pcap1, pcap2;
    
    // Create identical packets
    PacketInfo packet1, packet2;
    packet1.packet_number = 1;
    packet1.timestamp = std::chrono::system_clock::now();
    packet1.src_ip = "192.168.1.1";
    packet1.dst_ip = "192.168.1.2";
    packet1.protocol = 6;
    packet1.src_port = 80;
    packet1.dst_port = 8080;
    packet1.size = 100;
    
    packet2 = packet1; // Copy
    
    pcap1.packets.push_back(packet1);
    pcap2.packets.push_back(packet2);
    
    // Compare identical packets
    PcapDiffOptions options;
    EXPECT_TRUE(pcap_diff->compare_pcap_data(pcap1, pcap2, options));
    EXPECT_TRUE(pcap_diff->get_differences().empty());
    
    // Test with differences
    packet2.src_ip = "192.168.1.3"; // Different source IP
    pcap2.packets.clear();
    pcap2.packets.push_back(packet2);
    
    EXPECT_FALSE(pcap_diff->compare_pcap_data(pcap1, pcap2, options));
    EXPECT_FALSE(pcap_diff->get_differences().empty());
}

TEST_F(RouterIntegrationTest, PerformanceBenchmark) {
    // Performance benchmark test
    auto traffic_shaper = std::make_unique<TrafficShaper>();
    ASSERT_TRUE(traffic_shaper->initialize());
    ASSERT_TRUE(traffic_shaper->start());
    
    const int num_packets = 100000;
    const int num_threads = 4;
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Create multiple threads to process packets
    std::vector<std::thread> threads;
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&, t]() {
            for (int i = 0; i < num_packets / num_threads; ++i) {
                PacketInfo packet;
                packet.size = 64; // Small packet
                packet.src_ip = "192.168.1." + std::to_string(t + 1);
                packet.dst_ip = "192.168.1.2";
                packet.protocol = 6;
                packet.dscp = 0;
                
                traffic_shaper->process_packet_async(packet);
            }
        });
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_time - start_time).count();
    
    std::cout << "Performance Benchmark:" << std::endl;
    std::cout << "  Packets processed: " << num_packets << std::endl;
    std::cout << "  Threads: " << num_threads << std::endl;
    std::cout << "  Time: " << duration << " ms" << std::endl;
    std::cout << "  Packets per second: " << (num_packets * 1000) / duration << std::endl;
    
    // Should process packets at a reasonable rate
    EXPECT_LT(duration, 5000); // Less than 5 seconds
    
    traffic_shaper->stop();
}

TEST_F(RouterIntegrationTest, StressTest) {
    // Stress test with high packet rates
    auto traffic_shaper = std::make_unique<TrafficShaper>();
    ASSERT_TRUE(traffic_shaper->initialize());
    ASSERT_TRUE(traffic_shaper->start());
    
    const int num_packets = 1000000;
    const int batch_size = 1000;
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    for (int batch = 0; batch < num_packets / batch_size; ++batch) {
        for (int i = 0; i < batch_size; ++i) {
            PacketInfo packet;
            packet.size = 64;
            packet.src_ip = "192.168.1.1";
            packet.dst_ip = "192.168.1.2";
            packet.protocol = 6;
            packet.dscp = 0;
            
            traffic_shaper->process_packet_async(packet);
        }
        
        // Small delay between batches
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_time - start_time).count();
    
    std::cout << "Stress Test:" << std::endl;
    std::cout << "  Packets processed: " << num_packets << std::endl;
    std::cout << "  Time: " << duration << " ms" << std::endl;
    std::cout << "  Packets per second: " << (num_packets * 1000) / duration << std::endl;
    
    // Should handle high packet rates
    EXPECT_LT(duration, 10000); // Less than 10 seconds
    
    traffic_shaper->stop();
}

// Main test runner
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
