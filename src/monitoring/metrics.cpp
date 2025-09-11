#include "monitoring/metrics.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <thread>

namespace RouterSim {

MonitoringSystem::MonitoringSystem() : running_(false) {
    statistics_.metrics_collected = 0;
    statistics_.alerts_fired = 0;
    statistics_.alerts_resolved = 0;
}

MonitoringSystem::~MonitoringSystem() {
    stop();
}

void MonitoringSystem::register_collector(std::shared_ptr<MetricsCollector> collector) {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    collectors_.push_back(collector);
}

void MonitoringSystem::unregister_collector(std::shared_ptr<MetricsCollector> collector) {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    auto it = std::find(collectors_.begin(), collectors_.end(), collector);
    if (it != collectors_.end()) {
        collectors_.erase(it);
    }
}

void MonitoringSystem::increment_counter(const std::string& name, const std::map<std::string, std::string>& labels) {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    
    std::string key = generate_metric_key(name, labels);
    auto it = counters_.find(key);
    
    if (it != counters_.end()) {
        it->second.value += 1.0;
        it->second.timestamp = std::chrono::steady_clock::now();
    } else {
        MetricValue metric;
        metric.name = name;
        metric.help = "Counter metric";
        metric.type = MetricType::COUNTER;
        metric.labels = labels;
        metric.value = 1.0;
        metric.timestamp = std::chrono::steady_clock::now();
        counters_[key] = metric;
    }
    
    statistics_.metrics_collected++;
}

void MonitoringSystem::set_gauge(const std::string& name, double value, const std::map<std::string, std::string>& labels) {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    
    std::string key = generate_metric_key(name, labels);
    auto it = gauges_.find(key);
    
    if (it != gauges_.end()) {
        it->second.value = value;
        it->second.timestamp = std::chrono::steady_clock::now();
    } else {
        MetricValue metric;
        metric.name = name;
        metric.help = "Gauge metric";
        metric.type = MetricType::GAUGE;
        metric.labels = labels;
        metric.value = value;
        metric.timestamp = std::chrono::steady_clock::now();
        gauges_[key] = metric;
    }
    
    statistics_.metrics_collected++;
}

void MonitoringSystem::observe_histogram(const std::string& name, double value, const std::map<std::string, std::string>& labels) {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    
    std::string key = generate_metric_key(name, labels);
    auto it = histograms_.find(key);
    
    if (it != histograms_.end()) {
        it->second.count++;
        it->second.sum += value;
        it->second.timestamp = std::chrono::steady_clock::now();
        
        // Update buckets
        for (auto& bucket : it->second.buckets) {
            if (value <= bucket.upper_bound) {
                bucket.count++;
                break;
            }
        }
    } else {
        HistogramMetric metric;
        metric.name = name;
        metric.help = "Histogram metric";
        metric.labels = labels;
        metric.count = 1;
        metric.sum = value;
        metric.timestamp = std::chrono::steady_clock::now();
        
        // Default buckets
        metric.buckets = {
            {0.005, 0},
            {0.01, 0},
            {0.025, 0},
            {0.05, 0},
            {0.075, 0},
            {0.1, 0},
            {0.25, 0},
            {0.5, 0},
            {0.75, 0},
            {1.0, 0},
            {2.5, 0},
            {5.0, 0},
            {7.5, 0},
            {10.0, 0},
            {std::numeric_limits<double>::infinity(), 0}
        };
        
        // Update buckets
        for (auto& bucket : metric.buckets) {
            if (value <= bucket.upper_bound) {
                bucket.count++;
                break;
            }
        }
        
        histograms_[key] = metric;
    }
    
    statistics_.metrics_collected++;
}

void MonitoringSystem::observe_summary(const std::string& name, double value, const std::map<std::string, std::string>& labels) {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    
    std::string key = generate_metric_key(name, labels);
    auto it = summaries_.find(key);
    
    if (it != summaries_.end()) {
        it->second.count++;
        it->second.sum += value;
        it->second.timestamp = std::chrono::steady_clock::now();
        
        // Update quantiles (simplified implementation)
        for (auto& quantile : it->second.quantiles) {
            // This is a simplified implementation
            // In a real system, you would use a more sophisticated algorithm
            quantile.value = value;
        }
    } else {
        SummaryMetric metric;
        metric.name = name;
        metric.help = "Summary metric";
        metric.labels = labels;
        metric.count = 1;
        metric.sum = value;
        metric.timestamp = std::chrono::steady_clock::now();
        
        // Default quantiles
        metric.quantiles = {
            {0.5, value},
            {0.9, value},
            {0.95, value},
            {0.99, value}
        };
        
        summaries_[key] = metric;
    }
    
    statistics_.metrics_collected++;
}

std::vector<MetricValue> MonitoringSystem::get_metrics() const {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    
    std::vector<MetricValue> metrics;
    
    // Add counters
    for (const auto& [key, metric] : counters_) {
        metrics.push_back(metric);
    }
    
    // Add gauges
    for (const auto& [key, metric] : gauges_) {
        metrics.push_back(metric);
    }
    
    return metrics;
}

std::vector<HistogramMetric> MonitoringSystem::get_histograms() const {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    
    std::vector<HistogramMetric> histograms;
    for (const auto& [key, metric] : histograms_) {
        histograms.push_back(metric);
    }
    
    return histograms;
}

std::vector<SummaryMetric> MonitoringSystem::get_summaries() const {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    
    std::vector<SummaryMetric> summaries;
    for (const auto& [key, metric] : summaries_) {
        summaries.push_back(metric);
    }
    
    return summaries;
}

void MonitoringSystem::add_alert_rule(const AlertRule& rule) {
    std::lock_guard<std::mutex> lock(alerts_mutex_);
    alert_rules_[rule.name] = rule;
}

void MonitoringSystem::remove_alert_rule(const std::string& name) {
    std::lock_guard<std::mutex> lock(alerts_mutex_);
    alert_rules_.erase(name);
}

void MonitoringSystem::evaluate_alerts() {
    std::lock_guard<std::mutex> lock(alerts_mutex_);
    
    for (const auto& [name, rule] : alert_rules_) {
        if (rule.enabled) {
            evaluate_alert_rule(rule);
        }
    }
}

std::vector<Alert> MonitoringSystem::get_active_alerts() const {
    std::lock_guard<std::mutex> lock(alerts_mutex_);
    
    std::vector<Alert> alerts;
    for (const auto& [name, alert] : active_alerts_) {
        if (alert.state == "firing") {
            alerts.push_back(alert);
        }
    }
    
    return alerts;
}

bool MonitoringSystem::start() {
    if (running_.load()) {
        return true;
    }
    
    running_.store(true);
    
    // Start metrics collection thread
    std::thread collection_thread(&MonitoringSystem::collect_metrics_loop, this);
    collection_thread.detach();
    
    return true;
}

bool MonitoringSystem::stop() {
    if (!running_.load()) {
        return true;
    }
    
    running_.store(false);
    return true;
}

bool MonitoringSystem::is_running() const {
    return running_.load();
}

MonitoringSystem::Statistics MonitoringSystem::get_statistics() const {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    return statistics_;
}

void MonitoringSystem::collect_metrics_loop() {
    while (running_.load()) {
        std::vector<MetricValue> collected_metrics;
        
        // Collect metrics from all registered collectors
        {
            std::lock_guard<std::mutex> lock(metrics_mutex_);
            for (auto& collector : collectors_) {
                collector->collect_metrics(collected_metrics);
            }
        }
        
        // Process collected metrics
        for (const auto& metric : collected_metrics) {
            if (metric.type == MetricType::COUNTER) {
                increment_counter(metric.name, metric.labels);
            } else if (metric.type == MetricType::GAUGE) {
                set_gauge(metric.name, metric.value, metric.labels);
            }
        }
        
        // Evaluate alerts
        evaluate_alerts();
        
        // Update statistics
        statistics_.last_collection = std::chrono::steady_clock::now();
        
        // Sleep for collection interval
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
}

void MonitoringSystem::evaluate_alert_rule(const AlertRule& rule) {
    // Simplified alert evaluation
    // In a real system, this would parse the expression and evaluate it against metrics
    
    // Check if alert is already firing
    auto it = active_alerts_.find(rule.name);
    if (it != active_alerts_.end()) {
        // Alert is already active, check if it should be resolved
        // This is a simplified implementation
        if (rule.expression == "resolved") {
            it->second.state = "resolved";
            it->second.ends_at = std::chrono::steady_clock::now();
            statistics_.alerts_resolved++;
        }
    } else {
        // Check if alert should fire
        if (rule.expression == "firing") {
            Alert alert;
            alert.name = rule.name;
            alert.state = "firing";
            alert.severity = rule.severity;
            alert.description = rule.description;
            alert.summary = rule.summary;
            alert.starts_at = std::chrono::steady_clock::now();
            
            active_alerts_[rule.name] = alert;
            statistics_.alerts_fired++;
        }
    }
}

std::string MonitoringSystem::generate_metric_key(const std::string& name, const std::map<std::string, std::string>& labels) const {
    std::stringstream ss;
    ss << name;
    
    for (const auto& [key, value] : labels) {
        ss << "{" << key << "=" << value << "}";
    }
    
    return ss.str();
}

} // namespace RouterSim
