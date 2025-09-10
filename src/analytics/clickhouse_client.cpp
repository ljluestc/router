#include "analytics/clickhouse_client.h"
#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>
#include <future>

namespace RouterSim {

ClickHouseClient::ClickHouseClient(const std::string& host, int port, const std::string& database, const std::string& user, const std::string& password)
    : host_(host), port_(port), database_(database), user_(user), password_(password), connected_(false) {
    
    // Initialize connection string
    connection_string_ = "tcp://" + user_ + ":" + password_ + "@" + host_ + ":" + std::to_string(port_) + "/" + database_;
}

ClickHouseClient::~ClickHouseClient() {
    disconnect();
}

bool ClickHouseClient::connect() {
    if (connected_) {
        return true;
    }

    try {
        // Create connection
        client_ = std::make_unique<clickhouse::Client>(clickhouse::ClientOptions()
            .SetHost(host_)
            .SetPort(port_)
            .SetUser(user_)
            .SetPassword(password_)
            .SetDefaultDatabase(database_));

        // Test connection
        client_->Execute("SELECT 1");
        
        // Create tables if they don't exist
        create_tables();
        
        connected_ = true;
        std::cout << "Connected to ClickHouse at " << host_ << ":" << port_ << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to connect to ClickHouse: " << e.what() << std::endl;
        return false;
    }
}

void ClickHouseClient::disconnect() {
    if (connected_) {
        // Flush any remaining data
        flush_all_buffers();
        connected_ = false;
        std::cout << "Disconnected from ClickHouse" << std::endl;
    }
}

bool ClickHouseClient::is_connected() const {
    return connected_;
}

void ClickHouseClient::insert_metric(const NetworkMetric& metric) {
    if (!connected_) {
        return;
    }

    std::lock_guard<std::mutex> lock(metrics_mutex_);
    metrics_buffer_.push_back(metric);
    
    // Flush if buffer is full
    if (metrics_buffer_.size() >= 1000) {
        flush_metrics();
    }
}

void ClickHouseClient::insert_packet_flow(const PacketFlow& flow) {
    if (!connected_) {
        return;
    }

    std::lock_guard<std::mutex> lock(flows_mutex_);
    packet_flows_buffer_.push_back(flow);
    
    // Flush if buffer is full
    if (packet_flows_buffer_.size() >= 1000) {
        flush_packet_flows();
    }
}

void ClickHouseClient::insert_bgp_update(const BGPUpdate& update) {
    if (!connected_) {
        return;
    }

    std::lock_guard<std::mutex> lock(bgp_mutex_);
    bgp_updates_buffer_.push_back(update);
    
    // Flush if buffer is full
    if (bgp_updates_buffer_.size() >= 100) {
        flush_bgp_updates();
    }
}

void ClickHouseClient::insert_ospf_update(const OSPFUpdate& update) {
    if (!connected_) {
        return;
    }

    std::lock_guard<std::mutex> lock(ospf_mutex_);
    ospf_updates_buffer_.push_back(update);
    
    // Flush if buffer is full
    if (ospf_updates_buffer_.size() >= 100) {
        flush_ospf_updates();
    }
}

void ClickHouseClient::insert_isis_update(const ISISUpdate& update) {
    if (!connected_) {
        return;
    }

    std::lock_guard<std::mutex> lock(isis_mutex_);
    isis_updates_buffer_.push_back(update);
    
    // Flush if buffer is full
    if (isis_updates_buffer_.size() >= 100) {
        flush_isis_updates();
    }
}

void ClickHouseClient::insert_traffic_shaping_metric(const TrafficShapingMetric& metric) {
    if (!connected_) {
        return;
    }

    std::lock_guard<std::mutex> lock(traffic_mutex_);
    traffic_shaping_buffer_.push_back(metric);
    
    // Flush if buffer is full
    if (traffic_shaping_buffer_.size() >= 1000) {
        flush_traffic_shaping_metrics();
    }
}

void ClickHouseClient::insert_netem_impairment(const NetemImpairment& impairment) {
    if (!connected_) {
        return;
    }

    std::lock_guard<std::mutex> lock(netem_mutex_);
    netem_impairments_buffer_.push_back(impairment);
    
    // Flush if buffer is full
    if (netem_impairments_buffer_.size() >= 100) {
        flush_netem_impairments();
    }
}

void ClickHouseClient::flush_all_buffers() {
    flush_metrics();
    flush_packet_flows();
    flush_bgp_updates();
    flush_ospf_updates();
    flush_isis_updates();
    flush_traffic_shaping_metrics();
    flush_netem_impairments();
}

void ClickHouseClient::create_tables() {
    try {
        // Create network metrics table
        client_->Execute(R"(
            CREATE TABLE IF NOT EXISTS network_metrics (
                timestamp UInt64,
                router_id String,
                interface String,
                metric_type String,
                value Float64,
                tags Map(String, String),
                date Date MATERIALIZED toDate(timestamp / 1000)
            ) ENGINE = MergeTree()
            PARTITION BY date
            ORDER BY (router_id, interface, metric_type, timestamp)
        )");

        // Create packet flows table
        client_->Execute(R"(
            CREATE TABLE IF NOT EXISTS packet_flows (
                timestamp UInt64,
                src_ip String,
                dst_ip String,
                src_port UInt16,
                dst_port UInt16,
                protocol UInt8,
                bytes UInt64,
                packets UInt64,
                duration_ms UInt64,
                router_id String,
                date Date MATERIALIZED toDate(timestamp / 1000)
            ) ENGINE = MergeTree()
            PARTITION BY date
            ORDER BY (router_id, src_ip, dst_ip, timestamp)
        )");

        // Create BGP updates table
        client_->Execute(R"(
            CREATE TABLE IF NOT EXISTS bgp_updates (
                timestamp UInt64,
                router_id String,
                neighbor_ip String,
                prefix String,
                prefix_length UInt8,
                as_path Array(UInt32),
                next_hop String,
                origin String,
                local_pref UInt32,
                med UInt32,
                communities Array(String),
                action String,
                date Date MATERIALIZED toDate(timestamp / 1000)
            ) ENGINE = MergeTree()
            PARTITION BY date
            ORDER BY (router_id, neighbor_ip, prefix, timestamp)
        )");

        // Create OSPF updates table
        client_->Execute(R"(
            CREATE TABLE IF NOT EXISTS ospf_updates (
                timestamp UInt64,
                router_id String,
                area_id String,
                lsa_type UInt8,
                lsa_id String,
                advertising_router String,
                sequence_number UInt32,
                age UInt16,
                checksum UInt16,
                length UInt16,
                action String,
                date Date MATERIALIZED toDate(timestamp / 1000)
            ) ENGINE = MergeTree()
            PARTITION BY date
            ORDER BY (router_id, area_id, lsa_type, timestamp)
        )");

        // Create IS-IS updates table
        client_->Execute(R"(
            CREATE TABLE IF NOT EXISTS isis_updates (
                timestamp UInt64,
                system_id String,
                area_id String,
                level UInt8,
                lsp_id String,
                sequence_number UInt32,
                remaining_lifetime UInt16,
                checksum UInt16,
                pdu_length UInt16,
                action String,
                date Date MATERIALIZED toDate(timestamp / 1000)
            ) ENGINE = MergeTree()
            PARTITION BY date
            ORDER BY (system_id, area_id, level, timestamp)
        )");

        // Create traffic shaping metrics table
        client_->Execute(R"(
            CREATE TABLE IF NOT EXISTS traffic_shaping_metrics (
                timestamp UInt64,
                router_id String,
                interface String,
                algorithm String,
                class_id UInt8,
                packets_processed UInt64,
                packets_dropped UInt64,
                bytes_processed UInt64,
                bytes_dropped UInt64,
                queue_length UInt32,
                throughput_bps Float64,
                date Date MATERIALIZED toDate(timestamp / 1000)
            ) ENGINE = MergeTree()
            PARTITION BY date
            ORDER BY (router_id, interface, algorithm, timestamp)
        )");

        // Create netem impairments table
        client_->Execute(R"(
            CREATE TABLE IF NOT EXISTS netem_impairments (
                timestamp UInt64,
                router_id String,
                interface String,
                impairment_type String,
                parameters Map(String, String),
                active UInt8,
                date Date MATERIALIZED toDate(timestamp / 1000)
            ) ENGINE = MergeTree()
            PARTITION BY date
            ORDER BY (router_id, interface, impairment_type, timestamp)
        )");

        std::cout << "ClickHouse tables created successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Failed to create ClickHouse tables: " << e.what() << std::endl;
    }
}

void ClickHouseClient::flush_metrics() {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    if (metrics_buffer_.empty()) {
        return;
    }

    try {
        clickhouse::Block block;
        
        for (const auto& metric : metrics_buffer_) {
            block.AppendColumn("timestamp", std::vector<uint64_t>{metric.timestamp});
            block.AppendColumn("router_id", std::vector<std::string>{metric.router_id});
            block.AppendColumn("interface", std::vector<std::string>{metric.interface});
            block.AppendColumn("metric_type", std::vector<std::string>{metric.metric_type});
            block.AppendColumn("value", std::vector<double>{metric.value});
            
            // Convert tags map to ClickHouse format
            std::vector<std::string> tag_keys, tag_values;
            for (const auto& [key, value] : metric.tags) {
                tag_keys.push_back(key);
                tag_values.push_back(value);
            }
            block.AppendColumn("tags", std::make_pair(tag_keys, tag_values));
        }

        client_->Insert("network_metrics", block);
        metrics_buffer_.clear();
    } catch (const std::exception& e) {
        std::cerr << "Failed to flush metrics: " << e.what() << std::endl;
    }
}

void ClickHouseClient::flush_packet_flows() {
    std::lock_guard<std::mutex> lock(flows_mutex_);
    if (packet_flows_buffer_.empty()) {
        return;
    }

    try {
        clickhouse::Block block;
        
        for (const auto& flow : packet_flows_buffer_) {
            block.AppendColumn("timestamp", std::vector<uint64_t>{flow.timestamp});
            block.AppendColumn("src_ip", std::vector<std::string>{flow.src_ip});
            block.AppendColumn("dst_ip", std::vector<std::string>{flow.dst_ip});
            block.AppendColumn("src_port", std::vector<uint16_t>{flow.src_port});
            block.AppendColumn("dst_port", std::vector<uint16_t>{flow.dst_port});
            block.AppendColumn("protocol", std::vector<uint8_t>{flow.protocol});
            block.AppendColumn("bytes", std::vector<uint64_t>{flow.bytes});
            block.AppendColumn("packets", std::vector<uint64_t>{flow.packets});
            block.AppendColumn("duration_ms", std::vector<uint64_t>{flow.duration_ms});
            block.AppendColumn("router_id", std::vector<std::string>{flow.router_id});
        }

        client_->Insert("packet_flows", block);
        packet_flows_buffer_.clear();
    } catch (const std::exception& e) {
        std::cerr << "Failed to flush packet flows: " << e.what() << std::endl;
    }
}

void ClickHouseClient::flush_bgp_updates() {
    std::lock_guard<std::mutex> lock(bgp_mutex_);
    if (bgp_updates_buffer_.empty()) {
        return;
    }

    try {
        clickhouse::Block block;
        
        for (const auto& update : bgp_updates_buffer_) {
            block.AppendColumn("timestamp", std::vector<uint64_t>{update.timestamp});
            block.AppendColumn("router_id", std::vector<std::string>{update.router_id});
            block.AppendColumn("neighbor_ip", std::vector<std::string>{update.neighbor_ip});
            block.AppendColumn("prefix", std::vector<std::string>{update.prefix});
            block.AppendColumn("prefix_length", std::vector<uint8_t>{update.prefix_length});
            block.AppendColumn("as_path", std::vector<std::vector<uint32_t>>{update.as_path});
            block.AppendColumn("next_hop", std::vector<std::string>{update.next_hop});
            block.AppendColumn("origin", std::vector<std::string>{update.origin});
            block.AppendColumn("local_pref", std::vector<uint32_t>{update.local_pref});
            block.AppendColumn("med", std::vector<uint32_t>{update.med});
            block.AppendColumn("communities", std::vector<std::vector<std::string>>{update.communities});
            block.AppendColumn("action", std::vector<std::string>{update.action});
        }

        client_->Insert("bgp_updates", block);
        bgp_updates_buffer_.clear();
    } catch (const std::exception& e) {
        std::cerr << "Failed to flush BGP updates: " << e.what() << std::endl;
    }
}

void ClickHouseClient::flush_ospf_updates() {
    std::lock_guard<std::mutex> lock(ospf_mutex_);
    if (ospf_updates_buffer_.empty()) {
        return;
    }

    try {
        clickhouse::Block block;
        
        for (const auto& update : ospf_updates_buffer_) {
            block.AppendColumn("timestamp", std::vector<uint64_t>{update.timestamp});
            block.AppendColumn("router_id", std::vector<std::string>{update.router_id});
            block.AppendColumn("area_id", std::vector<std::string>{update.area_id});
            block.AppendColumn("lsa_type", std::vector<uint8_t>{update.lsa_type});
            block.AppendColumn("lsa_id", std::vector<std::string>{update.lsa_id});
            block.AppendColumn("advertising_router", std::vector<std::string>{update.advertising_router});
            block.AppendColumn("sequence_number", std::vector<uint32_t>{update.sequence_number});
            block.AppendColumn("age", std::vector<uint16_t>{update.age});
            block.AppendColumn("checksum", std::vector<uint16_t>{update.checksum});
            block.AppendColumn("length", std::vector<uint16_t>{update.length});
            block.AppendColumn("action", std::vector<std::string>{update.action});
        }

        client_->Insert("ospf_updates", block);
        ospf_updates_buffer_.clear();
    } catch (const std::exception& e) {
        std::cerr << "Failed to flush OSPF updates: " << e.what() << std::endl;
    }
}

void ClickHouseClient::flush_isis_updates() {
    std::lock_guard<std::mutex> lock(isis_mutex_);
    if (isis_updates_buffer_.empty()) {
        return;
    }

    try {
        clickhouse::Block block;
        
        for (const auto& update : isis_updates_buffer_) {
            block.AppendColumn("timestamp", std::vector<uint64_t>{update.timestamp});
            block.AppendColumn("system_id", std::vector<std::string>{update.system_id});
            block.AppendColumn("area_id", std::vector<std::string>{update.area_id});
            block.AppendColumn("level", std::vector<uint8_t>{update.level});
            block.AppendColumn("lsp_id", std::vector<std::string>{update.lsp_id});
            block.AppendColumn("sequence_number", std::vector<uint32_t>{update.sequence_number});
            block.AppendColumn("remaining_lifetime", std::vector<uint16_t>{update.remaining_lifetime});
            block.AppendColumn("checksum", std::vector<uint16_t>{update.checksum});
            block.AppendColumn("pdu_length", std::vector<uint16_t>{update.pdu_length});
            block.AppendColumn("action", std::vector<std::string>{update.action});
        }

        client_->Insert("isis_updates", block);
        isis_updates_buffer_.clear();
    } catch (const std::exception& e) {
        std::cerr << "Failed to flush IS-IS updates: " << e.what() << std::endl;
    }
}

void ClickHouseClient::flush_traffic_shaping_metrics() {
    std::lock_guard<std::mutex> lock(traffic_mutex_);
    if (traffic_shaping_buffer_.empty()) {
        return;
    }

    try {
        clickhouse::Block block;
        
        for (const auto& metric : traffic_shaping_buffer_) {
            block.AppendColumn("timestamp", std::vector<uint64_t>{metric.timestamp});
            block.AppendColumn("router_id", std::vector<std::string>{metric.router_id});
            block.AppendColumn("interface", std::vector<std::string>{metric.interface});
            block.AppendColumn("algorithm", std::vector<std::string>{metric.algorithm});
            block.AppendColumn("class_id", std::vector<uint8_t>{metric.class_id});
            block.AppendColumn("packets_processed", std::vector<uint64_t>{metric.packets_processed});
            block.AppendColumn("packets_dropped", std::vector<uint64_t>{metric.packets_dropped});
            block.AppendColumn("bytes_processed", std::vector<uint64_t>{metric.bytes_processed});
            block.AppendColumn("bytes_dropped", std::vector<uint64_t>{metric.bytes_dropped});
            block.AppendColumn("queue_length", std::vector<uint32_t>{metric.queue_length});
            block.AppendColumn("throughput_bps", std::vector<double>{metric.throughput_bps});
        }

        client_->Insert("traffic_shaping_metrics", block);
        traffic_shaping_buffer_.clear();
    } catch (const std::exception& e) {
        std::cerr << "Failed to flush traffic shaping metrics: " << e.what() << std::endl;
    }
}

void ClickHouseClient::flush_netem_impairments() {
    std::lock_guard<std::mutex> lock(netem_mutex_);
    if (netem_impairments_buffer_.empty()) {
        return;
    }

    try {
        clickhouse::Block block;
        
        for (const auto& impairment : netem_impairments_buffer_) {
            block.AppendColumn("timestamp", std::vector<uint64_t>{impairment.timestamp});
            block.AppendColumn("router_id", std::vector<std::string>{impairment.router_id});
            block.AppendColumn("interface", std::vector<std::string>{impairment.interface});
            block.AppendColumn("impairment_type", std::vector<std::string>{impairment.impairment_type});
            
            // Convert parameters map to ClickHouse format
            std::vector<std::string> param_keys, param_values;
            for (const auto& [key, value] : impairment.parameters) {
                param_keys.push_back(key);
                param_values.push_back(value);
            }
            block.AppendColumn("parameters", std::make_pair(param_keys, param_values));
            block.AppendColumn("active", std::vector<uint8_t>{impairment.active ? 1 : 0});
        }

        client_->Insert("netem_impairments", block);
        netem_impairments_buffer_.clear();
    } catch (const std::exception& e) {
        std::cerr << "Failed to flush netem impairments: " << e.what() << std::endl;
    }
}

std::vector<std::map<std::string, std::string>> ClickHouseClient::query(const std::string& query) {
    std::vector<std::map<std::string, std::string>> results;
    
    if (!connected_) {
        return results;
    }

    try {
        auto result = client_->Select(query);
        
        while (result->HasNext()) {
            auto row = result->Next();
            std::map<std::string, std::string> row_data;
            
            // This is a simplified implementation
            // In practice, you would need to handle different column types
            for (size_t i = 0; i < row.GetColumnCount(); ++i) {
                std::string column_name = row.GetColumnName(i);
                std::string value = row.GetColumnAsString(i);
                row_data[column_name] = value;
            }
            
            results.push_back(row_data);
        }
    } catch (const std::exception& e) {
        std::cerr << "Query failed: " << e.what() << std::endl;
    }
    
    return results;
}

} // namespace RouterSim
