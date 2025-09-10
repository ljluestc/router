#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "router_core.h"
#include "frr_integration.h"
#include "traffic_shaping.h"
#include "network_impairments.h"
#include <memory>
#include <thread>
#include <chrono>

using namespace RouterSim;
using ::testing::_;
using ::testing::Return;
using ::testing::StrictMock;

class MockFRRIntegration : public FRRIntegration {
public:
    MOCK_METHOD(bool, initialize, (const FRRConfig& config), (override));
    MOCK_METHOD(bool, start, (), (override));
    MOCK_METHOD(bool, stop, (), (override));
    MOCK_METHOD(bool, is_running, (), (const, override));
    MOCK_METHOD(std::vector<RouteInfo>, get_routes, (), (const, override));
    MOCK_METHOD(std::vector<NeighborInfo>, get_neighbors, (), (const, override));
    MOCK_METHOD(std::map<std::string, uint64_t>, get_statistics, (const std::string& protocol), (const, override));
};

class MockTrafficShaper : public TrafficShaper {
public:
    MOCK_METHOD(bool, initialize, (), (override));
    MOCK_METHOD(bool, processPacket, (const Packet& packet), (override));
    MOCK_METHOD(bool, dequeuePacket, (Packet& packet), (override));
    MOCK_METHOD(void, setEnabled, (bool enabled), (override));
    MOCK_METHOD(bool, isEnabled, (), (const, override));
    MOCK_METHOD(TrafficShaper::Statistics, getStatistics, (), (const, override));
    MOCK_METHOD(void, reset, (), (override));
};

class MockNetworkImpairmentManager : public NetworkImpairmentManager {
public:
    MOCK_METHOD(bool, initialize, (), (override));
    MOCK_METHOD(bool, addImpairment, (const ImpairmentConfig& config), (override));
    MOCK_METHOD(bool, removeImpairment, (const std::string& interface, const std::string& type), (override));
    MOCK_METHOD(bool, removeAllImpairments, (), (override));
    MOCK_METHOD(std::vector<ImpairmentConfig>, getActiveImpairments, (), (const, override));
    MOCK_METHOD(std::vector<ImpairmentStatistics>, getStatistics, (), (const, override));
    MOCK_METHOD(void, reset, (), (override));
};

class RouterCoreTest : public ::testing::Test {
protected:
    void SetUp() override {
        router_core_ = std::make_unique<RouterCore>();
        mock_frr_ = std::make_unique<StrictMock<MockFRRIntegration>>();
        mock_traffic_shaper_ = std::make_unique<StrictMock<MockTrafficShaper>>();
        mock_impairment_manager_ = std::make_unique<StrictMock<MockNetworkImpairmentManager>>();
    }

    void TearDown() override {
        if (router_core_) {
            router_core_->stop();
        }
    }

    std::unique_ptr<RouterCore> router_core_;
    std::unique_ptr<StrictMock<MockFRRIntegration>> mock_frr_;
    std::unique_ptr<StrictMock<MockTrafficShaper>> mock_traffic_shaper_;
    std::unique_ptr<StrictMock<MockNetworkImpairmentManager>> mock_impairment_manager_;
};

// Test router core initialization
TEST_F(RouterCoreTest, Initialization) {
    EXPECT_TRUE(router_core_->initialize());
    EXPECT_FALSE(router_core_->is_running());
}

// Test router core start/stop
TEST_F(RouterCoreTest, StartStop) {
    EXPECT_TRUE(router_core_->initialize());
    EXPECT_TRUE(router_core_->start());
    EXPECT_TRUE(router_core_->is_running());
    EXPECT_TRUE(router_core_->stop());
    EXPECT_FALSE(router_core_->is_running());
}

// Test protocol management
TEST_F(RouterCoreTest, ProtocolManagement) {
    EXPECT_TRUE(router_core_->initialize());
    EXPECT_TRUE(router_core_->start());
    
    // Test BGP protocol
    EXPECT_TRUE(router_core_->start_protocol("bgp"));
    EXPECT_TRUE(router_core_->is_protocol_running("bgp"));
    EXPECT_TRUE(router_core_->stop_protocol("bgp"));
    EXPECT_FALSE(router_core_->is_protocol_running("bgp"));
    
    // Test OSPF protocol
    EXPECT_TRUE(router_core_->start_protocol("ospf"));
    EXPECT_TRUE(router_core_->is_protocol_running("ospf"));
    EXPECT_TRUE(router_core_->stop_protocol("ospf"));
    EXPECT_FALSE(router_core_->is_protocol_running("ospf"));
    
    // Test IS-IS protocol
    EXPECT_TRUE(router_core_->start_protocol("isis"));
    EXPECT_TRUE(router_core_->is_protocol_running("isis"));
    EXPECT_TRUE(router_core_->stop_protocol("isis"));
    EXPECT_FALSE(router_core_->is_protocol_running("isis"));
}

// Test traffic shaping
TEST_F(RouterCoreTest, TrafficShaping) {
    EXPECT_TRUE(router_core_->initialize());
    EXPECT_TRUE(router_core_->start());
    
    // Test enabling traffic shaping
    EXPECT_TRUE(router_core_->enable_traffic_shaping());
    EXPECT_TRUE(router_core_->is_traffic_shaping_enabled());
    
    // Test disabling traffic shaping
    EXPECT_TRUE(router_core_->disable_traffic_shaping());
    EXPECT_FALSE(router_core_->is_traffic_shaping_enabled());
}

// Test network impairments
TEST_F(RouterCoreTest, NetworkImpairments) {
    EXPECT_TRUE(router_core_->initialize());
    EXPECT_TRUE(router_core_->start());
    
    // Test adding impairment
    ImpairmentConfig config;
    config.interface = "eth0";
    config.type = "packet_loss";
    config.parameters["percentage"] = "0.1";
    config.enabled = true;
    
    EXPECT_TRUE(router_core_->add_impairment(config));
    EXPECT_TRUE(router_core_->has_active_impairments());
    
    // Test removing impairment
    EXPECT_TRUE(router_core_->remove_impairment("eth0", "packet_loss"));
    EXPECT_FALSE(router_core_->has_active_impairments());
}

// Test route management
TEST_F(RouterCoreTest, RouteManagement) {
    EXPECT_TRUE(router_core_->initialize());
    EXPECT_TRUE(router_core_->start());
    
    // Test adding route
    RouteInfo route;
    route.prefix = "192.168.1.0/24";
    route.next_hop = "192.168.1.1";
    route.metric = 100;
    route.protocol = "BGP";
    
    EXPECT_TRUE(router_core_->add_route(route));
    
    // Test getting routes
    auto routes = router_core_->get_routes();
    EXPECT_EQ(routes.size(), 1);
    EXPECT_EQ(routes[0].prefix, "192.168.1.0/24");
    EXPECT_EQ(routes[0].next_hop, "192.168.1.1");
    EXPECT_EQ(routes[0].metric, 100);
    EXPECT_EQ(routes[0].protocol, "BGP");
    
    // Test removing route
    EXPECT_TRUE(router_core_->remove_route("192.168.1.0/24"));
    routes = router_core_->get_routes();
    EXPECT_EQ(routes.size(), 0);
}

// Test neighbor management
TEST_F(RouterCoreTest, NeighborManagement) {
    EXPECT_TRUE(router_core_->initialize());
    EXPECT_TRUE(router_core_->start());
    
    // Test adding neighbor
    NeighborInfo neighbor;
    neighbor.address = "192.168.1.2";
    neighbor.state = "Established";
    neighbor.protocol = "BGP";
    
    EXPECT_TRUE(router_core_->add_neighbor(neighbor));
    
    // Test getting neighbors
    auto neighbors = router_core_->get_neighbors();
    EXPECT_EQ(neighbors.size(), 1);
    EXPECT_EQ(neighbors[0].address, "192.168.1.2");
    EXPECT_EQ(neighbors[0].state, "Established");
    EXPECT_EQ(neighbors[0].protocol, "BGP");
    
    // Test removing neighbor
    EXPECT_TRUE(router_core_->remove_neighbor("192.168.1.2"));
    neighbors = router_core_->get_neighbors();
    EXPECT_EQ(neighbors.size(), 0);
}

// Test statistics
TEST_F(RouterCoreTest, Statistics) {
    EXPECT_TRUE(router_core_->initialize());
    EXPECT_TRUE(router_core_->start());
    
    auto stats = router_core_->get_statistics();
    EXPECT_GE(stats.packets_processed, 0);
    EXPECT_GE(stats.bytes_processed, 0);
    EXPECT_GE(stats.packets_dropped, 0);
    EXPECT_GE(stats.bytes_dropped, 0);
}

// Test scenario loading
TEST_F(RouterCoreTest, ScenarioLoading) {
    EXPECT_TRUE(router_core_->initialize());
    EXPECT_TRUE(router_core_->start());
    
    // Test loading scenario
    EXPECT_TRUE(router_core_->load_scenario("scenarios/cloud_networking_demo.yaml"));
    
    // Test saving scenario
    EXPECT_TRUE(router_core_->save_scenario("test_scenario.yaml"));
}

// Test packet processing
TEST_F(RouterCoreTest, PacketProcessing) {
    EXPECT_TRUE(router_core_->initialize());
    EXPECT_TRUE(router_core_->start());
    
    // Create test packet
    Packet packet;
    packet.size = 1500;
    packet.priority = 1;
    packet.flow_id = 1;
    packet.timestamp = std::chrono::steady_clock::now();
    
    // Test packet processing
    EXPECT_TRUE(router_core_->process_packet(packet));
}

// Test error handling
TEST_F(RouterCoreTest, ErrorHandling) {
    // Test starting without initialization
    EXPECT_FALSE(router_core_->start());
    
    // Test operations on stopped router
    EXPECT_FALSE(router_core_->is_running());
    EXPECT_FALSE(router_core_->is_protocol_running("bgp"));
    EXPECT_FALSE(router_core_->is_traffic_shaping_enabled());
    EXPECT_FALSE(router_core_->has_active_impairments());
}

// Test concurrent operations
TEST_F(RouterCoreTest, ConcurrentOperations) {
    EXPECT_TRUE(router_core_->initialize());
    EXPECT_TRUE(router_core_->start());
    
    // Start multiple protocols concurrently
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([this]() {
            router_core_->start_protocol("bgp");
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            router_core_->stop_protocol("bgp");
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    EXPECT_FALSE(router_core_->is_protocol_running("bgp"));
}

// Test performance
TEST_F(RouterCoreTest, Performance) {
    EXPECT_TRUE(router_core_->initialize());
    EXPECT_TRUE(router_core_->start());
    
    const int num_packets = 10000;
    const auto start_time = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < num_packets; ++i) {
        Packet packet;
        packet.size = 1500;
        packet.priority = i % 8;
        packet.flow_id = i % 100;
        packet.timestamp = std::chrono::steady_clock::now();
        
        router_core_->process_packet(packet);
    }
    
    const auto end_time = std::chrono::high_resolution_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    
    const double packets_per_second = static_cast<double>(num_packets) / duration.count() * 1000000;
    
    // Expect at least 100,000 packets per second
    EXPECT_GT(packets_per_second, 100000.0);
}

// Test memory usage
TEST_F(RouterCoreTest, MemoryUsage) {
    EXPECT_TRUE(router_core_->initialize());
    EXPECT_TRUE(router_core_->start());
    
    // Add many routes and neighbors
    for (int i = 0; i < 1000; ++i) {
        RouteInfo route;
        route.prefix = "192.168." + std::to_string(i / 256) + "." + std::to_string(i % 256) + "/24";
        route.next_hop = "192.168.1.1";
        route.metric = i;
        route.protocol = "BGP";
        
        router_core_->add_route(route);
        
        NeighborInfo neighbor;
        neighbor.address = "192.168.1." + std::to_string(i % 254 + 2);
        neighbor.state = "Established";
        neighbor.protocol = "BGP";
        
        router_core_->add_neighbor(neighbor);
    }
    
    auto routes = router_core_->get_routes();
    auto neighbors = router_core_->get_neighbors();
    
    EXPECT_EQ(routes.size(), 1000);
    EXPECT_EQ(neighbors.size(), 1000);
}

// Test configuration validation
TEST_F(RouterCoreTest, ConfigurationValidation) {
    EXPECT_TRUE(router_core_->initialize());
    
    // Test invalid protocol
    EXPECT_FALSE(router_core_->start_protocol("invalid"));
    
    // Test invalid impairment
    ImpairmentConfig config;
    config.interface = "";
    config.type = "invalid";
    config.enabled = true;
    
    EXPECT_FALSE(router_core_->add_impairment(config));
}

// Test cleanup
TEST_F(RouterCoreTest, Cleanup) {
    EXPECT_TRUE(router_core_->initialize());
    EXPECT_TRUE(router_core_->start());
    
    // Add some data
    RouteInfo route;
    route.prefix = "192.168.1.0/24";
    route.next_hop = "192.168.1.1";
    route.metric = 100;
    route.protocol = "BGP";
    
    router_core_->add_route(route);
    
    // Test cleanup
    EXPECT_TRUE(router_core_->clear_all_routes());
    auto routes = router_core_->get_routes();
    EXPECT_EQ(routes.size(), 0);
    
    EXPECT_TRUE(router_core_->clear_all_neighbors());
    auto neighbors = router_core_->get_neighbors();
    EXPECT_EQ(neighbors.size(), 0);
    
    EXPECT_TRUE(router_core_->clear_all_impairments());
    EXPECT_FALSE(router_core_->has_active_impairments());
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}