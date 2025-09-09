#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <atomic>

namespace RouterSim {

// Network Impairment Configuration
struct ImpairmentConfig {
    uint32_t delay_ms = 0;           // Base delay in milliseconds
    uint32_t jitter_ms = 0;          // Jitter in milliseconds
    double loss_percentage = 0.0;    // Packet loss percentage (0-100)
    double duplication_percentage = 0.0; // Packet duplication percentage (0-100)
    double reorder_percentage = 0.0; // Packet reordering percentage (0-100)
    uint32_t reorder_gap = 0;        // Reordering gap
    double corruption_percentage = 0.0; // Packet corruption percentage (0-100)
    uint64_t bandwidth_bps = 0;      // Bandwidth limit in bits per second
};

// Network Impairments Class
class NetworkImpairments {
public:
    NetworkImpairments();
    ~NetworkImpairments();
    
    // Core functionality
    bool initialize();
    void cleanup();
    
    // Individual impairment methods
    bool applyDelay(const std::string& interface, uint32_t delay_ms, uint32_t jitter_ms = 0);
    bool applyLoss(const std::string& interface, double loss_percentage);
    bool applyBandwidth(const std::string& interface, uint64_t bandwidth_bps);
    bool applyDuplication(const std::string& interface, double duplication_percentage);
    bool applyReordering(const std::string& interface, double reorder_percentage, uint32_t gap = 0);
    bool applyCorruption(const std::string& interface, double corruption_percentage);
    
    // Complex impairment (combines multiple effects)
    bool applyComplexImpairment(const std::string& interface, const ImpairmentConfig& config);
    
    // Management
    bool clearImpairments(const std::string& interface);
    bool clearAllImpairments();
    
    // Information
    std::vector<std::string> getNetworkInterfaces() const;
    std::string getInterfaceStatus(const std::string& interface) const;
    
    // Statistics
    struct InterfaceStatistics {
        std::string interface_name;
        std::string status;
    };
    
    struct Statistics {
        bool enabled;
        uint64_t total_packets_processed;
        uint64_t total_bytes_processed;
        uint64_t packets_dropped;
        uint64_t bytes_dropped;
        std::vector<InterfaceStatistics> interface_stats;
    };
    
    Statistics getStatistics() const;
    void reset();
    
    // Utility methods
    bool isEnabled() const { return enabled_; }

private:
    // Internal methods
    bool checkTCAvailability() const;
    bool checkNetemModule() const;
    std::string executeCommand(const std::string& command) const;
    
    // State
    std::atomic<bool> enabled_;
    
    // Statistics
    uint64_t total_packets_processed_;
    uint64_t total_bytes_processed_;
    uint64_t packets_dropped_;
    uint64_t bytes_dropped_;
    
    mutable std::mutex mutex_;
};

} // namespace RouterSim