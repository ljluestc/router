#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include <atomic>

namespace router_sim {

// Delay distribution types
enum class DelayDistribution {
    UNIFORM,
    NORMAL,
    PARETO,
    PARETONORMAL
};

// Delay configuration
struct DelayConfig {
    bool enabled = false;
    uint32_t delay_ms = 0;
    uint32_t jitter_ms = 0;
    DelayDistribution distribution = DelayDistribution::UNIFORM;
    double correlation = 0.0;
    
    DelayConfig() = default;
    DelayConfig(uint32_t delay, uint32_t jitter = 0, DelayDistribution dist = DelayDistribution::UNIFORM, double corr = 0.0)
        : enabled(true), delay_ms(delay), jitter_ms(jitter), distribution(dist), correlation(corr) {}
};

// Loss configuration
struct LossConfig {
    bool enabled = false;
    double loss_percentage = 0.0;
    double correlation = 0.0;
    bool random = false;
    
    LossConfig() = default;
    LossConfig(double percentage, double corr = 0.0, bool rand = false)
        : enabled(true), loss_percentage(percentage), correlation(corr), random(rand) {}
};

// Duplicate configuration
struct DuplicateConfig {
    bool enabled = false;
    double duplicate_percentage = 0.0;
    double correlation = 0.0;
    
    DuplicateConfig() = default;
    DuplicateConfig(double percentage, double corr = 0.0)
        : enabled(true), duplicate_percentage(percentage), correlation(corr) {}
};

// Corrupt configuration
struct CorruptConfig {
    bool enabled = false;
    double corrupt_percentage = 0.0;
    double correlation = 0.0;
    
    CorruptConfig() = default;
    CorruptConfig(double percentage, double corr = 0.0)
        : enabled(true), corrupt_percentage(percentage), correlation(corr) {}
};

// Reorder configuration
struct ReorderConfig {
    bool enabled = false;
    double reorder_percentage = 0.0;
    uint32_t gap = 0;
    double correlation = 0.0;
    
    ReorderConfig() = default;
    ReorderConfig(double percentage, uint32_t g = 0, double corr = 0.0)
        : enabled(true), reorder_percentage(percentage), gap(g), correlation(corr) {}
};

// Rate limit configuration
struct RateLimitConfig {
    bool enabled = false;
    uint32_t rate_kbps = 0;
    uint32_t burst_kb = 0;
    uint32_t latency_ms = 0;
    
    RateLimitConfig() = default;
    RateLimitConfig(uint32_t rate, uint32_t burst = 0, uint32_t latency = 0)
        : enabled(true), rate_kbps(rate), burst_kb(burst), latency_ms(latency) {}
};

// Bandwidth configuration
struct BandwidthConfig {
    bool enabled = false;
    uint32_t bandwidth_kbps = 0;
    int32_t packet_overhead = 0;
    uint32_t cell_size = 0;
    
    BandwidthConfig() = default;
    BandwidthConfig(uint32_t bandwidth, int32_t overhead = 0, uint32_t cell = 0)
        : enabled(true), bandwidth_kbps(bandwidth), packet_overhead(overhead), cell_size(cell) {}
};

// Complete impairment scenario
struct ImpairmentScenario {
    std::string name;
    std::string description;
    DelayConfig delay;
    LossConfig loss;
    DuplicateConfig duplicate;
    CorruptConfig corrupt;
    ReorderConfig reorder;
    RateLimitConfig rate_limit;
    BandwidthConfig bandwidth;
    
    ImpairmentScenario() = default;
    ImpairmentScenario(const std::string& n, const std::string& desc = "")
        : name(n), description(desc) {}
};

// NetEm Impairments class
class NetEmImpairments {
public:
    NetEmImpairments();
    ~NetEmImpairments();
    
    // Lifecycle
    bool initialize(const std::string& interface);
    bool enable();
    bool disable();
    bool is_enabled() const;
    
    // Individual impairment settings
    bool set_delay(const DelayConfig& config);
    bool set_loss(const LossConfig& config);
    bool set_duplicate(const DuplicateConfig& config);
    bool set_corrupt(const CorruptConfig& config);
    bool set_reorder(const ReorderConfig& config);
    bool set_rate_limit(const RateLimitConfig& config);
    bool set_bandwidth(const BandwidthConfig& config);
    
    // Scenario application
    bool apply_scenario(const ImpairmentScenario& scenario);
    
    // Statistics
    struct Statistics {
        bool enabled;
        std::string interface;
        DelayConfig delay_config;
        LossConfig loss_config;
        DuplicateConfig duplicate_config;
        CorruptConfig corrupt_config;
        ReorderConfig reorder_config;
        RateLimitConfig rate_limit_config;
        BandwidthConfig bandwidth_config;
        std::string qdisc_info;
    };
    
    Statistics get_statistics() const;
    
    // Configuration access
    const DelayConfig& get_delay_config() const { return delay_config_; }
    const LossConfig& get_loss_config() const { return loss_config_; }
    const DuplicateConfig& get_duplicate_config() const { return duplicate_config_; }
    const CorruptConfig& get_corrupt_config() const { return corrupt_config_; }
    const ReorderConfig& get_reorder_config() const { return reorder_config_; }
    const RateLimitConfig& get_rate_limit_config() const { return rate_limit_config_; }
    const BandwidthConfig& get_bandwidth_config() const { return bandwidth_config_; }

private:
    std::string interface_;
    std::atomic<bool> enabled_;
    mutable std::mutex mutex_;
    
    // Configuration storage
    DelayConfig delay_config_;
    LossConfig loss_config_;
    DuplicateConfig duplicate_config_;
    CorruptConfig corrupt_config_;
    ReorderConfig reorder_config_;
    RateLimitConfig rate_limit_config_;
    BandwidthConfig bandwidth_config_;
    
    // Internal methods
    void cleanup();
    std::string get_distribution_string(DelayDistribution dist) const;
};

// Network Impairment Manager for multiple interfaces
class NetworkImpairmentManager {
public:
    NetworkImpairmentManager();
    ~NetworkImpairmentManager();
    
    // Interface management
    bool add_interface(const std::string& interface);
    bool remove_interface(const std::string& interface);
    NetEmImpairments* get_impairments(const std::string& interface);
    std::vector<std::string> get_interfaces() const;
    
    // Scenario management
    bool apply_scenario_to_interface(const std::string& interface, const ImpairmentScenario& scenario);
    
    // Global statistics
    struct GlobalStatistics {
        size_t total_interfaces;
        size_t enabled_interfaces;
    };
    
    GlobalStatistics get_global_statistics() const;

private:
    std::map<std::string, NetEmImpairments*> impairments_;
    mutable std::mutex mutex_;
};

// Predefined impairment scenarios
class ImpairmentScenarios {
public:
    static ImpairmentScenario get_wifi_scenario();
    static ImpairmentScenario get_cellular_scenario();
    static ImpairmentScenario get_satellite_scenario();
    static ImpairmentScenario get_dsl_scenario();
    static ImpairmentScenario get_cable_scenario();
    static ImpairmentScenario get_fiber_scenario();
    static ImpairmentScenario get_high_latency_scenario();
    static ImpairmentScenario get_high_loss_scenario();
    static ImpairmentScenario get_congested_scenario();
    static ImpairmentScenario get_unreliable_scenario();
    
    static std::vector<ImpairmentScenario> get_all_scenarios();
    static ImpairmentScenario get_scenario_by_name(const std::string& name);
};

} // namespace router_sim