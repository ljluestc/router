#pragma once

#include "monitoring/metrics.h"
#include "router_core.h"
#include "protocols/bgp.h"
#include "protocols/ospf.h"
#include "protocols/isis.h"
#include "traffic_shaping.h"
#include <memory>

namespace RouterSim {

// Router metrics collector
class RouterMetricsCollector : public MetricsCollector {
public:
    RouterMetricsCollector(std::shared_ptr<RouterCore> router_core);
    ~RouterMetricsCollector() override = default;

    void collect_metrics(std::vector<MetricValue>& metrics) override;

private:
    std::shared_ptr<RouterCore> router_core_;
    
    // Internal methods
    void collect_router_metrics(std::vector<MetricValue>& metrics);
    void collect_protocol_metrics(std::vector<MetricValue>& metrics);
    void collect_traffic_metrics(std::vector<MetricValue>& metrics);
    void collect_interface_metrics(std::vector<MetricValue>& metrics);
};

// BGP metrics collector
class BGPMetricsCollector : public MetricsCollector {
public:
    BGPMetricsCollector(std::shared_ptr<BGPProtocol> bgp_protocol);
    ~BGPMetricsCollector() override = default;

    void collect_metrics(std::vector<MetricValue>& metrics) override;

private:
    std::shared_ptr<BGPProtocol> bgp_protocol_;
    
    // Internal methods
    void collect_bgp_session_metrics(std::vector<MetricValue>& metrics);
    void collect_bgp_route_metrics(std::vector<MetricValue>& metrics);
    void collect_bgp_message_metrics(std::vector<MetricValue>& metrics);
};

// Traffic shaping metrics collector
class TrafficShapingMetricsCollector : public MetricsCollector {
public:
    TrafficShapingMetricsCollector(std::shared_ptr<TrafficShaper> traffic_shaper);
    ~TrafficShapingMetricsCollector() override = default;

    void collect_metrics(std::vector<MetricValue>& metrics) override;

private:
    std::shared_ptr<TrafficShaper> traffic_shaper_;
    
    // Internal methods
    void collect_traffic_shaping_metrics(std::vector<MetricValue>& metrics);
    void collect_queue_metrics(std::vector<MetricValue>& metrics);
    void collect_bandwidth_metrics(std::vector<MetricValue>& metrics);
};

// Alert rules for router monitoring
class RouterAlertRules {
public:
    static std::vector<AlertRule> get_default_rules();
    
    // Specific alert rules
    static AlertRule high_cpu_usage();
    static AlertRule high_memory_usage();
    static AlertRule interface_down();
    static AlertRule bgp_session_down();
    static AlertRule high_packet_loss();
    static AlertRule queue_overflow();
    static AlertRule route_flapping();
    static AlertRule high_latency();
};

} // namespace RouterSim
