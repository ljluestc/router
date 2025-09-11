# API Documentation

## Overview

The Multi-Protocol Router Simulator provides a comprehensive API for programmatic control and monitoring. The API is available through both C++ and Go interfaces, with a REST API for external integration.

## C++ API

### Core Router Interface

#### RouterCore Class

The main router simulation class that orchestrates all components.

```cpp
class RouterCore {
public:
    // Lifecycle
    bool initialize(const std::string& config_file);
    bool start();
    bool stop();
    bool shutdown();
    bool is_running() const;
    bool is_initialized() const;
    
    // Interface management
    bool add_interface(const std::string& name, const std::string& address);
    bool remove_interface(const std::string& name);
    std::vector<Interface> get_interfaces() const;
    
    // Protocol management
    bool configure_protocol(Protocol protocol, const std::map<std::string, std::string>& config);
    std::shared_ptr<ProtocolInterface> get_protocol(Protocol protocol) const;
    
    // Route management
    bool add_route(const Route& route);
    bool remove_route(const std::string& destination);
    std::vector<Route> get_routes() const;
    
    // Traffic shaping
    std::shared_ptr<TrafficShaper> get_traffic_shaper() const;
    
    // Statistics
    RouterStatistics get_statistics() const;
};
```

#### Protocol Interface

Base interface for all routing protocols.

```cpp
class ProtocolInterface {
public:
    virtual ~ProtocolInterface() = default;
    
    // Lifecycle
    virtual bool initialize(const std::map<std::string, std::string>& config) = 0;
    virtual bool start() = 0;
    virtual bool stop() = 0;
    virtual bool is_running() const = 0;
    
    // Configuration
    virtual void update_configuration(const std::map<std::string, std::string>& config) = 0;
    virtual std::map<std::string, std::string> get_configuration() const = 0;
    
    // Statistics
    virtual ProtocolStatistics get_statistics() const = 0;
};
```

### BGP Protocol

#### BGPProtocol Class

```cpp
class BGPProtocol : public ProtocolInterface {
public:
    // Route management
    bool advertise_route(const std::string& prefix, uint8_t prefix_length, uint32_t metric);
    bool withdraw_route(const std::string& prefix, uint8_t prefix_length);
    std::vector<std::string> get_advertised_routes() const;
    std::vector<std::string> get_learned_routes() const;
    std::vector<BGPRoute> get_bgp_routes() const;
    
    // Neighbor management
    bool add_neighbor(const std::string& address, uint32_t as_number);
    bool remove_neighbor(const std::string& address);
    std::vector<BGPNeighbor> get_neighbors() const;
    BGPNeighbor get_neighbor(const std::string& address) const;
    
    // Policy management
    bool set_export_policy(const std::string& policy_name, const std::string& policy_definition);
    bool set_import_policy(const std::string& policy_name, const std::string& policy_definition);
    
    // Callbacks
    void set_route_update_callback(RouteUpdateCallback callback);
    void set_neighbor_callback(NeighborCallback callback);
};
```

#### BGP Data Structures

```cpp
struct BGPRoute {
    std::string prefix;
    uint8_t prefix_length;
    uint32_t metric;
    std::string next_hop;
    std::string origin;
    std::vector<uint32_t> as_path;
    std::map<std::string, std::string> attributes;
    bool is_valid;
    std::chrono::steady_clock::time_point last_updated;
};

struct BGPNeighbor {
    std::string address;
    uint32_t as_number;
    std::string state;
    uint32_t hold_time;
    uint32_t keepalive_interval;
    uint64_t messages_sent;
    uint64_t messages_received;
    std::string last_error;
    std::chrono::steady_clock::time_point last_hello;
    std::chrono::steady_clock::time_point last_keepalive_sent;
    std::map<std::string, std::string> capabilities;
};
```

### Traffic Shaping

#### WeightedFairQueue Class

```cpp
class WeightedFairQueue {
public:
    // Core operations
    bool initialize(const std::vector<WFQClass>& classes);
    bool enqueue_packet(const PacketInfo& packet, uint8_t class_id);
    bool dequeue_packet(PacketInfo& packet);
    bool is_empty() const;
    size_t queue_size() const;
    size_t queue_size(uint8_t class_id) const;
    
    // Class management
    bool add_class(const WFQClass& wfq_class);
    bool remove_class(uint8_t class_id);
    bool update_class(const WFQClass& wfq_class);
    std::vector<WFQClass> get_classes() const;
    
    // Packet classification
    void set_classifier(std::function<uint8_t(const PacketInfo&)> classifier);
    uint8_t classify_packet(const PacketInfo& packet) const;
    
    // Statistics
    WFQStatistics get_statistics() const;
    void reset_statistics();
};
```

#### DeficitRoundRobin Class

```cpp
class DeficitRoundRobin {
public:
    // Core operations
    bool initialize(const std::vector<DRRClass>& classes);
    bool enqueue_packet(const PacketInfo& packet, uint8_t class_id);
    bool dequeue_packet(PacketInfo& packet);
    bool is_empty() const;
    size_t queue_size() const;
    size_t queue_size(uint8_t class_id) const;
    
    // Class management
    bool add_class(const DRRClass& drr_class);
    bool remove_class(uint8_t class_id);
    bool update_class(const DRRClass& drr_class);
    std::vector<DRRClass> get_classes() const;
    
    // Statistics
    DRRStatistics get_statistics() const;
    void reset_statistics();
};
```

### Monitoring

#### MonitoringSystem Class

```cpp
class MonitoringSystem {
public:
    // Metrics management
    void register_collector(std::shared_ptr<MetricsCollector> collector);
    void unregister_collector(std::shared_ptr<MetricsCollector> collector);
    
    // Metric operations
    void increment_counter(const std::string& name, const std::map<std::string, std::string>& labels = {});
    void set_gauge(const std::string& name, double value, const std::map<std::string, std::string>& labels = {});
    void observe_histogram(const std::string& name, double value, const std::map<std::string, std::string>& labels = {});
    void observe_summary(const std::string& name, double value, const std::map<std::string, std::string>& labels = {});
    
    // Metric retrieval
    std::vector<MetricValue> get_metrics() const;
    std::vector<HistogramMetric> get_histograms() const;
    std::vector<SummaryMetric> get_summaries() const;
    
    // Alert management
    void add_alert_rule(const AlertRule& rule);
    void remove_alert_rule(const std::string& name);
    void evaluate_alerts();
    std::vector<Alert> get_active_alerts() const;
    
    // System management
    bool start();
    bool stop();
    bool is_running() const;
};
```

## Go API

### HTTP Server

The Go API server provides REST endpoints for external integration.

#### Base URL
```
http://localhost:8080/api/v1
```

### Router Endpoints

#### Get Router Status
```http
GET /router/status
```

**Response:**
```json
{
  "status": "running",
  "uptime": "1h30m45s",
  "interfaces": 2,
  "routes": 150,
  "protocols": ["bgp", "ospf"]
}
```

#### Get Routes
```http
GET /router/routes
```

**Response:**
```json
{
  "routes": [
    {
      "destination": "0.0.0.0/0",
      "next_hop": "192.168.1.254",
      "protocol": "static",
      "metric": 1,
      "age": "1h30m45s"
    }
  ]
}
```

#### Load Scenario
```http
POST /router/scenario
Content-Type: application/json

{
  "name": "bgp_convergence",
  "config": {
    "routers": 3,
    "protocols": ["bgp"],
    "duration": "5m"
  }
}
```

### Analytics Endpoints

#### Get Metrics
```http
GET /analytics/metrics
```

**Response:**
```json
{
  "metrics": [
    {
      "name": "router_status",
      "type": "gauge",
      "value": 1.0,
      "labels": {},
      "timestamp": "2024-01-01T12:00:00Z"
    }
  ]
}
```

#### Query Analytics
```http
POST /analytics/query
Content-Type: application/json

{
  "query": "rate(router_packets_processed_total[5m])",
  "start": "2024-01-01T12:00:00Z",
  "end": "2024-01-01T12:05:00Z"
}
```

### Testing Endpoints

#### Start Capture
```http
POST /test/capture
Content-Type: application/json

{
  "interface": "eth0",
  "duration": "30s",
  "filter": "tcp port 80"
}
```

#### Compare PCAPs
```http
POST /test/compare
Content-Type: application/json

{
  "baseline": "baseline.pcap",
  "current": "current.pcap",
  "options": {
    "ignore_timestamps": true,
    "tolerance": 0.01
  }
}
```

## WebSocket API

### Real-time Updates

Connect to `/ws` for real-time updates.

#### Message Format
```json
{
  "type": "metric_update",
  "data": {
    "name": "router_status",
    "value": 1.0,
    "timestamp": "2024-01-01T12:00:00Z"
  }
}
```

#### Message Types
- `metric_update`: Metric value changes
- `alert_fired`: New alert triggered
- `alert_resolved`: Alert resolved
- `route_update`: Route table changes
- `neighbor_update`: Neighbor status changes

## Error Handling

### HTTP Status Codes

- `200 OK`: Success
- `400 Bad Request`: Invalid request
- `404 Not Found`: Resource not found
- `500 Internal Server Error`: Server error

### Error Response Format

```json
{
  "error": {
    "code": "INVALID_REQUEST",
    "message": "Invalid configuration parameter",
    "details": {
      "field": "asn",
      "value": "invalid"
    }
  }
}
```

## Rate Limiting

API endpoints are rate-limited to prevent abuse:

- **General endpoints**: 100 requests/minute
- **Analytics endpoints**: 50 requests/minute
- **Testing endpoints**: 10 requests/minute

Rate limit headers are included in responses:
```
X-RateLimit-Limit: 100
X-RateLimit-Remaining: 95
X-RateLimit-Reset: 1640995200
```

## Authentication

Currently, the API does not require authentication. In production environments, implement appropriate authentication mechanisms:

- API keys
- OAuth 2.0
- JWT tokens

## SDKs

### Python SDK

```python
import requests

class RouterSimClient:
    def __init__(self, base_url="http://localhost:8080"):
        self.base_url = base_url
        self.session = requests.Session()
    
    def get_router_status(self):
        response = self.session.get(f"{self.base_url}/api/v1/router/status")
        return response.json()
    
    def get_routes(self):
        response = self.session.get(f"{self.base_url}/api/v1/router/routes")
        return response.json()
    
    def load_scenario(self, scenario_config):
        response = self.session.post(
            f"{self.base_url}/api/v1/router/scenario",
            json=scenario_config
        )
        return response.json()

# Usage
client = RouterSimClient()
status = client.get_router_status()
print(f"Router status: {status['status']}")
```

### JavaScript SDK

```javascript
class RouterSimClient {
    constructor(baseUrl = 'http://localhost:8080') {
        this.baseUrl = baseUrl;
    }
    
    async getRouterStatus() {
        const response = await fetch(`${this.baseUrl}/api/v1/router/status`);
        return await response.json();
    }
    
    async getRoutes() {
        const response = await fetch(`${this.baseUrl}/api/v1/router/routes`);
        return await response.json();
    }
    
    async loadScenario(scenarioConfig) {
        const response = await fetch(`${this.baseUrl}/api/v1/router/scenario`, {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify(scenarioConfig)
        });
        return await response.json();
    }
}

// Usage
const client = new RouterSimClient();
const status = await client.getRouterStatus();
console.log(`Router status: ${status.status}`);
```

## Examples

### Complete Router Setup

```cpp
#include "router_core.h"
#include "protocols/bgp.h"
#include "traffic_shaping/wfq.h"
#include "monitoring/metrics.h"

using namespace RouterSim;

int main() {
    // Create router
    auto router = std::make_unique<RouterCore>();
    router->initialize("config.yaml");
    
    // Add interfaces
    router->add_interface("eth0", "192.168.1.1/24");
    router->add_interface("eth1", "10.0.0.1/24");
    
    // Configure BGP
    std::map<std::string, std::string> bgp_config;
    bgp_config["local_as"] = "65001";
    bgp_config["router_id"] = "192.168.1.1";
    bgp_config["neighbors"] = "192.168.1.2,10.0.0.2";
    router->configure_protocol(Protocol::BGP, bgp_config);
    
    // Configure traffic shaping
    auto traffic_shaper = router->get_traffic_shaper();
    std::vector<WFQClass> classes;
    
    WFQClass high_priority;
    high_priority.class_id = 1;
    high_priority.weight = 10;
    high_priority.min_bandwidth = 1000000;
    high_priority.max_bandwidth = 10000000;
    high_priority.name = "High Priority";
    high_priority.is_active = true;
    classes.push_back(high_priority);
    
    traffic_shaper->configure_wfq(classes);
    
    // Setup monitoring
    auto monitoring = std::make_unique<MonitoringSystem>();
    auto router_collector = std::make_shared<RouterMetricsCollector>(router);
    monitoring->register_collector(router_collector);
    monitoring->start();
    
    // Start router
    router->start();
    
    // Run for 5 minutes
    std::this_thread::sleep_for(std::chrono::minutes(5));
    
    // Get statistics
    auto stats = router->get_statistics();
    std::cout << "Packets processed: " << stats.packets_processed << std::endl;
    std::cout << "Routes: " << stats.routes_total << std::endl;
    
    // Stop router
    router->stop();
    monitoring->stop();
    
    return 0;
}
```

This comprehensive API documentation provides all the information needed to integrate with and extend the router simulator.
