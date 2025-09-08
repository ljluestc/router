#!/usr/bin/env python3
"""
Documentation generation script for Router Simulator with DeepWiki integration
"""

import os
import sys
import json
import yaml
import requests
import subprocess
from pathlib import Path
from typing import Dict, List, Any
import argparse
import logging

# Configure logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

class DocumentationGenerator:
    def __init__(self, project_root: str):
        self.project_root = Path(project_root)
        self.docs_dir = self.project_root / "docs"
        self.docs_dir.mkdir(exist_ok=True)
        
        # DeepWiki configuration
        self.deepwiki_config = {
            "base_url": "https://deepwiki.com",
            "project": "yunionio/cloudpods",
            "api_key": os.getenv("DEEPWIKI_API_KEY", ""),
        }
        
        # Documentation structure
        self.doc_structure = {
            "overview": {
                "title": "Router Simulator Overview",
                "sections": [
                    "introduction",
                    "architecture",
                    "features",
                    "cloudpods_integration",
                    "getting_started"
                ]
            },
            "api": {
                "title": "API Reference",
                "sections": [
                    "c_api",
                    "rust_api",
                    "go_api",
                    "rest_api"
                ]
            },
            "cloudpods": {
                "title": "CloudPods Integration",
                "sections": [
                    "vpc_routing",
                    "nat_gateway",
                    "load_balancer",
                    "service_mesh",
                    "networking_concepts"
                ]
            },
            "development": {
                "title": "Development Guide",
                "sections": [
                    "building",
                    "testing",
                    "contributing",
                    "troubleshooting"
                ]
            }
        }

    def generate_overview_docs(self) -> str:
        """Generate overview documentation"""
        content = """# Router Simulator Overview

## Introduction

The Router Simulator is a comprehensive multi-protocol routing simulation system designed for testing and validating network routing protocols in cloud environments. It integrates with CloudPods networking concepts and provides high-performance analytics using ClickHouse and Rust.

## Architecture

The system is built with a modular architecture:

- **C++ Core**: High-performance packet processing and protocol implementation
- **Rust Analytics Engine**: Real-time data processing and analytics
- **Go API Server**: Web interface and orchestration
- **ClickHouse**: Time-series data storage and analytics
- **React Web UI**: Modern web interface for visualization

## Features

### Multi-Protocol Support
- BGP (Border Gateway Protocol)
- OSPF (Open Shortest Path First)
- IS-IS (Intermediate System to Intermediate System)

### Traffic Shaping
- Token Bucket Algorithm
- Weighted Fair Queuing (WFQ)
- Traffic Control (tc) integration

### Network Impairments
- Latency simulation
- Packet loss
- Jitter
- Bandwidth limiting

### CloudPods Integration
- VPC Routing simulation
- NAT Gateway functionality
- Load Balancer integration
- Service Mesh routing

### Analytics & Monitoring
- Real-time metrics collection
- ClickHouse integration
- Prometheus metrics
- Grafana dashboards

## CloudPods Integration

The Router Simulator integrates with CloudPods networking concepts:

### VPC Routing
- Virtual Private Cloud routing simulation
- Subnet management
- Route table configuration
- Inter-VPC connectivity

### NAT Gateway
- Network Address Translation
- Internet gateway functionality
- Elastic IP management
- Port forwarding

### Load Balancer
- Application Load Balancer (ALB)
- Network Load Balancer (NLB)
- Target group management
- Health checks

### Service Mesh
- Service discovery
- Load balancing
- Traffic management
- Security policies

## Getting Started

### Prerequisites
- CMake 3.20+
- C++20 compiler
- Rust 1.70+
- Go 1.21+
- Node.js 18+
- ClickHouse
- FRR (FRRouting)

### Building
```bash
# Clone the repository
git clone <repository-url>
cd router-simulator

# Build with CMake
mkdir build && cd build
cmake ..
make -j$(nproc)

# Build Rust components
cd ../rust
cargo build --release

# Build Go API server
cd ../go
go build -o api-server ./cmd/server

# Build web interface
cd ../web
npm install
npm run build
```

### Running
```bash
# Start all services with Docker Compose
docker-compose up -d

# Or run individual components
./build/router_sim --config config/router.yaml
./go/api-server
cd web && npm start
```

## Configuration

The system uses YAML configuration files:

```yaml
# config/router.yaml
router:
  id: "router-001"
  interfaces:
    - name: "eth0"
      ip: "192.168.1.1/24"
  protocols:
    bgp:
      enabled: true
      as_number: 65001
    ospf:
      enabled: true
      area: "0.0.0.0"
  traffic_shaping:
    enabled: true
    algorithms: ["token_bucket", "wfq"]
  analytics:
    clickhouse:
      host: "localhost"
      port: 9000
      database: "router_analytics"
```

## Testing

The system includes comprehensive testing:

```bash
# Run unit tests
make test

# Run integration tests
./build/test_router

# Run performance benchmarks
./build/router_sim --benchmark

# Run pcap diffing tests
./scripts/run_pcap_tests.sh
```

## Monitoring

Access monitoring interfaces:
- Grafana: http://localhost:3000
- Prometheus: http://localhost:9090
- Web UI: http://localhost:3001
- API: http://localhost:8080
"""
        return content

    def generate_api_docs(self) -> str:
        """Generate API documentation"""
        content = """# API Reference

## C++ API

### Core Classes

#### RouterCore
```cpp
class RouterCore {
public:
    bool initialize(const std::string& config_file);
    void start();
    void stop();
    bool is_running() const;
};
```

#### Traffic Shaping
```cpp
class TokenBucket {
public:
    TokenBucket(uint32_t rate, uint32_t burst);
    bool consume(uint32_t tokens);
    void refill();
};

class WFQScheduler {
public:
    void addFlow(const std::string& flow_id, uint32_t weight);
    std::string scheduleNext();
};
```

#### Network Impairments
```cpp
class NetEmImpairments {
public:
    void setLatency(uint32_t latency_ms);
    void setPacketLoss(double loss_rate);
    void setJitter(uint32_t jitter_ms);
    void setBandwidth(uint32_t bandwidth_kbps);
};
```

## Rust API

### Analytics Engine
```rust
pub struct RouterAnalytics {
    packet_engine: Arc<PacketEngine>,
    routing_table: Arc<RwLock<RoutingTable>>,
    analytics_engine: Arc<AnalyticsEngine>,
    cloudpods: Arc<CloudPodsIntegration>,
    metrics: Arc<MetricsCollector>,
}

impl RouterAnalytics {
    pub async fn start(&self) -> Result<()>;
    pub async fn process_packet(&self, packet: &[u8]) -> Result<()>;
    pub async fn get_routing_stats(&self) -> Result<RoutingStats>;
    pub async fn get_traffic_stats(&self) -> Result<TrafficStats>;
}
```

### CloudPods Integration
```rust
pub struct CloudPodsIntegration {
    config: CloudPodsConfig,
    vpc_count: u32,
    nat_gateway_count: u32,
    load_balancer_count: u32,
    service_mesh_count: u32,
}

impl CloudPodsIntegration {
    pub async fn get_vpc_stats(&self) -> Result<VPCRoutingStats>;
    pub async fn create_subnet(&self, config: SubnetConfig) -> Result<String>;
    pub async fn create_nat_gateway(&self, config: NATConfig) -> Result<String>;
}
```

## Go API

### HTTP Server
```go
type Server struct {
    config        *config.Config
    cloudpods     *cloudpods.Client
    analytics     *analytics.Engine
    router        *gin.Engine
}

func (s *Server) SetupRoutes() {
    s.router.GET("/api/v1/status", s.GetStatus)
    s.router.GET("/api/v1/routing/stats", s.GetRoutingStats)
    s.router.POST("/api/v1/cloudpods/vpc", s.CreateVPC)
    s.router.GET("/api/v1/cloudpods/vpc/:id/stats", s.GetVPCStats)
}
```

### CloudPods Client
```go
type Client struct {
    endpoint string
    username string
    password string
    client   *http.Client
}

func (c *Client) GetVPCStats(vpcID string) (*VPCRoutingStats, error) {
    // Implementation
}

func (c *Client) CreateSubnet(vpcID string, config SubnetConfig) (*SubnetInfo, error) {
    // Implementation
}
```

## REST API

### Endpoints

#### Status
- `GET /api/v1/status` - Get system status
- `GET /api/v1/health` - Health check

#### Routing
- `GET /api/v1/routing/stats` - Get routing statistics
- `GET /api/v1/routing/routes` - Get routing table
- `POST /api/v1/routing/routes` - Add route

#### CloudPods
- `GET /api/v1/cloudpods/vpc` - List VPCs
- `POST /api/v1/cloudpods/vpc` - Create VPC
- `GET /api/v1/cloudpods/vpc/:id/stats` - Get VPC stats
- `POST /api/v1/cloudpods/vpc/:id/subnets` - Create subnet
- `POST /api/v1/cloudpods/vpc/:id/nat-gateways` - Create NAT gateway
- `POST /api/v1/cloudpods/vpc/:id/load-balancers` - Create load balancer
- `POST /api/v1/cloudpods/vpc/:id/service-mesh` - Create service mesh route

#### Analytics
- `GET /api/v1/analytics/traffic` - Get traffic statistics
- `GET /api/v1/analytics/performance` - Get performance metrics
- `GET /api/v1/analytics/routing` - Get routing analytics

### Response Format
```json
{
  "status": "success",
  "data": {
    "vpc_id": "vpc-12345678",
    "subnet_count": 3,
    "nat_gateway_count": 1,
    "load_balancer_count": 2,
    "service_mesh_routes": 5
  },
  "timestamp": "2024-01-01T00:00:00Z"
}
```
"""
        return content

    def generate_cloudpods_docs(self) -> str:
        """Generate CloudPods integration documentation"""
        content = """# CloudPods Integration

## Overview

The Router Simulator integrates with CloudPods networking concepts to provide comprehensive cloud networking simulation capabilities.

## VPC Routing

### Concepts
Virtual Private Cloud (VPC) routing simulates cloud networking environments with:
- Private IP address spaces
- Subnet segmentation
- Route table management
- Internet gateway connectivity

### Implementation
```cpp
class VPCRouting {
public:
    bool addSubnet(const std::string& subnet_id, 
                   const std::string& cidr, 
                   const std::string& availability_zone);
    
    bool addNATGateway(const std::string& nat_id, 
                      const std::string& subnet_id,
                      const std::string& elastic_ip);
    
    std::string routePacket(const std::string& src_ip, 
                           const std::string& dst_ip,
                           const std::string& protocol, 
                           uint16_t port);
};
```

### Configuration
```yaml
vpc:
  id: "vpc-12345678"
  cidr_block: "10.0.0.0/16"
  region: "us-west-2"
  subnets:
    - id: "subnet-12345678"
      cidr: "10.0.1.0/24"
      availability_zone: "us-west-2a"
    - id: "subnet-87654321"
      cidr: "10.0.2.0/24"
      availability_zone: "us-west-2b"
```

## NAT Gateway

### Concepts
Network Address Translation (NAT) Gateway provides:
- Outbound internet connectivity
- Elastic IP management
- Port forwarding
- Security group integration

### Implementation
```cpp
class NATGateway {
public:
    bool create(const std::string& nat_id, 
                const std::string& subnet_id,
                const std::string& elastic_ip);
    
    bool delete(const std::string& nat_id);
    
    std::string translate(const std::string& private_ip, 
                         uint16_t private_port,
                         const std::string& public_ip, 
                         uint16_t public_port);
};
```

### Configuration
```yaml
nat_gateways:
  - id: "nat-12345678"
    subnet_id: "subnet-12345678"
    elastic_ip: "203.0.113.1"
    state: "available"
```

## Load Balancer

### Concepts
Load Balancer provides:
- Application Load Balancer (ALB)
- Network Load Balancer (NLB)
- Target group management
- Health checks
- SSL termination

### Implementation
```cpp
class LoadBalancer {
public:
    bool create(const std::string& lb_id, 
                const std::string& subnet_id,
                const std::vector<std::string>& target_groups);
    
    bool addTarget(const std::string& lb_id, 
                   const std::string& target_group,
                   const std::string& target_ip, 
                   uint16_t port);
    
    std::string routeRequest(const std::string& lb_id, 
                            const std::string& request);
};
```

### Configuration
```yaml
load_balancers:
  - id: "lb-12345678"
    type: "application"
    subnet_id: "subnet-12345678"
    target_groups:
      - "tg-12345678"
      - "tg-87654321"
    listeners:
      - port: 80
        protocol: "HTTP"
      - port: 443
        protocol: "HTTPS"
        ssl_certificate: "arn:aws:acm:us-west-2:123456789012:certificate/12345678-1234-1234-1234-123456789012"
```

## Service Mesh

### Concepts
Service Mesh provides:
- Service discovery
- Load balancing
- Traffic management
- Security policies
- Observability

### Implementation
```cpp
class ServiceMesh {
public:
    bool registerService(const std::string& service_name, 
                        const std::string& service_ip,
                        const std::vector<std::string>& endpoints);
    
    bool unregisterService(const std::string& service_name);
    
    std::string resolveService(const std::string& service_name);
    
    bool setTrafficPolicy(const std::string& service_name, 
                         const TrafficPolicy& policy);
};
```

### Configuration
```yaml
service_mesh:
  services:
    - name: "frontend"
      ip: "10.0.1.10"
      endpoints:
        - "10.0.1.10:8080"
        - "10.0.1.11:8080"
      policy:
        load_balancing: "round_robin"
        health_check:
          enabled: true
          interval: 30s
          timeout: 5s
    - name: "backend"
      ip: "10.0.2.10"
      endpoints:
        - "10.0.2.10:9090"
        - "10.0.2.11:9090"
      policy:
        load_balancing: "least_connections"
        circuit_breaker:
          enabled: true
          failure_threshold: 5
          timeout: 60s
```

## Networking Concepts

### IP Addressing
- Private IP ranges: 10.0.0.0/8, 172.16.0.0/12, 192.168.0.0/16
- Public IP allocation
- Elastic IP management
- IPv6 support

### Routing
- Route table management
- CIDR block routing
- Longest prefix matching
- Route propagation

### Security
- Security groups
- Network ACLs
- VPC endpoints
- PrivateLink

### Monitoring
- Flow logs
- VPC metrics
- Network performance insights
- Cost optimization

## Integration Examples

### Creating a VPC
```bash
# Create VPC
curl -X POST http://localhost:8080/api/v1/cloudpods/vpc \
  -H "Content-Type: application/json" \
  -d '{
    "cidr_block": "10.0.0.0/16",
    "region": "us-west-2"
  }'

# Create subnets
curl -X POST http://localhost:8080/api/v1/cloudpods/vpc/vpc-12345678/subnets \
  -H "Content-Type: application/json" \
  -d '{
    "subnet_id": "subnet-12345678",
    "cidr": "10.0.1.0/24",
    "availability_zone": "us-west-2a"
  }'

# Create NAT gateway
curl -X POST http://localhost:8080/api/v1/cloudpods/vpc/vpc-12345678/nat-gateways \
  -H "Content-Type: application/json" \
  -d '{
    "nat_id": "nat-12345678",
    "subnet_id": "subnet-12345678",
    "elastic_ip": "203.0.113.1"
  }'
```

### Service Mesh Setup
```bash
# Register service
curl -X POST http://localhost:8080/api/v1/cloudpods/vpc/vpc-12345678/service-mesh \
  -H "Content-Type: application/json" \
  -d '{
    "service_name": "frontend",
    "service_ip": "10.0.1.10",
    "endpoints": ["10.0.1.10:8080", "10.0.1.11:8080"]
  }'

# Set traffic policy
curl -X PUT http://localhost:8080/api/v1/cloudpods/vpc/vpc-12345678/service-mesh/frontend/policy \
  -H "Content-Type: application/json" \
  -d '{
    "load_balancing": "round_robin",
    "health_check": {
      "enabled": true,
      "interval": "30s",
      "timeout": "5s"
    }
  }'
```
"""
        return content

    def generate_development_docs(self) -> str:
        """Generate development documentation"""
        content = """# Development Guide

## Building

### Prerequisites
- CMake 3.20+
- C++20 compiler (GCC 11+, Clang 14+, MSVC 2022+)
- Rust 1.70+
- Go 1.21+
- Node.js 18+
- ClickHouse
- FRR (FRRouting)

### Dependencies

#### Ubuntu/Debian
```bash
sudo apt update
sudo apt install -y cmake build-essential pkg-config libpcap-dev libyaml-cpp-dev libssl-dev
sudo apt install -y frr-dev frr-pythontools
sudo apt install -y clickhouse-client clickhouse-server
sudo apt install -y golang-go nodejs npm
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
```

#### CentOS/RHEL
```bash
sudo yum install -y cmake gcc-c++ pkgconfig libpcap-devel yaml-cpp-devel openssl-devel
sudo yum install -y frr-devel
sudo yum install -y clickhouse-client clickhouse-server
sudo yum install -y golang nodejs npm
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
```

### Build Process

#### C++ Components
```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

#### Rust Components
```bash
cd rust
cargo build --release
```

#### Go Components
```bash
cd go
go mod tidy
go build -o api-server ./cmd/server
```

#### Web Interface
```bash
cd web
npm install
npm run build
```

### Docker Build
```bash
# Build all components
docker-compose build

# Build specific component
docker-compose build router-core
docker-compose build analytics-engine
docker-compose build api-server
docker-compose build web-ui
```

## Testing

### Unit Tests
```bash
# C++ tests
cd build
make test

# Rust tests
cd rust
cargo test

# Go tests
cd go
go test ./...

# Web tests
cd web
npm test
```

### Integration Tests
```bash
# Run integration test suite
./scripts/run_integration_tests.sh

# Run specific test category
./scripts/run_integration_tests.sh --category routing
./scripts/run_integration_tests.sh --category cloudpods
./scripts/run_integration_tests.sh --category analytics
```

### Performance Tests
```bash
# Run benchmarks
./build/router_sim --benchmark

# Run load tests
./scripts/run_load_tests.sh

# Run stress tests
./scripts/run_stress_tests.sh
```

### PCAP Diffing Tests
```bash
# Run pcap diffing tests
./scripts/run_pcap_tests.sh

# Compare specific scenarios
./scripts/run_pcap_tests.sh --scenario bgp_convergence
./scripts/run_pcap_tests.sh --scenario ospf_hello
```

## Contributing

### Code Style

#### C++
- Follow Google C++ Style Guide
- Use clang-format for formatting
- Maximum line length: 100 characters
- Use const whenever possible

#### Rust
- Follow Rust API Guidelines
- Use rustfmt for formatting
- Use clippy for linting
- Document all public APIs

#### Go
- Follow Go Code Review Comments
- Use gofmt for formatting
- Use golint for linting
- Document all exported functions

#### TypeScript/React
- Follow Airbnb TypeScript Style Guide
- Use Prettier for formatting
- Use ESLint for linting
- Use TypeScript strict mode

### Commit Messages
Follow conventional commits format:
```
type(scope): description

[optional body]

[optional footer]
```

Types:
- feat: New feature
- fix: Bug fix
- docs: Documentation changes
- style: Code style changes
- refactor: Code refactoring
- test: Test changes
- chore: Maintenance tasks

### Pull Request Process
1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests for new functionality
5. Ensure all tests pass
6. Update documentation
7. Submit a pull request

### Code Review
- All code must be reviewed before merging
- At least one approval required
- All CI checks must pass
- No merge conflicts

## Troubleshooting

### Common Issues

#### Build Failures
```bash
# Check CMake version
cmake --version

# Check compiler version
gcc --version
g++ --version

# Check dependencies
pkg-config --list-all | grep -E "(frr|yaml|pcap|ssl)"
```

#### Runtime Issues
```bash
# Check system requirements
./scripts/check_system_requirements.sh

# Check configuration
./build/router_sim --config-check config/router.yaml

# Enable debug logging
export ROUTER_LOG_LEVEL=debug
./build/router_sim --config config/router.yaml
```

#### Performance Issues
```bash
# Profile with perf
perf record -g ./build/router_sim --config config/router.yaml
perf report

# Profile with valgrind
valgrind --tool=callgrind ./build/router_sim --config config/router.yaml

# Monitor system resources
htop
iotop
nethogs
```

#### Network Issues
```bash
# Check network interfaces
ip addr show
ip route show

# Check FRR status
sudo systemctl status frr
sudo vtysh -c "show ip route"

# Check ClickHouse
clickhouse-client --query "SELECT 1"
```

### Debugging

#### Enable Debug Logging
```yaml
# config/router.yaml
logging:
  level: debug
  file: "/var/log/router-sim.log"
  console: true
```

#### GDB Debugging
```bash
# Compile with debug symbols
cmake -DCMAKE_BUILD_TYPE=Debug ..
make

# Run with GDB
gdb ./build/router_sim
(gdb) run --config config/router.yaml
(gdb) bt
```

#### Memory Debugging
```bash
# Use AddressSanitizer
cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_ASAN=ON ..
make

# Use Valgrind
valgrind --leak-check=full ./build/router_sim --config config/router.yaml
```

### Performance Tuning

#### System Tuning
```bash
# Increase file descriptor limits
echo "* soft nofile 65536" >> /etc/security/limits.conf
echo "* hard nofile 65536" >> /etc/security/limits.conf

# Tune network parameters
echo "net.core.rmem_max = 134217728" >> /etc/sysctl.conf
echo "net.core.wmem_max = 134217728" >> /etc/sysctl.conf
sysctl -p
```

#### Application Tuning
```yaml
# config/router.yaml
performance:
  worker_threads: 8
  packet_buffer_size: 65536
  analytics_batch_size: 1000
  clickhouse_connection_pool: 10
```

### Monitoring

#### Metrics
- Prometheus metrics: http://localhost:9090
- Grafana dashboards: http://localhost:3000
- System metrics: htop, iotop, nethogs

#### Logs
- Application logs: /var/log/router-sim.log
- System logs: journalctl -u router-sim
- FRR logs: /var/log/frr.log
- ClickHouse logs: /var/log/clickhouse-server.log
"""
        return content

    def generate_deepwiki_integration(self) -> Dict[str, Any]:
        """Generate DeepWiki integration configuration"""
        return {
            "project": {
                "name": "Router Simulator",
                "description": "Multi-Protocol Router Simulator with CloudPods Integration",
                "version": "1.0.0",
                "repository": "https://github.com/your-org/router-simulator",
                "license": "MIT",
                "tags": ["networking", "routing", "simulation", "cloudpods", "frr", "bgp", "ospf", "isis"]
            },
            "documentation": {
                "structure": self.doc_structure,
                "sections": {
                    "overview": self.generate_overview_docs(),
                    "api": self.generate_api_docs(),
                    "cloudpods": self.generate_cloudpods_docs(),
                    "development": self.generate_development_docs()
                }
            },
            "integration": {
                "cloudpods": {
                    "endpoint": "https://deepwiki.com/yunionio/cloudpods",
                    "concepts": [
                        "vpc_routing",
                        "nat_gateway",
                        "load_balancer",
                        "service_mesh",
                        "networking_concepts"
                    ]
                },
                "frr": {
                    "protocols": ["bgp", "ospf", "isis"],
                    "features": ["traffic_shaping", "network_impairments", "analytics"]
                }
            }
        }

    def generate_all_docs(self) -> None:
        """Generate all documentation"""
        logger.info("Generating documentation...")
        
        # Generate individual documentation files
        docs = {
            "overview.md": self.generate_overview_docs(),
            "api.md": self.generate_api_docs(),
            "cloudpods.md": self.generate_cloudpods_docs(),
            "development.md": self.generate_development_docs()
        }
        
        # Write documentation files
        for filename, content in docs.items():
            filepath = self.docs_dir / filename
            with open(filepath, 'w', encoding='utf-8') as f:
                f.write(content)
            logger.info(f"Generated {filepath}")
        
        # Generate DeepWiki integration
        deepwiki_config = self.generate_deepwiki_integration()
        deepwiki_file = self.docs_dir / "deepwiki.json"
        with open(deepwiki_file, 'w', encoding='utf-8') as f:
            json.dump(deepwiki_config, f, indent=2)
        logger.info(f"Generated {deepwiki_file}")
        
        # Generate index page
        index_content = self.generate_index_page()
        index_file = self.docs_dir / "index.md"
        with open(index_file, 'w', encoding='utf-8') as f:
            f.write(index_content)
        logger.info(f"Generated {index_file}")
        
        logger.info("Documentation generation completed!")

    def generate_index_page(self) -> str:
        """Generate main index page"""
        return """# Router Simulator Documentation

Welcome to the Router Simulator documentation! This comprehensive guide covers everything you need to know about building, deploying, and using the Router Simulator.

## Quick Start

1. [Overview](overview.md) - Learn about the system architecture and features
2. [Getting Started](overview.md#getting-started) - Set up your development environment
3. [Configuration](overview.md#configuration) - Configure the system for your needs
4. [API Reference](api.md) - Explore the available APIs
5. [CloudPods Integration](cloudpods.md) - Understand cloud networking concepts

## Documentation Sections

### [Overview](overview.md)
- Introduction and architecture
- Features and capabilities
- CloudPods integration
- Getting started guide

### [API Reference](api.md)
- C++ API documentation
- Rust API documentation
- Go API documentation
- REST API endpoints

### [CloudPods Integration](cloudpods.md)
- VPC routing concepts
- NAT Gateway functionality
- Load Balancer integration
- Service Mesh routing

### [Development Guide](development.md)
- Building from source
- Testing and validation
- Contributing guidelines
- Troubleshooting

## CloudPods Concepts

The Router Simulator integrates with CloudPods networking concepts:

- **VPC Routing**: Virtual Private Cloud routing simulation
- **NAT Gateway**: Network Address Translation functionality
- **Load Balancer**: Application and Network Load Balancer support
- **Service Mesh**: Service discovery and traffic management

## Multi-Protocol Support

- **BGP**: Border Gateway Protocol implementation
- **OSPF**: Open Shortest Path First protocol
- **IS-IS**: Intermediate System to Intermediate System protocol

## High-Performance Analytics

- **ClickHouse**: Time-series data storage and analytics
- **Rust Engine**: High-performance data processing
- **Real-time Metrics**: Prometheus and Grafana integration

## Getting Help

- Check the [troubleshooting guide](development.md#troubleshooting)
- Review the [FAQ](overview.md#faq)
- Join our community discussions
- Report issues on GitHub

## License

This project is licensed under the MIT License - see the LICENSE file for details.
"""

def main():
    parser = argparse.ArgumentParser(description='Generate Router Simulator documentation')
    parser.add_argument('--project-root', default='.', help='Project root directory')
    parser.add_argument('--output-dir', help='Output directory for documentation')
    parser.add_argument('--deepwiki', action='store_true', help='Generate DeepWiki integration')
    parser.add_argument('--verbose', '-v', action='store_true', help='Enable verbose logging')
    
    args = parser.parse_args()
    
    if args.verbose:
        logging.getLogger().setLevel(logging.DEBUG)
    
    generator = DocumentationGenerator(args.project_root)
    
    if args.output_dir:
        generator.docs_dir = Path(args.output_dir)
        generator.docs_dir.mkdir(exist_ok=True)
    
    generator.generate_all_docs()
    
    if args.deepwiki:
        logger.info("DeepWiki integration configuration generated in docs/deepwiki.json")

if __name__ == "__main__":
    main()
