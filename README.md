# Multi-Protocol Router Simulator

A comprehensive C++ router simulator with FRR control-plane integration, advanced traffic shaping, network impairments simulation, and extensive testing capabilities.

## Features

### 🔧 FRR Integration
- **BGP (Border Gateway Protocol)**: Full BGP implementation with AS management
- **OSPF (Open Shortest Path First)**: Area-based routing with neighbor discovery
- **IS-IS (Intermediate System to Intermediate System)**: Level-1 and Level-2 routing
- **Real-time route management**: Dynamic route updates and convergence
- **Neighbor monitoring**: Live neighbor status and statistics

### ⚡ Traffic Shaping
- **Token Bucket**: Rate limiting with burst capacity
- **Weighted Fair Queueing (WFQ)**: Multi-queue traffic management
- **Leaky Bucket**: Smooth traffic regulation
- **Priority-based queuing**: DSCP and priority handling
- **Real-time statistics**: Comprehensive traffic metrics

### 🌐 Network Impairments
- **tc/netem integration**: Linux traffic control integration
- **Delay simulation**: Configurable latency and jitter
- **Packet loss**: Percentage-based loss simulation
- **Corruption**: Bit-level packet corruption
- **Bandwidth limiting**: Rate limiting with impairments
- **Real-world scenarios**: Satellite, mobile, DSL, fiber simulations

### 🧪 Testing Framework
- **Google Test integration**: Comprehensive unit testing
- **PCAP diffing**: Packet capture analysis and comparison
- **YAML scenario configuration**: Declarative test scenarios
- **Regression test suites**: Automated testing pipeline
- **Coverage reporting**: Code coverage analysis

### 💻 CLI Interface
- **Interactive configuration**: Real-time router configuration
- **Monitoring dashboard**: Live statistics and status
- **Scenario execution**: Run test scenarios from CLI
- **Help system**: Built-in documentation and examples
- **Command completion**: Auto-completion for commands

### 📊 Cloud Networking
- **Multi-cloud connectivity**: Simulate cloud-to-cloud connections
- **SD-WAN simulation**: Software-defined wide area networking
- **Load balancing**: Traffic distribution algorithms
- **Failover scenarios**: High availability testing
- **Performance monitoring**: Real-time metrics and alerts

## Quick Start

### Prerequisites

- **C++17** compatible compiler (GCC 7+, Clang 5+, MSVC 2019+)
- **CMake** 3.16 or later
- **FRR** (Free Range Routing) - for protocol integration
- **libpcap** - for packet capture
- **yaml-cpp** - for YAML configuration
- **Linux** - for tc/netem integration

### Installation

```bash
# Clone the repository
git clone https://github.com/yourusername/router-sim.git
cd router-sim

# Install dependencies (Ubuntu/Debian)
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    libpcap-dev \
    libyaml-cpp-dev \
    frr \
    iproute2 \
    net-tools

# Build the project
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)

# Install (optional)
sudo make install
```

### Basic Usage

```bash
# Run with interactive CLI
./router_sim -i

# Load configuration from file
./router_sim -c config.yaml -i

# Execute test scenario
./router_sim -s scenario.yaml

# Run as daemon
./router_sim -d --config /etc/router_sim.yaml
```

## Configuration

### Router Configuration

```yaml
# Basic router setup
router_id: 1.1.1.1
hostname: production-router
interfaces:
  - eth0
  - eth1

# Protocol configuration
enable_bgp: true
enable_ospf: true
enable_isis: false
as_number: 65001
area_id: 0.0.0.0
system_id: 0000.0000.0001

# Interface details
interface_configs:
  eth0:
    ip_address: 192.168.1.1
    subnet_mask: 255.255.255.0
    bandwidth_mbps: 1000
    is_up: true
    description: "LAN Interface"
```

### Traffic Shaping

```yaml
traffic_shaping:
  eth0:
    rate_bps: 100000000  # 100 Mbps
    burst_size: 1000000  # 1 MB
    enable_wfq: true
    num_queues: 8
    queue_weights: [1, 2, 4, 8, 16, 32, 64, 128]
```

### Network Impairments

```yaml
impairments:
  eth0:
    enable_delay: true
    delay_ms: 100
    enable_jitter: true
    jitter_ms: 20
    enable_loss: true
    loss_percent: 2.0
    enable_corruption: true
    corruption_percent: 0.1
```

## Test Scenarios

The simulator includes comprehensive test scenarios:

### Basic Connectivity Test
```yaml
- name: basic_connectivity_test
  description: "Test basic router connectivity"
  steps:
    - type: configure_interface
      name: configure_eth0
      parameters:
        name: eth0
        ip_address: 192.168.1.1
        subnet_mask: 255.255.255.0
    - type: start_protocol
      name: start_bgp
      parameters:
        protocol: bgp
        as_number: 65001
```

### Traffic Shaping Test
```yaml
- name: traffic_shaping_test
  description: "Test QoS and rate limiting"
  steps:
    - type: configure_traffic_shaping
      name: enable_token_bucket
      parameters:
        interface: eth0
        rate_bps: 100000000
        burst_size: 1000000
    - type: send_packet
      name: send_test_packets
      parameters:
        interface: eth0
        packet_count: 1000
        rate: 200000000  # Exceeds limit
```

### Network Impairments Test
```yaml
- name: impairments_test
  description: "Test network impairments"
  steps:
    - type: configure_impairments
      name: enable_delay
      parameters:
        interface: eth0
        enable_delay: true
        delay_ms: 100
        enable_loss: true
        loss_percent: 5.0
```

## CLI Commands

### Interface Management
```bash
# Show interfaces
show interfaces

# Configure interface
configure interface eth0 192.168.1.1 255.255.255.0

# Interface control
interface eth0 up
interface eth0 down
```

### Protocol Management
```bash
# Show protocols
show protocols

# Start/stop protocols
protocol bgp start
protocol ospf stop
protocol isis restart

# Show routes
show routes
show routes bgp
```

### Traffic Shaping
```bash
# Show traffic shaping
traffic show

# Configure shaping
traffic configure eth0 rate 100000000 burst 1000000
traffic configure eth0 wfq enable 8
```

### Network Impairments
```bash
# Show impairments
impairment show

# Configure impairments
impairment configure eth0 delay 100ms loss 2%
impairment clear eth0
```

### Statistics
```bash
# Show statistics
statistics
statistics interface eth0
statistics protocol bgp
```

## API Reference

### Core Classes

#### RouterSimulator
Main router simulator class providing high-level interface.

```cpp
#include "router_sim.h"

RouterSimulator router;
RouterConfig config;
config.router_id = "1.1.1.1";
config.enable_bgp = true;

router.initialize(config);
router.start();
```

#### FRRIntegration
FRR protocol integration for BGP, OSPF, and IS-IS.

```cpp
#include "frr_integration.h"

FRRIntegration frr;
BGPConfig bgp_config;
bgp_config.as_number = 65001;
bgp_config.router_id = "1.1.1.1";

frr.start_bgp(bgp_config);
```

#### TrafficShaper
Advanced traffic shaping with token bucket and WFQ.

```cpp
#include "traffic_shaping.h"

TrafficShaper shaper;
ShapingConfig config;
config.rate_bps = 100000000;
config.enable_wfq = true;

shaper.add_interface("eth0", config);
```

#### NetemImpairments
Network impairments simulation using tc/netem.

```cpp
#include "netem_impairments.h"

NetemImpairments impairments;
ImpairmentConfig config;
config.enable_delay = true;
config.delay_ms = 100;

impairments.configure_impairments("eth0", config);
```

## Testing

### Unit Tests
```bash
# Run all tests
make test

# Run specific test
./router_sim_test --gtest_filter="TrafficShapingTest.*"

# Run with coverage
cmake -DENABLE_COVERAGE=ON ..
make
./router_sim_test
gcov router_sim_test
```

### Integration Tests
```bash
# Run scenario tests
./router_sim -s test_scenarios.yaml

# Run regression tests
./scripts/run_regression_tests.sh
```

### Performance Tests
```bash
# Run performance benchmarks
./scripts/run_performance_tests.sh

# Generate performance report
./scripts/generate_performance_report.sh
```

## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

### Development Setup

```bash
# Install development dependencies
sudo apt-get install -y \
    clang-format \
    cppcheck \
    valgrind \
    gdb

# Run code formatting
./scripts/format_code.sh

# Run static analysis
./scripts/run_static_analysis.sh

# Run memory checks
./scripts/run_memory_checks.sh
```

## Documentation

- [API Documentation](docs/api.md)
- [Configuration Guide](docs/configuration.md)
- [Testing Guide](docs/testing.md)
- [Performance Tuning](docs/performance.md)
- [Troubleshooting](docs/troubleshooting.md)

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- [FRR](https://frrouting.org/) - Free Range Routing
- [Google Test](https://github.com/google/googletest) - Testing framework
- [yaml-cpp](https://github.com/jbeder/yaml-cpp) - YAML parser
- [libpcap](https://www.tcpdump.org/) - Packet capture library

## Support

- 📧 Email: support@router-sim.dev
- 💬 Discord: [Router Sim Community](https://discord.gg/router-sim)
- 📖 Wiki: [GitHub Wiki](https://github.com/yourusername/router-sim/wiki)
- 🐛 Issues: [GitHub Issues](https://github.com/yourusername/router-sim/issues)

---

**Built with ❤️ for the networking community**