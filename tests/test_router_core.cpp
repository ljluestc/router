#include <gtest/gtest.h>
#include "router_core.h"
#include "protocols/bgp.h"
#include "protocols/ospf.h"
#include "protocols/isis.h"
#include "traffic_shaping/traffic_shaper.h"

using namespace router_sim;

class RouterCoreTest : public ::testing::Test {
protected:
    void SetUp() override {
        router_ = std::make_unique<RouterCore>();
    }

    void TearDown() override {
        if (router_) {
            router_->stop();
        }
    }

    std::unique_ptr<RouterCore> router_;
};

TEST_F(RouterCoreTest, Initialize) {
    EXPECT_TRUE(router_->initialize());
}

TEST_F(RouterCoreTest, StartStop) {
    ASSERT_TRUE(router_->initialize());
    EXPECT_TRUE(router_->start());
    EXPECT_TRUE(router_->is_running());
    router_->stop();
    EXPECT_FALSE(router_->is_running());
}

TEST_F(RouterCoreTest, AddRemoveInterface) {
    ASSERT_TRUE(router_->initialize());
    ASSERT_TRUE(router_->start());

    // Add interface
    EXPECT_TRUE(router_->add_interface("eth0", "192.168.1.1", "255.255.255.0"));
    
    // Check interface exists
    auto interfaces = router_->get_interfaces();
    EXPECT_EQ(interfaces.size(), 1);
    EXPECT_EQ(interfaces[0].name, "eth0");
    EXPECT_EQ(interfaces[0].ip_address, "192.168.1.1");

    // Remove interface
    EXPECT_TRUE(router_->remove_interface("eth0"));
    
    // Check interface is removed
    interfaces = router_->get_interfaces();
    EXPECT_EQ(interfaces.size(), 0);
}

TEST_F(RouterCoreTest, AddRemoveRoute) {
    ASSERT_TRUE(router_->initialize());
    ASSERT_TRUE(router_->start());

    // Add route
    Route route;
    route.destination = "10.0.0.0";
    route.prefix_length = 8;
    route.next_hop = "192.168.1.1";
    route.interface = "eth0";
    route.metric = 1;
    route.protocol = "static";
    route.is_active = true;

    EXPECT_TRUE(router_->add_route(route));
    
    // Check route exists
    auto routes = router_->get_routes();
    EXPECT_EQ(routes.size(), 1);
    EXPECT_EQ(routes[0].destination, "10.0.0.0");

    // Remove route
    EXPECT_TRUE(router_->remove_route("10.0.0.0", 8));
    
    // Check route is removed
    routes = router_->get_routes();
    EXPECT_EQ(routes.size(), 0);
}

TEST_F(RouterCoreTest, EnableDisableProtocol) {
    ASSERT_TRUE(router_->initialize());
    ASSERT_TRUE(router_->start());

    // Enable BGP
    std::map<std::string, std::string> bgp_config;
    bgp_config["local_as"] = "65001";
    bgp_config["router_id"] = "1.1.1.1";
    
    EXPECT_TRUE(router_->enable_protocol(ProtocolType::BGP, bgp_config));
    
    // Check protocol is active
    auto protocols = router_->get_active_protocols();
    EXPECT_EQ(protocols.size(), 1);
    EXPECT_EQ(protocols[0], ProtocolType::BGP);

    // Disable BGP
    EXPECT_TRUE(router_->disable_protocol(ProtocolType::BGP));
    
    // Check protocol is disabled
    protocols = router_->get_active_protocols();
    EXPECT_EQ(protocols.size(), 0);
}

TEST_F(RouterCoreTest, InterfaceState) {
    ASSERT_TRUE(router_->initialize());
    ASSERT_TRUE(router_->start());

    // Add interface
    EXPECT_TRUE(router_->add_interface("eth0", "192.168.1.1", "255.255.255.0"));
    
    // Check initial state
    auto interfaces = router_->get_interfaces();
    EXPECT_FALSE(interfaces[0].is_up);

    // Bring interface up
    EXPECT_TRUE(router_->set_interface_state("eth0", true));
    
    // Check interface is up
    interfaces = router_->get_interfaces();
    EXPECT_TRUE(interfaces[0].is_up);

    // Bring interface down
    EXPECT_TRUE(router_->set_interface_state("eth0", false));
    
    // Check interface is down
    interfaces = router_->get_interfaces();
    EXPECT_FALSE(interfaces[0].is_up);
}

TEST_F(RouterCoreTest, Statistics) {
    ASSERT_TRUE(router_->initialize());
    ASSERT_TRUE(router_->start());

    // Add some interfaces and routes
    router_->add_interface("eth0", "192.168.1.1", "255.255.255.0");
    router_->add_interface("eth1", "192.168.2.1", "255.255.255.0");
    
    Route route;
    route.destination = "10.0.0.0";
    route.prefix_length = 8;
    route.next_hop = "192.168.1.1";
    route.interface = "eth0";
    route.metric = 1;
    route.protocol = "static";
    route.is_active = true;
    router_->add_route(route);

    // Get statistics
    auto stats = router_->get_statistics();
    EXPECT_EQ(stats.interface_stats.size(), 2);
    EXPECT_TRUE(stats.interface_stats.find("eth0") != stats.interface_stats.end());
    EXPECT_TRUE(stats.interface_stats.find("eth1") != stats.interface_stats.end());
}

class BGPProtocolTest : public ::testing::Test {
protected:
    void SetUp() override {
        bgp_ = std::make_unique<BGPProtocol>();
    }

    void TearDown() override {
        if (bgp_) {
            bgp_->stop();
        }
    }

    std::unique_ptr<BGPProtocol> bgp_;
};

TEST_F(BGPProtocolTest, Initialize) {
    ProtocolConfig config;
    config.parameters["local_as"] = "65001";
    config.parameters["router_id"] = "1.1.1.1";
    config.enabled = true;
    config.update_interval_ms = 1000;

    EXPECT_TRUE(bgp_->initialize(config));
}

TEST_F(BGPProtocolTest, AddRemoveNeighbor) {
    ProtocolConfig config;
    config.parameters["local_as"] = "65001";
    config.parameters["router_id"] = "1.1.1.1";
    config.enabled = true;
    config.update_interval_ms = 1000;

    ASSERT_TRUE(bgp_->initialize(config));
    ASSERT_TRUE(bgp_->start());

    // Add neighbor
    EXPECT_TRUE(bgp_->add_neighbor("192.168.1.2", 65002));
    
    // Check neighbor exists
    auto neighbors = bgp_->get_neighbors();
    EXPECT_EQ(neighbors.size(), 1);
    EXPECT_EQ(neighbors[0].address, "192.168.1.2");
    EXPECT_EQ(neighbors[0].as_number, 65002);

    // Remove neighbor
    EXPECT_TRUE(bgp_->remove_neighbor("192.168.1.2"));
    
    // Check neighbor is removed
    neighbors = bgp_->get_neighbors();
    EXPECT_EQ(neighbors.size(), 0);
}

TEST_F(BGPProtocolTest, AdvertiseWithdrawRoute) {
    ProtocolConfig config;
    config.parameters["local_as"] = "65001";
    config.parameters["router_id"] = "1.1.1.1";
    config.enabled = true;
    config.update_interval_ms = 1000;

    ASSERT_TRUE(bgp_->initialize(config));
    ASSERT_TRUE(bgp_->start());

    // Advertise route
    EXPECT_TRUE(bgp_->advertise_route("10.0.0.0", 8, 100));
    
    // Check route is advertised
    auto routes = bgp_->get_advertised_routes();
    EXPECT_EQ(routes.size(), 1);
    EXPECT_EQ(routes[0], "10.0.0.0/8");

    // Withdraw route
    EXPECT_TRUE(bgp_->withdraw_route("10.0.0.0", 8));
    
    // Check route is withdrawn
    routes = bgp_->get_advertised_routes();
    EXPECT_EQ(routes.size(), 0);
}

class TrafficShaperTest : public ::testing::Test {
protected:
    void SetUp() override {
        shaper_ = TrafficShaperFactory::create("token_bucket");
    }

    void TearDown() override {
        if (shaper_) {
            shaper_->stop();
        }
    }

    std::unique_ptr<TrafficShaper> shaper_;
};

TEST_F(TrafficShaperTest, Initialize) {
    TrafficShapingConfig config;
    config.algorithm = "token_bucket";
    config.rate_bps = 1000000; // 1 Mbps
    config.burst_size = 10000; // 10 KB
    config.queue_size = 100;

    EXPECT_TRUE(shaper_->initialize(config));
}

TEST_F(TrafficShaperTest, ShapePacket) {
    TrafficShapingConfig config;
    config.algorithm = "token_bucket";
    config.rate_bps = 1000000; // 1 Mbps
    config.burst_size = 10000; // 10 KB
    config.queue_size = 100;

    ASSERT_TRUE(shaper_->initialize(config));
    ASSERT_TRUE(shaper_->start());

    // Create test packet
    std::vector<uint8_t> packet(1000, 0xAA);
    
    // Shape packet
    EXPECT_TRUE(shaper_->shape_packet("eth0", packet));
    
    // Check queue size
    EXPECT_GT(shaper_->get_queue_size("eth0"), 0);
}

TEST_F(TrafficShaperTest, SetRate) {
    TrafficShapingConfig config;
    config.algorithm = "token_bucket";
    config.rate_bps = 1000000; // 1 Mbps
    config.burst_size = 10000; // 10 KB
    config.queue_size = 100;

    ASSERT_TRUE(shaper_->initialize(config));
    ASSERT_TRUE(shaper_->start());

    // Set new rate
    EXPECT_TRUE(shaper_->set_rate("eth0", 2000000)); // 2 Mbps
    
    // Check rate is set
    EXPECT_EQ(shaper_->get_current_rate("eth0"), 2000000);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
