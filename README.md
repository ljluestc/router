# Multi-Protocol Router Simulator

A comprehensive network simulation platform that integrates FRR routing protocols (BGP/OSPF/ISIS), traffic shaping algorithms, network impairments, and cloud networking concepts with CloudPods and Aviatrix integration.

## 🚀 Features

### Core Router Simulation
- **FRR Integration**: Full control-plane integration with BGP, OSPF, and IS-IS protocols
- **Traffic Shaping**: Token-bucket and Weighted Fair Queue (WFQ) algorithms
- **Network Impairments**: tc/netem integration for realistic network simulation
- **Multi-Protocol Support**: BGP, OSPF, IS-IS with real-time convergence analysis

### Cloud Networking Integration
- **CloudPods Integration**: Native support for CloudPods cloud platform
- **Aviatrix Integration**: Multi-cloud networking with Aviatrix gateways
- **Terraform Automation**: Automatic infrastructure provisioning
- **Real-time Monitoring**: Live metrics and analytics

### Advanced Analytics
- **ClickHouse Analytics**: High-performance time-series database
- **Real-time Dashboards**: Live network topology and metrics
- **Protocol Analysis**: Convergence time analysis and optimization
- **Traffic Flow Analysis**: Deep packet inspection and flow analysis

### Testing & Validation
- **Comprehensive Testing**: gtest coverage with pcap diffing
- **Scenario Management**: YAML-based test scenarios
- **Regression Testing**: Automated test suites
- **Performance Benchmarking**: High-throughput packet processing

## 🏗️ Architecture

### C++ Core Engine
- **Router Core**: High-performance packet processing engine
- **Protocol Implementations**: BGP, OSPF, IS-IS with FRR integration
- **Traffic Shaping**: Token-bucket and WFQ algorithms
- **Network Impairments**: tc/netem integration

### Rust Analytics Engine
- **ClickHouse Client**: High-performance analytics database
- **Real-time Processing**: Stream processing for live metrics
- **Protocol Analysis**: Advanced routing protocol analytics

### Go Microservices
- **CloudPods Client**: CloudPods platform integration
- **Aviatrix Client**: Multi-cloud networking integration
- **API Gateway**: RESTful API for web interface
- **WebSocket Support**: Real-time data streaming

### React Web Interface
- **Interactive Topology**: Real-time network visualization
- **Analytics Dashboard**: Comprehensive metrics and charts
- **Scenario Management**: YAML-based configuration
- **Real-time Monitoring**: Live updates via WebSocket

## 📦 Installation

### Prerequisites
- C++17 compiler (GCC 9+ or Clang 10+)
- CMake 3.16+
- Rust 1.70+
- Go 1.19+
- Node.js 18+
- FRR routing suite
- ClickHouse database
- tc/netem tools

### Build Instructions

1. **Clone the repository**
```bash
git clone https://github.com/your-username/router-sim.git
cd router-sim
```

2. **Build C++ components**
```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

3. **Build Rust components**
```bash
cd rust
cargo build --release
```

4. **Build Go microservices**
```bash
cd go
go mod download
go build -o bin/server ./cmd/server
```

5. **Build web interface**
```bash
cd web
npm install
npm run build
```

### Docker Deployment
```bash
docker-compose up -d
```

## 🚀 Quick Start

### 1. Start the Router Simulator
```bash
./build/router_sim --config config.yaml
```

### 2. Launch Web Interface
```bash
cd web && npm start
```

### 3. Access the Dashboard
Open your browser to `http://localhost:3000`

### 4. Run a Simulation
```bash
# Load a scenario
./build/router_sim --scenario scenarios/bgp_convergence.yaml

# Start simulation
./build/router_sim --start --scenario scenarios/high_latency_test.yaml
```

## 📊 Usage Examples

### BGP Convergence Testing
```yaml
# scenarios/bgp_convergence.yaml
scenarios:
  bgp_convergence:
    description: "BGP convergence test with multiple ASes"
    router:
      hostname: "bgp-router-1"
      router_id: "1.1.1.1"
    protocols:
      bgp:
        enabled: true
        parameters:
          local_as: "65001"
          neighbors: "10.0.0.2,10.0.0.3"
    traffic_shaping:
      algorithm: "weighted_fair_queue"
    netem:
      - interface: "eth1"
        delay:
          delay_ms: 50
          jitter_ms: 5
```

### Traffic Shaping Analysis
```bash
# Configure token bucket
./build/router_sim --traffic-shaping token-bucket --rate 100Mbps --burst 50MB

# Configure WFQ
./build/router_sim --traffic-shaping wfq --classes high:10,medium:5,low:1
```

### Network Impairment Simulation
```bash
# Add delay and packet loss
./build/router_sim --netem delay 100ms --netem loss 2%

# Apply congestion scenario
./build/router_sim --scenario scenarios/congested_network_test.yaml
```

## 🔧 Configuration

### Router Configuration
```yaml
# config.yaml
router:
  hostname: "router-sim"
  router_id: "1.1.1.1"
  log_level: "info"
  enable_ipv6: false
  enable_mpls: false

interfaces:
  - name: "eth0"
    ip_address: "192.168.1.1"
    subnet_mask: "255.255.255.0"
    mtu: 1500
    enabled: true

protocols:
  bgp:
    enabled: true
    parameters:
      local_as: "65001"
      router_id: "1.1.1.1"
      neighbors: "10.0.0.2"
      hold_time: "180"
      keepalive_interval: "60"

traffic_shaping:
  algorithm: "weighted_fair_queue"
  wfq_classes:
    - class_id: 1
      weight: 10
      min_bandwidth: 1000000
      max_bandwidth: 10000000
      name: "High Priority"
```

### CloudPods Integration
```yaml
cloudpods:
  base_url: "https://api.cloudpods.example.com"
  api_key: "your-api-key"
  timeout_seconds: 30
```

### Aviatrix Integration
```yaml
aviatrix:
  base_url: "https://api.aviatrix.example.com"
  api_key: "your-api-key"
  timeout_seconds: 30
```

## 📈 Analytics & Monitoring

### Real-time Metrics
- **Bandwidth Utilization**: Live throughput monitoring
- **Latency Analysis**: End-to-end delay measurement
- **Packet Loss**: Real-time loss rate tracking
- **Protocol Convergence**: BGP/OSPF/ISIS convergence times

### ClickHouse Analytics
```sql
-- Top bandwidth consumers
SELECT src_ip, dst_ip, SUM(bytes) as total_bytes
FROM packet_flows
WHERE timestamp >= now() - INTERVAL 1 HOUR
GROUP BY src_ip, dst_ip
ORDER BY total_bytes DESC
LIMIT 10;

-- BGP convergence analysis
SELECT neighbor_ip, AVG(convergence_time) as avg_convergence
FROM bgp_updates
WHERE timestamp >= now() - INTERVAL 1 DAY
GROUP BY neighbor_ip;
```

## 🧪 Testing

### Run Test Suite
```bash
# C++ tests
cd build && make test

# Rust tests
cd rust && cargo test

# Go tests
cd go && go test ./...

# Web tests
cd web && npm test
```

### Coverage Analysis
```bash
# Generate coverage report
cd build && make coverage
open coverage/index.html
```

### PCAP Analysis
```bash
# Compare PCAP files
./build/router_sim --compare pcap1.pcap pcap2.pcap --output diff_report.txt
```

## 🌐 Web Interface

### Dashboard Features
- **Real-time Topology**: Interactive network visualization
- **Live Metrics**: Real-time performance monitoring
- **Scenario Management**: YAML-based configuration
- **Analytics Charts**: Comprehensive data visualization

### API Endpoints
- `GET /api/v1/router/status` - Router status
- `GET /api/v1/router/routes` - Routing table
- `POST /api/v1/router/scenario` - Load scenario
- `GET /api/v1/analytics/metrics` - Get metrics
- `WebSocket /ws` - Real-time updates

## 🚀 Deployment

### GitHub Pages
The web interface is automatically deployed to GitHub Pages on every push to main.

### Docker Compose
```yaml
version: '3.8'
services:
  router-sim:
    build: .
    ports:
      - "8080:8080"
    environment:
      - CLICKHOUSE_URL=clickhouse://localhost:9000
      - CLOUDPODS_API_KEY=your-key
      - AVIATRIX_API_KEY=your-key

  clickhouse:
    image: clickhouse/clickhouse-server
    ports:
      - "9000:9000"
    volumes:
      - clickhouse_data:/var/lib/clickhouse

  web:
    build: ./web
    ports:
      - "3000:3000"
    depends_on:
      - router-sim
```

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- [FRR](https://frrouting.org/) - Free Range Routing
- [ClickHouse](https://clickhouse.com/) - Analytics database
- [CloudPods](https://deepwiki.com/yunionio/cloudpods) - Cloud platform
- [Aviatrix](https://docs.aviatrix.com/) - Multi-cloud networking
- [React Flow](https://reactflow.dev/) - Network visualization
- [Material-UI](https://mui.com/) - React components

## 📞 Support

- **Documentation**: [Wiki](https://github.com/your-username/router-sim/wiki)
- **Issues**: [GitHub Issues](https://github.com/your-username/router-sim/issues)
- **Discussions**: [GitHub Discussions](https://github.com/your-username/router-sim/discussions)

## 🎯 Roadmap

- [ ] MPLS support
- [ ] IPv6 full support
- [ ] SDN integration
- [ ] Machine learning optimization
- [ ] Kubernetes deployment
- [ ] Advanced visualization features
- [ ] Multi-tenant support
- [ ] API rate limiting
- [ ] Advanced security features

---

**Built with ❤️ for the networking community**