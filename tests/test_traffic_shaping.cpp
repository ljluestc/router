#include <gtest/gtest.h>
#include "traffic_shaping.h"
#include <chrono>

using namespace RouterSim;

class TrafficShapingTest : public ::testing::Test {
protected:
    void SetUp() override {
        shaper_ = std::make_unique<TrafficShaper>();
        ASSERT_TRUE(shaper_->initialize());
    }
    
    void TearDown() override {
        if (shaper_) {
            shaper_->stop();
        }
    }
    
    std::unique_ptr<TrafficShaper> shaper_;
};

TEST_F(TrafficShapingTest, Initialize) {
    EXPECT_TRUE(shaper_->is_running() == false);
}

TEST_F(TrafficShapingTest, StartStop) {
    EXPECT_TRUE(shaper_->start());
    EXPECT_TRUE(shaper_->is_running());
    
    shaper_->stop();
    EXPECT_FALSE(shaper_->is_running());
}

TEST_F(TrafficShapingTest, TokenBucket) {
    TokenBucketConfig config;
    config.capacity = 1000;
    config.rate = 100;
    config.burst_size = 500;
    config.allow_burst = true;
    
    EXPECT_TRUE(shaper_->configure_token_bucket(config));
    EXPECT_TRUE(shaper_->start());
    
    PacketInfo packet;
    packet.size = 100;
    packet.source_ip = "192.168.1.1";
    packet.dest_ip = "192.168.1.2";
    packet.timestamp = std::chrono::steady_clock::now();
    
    // First packet should be allowed
    EXPECT_TRUE(shaper_->process_packet(packet));
    
    // Multiple packets should be allowed up to capacity
    for (int i = 0; i < 10; ++i) {
        EXPECT_TRUE(shaper_->process_packet(packet));
    }
}

TEST_F(TrafficShapingTest, WFQ) {
    std::vector<WFQClass> classes;
    
    WFQClass high_priority;
    high_priority.class_id = 1;
    high_priority.weight = 10;
    high_priority.name = "High Priority";
    high_priority.is_active = true;
    classes.push_back(high_priority);
    
    WFQClass low_priority;
    low_priority.class_id = 2;
    low_priority.weight = 1;
    low_priority.name = "Low Priority";
    low_priority.is_active = true;
    classes.push_back(low_priority);
    
    EXPECT_TRUE(shaper_->configure_wfq(classes));
    EXPECT_TRUE(shaper_->start());
    
    PacketInfo packet;
    packet.size = 100;
    packet.dscp = 48; // High priority
    packet.timestamp = std::chrono::steady_clock::now();
    
    EXPECT_TRUE(shaper_->process_packet(packet));
}

TEST_F(TrafficShapingTest, Statistics) {
    shaper_->start();
    
    PacketInfo packet;
    packet.size = 100;
    packet.source_ip = "192.168.1.1";
    packet.dest_ip = "192.168.1.2";
    packet.timestamp = std::chrono::steady_clock::now();
    
    shaper_->process_packet(packet);
    
    auto stats = shaper_->get_statistics();
    EXPECT_GE(stats.packets_processed, 0);
    EXPECT_GE(stats.bytes_processed, 0);
}
