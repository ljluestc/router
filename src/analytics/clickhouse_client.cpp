#include "clickhouse_client.h"
#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>

namespace router_sim {

ClickHouseClient::ClickHouseClient(const std::string& host, uint16_t port, 
                                 const std::string& database, const std::string& username, 
                                 const std::string& password)
    : host_(host), port_(port), database_(database), username_(username), password_(password)
    , connected_(false), client_(nullptr) {
}

ClickHouseClient::~ClickHouseClient() {
    disconnect();
}

bool ClickHouseClient::connect() {
    try {
        client_ = std::make_unique<clickhouse::Client>(
            clickhouse::ClientOptions()
                .SetHost(host_)
                .SetPort(port_)
                .SetDefaultDatabase(database_)
                .SetUser(username_)
                .SetPassword(password_)
        );
        
        // Test connection
        client_->Execute("SELECT 1");
        connected_ = true;
        std::cout << "Connected to ClickHouse at " << host_ << ":" << port_ << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to connect to ClickHouse: " << e.what() << std::endl;
        return false;
    }
}

bool ClickHouseClient::disconnect() {
    if (connected_) {
        client_.reset();
        connected_ = false;
        std::cout << "Disconnected from ClickHouse" << std::endl;
    }
    return true;
}

bool ClickHouseClient::is_connected() const {
    return connected_;
}

bool ClickHouseClient::create_tables() {
    if (!connected_) {
        return false;
    }

    try {
        // Create router_metrics table
        std::string create_metrics_table = R"(
            CREATE TABLE IF NOT EXISTS router_metrics (
                timestamp DateTime64(3),
                router_id String,
                interface String,
                metric_name String,
                metric_value Float64,
                tags Map(String, String)
            ) ENGINE = MergeTree()
            ORDER BY (timestamp, router_id, interface, metric_name)
        )";

        client_->Execute(create_metrics_table);

        // Create packet_flows table
        std::string create_flows_table = R"(
            CREATE TABLE IF NOT EXISTS packet_flows (
                timestamp DateTime64(3),
                source_ip String,
                dest_ip String,
                source_port UInt16,
                dest_port UInt16,
                protocol UInt8,
                packet_size UInt32,
                interface String,
                router_id String,
                flow_id String
            ) ENGINE = MergeTree()
            ORDER BY (timestamp, source_ip, dest_ip, protocol)
        )";

        client_->Execute(create_flows_table);

        // Create routing_events table
        std::string create_routing_table = R"(
            CREATE TABLE IF NOT EXISTS routing_events (
                timestamp DateTime64(3),
                router_id String,
                event_type String,
                destination String,
                next_hop String,
                protocol String,
                metric UInt32,
                as_path Array(String),
                communities Array(String)
            ) ENGINE = MergeTree()
            ORDER BY (timestamp, router_id, event_type)
        )";

        client_->Execute(create_routing_table);

        // Create traffic_shaping_stats table
        std::string create_shaping_table = R"(
            CREATE TABLE IF NOT EXISTS traffic_shaping_stats (
                timestamp DateTime64(3),
                router_id String,
                interface String,
                algorithm String,
                packets_processed UInt64,
                bytes_processed UInt64,
                packets_dropped UInt64,
                bytes_dropped UInt64,
                utilization_percent Float64
            ) ENGINE = MergeTree()
            ORDER BY (timestamp, router_id, interface)
        )";

        client_->Execute(create_shaping_table);

        // Create network_impairments table
        std::string create_impairments_table = R"(
            CREATE TABLE IF NOT EXISTS network_impairments (
                timestamp DateTime64(3),
                router_id String,
                interface String,
                impairment_type String,
                parameters Map(String, String),
                active Bool
            ) ENGINE = MergeTree()
            ORDER BY (timestamp, router_id, interface)
        )";

        client_->Execute(create_impairments_table);

        std::cout << "Created ClickHouse tables successfully" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to create tables: " << e.what() << std::endl;
        return false;
    }
}

bool ClickHouseClient::insert_metric(const MetricData& metric) {
    if (!connected_) {
        return false;
    }

    try {
        clickhouse::Block block;
        
        block.AppendColumn("timestamp", std::make_shared<clickhouse::ColumnDateTime64>(
            std::vector<uint64_t>{metric.timestamp}));
        block.AppendColumn("router_id", std::make_shared<clickhouse::ColumnString>(
            std::vector<std::string>{metric.router_id}));
        block.AppendColumn("interface", std::make_shared<clickhouse::ColumnString>(
            std::vector<std::string>{metric.interface}));
        block.AppendColumn("metric_name", std::make_shared<clickhouse::ColumnString>(
            std::vector<std::string>{metric.metric_name}));
        block.AppendColumn("metric_value", std::make_shared<clickhouse::ColumnFloat64>(
            std::vector<double>{metric.metric_value}));
        
        // Convert tags map to ClickHouse Map
        std::vector<std::string> keys, values;
        for (const auto& pair : metric.tags) {
            keys.push_back(pair.first);
            values.push_back(pair.second);
        }
        block.AppendColumn("tags", std::make_shared<clickhouse::ColumnMap>(
            std::make_shared<clickhouse::ColumnString>(keys),
            std::make_shared<clickhouse::ColumnString>(values)));

        client_->Insert("router_metrics", block);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to insert metric: " << e.what() << std::endl;
        return false;
    }
}

bool ClickHouseClient::insert_packet_flow(const PacketFlowData& flow) {
    if (!connected_) {
        return false;
    }

    try {
        clickhouse::Block block;
        
        block.AppendColumn("timestamp", std::make_shared<clickhouse::ColumnDateTime64>(
            std::vector<uint64_t>{flow.timestamp}));
        block.AppendColumn("source_ip", std::make_shared<clickhouse::ColumnString>(
            std::vector<std::string>{flow.source_ip}));
        block.AppendColumn("dest_ip", std::make_shared<clickhouse::ColumnString>(
            std::vector<std::string>{flow.dest_ip}));
        block.AppendColumn("source_port", std::make_shared<clickhouse::ColumnUInt16>(
            std::vector<uint16_t>{flow.source_port}));
        block.AppendColumn("dest_port", std::make_shared<clickhouse::ColumnUInt16>(
            std::vector<uint16_t>{flow.dest_port}));
        block.AppendColumn("protocol", std::make_shared<clickhouse::ColumnUInt8>(
            std::vector<uint8_t>{flow.protocol}));
        block.AppendColumn("packet_size", std::make_shared<clickhouse::ColumnUInt32>(
            std::vector<uint32_t>{flow.packet_size}));
        block.AppendColumn("interface", std::make_shared<clickhouse::ColumnString>(
            std::vector<std::string>{flow.interface}));
        block.AppendColumn("router_id", std::make_shared<clickhouse::ColumnString>(
            std::vector<std::string>{flow.router_id}));
        block.AppendColumn("flow_id", std::make_shared<clickhouse::ColumnString>(
            std::vector<std::string>{flow.flow_id}));

        client_->Insert("packet_flows", block);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to insert packet flow: " << e.what() << std::endl;
        return false;
    }
}

bool ClickHouseClient::insert_routing_event(const RoutingEventData& event) {
    if (!connected_) {
        return false;
    }

    try {
        clickhouse::Block block;
        
        block.AppendColumn("timestamp", std::make_shared<clickhouse::ColumnDateTime64>(
            std::vector<uint64_t>{event.timestamp}));
        block.AppendColumn("router_id", std::make_shared<clickhouse::ColumnString>(
            std::vector<std::string>{event.router_id}));
        block.AppendColumn("event_type", std::make_shared<clickhouse::ColumnString>(
            std::vector<std::string>{event.event_type}));
        block.AppendColumn("destination", std::make_shared<clickhouse::ColumnString>(
            std::vector<std::string>{event.destination}));
        block.AppendColumn("next_hop", std::make_shared<clickhouse::ColumnString>(
            std::vector<std::string>{event.next_hop}));
        block.AppendColumn("protocol", std::make_shared<clickhouse::ColumnString>(
            std::vector<std::string>{event.protocol}));
        block.AppendColumn("metric", std::make_shared<clickhouse::ColumnUInt32>(
            std::vector<uint32_t>{event.metric}));
        block.AppendColumn("as_path", std::make_shared<clickhouse::ColumnArray>(
            std::make_shared<clickhouse::ColumnString>(event.as_path)));
        block.AppendColumn("communities", std::make_shared<clickhouse::ColumnArray>(
            std::make_shared<clickhouse::ColumnString>(event.communities)));

        client_->Insert("routing_events", block);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to insert routing event: " << e.what() << std::endl;
        return false;
    }
}

bool ClickHouseClient::insert_traffic_shaping_stats(const TrafficShapingStats& stats) {
    if (!connected_) {
        return false;
    }

    try {
        clickhouse::Block block;
        
        block.AppendColumn("timestamp", std::make_shared<clickhouse::ColumnDateTime64>(
            std::vector<uint64_t>{stats.timestamp}));
        block.AppendColumn("router_id", std::make_shared<clickhouse::ColumnString>(
            std::vector<std::string>{stats.router_id}));
        block.AppendColumn("interface", std::make_shared<clickhouse::ColumnString>(
            std::vector<std::string>{stats.interface}));
        block.AppendColumn("algorithm", std::make_shared<clickhouse::ColumnString>(
            std::vector<std::string>{stats.algorithm}));
        block.AppendColumn("packets_processed", std::make_shared<clickhouse::ColumnUInt64>(
            std::vector<uint64_t>{stats.packets_processed}));
        block.AppendColumn("bytes_processed", std::make_shared<clickhouse::ColumnUInt64>(
            std::vector<uint64_t>{stats.bytes_processed}));
        block.AppendColumn("packets_dropped", std::make_shared<clickhouse::ColumnUInt64>(
            std::vector<uint64_t>{stats.packets_dropped}));
        block.AppendColumn("bytes_dropped", std::make_shared<clickhouse::ColumnUInt64>(
            std::vector<uint64_t>{stats.bytes_dropped}));
        block.AppendColumn("utilization_percent", std::make_shared<clickhouse::ColumnFloat64>(
            std::vector<double>{stats.utilization_percent}));

        client_->Insert("traffic_shaping_stats", block);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to insert traffic shaping stats: " << e.what() << std::endl;
        return false;
    }
}

bool ClickHouseClient::insert_network_impairment(const NetworkImpairmentData& impairment) {
    if (!connected_) {
        return false;
    }

    try {
        clickhouse::Block block;
        
        block.AppendColumn("timestamp", std::make_shared<clickhouse::ColumnDateTime64>(
            std::vector<uint64_t>{impairment.timestamp}));
        block.AppendColumn("router_id", std::make_shared<clickhouse::ColumnString>(
            std::vector<std::string>{impairment.router_id}));
        block.AppendColumn("interface", std::make_shared<clickhouse::ColumnString>(
            std::vector<std::string>{impairment.interface}));
        block.AppendColumn("impairment_type", std::make_shared<clickhouse::ColumnString>(
            std::vector<std::string>{impairment.impairment_type}));
        
        // Convert parameters map
        std::vector<std::string> keys, values;
        for (const auto& pair : impairment.parameters) {
            keys.push_back(pair.first);
            values.push_back(pair.second);
        }
        block.AppendColumn("parameters", std::make_shared<clickhouse::ColumnMap>(
            std::make_shared<clickhouse::ColumnString>(keys),
            std::make_shared<clickhouse::ColumnString>(values)));
        
        block.AppendColumn("active", std::make_shared<clickhouse::ColumnUInt8>(
            std::vector<uint8_t>{impairment.active ? 1 : 0}));

        client_->Insert("network_impairments", block);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to insert network impairment: " << e.what() << std::endl;
        return false;
    }
}

std::vector<MetricData> ClickHouseClient::query_metrics(const std::string& query) {
    std::vector<MetricData> results;
    
    if (!connected_) {
        return results;
    }

    try {
        client_->Select(query, [&results](const clickhouse::Block& block) {
            for (size_t i = 0; i < block.GetRowCount(); ++i) {
                MetricData metric;
                
                // Extract data from block (simplified - in real implementation, 
                // you'd need to handle different column types properly)
                results.push_back(metric);
            }
        });
    } catch (const std::exception& e) {
        std::cerr << "Failed to query metrics: " << e.what() << std::endl;
    }
    
    return results;
}

bool ClickHouseClient::execute_query(const std::string& query) {
    if (!connected_) {
        return false;
    }

    try {
        client_->Execute(query);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to execute query: " << e.what() << std::endl;
        return false;
    }
}

std::string ClickHouseClient::get_connection_info() const {
    std::ostringstream oss;
    oss << "ClickHouse://" << username_ << "@" << host_ << ":" << port_ << "/" << database_;
    return oss.str();
}

} // namespace router_sim
