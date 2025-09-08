#include "aviatrix_client.h"
#include <iostream>
#include <curl/curl.h>
#include <json/json.h>
#include <sstream>

namespace router_sim::cloud {

AviatrixClient::AviatrixClient(const std::string& controller_ip, const std::string& username, const std::string& password)
    : controller_ip_(controller_ip), username_(username), password_(password) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

AviatrixClient::~AviatrixClient() {
    curl_global_cleanup();
}

bool AviatrixClient::initialize() {
    // Login to Aviatrix controller
    if (!login()) {
        std::cerr << "Failed to login to Aviatrix controller" << std::endl;
        return false;
    }
    
    return true;
}

bool AviatrixClient::login() {
    Json::Value login_data;
    login_data["action"] = "login";
    login_data["username"] = username_;
    login_data["password"] = password_;
    
    Json::StreamWriterBuilder writer_builder;
    std::string json_string = Json::writeString(writer_builder, login_data);
    
    std::string response;
    if (!make_request("POST", "/v1/api", json_string, response)) {
        return false;
    }
    
    Json::Value root;
    Json::CharReaderBuilder builder;
    std::string errors;
    
    std::istringstream stream(response);
    if (!Json::parseFromStream(builder, stream, &root, &errors)) {
        std::cerr << "Failed to parse login response: " << errors << std::endl;
        return false;
    }
    
    if (root.isMember("return") && root["return"].asBool()) {
        // Extract CID from response
        if (root.isMember("CID")) {
            cid_ = root["CID"].asString();
        }
        return true;
    }
    
    return false;
}

bool AviatrixClient::create_gateway(const AviatrixGateway& gateway) {
    Json::Value gateway_data;
    gateway_data["action"] = "create_gateway";
    gateway_data["CID"] = cid_;
    gateway_data["gw_name"] = gateway.gw_name;
    gateway_data["cloud_type"] = gateway.cloud_type;
    gateway_data["account_name"] = gateway.account_name;
    gateway_data["region"] = gateway.region;
    gateway_data["vpc_id"] = gateway.vpc_id;
    gateway_data["subnet"] = gateway.subnet;
    gateway_data["gw_size"] = gateway.gw_size;
    gateway_data["enable_vpn_access"] = gateway.enable_vpn_access;
    gateway_data["enable_elb"] = gateway.enable_elb;
    
    if (!gateway.tags.empty()) {
        Json::Value tags(Json::objectValue);
        for (const auto& pair : gateway.tags) {
            tags[pair.first] = pair.second;
        }
        gateway_data["tags"] = tags;
    }
    
    Json::StreamWriterBuilder writer_builder;
    std::string json_string = Json::writeString(writer_builder, gateway_data);
    
    std::string response;
    return make_request("POST", "/v1/api", json_string, response);
}

bool AviatrixClient::create_transit_gateway(const AviatrixTransitGateway& transit_gw) {
    Json::Value transit_data;
    transit_data["action"] = "create_transit_gateway";
    transit_data["CID"] = cid_;
    transit_data["gw_name"] = transit_gw.gw_name;
    transit_data["cloud_type"] = transit_gw.cloud_type;
    transit_data["account_name"] = transit_gw.account_name;
    transit_data["region"] = transit_gw.region;
    transit_data["vpc_id"] = transit_gw.vpc_id;
    transit_data["subnet"] = transit_gw.subnet;
    transit_data["gw_size"] = transit_gw.gw_size;
    transit_data["enable_hybrid_connection"] = transit_gw.enable_hybrid_connection;
    transit_data["enable_firenet"] = transit_gw.enable_firenet;
    
    if (!transit_gw.tags.empty()) {
        Json::Value tags(Json::objectValue);
        for (const auto& pair : transit_gw.tags) {
            tags[pair.first] = pair.second;
        }
        transit_data["tags"] = tags;
    }
    
    Json::StreamWriterBuilder writer_builder;
    std::string json_string = Json::writeString(writer_builder, transit_data);
    
    std::string response;
    return make_request("POST", "/v1/api", json_string, response);
}

bool AviatrixClient::create_spoke_gateway(const AviatrixSpokeGateway& spoke_gw) {
    Json::Value spoke_data;
    spoke_data["action"] = "create_spoke_gateway";
    spoke_data["CID"] = cid_;
    spoke_data["gw_name"] = spoke_gw.gw_name;
    spoke_data["cloud_type"] = spoke_gw.cloud_type;
    spoke_data["account_name"] = spoke_gw.account_name;
    spoke_data["region"] = spoke_gw.region;
    spoke_data["vpc_id"] = spoke_gw.vpc_id;
    spoke_data["subnet"] = spoke_gw.subnet;
    spoke_data["gw_size"] = spoke_gw.gw_size;
    spoke_data["transit_gw"] = spoke_gw.transit_gw;
    spoke_data["enable_vpn_access"] = spoke_gw.enable_vpn_access;
    
    if (!spoke_gw.tags.empty()) {
        Json::Value tags(Json::objectValue);
        for (const auto& pair : spoke_gw.tags) {
            tags[pair.first] = pair.second;
        }
        spoke_data["tags"] = tags;
    }
    
    Json::StreamWriterBuilder writer_builder;
    std::string json_string = Json::writeString(writer_builder, spoke_data);
    
    std::string response;
    return make_request("POST", "/v1/api", json_string, response);
}

bool AviatrixClient::create_connection(const AviatrixConnection& connection) {
    Json::Value conn_data;
    conn_data["action"] = "create_connection";
    conn_data["CID"] = cid_;
    conn_data["source_gateway"] = connection.source_gateway;
    conn_data["destination_gateway"] = connection.destination_gateway;
    conn_data["type"] = connection.type;
    conn_data["bandwidth"] = connection.bandwidth;
    
    Json::StreamWriterBuilder writer_builder;
    std::string json_string = Json::writeString(writer_builder, conn_data);
    
    std::string response;
    return make_request("POST", "/v1/api", json_string, response);
}

std::vector<AviatrixGateway> AviatrixClient::list_gateways() {
    std::vector<AviatrixGateway> gateways;
    
    Json::Value list_data;
    list_data["action"] = "list_gateways";
    list_data["CID"] = cid_;
    
    Json::StreamWriterBuilder writer_builder;
    std::string json_string = Json::writeString(writer_builder, list_data);
    
    std::string response;
    if (!make_request("POST", "/v1/api", json_string, response)) {
        return gateways;
    }
    
    Json::Value root;
    Json::CharReaderBuilder builder;
    std::string errors;
    
    std::istringstream stream(response);
    if (!Json::parseFromStream(builder, stream, &root, &errors)) {
        std::cerr << "Failed to parse gateways response: " << errors << std::endl;
        return gateways;
    }
    
    if (root.isMember("return") && root["return"].isBool() && root["return"].asBool()) {
        if (root.isMember("results") && root["results"].isArray()) {
            for (const auto& gw_json : root["results"]) {
                AviatrixGateway gateway;
                gateway.gw_name = gw_json.get("gw_name", "").asString();
                gateway.cloud_type = gw_json.get("cloud_type", 0).asInt();
                gateway.account_name = gw_json.get("account_name", "").asString();
                gateway.region = gw_json.get("region", "").asString();
                gateway.vpc_id = gw_json.get("vpc_id", "").asString();
                gateway.subnet = gw_json.get("subnet", "").asString();
                gateway.gw_size = gw_json.get("gw_size", "").asString();
                gateway.public_ip = gw_json.get("public_ip", "").asString();
                gateway.private_ip = gw_json.get("private_ip", "").asString();
                gateway.status = gw_json.get("status", "").asString();
                gateway.created_at = gw_json.get("created_at", "").asString();
                
                if (gw_json.isMember("tags") && gw_json["tags"].isObject()) {
                    const Json::Value& tags = gw_json["tags"];
                    for (const auto& key : tags.getMemberNames()) {
                        gateway.tags[key] = tags[key].asString();
                    }
                }
                
                gateways.push_back(gateway);
            }
        }
    }
    
    return gateways;
}

std::vector<AviatrixConnection> AviatrixClient::list_connections() {
    std::vector<AviatrixConnection> connections;
    
    Json::Value list_data;
    list_data["action"] = "list_connections";
    list_data["CID"] = cid_;
    
    Json::StreamWriterBuilder writer_builder;
    std::string json_string = Json::writeString(writer_builder, list_data);
    
    std::string response;
    if (!make_request("POST", "/v1/api", json_string, response)) {
        return connections;
    }
    
    Json::Value root;
    Json::CharReaderBuilder builder;
    std::string errors;
    
    std::istringstream stream(response);
    if (!Json::parseFromStream(builder, stream, &root, &errors)) {
        std::cerr << "Failed to parse connections response: " << errors << std::endl;
        return connections;
    }
    
    if (root.isMember("return") && root["return"].isBool() && root["return"].asBool()) {
        if (root.isMember("results") && root["results"].isArray()) {
            for (const auto& conn_json : root["results"]) {
                AviatrixConnection connection;
                connection.id = conn_json.get("id", "").asString();
                connection.name = conn_json.get("name", "").asString();
                connection.source_gateway = conn_json.get("source_gateway", "").asString();
                connection.destination_gateway = conn_json.get("destination_gateway", "").asString();
                connection.type = conn_json.get("type", "").asString();
                connection.status = conn_json.get("status", "").asString();
                connection.bandwidth = conn_json.get("bandwidth", 0).asInt();
                connection.latency = conn_json.get("latency", 0.0).asDouble();
                connection.created_at = conn_json.get("created_at", "").asString();
                
                connections.push_back(connection);
            }
        }
    }
    
    return connections;
}

bool AviatrixClient::delete_gateway(const std::string& gw_name) {
    Json::Value delete_data;
    delete_data["action"] = "delete_gateway";
    delete_data["CID"] = cid_;
    delete_data["gw_name"] = gw_name;
    
    Json::StreamWriterBuilder writer_builder;
    std::string json_string = Json::writeString(writer_builder, delete_data);
    
    std::string response;
    return make_request("POST", "/v1/api", json_string, response);
}

bool AviatrixClient::delete_connection(const std::string& connection_id) {
    Json::Value delete_data;
    delete_data["action"] = "delete_connection";
    delete_data["CID"] = cid_;
    delete_data["connection_id"] = connection_id;
    
    Json::StreamWriterBuilder writer_builder;
    std::string json_string = Json::writeString(writer_builder, delete_data);
    
    std::string response;
    return make_request("POST", "/v1/api", json_string, response);
}

bool AviatrixClient::make_request(const std::string& method, const std::string& path, 
                                 const std::string& data, std::string& response) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        return false;
    }
    
    std::string url = "https://" + controller_ip_ + path;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "RouterSim/1.0");
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    
    // Set headers
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    
    if (method == "POST") {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data.length());
    }
    
    CURLcode res = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    return (res == CURLE_OK && http_code >= 200 && http_code < 300);
}

size_t AviatrixClient::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t total_size = size * nmemb;
    std::string* response = static_cast<std::string*>(userp);
    response->append(static_cast<char*>(contents), total_size);
    return total_size;
}

} // namespace router_sim::cloud
