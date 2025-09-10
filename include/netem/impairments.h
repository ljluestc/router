#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <random>
#include <cstdint>

namespace RouterSim {

// Netem impairment configurations
struct DelayConfig {
    uint32_t delay_ms;
    uint32_t jitter_ms;
    enum class DelayDistribution {
        UNIFORM,
        NORMAL,
        PARETO,
        PARETONORMAL
    } distribution;
};

struct LossConfig {
    enum class LossType {
        RANDOM,
        STATE,
        GEMODEL
    } loss_type;
    double loss_percentage;
    double p13, p31, p32, p23, p14; // State model parameters
    double p, r, h, k, 1_minus_h; // Gilbert-Elliot model parameters
};

struct DuplicateConfig {
    double duplicate_percentage;
};

struct CorruptConfig {
    double corrupt_percentage;
};

struct ReorderConfig {
    enum class ReorderType {
        PERCENTAGE,
        GAP
    } reorder_type;
    double reorder_percentage;
    uint32_t gap;
};

struct RateLimitConfig {
    uint32_t rate_kbps;
    uint32_t burst_kb;
};

struct ImpairmentInfo {
    DelayConfig delay;
    LossConfig loss;
    DuplicateConfig duplicate;
    CorruptConfig corrupt;
    ReorderConfig reorder;
    RateLimitConfig rate_limit;
    bool has_delay = false;
    bool has_loss = false;
    bool has_duplicate = false;
    bool has_corrupt = false;
    bool has_reorder = false;
    bool has_rate_limit = false;
};

class NetemImpairments {
public:
    NetemImpairments();
    ~NetemImpairments();

    // Core management
    bool initialize();
    bool start();
    bool stop();
    bool is_running() const;

    // Impairment operations
    bool add_delay(const std::string& interface, const DelayConfig& config);
    bool add_loss(const std::string& interface, const LossConfig& config);
    bool add_duplicate(const std::string& interface, const DuplicateConfig& config);
    bool add_corrupt(const std::string& interface, const CorruptConfig& config);
    bool add_reorder(const std::string& interface, const ReorderConfig& config);
    bool add_rate_limit(const std::string& interface, const RateLimitConfig& config);

    // Management operations
    bool remove_impairment(const std::string& interface);
    bool clear_all_impairments();

    // Information
    std::vector<std::string> get_impaired_interfaces() const;
    ImpairmentInfo get_interface_impairments(const std::string& interface) const;

    // Scenario management
    bool apply_scenario(const std::string& scenario_name);
    bool apply_high_latency_scenario();
    bool apply_packet_loss_scenario();
    bool apply_unreliable_network_scenario();
    bool apply_congested_network_scenario();

private:
    // Internal state
    bool initialized_;
    bool running_;
    std::map<std::string, ImpairmentInfo> active_impairments_;
    std::mt19937 rng_;

    // Helper methods
    void cleanup();
    std::string get_distribution_string(DelayDistribution distribution) const;
    std::vector<std::string> get_available_interfaces() const;
};

// Netem Manager for multiple impairment instances
class NetemManager {
public:
    NetemManager();
    ~NetemManager();

    // Core management
    bool initialize();
    bool start();
    bool stop();

    // Impairment management
    bool add_impairment(const std::string& name, std::unique_ptr<NetemImpairments> impairment);
    bool remove_impairment(const std::string& name);
    NetemImpairments* get_impairment(const std::string& name) const;
    std::vector<std::string> get_impairment_names() const;

private:
    bool initialized_;
    bool running_;
    std::map<std::string, std::unique_ptr<NetemImpairments>> impairments_;
};

} // namespace RouterSim
