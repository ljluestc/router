#include <gtest/gtest.h>
#include "traffic_shaping/drr.h"
#include <chrono>
#include <thread>

using namespace RouterSim;

class DRRTest : public ::testing::Test {
protected:
    void SetUp() override {
        drr_ = std::make_unique<DeficitRoundRobin>();
    }

    void TearDown() override {
        if (drr_) {
            // Cleanup if needed
        }
    }

    std::unique_ptr<DeficitRoundRobin> drr_;
};

TEST_F(DRRTest, Initialization) {
    std::vector<DRRClass> classes;
    
    DRRClass high_priority;
    high_priority.class_id = 1;
    high_priority.quantum = 1000;
    high_priority.deficit = 1000;
    high_priority.min_bandwidth = 1000000;
    high_priority.max_bandwidth = 10000000;
    high_priority.name = "High Priority";
    high_priority.is_active = true;
    classes.push_back(high_priority);
    
    DRRClass low_priority;
    low_priority.class_id = 2;
    low_priority.quantum = 100;
    low_priority.deficit = 100;
    low_priority.min_bandwidth = 100000;
    low_priority.max_bandwidth = 1000000;
    low_priority.name = "Low Priority";
    low_priority.is_active = true;
    classes.push_back(low_priority);
    
    EXPECT_TRUE(drr_->initialize(classes));
}

TEST_F(DRRTest, EnqueueDequeue) {
    std::vector<DRRClass> classes;
    
    DRRClass test_class;
    test_class.class_id = 1;
    test_class.quantum = 1000;
    test_class.deficit = 1000;
    test_class.min_bandwidth = 1000000;
    test_class.max_bandwidth = 10000000;
    test_class.name = "Test Class";
    test_class.is_active = true;
    classes.push_back(test_class);
    
    ASSERT_TRUE(drr_->initialize(classes));
    
    // Create test packet
    PacketInfo packet;
    packet.size = 100;
    packet.src_ip = "192.168.1.1";
    packet.dst_ip = "192.168.1.2";
    packet.protocol = 6; // TCP
    packet.dscp = 0;
    
    // Enqueue packet
    EXPECT_TRUE(drr_->enqueue_packet(packet, 1));
    EXPECT_FALSE(drr_->is_empty());
    EXPECT_EQ(drr_->queue_size(), 1);
    EXPECT_EQ(drr_->queue_size(1), 1);
    
    // Dequeue packet
    PacketInfo dequeued_packet;
    EXPECT_TRUE(drr_->dequeue_packet(dequeued_packet));
    EXPECT_EQ(dequeued_packet.size, packet.size);
    EXPECT_EQ(dequeued_packet.src_ip, packet.src_ip);
    EXPECT_EQ(dequeued_packet.dst_ip, packet.dst_ip);
    EXPECT_TRUE(drr_->is_empty());
}

TEST_F(DRRTest, FairnessTest) {
    std::vector<DRRClass> classes;
    
    // Create two classes with different quantums
    DRRClass class1;
    class1.class_id = 1;
    class1.quantum = 1000;
    class1.deficit = 1000;
    class1.min_bandwidth = 1000000;
    class1.max_bandwidth = 10000000;
    class1.name = "Class 1";
    class1.is_active = true;
    classes.push_back(class1);
    
    DRRClass class2;
    class2.class_id = 2;
    class2.quantum = 500;
    class2.deficit = 500;
    class2.min_bandwidth = 500000;
    class2.max_bandwidth = 5000000;
    class2.name = "Class 2";
    class2.is_active = true;
    classes.push_back(class2);
    
    ASSERT_TRUE(drr_->initialize(classes));
    
    // Enqueue packets for both classes
    for (int i = 0; i < 10; ++i) {
        PacketInfo packet1, packet2;
        packet1.size = 100;
        packet1.src_ip = "192.168.1.1";
        packet1.dst_ip = "192.168.1.2";
        packet1.protocol = 6;
        packet1.dscp = 0;
        
        packet2.size = 100;
        packet2.src_ip = "192.168.1.3";
        packet2.dst_ip = "192.168.1.4";
        packet2.protocol = 6;
        packet2.dscp = 0;
        
        EXPECT_TRUE(drr_->enqueue_packet(packet1, 1));
        EXPECT_TRUE(drr_->enqueue_packet(packet2, 2));
    }
    
    // Dequeue packets and count per class
    int class1_count = 0, class2_count = 0;
    PacketInfo packet;
    
    while (drr_->dequeue_packet(packet)) {
        if (packet.src_ip == "192.168.1.1") {
            class1_count++;
        } else if (packet.src_ip == "192.168.1.3") {
            class2_count++;
        }
    }
    
    // Class 1 should get more packets due to higher quantum (1000 vs 500)
    EXPECT_GT(class1_count, class2_count);
    EXPECT_EQ(class1_count + class2_count, 20);
}

TEST_F(DRRTest, Statistics) {
    std::vector<DRRClass> classes;
    
    DRRClass test_class;
    test_class.class_id = 1;
    test_class.quantum = 1000;
    test_class.deficit = 1000;
    test_class.min_bandwidth = 1000000;
    test_class.max_bandwidth = 10000000;
    test_class.name = "Test Class";
    test_class.is_active = true;
    classes.push_back(test_class);
    
    ASSERT_TRUE(drr_->initialize(classes));
    
    // Enqueue some packets
    for (int i = 0; i < 5; ++i) {
        PacketInfo packet;
        packet.size = 100;
        packet.src_ip = "192.168.1.1";
        packet.dst_ip = "192.168.1.2";
        packet.protocol = 6;
        packet.dscp = 0;
        
        EXPECT_TRUE(drr_->enqueue_packet(packet, 1));
    }
    
    // Check statistics
    auto stats = drr_->get_statistics();
    EXPECT_EQ(stats.total_packets_queued, 5);
    EXPECT_EQ(stats.total_bytes_queued, 500);
    EXPECT_EQ(stats.current_queue_length, 5);
    EXPECT_EQ(stats.class_statistics[1].packets_queued, 5);
    EXPECT_EQ(stats.class_statistics[1].bytes_queued, 500);
    
    // Dequeue some packets
    PacketInfo packet;
    int dequeued = 0;
    while (drr_->dequeue_packet(packet) && dequeued < 3) {
        dequeued++;
    }
    
    // Check updated statistics
    stats = drr_->get_statistics();
    EXPECT_EQ(stats.total_packets_dequeued, 3);
    EXPECT_EQ(stats.total_bytes_dequeued, 300);
    EXPECT_EQ(stats.current_queue_length, 2);
    EXPECT_EQ(stats.class_statistics[1].packets_dequeued, 3);
    EXPECT_EQ(stats.class_statistics[1].bytes_dequeued, 300);
}

TEST_F(DRRTest, ClassManagement) {
    std::vector<DRRClass> classes;
    
    DRRClass class1;
    class1.class_id = 1;
    class1.quantum = 1000;
    class1.deficit = 1000;
    class1.min_bandwidth = 1000000;
    class1.max_bandwidth = 10000000;
    class1.name = "Class 1";
    class1.is_active = true;
    classes.push_back(class1);
    
    ASSERT_TRUE(drr_->initialize(classes));
    
    // Add a new class
    DRRClass class2;
    class2.class_id = 2;
    class2.quantum = 500;
    class2.deficit = 500;
    class2.min_bandwidth = 500000;
    class2.max_bandwidth = 5000000;
    class2.name = "Class 2";
    class2.is_active = true;
    
    EXPECT_TRUE(drr_->add_class(class2));
    
    // Verify class was added
    auto classes_list = drr_->get_classes();
    EXPECT_EQ(classes_list.size(), 2);
    
    // Remove a class
    EXPECT_TRUE(drr_->remove_class(1));
    
    // Verify class was removed
    classes_list = drr_->get_classes();
    EXPECT_EQ(classes_list.size(), 1);
    EXPECT_EQ(classes_list[0].class_id, 2);
}

TEST_F(DRRTest, PerformanceTest) {
    std::vector<DRRClass> classes;
    
    DRRClass test_class;
    test_class.class_id = 1;
    test_class.quantum = 1000;
    test_class.deficit = 1000;
    test_class.min_bandwidth = 1000000;
    test_class.max_bandwidth = 10000000;
    test_class.name = "Test Class";
    test_class.is_active = true;
    classes.push_back(test_class);
    
    ASSERT_TRUE(drr_->initialize(classes));
    
    const int num_packets = 10000;
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Enqueue packets
    for (int i = 0; i < num_packets; ++i) {
        PacketInfo packet;
        packet.size = 64; // Small packet
        packet.src_ip = "192.168.1.1";
        packet.dst_ip = "192.168.1.2";
        packet.protocol = 6;
        packet.dscp = 0;
        
        EXPECT_TRUE(drr_->enqueue_packet(packet, 1));
    }
    
    auto enqueue_time = std::chrono::high_resolution_clock::now();
    
    // Dequeue packets
    PacketInfo packet;
    int dequeued = 0;
    while (drr_->dequeue_packet(packet) && dequeued < num_packets) {
        dequeued++;
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    
    auto enqueue_duration = std::chrono::duration_cast<std::chrono::microseconds>(
        enqueue_time - start_time).count();
    auto dequeue_duration = std::chrono::duration_cast<std::chrono::microseconds>(
        end_time - enqueue_time).count();
    auto total_duration = std::chrono::duration_cast<std::chrono::microseconds>(
        end_time - start_time).count();
    
    std::cout << "DRR Performance Test:" << std::endl;
    std::cout << "  Packets processed: " << num_packets << std::endl;
    std::cout << "  Enqueue time: " << enqueue_duration << " μs" << std::endl;
    std::cout << "  Dequeue time: " << dequeue_duration << " μs" << std::endl;
    std::cout << "  Total time: " << total_duration << " μs" << std::endl;
    std::cout << "  Packets per second: " << (num_packets * 1000000) / total_duration << std::endl;
    
    EXPECT_EQ(dequeued, num_packets);
    EXPECT_LT(total_duration, 100000); // Should complete within 100ms
}

// Main test runner
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
