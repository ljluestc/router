#include "yaml_config.h"
#include <iostream>
#include <fstream>
#include <regex>

using namespace RouterSim;

YamlConfig::YamlConfig() {
}

YamlConfig::~YamlConfig() {
}

bool YamlConfig::load_router_config(const std::string& filename, RouterConfig& config) {
    try {
        YAML::Node node = YAML::LoadFile(filename);
        config = parse_router_config(node);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading router config: " << e.what() << "\n";
        return false;
    }
}

bool YamlConfig::save_router_config(const std::string& filename, const RouterConfig& config) {
    try {
        YAML::Node node = generate_router_config(config);
        std::ofstream file(filename);
        file << node;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error saving router config: " << e.what() << "\n";
        return false;
    }
}

bool YamlConfig::load_scenarios(const std::string& filename) {
    try {
        YAML::Node node = YAML::LoadFile(filename);
        
        if (node["scenarios"]) {
            for (const auto& scenario_node : node["scenarios"]) {
                scenarios_.push_back(parse_scenario(scenario_node));
            }
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading scenarios: " << e.what() << "\n";
        return false;
    }
}

bool YamlConfig::save_scenarios(const std::string& filename) {
    try {
        YAML::Node node;
        node["scenarios"] = YAML::Node(YAML::NodeType::Sequence);
        
        for (const auto& scenario : scenarios_) {
            node["scenarios"].push_back(generate_scenario(scenario));
        }
        
        std::ofstream file(filename);
        file << node;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error saving scenarios: " << e.what() << "\n";
        return false;
    }
}

std::vector<Scenario> YamlConfig::get_scenarios() const {
    std::lock_guard<std::mutex> lock(scenarios_mutex_);
    return scenarios_;
}

std::vector<Scenario> YamlConfig::get_scenarios_by_tag(const std::string& tag) const {
    std::lock_guard<std::mutex> lock(scenarios_mutex_);
    
    std::vector<Scenario> result;
    for (const auto& scenario : scenarios_) {
        // Simple tag matching - in real implementation, scenarios would have tags
        if (scenario.name.find(tag) != std::string::npos) {
            result.push_back(scenario);
        }
    }
    return result;
}

Scenario YamlConfig::get_scenario(const std::string& name) const {
    std::lock_guard<std::mutex> lock(scenarios_mutex_);
    
    for (const auto& scenario : scenarios_) {
        if (scenario.name == name) {
            return scenario;
        }
    }
    
    return Scenario{}; // Empty scenario if not found
}

bool YamlConfig::load_test_cases(const std::string& filename) {
    try {
        YAML::Node node = YAML::LoadFile(filename);
        
        if (node["test_cases"]) {
            for (const auto& test_case_node : node["test_cases"]) {
                test_cases_.push_back(parse_test_case(test_case_node));
            }
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading test cases: " << e.what() << "\n";
        return false;
    }
}

bool YamlConfig::save_test_cases(const std::string& filename) {
    try {
        YAML::Node node;
        node["test_cases"] = YAML::Node(YAML::NodeType::Sequence);
        
        for (const auto& test_case : test_cases_) {
            node["test_cases"].push_back(generate_test_case(test_case));
        }
        
        std::ofstream file(filename);
        file << node;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error saving test cases: " << e.what() << "\n";
        return false;
    }
}

std::vector<TestCase> YamlConfig::get_test_cases() const {
    std::lock_guard<std::mutex> lock(test_cases_mutex_);
    return test_cases_;
}

TestCase YamlConfig::get_test_case(const std::string& name) const {
    std::lock_guard<std::mutex> lock(test_cases_mutex_);
    
    for (const auto& test_case : test_cases_) {
        if (test_case.name == name) {
            return test_case;
        }
    }
    
    return TestCase{}; // Empty test case if not found
}

bool YamlConfig::execute_scenario(const std::string& scenario_name) {
    std::lock_guard<std::mutex> log_lock(log_mutex_);
    
    auto scenario = get_scenario(scenario_name);
    if (scenario.name.empty()) {
        execution_log_.push_back("Scenario not found: " + scenario_name);
        return false;
    }
    
    execution_log_.push_back("Executing scenario: " + scenario_name);
    
    for (const auto& step : scenario.steps) {
        execution_log_.push_back("Executing step: " + step.name);
        
        // Simple step execution - in real implementation, this would be more sophisticated
        if (step.type == ScenarioStepType::CONFIGURE_INTERFACE) {
            execution_log_.push_back("  Configuring interface: " + step.parameters.at("name"));
        } else if (step.type == ScenarioStepType::START_PROTOCOL) {
            execution_log_.push_back("  Starting protocol: " + step.parameters.at("protocol"));
        } else if (step.type == ScenarioStepType::WAIT) {
            uint32_t wait_ms = std::stoul(step.parameters.at("duration"));
            std::this_thread::sleep_for(std::chrono::milliseconds(wait_ms));
            execution_log_.push_back("  Waited for " + std::to_string(wait_ms) + "ms");
        }
        
        // Add more step types as needed
    }
    
    execution_log_.push_back("Scenario completed: " + scenario_name);
    return true;
}

bool YamlConfig::execute_test_case(const std::string& test_case_name) {
    auto test_case = get_test_case(test_case_name);
    if (test_case.name.empty()) {
        return false;
    }
    
    for (const auto& scenario : test_case.scenarios) {
        if (!execute_scenario(scenario.name)) {
            return false;
        }
    }
    
    return true;
}

std::vector<std::string> YamlConfig::get_execution_log() const {
    std::lock_guard<std::mutex> lock(log_mutex_);
    return execution_log_;
}

bool YamlConfig::validate_scenario(const Scenario& scenario) const {
    if (scenario.name.empty()) {
        return false;
    }
    
    for (const auto& step : scenario.steps) {
        if (step.name.empty()) {
            return false;
        }
    }
    
    return true;
}

bool YamlConfig::validate_test_case(const TestCase& test_case) const {
    if (test_case.name.empty()) {
        return false;
    }
    
    for (const auto& scenario : test_case.scenarios) {
        if (!validate_scenario(scenario)) {
            return false;
        }
    }
    
    return true;
}

// Configuration parsing methods
RouterConfig YamlConfig::parse_router_config(const YAML::Node& node) const {
    RouterConfig config;
    
    if (node["router_id"]) {
        config.router_id = node["router_id"].as<std::string>();
    }
    
    if (node["hostname"]) {
        config.hostname = node["hostname"].as<std::string>();
    }
    
    if (node["interfaces"]) {
        for (const auto& iface_node : node["interfaces"]) {
            config.interfaces.push_back(iface_node.as<std::string>());
        }
    }
    
    if (node["enable_bgp"]) {
        config.enable_bgp = node["enable_bgp"].as<bool>();
    }
    
    if (node["enable_ospf"]) {
        config.enable_ospf = node["enable_ospf"].as<bool>();
    }
    
    if (node["enable_isis"]) {
        config.enable_isis = node["enable_isis"].as<bool>();
    }
    
    if (node["as_number"]) {
        config.as_number = node["as_number"].as<uint32_t>();
    }
    
    if (node["area_id"]) {
        config.area_id = node["area_id"].as<std::string>();
    }
    
    if (node["system_id"]) {
        config.system_id = node["system_id"].as<std::string>();
    }
    
    return config;
}

InterfaceConfig YamlConfig::parse_interface_config(const YAML::Node& node) const {
    InterfaceConfig config;
    
    if (node["name"]) {
        config.name = node["name"].as<std::string>();
    }
    
    if (node["ip_address"]) {
        config.ip_address = node["ip_address"].as<std::string>();
    }
    
    if (node["subnet_mask"]) {
        config.subnet_mask = node["subnet_mask"].as<std::string>();
    }
    
    if (node["bandwidth_mbps"]) {
        config.bandwidth_mbps = node["bandwidth_mbps"].as<uint32_t>();
    }
    
    if (node["is_up"]) {
        config.is_up = node["is_up"].as<bool>();
    }
    
    if (node["description"]) {
        config.description = node["description"].as<std::string>();
    }
    
    return config;
}

BGPConfig YamlConfig::parse_bgp_config(const YAML::Node& node) const {
    BGPConfig config;
    
    if (node["as_number"]) {
        config.as_number = node["as_number"].as<uint32_t>();
    }
    
    if (node["router_id"]) {
        config.router_id = node["router_id"].as<std::string>();
    }
    
    if (node["neighbors"]) {
        for (const auto& neighbor_node : node["neighbors"]) {
            config.neighbors.push_back(neighbor_node.as<std::string>());
        }
    }
    
    if (node["networks"]) {
        for (const auto& network_node : node["networks"]) {
            config.networks.push_back(network_node.as<std::string>());
        }
    }
    
    if (node["enable_graceful_restart"]) {
        config.enable_graceful_restart = node["enable_graceful_restart"].as<bool>();
    }
    
    if (node["hold_time"]) {
        config.hold_time = node["hold_time"].as<uint32_t>();
    }
    
    if (node["keepalive"]) {
        config.keepalive = node["keepalive"].as<uint32_t>();
    }
    
    return config;
}

OSPFConfig YamlConfig::parse_ospf_config(const YAML::Node& node) const {
    OSPFConfig config;
    
    if (node["area_id"]) {
        config.area_id = node["area_id"].as<std::string>();
    }
    
    if (node["router_id"]) {
        config.router_id = node["router_id"].as<std::string>();
    }
    
    if (node["networks"]) {
        for (const auto& network_node : node["networks"]) {
            config.networks.push_back(network_node.as<std::string>());
        }
    }
    
    if (node["hello_interval"]) {
        config.hello_interval = node["hello_interval"].as<uint32_t>();
    }
    
    if (node["dead_interval"]) {
        config.dead_interval = node["dead_interval"].as<uint32_t>();
    }
    
    if (node["retransmit_interval"]) {
        config.retransmit_interval = node["retransmit_interval"].as<uint32_t>();
    }
    
    if (node["transmit_delay"]) {
        config.transmit_delay = node["transmit_delay"].as<uint32_t>();
    }
    
    if (node["priority"]) {
        config.priority = node["priority"].as<uint32_t>();
    }
    
    return config;
}

ISISConfig YamlConfig::parse_isis_config(const YAML::Node& node) const {
    ISISConfig config;
    
    if (node["system_id"]) {
        config.system_id = node["system_id"].as<std::string>();
    }
    
    if (node["area_id"]) {
        config.area_id = node["area_id"].as<std::string>();
    }
    
    if (node["level"]) {
        config.level = node["level"].as<uint8_t>();
    }
    
    if (node["networks"]) {
        for (const auto& network_node : node["networks"]) {
            config.networks.push_back(network_node.as<std::string>());
        }
    }
    
    if (node["hello_interval"]) {
        config.hello_interval = node["hello_interval"].as<uint32_t>();
    }
    
    if (node["hold_time"]) {
        config.hold_time = node["hold_time"].as<uint32_t>();
    }
    
    if (node["priority"]) {
        config.priority = node["priority"].as<uint32_t>();
    }
    
    return config;
}

ShapingConfig YamlConfig::parse_shaping_config(const YAML::Node& node) const {
    ShapingConfig config;
    
    if (node["rate_bps"]) {
        config.rate_bps = node["rate_bps"].as<uint32_t>();
    }
    
    if (node["burst_size"]) {
        config.burst_size = node["burst_size"].as<uint32_t>();
    }
    
    if (node["queue_size"]) {
        config.queue_size = node["queue_size"].as<uint32_t>();
    }
    
    if (node["enable_wfq"]) {
        config.enable_wfq = node["enable_wfq"].as<bool>();
    }
    
    if (node["num_queues"]) {
        config.num_queues = node["num_queues"].as<uint32_t>();
    }
    
    return config;
}

ImpairmentConfig YamlConfig::parse_impairment_config(const YAML::Node& node) const {
    ImpairmentConfig config;
    
    if (node["enable_delay"]) {
        config.enable_delay = node["enable_delay"].as<bool>();
    }
    
    if (node["delay_ms"]) {
        config.delay_ms = node["delay_ms"].as<uint32_t>();
    }
    
    if (node["enable_jitter"]) {
        config.enable_jitter = node["enable_jitter"].as<bool>();
    }
    
    if (node["jitter_ms"]) {
        config.jitter_ms = node["jitter_ms"].as<uint32_t>();
    }
    
    if (node["enable_loss"]) {
        config.enable_loss = node["enable_loss"].as<bool>();
    }
    
    if (node["loss_percent"]) {
        config.loss_percent = node["loss_percent"].as<double>();
    }
    
    if (node["enable_duplicate"]) {
        config.enable_duplicate = node["enable_duplicate"].as<bool>();
    }
    
    if (node["duplicate_percent"]) {
        config.duplicate_percent = node["duplicate_percent"].as<double>();
    }
    
    if (node["enable_corruption"]) {
        config.enable_corruption = node["enable_corruption"].as<bool>();
    }
    
    if (node["corruption_percent"]) {
        config.corruption_percent = node["corruption_percent"].as<double>();
    }
    
    if (node["enable_reorder"]) {
        config.enable_reorder = node["enable_reorder"].as<bool>();
    }
    
    if (node["reorder_percent"]) {
        config.reorder_percent = node["reorder_percent"].as<double>();
    }
    
    return config;
}

Scenario YamlConfig::parse_scenario(const YAML::Node& node) const {
    Scenario scenario;
    
    if (node["name"]) {
        scenario.name = node["name"].as<std::string>();
    }
    
    if (node["description"]) {
        scenario.description = node["description"].as<std::string>();
    }
    
    if (node["version"]) {
        scenario.version = node["version"].as<std::string>();
    }
    
    if (node["steps"]) {
        for (const auto& step_node : node["steps"]) {
            scenario.steps.push_back(parse_scenario_step(step_node));
        }
    }
    
    if (node["variables"]) {
        for (const auto& var_node : node["variables"]) {
            for (const auto& pair : var_node) {
                scenario.variables[pair.first.as<std::string>()] = pair.second.as<std::string>();
            }
        }
    }
    
    if (node["timeout_ms"]) {
        scenario.timeout_ms = node["timeout_ms"].as<uint32_t>();
    }
    
    if (node["enabled"]) {
        scenario.enabled = node["enabled"].as<bool>();
    }
    
    return scenario;
}

ScenarioStep YamlConfig::parse_scenario_step(const YAML::Node& node) const {
    ScenarioStep step;
    
    if (node["type"]) {
        std::string type_str = node["type"].as<std::string>();
        // Convert string to enum - simplified
        if (type_str == "configure_interface") {
            step.type = ScenarioStepType::CONFIGURE_INTERFACE;
        } else if (type_str == "start_protocol") {
            step.type = ScenarioStepType::START_PROTOCOL;
        } else if (type_str == "wait") {
            step.type = ScenarioStepType::WAIT;
        }
        // Add more types as needed
    }
    
    if (node["name"]) {
        step.name = node["name"].as<std::string>();
    }
    
    if (node["description"]) {
        step.description = node["description"].as<std::string>();
    }
    
    if (node["parameters"]) {
        for (const auto& param_node : node["parameters"]) {
            for (const auto& pair : param_node) {
                step.parameters[pair.first.as<std::string>()] = pair.second.as<std::string>();
            }
        }
    }
    
    if (node["timeout_ms"]) {
        step.timeout_ms = node["timeout_ms"].as<uint32_t>();
    }
    
    if (node["required"]) {
        step.required = node["required"].as<bool>();
    }
    
    if (node["expected_results"]) {
        for (const auto& result_node : node["expected_results"]) {
            step.expected_results.push_back(result_node.as<std::string>());
        }
    }
    
    return step;
}

TestCase YamlConfig::parse_test_case(const YAML::Node& node) const {
    TestCase test_case;
    
    if (node["name"]) {
        test_case.name = node["name"].as<std::string>();
    }
    
    if (node["description"]) {
        test_case.description = node["description"].as<std::string>();
    }
    
    if (node["scenarios"]) {
        for (const auto& scenario_node : node["scenarios"]) {
            test_case.scenarios.push_back(parse_scenario(scenario_node));
        }
    }
    
    if (node["global_variables"]) {
        for (const auto& var_node : node["global_variables"]) {
            for (const auto& pair : var_node) {
                test_case.global_variables[pair.first.as<std::string>()] = pair.second.as<std::string>();
            }
        }
    }
    
    if (node["tags"]) {
        for (const auto& tag_node : node["tags"]) {
            test_case.tags.push_back(tag_node.as<std::string>());
        }
    }
    
    if (node["enabled"]) {
        test_case.enabled = node["enabled"].as<bool>();
    }
    
    return test_case;
}

// Configuration generation methods (stub implementations)
YAML::Node YamlConfig::generate_router_config(const RouterConfig& config) const {
    YAML::Node node;
    node["router_id"] = config.router_id;
    node["hostname"] = config.hostname;
    node["enable_bgp"] = config.enable_bgp;
    node["enable_ospf"] = config.enable_ospf;
    node["enable_isis"] = config.enable_isis;
    node["as_number"] = config.as_number;
    node["area_id"] = config.area_id;
    node["system_id"] = config.system_id;
    return node;
}

YAML::Node YamlConfig::generate_interface_config(const InterfaceConfig& config) const {
    YAML::Node node;
    node["name"] = config.name;
    node["ip_address"] = config.ip_address;
    node["subnet_mask"] = config.subnet_mask;
    node["bandwidth_mbps"] = config.bandwidth_mbps;
    node["is_up"] = config.is_up;
    node["description"] = config.description;
    return node;
}

YAML::Node YamlConfig::generate_bgp_config(const BGPConfig& config) const {
    YAML::Node node;
    node["as_number"] = config.as_number;
    node["router_id"] = config.router_id;
    node["enable_graceful_restart"] = config.enable_graceful_restart;
    node["hold_time"] = config.hold_time;
    node["keepalive"] = config.keepalive;
    return node;
}

YAML::Node YamlConfig::generate_ospf_config(const OSPFConfig& config) const {
    YAML::Node node;
    node["area_id"] = config.area_id;
    node["router_id"] = config.router_id;
    node["hello_interval"] = config.hello_interval;
    node["dead_interval"] = config.dead_interval;
    node["retransmit_interval"] = config.retransmit_interval;
    node["transmit_delay"] = config.transmit_delay;
    node["priority"] = config.priority;
    return node;
}

YAML::Node YamlConfig::generate_isis_config(const ISISConfig& config) const {
    YAML::Node node;
    node["system_id"] = config.system_id;
    node["area_id"] = config.area_id;
    node["level"] = config.level;
    node["hello_interval"] = config.hello_interval;
    node["hold_time"] = config.hold_time;
    node["priority"] = config.priority;
    return node;
}

YAML::Node YamlConfig::generate_shaping_config(const ShapingConfig& config) const {
    YAML::Node node;
    node["rate_bps"] = config.rate_bps;
    node["burst_size"] = config.burst_size;
    node["queue_size"] = config.queue_size;
    node["enable_wfq"] = config.enable_wfq;
    node["num_queues"] = config.num_queues;
    return node;
}

YAML::Node YamlConfig::generate_impairment_config(const ImpairmentConfig& config) const {
    YAML::Node node;
    node["enable_delay"] = config.enable_delay;
    node["delay_ms"] = config.delay_ms;
    node["enable_jitter"] = config.enable_jitter;
    node["jitter_ms"] = config.jitter_ms;
    node["enable_loss"] = config.enable_loss;
    node["loss_percent"] = config.loss_percent;
    node["enable_duplicate"] = config.enable_duplicate;
    node["duplicate_percent"] = config.duplicate_percent;
    node["enable_corruption"] = config.enable_corruption;
    node["corruption_percent"] = config.corruption_percent;
    node["enable_reorder"] = config.enable_reorder;
    node["reorder_percent"] = config.reorder_percent;
    return node;
}

YAML::Node YamlConfig::generate_scenario(const Scenario& scenario) const {
    YAML::Node node;
    node["name"] = scenario.name;
    node["description"] = scenario.description;
    node["version"] = scenario.version;
    node["timeout_ms"] = scenario.timeout_ms;
    node["enabled"] = scenario.enabled;
    return node;
}

YAML::Node YamlConfig::generate_scenario_step(const ScenarioStep& step) const {
    YAML::Node node;
    node["name"] = step.name;
    node["description"] = step.description;
    node["timeout_ms"] = step.timeout_ms;
    node["required"] = step.required;
    return node;
}

YAML::Node YamlConfig::generate_test_case(const TestCase& test_case) const {
    YAML::Node node;
    node["name"] = test_case.name;
    node["description"] = test_case.description;
    node["enabled"] = test_case.enabled;
    return node;
}

std::string YamlConfig::substitute_variables(const std::string& text, 
                                           const std::map<std::string, std::string>& variables) const {
    std::string result = text;
    
    for (const auto& pair : variables) {
        std::string placeholder = "${" + pair.first + "}";
        size_t pos = 0;
        while ((pos = result.find(placeholder, pos)) != std::string::npos) {
            result.replace(pos, placeholder.length(), pair.second);
            pos += pair.second.length();
        }
    }
    
    return result;
}

// YamlUtils implementation
bool YamlUtils::is_valid_yaml(const std::string& filename) {
    try {
        YAML::LoadFile(filename);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

bool YamlUtils::has_required_fields(const YAML::Node& node, const std::vector<std::string>& fields) {
    for (const auto& field : fields) {
        if (!node[field]) {
            return false;
        }
    }
    return true;
}

bool YamlUtils::is_valid_ip_address(const std::string& ip) {
    std::regex ip_regex(R"(^(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3})$)");
    std::smatch matches;
    
    if (!std::regex_match(ip, matches, ip_regex)) {
        return false;
    }
    
    for (int i = 1; i <= 4; i++) {
        int octet = std::stoi(matches[i].str());
        if (octet < 0 || octet > 255) {
            return false;
        }
    }
    
    return true;
}

bool YamlUtils::is_valid_subnet_mask(const std::string& mask) {
    return is_valid_ip_address(mask);
}

bool YamlUtils::is_valid_as_number(const std::string& as) {
    try {
        uint32_t as_num = std::stoul(as);
        return as_num > 0 && as_num <= 4294967295;
    } catch (const std::exception&) {
        return false;
    }
}

std::string YamlUtils::node_to_string(const YAML::Node& node) {
    std::stringstream ss;
    ss << node;
    return ss.str();
}

YAML::Node YamlUtils::string_to_node(const std::string& str) {
    return YAML::Load(str);
}

std::map<std::string, std::string> YamlUtils::node_to_map(const YAML::Node& node) {
    std::map<std::string, std::string> result;
    
    for (const auto& pair : node) {
        result[pair.first.as<std::string>()] = pair.second.as<std::string>();
    }
    
    return result;
}

bool YamlUtils::file_exists(const std::string& filename) {
    std::ifstream file(filename);
    return file.good();
}

bool YamlUtils::create_directory(const std::string& path) {
    return system(("mkdir -p " + path).c_str()) == 0;
}

std::vector<std::string> YamlUtils::list_yaml_files(const std::string& directory) {
    std::vector<std::string> files;
    
    // Simple implementation - in real code, use filesystem library
    std::string command = "find " + directory + " -name '*.yaml' -o -name '*.yml'";
    FILE* pipe = popen(command.c_str(), "r");
    if (pipe) {
        char buffer[128];
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            std::string filename = buffer;
            filename.erase(filename.find_last_not_of("\n\r") + 1);
            files.push_back(filename);
        }
        pclose(pipe);
    }
    
    return files;
}
