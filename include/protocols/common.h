#pragma once

#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <cstdint>

namespace router_sim {

// Common protocol structures
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
};

struct NeighborInfo {
    std::string address;
    std::string protocol;
    std::string state;
    std::chrono::steady_clock::time_point last_hello;
    uint32_t hold_time;
    std::map<std::string, std::string> capabilities;
    std::map<std::string, std::string> attributes;
};

struct ProtocolStatistics {
    uint64_t packets_sent;
    uint64_t packets_received;
    uint64_t bytes_sent;
    uint64_t bytes_received;
    uint64_t errors;
    uint64_t timeouts;
    std::chrono::steady_clock::time_point last_reset;
    
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

// Protocol types
enum class Protocol {
    BGP,
    OSPF,
    ISIS,
    STATIC,
    CONNECTED
};

// Protocol configuration
struct ProtocolConfig {
    std::map<std::string, std::string> parameters;
    bool enabled;
    uint32_t update_interval_ms;
};

// Callback types
using RouteUpdateCallback = std::function<void(const RouteInfo&, bool)>;
using NeighborUpdateCallback = std::function<void(const NeighborInfo&, bool)>;

} // namespace router_sim
