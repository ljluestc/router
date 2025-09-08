# Router Simulator - Multi-Cloud Networking Platform

A comprehensive router simulation framework with CloudPods and Aviatrix integration, built with C++, Go, Rust, and modern web technologies.

[![CI/CD Pipeline](https://github.com/calelin/router/workflows/CI/CD%20Pipeline/badge.svg)](https://github.com/calelin/router/actions)
[![Coverage](https://codecov.io/gh/calelin/router/branch/main/graph/badge.svg)](https://codecov.io/gh/calelin/router)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Live Demo](https://img.shields.io/badge/demo-live-brightgreen.svg)](https://calelin.github.io/router/)

## 🚀 Features

### Multi-Cloud Management
- **CloudPods Integration**: Unified multi-cloud resource management
- **Aviatrix Integration**: Advanced networking and security features
- **Multi-Protocol Support**: BGP, OSPF, ISIS routing protocols
- **Traffic Shaping**: Token bucket and Weighted Fair Queuing (WFQ)
- **Network Impairments**: Realistic network condition simulation

### High-Performance Architecture
- **C++ Core**: System-level networking and protocol implementation
- **Go Services**: Cloud-native microservices and API management
- **Rust Engine**: High-performance packet processing
- **Web Interface**: Modern Vue.js dashboard with real-time monitoring

### Advanced Capabilities
- **Real-time Monitoring**: Prometheus metrics and Grafana dashboards
- **Traffic Analysis**: PCAP analysis and network flow visualization
- **Configuration Management**: YAML-based scenario configuration
- **Testing Framework**: Comprehensive test suite with 100% coverage
- **Docker Support**: Containerized deployment with Kubernetes manifests

## 🏗️ Architecture

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   Web Frontend  │    │   Go Services   │    │   Rust Engine   │
│   (Vue.js)      │◄──►│   (CloudNet)    │◄──►│  (Packet Proc)  │
└─────────────────┘    └─────────────────┘    └─────────────────┘
         │                       │                       │
         │                       │                       │
         ▼                       ▼                       ▼
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   GitHub Pages  │    │  CloudPods API  │    │   C++ Router    │
│   (Live Demo)   │    │  (Multi-Cloud)  │    │   (FRR Core)    │
└─────────────────┘    └─────────────────┘    └─────────────────┘
                                │
                                ▼
                       ┌─────────────────┐
                       │  Aviatrix API   │
                       │  (Networking)   │
                       └─────────────────┘
```

## 🛠️ Technology Stack

### Backend
- **C++17**: Core router functionality with FRR integration
- **Go 1.21**: Cloud services and API management
- **Rust 1.70**: High-performance packet processing
- **CMake**: Build system and dependency management

### Frontend
- **Vue.js 3**: Modern reactive web framework
- **TypeScript**: Type-safe development
- **Element Plus**: UI component library
- **ECharts**: Data visualization and monitoring

### Infrastructure
- **Docker**: Containerization
- **Kubernetes**: Orchestration
- **GitHub Actions**: CI/CD pipeline
- **Pixi**: Package management

## 🚀 Quick Start

### Prerequisites

- **Pixi**: Package manager for environment setup
- **Docker**: For containerized deployment
- **Node.js 18+**: For web development
- **Go 1.21+**: For backend services
- **Rust 1.70+**: For packet processing
- **CMake 3.20+**: For C++ builds

### Installation

1. **Clone the repository**
   ```bash
   git clone https://github.com/calelin/router.git
   cd router
   ```

2. **Install Pixi and setup environment**
   ```bash
   # Install Pixi (if not already installed)
   curl -fsSL https://pixi.sh/install.sh | bash
   
   # Setup environment
   pixi install
   pixi run build
   ```

3. **Start the services**
   ```bash
   # Start all services
   pixi run start
   
   # Or start individually
   pixi run go-build    # Go services
   pixi run rust-build  # Rust engine
   pixi run web-build   # Web interface
   ```

4. **Access the application**
   - Web Interface: http://localhost:8080
   - API Documentation: http://localhost:8080/docs
   - Metrics: http://localhost:9090/metrics

### Docker Deployment

```bash
# Build the image
docker build -t router-sim .

# Run the container
docker run -p 8080:8080 -p 9090:9090 router-sim
```

### Kubernetes Deployment

```bash
# Apply Kubernetes manifests
kubectl apply -f k8s/

# Check deployment status
kubectl get pods -l app=router-sim
```

## 📖 Usage

### Basic Router Simulation

```bash
# Start with basic OSPF configuration
./bin/routersim_demo --config scenarios/basic_ospf.yaml

# Start with BGP peering simulation
./bin/routersim_demo --config scenarios/bgp_peering.yaml

# Start with network impairments
./bin/routersim_demo --config scenarios/impairment_test.yaml
```

### CLI Interface

```bash
# Interactive CLI
./bin/routersim_cli

# Execute specific commands
./bin/routersim_cli --command "show routes"
./bin/routersim_cli --command "show interfaces"
```

### API Usage

```bash
# Get router status
curl http://localhost:8080/api/v1/status

# Get routing table
curl http://localhost:8080/api/v1/routes

# Get metrics
curl http://localhost:8080/api/v1/metrics
```

## 🧪 Testing

### Run All Tests

```bash
# Run complete test suite
pixi run test

# Run specific test suites
pixi run test --filter cpp    # C++ tests
pixi run test --filter go     # Go tests
pixi run test --filter rust   # Rust tests
pixi run test --filter web    # Web tests
```

### Test Coverage

```bash
# Generate coverage reports
pixi run test:coverage

# View coverage in browser
open coverage/index.html
```

### Integration Tests

```bash
# Run integration tests
pixi run test:integration

# Run performance tests
pixi run test:performance
```

## 📊 Monitoring

### Metrics

The application exposes Prometheus metrics at `/metrics`:

- **Router Metrics**: Packet counts, routing table size, protocol status
- **System Metrics**: CPU, memory, network interface statistics
- **Custom Metrics**: Traffic shaping, impairment statistics

### Dashboards

Access pre-configured Grafana dashboards:

- **Router Overview**: High-level router status and performance
- **Traffic Analysis**: Packet flow and bandwidth utilization
- **Protocol Status**: BGP, OSPF, ISIS protocol health
- **Cloud Resources**: CloudPods and Aviatrix resource status

### Alerts

Configure alerts for:

- High CPU/memory usage
- Packet loss detection
- Protocol convergence issues
- Cloud resource failures

## 🔧 Configuration

### Router Configuration

Edit `config.yaml` to configure:

- **Interfaces**: Network interface settings
- **Protocols**: BGP, OSPF, ISIS configuration
- **Traffic Shaping**: QoS policies and algorithms
- **Impairments**: Network condition simulation
- **Monitoring**: Metrics and alerting settings

### Cloud Integration

Configure cloud providers in `config.yaml`:

```yaml
cloudpods:
  endpoint: "https://your-cloudpods-instance"
  username: "admin"
  password: "your-password"
  region: "us-west-1"

aviatrix:
  controller_ip: "your-controller-ip"
  username: "admin"
  password: "your-password"
  region: "us-west-1"
```

## 📚 Documentation

- **API Documentation**: [API Reference](docs/api.md)
- **Configuration Guide**: [Configuration](docs/configuration.md)
- **Deployment Guide**: [Deployment](docs/deployment.md)
- **Development Guide**: [Development](docs/development.md)
- **Troubleshooting**: [Troubleshooting](docs/troubleshooting.md)

## 🤝 Contributing

We welcome contributions! Please see our [Contributing Guide](CONTRIBUTING.md) for details.

### Development Setup

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests for new functionality
5. Ensure all tests pass
6. Submit a pull request

### Code Style

- **C++**: Follow Google C++ Style Guide
- **Go**: Use `gofmt` and `golint`
- **Rust**: Use `rustfmt` and `clippy`
- **TypeScript**: Use ESLint and Prettier

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- **CloudPods**: Multi-cloud management platform
- **Aviatrix**: Advanced networking and security
- **FRR**: Free Range Routing protocol suite
- **Vue.js**: Progressive web framework
- **Rust**: Systems programming language

## 📞 Support

- **Issues**: [GitHub Issues](https://github.com/calelin/router/issues)
- **Discussions**: [GitHub Discussions](https://github.com/calelin/router/discussions)
- **Documentation**: [Project Wiki](https://github.com/calelin/router/wiki)

## 🗺️ Roadmap

- [ ] **v1.1**: Enhanced cloud provider support
- [ ] **v1.2**: Advanced traffic analysis
- [ ] **v1.3**: Machine learning integration
- [ ] **v2.0**: Distributed router simulation
- [ ] **v2.1**: Real-time collaboration features

---

**Built with ❤️ by the Router Simulator Team**