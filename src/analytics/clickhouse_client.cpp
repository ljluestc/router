#include "clickhouse_client.h"
#include <iostream>
#include <sstream>
#include <curl/curl.h>
#include <json/json.h>
#include <chrono>
#include <thread>

namespace RouterSim {

ClickHouseClient::ClickHouseClient(const std::string& host, uint16_t port, 
                                   const std::string& database, const std::string& username, 
                                   const std::string& password)
    : host_(host), port_(port), database_(database), username_(username), password_(password) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl_ = curl_easy_init();
}

ClickHouseClient::~ClickHouseClient() {
    if (curl_) {
        curl_easy_cleanup(curl_);
    }
    curl_global_cleanup();
}

bool ClickHouseClient::connect() {
    if (!curl_) {
        std::cerr << "Failed to initialize CURL" << std::endl;
        return false;
    }

    // Test connection
    std::string query = "SELECT 1";
    std::string result = executeQuery(query);
    
    if (result.empty()) {
        std::cerr << "Failed to connect to ClickHouse" << std::endl;
        return false;
    }

    connected_ = true;
    std::cout << "Connected to ClickHouse at " << host_ << ":" << port_ << std::endl;
    return true;
}

void ClickHouseClient::disconnect() {
    connected_ = false;
}

bool ClickHouseClient::createTables() {
    if (!connected_) {
        if (!connect()) {
            return false;
        }
    }

    // Create metrics table
    std::string createMetricsTable = R"(
        CREATE TABLE IF NOT EXISTS network_metrics (
            timestamp DateTime64(3),
            node_id String,
            metric_type String,
            value Float64,
            tags Map(String, String),
            INDEX idx_timestamp timestamp TYPE minmax GRANULARITY 1,
            INDEX idx_node_id node_id TYPE bloom_filter GRANULARITY 1,
            INDEX idx_metric_type metric_type TYPE bloom_filter GRANULARITY 1
        ) ENGINE = MergeTree()
        ORDER BY (timestamp, node_id, metric_type)
        PARTITION BY toYYYYMM(timestamp)
    )";

    if (!executeQuery(createMetricsTable).empty()) {
        std::cerr << "Failed to create metrics table" << std::endl;
        return false;
    }

    // Create events table
    std::string createEventsTable = R"(
        CREATE TABLE IF NOT EXISTS network_events (
            timestamp DateTime64(3),
            event_type String,
            node_id String,
            description String,
            severity String,
            metadata Map(String, String),
            INDEX idx_timestamp timestamp TYPE minmax GRANULARITY 1,
            INDEX idx_event_type event_type TYPE bloom_filter GRANULARITY 1,
            INDEX idx_node_id node_id TYPE bloom_filter GRANULARITY 1,
            INDEX idx_severity severity TYPE bloom_filter GRANULARITY 1
        ) ENGINE = MergeTree()
        ORDER BY (timestamp, event_type, node_id)
        PARTITION BY toYYYYMM(timestamp)
    )";

    if (!executeQuery(createEventsTable).empty()) {
        std::cerr << "Failed to create events table" << std::endl;
        return false;
    }

    // Create health table
    std::string createHealthTable = R"(
        CREATE TABLE IF NOT EXISTS network_health (
            timestamp DateTime64(3),
            overall_score Float64,
            latency_ms Float64,
            packet_loss_percent Float64,
            throughput_mbps Float64,
            error_count UInt32,
            warning_count UInt32,
            active_nodes UInt32,
            total_nodes UInt32,
            INDEX idx_timestamp timestamp TYPE minmax GRANULARITY 1
        ) ENGINE = MergeTree()
        ORDER BY timestamp
        PARTITION BY toYYYYMM(timestamp)
    )";

    if (!executeQuery(createHealthTable).empty()) {
        std::cerr << "Failed to create health table" << std::endl;
        return false;
    }

    std::cout << "ClickHouse tables created successfully" << std::endl;
    return true;
}

bool ClickHouseClient::insertMetric(const NetworkMetric& metric) {
    if (!connected_) {
        return false;
    }

    std::stringstream query;
    query << "INSERT INTO " << database_ << ".network_metrics VALUES (";
    query << "toDateTime64(" << metric.timestamp.count() << ", 3), ";
    query << "'" << metric.node_id << "', ";
    query << "'" << metricTypeToString(metric.metric_type) << "', ";
    query << metric.value << ", ";
    query << "map(";
    
    // Convert tags to ClickHouse map format
    bool first = true;
    for (const auto& tag : metric.tags) {
        if (!first) query << ", ";
        query << "'" << tag.first << "', '" << tag.second << "'";
        first = false;
    }
    query << "))";

    std::string result = executeQuery(query.str());
    return result.empty(); // Empty result means success
}

bool ClickHouseClient::insertEvent(const NetworkEvent& event) {
    if (!connected_) {
        return false;
    }

    std::stringstream query;
    query << "INSERT INTO " << database_ << ".network_events VALUES (";
    query << "toDateTime64(" << event.timestamp.count() << ", 3), ";
    query << "'" << eventTypeToString(event.event_type) << "', ";
    query << "'" << event.node_id << "', ";
    query << "'" << event.description << "', ";
    query << "'" << severityToString(event.severity) << "', ";
    query << "map(";
    
    // Convert metadata to ClickHouse map format
    bool first = true;
    for (const auto& meta : event.metadata) {
        if (!first) query << ", ";
        query << "'" << meta.first << "', '" << meta.second << "'";
        first = false;
    }
    query << "))";

    std::string result = executeQuery(query.str());
    return result.empty();
}

bool ClickHouseClient::insertHealth(const NetworkHealth& health) {
    if (!connected_) {
        return false;
    }

    std::stringstream query;
    query << "INSERT INTO " << database_ << ".network_health VALUES (";
    query << "toDateTime64(" << health.timestamp.count() << ", 3), ";
    query << health.overall_score << ", ";
    query << health.latency_ms << ", ";
    query << health.packet_loss_percent << ", ";
    query << health.throughput_mbps << ", ";
    query << health.error_count << ", ";
    query << health.warning_count << ", ";
    query << health.active_nodes << ", ";
    query << health.total_nodes << ")";

    std::string result = executeQuery(query.str());
    return result.empty();
}

std::vector<NetworkMetric> ClickHouseClient::queryMetrics(const AnalyticsQuery& query) {
    std::vector<NetworkMetric> results;
    
    if (!connected_) {
        return results;
    }

    std::stringstream sql;
    sql << "SELECT timestamp, node_id, metric_type, value, tags FROM " << database_ << ".network_metrics ";
    sql << "WHERE timestamp >= toDateTime64(" << query.start_time.count() << ", 3) ";
    sql << "AND timestamp <= toDateTime64(" << query.end_time.count() << ", 3) ";

    if (!query.node_ids.empty()) {
        sql << "AND node_id IN (";
        for (size_t i = 0; i < query.node_ids.size(); ++i) {
            if (i > 0) sql << ", ";
            sql << "'" << query.node_ids[i] << "'";
        }
        sql << ") ";
    }

    if (!query.metric_types.empty()) {
        sql << "AND metric_type IN (";
        for (size_t i = 0; i < query.metric_types.size(); ++i) {
            if (i > 0) sql << ", ";
            sql << "'" << metricTypeToString(query.metric_types[i]) << "'";
        }
        sql << ") ";
    }

    sql << "ORDER BY timestamp";

    std::string result = executeQuery(sql.str());
    results = parseMetrics(result);
    
    return results;
}

std::vector<NetworkEvent> ClickHouseClient::queryEvents(const EventQuery& query) {
    std::vector<NetworkEvent> results;
    
    if (!connected_) {
        return results;
    }

    std::stringstream sql;
    sql << "SELECT timestamp, event_type, node_id, description, severity, metadata FROM " << database_ << ".network_events ";
    sql << "WHERE timestamp >= toDateTime64(" << query.start_time.count() << ", 3) ";
    sql << "AND timestamp <= toDateTime64(" << query.end_time.count() << ", 3) ";

    if (!query.node_ids.empty()) {
        sql << "AND node_id IN (";
        for (size_t i = 0; i < query.node_ids.size(); ++i) {
            if (i > 0) sql << ", ";
            sql << "'" << query.node_ids[i] << "'";
        }
        sql << ") ";
    }

    if (!query.event_types.empty()) {
        sql << "AND event_type IN (";
        for (size_t i = 0; i < query.event_types.size(); ++i) {
            if (i > 0) sql << ", ";
            sql << "'" << eventTypeToString(query.event_types[i]) << "'";
        }
        sql << ") ";
    }

    sql << "ORDER BY timestamp";

    std::string result = executeQuery(sql.str());
    results = parseEvents(result);
    
    return results;
}

NetworkHealth ClickHouseClient::getNetworkHealth() {
    NetworkHealth health;
    
    if (!connected_) {
        return health;
    }

    // Get latest health record
    std::stringstream sql;
    sql << "SELECT * FROM " << database_ << ".network_health ";
    sql << "ORDER BY timestamp DESC LIMIT 1";

    std::string result = executeQuery(sql.str());
    health = parseHealth(result);
    
    return health;
}

std::string ClickHouseClient::executeQuery(const std::string& query) {
    if (!curl_) {
        return "";
    }

    std::string response;
    
    // Set up CURL options
    curl_easy_setopt(curl_, CURLOPT_URL, ("http://" + host_ + ":" + std::to_string(port_) + "/").c_str());
    curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, query.c_str());
    curl_easy_setopt(curl_, CURLOPT_POSTFIELDSIZE, query.length());
    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response);
    
    // Set authentication
    std::string auth = username_ + ":" + password_;
    curl_easy_setopt(curl_, CURLOPT_USERPWD, auth.c_str());
    
    // Set headers
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: text/plain");
    curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers);
    
    // Execute request
    CURLcode res = curl_easy_perform(curl_);
    
    // Clean up
    curl_slist_free_all(headers);
    
    if (res != CURLE_OK) {
        std::cerr << "CURL error: " << curl_easy_strerror(res) << std::endl;
        return "";
    }
    
    return response;
}

size_t ClickHouseClient::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t total_size = size * nmemb;
    std::string* response = static_cast<std::string*>(userp);
    response->append(static_cast<char*>(contents), total_size);
    return total_size;
}

std::string ClickHouseClient::metricTypeToString(MetricType type) {
    switch (type) {
        case MetricType::CpuUsage: return "cpu_usage";
        case MetricType::MemoryUsage: return "memory_usage";
        case MetricType::NetworkIn: return "network_in";
        case MetricType::NetworkOut: return "network_out";
        case MetricType::PacketLoss: return "packet_loss";
        case MetricType::Latency: return "latency";
        case MetricType::Throughput: return "throughput";
        case MetricType::ConnectionCount: return "connection_count";
        case MetricType::ErrorRate: return "error_rate";
        default: return "unknown";
    }
}

std::string ClickHouseClient::eventTypeToString(EventType type) {
    switch (type) {
        case EventType::GatewayConnected: return "gateway_connected";
        case EventType::GatewayDisconnected: return "gateway_disconnected";
        case EventType::HighLatency: return "high_latency";
        case EventType::PacketLoss: return "packet_loss";
        case EventType::ConnectionError: return "connection_error";
        case EventType::ConfigurationChange: return "configuration_change";
        case EventType::Maintenance: return "maintenance";
        default: return "unknown";
    }
}

std::string ClickHouseClient::severityToString(Severity severity) {
    switch (severity) {
        case Severity::Info: return "info";
        case Severity::Warning: return "warning";
        case Severity::Error: return "error";
        case Severity::Critical: return "critical";
        default: return "unknown";
    }
}

std::vector<NetworkMetric> ClickHouseClient::parseMetrics(const std::string& response) {
    std::vector<NetworkMetric> metrics;
    
    // Simple TSV parsing (ClickHouse returns TSV by default)
    std::istringstream stream(response);
    std::string line;
    
    while (std::getline(stream, line)) {
        if (line.empty()) continue;
        
        NetworkMetric metric;
        std::istringstream line_stream(line);
        std::string token;
        std::vector<std::string> tokens;
        
        while (std::getline(line_stream, token, '\t')) {
            tokens.push_back(token);
        }
        
        if (tokens.size() >= 4) {
            // Parse timestamp (assuming Unix timestamp)
            metric.timestamp = std::chrono::system_clock::time_point(
                std::chrono::milliseconds(std::stoll(tokens[0]))
            );
            metric.node_id = tokens[1];
            metric.metric_type = stringToMetricType(tokens[2]);
            metric.value = std::stod(tokens[3]);
            
            // Parse tags (simplified)
            if (tokens.size() > 4) {
                // TODO: Parse tags from tokens[4]
            }
            
            metrics.push_back(metric);
        }
    }
    
    return metrics;
}

std::vector<NetworkEvent> ClickHouseClient::parseEvents(const std::string& response) {
    std::vector<NetworkEvent> events;
    
    // Similar to parseMetrics but for events
    std::istringstream stream(response);
    std::string line;
    
    while (std::getline(stream, line)) {
        if (line.empty()) continue;
        
        NetworkEvent event;
        std::istringstream line_stream(line);
        std::string token;
        std::vector<std::string> tokens;
        
        while (std::getline(line_stream, token, '\t')) {
            tokens.push_back(token);
        }
        
        if (tokens.size() >= 5) {
            event.timestamp = std::chrono::system_clock::time_point(
                std::chrono::milliseconds(std::stoll(tokens[0]))
            );
            event.event_type = stringToEventType(tokens[1]);
            event.node_id = tokens[2];
            event.description = tokens[3];
            event.severity = stringToSeverity(tokens[4]);
            
            events.push_back(event);
        }
    }
    
    return events;
}

NetworkHealth ClickHouseClient::parseHealth(const std::string& response) {
    NetworkHealth health;
    
    if (response.empty()) {
        return health;
    }
    
    // Parse single health record
    std::istringstream stream(response);
    std::string line;
    
    if (std::getline(stream, line)) {
        std::istringstream line_stream(line);
        std::string token;
        std::vector<std::string> tokens;
        
        while (std::getline(line_stream, token, '\t')) {
            tokens.push_back(token);
        }
        
        if (tokens.size() >= 9) {
            health.timestamp = std::chrono::system_clock::time_point(
                std::chrono::milliseconds(std::stoll(tokens[0]))
            );
            health.overall_score = std::stod(tokens[1]);
            health.latency_ms = std::stod(tokens[2]);
            health.packet_loss_percent = std::stod(tokens[3]);
            health.throughput_mbps = std::stod(tokens[4]);
            health.error_count = std::stoul(tokens[5]);
            health.warning_count = std::stoul(tokens[6]);
            health.active_nodes = std::stoul(tokens[7]);
            health.total_nodes = std::stoul(tokens[8]);
        }
    }
    
    return health;
}

MetricType ClickHouseClient::stringToMetricType(const std::string& str) {
    if (str == "cpu_usage") return MetricType::CpuUsage;
    if (str == "memory_usage") return MetricType::MemoryUsage;
    if (str == "network_in") return MetricType::NetworkIn;
    if (str == "network_out") return MetricType::NetworkOut;
    if (str == "packet_loss") return MetricType::PacketLoss;
    if (str == "latency") return MetricType::Latency;
    if (str == "throughput") return MetricType::Throughput;
    if (str == "connection_count") return MetricType::ConnectionCount;
    if (str == "error_rate") return MetricType::ErrorRate;
    return MetricType::CpuUsage; // Default
}

EventType ClickHouseClient::stringToEventType(const std::string& str) {
    if (str == "gateway_connected") return EventType::GatewayConnected;
    if (str == "gateway_disconnected") return EventType::GatewayDisconnected;
    if (str == "high_latency") return EventType::HighLatency;
    if (str == "packet_loss") return EventType::PacketLoss;
    if (str == "connection_error") return EventType::ConnectionError;
    if (str == "configuration_change") return EventType::ConfigurationChange;
    if (str == "maintenance") return EventType::Maintenance;
    return EventType::GatewayConnected; // Default
}

Severity ClickHouseClient::stringToSeverity(const std::string& str) {
    if (str == "info") return Severity::Info;
    if (str == "warning") return Severity::Warning;
    if (str == "error") return Severity::Error;
    if (str == "critical") return Severity::Critical;
    return Severity::Info; // Default
}

} // namespace RouterSim
