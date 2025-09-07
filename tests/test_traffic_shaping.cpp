#include <gtest/gtest.h>
#include "traffic_shaping.h"
#include <thread>
#include <chrono>

using namespace RouterSim;

class TrafficShapingTest : public ::testing::Test {
protected:
    void SetUp() override {
        shaper_ = std::make_unique<TrafficShaper>();
        shaper_->start();
    }
    
    void TearDown() override {
        if (shaper_) {
            shaper_->stop();
        }
    }
    
    std::unique_ptr<TrafficShaper> shaper_;
};

TEST_F(TrafficShapingTest, TokenBucketBasic) {
    TokenBucket bucket(1000000, 100000); // 1 Mbps, 100KB burst
    
    // Should have initial burst tokens
    EXPECT_GT(bucket.get_available_tokens(), 0);
    
    // Should be able to consume tokens
    EXPECT_TRUE(bucket.consume_tokens(50000)); // 50KB
    EXPECT_TRUE(bucket.consume_tokens(50000)); // Another 50KB
    
    // Should not be able to consume more than burst size
    EXPECT_FALSE(bucket.consume_tokens(1000000)); // 1MB
}

TEST_F(TrafficShapingTest, TokenBucketRefill) {
    TokenBucket bucket(1000000, 100000); // 1 Mbps, 100KB burst
    
    // Consume all tokens
    bucket.consume_tokens(100000);
    EXPECT_EQ(bucket.get_available_tokens(), 0);
    
    // Wait for refill
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    bucket.refill_tokens();
    
    // Should have some tokens back
    EXPECT_GT(bucket.get_available_tokens(), 0);
}

TEST_F(TrafficShapingTest, TokenBucketRateChange) {
    TokenBucket bucket(1000000, 100000);
    
    bucket.set_rate(2000000); // 2 Mbps
    bucket.set_burst_size(200000); // 200KB
    
    EXPECT_EQ(bucket.get_available_tokens(), 200000);
}

TEST_F(TrafficShapingTest, WFQQueueBasic) {
    WFQQueue queue(10, 1000); // Weight 10, max 1000 packets
    
    EXPECT_TRUE(queue.is_empty());
    EXPECT_EQ(queue.size(), 0);
    EXPECT_EQ(queue.get_weight(), 10);
    
    Packet packet;
    packet.data = {1, 2, 3, 4, 5};
    packet.size = 5;
    
    EXPECT_TRUE(queue.enqueue(packet));
    EXPECT_FALSE(queue.is_empty());
    EXPECT_EQ(queue.size(), 1);
    
    Packet dequeued;
    EXPECT_TRUE(queue.dequeue(dequeued));
    EXPECT_TRUE(queue.is_empty());
    EXPECT_EQ(dequeued.size, 5);
}

TEST_F(TrafficShapingTest, WFQQueueWeight) {
    WFQQueue queue(5, 1000);
    
    queue.set_weight(15);
    EXPECT_EQ(queue.get_weight(), 15);
    
    queue.set_virtual_finish_time(1000);
    EXPECT_EQ(queue.get_virtual_finish_time(), 1000);
}

TEST_F(TrafficShapingTest, TrafficShaperInterfaceManagement) {
    ShapingConfig config;
    config.rate_bps = 100000000; // 100 Mbps
    config.burst_size = 1000000; // 1 MB
    config.enable_wfq = true;
    config.num_queues = 8;
    
    EXPECT_TRUE(shaper_->add_interface("eth0", config));
    EXPECT_TRUE(shaper_->remove_interface("eth0"));
    EXPECT_FALSE(shaper_->remove_interface("nonexistent"));
}

TEST_F(TrafficShapingTest, TrafficShaperTokenBucket) {
    ShapingConfig config;
    config.rate_bps = 100000000; // 100 Mbps
    config.burst_size = 1000000; // 1 MB
    
    ASSERT_TRUE(shaper_->add_interface("eth0", config));
    
    EXPECT_TRUE(shaper_->enable_token_bucket("eth0", 50000000, 500000)); // 50 Mbps, 500KB burst
    EXPECT_TRUE(shaper_->disable_token_bucket("eth0"));
    EXPECT_FALSE(shaper_->enable_token_bucket("nonexistent", 1000000, 100000));
}

TEST_F(TrafficShapingTest, TrafficShaperWFQ) {
    ShapingConfig config;
    config.enable_wfq = true;
    config.num_queues = 8;
    
    ASSERT_TRUE(shaper_->add_interface("eth0", config));
    
    EXPECT_TRUE(shaper_->enable_wfq("eth0", 16));
    EXPECT_TRUE(shaper_->set_queue_weight("eth0", 0, 10));
    EXPECT_TRUE(shaper_->set_queue_weight("eth0", 1, 20));
    EXPECT_FALSE(shaper_->set_queue_weight("eth0", 20, 10)); // Invalid queue
    EXPECT_FALSE(shaper_->set_queue_weight("nonexistent", 0, 10));
    
    EXPECT_TRUE(shaper_->disable_wfq("eth0"));
}

TEST_F(TrafficShapingTest, TrafficShaperPacketProcessing) {
    ShapingConfig config;
    config.rate_bps = 100000000; // 100 Mbps
    config.burst_size = 1000000; // 1 MB
    config.enable_wfq = true;
    config.num_queues = 4;
    
    ASSERT_TRUE(shaper_->add_interface("eth0", config));
    
    Packet packet;
    packet.data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    packet.size = 10;
    packet.priority = 1;
    
    // Should be able to shape packet
    EXPECT_TRUE(shaper_->shape_packet("eth0", packet));
    
    // Process shaped packets
    EXPECT_TRUE(shaper_->process_shaped_packets());
}

TEST_F(TrafficShapingTest, TrafficShaperStatistics) {
    ShapingConfig config;
    config.rate_bps = 100000000;
    config.burst_size = 1000000;
    
    ASSERT_TRUE(shaper_->add_interface("eth0", config));
    
    auto stats = shaper_->get_interface_stats("eth0");
    auto queue_stats = shaper_->get_queue_stats("eth0", 0);
    
    // Statistics should be available
    EXPECT_FALSE(stats.empty());
    
    shaper_->reset_statistics();
}

TEST_F(TrafficShapingTest, ShapingAlgorithms) {
    Packet packet;
    packet.data = {1, 2, 3, 4, 5};
    packet.size = 5;
    packet.priority = 2;
    
    // Test leaky bucket
    EXPECT_TRUE(ShapingAlgorithms::leaky_bucket(packet, 1000000, 100000));
    
    // Test token bucket
    EXPECT_TRUE(ShapingAlgorithms::token_bucket(packet, 1000000, 100000));
    
    // Test WFQ weight calculation
    uint32_t weight = ShapingAlgorithms::calculate_wfq_weight(packet, 8);
    EXPECT_GT(weight, 0);
    EXPECT_LE(weight, 8);
    
    // Test priority queue calculation
    uint32_t queue = ShapingAlgorithms::calculate_priority_queue(packet, 8);
    EXPECT_GE(queue, 0);
    EXPECT_LT(queue, 8);
    
    // Test rate limiting
    EXPECT_TRUE(ShapingAlgorithms::rate_limit(packet, 1000000, 1000));
}

TEST_F(TrafficShapingTest, TrafficShaperControl) {
    EXPECT_TRUE(shaper_->is_running());
    
    shaper_->stop();
    EXPECT_FALSE(shaper_->is_running());
    
    shaper_->start();
    EXPECT_TRUE(shaper_->is_running());
}
