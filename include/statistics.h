#pragma once

#include <string>
#include <map>
#include <vector>
#include <mutex>
#include <chrono>
#include <atomic>
#include <thread>

namespace RouterSim {

// Forward declarations
enum class StatCategory;

// Statistics counter
class StatCounter {
public:
    StatCounter(const std::string& name, const std::string& description, StatCategory category);
    ~StatCounter() = default;
    
    void increment(uint64_t value = 1);
    void decrement(uint64_t value = 1);
    void set_value(uint64_t value);
    uint64_t get_value() const;
    void reset();
    
    std::string get_name() const { return name_; }
    std::string get_description() const { return description_; }
    StatCategory get_category() const { return category_; }
    std::chrono::steady_clock::time_point get_last_update() const { return last_update_; }
    
private:
    std::string name_;
    std::string description_;
    StatCategory category_;
    std::atomic<uint64_t> value_;
    std::chrono::steady_clock::time_point last_update_;
};

// Statistics gauge
class StatGauge {
public:
    StatGauge(const std::string& name, const std::string& description, StatCategory category);
    ~StatGauge() = default;
    
    void set_value(uint64_t value);
    uint64_t get_value() const;
    void reset();
    
    std::string get_name() const { return name_; }
    std::string get_description() const { return description_; }
    StatCategory get_category() const { return category_; }
    std::chrono::steady_clock::time_point get_last_update() const { return last_update_; }
    
private:
    std::string name_;
    std::string description_;
    StatCategory category_;
    std::atomic<uint64_t> value_;
    std::chrono::steady_clock::time_point last_update_;
};

// Statistics histogram
class StatHistogram {
public:
    StatHistogram(const std::string& name, const std::string& description, StatCategory category,
                  const std::vector<double>& buckets);
    ~StatHistogram() = default;
    
    void observe(double value);
    void reset();
    
    std::string get_name() const { return name_; }
    std::string get_description() const { return description_; }
    StatCategory get_category() const { return category_; }
    
    uint64_t get_count() const;
    double get_sum() const;
    std::vector<uint64_t> get_bucket_counts() const;
    std::vector<double> get_buckets() const;
    double get_percentile(double percentile) const;
    
private:
    std::string name_;
    std::string description_;
    StatCategory category_;
    std::vector<double> buckets_;
    std::atomic<uint64_t> count_;
    std::atomic<double> sum_;
    std::vector<std::atomic<uint64_t>> bucket_counts_;
    mutable std::mutex mutex_;
};

// Statistics summary
class StatSummary {
public:
    StatSummary(const std::string& name, const std::string& description, StatCategory category);
    ~StatSummary() = default;
    
    void observe(double value);
    void reset();
    
    std::string get_name() const { return name_; }
    std::string get_description() const { return description_; }
    StatCategory get_category() const { return category_; }
    
    uint64_t get_count() const;
    double get_sum() const;
    double get_min() const;
    double get_max() const;
    double get_mean() const;
    double get_std_dev() const;
    double get_percentile(double percentile) const;
    
private:
    std::string name_;
    std::string description_;
    StatCategory category_;
    std::atomic<uint64_t> count_;
    std::atomic<double> sum_;
    std::atomic<double> min_;
    std::atomic<double> max_;
    std::vector<double> values_;
    mutable std::mutex mutex_;
};

// Main statistics collector
class Statistics {
public:
    Statistics();
    ~Statistics();
    
    // Counter management
    bool register_counter(const std::string& name, const std::string& description, StatCategory category);
    bool unregister_counter(const std::string& name);
    StatCounter* get_counter(const std::string& name) const;
    void increment_counter(const std::string& name, uint64_t value = 1);
    void decrement_counter(const std::string& name, uint64_t value = 1);
    void set_counter_value(const std::string& name, uint64_t value);
    uint64_t get_counter_value(const std::string& name) const;
    
    // Gauge management
    bool register_gauge(const std::string& name, const std::string& description, StatCategory category);
    bool unregister_gauge(const std::string& name);
    StatGauge* get_gauge(const std::string& name) const;
    void set_gauge_value(const std::string& name, uint64_t value);
    uint64_t get_gauge_value(const std::string& name) const;
    
    // Histogram management
    bool register_histogram(const std::string& name, const std::string& description, StatCategory category,
                           const std::vector<double>& buckets);
    bool unregister_histogram(const std::string& name);
    StatHistogram* get_histogram(const std::string& name) const;
    void observe_histogram(const std::string& name, double value);
    
    // Summary management
    bool register_summary(const std::string& name, const std::string& description, StatCategory category);
    bool unregister_summary(const std::string& name);
    StatSummary* get_summary(const std::string& name) const;
    void observe_summary(const std::string& name, double value);
    
    // Statistics retrieval
    std::map<std::string, uint64_t> get_all_counters() const;
    std::map<std::string, uint64_t> get_all_gauges() const;
    std::map<std::string, std::vector<uint64_t>> get_all_histograms() const;
    std::map<std::string, std::map<std::string, double>> get_all_summaries() const;
    
    // Category-based retrieval
    std::map<std::string, uint64_t> get_counters_by_category(StatCategory category) const;
    std::map<std::string, uint64_t> get_gauges_by_category(StatCategory category) const;
    std::map<std::string, std::vector<uint64_t>> get_histograms_by_category(StatCategory category) const;
    std::map<std::string, std::map<std::string, double>> get_summaries_by_category(StatCategory category) const;
    
    // Statistics export
    std::string export_to_json() const;
    std::string export_to_prometheus() const;
    std::string export_to_csv() const;
    bool export_to_file(const std::string& filename, const std::string& format = "json") const;
    
    // Statistics import
    bool import_from_json(const std::string& json_data);
    bool import_from_file(const std::string& filename);
    
    // Statistics management
    void reset_all();
    void reset_by_category(StatCategory category);
    void reset_by_name(const std::string& name);
    
    // Monitoring
    void start_monitoring();
    void stop_monitoring();
    bool is_monitoring() const;
    
    // Statistics snapshot
    struct StatisticsSnapshot {
        std::map<std::string, uint64_t> counters;
        std::map<std::string, uint64_t> gauges;
        std::map<std::string, std::vector<uint64_t>> histograms;
        std::map<std::string, std::map<std::string, double>> summaries;
        std::chrono::steady_clock::time_point timestamp;
    };
    
    StatisticsSnapshot get_snapshot() const;
    std::vector<StatisticsSnapshot> get_snapshots(uint32_t count) const;
    
private:
    std::map<std::string, std::unique_ptr<StatCounter>> counters_;
    std::map<std::string, std::unique_ptr<StatGauge>> gauges_;
    std::map<std::string, std::unique_ptr<StatHistogram>> histograms_;
    std::map<std::string, std::unique_ptr<StatSummary>> summaries_;
    
    std::atomic<bool> monitoring_;
    std::thread monitoring_thread_;
    std::vector<StatisticsSnapshot> snapshots_;
    mutable std::mutex counters_mutex_;
    mutable std::mutex gauges_mutex_;
    mutable std::mutex histograms_mutex_;
    mutable std::mutex summaries_mutex_;
    mutable std::mutex snapshots_mutex_;
    
    // Internal methods
    void monitoring_loop();
    void take_snapshot();
    std::string serialize_counter(const StatCounter& counter) const;
    std::string serialize_gauge(const StatGauge& gauge) const;
    std::string serialize_histogram(const StatHistogram& histogram) const;
    std::string serialize_summary(const StatSummary& summary) const;
};

} // namespace RouterSim
