#include <gtest/gtest.h>
#include "netem_impairments.h"
#include <thread>
#include <chrono>

using namespace RouterSim;

class NetemImpairmentsTest : public ::testing::Test {
protected:
    void SetUp() override {
        netem_ = std::make_unique<NetemImpairments>();
        netem_->start();
    }
    
    void TearDown() override {
        if (netem_) {
            netem_->stop();
        }
    }
    
    std::unique_ptr<NetemImpairments> netem_;
};

TEST_F(NetemImpairmentsTest, ImpairmentBasic) {
    Impairment delay_impairment(ImpairmentType::DELAY, 100.0, 1.0); // 100ms delay, 100% probability
    Impairment loss_impairment(ImpairmentType::LOSS, 5.0, 0.05); // 5% loss, 5% probability
    
    EXPECT_EQ(delay_impairment.get_type(), ImpairmentType::DELAY);
    EXPECT_EQ(delay_impairment.get_value(), 100.0);
    EXPECT_EQ(delay_impairment.get_probability(), 1.0);
    
    EXPECT_EQ(loss_impairment.get_type(), ImpairmentType::LOSS);
    EXPECT_EQ(loss_impairment.get_value(), 5.0);
    EXPECT_EQ(loss_impairment.get_probability(), 0.05);
}

TEST_F(NetemImpairmentsTest, ImpairmentValueChanges) {
    Impairment impairment(ImpairmentType::DELAY, 50.0, 0.8);
    
    impairment.set_value(75.0);
    EXPECT_EQ(impairment.get_value(), 75.0);
    
    impairment.set_probability(0.9);
    EXPECT_EQ(impairment.get_probability(), 0.9);
    
    // Test probability bounds
    impairment.set_probability(1.5);
    EXPECT_EQ(impairment.get_probability(), 1.0);
    
    impairment.set_probability(-0.5);
    EXPECT_EQ(impairment.get_probability(), 0.0);
}

TEST_F(NetemImpairmentsTest, ImpairmentApplication) {
    Impairment delay_impairment(ImpairmentType::DELAY, 50.0, 1.0);
    Impairment loss_impairment(ImpairmentType::LOSS, 10.0, 1.0);
    
    Packet packet;
    packet.data = {1, 2, 3, 4, 5};
    packet.size = 5;
    packet.timestamp = std::chrono::steady_clock::now();
    
    // Delay impairment should modify timestamp
    auto original_time = packet.timestamp;
    EXPECT_TRUE(delay_impairment.apply(packet));
    EXPECT_GT(packet.timestamp, original_time);
    
    // Loss impairment should return false (packet dropped)
    EXPECT_FALSE(loss_impairment.apply(packet));
}

TEST_F(NetemImpairmentsTest, InterfaceManagement) {
    EXPECT_TRUE(netem_->add_interface("eth0"));
    EXPECT_TRUE(netem_->add_interface("eth1"));
    EXPECT_FALSE(netem_->add_interface("eth0")); // Duplicate
    
    EXPECT_TRUE(netem_->remove_interface("eth0"));
    EXPECT_FALSE(netem_->remove_interface("eth0")); // Already removed
    EXPECT_FALSE(netem_->remove_interface("nonexistent"));
}

TEST_F(NetemImpairmentsTest, ImpairmentConfiguration) {
    ASSERT_TRUE(netem_->add_interface("eth0"));
    
    ImpairmentConfig config;
    config.enable_delay = true;
    config.delay_ms = 100;
    config.enable_jitter = true;
    config.jitter_ms = 20;
    config.enable_loss = true;
    config.loss_percent = 2.0;
    config.enable_duplicate = true;
    config.duplicate_percent = 1.0;
    config.enable_corruption = true;
    config.corruption_percent = 0.5;
    config.enable_reorder = true;
    config.reorder_percent = 1.5;
    
    EXPECT_TRUE(netem_->configure_impairments("eth0", config));
    EXPECT_TRUE(netem_->clear_impairments("eth0"));
    EXPECT_FALSE(netem_->configure_impairments("nonexistent", config));
}

TEST_F(NetemImpairmentsTest, IndividualImpairments) {
    ASSERT_TRUE(netem_->add_interface("eth0"));
    
    EXPECT_TRUE(netem_->add_impairment("eth0", ImpairmentType::DELAY, 50.0, 1.0));
    EXPECT_TRUE(netem_->add_impairment("eth0", ImpairmentType::JITTER, 10.0, 0.8));
    EXPECT_TRUE(netem_->add_impairment("eth0", ImpairmentType::LOSS, 5.0, 0.1));
    
    EXPECT_TRUE(netem_->remove_impairment("eth0", ImpairmentType::DELAY));
    EXPECT_FALSE(netem_->remove_impairment("eth0", ImpairmentType::DELAY)); // Already removed
    EXPECT_FALSE(netem_->remove_impairment("nonexistent", ImpairmentType::DELAY));
    
    EXPECT_TRUE(netem_->clear_impairments("eth0"));
}

TEST_F(NetemImpairmentsTest, TCAvailability) {
    // Test if tc command is available (will be false in most test environments)
    bool tc_available = netem_->is_tc_available();
    // Don't assert on this as it depends on the test environment
}

TEST_F(NetemImpairmentsTest, PacketProcessing) {
    ASSERT_TRUE(netem_->add_interface("eth0"));
    
    Packet packet;
    packet.data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    packet.size = 10;
    packet.timestamp = std::chrono::steady_clock::now();
    
    // Process packet without impairments
    EXPECT_TRUE(netem_->process_packet("eth0", packet));
    
    // Add some impairments
    ImpairmentConfig config;
    config.enable_delay = true;
    config.delay_ms = 10;
    config.enable_corruption = true;
    config.corruption_percent = 0.1;
    
    ASSERT_TRUE(netem_->configure_impairments("eth0", config));
    
    // Process packet with impairments
    Packet impaired_packet = packet;
    EXPECT_TRUE(netem_->process_packet("eth0", impaired_packet));
}

TEST_F(NetemImpairmentsTest, BatchPacketProcessing) {
    ASSERT_TRUE(netem_->add_interface("eth0"));
    
    std::vector<Packet> packets;
    for (int i = 0; i < 5; i++) {
        Packet packet;
        packet.data = {static_cast<uint8_t>(i), 1, 2, 3, 4};
        packet.size = 5;
        packet.timestamp = std::chrono::steady_clock::now();
        packets.push_back(packet);
    }
    
    auto processed_packets = netem_->process_packets("eth0", packets);
    EXPECT_EQ(processed_packets.size(), packets.size());
}

TEST_F(NetemImpairmentsTest, Statistics) {
    ASSERT_TRUE(netem_->add_interface("eth0"));
    
    auto interface_stats = netem_->get_impairment_stats("eth0");
    auto global_stats = netem_->get_global_stats();
    
    // Statistics should be available
    EXPECT_FALSE(interface_stats.empty());
    
    netem_->reset_statistics();
}

TEST_F(NetemImpairmentsTest, Control) {
    EXPECT_TRUE(netem_->is_running());
    
    netem_->stop();
    EXPECT_FALSE(netem_->is_running());
    
    netem_->start();
    EXPECT_TRUE(netem_->is_running());
}

TEST_F(NetemImpairmentsTest, ImpairmentSimulator) {
    Packet packet;
    packet.data = {1, 2, 3, 4, 5};
    packet.size = 5;
    packet.timestamp = std::chrono::steady_clock::now();
    
    // Test various simulation scenarios
    EXPECT_TRUE(ImpairmentSimulator::simulate_high_latency(packet, 100, 20));
    EXPECT_TRUE(ImpairmentSimulator::simulate_packet_loss(packet, 0.1));
    EXPECT_TRUE(ImpairmentSimulator::simulate_bandwidth_constraint(packet, 1000000));
    EXPECT_TRUE(ImpairmentSimulator::simulate_network_congestion(packet, 1.5));
    
    // Test real-world scenarios
    EXPECT_TRUE(ImpairmentSimulator::simulate_satellite_link(packet));
    EXPECT_TRUE(ImpairmentSimulator::simulate_mobile_network(packet));
    EXPECT_TRUE(ImpairmentSimulator::simulate_dsl_connection(packet));
    EXPECT_TRUE(ImpairmentSimulator::simulate_fiber_connection(packet));
    
    // Test impairment combinations
    EXPECT_TRUE(ImpairmentSimulator::simulate_poor_connection(packet));
    EXPECT_TRUE(ImpairmentSimulator::simulate_unstable_connection(packet));
    EXPECT_TRUE(ImpairmentSimulator::simulate_congested_network(packet));
}
