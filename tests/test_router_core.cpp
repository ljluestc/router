#include <gtest/gtest.h>
#include "router_core.h"
#include "protocols/bgp.h"
#include "protocols/ospf.h"
#include "protocols/isis.h"
#include "traffic_shaping.h"
#include "netem/impairments.h"
#include "testing/pcap_diff.h"

using namespace RouterSim;
using namespace router_sim;

class RouterCoreTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize router core
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

TEST_F(RouterCoreTest, Initialization) {
    EXPECT_TRUE(router_core_->is_initialized());
    EXPECT_FALSE(router_core_->is_running());
}

TEST_F(RouterCoreTest, StartStop) {
    EXPECT_TRUE(router_core_->start());
    EXPECT_TRUE(router_core_->is_running());
    
    EXPECT_TRUE(router_core_->stop());
    EXPECT_FALSE(router_core_->is_running());
}

TEST_F(RouterCoreTest, ProtocolManagement) {
    // Test BGP protocol
    auto bgp = std::make_unique<BGPProtocol>();
    std::map<std::string, std::string> bgp_config;
    bgp_config["local_as"] = "65001";
    bgp_config["router_id"] = "1.1.1.1";
    
    EXPECT_TRUE(bgp->initialize(bgp_config));
    EXPECT_TRUE(bgp->start());
    EXPECT_TRUE(bgp->is_running());
    
    // Test OSPF protocol
    auto ospf = std::make_unique<OSPFProtocol>();
    std::map<std::string, std::string> ospf_config;
    ospf_config["router_id"] = "2.2.2.2";
    ospf_config["area_id"] = "0.0.0.0";
    
    EXPECT_TRUE(ospf->initialize(ospf_config));
    EXPECT_TRUE(ospf->start());
    EXPECT_TRUE(ospf->is_running());
    
    // Test IS-IS protocol
    auto isis = std::make_unique<ISISProtocol>();
    std::map<std::string, std::string> isis_config;
    isis_config["system_id"] = "1921.6800.1001";
    isis_config["area_id"] = "49.0001";
    
    EXPECT_TRUE(isis->initialize(isis_config));
    EXPECT_TRUE(isis->start());
    EXPECT_TRUE(isis->is_running());
}

class TrafficShapingTest : public ::testing::Test {
protected:
    void SetUp() override {
        traffic_shaper_ = std::make_unique<TrafficShaper>();
        ASSERT_TRUE(traffic_shaper_->initialize());
    }

    void TearDown() override {
        if (traffic_shaper_) {
            traffic_shaper_->stop();
        }
    }

    std::unique_ptr<TrafficShaper> traffic_shaper_;
};

TEST_F(TrafficShapingTest, TokenBucketInitialization) {
    TokenBucketConfig config;
    config.capacity = 1000;
    config.rate = 100;
    config.burst_size = 500;
    config.allow_burst = true;
    
    EXPECT_TRUE(traffic_shaper_->configure_token_bucket(config));
}

TEST_F(TrafficShapingTest, TokenBucketConsumption) {
    TokenBucketConfig config;
    config.capacity = 1000;
    config.rate = 100;
    config.burst_size = 500;
    config.allow_burst = true;
    
    traffic_shaper_->configure_token_bucket(config);
    traffic_shaper_->set_shaping_algorithm(ShapingAlgorithm::TOKEN_BUCKET);
    traffic_shaper_->start();
    
    // Create test packet
    PacketInfo packet;
    packet.size = 100;
    packet.src_ip = "192.168.1.1";
    packet.dst_ip = "192.168.1.2";
    packet.protocol = 6; // TCP
    
    // Should be able to process packet
    EXPECT_TRUE(traffic_shaper_->process_packet(packet));
}

TEST_F(TrafficShapingTest, WFQInitialization) {
    std::vector<WFQClass> classes;
    
    WFQClass high_priority;
    high_priority.class_id = 1;
    high_priority.weight = 10;
    high_priority.min_bandwidth = 1000000;
    high_priority.max_bandwidth = 10000000;
    high_priority.name = "High Priority";
    high_priority.is_active = true;
    classes.push_back(high_priority);
    
    WFQClass low_priority;
    low_priority.class_id = 2;
    low_priority.weight = 1;
    low_priority.min_bandwidth = 100000;
    low_priority.max_bandwidth = 1000000;
    low_priority.name = "Low Priority";
    low_priority.is_active = true;
    classes.push_back(low_priority);
    
    EXPECT_TRUE(traffic_shaper_->configure_wfq(classes));
}

class NetemImpairmentsTest : public ::testing::Test {
protected:
    void SetUp() override {
        netem_ = std::make_unique<NetemImpairments>();
        ASSERT_TRUE(netem_->initialize());
    }

    void TearDown() override {
        if (netem_) {
            netem_->stop();
        }
    }

    std::unique_ptr<NetemImpairments> netem_;
};

TEST_F(NetemImpairmentsTest, Initialization) {
    EXPECT_TRUE(netem_->is_running());
}

TEST_F(NetemImpairmentsTest, DelayImpairment) {
    DelayConfig delay_config;
    delay_config.delay_ms = 100;
    delay_config.jitter_ms = 10;
    delay_config.distribution = DelayDistribution::NORMAL;
    
    // Note: This test requires root privileges and a real interface
    // In a real test environment, you would use a test interface
    // EXPECT_TRUE(netem_->add_delay("lo", delay_config));
}

TEST_F(NetemImpairmentsTest, LossImpairment) {
    LossConfig loss_config;
    loss_config.loss_type = LossType::RANDOM;
    loss_config.loss_percentage = 5.0;
    
    // Note: This test requires root privileges and a real interface
    // EXPECT_TRUE(netem_->add_loss("lo", loss_config));
}

class PcapDiffTest : public ::testing::Test {
protected:
    void SetUp() override {
        pcap_diff_ = std::make_unique<PcapDiff>();
        ASSERT_TRUE(pcap_diff_->initialize());
    }

    std::unique_ptr<PcapDiff> pcap_diff_;
};

TEST_F(PcapDiffTest, Initialization) {
    EXPECT_TRUE(pcap_diff_->initialize());
}

TEST_F(PcapDiffTest, PacketComparison) {
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
    EXPECT_TRUE(pcap_diff_->compare_pcap_data(pcap1, pcap2, options));
    EXPECT_TRUE(pcap_diff_->get_differences().empty());
}

TEST_F(PcapDiffTest, PacketDifferences) {
    // Create test packet data with differences
    PcapData pcap1, pcap2;
    
    PacketInfo packet1, packet2;
    packet1.packet_number = 1;
    packet1.timestamp = std::chrono::system_clock::now();
    packet1.src_ip = "192.168.1.1";
    packet1.dst_ip = "192.168.1.2";
    packet1.protocol = 6;
    packet1.src_port = 80;
    packet1.dst_port = 8080;
    packet1.size = 100;
    
    packet2 = packet1;
    packet2.src_ip = "192.168.1.3"; // Different source IP
    
    pcap1.packets.push_back(packet1);
    pcap2.packets.push_back(packet2);
    
    // Compare different packets
    PcapDiffOptions options;
    EXPECT_FALSE(pcap_diff_->compare_pcap_data(pcap1, pcap2, options));
    EXPECT_FALSE(pcap_diff_->get_differences().empty());
}

// Integration tests
class RouterIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        router_core_ = std::make_unique<RouterCore>();
        ASSERT_TRUE(router_core_->initialize());
        ASSERT_TRUE(router_core_->start());
    }

    void TearDown() override {
        if (router_core_) {
            router_core_->stop();
            router_core_->shutdown();
        }
    }

    std::unique_ptr<RouterCore> router_core_;
};

TEST_F(RouterIntegrationTest, FullRouterSimulation) {
    // Test complete router simulation
    EXPECT_TRUE(router_core_->is_running());
    
    // Add interfaces
    EXPECT_TRUE(router_core_->add_interface("eth0", "192.168.1.1/24"));
    EXPECT_TRUE(router_core_->add_interface("eth1", "10.0.0.1/24"));
    
    // Configure routing protocols
    std::map<std::string, std::string> bgp_config;
    bgp_config["local_as"] = "65001";
    bgp_config["router_id"] = "1.1.1.1";
    
    EXPECT_TRUE(router_core_->configure_protocol(Protocol::BGP, bgp_config));
    
    // Add routes
    Route route;
    route.destination = "0.0.0.0/0";
    route.next_hop = "192.168.1.254";
    route.protocol = Protocol::STATIC;
    route.metric = 1;
    
    EXPECT_TRUE(router_core_->add_route(route));
    
    // Verify route exists
    auto routes = router_core_->get_routes();
    EXPECT_FALSE(routes.empty());
}

// Performance tests
class RouterPerformanceTest : public ::testing::Test {
protected:
    void SetUp() override {
        traffic_shaper_ = std::make_unique<TrafficShaper>();
        ASSERT_TRUE(traffic_shaper_->initialize());
        ASSERT_TRUE(traffic_shaper_->start());
    }

    void TearDown() override {
        if (traffic_shaper_) {
            traffic_shaper_->stop();
        }
    }

    std::unique_ptr<TrafficShaper> traffic_shaper_;
};

TEST_F(RouterPerformanceTest, HighThroughputProcessing) {
    // Test high throughput packet processing
    const int num_packets = 10000;
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < num_packets; ++i) {
        PacketInfo packet;
        packet.size = 64; // Small packet
        packet.src_ip = "192.168.1.1";
        packet.dst_ip = "192.168.1.2";
        packet.protocol = 6;
        
        traffic_shaper_->process_packet_async(packet);
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    std::cout << "Processed " << num_packets << " packets in " 
              << duration.count() << " ms" << std::endl;
    
    // Should complete within reasonable time
    EXPECT_LT(duration.count(), 1000); // Less than 1 second
}

// Main test runner
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}