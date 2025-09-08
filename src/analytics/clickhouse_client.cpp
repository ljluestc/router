#include "clickhouse_client.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <iostream>
#include <fstream>

namespace RouterSim {

ClickHouseClient::ClickHouseClient(const Config& config)
    : config_(config)
    , connections_(config.max_connections)
{
}

ClickHouseClient::~ClickHouseClient() {
    shutdown();
}

bool ClickHouseClient::initialize() {
    std::lock_guard<std::mutex> lock(connection_mutex_);
    
    if (connected_.load()) {
        return true;
    }

    // Initialize connections
    for (auto& conn : connections_) {
        conn.socket_fd = -1;
        conn.in_use = false;
        conn.last_used = std::chrono::system_clock::now();
    }

    // Connect to ClickHouse
    if (!connect_to_clickhouse()) {
        return false;
    }

    // Create tables
    if (!create_tables()) {
        disconnect_from_clickhouse();
        return false;
    }

    // Start background worker
    running_.store(true);
    worker_thread_ = std::thread(&ClickHouseClient::worker_loop, this);

    connected_.store(true);
    return true;
}

void ClickHouseClient::shutdown() {
    if (!running_.load()) {
        return;
    }

    running_.store(false);
    queue_cv_.notify_all();

    if (worker_thread_.joinable()) {
        worker_thread_.join();
    }

    // Flush remaining data
    flush_buffers();

    disconnect_from_clickhouse();
    connected_.store(false);
}

bool ClickHouseClient::is_connected() const {
    return connected_.load();
}

bool ClickHouseClient::connect_to_clickhouse() {
    struct sockaddr_in server_addr;
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    
    if (sock_fd < 0) {
        stats_.errors.fetch_add(1);
        return false;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(config_.port);
    
    if (inet_pton(AF_INET, config_.host.c_str(), &server_addr.sin_addr) <= 0) {
        close(sock_fd);
        stats_.errors.fetch_add(1);
        return false;
    }

    if (connect(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        close(sock_fd);
        stats_.errors.fetch_add(1);
        return false;
    }

    // Set non-blocking mode
    int flags = fcntl(sock_fd, F_GETFL, 0);
    fcntl(sock_fd, F_SETFL, flags | O_NONBLOCK);

    // Store connection
    for (auto& conn : connections_) {
        if (conn.socket_fd == -1) {
            conn.socket_fd = sock_fd;
            conn.in_use = false;
            conn.last_used = std::chrono::system_clock::now();
            break;
        }
    }

    return true;
}

void ClickHouseClient::disconnect_from_clickhouse() {
    std::lock_guard<std::mutex> lock(connection_mutex_);
    
    for (auto& conn : connections_) {
        if (conn.socket_fd != -1) {
            close(conn.socket_fd);
            conn.socket_fd = -1;
            conn.in_use = false;
        }
    }
}

ClickHouseClient::Connection* ClickHouseClient::get_connection() {
    std::lock_guard<std::mutex> lock(connection_mutex_);
    
    for (auto& conn : connections_) {
        if (conn.socket_fd != -1 && !conn.in_use) {
            conn.in_use = true;
            conn.last_used = std::chrono::system_clock::now();
            return &conn;
        }
    }
    
    return nullptr;
}

void ClickHouseClient::release_connection(Connection* conn) {
    if (conn) {
        std::lock_guard<std::mutex> lock(connection_mutex_);
        conn->in_use = false;
        conn->last_used = std::chrono::system_clock::now();
    }
}

bool ClickHouseClient::execute_query(const std::string& query, Connection* conn) {
    if (!conn || conn->socket_fd == -1) {
        return false;
    }

    std::string full_query = query + "\n";
    ssize_t bytes_sent = send(conn->socket_fd, full_query.c_str(), full_query.length(), 0);
    
    if (bytes_sent < 0) {
        stats_.errors.fetch_add(1);
        return false;
    }

    // Read response
    char buffer[4096];
    ssize_t bytes_received = recv(conn->socket_fd, buffer, sizeof(buffer) - 1, 0);
    
    if (bytes_received < 0) {
        stats_.errors.fetch_add(1);
        return false;
    }

    buffer[bytes_received] = '\0';
    stats_.queries_executed.fetch_add(1);
    
    return true;
}

bool ClickHouseClient::execute_insert(const std::string& insert_query, Connection* conn) {
    return execute_query(insert_query, conn);
}

void ClickHouseClient::worker_loop() {
    while (running_.load()) {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        
        if (queue_cv_.wait_for(lock, config_.flush_interval, [this] { 
            return !insert_queue_.empty() || !running_.load(); 
        })) {
            // Process queued inserts
            std::vector<std::string> batch;
            batch.reserve(config_.batch_size);
            
            while (!insert_queue_.empty() && batch.size() < config_.batch_size) {
                batch.push_back(std::move(insert_queue_.front()));
                insert_queue_.pop();
            }
            
            lock.unlock();
            
            if (!batch.empty()) {
                flush_batch(batch);
            }
        } else {
            // Timeout - flush any pending data
            flush_buffers();
        }
    }
}

void ClickHouseClient::flush_buffers() {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    
    if (insert_queue_.empty()) {
        return;
    }
    
    std::vector<std::string> batch;
    batch.reserve(insert_queue_.size());
    
    while (!insert_queue_.empty()) {
        batch.push_back(std::move(insert_queue_.front()));
        insert_queue_.pop();
    }
    
    flush_batch(batch);
}

void ClickHouseClient::flush_batch(const std::vector<std::string>& batch) {
    if (batch.empty()) {
        return;
    }
    
    auto* conn = get_connection();
    if (!conn) {
        stats_.dropped_metrics.fetch_add(batch.size());
        return;
    }
    
    // Combine batch into single insert
    std::ostringstream combined_query;
    combined_query << "INSERT INTO packet_metrics VALUES ";
    
    for (size_t i = 0; i < batch.size(); ++i) {
        if (i > 0) {
            combined_query << ",";
        }
        combined_query << batch[i];
    }
    
    if (execute_insert(combined_query.str(), conn)) {
        stats_.packets_processed.fetch_add(batch.size());
    } else {
        stats_.dropped_metrics.fetch_add(batch.size());
    }
    
    release_connection(conn);
}

bool ClickHouseClient::create_tables() {
    auto* conn = get_connection();
    if (!conn) {
        return false;
    }
    
    bool success = create_packet_metrics_table() &&
                   create_routing_metrics_table() &&
                   create_system_metrics_table() &&
                   create_traffic_flows_table();
    
    release_connection(conn);
    return success;
}

bool ClickHouseClient::create_packet_metrics_table() {
    std::string create_table = R"(
        CREATE TABLE IF NOT EXISTS packet_metrics (
            timestamp DateTime64(3),
            source_ip String,
            dest_ip String,
            source_port UInt16,
            dest_port UInt16,
            protocol String,
            packet_size UInt32,
            dscp UInt8,
            ttl UInt8,
            interface_name String,
            router_id String,
            is_fragmented UInt8,
            is_encrypted UInt8,
            processing_time_ms Float64
        ) ENGINE = MergeTree()
        ORDER BY (timestamp, router_id, protocol)
        PARTITION BY toYYYYMM(timestamp)
    )";
    
    return execute_query(create_table);
}

bool ClickHouseClient::create_routing_metrics_table() {
    std::string create_table = R"(
        CREATE TABLE IF NOT EXISTS routing_metrics (
            timestamp DateTime64(3),
            router_id String,
            protocol String,
            routes_count UInt32,
            active_routes UInt32,
            routes_added UInt32,
            routes_removed UInt32,
            routes_modified UInt32,
            convergence_time_ms Float64,
            neighbor_ip String,
            neighbor_as String,
            route_prefix String,
            route_prefix_length UInt8,
            route_metric UInt32,
            route_origin String,
            route_community String
        ) ENGINE = MergeTree()
        ORDER BY (timestamp, router_id, protocol)
        PARTITION BY toYYYYMM(timestamp)
    )";
    
    return execute_query(create_table);
}

bool ClickHouseClient::create_system_metrics_table() {
    std::string create_table = R"(
        CREATE TABLE IF NOT EXISTS system_metrics (
            timestamp DateTime64(3),
            router_id String,
            cpu_usage_percent Float64,
            memory_usage_percent Float64,
            disk_usage_percent Float64,
            network_bytes_in UInt64,
            network_bytes_out UInt64,
            packets_in UInt64,
            packets_out UInt64,
            packets_dropped UInt64,
            packets_forwarded UInt64,
            temperature_celsius Float64,
            fan_speed_rpm UInt32,
            power_status String,
            uptime_seconds String
        ) ENGINE = MergeTree()
        ORDER BY (timestamp, router_id)
        PARTITION BY toYYYYMM(timestamp)
    )";
    
    return execute_query(create_table);
}

bool ClickHouseClient::create_traffic_flows_table() {
    std::string create_table = R"(
        CREATE TABLE IF NOT EXISTS traffic_flows (
            timestamp DateTime64(3),
            source_ip String,
            dest_ip String,
            source_port UInt16,
            dest_port UInt16,
            protocol String,
            packet_count UInt64,
            byte_count UInt64,
            duration_ms UInt64,
            application String,
            service String,
            country_code String,
            as_number String,
            is_encrypted UInt8,
            throughput_mbps Float64,
            latency_ms Float64,
            packet_loss_percent Float64
        ) ENGINE = MergeTree()
        ORDER BY (timestamp, source_ip, dest_ip)
        PARTITION BY toYYYYMM(timestamp)
    )";
    
    return execute_query(create_table);
}

bool ClickHouseClient::insert_packet_metrics(const PacketMetrics& metrics) {
    if (!connected_.load()) {
        return false;
    }
    
    std::ostringstream values;
    values << "("
           << "toDateTime64(" << std::chrono::duration_cast<std::chrono::milliseconds>(
                   metrics.timestamp.time_since_epoch()).count() << ", 3),"
           << "'" << escape_string(metrics.source_ip) << "',"
           << "'" << escape_string(metrics.dest_ip) << "',"
           << metrics.source_port << ","
           << metrics.dest_port << ","
           << "'" << escape_string(metrics.protocol) << "',"
           << metrics.packet_size << ","
           << static_cast<int>(metrics.dscp) << ","
           << static_cast<int>(metrics.ttl) << ","
           << "'" << escape_string(metrics.interface_name) << "',"
           << "'" << escape_string(metrics.router_id) << "',"
           << (metrics.is_fragmented ? 1 : 0) << ","
           << (metrics.is_encrypted ? 1 : 0) << ","
           << metrics.processing_time_ms
           << ")";
    
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        if (insert_queue_.size() >= config_.buffer_size) {
            stats_.dropped_metrics.fetch_add(1);
            return false;
        }
        insert_queue_.push(values.str());
    }
    
    queue_cv_.notify_one();
    return true;
}

bool ClickHouseClient::insert_routing_metrics(const RoutingMetrics& metrics) {
    // Similar implementation for routing metrics
    return true;
}

bool ClickHouseClient::insert_system_metrics(const SystemMetrics& metrics) {
    // Similar implementation for system metrics
    return true;
}

bool ClickHouseClient::insert_traffic_flow(const TrafficFlow& flow) {
    // Similar implementation for traffic flows
    return true;
}

std::string ClickHouseClient::escape_string(const std::string& str) {
    std::string escaped = str;
    std::replace(escaped.begin(), escaped.end(), '\'', '\\');
    return escaped;
}

std::string ClickHouseClient::format_timestamp(const std::chrono::system_clock::time_point& tp) {
    auto time_t = std::chrono::system_clock::to_time_t(tp);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        tp.time_since_epoch()) % 1000;
    
    std::ostringstream oss;
    oss << std::put_time(std::gmtime(&time_t), "%Y-%m-%d %H:%M:%S");
    oss << "." << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
}

ClickHouseClient::Statistics ClickHouseClient::get_statistics() const {
    return stats_;
}

void ClickHouseClient::reset_statistics() {
    stats_.packets_processed.store(0);
    stats_.bytes_processed.store(0);
    stats_.queries_executed.store(0);
    stats_.errors.store(0);
    stats_.buffer_size.store(0);
    stats_.dropped_metrics.store(0);
}

} // namespace RouterSim
