#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <thread>
#include <atomic>
#include <mutex>
#include <random>

namespace RouterSim {

// Forward declarations
struct Packet;
struct ImpairmentConfig;

// Netem impairment types
enum class ImpairmentType {
    DELAY,
    JITTER,
    LOSS,
    DUPLICATE,
    CORRUPTION,
    REORDER,
    BANDWIDTH_LIMIT,
    PACKET_CORRUPTION
};

// Individual impairment class
class Impairment {
public:
    Impairment(ImpairmentType type, double value, double probability = 1.0);
    ~Impairment() = default;

    bool apply(Packet& packet);
    ImpairmentType get_type() const;
    double get_value() const;
    double get_probability() const;
    void set_value(double value);
    void set_probability(double probability);

private:
    ImpairmentType type_;
    double value_;
    double probability_;
    std::mt19937 rng_;
    std::uniform_real_distribution<double> dist_;

    bool apply_delay(Packet& packet);
    bool apply_jitter(Packet& packet);
    bool apply_loss(Packet& packet);
    bool apply_duplicate(Packet& packet);
    bool apply_corruption(Packet& packet);
    bool apply_reorder(Packet& packet);
    bool apply_bandwidth_limit(Packet& packet);
    bool apply_packet_corruption(Packet& packet);
};

// Netem impairments manager
class NetemImpairments {
public:
    NetemImpairments();
    ~NetemImpairments();

    // Interface management
    bool add_interface(const std::string& interface);
    bool remove_interface(const std::string& interface);

    // Impairment configuration
    bool configure_impairments(const std::string& interface, const ImpairmentConfig& config);
    bool add_impairment(const std::string& interface, ImpairmentType type, double value, double probability = 1.0);
    bool remove_impairment(const std::string& interface, ImpairmentType type);
    bool clear_impairments(const std::string& interface);

    // TC/Netem integration
    bool apply_tc_rules(const std::string& interface, const ImpairmentConfig& config);
    bool remove_tc_rules(const std::string& interface);
    bool is_tc_available() const;

    // Packet processing
    bool process_packet(const std::string& interface, Packet& packet);
    std::vector<Packet> process_packets(const std::string& interface, const std::vector<Packet>& packets);

    // Statistics
    std::map<std::string, uint64_t> get_impairment_stats(const std::string& interface) const;
    std::map<std::string, uint64_t> get_global_stats() const;
    void reset_statistics();

    // Control
    void start();
    void stop();
    bool is_running() const;

private:
    // Interface impairment configuration
    struct InterfaceImpairments {
        std::vector<std::unique_ptr<Impairment>> impairments;
        std::map<std::string, uint64_t> stats;
        bool tc_applied;
    };

    // TC/Netem command generation
    std::string generate_tc_delay_command(const std::string& interface, uint32_t delay_ms) const;
    std::string generate_tc_jitter_command(const std::string& interface, uint32_t jitter_ms) const;
    std::string generate_tc_loss_command(const std::string& interface, double loss_percent) const;
    std::string generate_tc_duplicate_command(const std::string& interface, double duplicate_percent) const;
    std::string generate_tc_corruption_command(const std::string& interface, double corruption_percent) const;
    std::string generate_tc_reorder_command(const std::string& interface, double reorder_percent) const;

    // TC command execution
    bool execute_tc_command(const std::string& command);
    std::string get_tc_output(const std::string& command);

    // Packet processing helpers
    bool should_apply_impairment(const std::string& interface, ImpairmentType type) const;
    void update_statistics(const std::string& interface, ImpairmentType type, bool applied);

    // Internal state
    std::map<std::string, InterfaceImpairments> interfaces_;
    std::atomic<bool> running_;
    mutable std::mutex interfaces_mutex_;

    // Global statistics
    std::map<std::string, uint64_t> global_stats_;
    mutable std::mutex stats_mutex_;

    // Random number generation
    std::mt19937 rng_;
    std::uniform_real_distribution<double> uniform_dist_;
};

// Impairment simulation utilities
class ImpairmentSimulator {
public:
    // Network condition simulation
    static bool simulate_high_latency(Packet& packet, uint32_t base_delay_ms, uint32_t jitter_ms);
    static bool simulate_packet_loss(Packet& packet, double loss_rate);
    static bool simulate_bandwidth_constraint(Packet& packet, uint32_t max_bandwidth_bps);
    static bool simulate_network_congestion(Packet& packet, double congestion_factor);

    // Real-world network scenarios
    static bool simulate_satellite_link(Packet& packet);
    static bool simulate_mobile_network(Packet& packet);
    static bool simulate_dsl_connection(Packet& packet);
    static bool simulate_fiber_connection(Packet& packet);

    // Impairment combinations
    static bool simulate_poor_connection(Packet& packet);
    static bool simulate_unstable_connection(Packet& packet);
    static bool simulate_congested_network(Packet& packet);
};

} // namespace RouterSim
