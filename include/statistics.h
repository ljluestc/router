#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include <chrono>
#include <atomic>

namespace RouterSim {

// Statistics categories
enum class StatCategory {
    INTERFACE,
    PROTOCOL,
    TRAFFIC_SHAPING,
    NETEM_IMPAIRMENTS,
    PACKET_PROCESSING,
    ROUTING,
    SYSTEM
};

// Statistics data types
enum class StatType {
    COUNTER,
    GAUGE,
    HISTOGRAM,
    RATE
};

// Individual statistic
struct Statistic {
    std::string name;
    std::string description;
    StatType type;
    StatCategory category;
    uint64_t value;
    std::chrono::steady_clock::time_point timestamp;
    std::map<std::string, std::string> tags;
};

// Statistics collector class
class Statistics {
public:
    Statistics();
    ~Statistics() = default;

    // Counter operations
    void increment_counter(const std::string& name, uint64_t value = 1);
    void decrement_counter(const std::string& name, uint64_t value = 1);
    void set_counter(const std::string& name, uint64_t value);

    // Gauge operations
    void set_gauge(const std::string& name, uint64_t value);
    void increment_gauge(const std::string& name, uint64_t value = 1);
    void decrement_gauge(const std::string& name, uint64_t value = 1);

    // Histogram operations
    void record_histogram(const std::string& name, uint64_t value);
    void record_histogram_bucket(const std::string& name, uint64_t value, uint64_t count);

    // Rate operations
    void record_rate(const std::string& name, uint64_t value);
    void record_rate_per_second(const std::string& name, uint64_t value);

    // Statistic queries
    uint64_t get_counter(const std::string& name) const;
    uint64_t get_gauge(const std::string& name) const;
    std::map<uint64_t, uint64_t> get_histogram(const std::string& name) const;
    double get_rate(const std::string& name) const;

    // Bulk operations
    std::map<std::string, uint64_t> get_all_counters() const;
    std::map<std::string, uint64_t> get_all_gauges() const;
    std::map<std::string, std::map<uint64_t, uint64_t>> get_all_histograms() const;
    std::map<std::string, double> get_all_rates() const;

    // Category filtering
    std::map<std::string, uint64_t> get_counters_by_category(StatCategory category) const;
    std::map<std::string, uint64_t> get_gauges_by_category(StatCategory category) const;

    // Statistics management
    void reset_statistics();
    void reset_statistics_by_category(StatCategory category);
    void reset_statistic(const std::string& name);

    // Statistics registration
    void register_counter(const std::string& name, const std::string& description, 
                         StatCategory category, const std::map<std::string, std::string>& tags = {});
    void register_gauge(const std::string& name, const std::string& description,
                       StatCategory category, const std::map<std::string, std::string>& tags = {});
    void register_histogram(const std::string& name, const std::string& description,
                           StatCategory category, const std::map<std::string, std::string>& tags = {});
    void register_rate(const std::string& name, const std::string& description,
                      StatCategory category, const std::map<std::string, std::string>& tags = {});

    // Statistics export
    std::string export_json() const;
    std::string export_prometheus() const;
    std::string export_csv() const;
    bool export_to_file(const std::string& filename, const std::string& format = "json") const;

    // Statistics import
    bool import_from_file(const std::string& filename, const std::string& format = "json");
    bool import_json(const std::string& json_data);
    bool import_prometheus(const std::string& prometheus_data);

    // Statistics aggregation
    std::map<std::string, uint64_t> aggregate_counters(const std::vector<std::string>& names) const;
    double calculate_average_rate(const std::string& name, uint32_t window_seconds = 60) const;
    uint64_t calculate_total(const std::vector<std::string>& names) const;

    // Statistics validation
    bool is_valid_statistic(const std::string& name) const;
    bool is_counter(const std::string& name) const;
    bool is_gauge(const std::string& name) const;
    bool is_histogram(const std::string& name) const;
    bool is_rate(const std::string& name) const;

private:
    // Internal storage
    std::map<std::string, Statistic> statistics_;
    mutable std::mutex statistics_mutex_;

    // Rate calculation
    struct RateData {
        std::vector<std::pair<std::chrono::steady_clock::time_point, uint64_t>> samples;
        std::chrono::steady_clock::time_point last_calculation;
        double current_rate;
    };
    std::map<std::string, RateData> rate_data_;

    // Helper methods
    void update_timestamp(const std::string& name);
    double calculate_rate(const std::string& name) const;
    void cleanup_old_rate_samples(const std::string& name);
};

// Statistics utilities
class StatisticsUtils {
public:
    // Formatting
    static std::string format_number(uint64_t value);
    static std::string format_bytes(uint64_t bytes);
    static std::string format_rate(double rate);
    static std::string format_percentage(double percentage);

    // Conversion
    static std::string category_to_string(StatCategory category);
    static std::string type_to_string(StatType type);
    static StatCategory string_to_category(const std::string& str);
    static StatType string_to_type(const std::string& str);

    // Validation
    static bool is_valid_category(const std::string& category);
    static bool is_valid_type(const std::string& type);
    static bool is_valid_statistic_name(const std::string& name);

    // Aggregation
    static uint64_t sum_counters(const std::map<std::string, uint64_t>& counters);
    static double average_gauges(const std::map<std::string, uint64_t>& gauges);
    static uint64_t max_value(const std::map<std::string, uint64_t>& values);
    static uint64_t min_value(const std::map<std::string, uint64_t>& values);
};

// Statistics monitor for real-time monitoring
class StatisticsMonitor {
public:
    StatisticsMonitor(std::shared_ptr<Statistics> stats);
    ~StatisticsMonitor();

    // Monitoring control
    void start_monitoring(uint32_t interval_ms = 1000);
    void stop_monitoring();
    bool is_monitoring() const;

    // Alert configuration
    void set_counter_alert(const std::string& name, uint64_t threshold, 
                          std::function<void(const std::string&, uint64_t)> callback);
    void set_gauge_alert(const std::string& name, uint64_t threshold,
                        std::function<void(const std::string&, uint64_t)> callback);
    void set_rate_alert(const std::string& name, double threshold,
                       std::function<void(const std::string&, double)> callback);

    // Monitoring callbacks
    void register_monitoring_callback(std::function<void(const std::map<std::string, uint64_t>&)> callback);

private:
    std::shared_ptr<Statistics> statistics_;
    std::atomic<bool> monitoring_;
    std::thread monitoring_thread_;
    uint32_t interval_ms_;

    // Alert configuration
    std::map<std::string, std::pair<uint64_t, std::function<void(const std::string&, uint64_t)>>> counter_alerts_;
    std::map<std::string, std::pair<uint64_t, std::function<void(const std::string&, uint64_t)>>> gauge_alerts_;
    std::map<std::string, std::pair<double, std::function<void(const std::string&, double)>>> rate_alerts_;

    // Monitoring callbacks
    std::function<void(const std::map<std::string, uint64_t>&)> monitoring_callback_;

    void monitoring_loop();
    void check_alerts();
};

} // namespace RouterSim
