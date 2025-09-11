#pragma once

#include <string>
#include <map>
#include <memory>

namespace RouterSim {

// Network impairment types
enum class ImpairmentType {
    DELAY,
    LOSS,
    JITTER,
    DUPLICATE,
    REORDER,
    CORRUPTION
};

// Impairment configuration
struct ImpairmentConfig {
    ImpairmentType type;
    std::string interface;
    std::map<std::string, std::string> parameters;
    bool enabled;
    
    ImpairmentConfig() : type(ImpairmentType::DELAY), enabled(false) {}
};

// Network impairments manager
class NetemImpairments {
public:
    NetemImpairments();
    ~NetemImpairments();
    
    // Core functionality
    bool initialize();
    bool add_impairment(const ImpairmentConfig& config);
    bool remove_impairment(const std::string& interface);
    bool update_impairment(const ImpairmentConfig& config);
    
    // Interface management
    bool enable_impairments(const std::string& interface);
    bool disable_impairments(const std::string& interface);
    bool are_impairments_enabled(const std::string& interface) const;
    
    // Statistics
    struct Statistics {
        uint64_t packets_processed;
        uint64_t packets_delayed;
        uint64_t packets_dropped;
        uint64_t packets_duplicated;
        uint64_t packets_reordered;
        uint64_t packets_corrupted;
    };
    
    Statistics get_statistics() const;
    void reset_statistics();

private:
    bool initialized_;
    std::map<std::string, ImpairmentConfig> impairments_;
    Statistics statistics_;
};

} // namespace RouterSim