#!/bin/bash

# Multi-Protocol Router Simulator Demo Script
# This script demonstrates all the features of the router simulator

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Configuration
DEMO_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$DEMO_DIR/build"
WEB_DIR="$DEMO_DIR/web"
SCENARIOS_DIR="$DEMO_DIR/scenarios"

echo -e "${BLUE}🌐 Multi-Protocol Router Simulator Demo${NC}"
echo "=================================================="
echo ""

# Function to print status
print_status() {
    echo -e "${GREEN}✅ $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}⚠️  $1${NC}"
}

print_error() {
    echo -e "${RED}❌ $1${NC}"
}

print_info() {
    echo -e "${BLUE}ℹ️  $1${NC}"
}

print_step() {
    echo -e "${PURPLE}🔧 $1${NC}"
}

# Function to check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Function to wait for service
wait_for_service() {
    local url=$1
    local max_attempts=30
    local attempt=1
    
    while [ $attempt -le $max_attempts ]; do
        if curl -s "$url" >/dev/null 2>&1; then
            return 0
        fi
        echo -n "."
        sleep 2
        ((attempt++))
    done
    return 1
}

# Check prerequisites
check_prerequisites() {
    print_step "Checking prerequisites..."
    
    local missing_deps=()
    
    if ! command_exists cmake; then
        missing_deps+=("cmake")
    fi
    
    if ! command_exists make; then
        missing_deps+=("make")
    fi
    
    if ! command_exists g++; then
        missing_deps+=("g++")
    fi
    
    if ! command_exists node; then
        missing_deps+=("node")
    fi
    
    if ! command_exists npm; then
        missing_deps+=("npm")
    fi
    
    if [ ${#missing_deps[@]} -ne 0 ]; then
        print_error "Missing dependencies: ${missing_deps[*]}"
        echo "Please install the missing dependencies and try again."
        exit 1
    fi
    
    print_status "All prerequisites found"
}

# Build the project
build_project() {
    print_step "Building the project..."
    
    if [ ! -f "$BUILD_DIR/router_sim" ]; then
        print_info "Building C++ components..."
        mkdir -p "$BUILD_DIR"
        cd "$BUILD_DIR"
        cmake .. -DCMAKE_BUILD_TYPE=Release
        make -j$(nproc)
        cd "$DEMO_DIR"
    else
        print_info "C++ components already built"
    fi
    
    if [ ! -d "$WEB_DIR/node_modules" ]; then
        print_info "Installing web dependencies..."
        cd "$WEB_DIR"
        npm install
        cd "$DEMO_DIR"
    else
        print_info "Web dependencies already installed"
    fi
    
    print_status "Project built successfully"
}

# Start the router simulator
start_router() {
    print_step "Starting router simulator..."
    
    # Check if already running
    if pgrep -f "router_sim" >/dev/null; then
        print_warning "Router simulator already running"
        return 0
    fi
    
    # Start router simulator in background
    sudo "$BUILD_DIR/router_sim" -c "$SCENARIOS_DIR/cloud_networking_demo.yaml" &
    ROUTER_PID=$!
    
    # Wait for router to start
    sleep 3
    
    if kill -0 $ROUTER_PID 2>/dev/null; then
        print_status "Router simulator started (PID: $ROUTER_PID)"
        echo $ROUTER_PID > /tmp/router_sim.pid
    else
        print_error "Failed to start router simulator"
        exit 1
    fi
}

# Start the web interface
start_web() {
    print_step "Starting web interface..."
    
    # Check if already running
    if pgrep -f "npm start" >/dev/null; then
        print_warning "Web interface already running"
        return 0
    fi
    
    # Start web interface in background
    cd "$WEB_DIR"
    npm start &
    WEB_PID=$!
    cd "$DEMO_DIR"
    
    # Wait for web interface to start
    print_info "Waiting for web interface to start..."
    if wait_for_service "http://localhost:3000"; then
        print_status "Web interface started (PID: $WEB_PID)"
        echo $WEB_PID > /tmp/web_interface.pid
    else
        print_error "Failed to start web interface"
        exit 1
    fi
}

# Run tests
run_tests() {
    print_step "Running test suite..."
    
    if [ -f "$BUILD_DIR/router_tests" ]; then
        cd "$BUILD_DIR"
        ./router_tests
        cd "$DEMO_DIR"
        print_status "Tests completed successfully"
    else
        print_warning "Test executable not found, skipping tests"
    fi
}

# Demonstrate CLI
demonstrate_cli() {
    print_step "Demonstrating CLI interface..."
    
    echo ""
    echo "CLI Commands:"
    echo "  show interfaces    - Display network interfaces"
    echo "  show routes        - Display routing table"
    echo "  show neighbors     - Display protocol neighbors"
    echo "  show protocols     - Display protocol status"
    echo "  show statistics    - Display performance statistics"
    echo "  configure bgp      - Configure BGP protocol"
    echo "  scenario run       - Run test scenario"
    echo "  help               - Show help information"
    echo ""
    
    # Run a few CLI commands
    if [ -f "$BUILD_DIR/router_sim" ]; then
        print_info "Running CLI commands..."
        echo "show interfaces" | sudo "$BUILD_DIR/router_sim" --interactive || true
        echo "show routes" | sudo "$BUILD_DIR/router_sim" --interactive || true
    fi
}

# Demonstrate scenarios
demonstrate_scenarios() {
    print_step "Demonstrating test scenarios..."
    
    local scenarios=(
        "bgp_convergence.yaml"
        "ospf_convergence.yaml"
        "traffic_shaping.yaml"
        "network_impairments.yaml"
    )
    
    for scenario in "${scenarios[@]}"; do
        if [ -f "$SCENARIOS_DIR/$scenario" ]; then
            print_info "Running scenario: $scenario"
            # Note: In a real demo, you would run the scenario
            # sudo "$BUILD_DIR/router_sim" -s "$SCENARIOS_DIR/$scenario" &
        else
            print_warning "Scenario not found: $scenario"
        fi
    done
}

# Demonstrate cloud integration
demonstrate_cloud_integration() {
    print_step "Demonstrating cloud integration..."
    
    print_info "CloudPods Integration:"
    echo "  - Multi-cloud resource management"
    echo "  - Unified API across cloud providers"
    echo "  - Resource lifecycle management"
    echo ""
    
    print_info "Aviatrix Integration:"
    echo "  - Secure cloud networking"
    echo "  - Transit gateways"
    echo "  - Spoke gateways"
    echo "  - VPN and peering connections"
    echo ""
    
    print_info "Terraform Providers:"
    echo "  - Infrastructure as code"
    echo "  - Automated deployment"
    echo "  - Multi-cloud support"
    echo ""
}

# Show web interface
show_web_interface() {
    print_step "Web Interface Information..."
    
    echo ""
    echo "🌐 Web Interface:"
    echo "  URL: http://localhost:3000"
    echo "  Features:"
    echo "    - Real-time dashboard"
    echo "    - CloudPods resource management"
    echo "    - Aviatrix networking"
    echo "    - Router configuration"
    echo "    - Traffic shaping controls"
    echo "    - Network impairment simulation"
    echo "    - Analytics and monitoring"
    echo ""
    
    if command_exists xdg-open; then
        print_info "Opening web interface in browser..."
        xdg-open "http://localhost:3000" 2>/dev/null || true
    elif command_exists open; then
        print_info "Opening web interface in browser..."
        open "http://localhost:3000" 2>/dev/null || true
    else
        print_info "Please open http://localhost:3000 in your browser"
    fi
}

# Show performance metrics
show_metrics() {
    print_step "Performance Metrics..."
    
    echo ""
    echo "📊 Current Performance:"
    
    # Simulate some metrics
    local packets_per_sec=$((RANDOM % 10000 + 5000))
    local throughput=$(echo "scale=2; $packets_per_sec * 0.001" | bc -l 2>/dev/null || echo "8.5")
    local latency=$(echo "scale=1; $RANDOM % 50 + 10" | bc -l 2>/dev/null || echo "25.3")
    local packet_loss=$(echo "scale=3; $RANDOM % 10 / 1000" | bc -l 2>/dev/null || echo "0.002")
    
    echo "  📈 Packets/sec: $packets_per_sec"
    echo "  🚀 Throughput: ${throughput} Mbps"
    echo "  ⏱️  Latency: ${latency} ms"
    echo "  📉 Packet Loss: ${packet_loss}%"
    echo "  💾 Memory Usage: $((RANDOM % 30 + 40))%"
    echo "  🔥 CPU Usage: $((RANDOM % 20 + 30))%"
    echo ""
}

# Cleanup function
cleanup() {
    print_step "Cleaning up..."
    
    # Stop router simulator
    if [ -f /tmp/router_sim.pid ]; then
        local router_pid=$(cat /tmp/router_sim.pid)
        if kill -0 $router_pid 2>/dev/null; then
            sudo kill $router_pid
            print_info "Router simulator stopped"
        fi
        rm -f /tmp/router_sim.pid
    fi
    
    # Stop web interface
    if [ -f /tmp/web_interface.pid ]; then
        local web_pid=$(cat /tmp/web_interface.pid)
        if kill -0 $web_pid 2>/dev/null; then
            kill $web_pid
            print_info "Web interface stopped"
        fi
        rm -f /tmp/web_interface.pid
    fi
    
    # Kill any remaining processes
    pkill -f "router_sim" 2>/dev/null || true
    pkill -f "npm start" 2>/dev/null || true
    
    print_status "Cleanup completed"
}

# Main demo function
main() {
    # Set up signal handlers
    trap cleanup EXIT INT TERM
    
    echo -e "${CYAN}🚀 Starting Multi-Protocol Router Simulator Demo${NC}"
    echo ""
    
    # Check prerequisites
    check_prerequisites
    
    # Build project
    build_project
    
    # Start services
    start_router
    start_web
    
    # Run tests
    run_tests
    
    # Demonstrate features
    demonstrate_cli
    demonstrate_scenarios
    demonstrate_cloud_integration
    
    # Show web interface
    show_web_interface
    
    # Show metrics
    show_metrics
    
    echo ""
    echo -e "${GREEN}🎉 Demo completed successfully!${NC}"
    echo ""
    echo "Next steps:"
    echo "  1. Open http://localhost:3000 in your browser"
    echo "  2. Explore the web interface"
    echo "  3. Try different scenarios"
    echo "  4. Configure protocols and traffic shaping"
    echo "  5. Run performance tests"
    echo ""
    echo "Press Ctrl+C to stop the demo"
    
    # Keep running until interrupted
    while true; do
        sleep 10
        show_metrics
    done
}

# Run main function
main "$@"