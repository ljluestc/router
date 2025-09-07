#!/bin/bash

# Multi-Protocol Router Simulator - Build and Demo Script
# This script builds the project and starts the demo

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Function to print colored output
print_header() {
    echo -e "${PURPLE}╔══════════════════════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${PURPLE}║                    Multi-Protocol Router Simulator v1.0.0                   ║${NC}"
    echo -e "${PURPLE}║                                                                              ║${NC}"
    echo -e "${PURPLE}║  Features:                                                                   ║${NC}"
    echo -e "${PURPLE}║  • FRR Integration (BGP/OSPF/ISIS)                                          ║${NC}"
    echo -e "${PURPLE}║  • Traffic Shaping (Token Bucket/WFQ)                                       ║${NC}"
    echo -e "${PURPLE}║  • Network Impairments (tc/netem)                                           ║${NC}"
    echo -e "${PURPLE}║  • Comprehensive Testing (gtest/pcap diff)                                  ║${NC}"
    echo -e "${PURPLE}║  • CLI & YAML Scenario Support                                              ║${NC}"
    echo -e "${PURPLE}║  • Cloud Networking Concepts                                                ║${NC}"
    echo -e "${PURPLE}╚══════════════════════════════════════════════════════════════════════════════╝${NC}"
    echo
}

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

print_step() {
    echo -e "${CYAN}[STEP]${NC} $1"
}

# Check if we're in the right directory
if [ ! -f "CMakeLists.txt" ]; then
    print_error "CMakeLists.txt not found. Please run this script from the project root."
    exit 1
fi

print_header

# Parse command line arguments
BUILD_TYPE="Release"
CLEAN_BUILD=false
RUN_TESTS=false
START_DEMO=false
SKIP_BUILD=false
VERBOSE=false

while [[ $# -gt 0 ]]; do
    case $1 in
        -d|--debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        -c|--clean)
            CLEAN_BUILD=true
            shift
            ;;
        -t|--test)
            RUN_TESTS=true
            shift
            ;;
        --demo)
            START_DEMO=true
            shift
            ;;
        --skip-build)
            SKIP_BUILD=true
            shift
            ;;
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        -h|--help)
            echo "Usage: $0 [OPTIONS]"
            echo "Options:"
            echo "  -d, --debug       Build in Debug mode (default: Release)"
            echo "  -c, --clean       Clean build directory before building"
            echo "  -t, --test        Run tests after building"
            echo "  --demo            Start web demo after building"
            echo "  --skip-build      Skip building, just start demo"
            echo "  -v, --verbose     Verbose output"
            echo "  -h, --help        Show this help message"
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            exit 1
            ;;
    esac
done

# Check dependencies
print_step "Checking dependencies..."

# Check for required tools
command -v cmake >/dev/null 2>&1 || { print_error "cmake is required but not installed. Aborting."; exit 1; }
command -v make >/dev/null 2>&1 || { print_error "make is required but not installed. Aborting."; exit 1; }
command -v g++ >/dev/null 2>&1 || { print_error "g++ is required but not installed. Aborting."; exit 1; }

# Check for optional dependencies
if ! pkg-config --exists libpcap; then
    print_warning "libpcap not found. PCAP functionality will be limited."
fi

if ! pkg-config --exists yaml-cpp; then
    print_warning "yaml-cpp not found. YAML configuration will not be available."
fi

if ! pkg-config --exists frr; then
    print_warning "FRR not found. FRR integration will be disabled."
fi

if ! pkg-config --exists libzmq; then
    print_warning "ZeroMQ not found. ZMQ functionality will be limited."
fi

# Build the project
if [ "$SKIP_BUILD" = false ]; then
    print_step "Building Multi-Protocol Router Simulator"
    print_status "Build type: $BUILD_TYPE"
    
    # Clean build directory if requested
    if [ "$CLEAN_BUILD" = true ]; then
        print_status "Cleaning build directory..."
        rm -rf build
    fi
    
    # Create build directory
    print_status "Creating build directory..."
    mkdir -p build
    cd build
    
    # Configure with CMake
    print_status "Configuring with CMake..."
    CMAKE_ARGS="-DCMAKE_BUILD_TYPE=$BUILD_TYPE"
    
    if [ "$VERBOSE" = true ]; then
        CMAKE_ARGS="$CMAKE_ARGS -DCMAKE_VERBOSE_MAKEFILE=ON"
    fi
    
    cmake .. $CMAKE_ARGS
    
    # Build
    print_status "Building..."
    if [ "$VERBOSE" = true ]; then
        make VERBOSE=1 -j$(nproc)
    else
        make -j$(nproc)
    fi
    
    print_success "Build completed successfully!"
    
    # Run tests if requested
    if [ "$RUN_TESTS" = true ]; then
        print_step "Running tests..."
        if [ -f "./router_tests" ]; then
            ./router_tests
            print_success "Tests completed!"
        else
            print_warning "Test executable not found. Skipping tests."
        fi
    fi
    
    # Go back to project root
    cd ..
fi

# Start demo if requested
if [ "$START_DEMO" = true ]; then
    print_step "Starting web demo..."
    
    # Check if Python is available
    if command -v python3 >/dev/null 2>&1; then
        PYTHON_CMD="python3"
    elif command -v python >/dev/null 2>&1; then
        PYTHON_CMD="python"
    else
        print_error "Python is required to run the web demo. Please install Python 3."
        exit 1
    fi
    
    # Check if demo files exist
    if [ ! -f "docs/index.html" ]; then
        print_error "Demo files not found. Please make sure docs/index.html exists."
        exit 1
    fi
    
    print_status "Starting demo server..."
    print_status "The demo will open in your default web browser."
    print_status "Press Ctrl+C to stop the demo server."
    echo
    
    # Start the demo server
    $PYTHON_CMD scripts/serve_demo.py
fi

# Show build summary
print_step "Build Summary:"
echo "  Build type: $BUILD_TYPE"
echo "  Build directory: $(pwd)/build"
echo "  Executables:"
if [ -f "build/router_sim" ]; then
    echo "    - router_sim"
fi
if [ -f "build/router_tests" ]; then
    echo "    - router_tests"
fi

print_success "Build and demo script completed successfully!"

# Show usage information
echo
print_status "Usage examples:"
echo "  ./build/router_sim --help"
echo "  ./build/router_sim -s scenarios/bgp_convergence.yaml"
echo "  ./build/router_sim --daemon"
echo "  ./build/router_tests"
echo "  python3 scripts/serve_demo.py"
