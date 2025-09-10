#pragma once

#include <string>
#include <vector>
#include <map>
#include <cstdint>

namespace router_sim {

struct ImpairmentConfig {
    std::string interface;
    std::string type;
    std::map<std::string, std::string> parameters;
};

struct ImpairmentProfile {
    uint32_t delay_ms = 0;
    uint32_t jitter_ms = 0;
    double loss_percentage = 0.0;
    double duplicate_percentage = 0.0;
    double corruption_percentage = 0.0;
    double reorder_percentage = 0.0;
    uint32_t reorder_gap = 0;
};

class NetworkImpairments {
public:
    struct Statistics {
        uint32_t total_impairments;
        uint32_t delay_impairments;
        uint32_t loss_impairments;
        uint32_t duplicate_impairments;
        uint32_t corruption_impairments;
        uint32_t reorder_impairments;
        uint32_t rate_limit_impairments;
        bool enabled;
    };

    NetworkImpairments();
    ~NetworkImpairments();

    // Basic impairments
    bool add_delay(const std::string& interface, uint32_t delay_ms, uint32_t jitter_ms = 0);
    bool add_loss(const std::string& interface, double loss_percentage);
    bool add_duplicate(const std::string& interface, double duplicate_percentage);
    bool add_corruption(const std::string& interface, double corruption_percentage);
    bool add_reorder(const std::string& interface, double reorder_percentage, uint32_t gap = 0);
    bool add_rate_limit(const std::string& interface, uint64_t rate_kbps);

    // Advanced impairments
    bool add_combined_impairment(const std::string& interface, const ImpairmentProfile& profile);

    // Management
    bool remove_impairment(const std::string& interface);
    bool clear_all_impairments();
    std::vector<ImpairmentConfig> get_impairments() const;
    ImpairmentConfig get_impairment(const std::string& interface) const;
    bool is_impairment_active(const std::string& interface) const;

    // Interface management
    std::vector<std::string> get_available_interfaces() const;

    // Status and statistics
    std::string get_tc_status(const std::string& interface) const;
    Statistics get_statistics() const;

    // Control
    void set_enabled(bool enabled);
    bool is_enabled() const;

private:
    bool execute_tc_command(const std::string& command);
    
    std::map<std::string, ImpairmentConfig> impairments_;
    bool enabled_;
};

} // namespace router_sim