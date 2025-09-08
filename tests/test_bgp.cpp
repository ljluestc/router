#include "testing_framework.h"
#include "frr_integration.h"
#include <gtest/gtest.h>

using namespace router_sim;
using namespace router_sim::testing;

class BGPConvergenceTest : public RouterTestCase {
public:
    void SetUp() override {
        RouterTestCase::SetUp();
        config_.test_name = "BGPConvergenceTest";
        config_.description = "Test BGP route convergence";
        config_.enable_pcap_capture = true;
        config_.timeout = std::chrono::seconds(60);
        set_test_config(config_);
    }
    
    bool run_test() override {
        return test_bgp_route_advertisement();
    }
    
    bool validate_results() override {
        return statistics_.packets_captured > 0;
    }

private:
    bool test_bgp_route_advertisement() {
        auto control_plane = std::make_shared<FRRControlPlane>();
        if (!control_plane->initialize(FRRConfig{})) {
            return false;
        }
        
        auto bgp = std::make_unique<FRRBGP>(control_plane);
        std::map<std::string, std::string> config;
        config["as_number"] = "65001";
        config["router_id"] = "1.1.1.1";
        
        if (!bgp->initialize(config)) {
            return false;
        }
        
        if (!bgp->start()) {
            return false;
        }
        
        // Add neighbor
        std::map<std::string, std::string> neighbor_config;
        neighbor_config["remote_as"] = "65002";
        
        if (!bgp->add_neighbor("192.168.1.2", neighbor_config)) {
            return false;
        }
        
        // Advertise route
        RouteInfo route;
        route.destination = "10.0.0.0";
        route.prefix_length = 24;
        route.next_hop = "192.168.1.1";
        route.protocol = "bgp";
        
        return bgp->advertise_route(route);
    }
};

class RouterTestFixture : public ::testing::Test {
protected:
    void SetUp() override {
        test_manager_ = std::make_unique<TestSuiteManager>();
    }
    
    void TearDown() override {
        test_manager_.reset();
    }
    
    std::unique_ptr<TestSuiteManager> test_manager_;
};

TEST_F(RouterTestFixture, BGPConvergenceTest) {
    auto test = std::make_shared<BGPConvergenceTest>();
    test_manager_->add_test_case(test);
    EXPECT_TRUE(test_manager_->run_test("BGPConvergenceTest"));
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
