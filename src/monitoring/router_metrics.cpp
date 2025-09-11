#include "monitoring/router_metrics.h"
#include <iostream>
#include <sstream>

namespace RouterSim {

RouterMetricsCollector::RouterMetricsCollector(std::shared_ptr<RouterCore> router_core)
    : router_core_(router_core) {
}

void RouterMetricsCollector::collect_metrics(std::vector<MetricValue>& metrics) {
    if (!router_core_) {
        return;
    }
    
    collect_router_metrics(metrics);
    collect_protocol_metrics(metrics);
    collect_traffic_metrics(metrics);
    collect_interface_metrics(metrics);
}

void RouterMetricsCollector::collect_router_metrics(std::vector<MetricValue>& metrics) {
    // Router status
    MetricValue status_metric;
    status_metric.name = "router_status";
    status_metric.help = "Router operational status";
    status_metric.type = MetricType::GAUGE;
    status_metric.value = router_core_->is_running() ? 1.0 : 0.0;
    status_metric.timestamp = std::chrono::steady_clock::now();
    metrics.push_back(status_metric);
    
    // Number of routes
    auto routes = router_core_->get_routes();
    MetricValue routes_metric;
    routes_metric.name = "router_routes_total";
    routes_metric.help = "Total number of routes";
    routes_metric.type = MetricType::GAUGE;
    routes_metric.value = static_cast<double>(routes.size());
    routes_metric.timestamp = std::chrono::steady_clock::now();
    metrics.push_back(routes_metric);
    
    // Number of interfaces
    auto interfaces = router_core_->get_interfaces();
    MetricValue interfaces_metric;
    interfaces_metric.name = "router_interfaces_total";
    interfaces_metric.help = "Total number of interfaces";
    interfaces_metric.type = MetricType::GAUGE;
    interfaces_metric.value = static_cast<double>(interfaces.size());
    interfaces_metric.timestamp = std::chrono::steady_clock::now();
    metrics.push_back(interfaces_metric);
}

void RouterMetricsCollector::collect_protocol_metrics(std::vector<MetricValue>& metrics) {
    // BGP metrics
    auto bgp_protocol = router_core_->get_protocol(Protocol::BGP);
    if (bgp_protocol) {
        auto bgp_stats = bgp_protocol->get_statistics();
        
        MetricValue bgp_routes_metric;
        bgp_routes_metric.name = "bgp_routes_total";
        bgp_routes_metric.help = "Total number of BGP routes";
        bgp_routes_metric.type = MetricType::GAUGE;
        bgp_routes_metric.value = static_cast<double>(bgp_stats.routes_advertised + bgp_stats.routes_learned);
        bgp_routes_metric.timestamp = std::chrono::steady_clock::now();
        metrics.push_back(bgp_routes_metric);
        
        MetricValue bgp_neighbors_metric;
        bgp_neighbors_metric.name = "bgp_neighbors_total";
        bgp_neighbors_metric.help = "Total number of BGP neighbors";
        bgp_neighbors_metric.type = MetricType::GAUGE;
        bgp_neighbors_metric.value = static_cast<double>(bgp_stats.neighbors_active);
        bgp_neighbors_metric.timestamp = std::chrono::steady_clock::now();
        metrics.push_back(bgp_neighbors_metric);
    }
    
    // OSPF metrics
    auto ospf_protocol = router_core_->get_protocol(Protocol::OSPF);
    if (ospf_protocol) {
        auto ospf_stats = ospf_protocol->get_statistics();
        
        MetricValue ospf_routes_metric;
        ospf_routes_metric.name = "ospf_routes_total";
        ospf_routes_metric.help = "Total number of OSPF routes";
        ospf_routes_metric.type = MetricType::GAUGE;
        ospf_routes_metric.value = static_cast<double>(ospf_stats.routes_advertised + ospf_stats.routes_learned);
        ospf_routes_metric.timestamp = std::chrono::steady_clock::now();
        metrics.push_back(ospf_routes_metric);
    }
    
    // IS-IS metrics
    auto isis_protocol = router_core_->get_protocol(Protocol::ISIS);
    if (isis_protocol) {
        auto isis_stats = isis_protocol->get_statistics();
        
        MetricValue isis_routes_metric;
        isis_routes_metric.name = "isis_routes_total";
        isis_routes_metric.help = "Total number of IS-IS routes";
        isis_routes_metric.type = MetricType::GAUGE;
        isis_routes_metric.value = static_cast<double>(isis_stats.routes_advertised + isis_stats.routes_learned);
        isis_routes_metric.timestamp = std::chrono::steady_clock::now();
        metrics.push_back(isis_routes_metric);
    }
}

void RouterMetricsCollector::collect_traffic_metrics(std::vector<MetricValue>& metrics) {
    auto traffic_shaper = router_core_->get_traffic_shaper();
    if (traffic_shaper) {
        auto traffic_stats = traffic_shaper->get_statistics();
        
        MetricValue packets_processed_metric;
        packets_processed_metric.name = "traffic_packets_processed_total";
        packets_processed_metric.help = "Total number of packets processed";
        packets_processed_metric.type = MetricType::COUNTER;
        packets_processed_metric.value = static_cast<double>(traffic_stats.packets_processed);
        packets_processed_metric.timestamp = std::chrono::steady_clock::now();
        metrics.push_back(packets_processed_metric);
        
        MetricValue bytes_processed_metric;
        bytes_processed_metric.name = "traffic_bytes_processed_total";
        bytes_processed_metric.help = "Total number of bytes processed";
        bytes_processed_metric.type = MetricType::COUNTER;
        bytes_processed_metric.value = static_cast<double>(traffic_stats.bytes_processed);
        bytes_processed_metric.timestamp = std::chrono::steady_clock::now();
        metrics.push_back(bytes_processed_metric);
        
        MetricValue packets_dropped_metric;
        packets_dropped_metric.name = "traffic_packets_dropped_total";
        packets_dropped_metric.help = "Total number of packets dropped";
        packets_dropped_metric.type = MetricType::COUNTER;
        packets_dropped_metric.value = static_cast<double>(traffic_stats.packets_dropped);
        packets_dropped_metric.timestamp = std::chrono::steady_clock::now();
        metrics.push_back(packets_dropped_metric);
    }
}

void RouterMetricsCollector::collect_interface_metrics(std::vector<MetricValue>& metrics) {
    auto interfaces = router_core_->get_interfaces();
    
    for (const auto& interface : interfaces) {
        std::map<std::string, std::string> labels;
        labels["interface"] = interface.name;
        labels["status"] = interface.is_up ? "up" : "down";
        
        MetricValue interface_status_metric;
        interface_status_metric.name = "interface_status";
        interface_status_metric.help = "Interface operational status";
        interface_status_metric.type = MetricType::GAUGE;
        interface_status_metric.value = interface.is_up ? 1.0 : 0.0;
        interface_status_metric.labels = labels;
        interface_status_metric.timestamp = std::chrono::steady_clock::now();
        metrics.push_back(interface_status_metric);
        
        MetricValue interface_packets_metric;
        interface_packets_metric.name = "interface_packets_total";
        interface_packets_metric.help = "Total number of packets on interface";
        interface_packets_metric.type = MetricType::COUNTER;
        interface_packets_metric.value = static_cast<double>(interface.packets_rx + interface.packets_tx);
        interface_packets_metric.labels = labels;
        interface_packets_metric.timestamp = std::chrono::steady_clock::now();
        metrics.push_back(interface_packets_metric);
        
        MetricValue interface_bytes_metric;
        interface_bytes_metric.name = "interface_bytes_total";
        interface_bytes_metric.help = "Total number of bytes on interface";
        interface_bytes_metric.type = MetricType::COUNTER;
        interface_bytes_metric.value = static_cast<double>(interface.bytes_rx + interface.bytes_tx);
        interface_bytes_metric.labels = labels;
        interface_bytes_metric.timestamp = std::chrono::steady_clock::now();
        metrics.push_back(interface_bytes_metric);
    }
}

BGPMetricsCollector::BGPMetricsCollector(std::shared_ptr<BGPProtocol> bgp_protocol)
    : bgp_protocol_(bgp_protocol) {
}

void BGPMetricsCollector::collect_metrics(std::vector<MetricValue>& metrics) {
    if (!bgp_protocol_) {
        return;
    }
    
    collect_bgp_session_metrics(metrics);
    collect_bgp_route_metrics(metrics);
    collect_bgp_message_metrics(metrics);
}

void BGPMetricsCollector::collect_bgp_session_metrics(std::vector<MetricValue>& metrics) {
    auto neighbors = bgp_protocol_->get_neighbors();
    
    for (const auto& neighbor : neighbors) {
        std::map<std::string, std::string> labels;
        labels["neighbor"] = neighbor.address;
        labels["as_number"] = std::to_string(neighbor.as_number);
        labels["state"] = neighbor.state;
        
        MetricValue session_status_metric;
        session_status_metric.name = "bgp_session_status";
        session_status_metric.help = "BGP session status";
        session_status_metric.type = MetricType::GAUGE;
        session_status_metric.value = (neighbor.state == "Established") ? 1.0 : 0.0;
        session_status_metric.labels = labels;
        session_status_metric.timestamp = std::chrono::steady_clock::now();
        metrics.push_back(session_status_metric);
        
        MetricValue messages_sent_metric;
        messages_sent_metric.name = "bgp_messages_sent_total";
        messages_sent_metric.help = "Total BGP messages sent";
        messages_sent_metric.type = MetricType::COUNTER;
        messages_sent_metric.value = static_cast<double>(neighbor.messages_sent);
        messages_sent_metric.labels = labels;
        messages_sent_metric.timestamp = std::chrono::steady_clock::now();
        metrics.push_back(messages_sent_metric);
        
        MetricValue messages_received_metric;
        messages_received_metric.name = "bgp_messages_received_total";
        messages_received_metric.help = "Total BGP messages received";
        messages_received_metric.type = MetricType::COUNTER;
        messages_received_metric.value = static_cast<double>(neighbor.messages_received);
        messages_received_metric.labels = labels;
        messages_received_metric.timestamp = std::chrono::steady_clock::now();
        metrics.push_back(messages_received_metric);
    }
}

void BGPMetricsCollector::collect_bgp_route_metrics(std::vector<MetricValue>& metrics) {
    auto advertised_routes = bgp_protocol_->get_advertised_routes();
    auto learned_routes = bgp_protocol_->get_learned_routes();
    
    MetricValue advertised_routes_metric;
    advertised_routes_metric.name = "bgp_routes_advertised_total";
    advertised_routes_metric.help = "Total number of advertised BGP routes";
    advertised_routes_metric.type = MetricType::GAUGE;
    advertised_routes_metric.value = static_cast<double>(advertised_routes.size());
    advertised_routes_metric.timestamp = std::chrono::steady_clock::now();
    metrics.push_back(advertised_routes_metric);
    
    MetricValue learned_routes_metric;
    learned_routes_metric.name = "bgp_routes_learned_total";
    learned_routes_metric.help = "Total number of learned BGP routes";
    learned_routes_metric.type = MetricType::GAUGE;
    learned_routes_metric.value = static_cast<double>(learned_routes.size());
    learned_routes_metric.timestamp = std::chrono::steady_clock::now();
    metrics.push_back(learned_routes_metric);
}

void BGPMetricsCollector::collect_bgp_message_metrics(std::vector<MetricValue>& metrics) {
    // This would collect BGP message statistics
    // Implementation depends on the specific BGP protocol implementation
}

TrafficShapingMetricsCollector::TrafficShapingMetricsCollector(std::shared_ptr<TrafficShaper> traffic_shaper)
    : traffic_shaper_(traffic_shaper) {
}

void TrafficShapingMetricsCollector::collect_metrics(std::vector<MetricValue>& metrics) {
    if (!traffic_shaper_) {
        return;
    }
    
    collect_traffic_shaping_metrics(metrics);
    collect_queue_metrics(metrics);
    collect_bandwidth_metrics(metrics);
}

void TrafficShapingMetricsCollector::collect_traffic_shaping_metrics(std::vector<MetricValue>& metrics) {
    auto stats = traffic_shaper_->get_statistics();
    
    MetricValue packets_processed_metric;
    packets_processed_metric.name = "traffic_shaping_packets_processed_total";
    packets_processed_metric.help = "Total packets processed by traffic shaper";
    packets_processed_metric.type = MetricType::COUNTER;
    packets_processed_metric.value = static_cast<double>(stats.packets_processed);
    packets_processed_metric.timestamp = std::chrono::steady_clock::now();
    metrics.push_back(packets_processed_metric);
    
    MetricValue packets_dropped_metric;
    packets_dropped_metric.name = "traffic_shaping_packets_dropped_total";
    packets_dropped_metric.help = "Total packets dropped by traffic shaper";
    packets_dropped_metric.type = MetricType::COUNTER;
    packets_dropped_metric.value = static_cast<double>(stats.packets_dropped);
    packets_dropped_metric.timestamp = std::chrono::steady_clock::now();
    metrics.push_back(packets_dropped_metric);
}

void TrafficShapingMetricsCollector::collect_queue_metrics(std::vector<MetricValue>& metrics) {
    // This would collect queue-specific metrics
    // Implementation depends on the specific traffic shaping algorithm
}

void TrafficShapingMetricsCollector::collect_bandwidth_metrics(std::vector<MetricValue>& metrics) {
    // This would collect bandwidth utilization metrics
    // Implementation depends on the specific traffic shaping algorithm
}

// RouterAlertRules implementation
std::vector<AlertRule> RouterAlertRules::get_default_rules() {
    std::vector<AlertRule> rules;
    
    rules.push_back(high_cpu_usage());
    rules.push_back(high_memory_usage());
    rules.push_back(interface_down());
    rules.push_back(bgp_session_down());
    rules.push_back(high_packet_loss());
    rules.push_back(queue_overflow());
    rules.push_back(route_flapping());
    rules.push_back(high_latency());
    
    return rules;
}

AlertRule RouterAlertRules::high_cpu_usage() {
    AlertRule rule;
    rule.name = "HighCPUUsage";
    rule.expression = "cpu_usage > 80";
    rule.severity = "warning";
    rule.description = "CPU usage is above 80%";
    rule.summary = "High CPU usage detected";
    rule.enabled = true;
    rule.duration = std::chrono::seconds(300); // 5 minutes
    return rule;
}

AlertRule RouterAlertRules::high_memory_usage() {
    AlertRule rule;
    rule.name = "HighMemoryUsage";
    rule.expression = "memory_usage > 90";
    rule.severity = "critical";
    rule.description = "Memory usage is above 90%";
    rule.summary = "High memory usage detected";
    rule.enabled = true;
    rule.duration = std::chrono::seconds(60); // 1 minute
    return rule;
}

AlertRule RouterAlertRules::interface_down() {
    AlertRule rule;
    rule.name = "InterfaceDown";
    rule.expression = "interface_status == 0";
    rule.severity = "critical";
    rule.description = "Network interface is down";
    rule.summary = "Interface down detected";
    rule.enabled = true;
    rule.duration = std::chrono::seconds(30); // 30 seconds
    return rule;
}

AlertRule RouterAlertRules::bgp_session_down() {
    AlertRule rule;
    rule.name = "BGPSessionDown";
    rule.expression = "bgp_session_status == 0";
    rule.severity = "warning";
    rule.description = "BGP session is down";
    rule.summary = "BGP session down detected";
    rule.enabled = true;
    rule.duration = std::chrono::seconds(60); // 1 minute
    return rule;
}

AlertRule RouterAlertRules::high_packet_loss() {
    AlertRule rule;
    rule.name = "HighPacketLoss";
    rule.expression = "packet_loss_rate > 0.01";
    rule.severity = "warning";
    rule.description = "Packet loss rate is above 1%";
    rule.summary = "High packet loss detected";
    rule.enabled = true;
    rule.duration = std::chrono::seconds(120); // 2 minutes
    return rule;
}

AlertRule RouterAlertRules::queue_overflow() {
    AlertRule rule;
    rule.name = "QueueOverflow";
    rule.expression = "queue_utilization > 0.95";
    rule.severity = "critical";
    rule.description = "Queue utilization is above 95%";
    rule.summary = "Queue overflow detected";
    rule.enabled = true;
    rule.duration = std::chrono::seconds(30); // 30 seconds
    return rule;
}

AlertRule RouterAlertRules::route_flapping() {
    AlertRule rule;
    rule.name = "RouteFlapping";
    rule.expression = "route_changes > 10";
    rule.severity = "warning";
    rule.description = "Route changes are too frequent";
    rule.summary = "Route flapping detected";
    rule.enabled = true;
    rule.duration = std::chrono::seconds(60); // 1 minute
    return rule;
}

AlertRule RouterAlertRules::high_latency() {
    AlertRule rule;
    rule.name = "HighLatency";
    rule.expression = "latency > 100";
    rule.severity = "warning";
    rule.description = "Latency is above 100ms";
    rule.summary = "High latency detected";
    rule.enabled = true;
    rule.duration = std::chrono::seconds(180); // 3 minutes
    return rule;
}

} // namespace RouterSim
