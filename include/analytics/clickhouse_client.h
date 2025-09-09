#pragma once

#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <curl/curl.h>

namespace RouterSim {

// Forward declarations
struct NetworkMetric;
struct NetworkEvent;
struct NetworkHealth;
struct AnalyticsQuery;
struct EventQuery;

// Metric types
enum class MetricType {
    CpuUsage,
    MemoryUsage,
    NetworkIn,
    NetworkOut,
    PacketLoss,
    Latency,
    Throughput,
    ConnectionCount,
    ErrorRate
};

// Event types
enum class EventType {
    GatewayConnected,
    GatewayDisconnected,
    HighLatency,
    PacketLoss,
    ConnectionError,
    ConfigurationChange,
    Maintenance
};

// Severity levels
enum class Severity {
    Info,
    Warning,
    Error,
    Critical
};

// Network metric structure
struct NetworkMetric {
    std::chrono::system_clock::time_point timestamp;
    std::string node_id;
    MetricType metric_type;
    double value;
    std::map<std::string, std::string> tags;
    
    NetworkMetric() : value(0.0) {}
    NetworkMetric(const std::string& node_id, MetricType type, double val)
        : node_id(node_id), metric_type(type), value(val) {
        timestamp = std::chrono::system_clock::now();
    }
};

// Network event structure
struct NetworkEvent {
    std::chrono::system_clock::time_point timestamp;
    EventType event_type;
    std::string node_id;
    std::string description;
    Severity severity;
    std::map<std::string, std::string> metadata;
    
    NetworkEvent() {}
    NetworkEvent(EventType type, const std::string& node_id, const std::string& desc, Severity sev)
        : event_type(type), node_id(node_id), description(desc), severity(sev) {
        timestamp = std::chrono::system_clock::now();
    }
};

// Network health structure
struct NetworkHealth {
    std::chrono::system_clock::time_point timestamp;
    double overall_score;
    double latency_ms;
    double packet_loss_percent;
    double throughput_mbps;
    uint32_t error_count;
    uint32_t warning_count;
    uint32_t active_nodes;
    uint32_t total_nodes;
    
    NetworkHealth() : overall_score(0.0), latency_ms(0.0), packet_loss_percent(0.0),
                     throughput_mbps(0.0), error_count(0), warning_count(0),
                     active_nodes(0), total_nodes(0) {
        timestamp = std::chrono::system_clock::now();
    }
};

// Analytics query structure
struct AnalyticsQuery {
    std::chrono::system_clock::time_point start_time;
    std::chrono::system_clock::time_point end_time;
    std::vector<std::string> node_ids;
    std::vector<MetricType> metric_types;
    std::string aggregation;
    std::vector<std::string> group_by;
    
    AnalyticsQuery() {
        end_time = std::chrono::system_clock::now();
        start_time = end_time - std::chrono::hours(1);
    }
};

// Event query structure
struct EventQuery {
    std::chrono::system_clock::time_point start_time;
    std::chrono::system_clock::time_point end_time;
    std::vector<std::string> node_ids;
    std::vector<EventType> event_types;
    std::vector<Severity> severities;
    
    EventQuery() {
        end_time = std::chrono::system_clock::now();
        start_time = end_time - std::chrono::hours(1);
    }
};

// ClickHouse client class
class ClickHouseClient {
public:
    ClickHouseClient(const std::string& host, uint16_t port, 
                     const std::string& database, const std::string& username, 
                     const std::string& password);
    ~ClickHouseClient();

    // Connection management
    bool connect();
    void disconnect();
    bool isConnected() const { return connected_; }

    // Table management
    bool createTables();
    bool optimizeTables();
    bool dropTables();

    // Data insertion
    bool insertMetric(const NetworkMetric& metric);
    bool insertEvent(const NetworkEvent& event);
    bool insertHealth(const NetworkHealth& health);
    bool insertBatch(const std::vector<NetworkMetric>& metrics);
    bool insertBatch(const std::vector<NetworkEvent>& events);

    // Data querying
    std::vector<NetworkMetric> queryMetrics(const AnalyticsQuery& query);
    std::vector<NetworkEvent> queryEvents(const EventQuery& query);
    NetworkHealth getNetworkHealth();
    std::vector<NetworkHealth> getHealthHistory(const std::chrono::system_clock::time_point& start,
                                                const std::chrono::system_clock::time_point& end);

    // Aggregation queries
    double getAverageMetric(const std::string& node_id, MetricType type, 
                           const std::chrono::system_clock::time_point& start,
                           const std::chrono::system_clock::time_point& end);
    double getMaxMetric(const std::string& node_id, MetricType type,
                       const std::chrono::system_clock::time_point& start,
                       const std::chrono::system_clock::time_point& end);
    double getMinMetric(const std::string& node_id, MetricType type,
                       const std::chrono::system_clock::time_point& start,
                       const std::chrono::system_clock::time_point& end);

    // Real-time analytics
    std::vector<NetworkMetric> getLatestMetrics(const std::string& node_id, int count = 100);
    std::vector<NetworkEvent> getLatestEvents(const std::string& node_id, int count = 100);
    std::map<std::string, double> getNodeMetrics(const std::string& node_id);

    // Health monitoring
    bool isNodeHealthy(const std::string& node_id);
    std::vector<std::string> getUnhealthyNodes();
    std::vector<std::string> getNodesWithHighLatency(double threshold_ms = 100.0);
    std::vector<std::string> getNodesWithHighPacketLoss(double threshold_percent = 5.0);

    // Performance monitoring
    uint64_t getQueryCount() const { return query_count_; }
    uint64_t getInsertCount() const { return insert_count_; }
    double getAverageQueryTime() const { return average_query_time_; }
    double getAverageInsertTime() const { return average_insert_time_; }

private:
    std::string host_;
    uint16_t port_;
    std::string database_;
    std::string username_;
    std::string password_;
    bool connected_;
    CURL* curl_;
    
    // Performance counters
    uint64_t query_count_;
    uint64_t insert_count_;
    double average_query_time_;
    double average_insert_time_;
    
    // Internal methods
    std::string executeQuery(const std::string& query);
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
    
    // Parsing methods
    std::vector<NetworkMetric> parseMetrics(const std::string& response);
    std::vector<NetworkEvent> parseEvents(const std::string& response);
    NetworkHealth parseHealth(const std::string& response);
    
    // Conversion methods
    std::string metricTypeToString(MetricType type);
    std::string eventTypeToString(EventType type);
    std::string severityToString(Severity severity);
    MetricType stringToMetricType(const std::string& str);
    EventType stringToEventType(const std::string& str);
    Severity stringToSeverity(const std::string& str);
    
    // Utility methods
    std::string escapeString(const std::string& str);
    std::string formatTimestamp(const std::chrono::system_clock::time_point& timestamp);
    void updatePerformanceCounters(bool is_query, double execution_time);
};

// Analytics manager for high-level operations
class AnalyticsManager {
public:
    AnalyticsManager();
    ~AnalyticsManager();

    // Initialization
    bool initialize(const std::string& config_file);
    void shutdown();

    // Metric collection
    void collectMetric(const std::string& node_id, MetricType type, double value, 
                      const std::map<std::string, std::string>& tags = {});
    void collectEvent(EventType type, const std::string& node_id, const std::string& description,
                     Severity severity, const std::map<std::string, std::string>& metadata = {});
    void updateHealth(const NetworkHealth& health);

    // Real-time processing
    void startRealTimeProcessing();
    void stopRealTimeProcessing();
    bool isRealTimeProcessing() const { return real_time_processing_; }

    // Querying
    std::vector<NetworkMetric> queryMetrics(const AnalyticsQuery& query);
    std::vector<NetworkEvent> queryEvents(const EventQuery& query);
    NetworkHealth getCurrentHealth();

    // Alerting
    void registerAlertHandler(std::function<void(const NetworkEvent&)> handler);
    void checkAlerts();

    // Configuration
    bool loadConfiguration(const std::string& config_file);
    bool saveConfiguration(const std::string& config_file);

private:
    std::unique_ptr<ClickHouseClient> client_;
    bool initialized_;
    bool real_time_processing_;
    std::thread processing_thread_;
    std::vector<std::function<void(const NetworkEvent&)>> alert_handlers_;
    
    // Internal methods
    void processRealTimeData();
    void checkMetricAlerts(const NetworkMetric& metric);
    void checkHealthAlerts(const NetworkHealth& health);
};

} // namespace RouterSim
