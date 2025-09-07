#include <gtest/gtest.h>
#include "router_sim.h"
#include <thread>
#include <chrono>

using namespace RouterSim;

class RouterCoreTest : public ::testing::Test {
protected:
    void SetUp() override {
        router_ = std::make_unique<RouterSimulator>();
    }
    
    void TearDown() override {
        if (router_) {
            router_->stop();
        }
    }
    
    std::unique_ptr<RouterSimulator> router_;
};

TEST_F(RouterCoreTest, Initialization) {
    RouterConfig config;
    config.router_id = "1.1.1.1";
    config.hostname = "test-router";
    config.enable_bgp = true;
    config.as_number = 65001;
    
    EXPECT_TRUE(router_->initialize(config));
    EXPECT_TRUE(router_->is_running());
}

TEST_F(RouterCoreTest, InterfaceManagement) {
    RouterConfig config;
    config.router_id = "1.1.1.1";
    config.hostname = "test-router";
    
    ASSERT_TRUE(router_->initialize(config));
    
    InterfaceConfig iface;
    iface.name = "eth0";
    iface.ip_address = "192.168.1.1";
    iface.subnet_mask = "255.255.255.0";
    iface.bandwidth_mbps = 1000;
    iface.is_up = true;
    
    EXPECT_TRUE(router_->add_interface(iface));
    
    auto interfaces = router_->get_interfaces();
    EXPECT_EQ(interfaces.size(), 1);
    EXPECT_EQ(interfaces[0].name, "eth0");
    EXPECT_EQ(interfaces[0].ip_address, "192.168.1.1");
    
    EXPECT_TRUE(router_->remove_interface("eth0"));
    
    interfaces = router_->get_interfaces();
    EXPECT_EQ(interfaces.size(), 0);
}

TEST_F(RouterCoreTest, ProtocolManagement) {
    RouterConfig config;
    config.router_id = "1.1.1.1";
    config.hostname = "test-router";
    config.enable_bgp = true;
    config.enable_ospf = true;
    config.as_number = 65001;
    config.area_id = "0.0.0.0";
    
    ASSERT_TRUE(router_->initialize(config));
    
    EXPECT_TRUE(router_->start_protocols());
    
    // Note: In a real test environment, these would require FRR to be installed
    // For now, we just test that the methods don't crash
    EXPECT_FALSE(router_->is_protocol_running("bgp"));
    EXPECT_FALSE(router_->is_protocol_running("ospf"));
    EXPECT_FALSE(router_->is_protocol_running("isis"));
    
    EXPECT_TRUE(router_->stop_protocols());
}

TEST_F(RouterCoreTest, TrafficShaping) {
    RouterConfig config;
    config.router_id = "1.1.1.1";
    config.hostname = "test-router";
    
    ASSERT_TRUE(router_->initialize(config));
    
    InterfaceConfig iface;
    iface.name = "eth0";
    iface.ip_address = "192.168.1.1";
    iface.subnet_mask = "255.255.255.0";
    iface.bandwidth_mbps = 1000;
    
    ASSERT_TRUE(router_->add_interface(iface));
    
    ShapingConfig shaping;
    shaping.rate_bps = 100000000; // 100 Mbps
    shaping.burst_size = 1000000; // 1 MB
    shaping.enable_wfq = true;
    shaping.num_queues = 8;
    
    EXPECT_TRUE(router_->configure_traffic_shaping("eth0", shaping));
    EXPECT_TRUE(router_->enable_wfq("eth0", 8));
}

TEST_F(RouterCoreTest, NetworkImpairments) {
    RouterConfig config;
    config.router_id = "1.1.1.1";
    config.hostname = "test-router";
    
    ASSERT_TRUE(router_->initialize(config));
    
    InterfaceConfig iface;
    iface.name = "eth0";
    iface.ip_address = "192.168.1.1";
    iface.subnet_mask = "255.255.255.0";
    
    ASSERT_TRUE(router_->add_interface(iface));
    
    ImpairmentConfig impairment;
    impairment.enable_delay = true;
    impairment.delay_ms = 100;
    impairment.enable_loss = true;
    impairment.loss_percent = 1.0;
    
    EXPECT_TRUE(router_->configure_impairments("eth0", impairment));
    EXPECT_TRUE(router_->clear_impairments("eth0"));
}

TEST_F(RouterCoreTest, PacketProcessing) {
    RouterConfig config;
    config.router_id = "1.1.1.1";
    config.hostname = "test-router";
    
    ASSERT_TRUE(router_->initialize(config));
    
    Packet packet;
    packet.data = {0x45, 0x00, 0x00, 0x14, 0x00, 0x00, 0x40, 0x00, 0x40, 0x01, 0x00, 0x00, 
                   0xc0, 0xa8, 0x01, 0x01, 0xc0, 0xa8, 0x01, 0x02}; // Simple IP packet
    packet.size = packet.data.size();
    packet.source_interface = "eth0";
    packet.destination_interface = "eth1";
    packet.timestamp = std::chrono::steady_clock::now();
    
    EXPECT_TRUE(router_->send_packet(packet));
    
    Packet received;
    EXPECT_TRUE(router_->receive_packet(received));
    EXPECT_EQ(received.size, packet.size);
}

TEST_F(RouterCoreTest, Statistics) {
    RouterConfig config;
    config.router_id = "1.1.1.1";
    config.hostname = "test-router";
    
    ASSERT_TRUE(router_->initialize(config));
    
    auto interface_stats = router_->get_interface_stats("eth0");
    auto protocol_stats = router_->get_protocol_stats("bgp");
    
    // Statistics should be empty initially
    EXPECT_TRUE(interface_stats.empty());
    EXPECT_TRUE(protocol_stats.empty());
    
    router_->reset_statistics();
}

TEST_F(RouterCoreTest, ScenarioManagement) {
    RouterConfig config;
    config.router_id = "1.1.1.1";
    config.hostname = "test-router";
    
    ASSERT_TRUE(router_->initialize(config));
    
    auto scenarios = router_->list_scenarios();
    EXPECT_TRUE(scenarios.empty());
    
    // Test scenario loading (would require a scenario file)
    // EXPECT_TRUE(router_->load_scenario("test_scenario.yaml"));
}
