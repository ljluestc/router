#include "clickhouse_client.h"
#include <clickhouse/client.h>
#include <clickhouse/columns/column.h>
#include <clickhouse/columns/string.h>
#include <clickhouse/columns/numeric.h>
#include <clickhouse/columns/date.h>
#include <clickhouse/block.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <random>

namespace router_sim {
namespace analytics {

using namespace clickhouse;

class ClickHouseClient::ConnectionPool {
public:
    explicit ConnectionPool(const Config& config) : config_(config) {
        for (size_t i = 0; i < config_.max_connections; ++i) {
            connections_.emplace_back(create_connection());
        }
    }

    ~ConnectionPool() = default;

    std::shared_ptr<Client> get_connection() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (available_connections_.empty()) {
            return connections_[0]; // Return first connection if pool is empty
        }
        auto conn = available_connections_.front();
        available_connections_.pop();
        return conn;
    }

    void return_connection(std::shared_ptr<Client> conn) {
        std::lock_guard<std::mutex> lock(mutex_);
        available_connections_.push(conn);
    }

private:
    std::shared_ptr<Client> create_connection() {
        ClientOptions opts;
        opts.SetHost(config_.host);
        opts.SetPort(config_.port);
        opts.SetUser(config_.username);
        opts.SetPassword(config_.password);
        opts.SetDatabase(config_.database);
        opts.SetCompressionMethod(CompressionMethod::LZ4);
        
        return std::make_shared<Client>(opts);
    }

    const Config& config_;
    std::vector<std::shared_ptr<Client>> connections_;
    std::queue<std::shared_ptr<Client>> available_connections_;
    std::mutex mutex_;
};

class ClickHouseClient::BatchProcessor {
public:
    explicit BatchProcessor(ClickHouseClient* client, const Config& config)
        : client_(client), config_(config), running_(false) {}

    ~BatchProcessor() {
        stop();
    }

    void start() {
        if (running_) return;
        running_ = true;
        thread_ = std::thread(&BatchProcessor::process_loop, this);
    }

    void stop() {
        if (!running_) return;
        running_ = false;
        cv_.notify_all();
        if (thread_.joinable()) {
            thread_.join();
        }
    }

    void add_packet_metrics(const PacketMetrics& metrics) {
        std::lock_guard<std::mutex> lock(mutex_);
        packet_metrics_batch_.push_back(metrics);
        if (packet_metrics_batch_.size() >= config_.batch_size) {
            cv_.notify_one();
        }
    }

    void add_route_metrics(const RouteMetrics& metrics) {
        std::lock_guard<std::mutex> lock(mutex_);
        route_metrics_batch_.push_back(metrics);
        if (route_metrics_batch_.size() >= config_.batch_size) {
            cv_.notify_one();
        }
    }

    void add_system_metrics(const SystemMetrics& metrics) {
        std::lock_guard<std::mutex> lock(mutex_);
        system_metrics_batch_.push_back(metrics);
        if (system_metrics_batch_.size() >= config_.batch_size) {
            cv_.notify_one();
        }
    }

    void add_traffic_flow(const TrafficFlow& flow) {
        std::lock_guard<std::mutex> lock(mutex_);
        traffic_flow_batch_.push_back(flow);
        if (traffic_flow_batch_.size() >= config_.batch_size) {
            cv_.notify_one();
        }
    }

private:
    void process_loop() {
        auto last_flush = std::chrono::steady_clock::now();
        
        while (running_) {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait_for(lock, config_.flush_interval, [this] { 
                return !running_ || 
                       packet_metrics_batch_.size() >= config_.batch_size ||
                       route_metrics_batch_.size() >= config_.batch_size ||
                       system_metrics_batch_.size() >= config_.batch_size ||
                       traffic_flow_batch_.size() >= config_.batch_size;
            });

            if (!running_) break;

            auto now = std::chrono::steady_clock::now();
            bool should_flush = (now - last_flush) >= config_.flush_interval;

            if (should_flush || 
                packet_metrics_batch_.size() >= config_.batch_size ||
                route_metrics_batch_.size() >= config_.batch_size ||
                system_metrics_batch_.size() >= config_.batch_size ||
                traffic_flow_batch_.size() >= config_.batch_size) {
                
                flush_batches();
                last_flush = now;
            }
        }
    }

    void flush_batches() {
        if (!packet_metrics_batch_.empty()) {
            flush_packet_metrics();
        }
        if (!route_metrics_batch_.empty()) {
            flush_route_metrics();
        }
        if (!system_metrics_batch_.empty()) {
            flush_system_metrics();
        }
        if (!traffic_flow_batch_.empty()) {
            flush_traffic_flows();
        }
    }

    void flush_packet_metrics() {
        if (packet_metrics_batch_.empty()) return;

        try {
            auto conn = client_->connection_pool_->get_connection();
            
            Block block;
            
            // Create columns
            auto timestamp_col = std::make_shared<ColumnDateTime64>(3); // millisecond precision
            auto interface_col = std::make_shared<ColumnString>();
            auto source_ip_col = std::make_shared<ColumnString>();
            auto dest_ip_col = std::make_shared<ColumnString>();
            auto source_port_col = std::make_shared<ColumnUInt16>();
            auto dest_port_col = std::make_shared<ColumnUInt16>();
            auto protocol_col = std::make_shared<ColumnUInt8>();
            auto packet_size_col = std::make_shared<ColumnUInt16>();
            auto dscp_col = std::make_shared<ColumnUInt8>();
            auto ttl_col = std::make_shared<ColumnUInt8>();
            auto is_fragmented_col = std::make_shared<ColumnUInt8>();
            auto is_routed_col = std::make_shared<ColumnUInt8>();
            auto is_dropped_col = std::make_shared<ColumnUInt8>();
            auto processing_time_col = std::make_shared<ColumnUInt32>();
            auto next_hop_col = std::make_shared<ColumnString>();
            auto route_source_col = std::make_shared<ColumnString>();

            // Fill columns
            for (const auto& metrics : packet_metrics_batch_) {
                auto timestamp_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                    metrics.timestamp.time_since_epoch()).count();
                timestamp_col->Append(timestamp_ms);
                interface_col->Append(metrics.interface_name);
                source_ip_col->Append(metrics.source_ip);
                dest_ip_col->Append(metrics.destination_ip);
                source_port_col->Append(metrics.source_port);
                dest_port_col->Append(metrics.destination_port);
                protocol_col->Append(metrics.protocol);
                packet_size_col->Append(metrics.packet_size);
                dscp_col->Append(metrics.dscp);
                ttl_col->Append(metrics.ttl);
                is_fragmented_col->Append(metrics.is_fragmented ? 1 : 0);
                is_routed_col->Append(metrics.is_routed ? 1 : 0);
                is_dropped_col->Append(metrics.is_dropped ? 1 : 0);
                processing_time_col->Append(static_cast<uint32_t>(metrics.processing_time.count()));
                next_hop_col->Append(metrics.next_hop);
                route_source_col->Append(metrics.route_source);
            }

            // Add columns to block
            block.AppendColumn("timestamp", timestamp_col);
            block.AppendColumn("interface_name", interface_col);
            block.AppendColumn("source_ip", source_ip_col);
            block.AppendColumn("destination_ip", dest_ip_col);
            block.AppendColumn("source_port", source_port_col);
            block.AppendColumn("destination_port", dest_port_col);
            block.AppendColumn("protocol", protocol_col);
            block.AppendColumn("packet_size", packet_size_col);
            block.AppendColumn("dscp", dscp_col);
            block.AppendColumn("ttl", ttl_col);
            block.AppendColumn("is_fragmented", is_fragmented_col);
            block.AppendColumn("is_routed", is_routed_col);
            block.AppendColumn("is_dropped", is_dropped_col);
            block.AppendColumn("processing_time_us", processing_time_col);
            block.AppendColumn("next_hop", next_hop_col);
            block.AppendColumn("route_source", route_source_col);

            conn->Insert("packet_metrics", block);
            client_->processed_count_ += packet_metrics_batch_.size();
            
            packet_metrics_batch_.clear();
            
        } catch (const std::exception& e) {
            std::cerr << "Error inserting packet metrics: " << e.what() << std::endl;
            client_->error_count_ += packet_metrics_batch_.size();
            packet_metrics_batch_.clear();
        }
    }

    void flush_route_metrics() {
        // Similar implementation for route metrics
        route_metrics_batch_.clear();
    }

    void flush_system_metrics() {
        // Similar implementation for system metrics
        system_metrics_batch_.clear();
    }

    void flush_traffic_flows() {
        // Similar implementation for traffic flows
        traffic_flow_batch_.clear();
    }

    ClickHouseClient* client_;
    const Config& config_;
    std::atomic<bool> running_;
    std::thread thread_;
    std::mutex mutex_;
    std::condition_variable cv_;

    std::vector<PacketMetrics> packet_metrics_batch_;
    std::vector<RouteMetrics> route_metrics_batch_;
    std::vector<SystemMetrics> system_metrics_batch_;
    std::vector<TrafficFlow> traffic_flow_batch_;
};

class ClickHouseClient::MetricsCollector {
public:
    explicit MetricsCollector(ClickHouseClient* client) : client_(client) {}

    size_t get_queue_size() const {
        return client_->batch_processor_->packet_metrics_batch_.size() +
               client_->batch_processor_->route_metrics_batch_.size() +
               client_->batch_processor_->system_metrics_batch_.size() +
               client_->batch_processor_->traffic_flow_batch_.size();
    }

    size_t get_processed_count() const {
        return client_->processed_count_.load();
    }

    size_t get_error_count() const {
        return client_->error_count_.load();
    }

    double get_throughput() const {
        // Calculate records per second based on processed count and uptime
        auto now = std::chrono::steady_clock::now();
        auto uptime = std::chrono::duration_cast<std::chrono::seconds>(
            now - start_time_).count();
        return uptime > 0 ? static_cast<double>(get_processed_count()) / uptime : 0.0;
    }

private:
    ClickHouseClient* client_;
    std::chrono::steady_clock::time_point start_time_ = std::chrono::steady_clock::now();
};

// ClickHouseClient implementation
ClickHouseClient::ClickHouseClient(const Config& config)
    : config_(config) {
    connection_pool_ = std::make_unique<ConnectionPool>(config_);
    batch_processor_ = std::make_unique<BatchProcessor>(this, config_);
    metrics_collector_ = std::make_unique<MetricsCollector>(this);
}

ClickHouseClient::~ClickHouseClient() {
    stop_background_processing();
    disconnect();
}

bool ClickHouseClient::connect() {
    try {
        auto conn = connection_pool_->get_connection();
        // Test connection
        conn->Execute("SELECT 1");
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to connect to ClickHouse: " << e.what() << std::endl;
        return false;
    }
}

void ClickHouseClient::disconnect() {
    // Connection pool will handle cleanup
}

bool ClickHouseClient::is_connected() const {
    try {
        auto conn = connection_pool_->get_connection();
        conn->Execute("SELECT 1");
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

void ClickHouseClient::insert_packet_metrics(const PacketMetrics& metrics) {
    batch_processor_->add_packet_metrics(metrics);
}

void ClickHouseClient::insert_route_metrics(const RouteMetrics& metrics) {
    batch_processor_->add_route_metrics(metrics);
}

void ClickHouseClient::insert_system_metrics(const SystemMetrics& metrics) {
    batch_processor_->add_system_metrics(metrics);
}

void ClickHouseClient::insert_traffic_flow(const TrafficFlow& flow) {
    batch_processor_->add_traffic_flow(flow);
}

void ClickHouseClient::insert_packet_metrics_batch(const std::vector<PacketMetrics>& metrics) {
    for (const auto& m : metrics) {
        batch_processor_->add_packet_metrics(m);
    }
}

void ClickHouseClient::insert_route_metrics_batch(const std::vector<RouteMetrics>& metrics) {
    for (const auto& m : metrics) {
        batch_processor_->add_route_metrics(m);
    }
}

void ClickHouseClient::insert_system_metrics_batch(const std::vector<SystemMetrics>& metrics) {
    for (const auto& m : metrics) {
        batch_processor_->add_system_metrics(m);
    }
}

void ClickHouseClient::insert_traffic_flow_batch(const std::vector<TrafficFlow>& flows) {
    for (const auto& f : flows) {
        batch_processor_->add_traffic_flow(f);
    }
}

size_t ClickHouseClient::get_queue_size() const {
    return metrics_collector_->get_queue_size();
}

size_t ClickHouseClient::get_processed_count() const {
    return metrics_collector_->get_processed_count();
}

size_t ClickHouseClient::get_error_count() const {
    return metrics_collector_->get_error_count();
}

double ClickHouseClient::get_throughput() const {
    return metrics_collector_->get_throughput();
}

void ClickHouseClient::flush() {
    // Force flush by notifying the batch processor
    std::lock_guard<std::mutex> lock(batch_processor_->mutex_);
    batch_processor_->cv_.notify_one();
}

void ClickHouseClient::start_background_processing() {
    if (running_) return;
    running_ = true;
    batch_processor_->start();
}

void ClickHouseClient::stop_background_processing() {
    if (!running_) return;
    running_ = false;
    batch_processor_->stop();
}

} // namespace analytics
} // namespace router_sim
