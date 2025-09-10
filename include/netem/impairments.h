#pragma once

#include <string>
#include <map>
#include <vector>
#include <mutex>
#include <optional>
#include <chrono>

namespace RouterSim {

// Impairment types
enum class ImpairmentType {
    DELAY,
    LOSS,
    DUPLICATE,
    CORRUPT,
    REORDER,
    RATE_LIMIT
};

// Delay configuration
struct DelayConfig {
    uint32_t delay_ms;
    uint32_t jitter_ms;
    std::string distribution; // "uniform", "normal", "pareto", "paretonormal"
    
    DelayConfig() : delay_ms(0), jitter_ms(0), distribution("uniform") {}
};

// Loss configuration
struct LossConfig {
    std::string loss_type; // "random", "state", "gemodel"
    double loss_percentage;
    
    // For state model
    double p13, p31, p32, p23, p14;
    
    // For gemodel
    double p, r, h, k, one, two;
    
    LossConfig() : loss_type("random"), loss_percentage(0.0), 
                   p13(0), p31(0), p32(0), p23(0), p14(0),
                   p(0), r(0), h(0), k(0), one(0), two(0) {}
};

// Duplicate configuration
struct DuplicateConfig {
    double duplicate_percentage;
    
    DuplicateConfig() : duplicate_percentage(0.0) {}
};

// Corrupt configuration
struct CorruptConfig {
    double corrupt_percentage;
    
    CorruptConfig() : corrupt_percentage(0.0) {}
};

// Reorder configuration
struct ReorderConfig {
    double reorder_percentage;
    uint32_t gap;
    
    ReorderConfig() : reorder_percentage(0.0), gap(0) {}
};

// Rate limit configuration
struct RateLimitConfig {
    std::string rate; // e.g., "1mbit", "100kbit"
    uint32_t burst;
    uint32_t latency; // in milliseconds
    
    RateLimitConfig() : rate("1mbit"), burst(100000), latency(50) {}
};

// Impairment step for sequences
struct ImpairmentStep {
    ImpairmentType type;
    uint32_t delay_ms; // Delay before applying this step
    
    // Configuration for each type
    DelayConfig delay_config;
    LossConfig loss_config;
    DuplicateConfig duplicate_config;
    CorruptConfig corrupt_config;
    ReorderConfig reorder_config;
    RateLimitConfig rate_limit_config;
    
    ImpairmentStep() : type(ImpairmentType::DELAY), delay_ms(0) {}
};

// Complete impairment configuration for an interface
struct ImpairmentConfig {
    std::optional<DelayConfig> delay;
    std::optional<LossConfig> loss;
    std::optional<DuplicateConfig> duplicate;
    std::optional<CorruptConfig> corrupt;
    std::optional<ReorderConfig> reorder;
    std::optional<RateLimitConfig> rate_limit;
};

// Impairment statistics
struct ImpairmentStats {
    uint64_t impairments_applied;
    uint64_t impairments_removed;
    uint64_t interfaces_affected;
    uint64_t scenarios_executed;
    
    void reset() {
        impairments_applied = 0;
        impairments_removed = 0;
        interfaces_affected = 0;
        scenarios_executed = 0;
    }
};

// Netem impairments manager
class NetemImpairments {
public:
    NetemImpairments();
    ~NetemImpairments();
    
    // Lifecycle
    bool initialize();
    bool start();
    bool stop();
    bool is_running() const;
    
    // Impairment management
    bool add_delay(const std::string& interface, const DelayConfig& config);
    bool add_loss(const std::string& interface, const LossConfig& config);
    bool add_duplicate(const std::string& interface, const DuplicateConfig& config);
    bool add_corrupt(const std::string& interface, const CorruptConfig& config);
    bool add_reorder(const std::string& interface, const ReorderConfig& config);
    bool add_rate_limit(const std::string& interface, const RateLimitConfig& config);
    
    // Impairment removal
    bool remove_impairment(const std::string& interface, const std::string& type);
    bool clear_interface_impairments(const std::string& interface);
    bool clear_all_impairments();
    
    // Information retrieval
    std::vector<std::string> get_interfaces() const;
    std::map<std::string, ImpairmentConfig> get_impairments() const;
    ImpairmentConfig get_interface_impairments(const std::string& interface) const;
    
    // Scenario simulation
    bool simulate_network_conditions(const std::string& scenario);
    bool apply_impairment_sequence(const std::string& interface, 
                                  const std::vector<ImpairmentStep>& sequence);
    
    // Statistics
    std::map<std::string, uint64_t> get_statistics() const;
    void reset_statistics();
    
    // Utility
    bool validate_interface(const std::string& interface);
    
private:
    bool initialized_;
    bool running_;
    
    // Impairment tracking
    std::map<std::string, ImpairmentConfig> impairments_;
    
    // Statistics
    mutable std::mutex stats_mutex_;
    ImpairmentStats stats_;
};

} // namespace RouterSim
