#include <gtest/gtest.h>
#include "router_core.h"
#include <thread>
#include <chrono>

using namespace RouterSim;

class RouterCoreTest : public ::testing::Test {
protected:
    void SetUp() override {
        router = std::make_unique<RouterCore>();
        ASSERT_TRUE(router->initialize());
    }
    
    void TearDown() override {
        if (router && router->is_running()) {
            router->stop();
        }
    }
    
    std::unique_ptr<RouterCore> router;
};

TEST_F(RouterCoreTest, Initialization) {
    EXPECT_TRUE(router->initialize());
    EXPECT_FALSE(router->is_running());
}

TEST_F(RouterCoreTest, StartStop) {
    router->start();
    EXPECT_TRUE(router->is_running());
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    router->stop();
    EXPECT_FALSE(router->is_running());
}

TEST_F(RouterCoreTest, InterfaceManagement) {
    // Add interface
    EXPECT_TRUE(router->add_interface("eth0", "192.168.1.1", "255.255.255.0"));
    
    // Check interface exists
    auto interfaces = router->get_interfaces();
    EXPECT_EQ(interfaces.size(), 2); // lo + eth0
    
    // Find our interface
    bool found = false;
    for (const auto& iface : interfaces) {
        if (iface.name == "eth0") {
            EXPECT_EQ(iface.ip_address, "192.168.1.1");
            EXPECT_EQ(iface.subnet_mask, "255.255.255.0");
            EXPECT_FALSE(iface.is_up);
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found);
    
    // Set interface up
    EXPECT_TRUE(router->set_interface_up("eth0", true));
    
    // Remove interface
    EXPECT_TRUE(router->remove_interface("eth0"));
    
    interfaces = router->get_interfaces();
    EXPECT_EQ(interfaces.size(), 1); // Only lo left
}

TEST_F(RouterCoreTest, RouteManagement) {
    Route route1;
    route1.network = "192.168.1.0/24";
    route1.next_hop = "192.168.1.1";
    route1.interface = "eth0";
    route1.metric = 1;
    route1.protocol = "static";
    route1.is_active = true;
    
    // Add route
    EXPECT_TRUE(router->add_route(route1));
    
    // Check route exists
    auto routes = router->get_routes();
    EXPECT_EQ(routes.size(), 1);
    EXPECT_EQ(routes[0].network, "192.168.1.0/24");
    
    // Find route
    Route* found_route = router->find_route("192.168.1.10");
    EXPECT_NE(found_route, nullptr);
    EXPECT_EQ(found_route->network, "192.168.1.0/24");
    
    // Remove route
    EXPECT_TRUE(router->remove_route("192.168.1.0/24"));
    
    routes = router->get_routes();
    EXPECT_EQ(routes.size(), 0);
}

TEST_F(RouterCoreTest, PacketProcessing) {
    router->start();
    
    Packet packet;
    packet.data = {0x45, 0x00, 0x00, 0x14, 0x00, 0x00, 0x40, 0x00};
    packet.source_interface = "eth0";
    packet.dest_interface = "eth1";
    packet.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    packet.size = packet.data.size();
    
    // Process packet
    router->process_packet(packet);
    
    // Check statistics
    auto stats = router->get_statistics();
    EXPECT_EQ(stats.total_packets_processed, 1);
    EXPECT_EQ(stats.total_bytes_processed, packet.size);
    
    router->stop();
}

TEST_F(RouterCoreTest, Statistics) {
    auto stats = router->get_statistics();
    EXPECT_EQ(stats.total_packets_processed, 0);
    EXPECT_EQ(stats.total_bytes_processed, 0);
    EXPECT_EQ(stats.routing_table_updates, 0);
    EXPECT_EQ(stats.interface_state_changes, 0);
}
