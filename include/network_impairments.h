#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <chrono>
#include <mutex>
#include <atomic>
#include <random>

namespace RouterSim {

// Network impairment types
enum class ImpairmentType {
    DELAY,
    JITTER,
    PACKET_LOSS,
    BANDWIDTH_LIMIT,
    REORDERING,
    DUPLICATION,
    CORRUPTION,
    REORDERING_WITH_DELAY
};

// Impairment configuration
struct ImpairmentConfig {
    ImpairmentType type;
    std::string interface;
    uint32_t delay_ms;           // Delay in milliseconds
    uint32_t jitter_ms;          // Jitter in milliseconds
    double loss_percentage;      // Packet loss percentage (0.0 - 100.0)
    uint64_t bandwidth_bps;      // Bandwidth limit in bits per second
    uint32_t reorder_percentage; // Packet reordering percentage
    uint32_t duplicate_percentage; // Packet duplication percentage
    uint32_t corruption_percentage; // Packet corruption percentage
    bool enabled;
    
    ImpairmentConfig() : type(ImpairmentType::DELAY), interface(""), delay_ms(0), 
                         jitter_ms(0), loss_percentage(0.0), bandwidth_bps(0),
                         reorder_percentage(0), duplicate_percentage(0), 
                         corruption_percentage(0), enabled(false) {}
};

// Impairment statistics
struct ImpairmentStats {
    uint64_t packets_processed;
    uint64_t packets_delayed;
    uint64_t packets_dropped;
    uint64_t packets_reordered;
    uint64_t packets_duplicated;
    uint64_t packets_corrupted;
    uint64_t bytes_processed;
    uint64_t bytes_dropped;
    std::chrono::steady_clock::time_point start_time;
    
    ImpairmentStats() : packets_processed(0), packets_delayed(0), packets_dropped(0),
                        packets_reordered(0), packets_duplicated(0), packets_corrupted(0),
                        bytes_processed(0), bytes_dropped(0) {
        start_time = std::chrono::steady_clock::now();
    }
};

// Base network impairment interface
class INetworkImpairment {
public:
    virtual ~INetworkImpairment() = default;
    
    // Lifecycle
    virtual bool initialize(const ImpairmentConfig& config) = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual bool is_running() const = 0;
    
    // Configuration
    virtual void set_config(const ImpairmentConfig& config) = 0;
    virtual ImpairmentConfig get_config() const = 0;
    
    // Packet processing
    virtual bool process_packet(const std::vector<uint8_t>& packet) = 0;
    virtual bool should_drop_packet() = 0;
    virtual bool should_delay_packet() = 0;
    virtual bool should_reorder_packet() = 0;
    virtual bool should_duplicate_packet() = 0;
    virtual bool should_corrupt_packet() = 0;
    
    // Statistics
    virtual ImpairmentStats get_statistics() const = 0;
    virtual void reset_statistics() = 0;
    
    // Control
    virtual void enable() = 0;
    virtual void disable() = 0;
    virtual bool is_enabled() const = 0;
};

// Delay impairment implementation
class DelayImpairment : public INetworkImpairment {
public:
    DelayImpairment();
    ~DelayImpairment() override;

    bool initialize(const ImpairmentConfig& config) override;
    void start() override;
    void stop() override;
    bool is_running() const override;

    void set_config(const ImpairmentConfig& config) override;
    ImpairmentConfig get_config() const override;

    bool process_packet(const std::vector<uint8_t>& packet) override;
    bool should_drop_packet() override;
    bool should_delay_packet() override;
    bool should_reorder_packet() override;
    bool should_duplicate_packet() override;
    bool should_corrupt_packet() override;

    ImpairmentStats get_statistics() const override;
    void reset_statistics() override;

    void enable() override;
    void disable() override;
    bool is_enabled() const override;

private:
    void delay_processing_loop();
    uint32_t calculate_delay();
    
    std::atomic<bool> running_;
    std::atomic<bool> enabled_;
    ImpairmentConfig config_;
    
    std::thread processing_thread_;
    std::queue<std::vector<uint8_t>> packet_queue_;
    mutable std::mutex queue_mutex_;
    
    ImpairmentStats stats_;
    mutable std::mutex stats_mutex_;
    
    std::random_device rd_;
    std::mt19937 gen_;
    std::uniform_int_distribution<uint32_t> delay_dist_;
};

// Packet loss impairment implementation
class PacketLossImpairment : public INetworkImpairment {
public:
    PacketLossImpairment();
    ~PacketLossImpairment() override;

    bool initialize(const ImpairmentConfig& config) override;
    void start() override;
    void stop() override;
    bool is_running() const override;

    void set_config(const ImpairmentConfig& config) override;
    ImpairmentConfig get_config() const override;

    bool process_packet(const std::vector<uint8_t>& packet) override;
    bool should_drop_packet() override;
    bool should_delay_packet() override;
    bool should_reorder_packet() override;
    bool should_duplicate_packet() override;
    bool should_corrupt_packet() override;

    ImpairmentStats get_statistics() const override;
    void reset_statistics() override;

    void enable() override;
    void disable() override;
    bool is_enabled() const override;

private:
    std::atomic<bool> running_;
    std::atomic<bool> enabled_;
    ImpairmentConfig config_;
    
    ImpairmentStats stats_;
    mutable std::mutex stats_mutex_;
    
    std::random_device rd_;
    std::mt19937 gen_;
    std::uniform_real_distribution<double> loss_dist_;
};

// Bandwidth limitation impairment implementation
class BandwidthLimitImpairment : public INetworkImpairment {
public:
    BandwidthLimitImpairment();
    ~BandwidthLimitImpairment() override;

    bool initialize(const ImpairmentConfig& config) override;
    void start() override;
    void stop() override;
    bool is_running() const override;

    void set_config(const ImpairmentConfig& config) override;
    ImpairmentConfig get_config() const override;

    bool process_packet(const std::vector<uint8_t>& packet) override;
    bool should_drop_packet() override;
    bool should_delay_packet() override;
    bool should_reorder_packet() override;
    bool should_duplicate_packet() override;
    bool should_corrupt_packet() override;

    ImpairmentStats get_statistics() const override;
    void reset_statistics() override;

    void enable() override;
    void disable() override;
    bool is_enabled() const override;

private:
    void bandwidth_control_loop();
    uint64_t calculate_transmission_time(uint32_t packet_size);
    
    std::atomic<bool> running_;
    std::atomic<bool> enabled_;
    ImpairmentConfig config_;
    
    std::thread control_thread_;
    std::queue<std::vector<uint8_t>> packet_queue_;
    mutable std::mutex queue_mutex_;
    
    std::atomic<uint64_t> last_transmission_time_;
    
    ImpairmentStats stats_;
    mutable std::mutex stats_mutex_;
};

// Reordering impairment implementation
class ReorderingImpairment : public INetworkImpairment {
public:
    ReorderingImpairment();
    ~ReorderingImpairment() override;

    bool initialize(const ImpairmentConfig& config) override;
    void start() override;
    void stop() override;
    bool is_running() const override;

    void set_config(const ImpairmentConfig& config) override;
    ImpairmentConfig get_config() const override;

    bool process_packet(const std::vector<uint8_t>& packet) override;
    bool should_drop_packet() override;
    bool should_delay_packet() override;
    bool should_reorder_packet() override;
    bool should_duplicate_packet() override;
    bool should_corrupt_packet() override;

    ImpairmentStats get_statistics() const override;
    void reset_statistics() override;

    void enable() override;
    void disable() override;
    bool is_enabled() const override;

private:
    void reordering_loop();
    void reorder_packets();
    
    std::atomic<bool> running_;
    std::atomic<bool> enabled_;
    ImpairmentConfig config_;
    
    std::thread reordering_thread_;
    std::vector<std::vector<uint8_t>> packet_buffer_;
    mutable std::mutex buffer_mutex_;
    
    ImpairmentStats stats_;
    mutable std::mutex stats_mutex_;
    
    std::random_device rd_;
    std::mt19937 gen_;
    std::uniform_int_distribution<uint32_t> reorder_dist_;
};

// Duplication impairment implementation
class DuplicationImpairment : public INetworkImpairment {
public:
    DuplicationImpairment();
    ~DuplicationImpairment() override;

    bool initialize(const ImpairmentConfig& config) override;
    void start() override;
    void stop() override;
    bool is_running() const override;

    void set_config(const ImpairmentConfig& config) override;
    ImpairmentConfig get_config() const override;

    bool process_packet(const std::vector<uint8_t>& packet) override;
    bool should_drop_packet() override;
    bool should_delay_packet() override;
    bool should_reorder_packet() override;
    bool should_duplicate_packet() override;
    bool should_corrupt_packet() override;

    ImpairmentStats get_statistics() const override;
    void reset_statistics() override;

    void enable() override;
    void disable() override;
    bool is_enabled() const override;

private:
    std::atomic<bool> running_;
    std::atomic<bool> enabled_;
    ImpairmentConfig config_;
    
    ImpairmentStats stats_;
    mutable std::mutex stats_mutex_;
    
    std::random_device rd_;
    std::mt19937 gen_;
    std::uniform_int_distribution<uint32_t> duplicate_dist_;
};

// Corruption impairment implementation
class CorruptionImpairment : public INetworkImpairment {
public:
    CorruptionImpairment();
    ~CorruptionImpairment() override;

    bool initialize(const ImpairmentConfig& config) override;
    void start() override;
    void stop() override;
    bool is_running() const override;

    void set_config(const ImpairmentConfig& config) override;
    ImpairmentConfig get_config() const override;

    bool process_packet(const std::vector<uint8_t>& packet) override;
    bool should_drop_packet() override;
    bool should_delay_packet() override;
    bool should_reorder_packet() override;
    bool should_duplicate_packet() override;
    bool should_corrupt_packet() override;

    ImpairmentStats get_statistics() const override;
    void reset_statistics() override;

    void enable() override;
    void disable() override;
    bool is_enabled() const override;

private:
    void corrupt_packet(std::vector<uint8_t>& packet);
    
    std::atomic<bool> running_;
    std::atomic<bool> enabled_;
    ImpairmentConfig config_;
    
    ImpairmentStats stats_;
    mutable std::mutex stats_mutex_;
    
    std::random_device rd_;
    std::mt19937 gen_;
    std::uniform_int_distribution<uint32_t> corruption_dist_;
    std::uniform_int_distribution<uint8_t> byte_dist_;
};

// Network impairment factory
class ImpairmentFactory {
public:
    static std::unique_ptr<INetworkImpairment> create_impairment(ImpairmentType type);
    static std::vector<ImpairmentType> get_supported_impairments();
    static std::string get_impairment_name(ImpairmentType type);
};

// Network impairment manager
class NetworkImpairment {
public:
    NetworkImpairment();
    ~NetworkImpairment();

    // Lifecycle
    bool initialize();
    void start();
    void stop();
    bool is_running() const;

    // Impairment management
    bool add_impairment(const std::string& interface, ImpairmentType type, const ImpairmentConfig& config);
    bool remove_impairment(const std::string& interface);
    bool update_impairment(const std::string& interface, const ImpairmentConfig& config);
    
    // Interface management
    bool enable_impairment(const std::string& interface);
    bool disable_impairment(const std::string& interface);
    bool is_impairment_enabled(const std::string& interface) const;
    
    // Packet processing
    bool process_packet(const std::string& interface, const std::vector<uint8_t>& packet);
    
    // Statistics
    std::map<std::string, ImpairmentStats> get_statistics() const;
    ImpairmentStats get_statistics(const std::string& interface) const;
    void reset_statistics();
    void reset_statistics(const std::string& interface);
    
    // Configuration
    std::map<std::string, ImpairmentConfig> get_configurations() const;
    ImpairmentConfig get_configuration(const std::string& interface) const;

private:
    std::map<std::string, std::unique_ptr<INetworkImpairment>> impairments_;
    mutable std::mutex impairments_mutex_;
    std::atomic<bool> running_;
};

// TC/NetEm integration
class NetEmIntegration {
public:
    NetEmIntegration();
    ~NetEmIntegration();

    // Lifecycle
    bool initialize();
    void start();
    void stop();
    bool is_running() const;

    // TC commands
    bool add_delay(const std::string& interface, uint32_t delay_ms, uint32_t jitter_ms = 0);
    bool add_loss(const std::string& interface, double loss_percentage);
    bool add_bandwidth_limit(const std::string& interface, uint64_t bandwidth_bps);
    bool add_reordering(const std::string& interface, uint32_t reorder_percentage, uint32_t delay_ms = 0);
    bool add_duplication(const std::string& interface, uint32_t duplicate_percentage);
    bool add_corruption(const std::string& interface, uint32_t corruption_percentage);
    
    // TC management
    bool clear_impairments(const std::string& interface);
    bool clear_all_impairments();
    bool show_impairments(const std::string& interface);
    
    // Statistics
    std::map<std::string, std::string> get_tc_statistics(const std::string& interface) const;

private:
    bool execute_tc_command(const std::string& command);
    std::string build_tc_command(const std::string& interface, const std::string& action, const std::string& parameters);
    
    std::atomic<bool> running_;
    mutable std::mutex tc_mutex_;
};

} // namespace RouterSim