#pragma once

#include <string>
#include <map>
#include <vector>
#include <chrono>
#include <atomic>
#include <mutex>
#include <functional>

namespace RouterSim {

// Metric types
enum class MetricType {
    COUNTER,
    GAUGE,
    HISTOGRAM,
    SUMMARY
};

// Metric value
struct MetricValue {
    std::string name;
    std::string help;
    MetricType type;
    std::map<std::string, std::string> labels;
    double value;
    std::chrono::steady_clock::time_point timestamp;
};

// Histogram bucket
struct HistogramBucket {
    double upper_bound;
    uint64_t count;
};

// Histogram metric
struct HistogramMetric {
    std::string name;
    std::string help;
    std::map<std::string, std::string> labels;
    std::vector<HistogramBucket> buckets;
    uint64_t count;
    double sum;
    std::chrono::steady_clock::time_point timestamp;
};

// Summary quantile
struct SummaryQuantile {
    double quantile;
    double value;
};

// Summary metric
struct SummaryMetric {
    std::string name;
    std::string help;
    std::map<std::string, std::string> labels;
    std::vector<SummaryQuantile> quantiles;
    uint64_t count;
    double sum;
    std::chrono::steady_clock::time_point timestamp;
};

// Alert rule
struct AlertRule {
    std::string name;
    std::string expression;
    std::string severity;
    std::string description;
    std::string summary;
    bool enabled;
    std::chrono::seconds duration;
};

// Alert
struct Alert {
    std::string name;
    std::string state; // "firing", "resolved"
    std::string severity;
    std::string description;
    std::string summary;
    std::map<std::string, std::string> labels;
    std::chrono::steady_clock::time_point starts_at;
    std::chrono::steady_clock::time_point ends_at;
};

// Metrics collector interface
class MetricsCollector {
public:
    virtual ~MetricsCollector() = default;
    virtual void collect_metrics(std::vector<MetricValue>& metrics) = 0;
};

// Alert manager interface
class AlertManager {
public:
    virtual ~AlertManager() = default;
    virtual void add_rule(const AlertRule& rule) = 0;
    virtual void remove_rule(const std::string& name) = 0;
    virtual void evaluate_alerts() = 0;
    virtual std::vector<Alert> get_active_alerts() const = 0;
};

// Monitoring system
class MonitoringSystem {
public:
    MonitoringSystem();
    ~MonitoringSystem();

    // Metrics management
    void register_collector(std::shared_ptr<MetricsCollector> collector);
    void unregister_collector(std::shared_ptr<MetricsCollector> collector);
    
    // Metric operations
    void increment_counter(const std::string& name, const std::map<std::string, std::string>& labels = {});
    void set_gauge(const std::string& name, double value, const std::map<std::string, std::string>& labels = {});
    void observe_histogram(const std::string& name, double value, const std::map<std::string, std::string>& labels = {});
    void observe_summary(const std::string& name, double value, const std::map<std::string, std::string>& labels = {});
    
    // Metric retrieval
    std::vector<MetricValue> get_metrics() const;
    std::vector<HistogramMetric> get_histograms() const;
    std::vector<SummaryMetric> get_summaries() const;
    
    // Alert management
    void add_alert_rule(const AlertRule& rule);
    void remove_alert_rule(const std::string& name);
    void evaluate_alerts();
    std::vector<Alert> get_active_alerts() const;
    
    // System management
    bool start();
    bool stop();
    bool is_running() const;
    
    // Statistics
    struct Statistics {
        uint64_t metrics_collected;
        uint64_t alerts_fired;
        uint64_t alerts_resolved;
        std::chrono::steady_clock::time_point last_collection;
    };
    
    Statistics get_statistics() const;

private:
    // Internal state
    std::atomic<bool> running_;
    mutable std::mutex metrics_mutex_;
    mutable std::mutex alerts_mutex_;
    
    // Metrics storage
    std::map<std::string, MetricValue> counters_;
    std::map<std::string, MetricValue> gauges_;
    std::map<std::string, HistogramMetric> histograms_;
    std::map<std::string, SummaryMetric> summaries_;
    
    // Collectors
    std::vector<std::shared_ptr<MetricsCollector>> collectors_;
    
    // Alert management
    std::map<std::string, AlertRule> alert_rules_;
    std::map<std::string, Alert> active_alerts_;
    
    // Statistics
    Statistics statistics_;
    
    // Internal methods
    void collect_metrics_loop();
    void evaluate_alert_rule(const AlertRule& rule);
    std::string generate_metric_key(const std::string& name, const std::map<std::string, std::string>& labels) const;
};

} // namespace RouterSim
