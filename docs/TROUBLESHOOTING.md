# Troubleshooting Guide

## Common Issues

### Build Issues

#### CMake Configuration Fails

**Problem**: CMake fails to find required dependencies.

**Solution**:
```bash
# Install missing dependencies
sudo apt-get install libpcap-dev libnl-3-dev libyaml-cpp-dev libgtest-dev frr-dev

# Or on CentOS/RHEL
sudo yum install libpcap-devel libnl3-devel yaml-cpp-devel gtest-devel frr-devel

# Clean and reconfigure
rm -rf build
mkdir build && cd build
cmake ..
```

#### Compilation Errors

**Problem**: C++ compilation fails with template or standard library errors.

**Solution**:
```bash
# Ensure C++17 support
export CXXFLAGS="-std=c++17"

# Check compiler version
g++ --version  # Should be 7.0 or later
clang++ --version  # Should be 5.0 or later

# Rebuild with verbose output
make VERBOSE=1
```

#### FRR Integration Issues

**Problem**: FRR libraries not found or incompatible.

**Solution**:
```bash
# Check FRR installation
pkg-config --modversion frr

# Install FRR development packages
sudo apt-get install frr-dev frr-doc

# Or build FRR from source
git clone https://github.com/FRRouting/frr.git
cd frr
./bootstrap.sh
./configure --enable-shared
make
sudo make install
```

### Runtime Issues

#### Router Fails to Start

**Problem**: Router simulator fails to start with error messages.

**Solution**:
1. **Check configuration file**:
   ```bash
   # Validate YAML syntax
   python -c "import yaml; yaml.safe_load(open('config.yaml'))"
   ```

2. **Check permissions**:
   ```bash
   # Ensure proper permissions for network operations
   sudo setcap cap_net_raw,cap_net_admin+eip ./router_sim
   ```

3. **Check port availability**:
   ```bash
   # Check if ports are in use
   netstat -tulpn | grep :8080
   lsof -i :8080
   ```

#### Interface Issues

**Problem**: Network interfaces not recognized or not working.

**Solution**:
1. **Check interface status**:
   ```bash
   ip link show
   ip addr show
   ```

2. **Create test interfaces**:
   ```bash
   # Create virtual interfaces for testing
   sudo ip link add veth0 type veth peer name veth1
   sudo ip link set veth0 up
   sudo ip link set veth1 up
   sudo ip addr add 192.168.1.1/24 dev veth0
   sudo ip addr add 192.168.1.2/24 dev veth1
   ```

3. **Check interface configuration**:
   ```yaml
   router:
     interfaces:
       - name: "veth0"
         ip_address: "192.168.1.1"
         netmask: "255.255.255.0"
         mtu: 1500
         enabled: true
   ```

#### Protocol Issues

**Problem**: BGP, OSPF, or IS-IS protocols not working correctly.

**Solution**:
1. **Check protocol configuration**:
   ```yaml
   protocols:
     - type: "bgp"
       name: "BGP"
       enabled: true
       parameters:
         asn: 65001
         router_id: "192.168.1.1"
         neighbors: "192.168.1.2"
   ```

2. **Check neighbor connectivity**:
   ```bash
   # Test connectivity to neighbors
   ping 192.168.1.2
   telnet 192.168.1.2 179  # BGP port
   ```

3. **Check protocol logs**:
   ```bash
   # Enable verbose logging
   ./router_sim --verbose --config config.yaml
   ```

#### Traffic Shaping Issues

**Problem**: Traffic shaping not working as expected.

**Solution**:
1. **Check algorithm configuration**:
   ```yaml
   traffic_shaping:
     algorithm: "wfq"  # or "token_bucket", "drr"
     rate: 1000000
     burst_size: 100000
     enabled: true
   ```

2. **Verify class configuration**:
   ```yaml
   parameters:
     classes:
       - class_id: 1
         weight: 10
         min_bandwidth: 1000000
         max_bandwidth: 10000000
         name: "High Priority"
         is_active: true
   ```

3. **Check packet classification**:
   ```cpp
   // Ensure packets are properly classified
   auto classifier = [](const PacketInfo& packet) -> uint8_t {
       if (packet.dscp >= 48) return 1;  // High priority
       return 2;  // Low priority
   };
   wfq->set_classifier(classifier);
   ```

### Performance Issues

#### High CPU Usage

**Problem**: Router simulator consumes too much CPU.

**Solution**:
1. **Check thread configuration**:
   ```yaml
   router:
     threading:
       max_threads: 4
       io_threads: 2
       protocol_threads: 2
   ```

2. **Optimize traffic shaping**:
   - Use DRR instead of WFQ for high-performance scenarios
   - Reduce queue sizes
   - Use simpler classification logic

3. **Profile the application**:
   ```bash
   # Use perf to profile
   perf record ./router_sim
   perf report
   
   # Or use valgrind
   valgrind --tool=callgrind ./router_sim
   ```

#### Memory Leaks

**Problem**: Memory usage continuously increases.

**Solution**:
1. **Use Valgrind to detect leaks**:
   ```bash
   valgrind --leak-check=full --show-leak-kinds=all ./router_sim
   ```

2. **Check for circular references**:
   - Ensure proper cleanup in destructors
   - Use smart pointers appropriately
   - Avoid holding references to deleted objects

3. **Monitor memory usage**:
   ```bash
   # Monitor memory usage
   top -p $(pgrep router_sim)
   htop -p $(pgrep router_sim)
   ```

#### Slow Packet Processing

**Problem**: Packet processing is slower than expected.

**Solution**:
1. **Check queue sizes**:
   - Too large queues can cause delays
   - Too small queues can cause drops

2. **Optimize algorithms**:
   - Use DRR for high-throughput scenarios
   - Implement packet batching
   - Use lock-free data structures where possible

3. **Profile packet processing**:
   ```cpp
   // Add timing measurements
   auto start = std::chrono::high_resolution_clock::now();
   process_packet(packet);
   auto end = std::chrono::high_resolution_clock::now();
   auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
   ```

### Testing Issues

#### Test Failures

**Problem**: Unit or integration tests fail.

**Solution**:
1. **Check test environment**:
   ```bash
   # Ensure test dependencies are installed
   sudo apt-get install gtest-dev
   
   # Run tests with verbose output
   ./router_tests --gtest_output=xml:test_results.xml
   ```

2. **Check test configuration**:
   ```yaml
   testing:
     timeout: 30s
     retries: 3
     parallel: true
   ```

3. **Debug specific tests**:
   ```bash
   # Run specific test with debug output
   ./router_tests --gtest_filter="RouterCoreTest.*" --gtest_output=console
   ```

#### PCAP Comparison Issues

**Problem**: PCAP diffing fails or produces unexpected results.

**Solution**:
1. **Check PCAP file format**:
   ```bash
   # Validate PCAP files
   tcpdump -r file.pcap -c 10
   ```

2. **Check comparison options**:
   ```cpp
   PcapDiffOptions options;
   options.ignore_timestamps = true;
   options.tolerance = 0.01;
   options.ignore_checksums = true;
   ```

3. **Use appropriate filters**:
   ```bash
   # Filter packets before comparison
   tcpdump -r input.pcap -w filtered.pcap "tcp port 80"
   ```

### Monitoring Issues

#### Metrics Not Collected

**Problem**: Monitoring system not collecting metrics.

**Solution**:
1. **Check collector registration**:
   ```cpp
   auto collector = std::make_shared<RouterMetricsCollector>(router);
   monitoring->register_collector(collector);
   monitoring->start();
   ```

2. **Check metric names**:
   - Ensure metric names are valid
   - Check for naming conflicts
   - Use consistent naming conventions

3. **Verify collection interval**:
   ```cpp
   // Check collection interval
   auto stats = monitoring->get_statistics();
   auto last_collection = stats.last_collection;
   ```

#### Alerts Not Firing

**Problem**: Alert rules not triggering when expected.

**Solution**:
1. **Check alert rule configuration**:
   ```cpp
   AlertRule rule;
   rule.name = "HighCPUUsage";
   rule.expression = "cpu_usage > 80";
   rule.severity = "warning";
   rule.enabled = true;
   rule.duration = std::chrono::seconds(300);
   ```

2. **Verify metric values**:
   ```cpp
   // Check if metrics are being collected
   auto metrics = monitoring->get_metrics();
   for (const auto& metric : metrics) {
       std::cout << metric.name << ": " << metric.value << std::endl;
   }
   ```

3. **Test alert rules**:
   ```cpp
   // Manually trigger alerts for testing
   monitoring->set_gauge("cpu_usage", 90.0);
   monitoring->evaluate_alerts();
   ```

### Web Interface Issues

#### Frontend Not Loading

**Problem**: Web interface not accessible or not loading.

**Solution**:
1. **Check API server**:
   ```bash
   # Ensure API server is running
   curl http://localhost:8080/api/v1/router/status
   ```

2. **Check CORS configuration**:
   ```go
   // Ensure CORS is properly configured
   router.Use(func(c *gin.Context) {
       c.Header("Access-Control-Allow-Origin", "*")
       c.Header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS")
       c.Header("Access-Control-Allow-Headers", "Origin, Content-Type, Accept, Authorization")
   })
   ```

3. **Check static file serving**:
   ```go
   // Ensure static files are served correctly
   router.Static("/static", "./web/dist")
   router.StaticFile("/", "./web/dist/index.html")
   ```

#### WebSocket Connection Issues

**Problem**: WebSocket connections fail or disconnect frequently.

**Solution**:
1. **Check WebSocket configuration**:
   ```go
   var upgrader = websocket.Upgrader{
       CheckOrigin: func(r *http.Request) bool {
           return true // Allow all origins in development
       },
   }
   ```

2. **Implement reconnection logic**:
   ```javascript
   function connectWebSocket() {
       const ws = new WebSocket('ws://localhost:8080/ws');
       
       ws.onopen = function() {
           console.log('WebSocket connected');
       };
       
       ws.onclose = function() {
           console.log('WebSocket disconnected, reconnecting...');
           setTimeout(connectWebSocket, 1000);
       };
       
       ws.onerror = function(error) {
           console.error('WebSocket error:', error);
       };
   }
   ```

3. **Check network connectivity**:
   ```bash
   # Test WebSocket connection
   wscat -c ws://localhost:8080/ws
   ```

## Debugging Tools

### Logging

Enable verbose logging for debugging:

```bash
# Enable debug logging
export ROUTER_LOG_LEVEL=debug
./router_sim --verbose --config config.yaml

# Or set in configuration
log:
  level: "debug"
  json: false
  file: "/var/log/router_sim.log"
```

### Profiling

Use profiling tools to identify performance bottlenecks:

```bash
# CPU profiling
perf record -g ./router_sim
perf report

# Memory profiling
valgrind --tool=massif ./router_sim
ms_print massif.out.* > memory_profile.txt

# Call graph analysis
valgrind --tool=callgrind ./router_sim
callgrind_annotate callgrind.out.* > callgraph.txt
```

### Network Analysis

Use network analysis tools to debug network issues:

```bash
# Packet capture
tcpdump -i any -w capture.pcap

# Network statistics
netstat -i
ss -tuln

# Traffic analysis
iftop -i eth0
nethogs
```

## Getting Help

### Log Files

Check log files for error messages:

```bash
# System logs
journalctl -u router_sim

# Application logs
tail -f /var/log/router_sim.log

# Error logs
grep -i error /var/log/router_sim.log
```

### Community Support

- **GitHub Issues**: Report bugs and request features
- **GitHub Discussions**: Ask questions and share ideas
- **Documentation**: Check the comprehensive documentation
- **Examples**: Look at example configurations and code

### Professional Support

For enterprise deployments and professional support:

- **Consulting Services**: Custom implementations and optimizations
- **Training**: Team training and knowledge transfer
- **Support Contracts**: Priority support and maintenance
- **Custom Development**: Feature development and integration

## Best Practices

### Configuration Management

1. **Use version control** for configuration files
2. **Validate configurations** before deployment
3. **Use environment-specific** configurations
4. **Document configuration changes**

### Monitoring and Alerting

1. **Set up comprehensive monitoring** from the start
2. **Configure appropriate alert thresholds**
3. **Test alert rules** regularly
4. **Monitor system resources** continuously

### Testing

1. **Write comprehensive tests** for all components
2. **Use integration tests** for end-to-end validation
3. **Implement performance tests** for critical paths
4. **Test failure scenarios** and recovery procedures

### Security

1. **Use secure configurations** in production
2. **Implement proper authentication** and authorization
3. **Regular security updates** and patches
4. **Monitor for security vulnerabilities**

This troubleshooting guide should help you resolve most common issues with the router simulator. If you encounter issues not covered here, please check the GitHub issues or create a new one with detailed information about your problem.
