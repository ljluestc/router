#include "clickhouse_client.h"
#include <curl/curl.h>
#include <json/json.h>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace router_sim {
namespace analytics {

ClickHouseClient::ClickHouseClient(const std::string& host, 
                                 uint16_t port,
                                 const std::string& database,
                                 const std::string& username,
                                 const std::string& password)
    : host_(host)
    , port_(port)
    , database_(database)
    , username_(username)
    , password_(password)
    , connected_(false)
    , insert_count_(0)
    , query_count_(0)
    , total_insert_time_ms_(0.0)
    , total_query_time_ms_(0.0)
    , batch_size_(1000)
    , flush_interval_(std::chrono::milliseconds(100))
    , max_queue_size_(10000)
    , stop_workers_(false)
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

ClickHouseClient::~ClickHouseClient() {
    stop_workers_ = true;
    
    if (metrics_worker_.joinable()) {
        metrics_cv_.notify_all();
        metrics_worker_.join();
    }
    
    if (packet_metrics_worker_.joinable()) {
        packet_metrics_cv_.notify_all();
        packet_metrics_worker_.join();
    }
    
    if (routing_metrics_worker_.joinable()) {
        routing_metrics_cv_.notify_all();
        routing_metrics_worker_.join();
    }
    
    disconnect();
    curl_global_cleanup();
}

bool ClickHouseClient::connect() {
    if (connected_) {
        return true;
    }
    
    // Test connection
    std::string test_query = "SELECT 1";
    if (!execute_query(test_query)) {
        return false;
    }
    
    connected_ = true;
    
    // Create tables
    create_tables();
    
    // Start worker threads
    stop_workers_ = false;
    metrics_worker_ = std::thread(&ClickHouseClient::metrics_worker_loop, this);
    packet_metrics_worker_ = std::thread(&ClickHouseClient::packet_metrics_worker_loop, this);
    routing_metrics_worker_ = std::thread(&ClickHouseClient::routing_metrics_worker_loop, this);
    
    return true;
}

void ClickHouseClient::disconnect() {
    if (!connected_) {
        return;
    }
    
    stop_workers_ = true;
    connected_ = false;
}

bool ClickHouseClient::is_connected() const {
    return connected_;
}

bool ClickHouseClient::insert_metrics(const std::vector<MetricData>& metrics) {
    if (!connected_ || metrics.empty()) {
        return false;
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    
    std::string data = serialize_metrics(metrics);
    bool result = execute_insert("metrics", data);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    if (result) {
        insert_count_ += metrics.size();
        total_insert_time_ms_ += duration.count() / 1000.0;
    }
    
    return result;
}

bool ClickHouseClient::insert_packet_metrics(const std::vector<PacketMetrics>& metrics) {
    if (!connected_ || metrics.empty()) {
        return false;
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    
    std::string data = serialize_packet_metrics(metrics);
    bool result = execute_insert("packet_metrics", data);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    if (result) {
        insert_count_ += metrics.size();
        total_insert_time_ms_ += duration.count() / 1000.0;
    }
    
    return result;
}

bool ClickHouseClient::insert_routing_metrics(const std::vector<RoutingMetrics>& metrics) {
    if (!connected_ || metrics.empty()) {
        return false;
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    
    std::string data = serialize_routing_metrics(metrics);
    bool result = execute_insert("routing_metrics", data);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    if (result) {
        insert_count_ += metrics.size();
        total_insert_time_ms_ += duration.count() / 1000.0;
    }
    
    return result;
}

void ClickHouseClient::async_insert_metrics(const std::vector<MetricData>& metrics) {
    if (!connected_ || metrics.empty()) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    if (metrics_queue_.size() < max_queue_size_) {
        metrics_queue_.push(metrics);
        metrics_cv_.notify_one();
    }
}

void ClickHouseClient::async_insert_packet_metrics(const std::vector<PacketMetrics>& metrics) {
    if (!connected_ || metrics.empty()) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(packet_metrics_mutex_);
    if (packet_metrics_queue_.size() < max_queue_size_) {
        packet_metrics_queue_.push(metrics);
        packet_metrics_cv_.notify_one();
    }
}

void ClickHouseClient::async_insert_routing_metrics(const std::vector<RoutingMetrics>& metrics) {
    if (!connected_ || metrics.empty()) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(routing_metrics_mutex_);
    if (routing_metrics_queue_.size() < max_queue_size_) {
        routing_metrics_queue_.push(metrics);
        routing_metrics_cv_.notify_one();
    }
}

std::vector<MetricData> ClickHouseClient::query_metrics(const std::string& query) {
    // Implementation for querying metrics
    // This would parse the ClickHouse response and return MetricData objects
    return {};
}

std::vector<PacketMetrics> ClickHouseClient::query_packet_metrics(const std::string& query) {
    // Implementation for querying packet metrics
    return {};
}

std::vector<RoutingMetrics> ClickHouseClient::query_routing_metrics(const std::string& query) {
    // Implementation for querying routing metrics
    return {};
}

uint64_t ClickHouseClient::get_insert_count() const {
    return insert_count_.load();
}

uint64_t ClickHouseClient::get_query_count() const {
    return query_count_.load();
}

double ClickHouseClient::get_avg_insert_time_ms() const {
    uint64_t count = insert_count_.load();
    return count > 0 ? total_insert_time_ms_.load() / count : 0.0;
}

double ClickHouseClient::get_avg_query_time_ms() const {
    uint64_t count = query_count_.load();
    return count > 0 ? total_query_time_ms_.load() / count : 0.0;
}

void ClickHouseClient::set_batch_size(size_t size) {
    batch_size_ = size;
}

void ClickHouseClient::set_flush_interval(std::chrono::milliseconds interval) {
    flush_interval_ = interval;
}

void ClickHouseClient::set_max_queue_size(size_t size) {
    max_queue_size_ = size;
}

void ClickHouseClient::metrics_worker_loop() {
    std::vector<MetricData> batch;
    batch.reserve(batch_size_);
    
    while (!stop_workers_) {
        std::unique_lock<std::mutex> lock(metrics_mutex_);
        
        if (metrics_queue_.empty()) {
            metrics_cv_.wait_for(lock, flush_interval_);
            if (metrics_queue_.empty() && !batch.empty()) {
                // Flush remaining batch
                insert_metrics(batch);
                batch.clear();
            }
            continue;
        }
        
        // Collect batch
        while (!metrics_queue_.empty() && batch.size() < batch_size_) {
            const auto& metrics = metrics_queue_.front();
            batch.insert(batch.end(), metrics.begin(), metrics.end());
            metrics_queue_.pop();
        }
        
        lock.unlock();
        
        if (!batch.empty()) {
            insert_metrics(batch);
            batch.clear();
        }
    }
}

void ClickHouseClient::packet_metrics_worker_loop() {
    std::vector<PacketMetrics> batch;
    batch.reserve(batch_size_);
    
    while (!stop_workers_) {
        std::unique_lock<std::mutex> lock(packet_metrics_mutex_);
        
        if (packet_metrics_queue_.empty()) {
            packet_metrics_cv_.wait_for(lock, flush_interval_);
            if (packet_metrics_queue_.empty() && !batch.empty()) {
                insert_packet_metrics(batch);
                batch.clear();
            }
            continue;
        }
        
        while (!packet_metrics_queue_.empty() && batch.size() < batch_size_) {
            const auto& metrics = packet_metrics_queue_.front();
            batch.insert(batch.end(), metrics.begin(), metrics.end());
            packet_metrics_queue_.pop();
        }
        
        lock.unlock();
        
        if (!batch.empty()) {
            insert_packet_metrics(batch);
            batch.clear();
        }
    }
}

void ClickHouseClient::routing_metrics_worker_loop() {
    std::vector<RoutingMetrics> batch;
    batch.reserve(batch_size_);
    
    while (!stop_workers_) {
        std::unique_lock<std::mutex> lock(routing_metrics_mutex_);
        
        if (routing_metrics_queue_.empty()) {
            routing_metrics_cv_.wait_for(lock, flush_interval_);
            if (routing_metrics_queue_.empty() && !batch.empty()) {
                insert_routing_metrics(batch);
                batch.clear();
            }
            continue;
        }
        
        while (!routing_metrics_queue_.empty() && batch.size() < batch_size_) {
            const auto& metrics = routing_metrics_queue_.front();
            batch.insert(batch.end(), metrics.begin(), metrics.end());
            routing_metrics_queue_.pop();
        }
        
        lock.unlock();
        
        if (!batch.empty()) {
            insert_routing_metrics(batch);
            batch.clear();
        }
    }
}

bool ClickHouseClient::execute_query(const std::string& query) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        return false;
    }
    
    std::string url = "http://" + host_ + ":" + std::to_string(port_) + "/";
    std::string post_data = query;
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, post_data.length());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, [](void* contents, size_t size, size_t nmemb, void* userp) {
        return size * nmemb;
    });
    
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    
    return res == CURLE_OK;
}

bool ClickHouseClient::execute_insert(const std::string& table, const std::string& data) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        return false;
    }
    
    std::string url = "http://" + host_ + ":" + std::to_string(port_) + "/?database=" + database_ + "&query=INSERT INTO " + table + " FORMAT JSONEachRow";
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data.length());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, [](void* contents, size_t size, size_t nmemb, void* userp) {
        return size * nmemb;
    });
    
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    
    return res == CURLE_OK;
}

std::string ClickHouseClient::serialize_metrics(const std::vector<MetricData>& metrics) {
    std::ostringstream oss;
    
    for (const auto& metric : metrics) {
        Json::Value json;
        json["name"] = metric.name;
        json["value"] = metric.value;
        json["labels"] = metric.labels;
        json["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
            metric.timestamp.time_since_epoch()).count();
        json["source"] = metric.source;
        json["type"] = metric.type;
        
        Json::StreamWriterBuilder builder;
        builder["indentation"] = "";
        std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
        writer->write(json, &oss);
        oss << "\n";
    }
    
    return oss.str();
}

std::string ClickHouseClient::serialize_packet_metrics(const std::vector<PacketMetrics>& metrics) {
    std::ostringstream oss;
    
    for (const auto& metric : metrics) {
        Json::Value json;
        json["total_packets"] = metric.total_packets;
        json["bytes_transferred"] = metric.bytes_transferred;
        json["packets_dropped"] = metric.packets_dropped;
        json["packets_duplicated"] = metric.packets_duplicated;
        json["packets_reordered"] = metric.packets_reordered;
        json["avg_latency_ms"] = metric.avg_latency_ms;
        json["max_latency_ms"] = metric.max_latency_ms;
        json["min_latency_ms"] = metric.min_latency_ms;
        json["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
            metric.timestamp.time_since_epoch()).count();
        
        Json::StreamWriterBuilder builder;
        builder["indentation"] = "";
        std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
        writer->write(json, &oss);
        oss << "\n";
    }
    
    return oss.str();
}

std::string ClickHouseClient::serialize_routing_metrics(const std::vector<RoutingMetrics>& metrics) {
    std::ostringstream oss;
    
    for (const auto& metric : metrics) {
        Json::Value json;
        json["total_routes"] = metric.total_routes;
        json["bgp_routes"] = metric.bgp_routes;
        json["ospf_routes"] = metric.ospf_routes;
        json["isis_routes"] = metric.isis_routes;
        json["static_routes"] = metric.static_routes;
        json["route_changes"] = metric.route_changes;
        json["convergence_time_ms"] = metric.convergence_time_ms;
        json["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
            metric.timestamp.time_since_epoch()).count();
        
        Json::StreamWriterBuilder builder;
        builder["indentation"] = "";
        std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
        writer->write(json, &oss);
        oss << "\n";
    }
    
    return oss.str();
}

void ClickHouseClient::create_tables() {
    create_metrics_table();
    create_packet_metrics_table();
    create_routing_metrics_table();
}

void ClickHouseClient::create_metrics_table() {
    std::string query = R"(
        CREATE TABLE IF NOT EXISTS metrics (
            name String,
            value String,
            labels String,
            timestamp UInt64,
            source String,
            type String
        ) ENGINE = MergeTree()
        ORDER BY (timestamp, name)
    )";
    
    execute_query(query);
}

void ClickHouseClient::create_packet_metrics_table() {
    std::string query = R"(
        CREATE TABLE IF NOT EXISTS packet_metrics (
            total_packets UInt64,
            bytes_transferred UInt64,
            packets_dropped UInt64,
            packets_duplicated UInt64,
            packets_reordered UInt64,
            avg_latency_ms Float64,
            max_latency_ms Float64,
            min_latency_ms Float64,
            timestamp UInt64
        ) ENGINE = MergeTree()
        ORDER BY timestamp
    )";
    
    execute_query(query);
}

void ClickHouseClient::create_routing_metrics_table() {
    std::string query = R"(
        CREATE TABLE IF NOT EXISTS routing_metrics (
            total_routes UInt64,
            bgp_routes UInt64,
            ospf_routes UInt64,
            isis_routes UInt64,
            static_routes UInt64,
            route_changes UInt64,
            convergence_time_ms Float64,
            timestamp UInt64
        ) ENGINE = MergeTree()
        ORDER BY timestamp
    )";
    
    execute_query(query);
}

} // namespace analytics
} // namespace router_sim
