#include "cloudpods_client.h"
#include <iostream>
#include <curl/curl.h>
#include <json/json.h>
#include <sstream>

namespace router_sim::cloud {

CloudPodsClient::CloudPodsClient(const std::string& endpoint, const std::string& api_key)
    : endpoint_(endpoint), api_key_(api_key) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

CloudPodsClient::~CloudPodsClient() {
    curl_global_cleanup();
}

bool CloudPodsClient::initialize() {
    // Test connection to CloudPods API
    std::string response;
    if (!make_request("GET", "/api/v1/status", "", response)) {
        std::cerr << "Failed to connect to CloudPods API" << std::endl;
        return false;
    }
    
    // Parse response and verify API is accessible
    Json::Value root;
    Json::CharReaderBuilder builder;
    std::string errors;
    
    std::istringstream stream(response);
    if (!Json::parseFromStream(builder, stream, &root, &errors)) {
        std::cerr << "Failed to parse CloudPods API response: " << errors << std::endl;
        return false;
    }
    
    return true;
}

bool CloudPodsClient::create_instance(const CloudPodsInstance& instance) {
    Json::Value instance_json;
    instance_json["name"] = instance.name;
    instance_json["image_id"] = instance.image_id;
    instance_json["flavor_id"] = instance.flavor_id;
    instance_json["network_id"] = instance.network_id;
    instance_json["security_group_ids"] = Json::Value(Json::arrayValue);
    for (const auto& sg_id : instance.security_group_ids) {
        instance_json["security_group_ids"].append(sg_id);
    }
    instance_json["keypair"] = instance.keypair;
    
    Json::Value tags(Json::objectValue);
    for (const auto& pair : instance.tags) {
        tags[pair.first] = pair.second;
    }
    instance_json["tags"] = tags;
    
    Json::StreamWriterBuilder writer_builder;
    std::string json_string = Json::writeString(writer_builder, instance_json);
    
    std::string response;
    return make_request("POST", "/api/v1/instances", json_string, response);
}

bool CloudPodsClient::create_network(const CloudPodsNetwork& network) {
    Json::Value network_json;
    network_json["name"] = network.name;
    network_json["cidr"] = network.cidr;
    network_json["vpc_id"] = network.vpc_id;
    network_json["zone_id"] = network.zone_id;
    
    Json::Value tags(Json::objectValue);
    for (const auto& pair : network.tags) {
        tags[pair.first] = pair.second;
    }
    network_json["tags"] = tags;
    
    Json::StreamWriterBuilder writer_builder;
    std::string json_string = Json::writeString(writer_builder, network_json);
    
    std::string response;
    return make_request("POST", "/api/v1/networks", json_string, response);
}

bool CloudPodsClient::create_loadbalancer(const CloudPodsLoadBalancer& lb) {
    Json::Value lb_json;
    lb_json["name"] = lb.name;
    lb_json["network_id"] = lb.network_id;
    
    Json::Value listeners(Json::arrayValue);
    for (const auto& listener : lb.listeners) {
        Json::Value listener_json;
        listener_json["port"] = listener.port;
        listener_json["protocol"] = listener.protocol;
        listeners.append(listener_json);
    }
    lb_json["listeners"] = listeners;
    
    Json::Value backend_groups(Json::arrayValue);
    for (const auto& bg : lb.backend_groups) {
        Json::Value bg_json;
        bg_json["name"] = bg.name;
        bg_json["protocol"] = bg.protocol;
        bg_json["port"] = bg.port;
        backend_groups.append(bg_json);
    }
    lb_json["backend_groups"] = backend_groups;
    
    Json::Value tags(Json::objectValue);
    for (const auto& pair : lb.tags) {
        tags[pair.first] = pair.second;
    }
    lb_json["tags"] = tags;
    
    Json::StreamWriterBuilder writer_builder;
    std::string json_string = Json::writeString(writer_builder, lb_json);
    
    std::string response;
    return make_request("POST", "/api/v1/loadbalancers", json_string, response);
}

std::vector<CloudPodsInstance> CloudPodsClient::list_instances() {
    std::vector<CloudPodsInstance> instances;
    
    std::string response;
    if (!make_request("GET", "/api/v1/instances", "", response)) {
        return instances;
    }
    
    Json::Value root;
    Json::CharReaderBuilder builder;
    std::string errors;
    
    std::istringstream stream(response);
    if (!Json::parseFromStream(builder, stream, &root, &errors)) {
        std::cerr << "Failed to parse instances response: " << errors << std::endl;
        return instances;
    }
    
    if (root.isMember("instances") && root["instances"].isArray()) {
        for (const auto& instance_json : root["instances"]) {
            CloudPodsInstance instance;
            instance.id = instance_json.get("id", "").asString();
            instance.name = instance_json.get("name", "").asString();
            instance.image_id = instance_json.get("image_id", "").asString();
            instance.flavor_id = instance_json.get("flavor_id", "").asString();
            instance.network_id = instance_json.get("network_id", "").asString();
            instance.keypair = instance_json.get("keypair", "").asString();
            instance.status = instance_json.get("status", "").asString();
            instance.created_at = instance_json.get("created_at", "").asString();
            
            if (instance_json.isMember("security_group_ids") && instance_json["security_group_ids"].isArray()) {
                for (const auto& sg_id : instance_json["security_group_ids"]) {
                    instance.security_group_ids.push_back(sg_id.asString());
                }
            }
            
            if (instance_json.isMember("tags") && instance_json["tags"].isObject()) {
                const Json::Value& tags = instance_json["tags"];
                for (const auto& key : tags.getMemberNames()) {
                    instance.tags[key] = tags[key].asString();
                }
            }
            
            instances.push_back(instance);
        }
    }
    
    return instances;
}

std::vector<CloudPodsNetwork> CloudPodsClient::list_networks() {
    std::vector<CloudPodsNetwork> networks;
    
    std::string response;
    if (!make_request("GET", "/api/v1/networks", "", response)) {
        return networks;
    }
    
    Json::Value root;
    Json::CharReaderBuilder builder;
    std::string errors;
    
    std::istringstream stream(response);
    if (!Json::parseFromStream(builder, stream, &root, &errors)) {
        std::cerr << "Failed to parse networks response: " << errors << std::endl;
        return networks;
    }
    
    if (root.isMember("networks") && root["networks"].isArray()) {
        for (const auto& network_json : root["networks"]) {
            CloudPodsNetwork network;
            network.id = network_json.get("id", "").asString();
            network.name = network_json.get("name", "").asString();
            network.cidr = network_json.get("cidr", "").asString();
            network.vpc_id = network_json.get("vpc_id", "").asString();
            network.zone_id = network_json.get("zone_id", "").asString();
            network.status = network_json.get("status", "").asString();
            network.created_at = network_json.get("created_at", "").asString();
            
            if (network_json.isMember("tags") && network_json["tags"].isObject()) {
                const Json::Value& tags = network_json["tags"];
                for (const auto& key : tags.getMemberNames()) {
                    network.tags[key] = tags[key].asString();
                }
            }
            
            networks.push_back(network);
        }
    }
    
    return networks;
}

bool CloudPodsClient::delete_instance(const std::string& instance_id) {
    std::string response;
    return make_request("DELETE", "/api/v1/instances/" + instance_id, "", response);
}

bool CloudPodsClient::delete_network(const std::string& network_id) {
    std::string response;
    return make_request("DELETE", "/api/v1/networks/" + network_id, "", response);
}

bool CloudPodsClient::delete_loadbalancer(const std::string& lb_id) {
    std::string response;
    return make_request("DELETE", "/api/v1/loadbalancers/" + lb_id, "", response);
}

bool CloudPodsClient::make_request(const std::string& method, const std::string& path, 
                                  const std::string& data, std::string& response) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        return false;
    }
    
    std::string url = endpoint_ + path;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "RouterSim/1.0");
    
    // Set headers
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, ("X-API-Key: " + api_key_).c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    
    if (method == "POST" || method == "PUT") {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data.length());
    }
    
    if (method == "DELETE") {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    }
    
    CURLcode res = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    return (res == CURLE_OK && http_code >= 200 && http_code < 300);
}

size_t CloudPodsClient::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t total_size = size * nmemb;
    std::string* response = static_cast<std::string*>(userp);
    response->append(static_cast<char*>(contents), total_size);
    return total_size;
}

} // namespace router_sim::cloud
