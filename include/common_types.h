#pragma once

#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <atomic>

namespace RouterSim {

// Common packet structure
struct PacketInfo {
    uint64_t id;
    uint32_t size;
    uint32_t priority;
    std::string src_ip;
    std::string dst_ip;
    uint16_t src_port;
    uint16_t dest_port;
    uint16_t dst_port;
    uint8_t protocol;
    uint8_t dscp;
    std::chrono::steady_clock::time_point timestamp;
    
    PacketInfo() : id(0), size(0), priority(0), src_port(0), dest_port(0), dst_port(0), protocol(0), dscp(0) {
        timestamp = std::chrono::steady_clock::now();
    }
};

// Route information
struct RouteInfo {
    std::string destination;
    uint8_t prefix_length;
    std::string next_hop;
    std::string protocol;
    uint32_t metric;
    uint32_t admin_distance;
    bool is_active;
    std::chrono::steady_clock::time_point last_updated;
    std::map<std::string, std::string> attributes;
    
    RouteInfo() : prefix_length(0), metric(0), admin_distance(0), is_active(false) {
        last_updated = std::chrono::steady_clock::now();
    }
};

// Neighbor information
struct NeighborInfo {
    std::string address;
    std::string protocol;
    std::string state;
    std::chrono::steady_clock::time_point last_hello;
    uint32_t hold_time;
    std::map<std::string, std::string> capabilities;
    std::map<std::string, std::string> attributes;
    
    NeighborInfo() : hold_time(0) {
        last_hello = std::chrono::steady_clock::now();
    }
};

// Protocol statistics
struct ProtocolStatistics {
    uint64_t packets_sent;
    uint64_t packets_received;
    uint64_t bytes_sent;
    uint64_t bytes_received;
    uint64_t errors;
    uint64_t timeouts;
    std::chrono::steady_clock::time_point last_reset;
    
    ProtocolStatistics() : packets_sent(0), packets_received(0), bytes_sent(0), 
                          bytes_received(0), errors(0), timeouts(0) {
        last_reset = std::chrono::steady_clock::now();
    }
    
    void reset() {
        packets_sent = 0;
        packets_received = 0;
        bytes_sent = 0;
        bytes_received = 0;
        errors = 0;
        timeouts = 0;
        last_reset = std::chrono::steady_clock::now();
    }
};

// Traffic statistics
struct TrafficStats {
    uint64_t packets_processed;
    uint64_t bytes_processed;
    uint64_t packets_dropped;
    uint64_t bytes_dropped;
    double utilization_percentage;
    std::chrono::steady_clock::time_point last_reset;
    
    TrafficStats() : packets_processed(0), bytes_processed(0), packets_dropped(0), 
                    bytes_dropped(0), utilization_percentage(0.0) {
        last_reset = std::chrono::steady_clock::now();
    }
    
    void reset() {
        packets_processed = 0;
        bytes_processed = 0;
        packets_dropped = 0;
        bytes_dropped = 0;
        utilization_percentage = 0.0;
        last_reset = std::chrono::steady_clock::now();
    }
};

// Protocol types
enum class Protocol {
    BGP,
    OSPF,
    ISIS,
    STATIC,
    CONNECTED
};

// Shaping algorithms
enum class ShapingAlgorithm {
    TOKEN_BUCKET,
    WEIGHTED_FAIR_QUEUE,
    RATE_LIMITING
};

// Token bucket configuration
struct TokenBucketConfig {
    uint64_t capacity;
    uint64_t rate;
    uint64_t burst_size;
    bool allow_burst;
    
    TokenBucketConfig() : capacity(1000), rate(100), burst_size(500), allow_burst(true) {}
};

// WFQ class configuration
struct WFQClass {
    uint8_t class_id;
    uint32_t weight;
    uint64_t min_bandwidth;
    uint64_t max_bandwidth;
    std::string name;
    bool is_active;
    
    WFQClass() : class_id(0), weight(1), min_bandwidth(0), max_bandwidth(0), is_active(true) {}
};

// Queue item for WFQ
struct QueueItem {
    PacketInfo packet;
    uint8_t class_id;
    uint64_t virtual_finish_time;
    std::chrono::steady_clock::time_point arrival_time;
    
    QueueItem() : class_id(0), virtual_finish_time(0) {
        arrival_time = std::chrono::steady_clock::now();
    }
};

// Delay configuration for netem
struct DelayConfig {
    uint32_t delay_ms;
    uint32_t jitter_ms;
    std::string distribution;
    
    DelayConfig() : delay_ms(0), jitter_ms(0), distribution("normal") {}
};

// Loss configuration for netem
struct LossConfig {
    std::string loss_type;
    double loss_percentage;
    
    LossConfig() : loss_type("random"), loss_percentage(0.0) {}
};

// Delay distribution types
enum class DelayDistribution {
    NORMAL,
    UNIFORM,
    PARETO
};

// Loss types
enum class LossType {
    RANDOM,
    STATE,
    GEOMETRIC
};

// PCAP data structures
struct PcapData {
    std::vector<PacketInfo> packets;
    std::string filename;
    std::chrono::system_clock::time_point capture_time;
    
    PcapData() {
        capture_time = std::chrono::system_clock::now();
    }
};

struct PcapDiffOptions {
    bool ignore_timestamps;
    bool ignore_sequence_numbers;
    double tolerance_percentage;
    
    PcapDiffOptions() : ignore_timestamps(false), ignore_sequence_numbers(false), tolerance_percentage(0.0) {}
};

// Callback types
using RouteUpdateCallback = std::function<void(const RouteInfo&, bool)>;
using NeighborUpdateCallback = std::function<void(const NeighborInfo&, bool)>;
using PacketCallback = std::function<void(const PacketInfo&)>;
using DropCallback = std::function<void(const PacketInfo&, const std::string&)>;

} // namespace RouterSim
