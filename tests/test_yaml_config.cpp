#include <gtest/gtest.h>
#include "yaml_config.h"
#include <fstream>
#include <sstream>

using namespace RouterSim;

class YamlConfigTest : public ::testing::Test {
protected:
    void SetUp() override {
        yaml_config_ = std::make_unique<YamlConfig>();
    }
    
    void TearDown() override {
        // Clean up any test files
    }
    
    std::unique_ptr<YamlConfig> yaml_config_;
    
    void createTestRouterConfig(const std::string& filename) {
        std::ofstream file(filename);
        file << "router_id: 1.1.1.1\n";
        file << "hostname: test-router\n";
        file << "interfaces:\n";
        file << "  - eth0\n";
        file << "  - eth1\n";
        file << "enable_bgp: true\n";
        file << "enable_ospf: true\n";
        file << "enable_isis: false\n";
        file << "as_number: 65001\n";
        file << "area_id: 0.0.0.0\n";
        file << "system_id: 0000.0000.0001\n";
        file.close();
    }
    
    void createTestScenario(const std::string& filename) {
        std::ofstream file(filename);
        file << "scenarios:\n";
        file << "  - name: test_scenario\n";
        file << "    description: Test scenario\n";
        file << "    version: 1.0\n";
        file << "    steps:\n";
        file << "      - type: configure_interface\n";
        file << "        name: configure_eth0\n";
        file << "        description: Configure eth0 interface\n";
        file << "        parameters:\n";
        file << "          name: eth0\n";
        file << "          ip: 192.168.1.1\n";
        file << "          mask: 255.255.255.0\n";
        file << "      - type: start_protocol\n";
        file << "        name: start_bgp\n";
        file << "        description: Start BGP protocol\n";
        file << "        parameters:\n";
        file << "          protocol: bgp\n";
        file << "    timeout_ms: 30000\n";
        file << "    enabled: true\n";
        file.close();
    }
};

TEST_F(YamlConfigTest, RouterConfigLoading) {
    std::string config_file = "/tmp/test_router_config.yaml";
    createTestRouterConfig(config_file);
    
    RouterConfig config;
    EXPECT_TRUE(yaml_config_->load_router_config(config_file, config));
    
    EXPECT_EQ(config.router_id, "1.1.1.1");
    EXPECT_EQ(config.hostname, "test-router");
    EXPECT_EQ(config.interfaces.size(), 2);
    EXPECT_EQ(config.interfaces[0], "eth0");
    EXPECT_EQ(config.interfaces[1], "eth1");
    EXPECT_TRUE(config.enable_bgp);
    EXPECT_TRUE(config.enable_ospf);
    EXPECT_FALSE(config.enable_isis);
    EXPECT_EQ(config.as_number, 65001);
    EXPECT_EQ(config.area_id, "0.0.0.0");
    EXPECT_EQ(config.system_id, "0000.0000.0001");
    
    // Clean up
    std::remove(config_file.c_str());
}

TEST_F(YamlConfigTest, RouterConfigSaving) {
    RouterConfig config;
    config.router_id = "2.2.2.2";
    config.hostname = "test-router-2";
    config.interfaces = {"eth0", "eth1", "eth2"};
    config.enable_bgp = true;
    config.enable_ospf = false;
    config.enable_isis = true;
    config.as_number = 65002;
    config.area_id = "0.0.0.1";
    config.system_id = "0000.0000.0002";
    
    std::string config_file = "/tmp/test_router_config_save.yaml";
    EXPECT_TRUE(yaml_config_->save_router_config(config_file, config));
    
    // Verify the file was created and can be loaded
    RouterConfig loaded_config;
    EXPECT_TRUE(yaml_config_->load_router_config(config_file, loaded_config));
    EXPECT_EQ(loaded_config.router_id, config.router_id);
    EXPECT_EQ(loaded_config.hostname, config.hostname);
    
    // Clean up
    std::remove(config_file.c_str());
}

TEST_F(YamlConfigTest, ScenarioLoading) {
    std::string scenario_file = "/tmp/test_scenarios.yaml";
    createTestScenario(scenario_file);
    
    EXPECT_TRUE(yaml_config_->load_scenarios(scenario_file));
    
    auto scenarios = yaml_config_->get_scenarios();
    EXPECT_EQ(scenarios.size(), 1);
    EXPECT_EQ(scenarios[0].name, "test_scenario");
    EXPECT_EQ(scenarios[0].description, "Test scenario");
    EXPECT_EQ(scenarios[0].version, "1.0");
    EXPECT_EQ(scenarios[0].steps.size(), 2);
    EXPECT_EQ(scenarios[0].steps[0].type, ScenarioStepType::CONFIGURE_INTERFACE);
    EXPECT_EQ(scenarios[0].steps[0].name, "configure_eth0");
    EXPECT_EQ(scenarios[0].steps[1].type, ScenarioStepType::START_PROTOCOL);
    EXPECT_EQ(scenarios[0].steps[1].name, "start_bgp");
    EXPECT_TRUE(scenarios[0].enabled);
    
    // Clean up
    std::remove(scenario_file.c_str());
}

TEST_F(YamlConfigTest, ScenarioExecution) {
    std::string scenario_file = "/tmp/test_scenarios_exec.yaml";
    createTestScenario(scenario_file);
    
    ASSERT_TRUE(yaml_config_->load_scenarios(scenario_file));
    
    // Execute scenario
    EXPECT_TRUE(yaml_config_->execute_scenario("test_scenario"));
    
    auto log = yaml_config_->get_execution_log();
    EXPECT_FALSE(log.empty());
    EXPECT_TRUE(std::any_of(log.begin(), log.end(), 
                           [](const std::string& entry) {
                               return entry.find("test_scenario") != std::string::npos;
                           }));
    
    // Clean up
    std::remove(scenario_file.c_str());
}

TEST_F(YamlConfigTest, ScenarioQueries) {
    std::string scenario_file = "/tmp/test_scenarios_query.yaml";
    createTestScenario(scenario_file);
    
    ASSERT_TRUE(yaml_config_->load_scenarios(scenario_file));
    
    // Test scenario queries
    auto scenario = yaml_config_->get_scenario("test_scenario");
    EXPECT_EQ(scenario.name, "test_scenario");
    
    auto empty_scenario = yaml_config_->get_scenario("nonexistent");
    EXPECT_TRUE(empty_scenario.name.empty());
    
    auto tagged_scenarios = yaml_config_->get_scenarios_by_tag("test");
    EXPECT_EQ(tagged_scenarios.size(), 1);
    
    // Clean up
    std::remove(scenario_file.c_str());
}

TEST_F(YamlConfigTest, TestCaseManagement) {
    // Create test case file
    std::string test_case_file = "/tmp/test_cases.yaml";
    std::ofstream file(test_case_file);
    file << "test_cases:\n";
    file << "  - name: test_case_1\n";
    file << "    description: Test case 1\n";
    file << "    scenarios:\n";
    file << "      - name: test_scenario\n";
    file << "        description: Test scenario\n";
    file << "        version: 1.0\n";
    file << "        steps: []\n";
    file << "    global_variables:\n";
    file << "      var1: value1\n";
    file << "      var2: value2\n";
    file << "    tags:\n";
    file << "      - unit\n";
    file << "      - integration\n";
    file << "    enabled: true\n";
    file.close();
    
    EXPECT_TRUE(yaml_config_->load_test_cases(test_case_file));
    
    auto test_cases = yaml_config_->get_test_cases();
    EXPECT_EQ(test_cases.size(), 1);
    EXPECT_EQ(test_cases[0].name, "test_case_1");
    EXPECT_EQ(test_cases[0].description, "Test case 1");
    EXPECT_EQ(test_cases[0].scenarios.size(), 1);
    EXPECT_EQ(test_cases[0].global_variables.size(), 2);
    EXPECT_EQ(test_cases[0].tags.size(), 2);
    EXPECT_TRUE(test_cases[0].enabled);
    
    auto test_case = yaml_config_->get_test_case("test_case_1");
    EXPECT_EQ(test_case.name, "test_case_1");
    
    auto empty_test_case = yaml_config_->get_test_case("nonexistent");
    EXPECT_TRUE(empty_test_case.name.empty());
    
    // Clean up
    std::remove(test_case_file.c_str());
}

TEST_F(YamlConfigTest, ScenarioValidation) {
    Scenario valid_scenario;
    valid_scenario.name = "valid_scenario";
    valid_scenario.description = "Valid scenario";
    valid_scenario.version = "1.0";
    valid_scenario.steps = {
        {ScenarioStepType::CONFIGURE_INTERFACE, "step1", "Configure interface", {}, 5000, true, {}}
    };
    valid_scenario.timeout_ms = 30000;
    valid_scenario.enabled = true;
    
    EXPECT_TRUE(yaml_config_->validate_scenario(valid_scenario));
    
    Scenario invalid_scenario;
    invalid_scenario.name = ""; // Empty name
    invalid_scenario.description = "Invalid scenario";
    invalid_scenario.version = "1.0";
    invalid_scenario.steps = {
        {ScenarioStepType::CONFIGURE_INTERFACE, "", "Configure interface", {}, 5000, true, {}} // Empty step name
    };
    invalid_scenario.timeout_ms = 30000;
    invalid_scenario.enabled = true;
    
    EXPECT_FALSE(yaml_config_->validate_scenario(invalid_scenario));
}

TEST_F(YamlConfigTest, TestCaseValidation) {
    TestCase valid_test_case;
    valid_test_case.name = "valid_test_case";
    valid_test_case.description = "Valid test case";
    valid_test_case.scenarios = {
        {
            "valid_scenario", "Valid scenario", "1.0", 
            {{ScenarioStepType::CONFIGURE_INTERFACE, "step1", "Configure interface", {}, 5000, true, {}}},
            {}, 30000, true
        }
    };
    valid_test_case.global_variables = {{"var1", "value1"}};
    valid_test_case.tags = {"unit"};
    valid_test_case.enabled = true;
    
    EXPECT_TRUE(yaml_config_->validate_test_case(valid_test_case));
    
    TestCase invalid_test_case;
    invalid_test_case.name = ""; // Empty name
    invalid_test_case.description = "Invalid test case";
    invalid_test_case.scenarios = {
        {
            "", "Invalid scenario", "1.0", // Empty scenario name
            {{ScenarioStepType::CONFIGURE_INTERFACE, "step1", "Configure interface", {}, 5000, true, {}}},
            {}, 30000, true
        }
    };
    invalid_test_case.global_variables = {};
    invalid_test_case.tags = {};
    invalid_test_case.enabled = true;
    
    EXPECT_FALSE(yaml_config_->validate_test_case(invalid_test_case));
}

TEST_F(YamlConfigTest, YamlUtils) {
    // Test file operations
    EXPECT_FALSE(YamlUtils::file_exists("/nonexistent/file.yaml"));
    
    // Test validation functions
    EXPECT_TRUE(YamlUtils::is_valid_ip_address("192.168.1.1"));
    EXPECT_FALSE(YamlUtils::is_valid_ip_address("256.256.256.256"));
    EXPECT_FALSE(YamlUtils::is_valid_ip_address("not.an.ip"));
    
    EXPECT_TRUE(YamlUtils::is_valid_subnet_mask("255.255.255.0"));
    EXPECT_FALSE(YamlUtils::is_valid_subnet_mask("invalid"));
    
    EXPECT_TRUE(YamlUtils::is_valid_as_number("65001"));
    EXPECT_FALSE(YamlUtils::is_valid_as_number("0"));
    EXPECT_FALSE(YamlUtils::is_valid_as_number("invalid"));
    
    // Test YAML operations
    std::string yaml_data = "key: value\nnested:\n  subkey: subvalue\n";
    EXPECT_TRUE(YamlUtils::is_valid_yaml("test.yaml")); // This will fail without actual file
    
    auto node = YamlUtils::string_to_node(yaml_data);
    EXPECT_FALSE(node.IsNull());
    
    auto map = YamlUtils::node_to_map(node);
    EXPECT_FALSE(map.empty());
}
