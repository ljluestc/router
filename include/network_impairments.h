#pragma once

#include <string>
#include <map>
#include <memory>
#include <vector>

namespace RouterSim {

// Network impairment configuration
struct ImpairmentConfig {
    // Delay parameters
    uint32_t delay_ms;
    uint32_t delay_jitter_ms;
    uint32_t delay_correlation; // 0-100%
    
    // Loss parameters
    uint32_t loss_percent;
    uint32_t loss_correlation; // 0-100%
    
    // Duplicate parameters
    uint32_t duplicate_percent;
    
    // Reordering parameters
    uint32_t reorder_percent;
    uint32_t reorder_correlation; // 0-100%
    
    // Corruption parameters
    uint32_t corrupt_percent;
    
    // Bandwidth limiting
    uint32_t bandwidth_kbps;
    
    // Constructor with default values
    ImpairmentConfig() 
        : delay_ms(0), delay_jitter_ms(0), delay_correlation(0),
          loss_percent(0), loss_correlation(0),
          duplicate_percent(0),
          reorder_percent(0), reorder_correlation(0),
          corrupt_percent(0), bandwidth_kbps(0) {}
};

// Individual network impairment
class NetworkImpairment {
public:
    NetworkImpairment();
    ~NetworkImpairment();

    // Control
    bool enable(const std::string& interface, const ImpairmentConfig& config);
    void disable();
    bool isEnabled() const;

    // Configuration
    const ImpairmentConfig& getConfig() const;
    std::string getInterface() const;

private:
    bool enabled_;
    std::string interface_;
    ImpairmentConfig config_;
    
    // Internal methods
    bool applyTCRules();
    void removeTCRules();
    bool executeCommand(const std::string& command);
};

// Impairment manager for multiple impairments
class ImpairmentManager {
public:
    ImpairmentManager();
    ~ImpairmentManager();

    // Impairment management
    bool addImpairment(const std::string& name, const std::string& interface, const ImpairmentConfig& config);
    bool removeImpairment(const std::string& name);
    bool updateImpairment(const std::string& name, const ImpairmentConfig& config);
    
    // Control
    bool enableImpairment(const std::string& name);
    bool disableImpairment(const std::string& name);
    
    // Information
    std::vector<std::string> getImpairmentNames() const;
    bool isImpairmentEnabled(const std::string& name) const;
    ImpairmentConfig getImpairmentConfig(const std::string& name) const;
    std::map<std::string, ImpairmentConfig> getAllImpairments() const;

    // Configuration management
    bool loadConfiguration(const std::string& config_file);
    bool saveConfiguration(const std::string& config_file);

    // Scenario-based testing
    bool loadScenario(const std::string& scenario_file);
    bool runScenario(const std::string& scenario_name);
    std::vector<std::string> getAvailableScenarios() const;

private:
    std::map<std::string, std::unique_ptr<NetworkImpairment>> impairments_;
};

// Predefined impairment scenarios
class ImpairmentScenarios {
public:
    // Common network conditions
    static ImpairmentConfig getHighLatencyScenario();
    static ImpairmentConfig getPacketLossScenario();
    static ImpairmentConfig getBandwidthLimitedScenario();
    static ImpairmentConfig getUnstableConnectionScenario();
    static ImpairmentConfig getCorruptedDataScenario();
    
    // Real-world scenarios
    static ImpairmentConfig getSatelliteLinkScenario();
    static ImpairmentConfig getCellularNetworkScenario();
    static ImpairmentConfig getWiFiScenario();
    static ImpairmentConfig getDSLScenario();
    static ImpairmentConfig getFiberScenario();
    
    // Stress testing scenarios
    static ImpairmentConfig getExtremeLatencyScenario();
    static ImpairmentConfig getHighLossScenario();
    static ImpairmentConfig getSevereReorderingScenario();
    static ImpairmentConfig getBandwidthStarvedScenario();
};

// Impairment monitoring and statistics
struct ImpairmentStats {
    uint64_t packets_processed;
    uint64_t packets_delayed;
    uint64_t packets_dropped;
    uint64_t packets_duplicated;
    uint64_t packets_reordered;
    uint64_t packets_corrupted;
    double average_delay_ms;
    double current_loss_rate;
    double current_duplicate_rate;
    double current_reorder_rate;
    double current_corrupt_rate;
    
    ImpairmentStats() 
        : packets_processed(0), packets_delayed(0), packets_dropped(0),
          packets_duplicated(0), packets_reordered(0), packets_corrupted(0),
          average_delay_ms(0.0), current_loss_rate(0.0), current_duplicate_rate(0.0),
          current_reorder_rate(0.0), current_corrupt_rate(0.0) {}
};

class ImpairmentMonitor {
public:
    ImpairmentMonitor();
    ~ImpairmentMonitor();

    // Statistics
    ImpairmentStats getStats(const std::string& impairment_name) const;
    std::map<std::string, ImpairmentStats> getAllStats() const;
    void resetStats(const std::string& impairment_name);
    void resetAllStats();

    // Monitoring
    void startMonitoring();
    void stopMonitoring();
    bool isMonitoring() const;

private:
    bool monitoring_;
    std::map<std::string, ImpairmentStats> stats_;
    
    // Internal methods
    void updateStats(const std::string& impairment_name, const ImpairmentConfig& config);
};

} // namespace RouterSim