#include "aviatrix_features.h"
#include <curl/curl.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>
#include <openssl/hmac.h>

namespace RouterSim {

// Callback function for libcurl
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s) {
    size_t newLength = size * nmemb;
    try {
        s->append((char*)contents, newLength);
        return newLength;
    } catch (std::bad_alloc& e) {
        return 0;
    }
}

// AviatrixAPI implementation
class AviatrixIntegration::AviatrixAPI {
public:
    AviatrixAPI(const AviatrixConfig& config) : config_(config), session_id_("") {}

    ~AviatrixAPI() {
        if (!session_id_.empty()) {
            logout();
        }
    }

    nlohmann::json call_api(const std::string& action, const nlohmann::json& params) const {
        std::lock_guard<std::mutex> lock(session_mutex_);
        
        if (session_id_.empty()) {
            if (!login()) {
                return nlohmann::json{{"error", "Authentication failed"}};
            }
        }

        CURL* curl;
        CURLcode res;
        std::string response;

        curl = curl_easy_init();
        if (curl) {
            std::string url = "https://" + config_.controller_ip + "/v1/api";
            
            // Prepare request data
            nlohmann::json request_data = params;
            request_data["action"] = action;
            request_data["CID"] = session_id_;

            std::string post_data = request_data.dump();

            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, config_.verify_ssl ? 1L : 0L);
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, config_.timeout_seconds);

            struct curl_slist* headers = nullptr;
            headers = curl_slist_append(headers, "Content-Type: application/json");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

            res = curl_easy_perform(curl);
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);

            if (res != CURLE_OK) {
                return nlohmann::json{{"error", "API call failed: " + std::string(curl_easy_strerror(res))}};
            }

            try {
                return nlohmann::json::parse(response);
            } catch (const std::exception& e) {
                return nlohmann::json{{"error", "Failed to parse response: " + std::string(e.what())}};
            }
        }

        return nlohmann::json{{"error", "Failed to initialize CURL"}};
    }

    bool login() {
        nlohmann::json login_params = {
            {"username", config_.username},
            {"password", config_.password}
        };

        auto response = call_api("login", login_params);
        if (response.contains("CID")) {
            session_id_ = response["CID"];
            return true;
        }
        return false;
    }
    
    bool logout() {
        if (session_id_.empty()) {
            return true;
        }

        nlohmann::json logout_params = {};
        auto response = call_api("logout", logout_params);
        session_id_ = "";
        return response.contains("return") && response["return"];
    }

private:
    AviatrixConfig config_;
    mutable std::string session_id_;
    mutable std::mutex session_mutex_;
};

// AviatrixIntegration implementation
AviatrixIntegration::AviatrixIntegration() 
    : connected_(false), authenticated_(false), monitor_running_(false) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

AviatrixIntegration::~AviatrixIntegration() {
    shutdown();
    curl_global_cleanup();
}

bool AviatrixIntegration::initialize(const AviatrixConfig& config) {
    std::lock_guard<std::mutex> lock(state_mutex_);
    
    config_ = config;
    api_client_ = std::make_unique<AviatrixAPI>(config_);
    
    if (authenticate()) {
        connected_ = true;
        authenticated_ = true;
        
        // Start monitoring thread
        monitor_running_ = true;
        monitor_thread_ = std::thread(&AviatrixIntegration::monitor_loop, this);
        
        return true;
    }
    
    return false;
}

void AviatrixIntegration::shutdown() {
    {
        std::lock_guard<std::mutex> lock(state_mutex_);
    if (monitor_running_) {
        monitor_running_ = false;
        if (monitor_thread_.joinable()) {
            monitor_thread_.join();
        }
    }
    }
    
    if (api_client_) {
        api_client_->logout();
    }
    
    connected_ = false;
    authenticated_ = false;
}

bool AviatrixIntegration::is_connected() const {
    return connected_ && authenticated_;
}

bool AviatrixIntegration::authenticate() {
    if (!api_client_) {
        return false;
    }
    return api_client_->login();
}

bool AviatrixIntegration::refresh_token() {
    return authenticate();
}

std::string AviatrixIntegration::get_auth_token() const {
    // Aviatrix uses session-based authentication
    return "session_based";
}

// Transit Gateway management
std::vector<TransitGateway> AviatrixIntegration::list_transit_gateways() const {
    if (!is_connected()) {
        return {};
    }

    nlohmann::json params = {};
    auto response = api_client_->call_api("list_transit_gateways", params);
    
    std::vector<TransitGateway> gateways;
    if (response.contains("results") && response["results"].is_array()) {
        for (const auto& gw_data : response["results"]) {
            gateways.push_back(parse_transit_gateway_response(gw_data));
        }
    }
    
    return gateways;
}

TransitGateway AviatrixIntegration::get_transit_gateway(const std::string& gw_name) const {
    if (!is_connected()) {
        return {};
    }

    nlohmann::json params = {{"gateway_name", gw_name}};
    auto response = api_client_->call_api("get_transit_gateway", params);
    
    if (response.contains("results")) {
        return parse_transit_gateway_response(response["results"]);
    }
    
    return {};
}

bool AviatrixIntegration::create_transit_gateway(const TransitGateway& tgw) {
    if (!is_connected()) {
        return false;
    }
    
    nlohmann::json params = {
        {"cloud_type", tgw.cloud_type},
        {"account_name", tgw.account_name},
        {"gw_name", tgw.gw_name},
        {"vpc_id", tgw.vpc_id},
        {"vpc_reg", tgw.region},
        {"subnet", tgw.subnet},
        {"gw_size", tgw.gw_size},
        {"enable_encrypt_peering", tgw.enable_encrypt_peering},
        {"enable_learned_cidrs_approval", tgw.enable_learned_cidrs_approval}
    };

    auto response = api_client_->call_api("create_transit_gateway", params);
    return response.contains("return") && response["return"];
}

bool AviatrixIntegration::delete_transit_gateway(const std::string& gw_name) {
    if (!is_connected()) {
        return false;
    }
    
    nlohmann::json params = {{"gw_name", gw_name}};
    auto response = api_client_->call_api("delete_transit_gateway", params);
    return response.contains("return") && response["return"];
}

bool AviatrixIntegration::update_transit_gateway(const std::string& gw_name, const TransitGateway& tgw) {
    // Aviatrix doesn't support direct updates, need to delete and recreate
    if (delete_transit_gateway(gw_name)) {
        return create_transit_gateway(tgw);
    }
        return false;
    }
    
// Spoke Gateway management
std::vector<SpokeGateway> AviatrixIntegration::list_spoke_gateways() const {
    if (!is_connected()) {
        return {};
    }

    nlohmann::json params = {};
    auto response = api_client_->call_api("list_spoke_gateways", params);
    
    std::vector<SpokeGateway> gateways;
    if (response.contains("results") && response["results"].is_array()) {
        for (const auto& gw_data : response["results"]) {
            gateways.push_back(parse_spoke_gateway_response(gw_data));
        }
    }
    
    return gateways;
}

SpokeGateway AviatrixIntegration::get_spoke_gateway(const std::string& gw_name) const {
    if (!is_connected()) {
        return {};
    }

    nlohmann::json params = {{"gw_name", gw_name}};
    auto response = api_client_->call_api("get_spoke_gateway", params);
    
    if (response.contains("results")) {
        return parse_spoke_gateway_response(response["results"]);
    }
    
    return {};
}

bool AviatrixIntegration::create_spoke_gateway(const SpokeGateway& sgw) {
    if (!is_connected()) {
        return false;
    }
    
    nlohmann::json params = {
        {"cloud_type", sgw.cloud_type},
        {"account_name", sgw.account_name},
        {"gw_name", sgw.gw_name},
        {"vpc_id", sgw.vpc_id},
        {"vpc_reg", sgw.region},
        {"subnet", sgw.subnet},
        {"gw_size", sgw.gw_size},
        {"transit_gw", sgw.transit_gw},
        {"enable_encrypt_peering", sgw.enable_encrypt_peering}
    };

    auto response = api_client_->call_api("create_spoke_gateway", params);
    return response.contains("return") && response["return"];
}

bool AviatrixIntegration::delete_spoke_gateway(const std::string& gw_name) {
    if (!is_connected()) {
        return false;
    }
    
    nlohmann::json params = {{"gw_name", gw_name}};
    auto response = api_client_->call_api("delete_spoke_gateway", params);
    return response.contains("return") && response["return"];
}

bool AviatrixIntegration::update_spoke_gateway(const std::string& gw_name, const SpokeGateway& sgw) {
    if (delete_spoke_gateway(gw_name)) {
        return create_spoke_gateway(sgw);
    }
        return false;
    }
    
// VPC Connection management
std::vector<VPCConnection> AviatrixIntegration::list_vpc_connections() const {
    if (!is_connected()) {
        return {};
    }

    nlohmann::json params = {};
    auto response = api_client_->call_api("list_vpc_connections", params);
    
    std::vector<VPCConnection> connections;
    if (response.contains("results") && response["results"].is_array()) {
        for (const auto& conn_data : response["results"]) {
            connections.push_back(parse_vpc_connection_response(conn_data));
        }
    }
    
    return connections;
}

VPCConnection AviatrixIntegration::get_vpc_connection(const std::string& conn_name) const {
    if (!is_connected()) {
        return {};
    }

    nlohmann::json params = {{"connection_name", conn_name}};
    auto response = api_client_->call_api("get_vpc_connection", params);
    
    if (response.contains("results")) {
        return parse_vpc_connection_response(response["results"]);
    }
    
    return {};
}

bool AviatrixIntegration::create_vpc_connection(const VPCConnection& conn) {
    if (!is_connected()) {
        return false;
    }
    
    nlohmann::json params = {
        {"connection_name", conn.connection_name},
        {"vpc_id", conn.vpc_id},
        {"account_name", conn.account_name},
        {"region", conn.region},
        {"transit_gateway", conn.transit_gateway},
        {"spoke_gateway", conn.spoke_gateway},
        {"connection_type", conn.connection_type},
        {"enable_learned_cidrs_approval", conn.enable_learned_cidrs_approval}
    };

    if (!conn.approved_cidrs.empty()) {
        params["approved_cidrs"] = conn.approved_cidrs;
    }

    auto response = api_client_->call_api("create_vpc_connection", params);
    return response.contains("return") && response["return"];
}

bool AviatrixIntegration::delete_vpc_connection(const std::string& conn_name) {
    if (!is_connected()) {
        return false;
    }
    
    nlohmann::json params = {{"connection_name", conn_name}};
    auto response = api_client_->call_api("delete_vpc_connection", params);
    return response.contains("return") && response["return"];
}

bool AviatrixIntegration::update_vpc_connection(const std::string& conn_name, const VPCConnection& conn) {
    if (delete_vpc_connection(conn_name)) {
        return create_vpc_connection(conn);
    }
    return false;
}

// Firewall management
std::vector<FirewallRule> AviatrixIntegration::list_firewall_rules() const {
    if (!is_connected()) {
        return {};
    }

    nlohmann::json params = {};
    auto response = api_client_->call_api("list_firewall_rules", params);
    
    std::vector<FirewallRule> rules;
    if (response.contains("results") && response["results"].is_array()) {
        for (const auto& rule_data : response["results"]) {
            rules.push_back(parse_firewall_rule_response(rule_data));
        }
    }
    
    return rules;
}

FirewallRule AviatrixIntegration::get_firewall_rule(const std::string& rule_name) const {
    if (!is_connected()) {
        return {};
    }

    nlohmann::json params = {{"rule_name", rule_name}};
    auto response = api_client_->call_api("get_firewall_rule", params);
    
    if (response.contains("results")) {
        return parse_firewall_rule_response(response["results"]);
    }
    
    return {};
}

bool AviatrixIntegration::create_firewall_rule(const FirewallRule& rule) {
    if (!is_connected()) {
        return false;
    }
    
    nlohmann::json params = {
        {"rule_name", rule.rule_name},
        {"src_ip", rule.src_ip},
        {"dst_ip", rule.dst_ip},
        {"protocol", rule.protocol},
        {"port", rule.port},
        {"action", rule.action},
        {"log_enabled", rule.log_enabled},
        {"description", rule.description}
    };

    auto response = api_client_->call_api("create_firewall_rule", params);
    return response.contains("return") && response["return"];
}

bool AviatrixIntegration::delete_firewall_rule(const std::string& rule_name) {
    if (!is_connected()) {
        return false;
    }
    
    nlohmann::json params = {{"rule_name", rule_name}};
    auto response = api_client_->call_api("delete_firewall_rule", params);
    return response.contains("return") && response["return"];
}

bool AviatrixIntegration::update_firewall_rule(const std::string& rule_name, const FirewallRule& rule) {
    if (delete_firewall_rule(rule_name)) {
        return create_firewall_rule(rule);
    }
    return false;
}

// Network Domain management
std::vector<NetworkDomain> AviatrixIntegration::list_network_domains() const {
    if (!is_connected()) {
        return {};
    }

    nlohmann::json params = {};
    auto response = api_client_->call_api("list_network_domains", params);
    
    std::vector<NetworkDomain> domains;
    if (response.contains("results") && response["results"].is_array()) {
        for (const auto& domain_data : response["results"]) {
            domains.push_back(parse_network_domain_response(domain_data));
        }
    }
    
    return domains;
}

NetworkDomain AviatrixIntegration::get_network_domain(const std::string& domain_name) const {
    if (!is_connected()) {
        return {};
    }

    nlohmann::json params = {{"domain_name", domain_name}};
    auto response = api_client_->call_api("get_network_domain", params);
    
    if (response.contains("results")) {
        return parse_network_domain_response(response["results"]);
    }
    
    return {};
}

bool AviatrixIntegration::create_network_domain(const NetworkDomain& domain) {
    if (!is_connected()) {
        return false;
    }

    nlohmann::json params = {
        {"domain_name", domain.domain_name},
        {"domain_type", domain.domain_type},
        {"attached_gateways", domain.attached_gateways},
        {"policies", domain.policies}
    };

    auto response = api_client_->call_api("create_network_domain", params);
    return response.contains("return") && response["return"];
}

bool AviatrixIntegration::delete_network_domain(const std::string& domain_name) {
    if (!is_connected()) {
        return false;
    }

    nlohmann::json params = {{"domain_name", domain_name}};
    auto response = api_client_->call_api("delete_network_domain", params);
    return response.contains("return") && response["return"];
}

bool AviatrixIntegration::update_network_domain(const std::string& domain_name, const NetworkDomain& domain) {
    if (delete_network_domain(domain_name)) {
        return create_network_domain(domain);
    }
    return false;
}

// CoPilot integration
nlohmann::json AviatrixIntegration::get_copilot_metrics(const std::string& resource_id,
                                                       const std::string& metric_name,
                                                       const std::string& start_time,
                                                       const std::string& end_time) const {
    if (!is_connected()) {
        return nlohmann::json{{"error", "Not connected"}};
    }

    nlohmann::json params = {
        {"resource_id", resource_id},
        {"metric_name", metric_name},
        {"start_time", start_time},
        {"end_time", end_time}
    };

    return api_client_->call_api("get_copilot_metrics", params);
}

std::vector<nlohmann::json> AviatrixIntegration::get_copilot_logs(const std::string& resource_id,
                                                                 const std::string& log_group,
                                                                 const std::string& start_time,
                                                                 const std::string& end_time) const {
    if (!is_connected()) {
        return {};
    }

    nlohmann::json params = {
        {"resource_id", resource_id},
        {"log_group", log_group},
        {"start_time", start_time},
        {"end_time", end_time}
    };

    auto response = api_client_->call_api("get_copilot_logs", params);
    std::vector<nlohmann::json> logs;
    
    if (response.contains("results") && response["results"].is_array()) {
        for (const auto& log_entry : response["results"]) {
            logs.push_back(log_entry);
        }
    }
    
    return logs;
}

// Multi-cloud transit
bool AviatrixIntegration::create_multi_cloud_transit(const std::string& transit_name,
                                                    const std::vector<std::string>& cloud_accounts) {
    if (!is_connected()) {
        return false;
    }

    nlohmann::json params = {
        {"transit_name", transit_name},
        {"cloud_accounts", cloud_accounts}
    };

    auto response = api_client_->call_api("create_multi_cloud_transit", params);
    return response.contains("return") && response["return"];
}

bool AviatrixIntegration::delete_multi_cloud_transit(const std::string& transit_name) {
    if (!is_connected()) {
        return false;
    }

    nlohmann::json params = {{"transit_name", transit_name}};
    auto response = api_client_->call_api("delete_multi_cloud_transit", params);
    return response.contains("return") && response["return"];
}

// Secure connectivity
bool AviatrixIntegration::create_site2cloud_connection(const std::string& conn_name,
                                                      const std::string& vpc_id,
                                                      const std::string& remote_gateway_ip,
                                                      const std::string& pre_shared_key) {
    if (!is_connected()) {
        return false;
    }

    nlohmann::json params = {
        {"connection_name", conn_name},
        {"vpc_id", vpc_id},
        {"remote_gateway_ip", remote_gateway_ip},
        {"pre_shared_key", pre_shared_key}
    };

    auto response = api_client_->call_api("create_site2cloud_connection", params);
    return response.contains("return") && response["return"];
}

bool AviatrixIntegration::delete_site2cloud_connection(const std::string& conn_name) {
    if (!is_connected()) {
        return false;
    }

    nlohmann::json params = {{"connection_name", conn_name}};
    auto response = api_client_->call_api("delete_site2cloud_connection", params);
    return response.contains("return") && response["return"];
}

// Network segmentation
bool AviatrixIntegration::create_network_segmentation(const std::string& segment_name,
                                                     const std::vector<std::string>& attached_gateways,
                                                     const std::map<std::string, std::string>& policies) {
    if (!is_connected()) {
        return false;
    }

    nlohmann::json params = {
        {"segment_name", segment_name},
        {"attached_gateways", attached_gateways},
        {"policies", policies}
    };

    auto response = api_client_->call_api("create_network_segmentation", params);
    return response.contains("return") && response["return"];
}

bool AviatrixIntegration::delete_network_segmentation(const std::string& segment_name) {
    if (!is_connected()) {
        return false;
    }

    nlohmann::json params = {{"segment_name", segment_name}};
    auto response = api_client_->call_api("delete_network_segmentation", params);
    return response.contains("return") && response["return"];
}

// Event callbacks
void AviatrixIntegration::set_gateway_change_callback(GatewayChangeCallback callback) {
    gateway_change_callback_ = callback;
}

void AviatrixIntegration::set_connection_change_callback(ConnectionChangeCallback callback) {
    connection_change_callback_ = callback;
}

// Private helper methods
std::string AviatrixIntegration::build_endpoint(const std::string& action) const {
    return "https://" + config_.controller_ip + "/v1/api?action=" + action;
}

std::string AviatrixIntegration::get_transit_gateways_endpoint() const {
    return build_endpoint("list_transit_gateways");
}

std::string AviatrixIntegration::get_spoke_gateways_endpoint() const {
    return build_endpoint("list_spoke_gateways");
}

std::string AviatrixIntegration::get_vpc_connections_endpoint() const {
    return build_endpoint("list_vpc_connections");
}

std::string AviatrixIntegration::get_firewall_rules_endpoint() const {
    return build_endpoint("list_firewall_rules");
}

// Response parsing methods
TransitGateway AviatrixIntegration::parse_transit_gateway_response(const nlohmann::json& json) const {
    TransitGateway tgw;
    tgw.gw_name = json.value("gw_name", "");
    tgw.cloud_type = json.value("cloud_type", "");
    tgw.account_name = json.value("account_name", "");
    tgw.region = json.value("vpc_reg", "");
    tgw.vpc_id = json.value("vpc_id", "");
    tgw.subnet = json.value("subnet", "");
    tgw.gw_size = json.value("gw_size", "");
    tgw.enable_encrypt_peering = json.value("enable_encrypt_peering", false);
    tgw.enable_learned_cidrs_approval = json.value("enable_learned_cidrs_approval", false);
    
    if (json.contains("connected_gateways") && json["connected_gateways"].is_array()) {
        for (const auto& gw : json["connected_gateways"]) {
            tgw.connected_gateways.push_back(gw.get<std::string>());
        }
    }
    
    if (json.contains("tags") && json["tags"].is_object()) {
        for (auto& [key, value] : json["tags"].items()) {
            tgw.tags[key] = value.get<std::string>();
        }
    }
    
    return tgw;
}

SpokeGateway AviatrixIntegration::parse_spoke_gateway_response(const nlohmann::json& json) const {
    SpokeGateway sgw;
    sgw.gw_name = json.value("gw_name", "");
    sgw.cloud_type = json.value("cloud_type", "");
    sgw.account_name = json.value("account_name", "");
    sgw.region = json.value("vpc_reg", "");
    sgw.vpc_id = json.value("vpc_id", "");
    sgw.subnet = json.value("subnet", "");
    sgw.gw_size = json.value("gw_size", "");
    sgw.transit_gw = json.value("transit_gw", "");
    sgw.enable_encrypt_peering = json.value("enable_encrypt_peering", false);
    
    if (json.contains("tags") && json["tags"].is_object()) {
        for (auto& [key, value] : json["tags"].items()) {
            sgw.tags[key] = value.get<std::string>();
        }
    }
    
    return sgw;
}

VPCConnection AviatrixIntegration::parse_vpc_connection_response(const nlohmann::json& json) const {
    VPCConnection conn;
    conn.connection_name = json.value("connection_name", "");
    conn.vpc_id = json.value("vpc_id", "");
    conn.account_name = json.value("account_name", "");
    conn.region = json.value("region", "");
    conn.transit_gateway = json.value("transit_gateway", "");
    conn.spoke_gateway = json.value("spoke_gateway", "");
    conn.connection_type = json.value("connection_type", "");
    conn.enable_learned_cidrs_approval = json.value("enable_learned_cidrs_approval", false);
    
    if (json.contains("approved_cidrs") && json["approved_cidrs"].is_array()) {
        for (const auto& cidr : json["approved_cidrs"]) {
            conn.approved_cidrs.push_back(cidr.get<std::string>());
        }
    }
    
    return conn;
}

FirewallRule AviatrixIntegration::parse_firewall_rule_response(const nlohmann::json& json) const {
    FirewallRule rule;
    rule.rule_name = json.value("rule_name", "");
    rule.src_ip = json.value("src_ip", "");
    rule.dst_ip = json.value("dst_ip", "");
    rule.protocol = json.value("protocol", "");
    rule.port = json.value("port", 0);
    rule.action = json.value("action", "");
    rule.log_enabled = json.value("log_enabled", "");
    rule.description = json.value("description", "");
    return rule;
}

NetworkDomain AviatrixIntegration::parse_network_domain_response(const nlohmann::json& json) const {
    NetworkDomain domain;
    domain.domain_name = json.value("domain_name", "");
    domain.domain_type = json.value("domain_type", "");
    
    if (json.contains("attached_gateways") && json["attached_gateways"].is_array()) {
        for (const auto& gw : json["attached_gateways"]) {
            domain.attached_gateways.push_back(gw.get<std::string>());
        }
    }
    
    if (json.contains("policies") && json["policies"].is_object()) {
        for (auto& [key, value] : json["policies"].items()) {
            domain.policies[key] = value.get<std::string>();
        }
    }
    
    return domain;
}

void AviatrixIntegration::monitor_loop() {
    while (monitor_running_) {
        std::this_thread::sleep_for(std::chrono::seconds(30));
        
        // Check connection status
        if (!is_connected()) {
            if (authenticate()) {
                connected_ = true;
                authenticated_ = true;
            }
        }
    }
}

} // namespace RouterSim
