#include <gtest/gtest.h>
#include "protocols/bgp.h"
#include "protocols/ospf.h"
#include "protocols/isis.h"
#include "protocol_interface.h"

using namespace router_sim;

class BGPProtocolTest : public ::testing::Test {
protected:
    void SetUp() override {
        bgp_protocol = std::make_unique<BGPProtocol>();
    }
    
    void TearDown() override {
        bgp_protocol->stop();
    }
    
    std::unique_ptr<BGPProtocol> bgp_protocol;
};

TEST_F(BGPProtocolTest, Initialization) {
    EXPECT_FALSE(bgp_protocol->is_running());
}

TEST_F(BGPProtocolTest, StartStop) {
    std::map<std::string, std::string> config;
    config["as_number"] = "65001";
    config["router_id"] = "1.1.1.1";
    
    EXPECT_TRUE(bgp_protocol->start(config));
    EXPECT_TRUE(bgp_protocol->is_running());
    
    EXPECT_TRUE(bgp_protocol->stop());
    EXPECT_FALSE(bgp_protocol->is_running());
}

TEST_F(BGPProtocolTest, AddRemoveNeighbor) {
    std::map<std::string, std::string> config;
    config["as_number"] = "65001";
    config["router_id"] = "1.1.1.1";
    
    EXPECT_TRUE(bgp_protocol->start(config));
    
    std::map<std::string, std::string> neighbor_config;
    neighbor_config["as_number"] = "65002";
    neighbor_config["hold_time"] = "180";
    
    EXPECT_TRUE(bgp_protocol->add_neighbor("192.168.1.1", neighbor_config));
    
    auto neighbors = bgp_protocol->get_neighbors();
    EXPECT_EQ(neighbors.size(), 1);
    EXPECT_EQ(neighbors[0].address, "192.168.1.1");
    EXPECT_EQ(neighbors[0].as_number, 65002);
    
    EXPECT_TRUE(bgp_protocol->remove_neighbor("192.168.1.1"));
    
    neighbors = bgp_protocol->get_neighbors();
    EXPECT_EQ(neighbors.size(), 0);
}

TEST_F(BGPProtocolTest, AdvertiseWithdrawRoute) {
    std::map<std::string, std::string> config;
    config["as_number"] = "65001";
    config["router_id"] = "1.1.1.1";
    
    EXPECT_TRUE(bgp_protocol->start(config));
    
    std::map<std::string, std::string> route_attrs;
    route_attrs["next_hop"] = "192.168.1.1";
    route_attrs["as_path"] = "65001";
    route_attrs["origin"] = "igp";
    route_attrs["local_pref"] = "100";
    route_attrs["med"] = "0";
    route_attrs["community"] = "65001:100";
    
    EXPECT_TRUE(bgp_protocol->advertise_route("10.0.0.0/24", route_attrs));
    
    auto routes = bgp_protocol->get_routes();
    EXPECT_EQ(routes.size(), 1);
    EXPECT_EQ(routes[0].prefix, "10.0.0.0/24");
    EXPECT_EQ(routes[0].next_hop, "192.168.1.1");
    EXPECT_EQ(routes[0].protocol, "BGP");
    
    EXPECT_TRUE(bgp_protocol->withdraw_route("10.0.0.0/24"));
    
    routes = bgp_protocol->get_routes();
    EXPECT_EQ(routes.size(), 0);
}

TEST_F(BGPProtocolTest, Statistics) {
    std::map<std::string, std::string> config;
    config["as_number"] = "65001";
    config["router_id"] = "1.1.1.1";
    
    EXPECT_TRUE(bgp_protocol->start(config));
    
    auto stats = bgp_protocol->get_statistics();
    EXPECT_TRUE(stats.find("packets_sent") != stats.end());
    EXPECT_TRUE(stats.find("packets_received") != stats.end());
    EXPECT_TRUE(stats.find("routes_advertised") != stats.end());
}

class OSPFProtocolTest : public ::testing::Test {
protected:
    void SetUp() override {
        ospf_protocol = std::make_unique<OSPFProtocol>();
    }
    
    void TearDown() override {
        ospf_protocol->stop();
    }
    
    std::unique_ptr<OSPFProtocol> ospf_protocol;
};

TEST_F(OSPFProtocolTest, Initialization) {
    EXPECT_FALSE(ospf_protocol->is_running());
}

TEST_F(OSPFProtocolTest, StartStop) {
    std::map<std::string, std::string> config;
    config["router_id"] = "1.1.1.1";
    config["area_id"] = "0.0.0.0";
    
    EXPECT_TRUE(ospf_protocol->start(config));
    EXPECT_TRUE(ospf_protocol->is_running());
    
    EXPECT_TRUE(ospf_protocol->stop());
    EXPECT_FALSE(ospf_protocol->is_running());
}

TEST_F(OSPFProtocolTest, AddRemoveInterface) {
    std::map<std::string, std::string> config;
    config["router_id"] = "1.1.1.1";
    config["area_id"] = "0.0.0.0";
    
    EXPECT_TRUE(ospf_protocol->start(config));
    
    std::map<std::string, std::string> interface_config;
    interface_config["area_id"] = "0.0.0.0";
    interface_config["cost"] = "10";
    interface_config["priority"] = "1";
    
    EXPECT_TRUE(ospf_protocol->add_interface("eth0", interface_config));
    
    auto neighbors = ospf_protocol->get_neighbors();
    // OSPF neighbors are discovered, not manually added
    
    EXPECT_TRUE(ospf_protocol->remove_interface("eth0"));
}

TEST_F(OSPFProtocolTest, AdvertiseWithdrawRoute) {
    std::map<std::string, std::string> config;
    config["router_id"] = "1.1.1.1";
    config["area_id"] = "0.0.0.0";
    
    EXPECT_TRUE(ospf_protocol->start(config));
    
    std::map<std::string, std::string> route_attrs;
    route_attrs["type"] = "intra-area";
    route_attrs["cost"] = "10";
    route_attrs["next_hop"] = "192.168.1.1";
    
    EXPECT_TRUE(ospf_protocol->advertise_route("10.0.0.0/24", route_attrs));
    
    auto routes = ospf_protocol->get_routes();
    EXPECT_EQ(routes.size(), 1);
    EXPECT_EQ(routes[0].prefix, "10.0.0.0/24");
    EXPECT_EQ(routes[0].next_hop, "192.168.1.1");
    EXPECT_EQ(routes[0].protocol, "OSPF");
    
    EXPECT_TRUE(ospf_protocol->withdraw_route("10.0.0.0/24"));
    
    routes = ospf_protocol->get_routes();
    EXPECT_EQ(routes.size(), 0);
}

class ISISProtocolTest : public ::testing::Test {
protected:
    void SetUp() override {
        isis_protocol = std::make_unique<ISISProtocol>();
    }
    
    void TearDown() override {
        isis_protocol->stop();
    }
    
    std::unique_ptr<ISISProtocol> isis_protocol;
};

TEST_F(ISISProtocolTest, Initialization) {
    EXPECT_FALSE(isis_protocol->is_running());
}

TEST_F(ISISProtocolTest, StartStop) {
    std::map<std::string, std::string> config;
    config["system_id"] = "0000.0000.0001";
    config["area_id"] = "49.0001";
    
    EXPECT_TRUE(isis_protocol->start(config));
    EXPECT_TRUE(isis_protocol->is_running());
    
    EXPECT_TRUE(isis_protocol->stop());
    EXPECT_FALSE(isis_protocol->is_running());
}

TEST_F(ISISProtocolTest, AddRemoveInterface) {
    std::map<std::string, std::string> config;
    config["system_id"] = "0000.0000.0001";
    config["area_id"] = "49.0001";
    
    EXPECT_TRUE(isis_protocol->start(config));
    
    std::map<std::string, std::string> interface_config;
    interface_config["area_id"] = "49.0001";
    interface_config["cost"] = "10";
    interface_config["priority"] = "64";
    
    EXPECT_TRUE(isis_protocol->add_interface("eth0", interface_config));
    
    auto neighbors = isis_protocol->get_neighbors();
    // IS-IS neighbors are discovered, not manually added
    
    EXPECT_TRUE(isis_protocol->remove_interface("eth0"));
}

TEST_F(ISISProtocolTest, AdvertiseWithdrawRoute) {
    std::map<std::string, std::string> config;
    config["system_id"] = "0000.0000.0001";
    config["area_id"] = "49.0001";
    
    EXPECT_TRUE(isis_protocol->start(config));
    
    std::map<std::string, std::string> route_attrs;
    route_attrs["type"] = "internal";
    route_attrs["cost"] = "10";
    route_attrs["next_hop"] = "192.168.1.1";
    
    EXPECT_TRUE(isis_protocol->advertise_route("10.0.0.0/24", route_attrs));
    
    auto routes = isis_protocol->get_routes();
    EXPECT_EQ(routes.size(), 1);
    EXPECT_EQ(routes[0].prefix, "10.0.0.0/24");
    EXPECT_EQ(routes[0].next_hop, "192.168.1.1");
    EXPECT_EQ(routes[0].protocol, "IS-IS");
    
    EXPECT_TRUE(isis_protocol->withdraw_route("10.0.0.0/24"));
    
    routes = isis_protocol->get_routes();
    EXPECT_EQ(routes.size(), 0);
}

// Integration tests
class ProtocolIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        bgp_protocol = std::make_unique<BGPProtocol>();
        ospf_protocol = std::make_unique<OSPFProtocol>();
        isis_protocol = std::make_unique<ISISProtocol>();
    }
    
    void TearDown() override {
        bgp_protocol->stop();
        ospf_protocol->stop();
        isis_protocol->stop();
    }
    
    std::unique_ptr<BGPProtocol> bgp_protocol;
    std::unique_ptr<OSPFProtocol> ospf_protocol;
    std::unique_ptr<ISISProtocol> isis_protocol;
};

TEST_F(ProtocolIntegrationTest, MultipleProtocols) {
    // Start all protocols
    std::map<std::string, std::string> bgp_config;
    bgp_config["as_number"] = "65001";
    bgp_config["router_id"] = "1.1.1.1";
    
    std::map<std::string, std::string> ospf_config;
    ospf_config["router_id"] = "1.1.1.1";
    ospf_config["area_id"] = "0.0.0.0";
    
    std::map<std::string, std::string> isis_config;
    isis_config["system_id"] = "0000.0000.0001";
    isis_config["area_id"] = "49.0001";
    
    EXPECT_TRUE(bgp_protocol->start(bgp_config));
    EXPECT_TRUE(ospf_protocol->start(ospf_config));
    EXPECT_TRUE(isis_protocol->start(isis_config));
    
    // All protocols should be running
    EXPECT_TRUE(bgp_protocol->is_running());
    EXPECT_TRUE(ospf_protocol->is_running());
    EXPECT_TRUE(isis_protocol->is_running());
    
    // Add routes to each protocol
    std::map<std::string, std::string> bgp_route;
    bgp_route["next_hop"] = "192.168.1.1";
    bgp_route["as_path"] = "65001";
    bgp_route["origin"] = "igp";
    bgp_route["local_pref"] = "100";
    bgp_route["med"] = "0";
    bgp_route["community"] = "65001:100";
    
    std::map<std::string, std::string> ospf_route;
    ospf_route["type"] = "intra-area";
    ospf_route["cost"] = "10";
    ospf_route["next_hop"] = "192.168.1.2";
    
    std::map<std::string, std::string> isis_route;
    isis_route["type"] = "internal";
    isis_route["cost"] = "10";
    isis_route["next_hop"] = "192.168.1.3";
    
    EXPECT_TRUE(bgp_protocol->advertise_route("10.0.0.0/24", bgp_route));
    EXPECT_TRUE(ospf_protocol->advertise_route("10.0.1.0/24", ospf_route));
    EXPECT_TRUE(isis_protocol->advertise_route("10.0.2.0/24", isis_route));
    
    // Check routes
    auto bgp_routes = bgp_protocol->get_routes();
    auto ospf_routes = ospf_protocol->get_routes();
    auto isis_routes = isis_protocol->get_routes();
    
    EXPECT_EQ(bgp_routes.size(), 1);
    EXPECT_EQ(ospf_routes.size(), 1);
    EXPECT_EQ(isis_routes.size(), 1);
    
    EXPECT_EQ(bgp_routes[0].protocol, "BGP");
    EXPECT_EQ(ospf_routes[0].protocol, "OSPF");
    EXPECT_EQ(isis_routes[0].protocol, "IS-IS");
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
