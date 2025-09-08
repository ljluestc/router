#pragma once

#include "router_core.h"
#include <random>
#include <chrono>
#include <thread>
#include <atomic>
#include <mutex>
#include <queue>
#include <vector>
#include <memory>

namespace RouterSim {

// Network impairment types
enum class ImpairmentType {
    DELAY,
    JITTER,
    PACKET_LOSS,
    PACKET_DUPLICATION,
    PACKET_REORDERING,
    PACKET_CORRUPTION,
    BANDWIDTH_LIMITATION,
    BURST_LOSS
};

// Impairment configuration
struct ImpairmentProfile {
    std::string name;
    std::map<ImpairmentType, double> parameters;
    bool enabled;
    std::chrono::milliseconds duration;
    std::string interface_filter;
};

// Delay and jitter implementation
class DelayImpairment {
public:
    DelayImpairment();
    ~DelayImpairment() = default;

    void set_delay(double delay_ms);
    void set_jitter(double jitter_ms);
    void set_delay_distribution(const std::string& distribution);
    
    std::chrono::milliseconds calculate_delay() const;
    bool should_apply_delay() const;

private:
    double base_delay_ms_;
    double jitter_ms_;
    std::string distribution_;
    mutable std::mt19937 rng_;
    mutable std::mutex rng_mutex_;
    
    std::chrono::milliseconds calculate_normal_delay() const;
    std::chrono::milliseconds calculate_uniform_delay() const;
    std::chrono::milliseconds calculate_exponential_delay() const;
};

// Packet loss implementation
class PacketLossImpairment {
public:
    PacketLossImpairment();
    ~PacketLossImpairment() = default;

    void set_loss_rate(double loss_percent);
    void set_burst_loss(double burst_probability, int burst_length);
    void set_loss_pattern(const std::vector<bool>& pattern);
    
    bool should_drop_packet() const;
    bool should_drop_burst() const;

private:
    double loss_rate_;
    double burst_probability_;
    int burst_length_;
    std::vector<bool> loss_pattern_;
    mutable size_t pattern_index_;
    mutable std::mt19937 rng_;
    mutable std::mutex rng_mutex_;
    mutable bool in_burst_;
    mutable int burst_remaining_;
};

// Packet duplication implementation
class PacketDuplicationImpairment {
public:
    PacketDuplicationImpairment();
    ~PacketDuplicationImpairment() = default;

    void set_duplication_rate(double dup_percent);
    void set_max_duplicates(int max_dups);
    
    int get_duplicate_count() const;

private:
    double duplication_rate_;
    int max_duplicates_;
    mutable std::mt19937 rng_;
    mutable std::mutex rng_mutex_;
};

// Packet reordering implementation
class PacketReorderingImpairment {
public:
    PacketReorderingImpairment();
    ~PacketReorderingImpairment() = default;

    void set_reorder_rate(double reorder_percent);
    void set_reorder_window(int window_size);
    void set_reorder_delay(std::chrono::milliseconds delay);
    
    bool should_reorder() const;
    std::chrono::milliseconds get_reorder_delay() const;

private:
    double reorder_rate_;
    int reorder_window_;
    std::chrono::milliseconds reorder_delay_;
    mutable std::mt19937 rng_;
    mutable std::mutex rng_mutex_;
};

// Packet corruption implementation
class PacketCorruptionImpairment {
public:
    PacketCorruptionImpairment();
    ~PacketCorruptionImpairment() = default;

    void set_corruption_rate(double corruption_percent);
    void set_corruption_type(const std::string& type);
    void set_corruption_pattern(const std::vector<uint8_t>& pattern);
    
    bool should_corrupt() const;
    std::vector<uint8_t> corrupt_packet(const std::vector<uint8_t>& packet) const;

private:
    double corruption_rate_;
    std::string corruption_type_;
    std::vector<uint8_t> corruption_pattern_;
    mutable std::mt19937 rng_;
    mutable std::mt19937 bit_rng_;
    mutable std::mutex rng_mutex_;
    
    std::vector<uint8_t> random_corruption(const std::vector<uint8_t>& packet) const;
    std::vector<uint8_t> bit_flip_corruption(const std::vector<uint8_t>& packet) const;
    std::vector<uint8_t> pattern_corruption(const std::vector<uint8_t>& packet) const;
};

// Bandwidth limitation implementation
class BandwidthLimitationImpairment {
public:
    BandwidthLimitationImpairment();
    ~BandwidthLimitationImpairment() = default;

    void set_bandwidth_limit(double bandwidth_mbps);
    void set_burst_allowance(double burst_mbps);
    void set_traffic_shaping(bool enable);
    
    bool should_limit_bandwidth() const;
    std::chrono::milliseconds get_transmission_delay(int packet_size_bytes) const;

private:
    double bandwidth_mbps_;
    double burst_mbps_;
    bool traffic_shaping_enabled_;
    double current_bucket_tokens_;
    std::chrono::steady_clock::time_point last_update_;
    mutable std::mutex mutex_;
};

// Network Impairments main class
class NetworkImpairments {
public:
    NetworkImpairments();
    ~NetworkImpairments();

    // Profile management
    bool create_profile(const ImpairmentProfile& profile);
    bool delete_profile(const std::string& name);
    bool activate_profile(const std::string& name);
    bool deactivate_profile(const std::string& name);
    std::vector<ImpairmentProfile> get_profiles() const;
    ImpairmentProfile* get_profile(const std::string& name);

    // Interface-specific impairments
    bool configure_interface_impairments(const std::string& interface,
                                        const ImpairmentProfile& profile);
    bool remove_interface_impairments(const std::string& interface);

    // Real-time impairment control
    bool enable_impairment(const std::string& interface, ImpairmentType type);
    bool disable_impairment(const std::string& interface, ImpairmentType type);
    bool update_impairment_parameter(const std::string& interface, 
                                    ImpairmentType type, double value);

    // Packet processing
    std::vector<std::vector<uint8_t>> process_packet(const std::string& interface,
                                                    const std::vector<uint8_t>& packet);
    bool is_impairment_active(const std::string& interface) const;

    // Statistics
    struct ImpairmentStats {
        uint64_t packets_processed;
        uint64_t packets_delayed;
        uint64_t packets_dropped;
        uint64_t packets_duplicated;
        uint64_t packets_reordered;
        uint64_t packets_corrupted;
        uint64_t bytes_processed;
        double average_delay_ms;
        double current_loss_rate;
        std::map<ImpairmentType, uint64_t> type_stats;
    };

    ImpairmentStats get_interface_stats(const std::string& interface) const;
    void reset_interface_stats(const std::string& interface);

    // tc/netem integration
    bool apply_tc_netem_impairments(const std::string& interface,
                                   const ImpairmentProfile& profile);
    bool remove_tc_netem_impairments(const std::string& interface);
    bool is_tc_netem_available() const;

    // Configuration
    void set_global_impairment_limit(int max_impairments);
    void set_impairment_logging(bool enable);

private:
    // Per-interface impairment state
    struct InterfaceImpairments {
        std::unique_ptr<DelayImpairment> delay;
        std::unique_ptr<PacketLossImpairment> packet_loss;
        std::unique_ptr<PacketDuplicationImpairment> duplication;
        std::unique_ptr<PacketReorderingImpairment> reordering;
        std::unique_ptr<PacketCorruptionImpairment> corruption;
        std::unique_ptr<BandwidthLimitationImpairment> bandwidth_limit;
        
        ImpairmentProfile active_profile;
        ImpairmentStats stats;
        bool is_active;
        mutable std::mutex mutex;
    };

    // Impairment application
    std::vector<std::vector<uint8_t>> apply_delay_impairment(
        InterfaceImpairments& impairments, 
        const std::vector<uint8_t>& packet);
    std::vector<std::vector<uint8_t>> apply_loss_impairment(
        InterfaceImpairments& impairments, 
        const std::vector<uint8_t>& packet);
    std::vector<std::vector<uint8_t>> apply_duplication_impairment(
        InterfaceImpairments& impairments, 
        const std::vector<uint8_t>& packet);
    std::vector<std::vector<uint8_t>> apply_reordering_impairment(
        InterfaceImpairments& impairments, 
        const std::vector<uint8_t>& packet);
    std::vector<std::vector<uint8_t>> apply_corruption_impairment(
        InterfaceImpairments& impairments, 
        const std::vector<uint8_t>& packet);
    std::vector<std::vector<uint8_t>> apply_bandwidth_impairment(
        InterfaceImpairments& impairments, 
        const std::vector<uint8_t>& packet);

    // tc/netem commands
    std::string build_tc_netem_command(const ImpairmentProfile& profile) const;
    bool execute_tc_command(const std::string& command) const;

    // State
    std::map<std::string, std::unique_ptr<InterfaceImpairments>> interface_impairments_;
    std::map<std::string, ImpairmentProfile> profiles_;
    int max_impairments_;
    bool logging_enabled_;
    mutable std::mutex impairments_mutex_;

    // Background processing
    std::thread impairment_thread_;
    std::atomic<bool> impairment_running_;
    std::queue<std::pair<std::string, std::vector<uint8_t>>> packet_queue_;
    std::mutex queue_mutex_;
    std::condition_variable cv_;

    void impairment_processing_loop();
};

} // namespace RouterSim
