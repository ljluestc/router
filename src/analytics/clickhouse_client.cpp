#include "analytics/clickhouse_client.h"
#include <clickhouse/client.h>
#include <clickhouse/columns/column.h>
#include <clickhouse/columns/string.h>
#include <clickhouse/columns/numeric.h>
#include <clickhouse/columns/date.h>
#include <clickhouse/columns/ipv4.h>
#include <clickhouse/columns/ipv6.h>
#include <iostream>
#include <chrono>
#include <thread>

namespace RouterSim {
namespace Analytics {

ClickHouseClient::ClickHouseClient(const std::string& host, uint16_t port, const std::string& database)
    : host_(host), port_(port), database_(database), connected_(false) {
    // Initialize ClickHouse client
    clickhouse::ClientOptions opts;
    opts.SetHost(host_);
    opts.SetPort(port_);
    opts.SetDefaultDatabase(database_);
    opts.SetUser("default");
    opts.SetPassword("");
    opts.SetCompressionMethod(clickhouse::CompressionMethod::LZ4);
    
    client_ = std::make_unique<clickhouse::Client>(opts);
}

ClickHouseClient::~ClickHouseClient() {
    if (connected_) {
        disconnect();
    }
}

bool ClickHouseClient::connect() {
    try {
        // Test connection
        client_->Execute("SELECT 1");
        connected_ = true;
        
        // Create database if it doesn't exist
        createDatabase();
        
        // Create tables
        createTables();
        
        std::cout << "Connected to ClickHouse at " << host_ << ":" << port_ << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to connect to ClickHouse: " << e.what() << std::endl;
        return false;
    }
}

void ClickHouseClient::disconnect() {
    if (connected_) {
        connected_ = false;
        std::cout << "Disconnected from ClickHouse" << std::endl;
    }
}

void ClickHouseClient::createDatabase() {
    try {
        std::string query = "CREATE DATABASE IF NOT EXISTS " + database_;
        client_->Execute(query);
    } catch (const std::exception& e) {
        std::cerr << "Failed to create database: " << e.what() << std::endl;
    }
}

void ClickHouseClient::createTables() {
    // Create routing_events table
    std::string routing_events_sql = R"(
        CREATE TABLE IF NOT EXISTS routing_events (
            timestamp DateTime64(3),
            router_id String,
            protocol String,
            event_type String,
            prefix String,
            next_hop String,
            metric UInt32,
            as_path Array(String),
            community Array(String),
            local_pref UInt32,
            origin String,
            path_id UInt32,
            peer_ip String,
            peer_as UInt32,
            session_id String,
            duration_ms UInt32,
            success UInt8
        ) ENGINE = MergeTree()
        ORDER BY (timestamp, router_id, protocol)
        PARTITION BY toYYYYMM(timestamp)
    )";
    
    // Create traffic_flows table
    std::string traffic_flows_sql = R"(
        CREATE TABLE IF NOT EXISTS traffic_flows (
            timestamp DateTime64(3),
            router_id String,
            interface String,
            src_ip String,
            dst_ip String,
            src_port UInt16,
            dst_port UInt16,
            protocol String,
            bytes UInt64,
            packets UInt64,
            duration_ms UInt32,
            tcp_flags UInt8,
            tos UInt8,
            ttl UInt8,
            flow_id String,
            vlan_id UInt16,
            mpls_label UInt32
        ) ENGINE = MergeTree()
        ORDER BY (timestamp, router_id, interface)
        PARTITION BY toYYYYMM(timestamp)
    )";
    
    // Create performance_metrics table
    std::string performance_metrics_sql = R"(
        CREATE TABLE IF NOT EXISTS performance_metrics (
            timestamp DateTime64(3),
            router_id String,
            metric_name String,
            metric_value Float64,
            metric_unit String,
            tags Map(String, String),
            aggregation_level String
        ) ENGINE = MergeTree()
        ORDER BY (timestamp, router_id, metric_name)
        PARTITION BY toYYYYMM(timestamp)
    )";
    
    // Create network_topology table
    std::string network_topology_sql = R"(
        CREATE TABLE IF NOT EXISTS network_topology (
            timestamp DateTime64(3),
            router_id String,
            neighbor_id String,
            neighbor_ip String,
            neighbor_as UInt32,
            protocol String,
            state String,
            uptime UInt32,
            keepalive_interval UInt16,
            hold_time UInt16,
            capabilities Array(String),
            remote_capabilities Array(String),
            session_id String
        ) ENGINE = MergeTree()
        ORDER BY (timestamp, router_id, neighbor_id)
        PARTITION BY toYYYYMM(timestamp)
    )";
    
    try {
        client_->Execute(routing_events_sql);
        client_->Execute(traffic_flows_sql);
        client_->Execute(performance_metrics_sql);
        client_->Execute(network_topology_sql);
        std::cout << "Created ClickHouse tables successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Failed to create tables: " << e.what() << std::endl;
    }
}

void ClickHouseClient::insertRoutingEvent(const RoutingEvent& event) {
    if (!connected_) return;
    
    try {
        auto block = clickhouse::Block();
        
        // Add columns
        block.AppendColumn("timestamp", std::make_shared<clickhouse::ColumnDateTime64>(3, std::vector<int64_t>{event.timestamp}));
        block.AppendColumn("router_id", std::make_shared<clickhouse::ColumnString>(std::vector<std::string>{event.router_id}));
        block.AppendColumn("protocol", std::make_shared<clickhouse::ColumnString>(std::vector<std::string>{event.protocol}));
        block.AppendColumn("event_type", std::make_shared<clickhouse::ColumnString>(std::vector<std::string>{event.event_type}));
        block.AppendColumn("prefix", std::make_shared<clickhouse::ColumnString>(std::vector<std::string>{event.prefix}));
        block.AppendColumn("next_hop", std::make_shared<clickhouse::ColumnString>(std::vector<std::string>{event.next_hop}));
        block.AppendColumn("metric", std::make_shared<clickhouse::ColumnUInt32>(std::vector<uint32_t>{event.metric}));
        block.AppendColumn("as_path", std::make_shared<clickhouse::ColumnArray>(std::make_shared<clickhouse::ColumnString>(event.as_path)));
        block.AppendColumn("community", std::make_shared<clickhouse::ColumnArray>(std::make_shared<clickhouse::ColumnString>(event.community)));
        block.AppendColumn("local_pref", std::make_shared<clickhouse::ColumnUInt32>(std::vector<uint32_t>{event.local_pref}));
        block.AppendColumn("origin", std::make_shared<clickhouse::ColumnString>(std::vector<std::string>{event.origin}));
        block.AppendColumn("path_id", std::make_shared<clickhouse::ColumnUInt32>(std::vector<uint32_t>{event.path_id}));
        block.AppendColumn("peer_ip", std::make_shared<clickhouse::ColumnString>(std::vector<std::string>{event.peer_ip}));
        block.AppendColumn("peer_as", std::make_shared<clickhouse::ColumnUInt32>(std::vector<uint32_t>{event.peer_as}));
        block.AppendColumn("session_id", std::make_shared<clickhouse::ColumnString>(std::vector<std::string>{event.session_id}));
        block.AppendColumn("duration_ms", std::make_shared<clickhouse::ColumnUInt32>(std::vector<uint32_t>{event.duration_ms}));
        block.AppendColumn("success", std::make_shared<clickhouse::ColumnUInt8>(std::vector<uint8_t>{event.success}));
        
        client_->Insert("routing_events", block);
    } catch (const std::exception& e) {
        std::cerr << "Failed to insert routing event: " << e.what() << std::endl;
    }
}

void ClickHouseClient::insertTrafficFlow(const TrafficFlow& flow) {
    if (!connected_) return;
    
    try {
        auto block = clickhouse::Block();
        
        // Add columns
        block.AppendColumn("timestamp", std::make_shared<clickhouse::ColumnDateTime64>(3, std::vector<int64_t>{flow.timestamp}));
        block.AppendColumn("router_id", std::make_shared<clickhouse::ColumnString>(std::vector<std::string>{flow.router_id}));
        block.AppendColumn("interface", std::make_shared<clickhouse::ColumnString>(std::vector<std::string>{flow.interface}));
        block.AppendColumn("src_ip", std::make_shared<clickhouse::ColumnString>(std::vector<std::string>{flow.src_ip}));
        block.AppendColumn("dst_ip", std::make_shared<clickhouse::ColumnString>(std::vector<std::string>{flow.dst_ip}));
        block.AppendColumn("src_port", std::make_shared<clickhouse::ColumnUInt16>(std::vector<uint16_t>{flow.src_port}));
        block.AppendColumn("dst_port", std::make_shared<clickhouse::ColumnUInt16>(std::vector<uint16_t>{flow.dst_port}));
        block.AppendColumn("protocol", std::make_shared<clickhouse::ColumnString>(std::vector<std::string>{flow.protocol}));
        block.AppendColumn("bytes", std::make_shared<clickhouse::ColumnUInt64>(std::vector<uint64_t>{flow.bytes}));
        block.AppendColumn("packets", std::make_shared<clickhouse::ColumnUInt64>(std::vector<uint64_t>{flow.packets}));
        block.AppendColumn("duration_ms", std::make_shared<clickhouse::ColumnUInt32>(std::vector<uint32_t>{flow.duration_ms}));
        block.AppendColumn("tcp_flags", std::make_shared<clickhouse::ColumnUInt8>(std::vector<uint8_t>{flow.tcp_flags}));
        block.AppendColumn("tos", std::make_shared<clickhouse::ColumnUInt8>(std::vector<uint8_t>{flow.tos}));
        block.AppendColumn("ttl", std::make_shared<clickhouse::ColumnUInt8>(std::vector<uint8_t>{flow.ttl}));
        block.AppendColumn("flow_id", std::make_shared<clickhouse::ColumnString>(std::vector<std::string>{flow.flow_id}));
        block.AppendColumn("vlan_id", std::make_shared<clickhouse::ColumnUInt16>(std::vector<uint16_t>{flow.vlan_id}));
        block.AppendColumn("mpls_label", std::make_shared<clickhouse::ColumnUInt32>(std::vector<uint32_t>{flow.mpls_label}));
        
        client_->Insert("traffic_flows", block);
    } catch (const std::exception& e) {
        std::cerr << "Failed to insert traffic flow: " << e.what() << std::endl;
    }
}

void ClickHouseClient::insertPerformanceMetric(const PerformanceMetric& metric) {
    if (!connected_) return;
    
    try {
        auto block = clickhouse::Block();
        
        // Add columns
        block.AppendColumn("timestamp", std::make_shared<clickhouse::ColumnDateTime64>(3, std::vector<int64_t>{metric.timestamp}));
        block.AppendColumn("router_id", std::make_shared<clickhouse::ColumnString>(std::vector<std::string>{metric.router_id}));
        block.AppendColumn("metric_name", std::make_shared<clickhouse::ColumnString>(std::vector<std::string>{metric.metric_name}));
        block.AppendColumn("metric_value", std::make_shared<clickhouse::ColumnFloat64>(std::vector<double>{metric.metric_value}));
        block.AppendColumn("metric_unit", std::make_shared<clickhouse::ColumnString>(std::vector<std::string>{metric.metric_unit}));
        
        // Convert tags map to ClickHouse format
        std::vector<std::string> tag_keys, tag_values;
        for (const auto& tag : metric.tags) {
            tag_keys.push_back(tag.first);
            tag_values.push_back(tag.second);
        }
        
        block.AppendColumn("tags", std::make_shared<clickhouse::ColumnMap>(
            std::make_shared<clickhouse::ColumnString>(tag_keys),
            std::make_shared<clickhouse::ColumnString>(tag_values)
        ));
        block.AppendColumn("aggregation_level", std::make_shared<clickhouse::ColumnString>(std::vector<std::string>{metric.aggregation_level}));
        
        client_->Insert("performance_metrics", block);
    } catch (const std::exception& e) {
        std::cerr << "Failed to insert performance metric: " << e.what() << std::endl;
    }
}

void ClickHouseClient::insertNetworkTopology(const NetworkTopology& topology) {
    if (!connected_) return;
    
    try {
        auto block = clickhouse::Block();
        
        // Add columns
        block.AppendColumn("timestamp", std::make_shared<clickhouse::ColumnDateTime64>(3, std::vector<int64_t>{topology.timestamp}));
        block.AppendColumn("router_id", std::make_shared<clickhouse::ColumnString>(std::vector<std::string>{topology.router_id}));
        block.AppendColumn("neighbor_id", std::make_shared<clickhouse::ColumnString>(std::vector<std::string>{topology.neighbor_id}));
        block.AppendColumn("neighbor_ip", std::make_shared<clickhouse::ColumnString>(std::vector<std::string>{topology.neighbor_ip}));
        block.AppendColumn("neighbor_as", std::make_shared<clickhouse::ColumnUInt32>(std::vector<uint32_t>{topology.neighbor_as}));
        block.AppendColumn("protocol", std::make_shared<clickhouse::ColumnString>(std::vector<std::string>{topology.protocol}));
        block.AppendColumn("state", std::make_shared<clickhouse::ColumnString>(std::vector<std::string>{topology.state}));
        block.AppendColumn("uptime", std::make_shared<clickhouse::ColumnUInt32>(std::vector<uint32_t>{topology.uptime}));
        block.AppendColumn("keepalive_interval", std::make_shared<clickhouse::ColumnUInt16>(std::vector<uint16_t>{topology.keepalive_interval}));
        block.AppendColumn("hold_time", std::make_shared<clickhouse::ColumnUInt16>(std::vector<uint16_t>{topology.hold_time}));
        block.AppendColumn("capabilities", std::make_shared<clickhouse::ColumnArray>(std::make_shared<clickhouse::ColumnString>(topology.capabilities)));
        block.AppendColumn("remote_capabilities", std::make_shared<clickhouse::ColumnArray>(std::make_shared<clickhouse::ColumnString>(topology.remote_capabilities)));
        block.AppendColumn("session_id", std::make_shared<clickhouse::ColumnString>(std::vector<std::string>{topology.session_id}));
        
        client_->Insert("network_topology", block);
    } catch (const std::exception& e) {
        std::cerr << "Failed to insert network topology: " << e.what() << std::endl;
    }
}

std::vector<RoutingEvent> ClickHouseClient::queryRoutingEvents(const std::string& filter, uint32_t limit) {
    std::vector<RoutingEvent> events;
    if (!connected_) return events;
    
    try {
        std::string query = "SELECT * FROM routing_events";
        if (!filter.empty()) {
            query += " WHERE " + filter;
        }
        query += " ORDER BY timestamp DESC";
        if (limit > 0) {
            query += " LIMIT " + std::to_string(limit);
        }
        
        auto result = client_->Select(query);
        
        // Process results (simplified - in real implementation, you'd parse the result)
        // This is a placeholder for the actual result parsing
        
    } catch (const std::exception& e) {
        std::cerr << "Failed to query routing events: " << e.what() << std::endl;
    }
    
    return events;
}

std::vector<TrafficFlow> ClickHouseClient::queryTrafficFlows(const std::string& filter, uint32_t limit) {
    std::vector<TrafficFlow> flows;
    if (!connected_) return flows;
    
    try {
        std::string query = "SELECT * FROM traffic_flows";
        if (!filter.empty()) {
            query += " WHERE " + filter;
        }
        query += " ORDER BY timestamp DESC";
        if (limit > 0) {
            query += " LIMIT " + std::to_string(limit);
        }
        
        auto result = client_->Select(query);
        
        // Process results (simplified - in real implementation, you'd parse the result)
        
    } catch (const std::exception& e) {
        std::cerr << "Failed to query traffic flows: " << e.what() << std::endl;
    }
    
    return flows;
}

std::vector<PerformanceMetric> ClickHouseClient::queryPerformanceMetrics(const std::string& filter, uint32_t limit) {
    std::vector<PerformanceMetric> metrics;
    if (!connected_) return metrics;
    
    try {
        std::string query = "SELECT * FROM performance_metrics";
        if (!filter.empty()) {
            query += " WHERE " + filter;
        }
        query += " ORDER BY timestamp DESC";
        if (limit > 0) {
            query += " LIMIT " + std::to_string(limit);
        }
        
        auto result = client_->Select(query);
        
        // Process results (simplified - in real implementation, you'd parse the result)
        
    } catch (const std::exception& e) {
        std::cerr << "Failed to query performance metrics: " << e.what() << std::endl;
    }
    
    return metrics;
}

void ClickHouseClient::batchInsert(const std::vector<RoutingEvent>& events) {
    if (!connected_ || events.empty()) return;
    
    try {
        auto block = clickhouse::Block();
        
        // Prepare vectors for batch insert
        std::vector<int64_t> timestamps;
        std::vector<std::string> router_ids, protocols, event_types, prefixes, next_hops;
        std::vector<uint32_t> metrics, local_prefs, path_ids, peer_ass, duration_mss;
        std::vector<std::string> origins, peer_ips, session_ids;
        std::vector<uint8_t> successes;
        std::vector<std::vector<std::string>> as_paths, communities;
        
        for (const auto& event : events) {
            timestamps.push_back(event.timestamp);
            router_ids.push_back(event.router_id);
            protocols.push_back(event.protocol);
            event_types.push_back(event.event_type);
            prefixes.push_back(event.prefix);
            next_hops.push_back(event.next_hop);
            metrics.push_back(event.metric);
            as_paths.push_back(event.as_path);
            communities.push_back(event.community);
            local_prefs.push_back(event.local_pref);
            origins.push_back(event.origin);
            path_ids.push_back(event.path_id);
            peer_ips.push_back(event.peer_ip);
            peer_ass.push_back(event.peer_as);
            session_ids.push_back(event.session_id);
            duration_mss.push_back(event.duration_ms);
            successes.push_back(event.success);
        }
        
        // Add columns
        block.AppendColumn("timestamp", std::make_shared<clickhouse::ColumnDateTime64>(3, timestamps));
        block.AppendColumn("router_id", std::make_shared<clickhouse::ColumnString>(router_ids));
        block.AppendColumn("protocol", std::make_shared<clickhouse::ColumnString>(protocols));
        block.AppendColumn("event_type", std::make_shared<clickhouse::ColumnString>(event_types));
        block.AppendColumn("prefix", std::make_shared<clickhouse::ColumnString>(prefixes));
        block.AppendColumn("next_hop", std::make_shared<clickhouse::ColumnString>(next_hops));
        block.AppendColumn("metric", std::make_shared<clickhouse::ColumnUInt32>(metrics));
        block.AppendColumn("as_path", std::make_shared<clickhouse::ColumnArray>(std::make_shared<clickhouse::ColumnString>(as_paths)));
        block.AppendColumn("community", std::make_shared<clickhouse::ColumnArray>(std::make_shared<clickhouse::ColumnString>(communities)));
        block.AppendColumn("local_pref", std::make_shared<clickhouse::ColumnUInt32>(local_prefs));
        block.AppendColumn("origin", std::make_shared<clickhouse::ColumnString>(origins));
        block.AppendColumn("path_id", std::make_shared<clickhouse::ColumnUInt32>(path_ids));
        block.AppendColumn("peer_ip", std::make_shared<clickhouse::ColumnString>(peer_ips));
        block.AppendColumn("peer_as", std::make_shared<clickhouse::ColumnUInt32>(peer_ass));
        block.AppendColumn("session_id", std::make_shared<clickhouse::ColumnString>(session_ids));
        block.AppendColumn("duration_ms", std::make_shared<clickhouse::ColumnUInt32>(duration_mss));
        block.AppendColumn("success", std::make_shared<clickhouse::ColumnUInt8>(successes));
        
        client_->Insert("routing_events", block);
        
    } catch (const std::exception& e) {
        std::cerr << "Failed to batch insert routing events: " << e.what() << std::endl;
    }
}

} // namespace Analytics
} // namespace RouterSim
