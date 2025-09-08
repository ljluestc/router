# Multi-Protocol Router Simulator

A comprehensive cloud-native router simulation platform with FRR control-plane integration, traffic shaping, network impairments, and cloud networking concepts inspired by CloudPods and Aviatrix.

## 🌟 Features

### Core Router Simulation
- **FRR Control-Plane Integration**: Full BGP/OSPF/ISIS protocol support
- **Traffic Shaping**: Token-bucket and Weighted Fair Queuing (WFQ) algorithms
- **Network Impairments**: tc/netem integration for realistic network simulation
- **Multi-Protocol Support**: BGP, OSPF, ISIS routing protocols
- **Real-time Monitoring**: Comprehensive statistics and analytics

### Cloud Networking Integration
- **CloudPods Integration**: Multi-cloud resource management
- **Aviatrix Concepts**: Secure cloud networking and transit gateways
- **Terraform Providers**: Automated infrastructure as code
- **Service Mesh**: Istio integration for microservices networking

### Testing & Validation
- **Comprehensive Test Suite**: gtest-based testing framework
- **PCAP Diffing**: Network packet analysis and comparison
- **Coverage Reporting**: Code coverage analysis
- **Performance Benchmarks**: Throughput and latency testing

### Web Interface
- **Modern React UI**: Material-UI based dashboard
- **Real-time Monitoring**: Live statistics and visualizations
- **Cloud Management**: CloudPods and Aviatrix resource management
- **Scenario Configuration**: YAML-based test scenarios

## 🚀 Quick Start

### Prerequisites
- Ubuntu 20.04+ or CentOS 8+
- Docker and Docker Compose
- Node.js 18+ (for web interface)
- Go 1.19+ (for cloud integration)
- Rust 1.70+ (for performance components)

### Installation

1. **Clone the repository**
```bash
git clone https://github.com/your-username/router-simulator.git
cd router-simulator
```

2. **Install dependencies**
```bash
# Install system dependencies
sudo ./scripts/build_all.sh --deps

# Build all components
./scripts/build_all.sh --clean --test
```

3. **Start the services**
```bash
# Start router simulator
sudo ./build/router_sim -c scenarios/cloud_networking_demo.yaml

# Start web interface (in another terminal)
cd web
npm install
npm start
```

4. **Access the web interface**
Open http://localhost:3000 in your browser

## 📖 Usage

### Command Line Interface

```bash
# Start router with configuration
sudo ./build/router_sim -c config/router.yaml

# Run specific scenario
sudo ./build/router_sim -s scenarios/bgp_convergence.yaml

# Run tests
./build/router_tests

# CLI interface
./build/router_sim --interactive
```

### Web Interface

The web interface provides:
- **Dashboard**: System overview and real-time metrics
- **CloudPods**: Multi-cloud resource management
- **Aviatrix**: Secure cloud networking
- **Router Sim**: Protocol configuration and monitoring
- **Traffic Shaping**: QoS configuration
- **Network Impairments**: Network simulation
- **Analytics**: Performance analysis

### YAML Configuration

```yaml
# Example configuration
name: "Cloud Networking Demo"
description: "Multi-cloud router simulation"

global:
  router_id: "1.1.1.1"
  as_number: 65001

protocols:
  bgp:
    enabled: true
    neighbors:
      - address: "192.168.1.2"
        remote_as: 65002
  ospf:
    enabled: true
    area: "0.0.0.0"

traffic_shaping:
  enabled: true
  shapers:
    - name: "cloud_traffic"
      type: "wfq"
      total_bandwidth: "10000000000"

impairments:
  enabled: true
  interfaces:
    - name: "eth0"
      impairments:
        - type: "delay"
          value: 50.0
```

## 🏗️ Architecture

### Core Components

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   Web Interface │    │   CLI Interface │    │  YAML Config    │
└─────────┬───────┘    └─────────┬───────┘    └─────────┬───────┘
          │                      │                      │
          └──────────────────────┼──────────────────────┘
                                 │
                    ┌─────────────┴─────────────┐
                    │     Router Core           │
                    └─────────────┬─────────────┘
                                  │
        ┌─────────────────────────┼─────────────────────────┐
        │                         │                         │
┌───────▼────────┐    ┌──────────▼──────────┐    ┌────────▼────────┐
│ FRR Integration│    │  Traffic Shaping    │    │  Network        │
│ BGP/OSPF/ISIS  │    │  Token-Bucket/WFQ   │    │  Impairments    │
└────────────────┘    └─────────────────────┘    └─────────────────┘
        │                         │                         │
        └─────────────────────────┼─────────────────────────┘
                                  │
                    ┌─────────────┴─────────────┐
                    │   Cloud Integration       │
                    │ CloudPods + Aviatrix     │
                    └───────────────────────────┘
```

### Cloud Integration

- **CloudPods Client**: Multi-cloud resource management
- **Aviatrix Client**: Secure cloud networking
- **Terraform Providers**: Infrastructure as code
- **Service Mesh**: Microservices networking

## 🧪 Testing

### Running Tests

```bash
# Run all tests
make test

# Run specific test categories
./build/router_tests --gtest_filter="BGP*"
./build/router_tests --gtest_filter="TrafficShaping*"

# Run with coverage
make test-coverage
```

### Test Scenarios

- **BGP Convergence**: Route advertisement and convergence testing
- **OSPF Convergence**: Link-state database synchronization
- **Traffic Shaping**: QoS algorithm validation
- **Network Impairments**: Delay, loss, and jitter simulation
- **Performance**: Throughput and latency benchmarks

## 📊 Monitoring & Analytics

### Real-time Metrics
- Packet processing rate
- Throughput (Mbps)
- Latency (ms)
- Packet loss percentage
- CPU and memory usage

### Historical Data
- Performance trends
- Protocol statistics
- Network topology changes
- Error logs and alerts

## 🔧 Development

### Building from Source

```bash
# Install dependencies
sudo ./scripts/build_all.sh --deps

# Build in debug mode
./scripts/build_all.sh --debug --test

# Build with coverage
./scripts/build_all.sh --coverage

# Create distribution package
./scripts/build_all.sh --package
```

### Code Structure

```
router/
├── src/                    # C++ source code
│   ├── frr_integration/    # FRR protocol integration
│   ├── traffic_shaping/    # Traffic shaping algorithms
│   ├── network_impairments/ # Network simulation
│   ├── cloud_integration/  # Cloud provider clients
│   └── testing/           # Test framework
├── web/                   # React web interface
├── terraform/             # Terraform providers
├── scenarios/             # YAML test scenarios
├── tests/                 # Test cases
└── docs/                  # Documentation
```

### Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests for new functionality
5. Submit a pull request

## 📚 Documentation

- [API Documentation](docs/api.md)
- [Configuration Guide](docs/configuration.md)
- [Testing Guide](docs/testing.md)
- [Cloud Integration](docs/cloud-integration.md)
- [Terraform Providers](docs/terraform.md)

## 🌐 Cloud Networking Concepts

### CloudPods Integration
- Multi-cloud resource management
- Unified API across cloud providers
- Resource lifecycle management
- Cost optimization

### Aviatrix Concepts
- Secure cloud networking
- Transit gateways
- Spoke gateways
- VPN and peering connections
- Firewall and security policies

### Service Mesh
- Istio integration
- Traffic management
- Security policies
- Observability

## 🚀 Deployment

### Docker Deployment

```bash
# Build Docker image
docker build -t router-simulator .

# Run with Docker Compose
docker-compose up -d
```

### Kubernetes Deployment

```bash
# Apply Kubernetes manifests
kubectl apply -f k8s/
```

### GitHub Pages

The web interface is automatically deployed to GitHub Pages on every push to main branch.

## 📄 License

This project is licensed under the Apache License 2.0 - see the [LICENSE](LICENSE) file for details.

## 🤝 Contributing

Contributions are welcome! Please read our [Contributing Guide](CONTRIBUTING.md) for details on our code of conduct and the process for submitting pull requests.

## 📞 Support

- **Documentation**: [docs/](docs/)
- **Issues**: [GitHub Issues](https://github.com/your-username/router-simulator/issues)
- **Discussions**: [GitHub Discussions](https://github.com/your-username/router-simulator/discussions)

## 🙏 Acknowledgments

- [FRR](https://frrouting.org/) - Free Range Routing
- [CloudPods](https://github.com/yunionio/cloudpods) - Multi-cloud management platform
- [Aviatrix](https://www.aviatrix.com/) - Cloud networking platform
- [Material-UI](https://mui.com/) - React component library
- [Terraform](https://terraform.io/) - Infrastructure as code

---

**Multi-Protocol Router Simulator** - Bringing cloud networking concepts to network simulation and testing.