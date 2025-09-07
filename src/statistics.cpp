#include "statistics.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>

using namespace RouterSim;

Statistics::Statistics() {
}

Statistics::~Statistics() {
}

void Statistics::increment_counter(const std::string& name, uint64_t value) {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    statistics_[name].value += value;
    update_timestamp(name);
}

void Statistics::decrement_counter(const std::string& name, uint64_t value) {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    statistics_[name].value -= value;
    update_timestamp(name);
}

void Statistics::set_counter(const std::string& name, uint64_t value) {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    statistics_[name].value = value;
    update_timestamp(name);
}

void Statistics::set_gauge(const std::string& name, uint64_t value) {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    statistics_[name].value = value;
    update_timestamp(name);
}

void Statistics::increment_gauge(const std::string& name, uint64_t value) {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    statistics_[name].value += value;
    update_timestamp(name);
}

void Statistics::decrement_gauge(const std::string& name, uint64_t value) {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    statistics_[name].value -= value;
    update_timestamp(name);
}

void Statistics::record_histogram(const std::string& name, uint64_t value) {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    statistics_[name].value = value; // Simplified - in real implementation, this would store histogram data
    update_timestamp(name);
}

void Statistics::record_histogram_bucket(const std::string& name, uint64_t value, uint64_t count) {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    statistics_[name].value = count; // Simplified - in real implementation, this would store bucket data
    update_timestamp(name);
}

void Statistics::record_rate(const std::string& name, uint64_t value) {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    
    auto now = std::chrono::steady_clock::now();
    rate_data_[name].samples.push_back({now, value});
    
    // Clean up old samples
    cleanup_old_rate_samples(name);
    
    // Calculate current rate
    rate_data_[name].current_rate = calculate_rate(name);
    rate_data_[name].last_calculation = now;
    
    update_timestamp(name);
}

void Statistics::record_rate_per_second(const std::string& name, uint64_t value) {
    record_rate(name, value);
}

uint64_t Statistics::get_counter(const std::string& name) const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    
    auto it = statistics_.find(name);
    if (it == statistics_.end()) {
        return 0;
    }
    
    return it->second.value;
}

uint64_t Statistics::get_gauge(const std::string& name) const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    
    auto it = statistics_.find(name);
    if (it == statistics_.end()) {
        return 0;
    }
    
    return it->second.value;
}

std::map<uint64_t, uint64_t> Statistics::get_histogram(const std::string& name) const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    
    // Simplified implementation - in real code, this would return histogram data
    std::map<uint64_t, uint64_t> histogram;
    return histogram;
}

double Statistics::get_rate(const std::string& name) const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    
    auto it = rate_data_.find(name);
    if (it == rate_data_.end()) {
        return 0.0;
    }
    
    return it->second.current_rate;
}

std::map<std::string, uint64_t> Statistics::get_all_counters() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    
    std::map<std::string, uint64_t> counters;
    for (const auto& pair : statistics_) {
        if (pair.second.type == StatType::COUNTER) {
            counters[pair.first] = pair.second.value;
        }
    }
    
    return counters;
}

std::map<std::string, uint64_t> Statistics::get_all_gauges() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    
    std::map<std::string, uint64_t> gauges;
    for (const auto& pair : statistics_) {
        if (pair.second.type == StatType::GAUGE) {
            gauges[pair.first] = pair.second.value;
        }
    }
    
    return gauges;
}

std::map<std::string, std::map<uint64_t, uint64_t>> Statistics::get_all_histograms() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    
    std::map<std::string, std::map<uint64_t, uint64_t>> histograms;
    for (const auto& pair : statistics_) {
        if (pair.second.type == StatType::HISTOGRAM) {
            histograms[pair.first] = get_histogram(pair.first);
        }
    }
    
    return histograms;
}

std::map<std::string, double> Statistics::get_all_rates() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    
    std::map<std::string, double> rates;
    for (const auto& pair : rate_data_) {
        rates[pair.first] = pair.second.current_rate;
    }
    
    return rates;
}

std::map<std::string, uint64_t> Statistics::get_counters_by_category(StatCategory category) const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    
    std::map<std::string, uint64_t> counters;
    for (const auto& pair : statistics_) {
        if (pair.second.type == StatType::COUNTER && pair.second.category == category) {
            counters[pair.first] = pair.second.value;
        }
    }
    
    return counters;
}

std::map<std::string, uint64_t> Statistics::get_gauges_by_category(StatCategory category) const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    
    std::map<std::string, uint64_t> gauges;
    for (const auto& pair : statistics_) {
        if (pair.second.type == StatType::GAUGE && pair.second.category == category) {
            gauges[pair.first] = pair.second.value;
        }
    }
    
    return gauges;
}

void Statistics::reset_statistics() {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    statistics_.clear();
    rate_data_.clear();
}

void Statistics::reset_statistics_by_category(StatCategory category) {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    
    auto it = statistics_.begin();
    while (it != statistics_.end()) {
        if (it->second.category == category) {
            it = statistics_.erase(it);
        } else {
            ++it;
        }
    }
}

void Statistics::reset_statistic(const std::string& name) {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    statistics_.erase(name);
    rate_data_.erase(name);
}

void Statistics::register_counter(const std::string& name, const std::string& description, 
                                 StatCategory category, const std::map<std::string, std::string>& tags) {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    
    Statistic stat;
    stat.name = name;
    stat.description = description;
    stat.type = StatType::COUNTER;
    stat.category = category;
    stat.value = 0;
    stat.timestamp = std::chrono::steady_clock::now();
    stat.tags = tags;
    
    statistics_[name] = stat;
}

void Statistics::register_gauge(const std::string& name, const std::string& description,
                               StatCategory category, const std::map<std::string, std::string>& tags) {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    
    Statistic stat;
    stat.name = name;
    stat.description = description;
    stat.type = StatType::GAUGE;
    stat.category = category;
    stat.value = 0;
    stat.timestamp = std::chrono::steady_clock::now();
    stat.tags = tags;
    
    statistics_[name] = stat;
}

void Statistics::register_histogram(const std::string& name, const std::string& description,
                                   StatCategory category, const std::map<std::string, std::string>& tags) {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    
    Statistic stat;
    stat.name = name;
    stat.description = description;
    stat.type = StatType::HISTOGRAM;
    stat.category = category;
    stat.value = 0;
    stat.timestamp = std::chrono::steady_clock::now();
    stat.tags = tags;
    
    statistics_[name] = stat;
}

void Statistics::register_rate(const std::string& name, const std::string& description,
                              StatCategory category, const std::map<std::string, std::string>& tags) {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    
    Statistic stat;
    stat.name = name;
    stat.description = description;
    stat.type = StatType::RATE;
    stat.category = category;
    stat.value = 0;
    stat.timestamp = std::chrono::steady_clock::now();
    stat.tags = tags;
    
    statistics_[name] = stat;
    
    // Initialize rate data
    rate_data_[name] = RateData{};
}

std::string Statistics::export_json() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    
    std::stringstream ss;
    ss << "{\n";
    ss << "  \"timestamp\": \"" << std::chrono::duration_cast<std::chrono::milliseconds>(
           std::chrono::system_clock::now().time_since_epoch()).count() << "\",\n";
    ss << "  \"statistics\": {\n";
    
    bool first = true;
    for (const auto& pair : statistics_) {
        if (!first) ss << ",\n";
        ss << "    \"" << pair.first << "\": {\n";
        ss << "      \"value\": " << pair.second.value << ",\n";
        ss << "      \"type\": \"" << StatisticsUtils::type_to_string(pair.second.type) << "\",\n";
        ss << "      \"category\": \"" << StatisticsUtils::category_to_string(pair.second.category) << "\",\n";
        ss << "      \"description\": \"" << pair.second.description << "\"\n";
        ss << "    }";
        first = false;
    }
    
    ss << "\n  }\n";
    ss << "}\n";
    
    return ss.str();
}

std::string Statistics::export_prometheus() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    
    std::stringstream ss;
    
    for (const auto& pair : statistics_) {
        ss << "# HELP " << pair.first << " " << pair.second.description << "\n";
        ss << "# TYPE " << pair.first << " " << StatisticsUtils::type_to_string(pair.second.type) << "\n";
        ss << pair.first << " " << pair.second.value << "\n";
    }
    
    return ss.str();
}

std::string Statistics::export_csv() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    
    std::stringstream ss;
    ss << "name,value,type,category,description,timestamp\n";
    
    for (const auto& pair : statistics_) {
        ss << pair.first << "," << pair.second.value << ","
           << StatisticsUtils::type_to_string(pair.second.type) << ","
           << StatisticsUtils::category_to_string(pair.second.category) << ","
           << pair.second.description << ","
           << std::chrono::duration_cast<std::chrono::milliseconds>(
                  pair.second.timestamp.time_since_epoch()).count() << "\n";
    }
    
    return ss.str();
}

bool Statistics::export_to_file(const std::string& filename, const std::string& format) const {
    std::string data;
    
    if (format == "json") {
        data = export_json();
    } else if (format == "prometheus") {
        data = export_prometheus();
    } else if (format == "csv") {
        data = export_csv();
    } else {
        return false;
    }
    
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    file << data;
    return true;
}

bool Statistics::import_from_file(const std::string& filename, const std::string& format) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    std::string data((std::istreambuf_iterator<char>(file)),
                     std::istreambuf_iterator<char>());
    
    if (format == "json") {
        return import_json(data);
    } else if (format == "prometheus") {
        return import_prometheus(data);
    } else {
        return false;
    }
}

bool Statistics::import_json(const std::string& json_data) {
    // Simplified JSON import - in real implementation, use a JSON library
    return true;
}

bool Statistics::import_prometheus(const std::string& prometheus_data) {
    // Simplified Prometheus import - in real implementation, parse Prometheus format
    return true;
}

std::map<std::string, uint64_t> Statistics::aggregate_counters(const std::vector<std::string>& names) const {
    std::map<std::string, uint64_t> result;
    
    for (const auto& name : names) {
        result[name] = get_counter(name);
    }
    
    return result;
}

double Statistics::calculate_average_rate(const std::string& name, uint32_t window_seconds) const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    
    auto it = rate_data_.find(name);
    if (it == rate_data_.end()) {
        return 0.0;
    }
    
    auto now = std::chrono::steady_clock::now();
    auto cutoff = now - std::chrono::seconds(window_seconds);
    
    uint64_t total_value = 0;
    size_t count = 0;
    
    for (const auto& sample : it->second.samples) {
        if (sample.first >= cutoff) {
            total_value += sample.second;
            count++;
        }
    }
    
    if (count == 0) {
        return 0.0;
    }
    
    return static_cast<double>(total_value) / count;
}

uint64_t Statistics::calculate_total(const std::vector<std::string>& names) const {
    uint64_t total = 0;
    
    for (const auto& name : names) {
        total += get_counter(name);
    }
    
    return total;
}

bool Statistics::is_valid_statistic(const std::string& name) const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    return statistics_.find(name) != statistics_.end();
}

bool Statistics::is_counter(const std::string& name) const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    
    auto it = statistics_.find(name);
    if (it == statistics_.end()) {
        return false;
    }
    
    return it->second.type == StatType::COUNTER;
}

bool Statistics::is_gauge(const std::string& name) const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    
    auto it = statistics_.find(name);
    if (it == statistics_.end()) {
        return false;
    }
    
    return it->second.type == StatType::GAUGE;
}

bool Statistics::is_histogram(const std::string& name) const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    
    auto it = statistics_.find(name);
    if (it == statistics_.end()) {
        return false;
    }
    
    return it->second.type == StatType::HISTOGRAM;
}

bool Statistics::is_rate(const std::string& name) const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    
    auto it = statistics_.find(name);
    if (it == statistics_.end()) {
        return false;
    }
    
    return it->second.type == StatType::RATE;
}

void Statistics::update_timestamp(const std::string& name) {
    auto it = statistics_.find(name);
    if (it != statistics_.end()) {
        it->second.timestamp = std::chrono::steady_clock::now();
    }
}

double Statistics::calculate_rate(const std::string& name) const {
    auto it = rate_data_.find(name);
    if (it == rate_data_.end()) {
        return 0.0;
    }
    
    const auto& samples = it->second.samples;
    if (samples.size() < 2) {
        return 0.0;
    }
    
    auto now = std::chrono::steady_clock::now();
    auto window_start = now - std::chrono::seconds(60); // 60-second window
    
    uint64_t total_value = 0;
    size_t count = 0;
    
    for (const auto& sample : samples) {
        if (sample.first >= window_start) {
            total_value += sample.second;
            count++;
        }
    }
    
    if (count == 0) {
        return 0.0;
    }
    
    return static_cast<double>(total_value) / 60.0; // Per second
}

void Statistics::cleanup_old_rate_samples(const std::string& name) {
    auto it = rate_data_.find(name);
    if (it == rate_data_.end()) {
        return;
    }
    
    auto now = std::chrono::steady_clock::now();
    auto cutoff = now - std::chrono::seconds(300); // Keep 5 minutes of data
    
    auto& samples = it->second.samples;
    samples.erase(
        std::remove_if(samples.begin(), samples.end(),
                      [cutoff](const std::pair<std::chrono::steady_clock::time_point, uint64_t>& sample) {
                          return sample.first < cutoff;
                      }),
        samples.end()
    );
}

// StatisticsUtils implementation
std::string StatisticsUtils::format_number(uint64_t value) {
    if (value >= 1000000000) {
        return std::to_string(value / 1000000000) + "B";
    } else if (value >= 1000000) {
        return std::to_string(value / 1000000) + "M";
    } else if (value >= 1000) {
        return std::to_string(value / 1000) + "K";
    } else {
        return std::to_string(value);
    }
}

std::string StatisticsUtils::format_bytes(uint64_t bytes) {
    if (bytes >= 1024 * 1024 * 1024) {
        return std::to_string(bytes / (1024 * 1024 * 1024)) + " GB";
    } else if (bytes >= 1024 * 1024) {
        return std::to_string(bytes / (1024 * 1024)) + " MB";
    } else if (bytes >= 1024) {
        return std::to_string(bytes / 1024) + " KB";
    } else {
        return std::to_string(bytes) + " B";
    }
}

std::string StatisticsUtils::format_rate(double rate) {
    if (rate >= 1000000) {
        return std::to_string(rate / 1000000) + " M/s";
    } else if (rate >= 1000) {
        return std::to_string(rate / 1000) + " K/s";
    } else {
        return std::to_string(rate) + " /s";
    }
}

std::string StatisticsUtils::format_percentage(double percentage) {
    return std::to_string(percentage) + "%";
}

std::string StatisticsUtils::category_to_string(StatCategory category) {
    switch (category) {
        case StatCategory::INTERFACE: return "interface";
        case StatCategory::PROTOCOL: return "protocol";
        case StatCategory::TRAFFIC_SHAPING: return "traffic_shaping";
        case StatCategory::NETEM_IMPAIRMENTS: return "netem_impairments";
        case StatCategory::PACKET_PROCESSING: return "packet_processing";
        case StatCategory::ROUTING: return "routing";
        case StatCategory::SYSTEM: return "system";
        default: return "unknown";
    }
}

std::string StatisticsUtils::type_to_string(StatType type) {
    switch (type) {
        case StatType::COUNTER: return "counter";
        case StatType::GAUGE: return "gauge";
        case StatType::HISTOGRAM: return "histogram";
        case StatType::RATE: return "rate";
        default: return "unknown";
    }
}

StatCategory StatisticsUtils::string_to_category(const std::string& str) {
    if (str == "interface") return StatCategory::INTERFACE;
    if (str == "protocol") return StatCategory::PROTOCOL;
    if (str == "traffic_shaping") return StatCategory::TRAFFIC_SHAPING;
    if (str == "netem_impairments") return StatCategory::NETEM_IMPAIRMENTS;
    if (str == "packet_processing") return StatCategory::PACKET_PROCESSING;
    if (str == "routing") return StatCategory::ROUTING;
    if (str == "system") return StatCategory::SYSTEM;
    return StatCategory::SYSTEM;
}

StatType StatisticsUtils::string_to_type(const std::string& str) {
    if (str == "counter") return StatType::COUNTER;
    if (str == "gauge") return StatType::GAUGE;
    if (str == "histogram") return StatType::HISTOGRAM;
    if (str == "rate") return StatType::RATE;
    return StatType::COUNTER;
}

bool StatisticsUtils::is_valid_category(const std::string& category) {
    return category == "interface" || category == "protocol" || category == "traffic_shaping" ||
           category == "netem_impairments" || category == "packet_processing" ||
           category == "routing" || category == "system";
}

bool StatisticsUtils::is_valid_type(const std::string& type) {
    return type == "counter" || type == "gauge" || type == "histogram" || type == "rate";
}

bool StatisticsUtils::is_valid_statistic_name(const std::string& name) {
    return !name.empty() && name.length() <= 64 && 
           std::all_of(name.begin(), name.end(), [](char c) {
               return std::isalnum(c) || c == '_' || c == '-';
           });
}

uint64_t StatisticsUtils::sum_counters(const std::map<std::string, uint64_t>& counters) {
    uint64_t sum = 0;
    for (const auto& pair : counters) {
        sum += pair.second;
    }
    return sum;
}

double StatisticsUtils::average_gauges(const std::map<std::string, uint64_t>& gauges) {
    if (gauges.empty()) {
        return 0.0;
    }
    
    uint64_t sum = 0;
    for (const auto& pair : gauges) {
        sum += pair.second;
    }
    
    return static_cast<double>(sum) / gauges.size();
}

uint64_t StatisticsUtils::max_value(const std::map<std::string, uint64_t>& values) {
    if (values.empty()) {
        return 0;
    }
    
    return std::max_element(values.begin(), values.end(),
                           [](const std::pair<std::string, uint64_t>& a,
                              const std::pair<std::string, uint64_t>& b) {
                               return a.second < b.second;
                           })->second;
}

uint64_t StatisticsUtils::min_value(const std::map<std::string, uint64_t>& values) {
    if (values.empty()) {
        return 0;
    }
    
    return std::min_element(values.begin(), values.end(),
                           [](const std::pair<std::string, uint64_t>& a,
                              const std::pair<std::string, uint64_t>& b) {
                               return a.second < b.second;
                           })->second;
}

// StatisticsMonitor implementation
StatisticsMonitor::StatisticsMonitor(std::shared_ptr<Statistics> stats) 
    : statistics_(stats), monitoring_(false), interval_ms_(1000) {
}

StatisticsMonitor::~StatisticsMonitor() {
    stop_monitoring();
}

void StatisticsMonitor::start_monitoring(uint32_t interval_ms) {
    if (monitoring_) {
        return;
    }
    
    monitoring_ = true;
    interval_ms_ = interval_ms;
    monitoring_thread_ = std::thread(&StatisticsMonitor::monitoring_loop, this);
}

void StatisticsMonitor::stop_monitoring() {
    if (!monitoring_) {
        return;
    }
    
    monitoring_ = false;
    
    if (monitoring_thread_.joinable()) {
        monitoring_thread_.join();
    }
}

bool StatisticsMonitor::is_monitoring() const {
    return monitoring_;
}

void StatisticsMonitor::set_counter_alert(const std::string& name, uint64_t threshold, 
                                        std::function<void(const std::string&, uint64_t)> callback) {
    counter_alerts_[name] = {threshold, callback};
}

void StatisticsMonitor::set_gauge_alert(const std::string& name, uint64_t threshold,
                                      std::function<void(const std::string&, uint64_t)> callback) {
    gauge_alerts_[name] = {threshold, callback};
}

void StatisticsMonitor::set_rate_alert(const std::string& name, double threshold,
                                     std::function<void(const std::string&, double)> callback) {
    rate_alerts_[name] = {threshold, callback};
}

void StatisticsMonitor::register_monitoring_callback(std::function<void(const std::map<std::string, uint64_t>&)> callback) {
    monitoring_callback_ = callback;
}

void StatisticsMonitor::monitoring_loop() {
    while (monitoring_) {
        check_alerts();
        
        if (monitoring_callback_) {
            auto counters = statistics_->get_all_counters();
            monitoring_callback_(counters);
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms_));
    }
}

void StatisticsMonitor::check_alerts() {
    // Check counter alerts
    for (const auto& alert : counter_alerts_) {
        uint64_t value = statistics_->get_counter(alert.first);
        if (value >= alert.second.first) {
            alert.second.second(alert.first, value);
        }
    }
    
    // Check gauge alerts
    for (const auto& alert : gauge_alerts_) {
        uint64_t value = statistics_->get_gauge(alert.first);
        if (value >= alert.second.first) {
            alert.second.second(alert.first, value);
        }
    }
    
    // Check rate alerts
    for (const auto& alert : rate_alerts_) {
        double value = statistics_->get_rate(alert.first);
        if (value >= alert.second.first) {
            alert.second.second(alert.first, value);
        }
    }
}
