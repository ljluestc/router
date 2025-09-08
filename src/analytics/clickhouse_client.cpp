#include "clickhouse_client.h"
#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>
#include <cstring>

namespace RouterSim {

ClickHouseClient::ClickHouseClient(const std::string& host, uint16_t port, const std::string& database)
    : host_(host)
    , port_(port)
    , database_(database)
    , connected_(false)
    , total_queries_executed_(0)
    , total_queries_failed_(0)
    , total_bytes_sent_(0)
    , total_bytes_received_(0)
{
}

ClickHouseClient::~ClickHouseClient() {
    disconnect();
}

bool ClickHouseClient::connect() {
    if (connected_) {
        return true;
    }
    
    std::cout << "Connecting to ClickHouse at " << host_ << ":" << port_ << std::endl;
    
    try {
        // Create ClickHouse client
        client_ = std::make_unique<clickhouse::Client>(
            clickhouse::ClientOptions()
                .SetHost(host_)
                .SetPort(port_)
                .SetDefaultDatabase(database_)
                .SetUser("default")
                .SetPassword("")
                .SetPingBeforeQuery(true)
        );
        
        // Test connection
        if (testConnection()) {
            connected_ = true;
            std::cout << "Successfully connected to ClickHouse" << std::endl;
            return true;
        } else {
            std::cerr << "Failed to connect to ClickHouse" << std::endl;
            return false;
        }
    } catch (const std::exception& e) {
        std::cerr << "ClickHouse connection error: " << e.what() << std::endl;
        return false;
    }
}

void ClickHouseClient::disconnect() {
    if (connected_) {
        client_.reset();
        connected_ = false;
        std::cout << "Disconnected from ClickHouse" << std::endl;
    }
}

bool ClickHouseClient::isConnected() const {
    return connected_;
}

bool ClickHouseClient::createTables() {
    if (!connected_) {
        std::cerr << "Not connected to ClickHouse" << std::endl;
        return false;
    }
    
    std::cout << "Creating ClickHouse tables..." << std::endl;
    
    try {
        // Create router_metrics table
        std::string create_metrics_table = R"(
            CREATE TABLE IF NOT EXISTS router_metrics (
                timestamp DateTime64(3),
                router_id String,
                interface_name String,
                metric_name String,
                metric_value Float64,
                tags Map(String, String)
            ) ENGINE = MergeTree()
            ORDER BY (timestamp, router_id, interface_name, metric_name)
            TTL timestamp + INTERVAL 30 DAY
        )";
        
        if (!executeQuery(create_metrics_table)) {
            return false;
        }
        
        // Create packet_analytics table
        std::string create_packets_table = R"(
            CREATE TABLE IF NOT EXISTS packet_analytics (
                timestamp DateTime64(3),
                router_id String,
                interface_name String,
                source_ip String,
                dest_ip String,
                source_port UInt16,
                dest_port UInt16,
                protocol UInt8,
                packet_size UInt32,
                packet_count UInt64,
                bytes_transferred UInt64
            ) ENGINE = MergeTree()
            ORDER BY (timestamp, router_id, interface_name, source_ip, dest_ip)
            TTL timestamp + INTERVAL 7 DAY
        )";
        
        if (!executeQuery(create_packets_table)) {
            return false;
        }
        
        // Create routing_events table
        std::string create_routing_table = R"(
            CREATE TABLE IF NOT EXISTS routing_events (
                timestamp DateTime64(3),
                router_id String,
                event_type String,
                protocol String,
                destination String,
                gateway String,
                interface String,
                metric UInt32,
                action String,
                details Map(String, String)
            ) ENGINE = MergeTree()
            ORDER BY (timestamp, router_id, event_type, protocol)
            TTL timestamp + INTERVAL 14 DAY
        )";
        
        if (!executeQuery(create_routing_table)) {
            return false;
        }
        
        // Create traffic_shaping_events table
        std::string create_shaping_table = R"(
            CREATE TABLE IF NOT EXISTS traffic_shaping_events (
                timestamp DateTime64(3),
                router_id String,
                interface_name String,
                shaping_type String,
                queue_id UInt32,
                packets_processed UInt64,
                bytes_processed UInt64,
                packets_dropped UInt64,
                bytes_dropped UInt64,
                utilization_percentage Float64
            ) ENGINE = MergeTree()
            ORDER BY (timestamp, router_id, interface_name, shaping_type)
            TTL timestamp + INTERVAL 7 DAY
        )";
        
        if (!executeQuery(create_shaping_table)) {
            return false;
        }
        
        std::cout << "ClickHouse tables created successfully" << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error creating ClickHouse tables: " << e.what() << std::endl;
        return false;
    }
}

bool ClickHouseClient::insertMetric(const Metric& metric) {
    if (!connected_) {
        return false;
    }
    
    try {
        clickhouse::Block block;
        
        // Add columns
        block.AppendColumn("timestamp", std::make_shared<clickhouse::ColumnDateTime64>(3));
        block.AppendColumn("router_id", std::make_shared<clickhouse::ColumnString>());
        block.AppendColumn("interface_name", std::make_shared<clickhouse::ColumnString>());
        block.AppendColumn("metric_name", std::make_shared<clickhouse::ColumnString>());
        block.AppendColumn("metric_value", std::make_shared<clickhouse::ColumnFloat64>());
        block.AppendColumn("tags", std::make_shared<clickhouse::ColumnMap>(
            std::make_shared<clickhouse::ColumnString>(),
            std::make_shared<clickhouse::ColumnString>()
        ));
        
        // Add data
        auto timestamp_col = block[0]->As<clickhouse::ColumnDateTime64>();
        auto router_id_col = block[1]->As<clickhouse::ColumnString>();
        auto interface_col = block[2]->As<clickhouse::ColumnString>();
        auto metric_name_col = block[3]->As<clickhouse::ColumnString>();
        auto metric_value_col = block[4]->As<clickhouse::ColumnFloat64>();
        auto tags_col = block[5]->As<clickhouse::ColumnMap>();
        
        timestamp_col->Append(metric.timestamp);
        router_id_col->Append(metric.router_id);
        interface_col->Append(metric.interface_name);
        metric_name_col->Append(metric.metric_name);
        metric_value_col->Append(metric.metric_value);
        
        // Add tags
        clickhouse::ColumnString tag_keys, tag_values;
        for (const auto& tag : metric.tags) {
            tag_keys.Append(tag.first);
            tag_values.Append(tag.second);
        }
        tags_col->Append(tag_keys, tag_values);
        
        // Insert data
        client_->Insert("router_metrics", block);
        
        total_queries_executed_++;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error inserting metric: " << e.what() << std::endl;
        total_queries_failed_++;
        return false;
    }
}

bool ClickHouseClient::insertPacketAnalytics(const PacketAnalytics& analytics) {
    if (!connected_) {
        return false;
    }
    
    try {
        clickhouse::Block block;
        
        // Add columns
        block.AppendColumn("timestamp", std::make_shared<clickhouse::ColumnDateTime64>(3));
        block.AppendColumn("router_id", std::make_shared<clickhouse::ColumnString>());
        block.AppendColumn("interface_name", std::make_shared<clickhouse::ColumnString>());
        block.AppendColumn("source_ip", std::make_shared<clickhouse::ColumnString>());
        block.AppendColumn("dest_ip", std::make_shared<clickhouse::ColumnString>());
        block.AppendColumn("source_port", std::make_shared<clickhouse::ColumnUInt16>());
        block.AppendColumn("dest_port", std::make_shared<clickhouse::ColumnUInt16>());
        block.AppendColumn("protocol", std::make_shared<clickhouse::ColumnUInt8>());
        block.AppendColumn("packet_size", std::make_shared<clickhouse::ColumnUInt32>());
        block.AppendColumn("packet_count", std::make_shared<clickhouse::ColumnUInt64>());
        block.AppendColumn("bytes_transferred", std::make_shared<clickhouse::ColumnUInt64>());
        
        // Add data
        auto timestamp_col = block[0]->As<clickhouse::ColumnDateTime64>();
        auto router_id_col = block[1]->As<clickhouse::ColumnString>();
        auto interface_col = block[2]->As<clickhouse::ColumnString>();
        auto source_ip_col = block[3]->As<clickhouse::ColumnString>();
        auto dest_ip_col = block[4]->As<clickhouse::ColumnString>();
        auto source_port_col = block[5]->As<clickhouse::ColumnUInt16>();
        auto dest_port_col = block[6]->As<clickhouse::ColumnUInt16>();
        auto protocol_col = block[7]->As<clickhouse::ColumnUInt8>();
        auto packet_size_col = block[8]->As<clickhouse::ColumnUInt32>();
        auto packet_count_col = block[9]->As<clickhouse::ColumnUInt64>();
        auto bytes_col = block[10]->As<clickhouse::ColumnUInt64>();
        
        timestamp_col->Append(analytics.timestamp);
        router_id_col->Append(analytics.router_id);
        interface_col->Append(analytics.interface_name);
        source_ip_col->Append(analytics.source_ip);
        dest_ip_col->Append(analytics.dest_ip);
        source_port_col->Append(analytics.source_port);
        dest_port_col->Append(analytics.dest_port);
        protocol_col->Append(analytics.protocol);
        packet_size_col->Append(analytics.packet_size);
        packet_count_col->Append(analytics.packet_count);
        bytes_col->Append(analytics.bytes_transferred);
        
        // Insert data
        client_->Insert("packet_analytics", block);
        
        total_queries_executed_++;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error inserting packet analytics: " << e.what() << std::endl;
        total_queries_failed_++;
        return false;
    }
}

bool ClickHouseClient::insertRoutingEvent(const RoutingEvent& event) {
    if (!connected_) {
        return false;
    }
    
    try {
        clickhouse::Block block;
        
        // Add columns
        block.AppendColumn("timestamp", std::make_shared<clickhouse::ColumnDateTime64>(3));
        block.AppendColumn("router_id", std::make_shared<clickhouse::ColumnString>());
        block.AppendColumn("event_type", std::make_shared<clickhouse::ColumnString>());
        block.AppendColumn("protocol", std::make_shared<clickhouse::ColumnString>());
        block.AppendColumn("destination", std::make_shared<clickhouse::ColumnString>());
        block.AppendColumn("gateway", std::make_shared<clickhouse::ColumnString>());
        block.AppendColumn("interface", std::make_shared<clickhouse::ColumnString>());
        block.AppendColumn("metric", std::make_shared<clickhouse::ColumnUInt32>());
        block.AppendColumn("action", std::make_shared<clickhouse::ColumnString>());
        block.AppendColumn("details", std::make_shared<clickhouse::ColumnMap>(
            std::make_shared<clickhouse::ColumnString>(),
            std::make_shared<clickhouse::ColumnString>()
        ));
        
        // Add data
        auto timestamp_col = block[0]->As<clickhouse::ColumnDateTime64>();
        auto router_id_col = block[1]->As<clickhouse::ColumnString>();
        auto event_type_col = block[2]->As<clickhouse::ColumnString>();
        auto protocol_col = block[3]->As<clickhouse::ColumnString>();
        auto destination_col = block[4]->As<clickhouse::ColumnString>();
        auto gateway_col = block[5]->As<clickhouse::ColumnString>();
        auto interface_col = block[6]->As<clickhouse::ColumnString>();
        auto metric_col = block[7]->As<clickhouse::ColumnUInt32>();
        auto action_col = block[8]->As<clickhouse::ColumnString>();
        auto details_col = block[9]->As<clickhouse::ColumnMap>();
        
        timestamp_col->Append(event.timestamp);
        router_id_col->Append(event.router_id);
        event_type_col->Append(event.event_type);
        protocol_col->Append(event.protocol);
        destination_col->Append(event.destination);
        gateway_col->Append(event.gateway);
        interface_col->Append(event.interface);
        metric_col->Append(event.metric);
        action_col->Append(event.action);
        
        // Add details
        clickhouse::ColumnString detail_keys, detail_values;
        for (const auto& detail : event.details) {
            detail_keys.Append(detail.first);
            detail_values.Append(detail.second);
        }
        details_col->Append(detail_keys, detail_values);
        
        // Insert data
        client_->Insert("routing_events", block);
        
        total_queries_executed_++;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error inserting routing event: " << e.what() << std::endl;
        total_queries_failed_++;
        return false;
    }
}

bool ClickHouseClient::insertTrafficShapingEvent(const TrafficShapingEvent& event) {
    if (!connected_) {
        return false;
    }
    
    try {
        clickhouse::Block block;
        
        // Add columns
        block.AppendColumn("timestamp", std::make_shared<clickhouse::ColumnDateTime64>(3));
        block.AppendColumn("router_id", std::make_shared<clickhouse::ColumnString>());
        block.AppendColumn("interface_name", std::make_shared<clickhouse::ColumnString>());
        block.AppendColumn("shaping_type", std::make_shared<clickhouse::ColumnString>());
        block.AppendColumn("queue_id", std::make_shared<clickhouse::ColumnUInt32>());
        block.AppendColumn("packets_processed", std::make_shared<clickhouse::ColumnUInt64>());
        block.AppendColumn("bytes_processed", std::make_shared<clickhouse::ColumnUInt64>());
        block.AppendColumn("packets_dropped", std::make_shared<clickhouse::ColumnUInt64>());
        block.AppendColumn("bytes_dropped", std::make_shared<clickhouse::ColumnUInt64>());
        block.AppendColumn("utilization_percentage", std::make_shared<clickhouse::ColumnFloat64>());
        
        // Add data
        auto timestamp_col = block[0]->As<clickhouse::ColumnDateTime64>();
        auto router_id_col = block[1]->As<clickhouse::ColumnString>();
        auto interface_col = block[2]->As<clickhouse::ColumnString>();
        auto shaping_type_col = block[3]->As<clickhouse::ColumnString>();
        auto queue_id_col = block[4]->As<clickhouse::ColumnUInt32>();
        auto packets_processed_col = block[5]->As<clickhouse::ColumnUInt64>();
        auto bytes_processed_col = block[6]->As<clickhouse::ColumnUInt64>();
        auto packets_dropped_col = block[7]->As<clickhouse::ColumnUInt64>();
        auto bytes_dropped_col = block[8]->As<clickhouse::ColumnUInt64>();
        auto utilization_col = block[9]->As<clickhouse::ColumnFloat64>();
        
        timestamp_col->Append(event.timestamp);
        router_id_col->Append(event.router_id);
        interface_col->Append(event.interface_name);
        shaping_type_col->Append(event.shaping_type);
        queue_id_col->Append(event.queue_id);
        packets_processed_col->Append(event.packets_processed);
        bytes_processed_col->Append(event.bytes_processed);
        packets_dropped_col->Append(event.packets_dropped);
        bytes_dropped_col->Append(event.bytes_dropped);
        utilization_col->Append(event.utilization_percentage);
        
        // Insert data
        client_->Insert("traffic_shaping_events", block);
        
        total_queries_executed_++;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error inserting traffic shaping event: " << e.what() << std::endl;
        total_queries_failed_++;
        return false;
    }
}

std::vector<Metric> ClickHouseClient::queryMetrics(const std::string& query) {
    std::vector<Metric> metrics;
    
    if (!connected_) {
        return metrics;
    }
    
    try {
        clickhouse::Block block;
        client_->Select(query, &block);
        
        // Parse results (simplified implementation)
        // In a real implementation, you would parse the block and convert to Metric objects
        
        total_queries_executed_++;
        return metrics;
        
    } catch (const std::exception& e) {
        std::cerr << "Error querying metrics: " << e.what() << std::endl;
        total_queries_failed_++;
        return metrics;
    }
}

bool ClickHouseClient::executeQuery(const std::string& query) {
    if (!connected_) {
        return false;
    }
    
    try {
        client_->Execute(query);
        total_queries_executed_++;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error executing query: " << e.what() << std::endl;
        total_queries_failed_++;
        return false;
    }
}

bool ClickHouseClient::testConnection() {
    try {
        std::string query = "SELECT 1";
        client_->Execute(query);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "ClickHouse connection test failed: " << e.what() << std::endl;
        return false;
    }
}

ClickHouseClient::Statistics ClickHouseClient::getStatistics() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    Statistics stats;
    stats.connected = connected_;
    stats.host = host_;
    stats.port = port_;
    stats.database = database_;
    stats.total_queries_executed = total_queries_executed_;
    stats.total_queries_failed = total_queries_failed_;
    stats.total_bytes_sent = total_bytes_sent_;
    stats.total_bytes_received = total_bytes_received_;
    
    if (total_queries_executed_ + total_queries_failed_ > 0) {
        stats.success_rate = (double)total_queries_executed_ / 
                           (total_queries_executed_ + total_queries_failed_) * 100.0;
    } else {
        stats.success_rate = 0.0;
    }
    
    return stats;
}

void ClickHouseClient::reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    total_queries_executed_ = 0;
    total_queries_failed_ = 0;
    total_bytes_sent_ = 0;
    total_bytes_received_ = 0;
}

} // namespace RouterSim
