#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <chrono>
#include <thread>
#include <vector>
#include <memory>

#include "router_core.h"
#include "frr_integration/frr_client.h"
#include "traffic_shaping/token_bucket.h"
#include "traffic_shaping/wfq.h"
#include "analytics/clickhouse_client.h"

using namespace router_sim;
using namespace router_sim::frr;
using namespace router_sim::traffic_shaping;
using namespace router_sim::analytics;

class RouterCoreTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize test environment
        router_core_ = std::make_unique<RouterCore>();
    }
    
    void TearDown() override {
        router_core_.reset();
    }
    
    std::unique_ptr<RouterCore> router_core_;
};

TEST_F(RouterCoreTest, Initialization) {
    EXPECT_TRUE(router_core_ != nullptr);
    EXPECT_FALSE(router_core_->is_running());
}

TEST_F(RouterCoreTest, StartStop) {
    EXPECT_TRUE(router_core_->start());
    EXPECT_TRUE(router_core_->is_running());
    
    router_core_->stop();
    EXPECT_FALSE(router_core_->is_running());
}

TEST_F(RouterCoreTest, InterfaceManagement) {
    InterfaceConfig config;
    config.name = "eth0";
    config.ip = "192.168.1.1";
    config.mask = "255.255.255.0";
    config.mtu = 1500;
    config.enabled = true;
    
    EXPECT_TRUE(router_core_->add_interface(config));
    
    auto interfaces = router_core_->get_interfaces();
    EXPECT_EQ(interfaces.size(), 1);
    EXPECT_EQ(interfaces[0].name, "eth0");
    EXPECT_EQ(interfaces[0].ip, "192.168.1.1");
}

TEST_F(RouterCoreTest, RouteManagement) {
    Route route;
    route.destination = "192.168.2.0/24";
    route.gateway = "192.168.1.2";
    route.interface = "eth0";
    route.metric = 10;
    route.protocol = "static";
    
    EXPECT_TRUE(router_core_->add_route(route));
    
    auto routes = router_core_->get_routes();
    EXPECT_EQ(routes.size(), 1);
    EXPECT_EQ(routes[0].destination, "192.168.2.0/24");
}

class FRRIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        frr_client_ = std::make_unique<FRRClient>();
    }
    
    void TearDown() override {
        frr_client_->disconnect();
        frr_client_.reset();
    }
    
    std::unique_ptr<FRRClient> frr_client_;
};

TEST_F(FRRIntegrationTest, Connection) {
    EXPECT_TRUE(frr_client_->connect());
    EXPECT_TRUE(frr_client_->is_connected());
    
    frr_client_->disconnect();
    EXPECT_FALSE(frr_client_->is_connected());
}

TEST_F(FRRIntegrationTest, BGPConfiguration) {
    EXPECT_TRUE(frr_client_->connect());
    
    BGPConfig config;
    config.local_asn = 65001;
    config.router_id = "192.168.1.1";
    
    BGPNeighbor neighbor;
    neighbor.ip = "192.168.1.2";
    neighbor.asn = 65002;
    neighbor.enabled = true;
    config.neighbors.push_back(neighbor);
    
    EXPECT_TRUE(frr_client_->configure_bgp(config));
    EXPECT_TRUE(frr_client_->start_bgp());
    EXPECT_TRUE(frr_client_->is_bgp_running());
    
    frr_client_->stop_bgp();
    EXPECT_FALSE(frr_client_->is_bgp_running());
}

TEST_F(FRRIntegrationTest, OSPFConfiguration) {
    EXPECT_TRUE(frr_client_->connect());
    
    OSPFConfig config;
    config.router_id = "192.168.1.1";
    
    OSPFInterface interface;
    interface.name = "eth0";
    interface.area = 0;
    interface.cost = 10;
    interface.priority = 1;
    interface.enabled = true;
    config.interfaces.push_back(interface);
    
    EXPECT_TRUE(frr_client_->configure_ospf(config));
    EXPECT_TRUE(frr_client_->start_ospf());
    EXPECT_TRUE(frr_client_->is_ospf_running());
    
    frr_client_->stop_ospf();
    EXPECT_FALSE(frr_client_->is_ospf_running());
}

class TokenBucketTest : public ::testing::Test {
protected:
    void SetUp() override {
        TokenBucketConfig config;
        config.rate_bps = 1000000; // 1 Mbps
        config.burst_bytes = 100000; // 100 KB
        config.bucket_size = 100000;
        config.enabled = true;
        
        token_bucket_ = std::make_unique<TokenBucket>(config);
    }
    
    void TearDown() override {
        token_bucket_->stop();
        token_bucket_.reset();
    }
    
    std::unique_ptr<TokenBucket> token_bucket_;
};

TEST_F(TokenBucketTest, TokenConsumption) {
    token_bucket_->start();
    
    // Should be able to consume tokens up to burst size
    EXPECT_TRUE(token_bucket_->consume_tokens(50000));
    EXPECT_TRUE(token_bucket_->consume_tokens(50000));
    
    // Should not be able to consume more than burst size
    EXPECT_FALSE(token_bucket_->consume_tokens(10000));
}

TEST_F(TokenBucketTest, RateLimiting) {
    token_bucket_->start();
    
    // Consume all tokens
    EXPECT_TRUE(token_bucket_->consume_tokens(100000));
    
    // Wait for refill
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Should be able to consume some tokens
    EXPECT_TRUE(token_bucket_->consume_tokens(10000));
}

TEST_F(TokenBucketTest, Statistics) {
    token_bucket_->start();
    
    token_bucket_->consume_tokens(50000);
    token_bucket_->consume_tokens(50000);
    token_bucket_->consume_tokens(10000); // Should fail
    
    EXPECT_EQ(token_bucket_->get_total_packets_processed(), 2);
    EXPECT_EQ(token_bucket_->get_total_packets_dropped(), 1);
    EXPECT_GT(token_bucket_->get_drop_rate(), 0.0);
}

class WFQTest : public ::testing::Test {
protected:
    void SetUp() override {
        WFQConfig config;
        config.total_bandwidth = 1000000; // 1 Mbps
        config.enabled = true;
        
        QueueConfig queue1;
        queue1.queue_id = 1;
        queue1.weight = 1;
        queue1.max_size = 1000;
        queue1.enabled = true;
        queue1.name = "high_priority";
        config.queues.push_back(queue1);
        
        QueueConfig queue2;
        queue2.queue_id = 2;
        queue2.weight = 2;
        queue2.max_size = 1000;
        queue2.enabled = true;
        queue2.name = "low_priority";
        config.queues.push_back(queue2);
        
        wfq_scheduler_ = std::make_unique<WFQScheduler>(config);
    }
    
    void TearDown() override {
        wfq_scheduler_->stop();
        wfq_scheduler_.reset();
    }
    
    std::unique_ptr<WFQScheduler> wfq_scheduler_;
};

TEST_F(WFQTest, QueueManagement) {
    EXPECT_TRUE(wfq_scheduler_->add_queue({3, 1, 1000, 0, 0, true, "test_queue"}));
    EXPECT_EQ(wfq_scheduler_->get_queue_size(3), 0);
    EXPECT_TRUE(wfq_scheduler_->is_queue_empty(3));
    
    EXPECT_TRUE(wfq_scheduler_->remove_queue(3));
}

TEST_F(WFQTest, PacketScheduling) {
    wfq_scheduler_->start();
    
    Packet packet1;
    packet1.data = std::vector<uint8_t>(100, 0xAA);
    packet1.size = 100;
    packet1.priority = 1;
    packet1.flow_id = 1;
    packet1.arrival_time = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    
    Packet packet2;
    packet2.data = std::vector<uint8_t>(200, 0xBB);
    packet2.size = 200;
    packet2.priority = 2;
    packet2.flow_id = 2;
    packet2.arrival_time = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    
    EXPECT_TRUE(wfq_scheduler_->enqueue_packet(1, packet1));
    EXPECT_TRUE(wfq_scheduler_->enqueue_packet(2, packet2));
    
    EXPECT_EQ(wfq_scheduler_->get_total_packets(), 2);
    
    Packet dequeued;
    EXPECT_TRUE(wfq_scheduler_->dequeue_packet(dequeued));
    EXPECT_EQ(dequeued.size, 100);
}

class ClickHouseTest : public ::testing::Test {
protected:
    void SetUp() override {
        clickhouse_client_ = std::make_unique<ClickHouseClient>();
    }
    
    void TearDown() override {
        clickhouse_client_->disconnect();
        clickhouse_client_.reset();
    }
    
    std::unique_ptr<ClickHouseClient> clickhouse_client_;
};

TEST_F(ClickHouseTest, Connection) {
    EXPECT_TRUE(clickhouse_client_->connect());
    EXPECT_TRUE(clickhouse_client_->is_connected());
    
    clickhouse_client_->disconnect();
    EXPECT_FALSE(clickhouse_client_->is_connected());
}

TEST_F(ClickHouseTest, MetricsInsertion) {
    EXPECT_TRUE(clickhouse_client_->connect());
    
    std::vector<MetricData> metrics;
    MetricData metric;
    metric.name = "test_metric";
    metric.value = "100";
    metric.labels = "test=value";
    metric.timestamp = std::chrono::system_clock::now();
    metric.source = "test";
    metric.type = "counter";
    metrics.push_back(metric);
    
    EXPECT_TRUE(clickhouse_client_->insert_metrics(metrics));
    
    EXPECT_GT(clickhouse_client_->get_insert_count(), 0);
}

TEST_F(ClickHouseTest, PacketMetricsInsertion) {
    EXPECT_TRUE(clickhouse_client_->connect());
    
    std::vector<PacketMetrics> packet_metrics;
    PacketMetrics packet_metric;
    packet_metric.total_packets = 1000;
    packet_metric.bytes_transferred = 1000000;
    packet_metric.packets_dropped = 10;
    packet_metric.packets_duplicated = 5;
    packet_metric.packets_reordered = 2;
    packet_metric.avg_latency_ms = 10.5;
    packet_metric.max_latency_ms = 50.0;
    packet_metric.min_latency_ms = 1.0;
    packet_metric.timestamp = std::chrono::system_clock::now();
    packet_metrics.push_back(packet_metric);
    
    EXPECT_TRUE(clickhouse_client_->insert_packet_metrics(packet_metrics));
    
    EXPECT_GT(clickhouse_client_->get_insert_count(), 0);
}

class IntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        router_core_ = std::make_unique<RouterCore>();
        frr_client_ = std::make_unique<FRRClient>();
        token_bucket_ = std::make_unique<TokenBucket>(TokenBucketConfig{1000000, 100000, 100000, std::chrono::milliseconds(10), true});
        clickhouse_client_ = std::make_unique<ClickHouseClient>();
    }
    
    void TearDown() override {
        router_core_->stop();
        frr_client_->disconnect();
        token_bucket_->stop();
        clickhouse_client_->disconnect();
    }
    
    std::unique_ptr<RouterCore> router_core_;
    std::unique_ptr<FRRClient> frr_client_;
    std::unique_ptr<TokenBucket> token_bucket_;
    std::unique_ptr<ClickHouseClient> clickhouse_client_;
};

TEST_F(IntegrationTest, FullSystemIntegration) {
    // Start all components
    EXPECT_TRUE(router_core_->start());
    EXPECT_TRUE(frr_client_->connect());
    EXPECT_TRUE(clickhouse_client_->connect());
    token_bucket_->start();
    
    // Configure BGP
    BGPConfig bgp_config;
    bgp_config.local_asn = 65001;
    bgp_config.router_id = "192.168.1.1";
    EXPECT_TRUE(frr_client_->configure_bgp(bgp_config));
    EXPECT_TRUE(frr_client_->start_bgp());
    
    // Add interface
    InterfaceConfig interface_config;
    interface_config.name = "eth0";
    interface_config.ip = "192.168.1.1";
    interface_config.mask = "255.255.255.0";
    interface_config.mtu = 1500;
    interface_config.enabled = true;
    EXPECT_TRUE(router_core_->add_interface(interface_config));
    
    // Add route
    Route route;
    route.destination = "192.168.2.0/24";
    route.gateway = "192.168.1.2";
    route.interface = "eth0";
    route.metric = 10;
    route.protocol = "bgp";
    EXPECT_TRUE(router_core_->add_route(route));
    
    // Test packet processing
    std::vector<uint8_t> packet_data(100, 0xAA);
    EXPECT_TRUE(router_core_->process_packet(packet_data, "eth0"));
    
    // Test traffic shaping
    EXPECT_TRUE(token_bucket_->consume_tokens(1000));
    
    // Test metrics collection
    std::vector<MetricData> metrics;
    MetricData metric;
    metric.name = "packets_processed";
    metric.value = "1";
    metric.timestamp = std::chrono::system_clock::now();
    metrics.push_back(metric);
    EXPECT_TRUE(clickhouse_client_->insert_metrics(metrics));
    
    // Verify system state
    EXPECT_TRUE(router_core_->is_running());
    EXPECT_TRUE(frr_client_->is_connected());
    EXPECT_TRUE(frr_client_->is_bgp_running());
    EXPECT_TRUE(clickhouse_client_->is_connected());
    EXPECT_TRUE(token_bucket_->is_running());
}

// Performance tests
class PerformanceTest : public ::testing::Test {
protected:
    void SetUp() override {
        router_core_ = std::make_unique<RouterCore>();
        token_bucket_ = std::make_unique<TokenBucket>(TokenBucketConfig{10000000, 1000000, 1000000, std::chrono::milliseconds(1), true});
    }
    
    void TearDown() override {
        router_core_->stop();
        token_bucket_->stop();
    }
    
    std::unique_ptr<RouterCore> router_core_;
    std::unique_ptr<TokenBucket> token_bucket_;
};

TEST_F(PerformanceTest, HighThroughputPacketProcessing) {
    router_core_->start();
    token_bucket_->start();
    
    const int num_packets = 100000;
    const int packet_size = 1000;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < num_packets; ++i) {
        std::vector<uint8_t> packet_data(packet_size, 0xAA);
        router_core_->process_packet(packet_data, "eth0");
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    double packets_per_second = static_cast<double>(num_packets) / duration.count() * 1000.0;
    
    EXPECT_GT(packets_per_second, 10000.0); // Should process at least 10K packets/second
}

TEST_F(PerformanceTest, TokenBucketThroughput) {
    token_bucket_->start();
    
    const int num_operations = 1000000;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < num_operations; ++i) {
        token_bucket_->consume_tokens(100);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    double operations_per_second = static_cast<double>(num_operations) / duration.count() * 1000.0;
    
    EXPECT_GT(operations_per_second, 100000.0); // Should handle at least 100K operations/second
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}