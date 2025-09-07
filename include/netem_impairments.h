#pragma once

#include "common_structures.h"
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <atomic>
#include <thread>

namespace RouterSim {

// Netem impairment types
enum class ImpairmentType {
    DELAY,
    JITTER,
    LOSS,
    CORRUPTION,
    DUPLICATION,
    REORDERING,
    BANDWIDTH_LIMIT
};

// Netem command builder
class NetemCommandBuilder {
public:
    static std::string build_delay_command(const std::string& interface, uint32_t delay_ms);
    static std::string build_jitter_command(const std::string& interface, uint32_t delay_ms, uint32_t jitter_ms);
    static std::string build_loss_command(const std::string& interface, double loss_percent);
    static std::string build_corruption_command(const std::string& interface, double corruption_percent);
    static std::string build_duplication_command(const std::string& interface, double duplication_percent);
    static std::string build_reordering_command(const std::string& interface, uint32_t reorder_percent);
    static std::string build_bandwidth_command(const std::string& interface, uint64_t rate_bps);
    static std::string build_combined_command(const std::string& interface, const ImpairmentConfig& config);
    static std::string build_clear_command(const std::string& interface);
};

// Netem impairments manager
class NetemImpairments {
public:
    NetemImpairments();
    ~NetemImpairments();
    
    // Core functionality
    bool start();
    bool stop();
    bool is_running() const;
    
    // Interface management
    bool add_interface(const std::string& name);
    bool remove_interface(const std::string& name);
    bool has_interface(const std::string& name) const;
    std::vector<std::string> get_interfaces() const;
    
    // Impairment configuration
    bool configure_impairments(const std::string& interface, const ImpairmentConfig& config);
    bool clear_impairments(const std::string& interface);
    bool clear_all_impairments();
    
    // Individual impairment types
    bool set_delay(const std::string& interface, uint32_t delay_ms);
    bool set_jitter(const std::string& interface, uint32_t delay_ms, uint32_t jitter_ms);
    bool set_loss(const std::string& interface, double loss_percent);
    bool set_corruption(const std::string& interface, double corruption_percent);
    bool set_duplication(const std::string& interface, double duplication_percent);
    bool set_reordering(const std::string& interface, uint32_t reorder_percent);
    bool set_bandwidth_limit(const std::string& interface, uint64_t rate_bps);
    
    // Packet processing
    bool process_packet(const std::string& interface, Packet& packet);
    
    // Statistics
    std::map<std::string, uint64_t> get_interface_stats(const std::string& interface) const;
    std::map<std::string, uint64_t> get_global_stats() const;
    void reset_statistics();
    void reset_interface_statistics(const std::string& interface);
    
    // Configuration
    ImpairmentConfig get_interface_config(const std::string& interface) const;
    bool is_impairment_active(const std::string& interface, ImpairmentType type) const;
    
    // Predefined scenarios
    bool apply_satellite_scenario(const std::string& interface);
    bool apply_mobile_scenario(const std::string& interface);
    bool apply_dsl_scenario(const std::string& interface);
    bool apply_fiber_scenario(const std::string& interface);
    bool apply_wireless_scenario(const std::string& interface);
    
private:
    struct InterfaceImpairments {
        ImpairmentConfig config;
        bool is_active;
        
        // Statistics
        uint64_t packets_processed;
        uint64_t packets_dropped;
        uint64_t packets_corrupted;
        uint64_t packets_duplicated;
        uint64_t packets_reordered;
        uint64_t bytes_processed;
        uint64_t bytes_dropped;
        uint64_t total_delay_ms;
        uint64_t total_jitter_ms;
        
        InterfaceImpairments() : is_active(false), packets_processed(0), packets_dropped(0),
                               packets_corrupted(0), packets_duplicated(0), packets_reordered(0),
                               bytes_processed(0), bytes_dropped(0), total_delay_ms(0), total_jitter_ms(0) {}
    };
    
    // Internal methods
    bool execute_tc_command(const std::string& command);
    bool is_tc_available() const;
    bool is_interface_valid(const std::string& interface) const;
    void processing_loop();
    bool process_interface_packet(const std::string& interface, Packet& packet);
    void update_statistics(const std::string& interface, const Packet& packet, bool dropped);
    
    // Packet processing helpers
    bool apply_delay(Packet& packet, uint32_t delay_ms);
    bool apply_jitter(Packet& packet, uint32_t delay_ms, uint32_t jitter_ms);
    bool apply_loss(Packet& packet, double loss_percent);
    bool apply_corruption(Packet& packet, double corruption_percent);
    bool apply_duplication(Packet& packet, double duplication_percent);
    bool apply_reordering(Packet& packet, uint32_t reorder_percent);
    bool apply_bandwidth_limit(Packet& packet, uint64_t rate_bps);
    
    // State
    std::atomic<bool> running_;
    std::thread processing_thread_;
    
    // Interface management
    std::map<std::string, InterfaceImpairments> interfaces_;
    mutable std::mutex interfaces_mutex_;
    
    // Global statistics
    uint64_t total_packets_processed_;
    uint64_t total_packets_dropped_;
    uint64_t total_bytes_processed_;
    uint64_t total_bytes_dropped_;
    mutable std::mutex stats_mutex_;
    
    // TC command execution
    bool tc_available_;
    std::string tc_path_;
};

} // namespace RouterSim