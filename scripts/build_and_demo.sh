#!/bin/bash

# Multi-Protocol Router Simulator - Build and Demo Script
# This script builds the complete router simulator and starts the demo

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Function to check dependencies
check_dependencies() {
    print_status "Checking dependencies..."
    
    local missing_deps=()
    
    # Check for required tools
    if ! command_exists cmake; then
        missing_deps+=("cmake")
    fi
    
    if ! command_exists make; then
        missing_deps+=("make")
    fi
    
    if ! command_exists g++; then
        missing_deps+=("g++")
    fi
    
    if ! command_exists pkg-config; then
        missing_deps+=("pkg-config")
    fi
    
    if ! command_exists cargo; then
        missing_deps+=("cargo")
    fi
    
    if ! command_exists python3; then
        missing_deps+=("python3")
    fi
    
    # Check for required libraries
    if ! pkg-config --exists libpcap; then
        missing_deps+=("libpcap-dev")
    fi
    
    if ! pkg-config --exists yaml-cpp; then
        missing_deps+=("libyaml-cpp-dev")
    fi
    
    if ! pkg-config --exists frr; then
        missing_deps+=("frr")
    fi
    
    if [ ${#missing_deps[@]} -ne 0 ]; then
        print_error "Missing dependencies: ${missing_deps[*]}"
        print_status "Please install them using:"
        print_status "sudo apt-get update && sudo apt-get install -y ${missing_deps[*]}"
        exit 1
    fi
    
    print_success "All dependencies are installed"
}

# Function to build C++ components
build_cpp() {
    print_status "Building C++ components..."
    
    # Create build directory
    mkdir -p build
    cd build
    
    # Configure with CMake
    print_status "Configuring with CMake..."
    cmake -DCMAKE_BUILD_TYPE=Release \
          -DENABLE_COVERAGE=ON \
          -DENABLE_TESTING=ON \
          ..
    
    # Build
    print_status "Building with make..."
    make -j$(nproc)
    
    # Run tests
    print_status "Running tests..."
    if [ -f "router_sim_test" ]; then
        ./router_sim_test
        print_success "All tests passed"
    else
        print_warning "Test executable not found, skipping tests"
    fi
    
    cd ..
    print_success "C++ build completed"
}

# Function to build Rust components
build_rust() {
    print_status "Building Rust components..."
    
    cd rust
    
    # Build Rust library
    print_status "Building Rust library..."
    cargo build --release
    
    # Run Rust tests
    print_status "Running Rust tests..."
    cargo test
    
    cd ..
    print_success "Rust build completed"
}

# Function to start demo
start_demo() {
    print_status "Starting demo..."
    
    # Check if demo files exist
    if [ ! -f "demo/index.html" ]; then
        print_error "Demo files not found"
        exit 1
    fi
    
    # Start Python HTTP server
    print_status "Starting HTTP server on port 8080..."
    print_status "Open your browser and go to: http://localhost:8080/demo/"
    
    # Start the server in background
    python3 -m http.server 8080 &
    local server_pid=$!
    
    # Wait for user to stop
    print_status "Press Ctrl+C to stop the demo"
    trap "kill $server_pid; exit" INT
    
    # Keep script running
    wait $server_pid
}

# Function to run router simulator
run_simulator() {
    print_status "Starting router simulator..."
    
    if [ ! -f "build/router_sim" ]; then
        print_error "Router simulator not found. Please build first."
        exit 1
    fi
    
    # Check if running as root (needed for network operations)
    if [ "$EUID" -ne 0 ]; then
        print_warning "Running without root privileges. Some features may not work."
        print_status "Consider running with sudo for full functionality."
    fi
    
    # Start the simulator
    cd build
    ./router_sim -i
}

# Function to show help
show_help() {
    echo "Multi-Protocol Router Simulator - Build and Demo Script"
    echo ""
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  -h, --help          Show this help message"
    echo "  -d, --demo          Start the web demo"
    echo "  -r, --run           Run the router simulator"
    echo "  -b, --build         Build all components"
    echo "  -c, --clean         Clean build artifacts"
    echo "  -t, --test          Run tests only"
    echo "  -a, --all           Build and start demo (default)"
    echo ""
    echo "Examples:"
    echo "  $0                  # Build and start demo"
    echo "  $0 -d               # Start demo only"
    echo "  $0 -r               # Run simulator only"
    echo "  $0 -b               # Build only"
    echo "  $0 -c               # Clean build"
}

# Function to clean build artifacts
clean_build() {
    print_status "Cleaning build artifacts..."
    
    if [ -d "build" ]; then
        rm -rf build
        print_success "Build directory cleaned"
    fi
    
    if [ -d "rust/target" ]; then
        cd rust
        cargo clean
        cd ..
        print_success "Rust target directory cleaned"
    fi
    
    print_success "Clean completed"
}

# Function to run tests only
run_tests() {
    print_status "Running tests..."
    
    # Run C++ tests
    if [ -f "build/router_sim_test" ]; then
        print_status "Running C++ tests..."
        cd build
        ./router_sim_test
        cd ..
        print_success "C++ tests completed"
    else
        print_warning "C++ test executable not found"
    fi
    
    # Run Rust tests
    if [ -d "rust" ]; then
        print_status "Running Rust tests..."
        cd rust
        cargo test
        cd ..
        print_success "Rust tests completed"
    else
        print_warning "Rust directory not found"
    fi
}

# Main function
main() {
    local action="all"
    
    # Parse command line arguments
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                show_help
                exit 0
                ;;
            -d|--demo)
                action="demo"
                shift
                ;;
            -r|--run)
                action="run"
                shift
                ;;
            -b|--build)
                action="build"
                shift
                ;;
            -c|--clean)
                action="clean"
                shift
                ;;
            -t|--test)
                action="test"
                shift
                ;;
            -a|--all)
                action="all"
                shift
                ;;
            *)
                print_error "Unknown option: $1"
                show_help
                exit 1
                ;;
        esac
    done
    
    # Execute based on action
    case $action in
        "all")
            check_dependencies
            build_cpp
            build_rust
            start_demo
            ;;
        "demo")
            start_demo
            ;;
        "run")
            run_simulator
            ;;
        "build")
            check_dependencies
            build_cpp
            build_rust
            ;;
        "clean")
            clean_build
            ;;
        "test")
            run_tests
            ;;
        *)
            print_error "Unknown action: $action"
            exit 1
            ;;
    esac
}

# Run main function with all arguments
main "$@"