#!/bin/bash

# Multi-Protocol Router Simulator Build Script
# This script builds the C++ core, Rust analytics, Go microservices, and web interface

set -e

echo "🚀 Building Multi-Protocol Router Simulator..."

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

# Check dependencies
check_dependencies() {
    print_status "Checking dependencies..."
    
    # Check for required tools
    local missing_deps=()
    
    if ! command -v cmake &> /dev/null; then
        missing_deps+=("cmake")
    fi
    
    if ! command -v g++ &> /dev/null; then
        missing_deps+=("g++")
    fi
    
    if ! command -v make &> /dev/null; then
        missing_deps+=("make")
    fi
    
    if ! command -v pkg-config &> /dev/null; then
        missing_deps+=("pkg-config")
    fi
    
    if ! command -v cargo &> /dev/null; then
        missing_deps+=("rust")
    fi
    
    if ! command -v go &> /dev/null; then
        missing_deps+=("go")
    fi
    
    if ! command -v node &> /dev/null; then
        missing_deps+=("nodejs")
    fi
    
    if ! command -v npm &> /dev/null; then
        missing_deps+=("npm")
    fi
    
    if [ ${#missing_deps[@]} -ne 0 ]; then
        print_error "Missing dependencies: ${missing_deps[*]}"
        print_status "Please install missing dependencies and try again."
        print_status "On Ubuntu/Debian: sudo apt-get install cmake build-essential pkg-config rustc cargo golang-go nodejs npm"
        print_status "On CentOS/RHEL: sudo yum install cmake gcc-c++ pkgconfig rust cargo golang nodejs npm"
        exit 1
    fi
    
    print_success "All dependencies found"
}

# Build C++ core
build_cpp_core() {
    print_status "Building C++ core..."
    
    # Create build directory
    mkdir -p build
    cd build
    
    # Configure with CMake
    print_status "Configuring CMake..."
    if ! cmake .. -DCMAKE_BUILD_TYPE=Release; then
        print_error "CMake configuration failed"
        print_warning "This might be due to missing FRR development packages"
        print_status "Try installing: sudo apt-get install libfrr-dev frr-dev"
        exit 1
    fi
    
    # Build
    print_status "Compiling C++ core..."
    if ! make -j$(nproc); then
        print_error "C++ compilation failed"
        exit 1
    fi
    
    cd ..
    print_success "C++ core built successfully"
}

# Build Rust analytics
build_rust_analytics() {
    print_status "Building Rust analytics engine..."
    
    cd rust
    if ! cargo build --release; then
        print_error "Rust compilation failed"
        exit 1
    fi
    cd ..
    
    print_success "Rust analytics engine built successfully"
}

# Build Go microservices
build_go_services() {
    print_status "Building Go microservices..."
    
    cd go
    if ! go mod download; then
        print_error "Go module download failed"
        exit 1
    fi
    
    if ! go build -o bin/server ./cmd/server; then
        print_error "Go compilation failed"
        exit 1
    fi
    cd ..
    
    print_success "Go microservices built successfully"
}

# Build web interface
build_web_interface() {
    print_status "Building web interface..."
    
    cd web
    if ! npm install; then
        print_error "npm install failed"
        exit 1
    fi
    
    if ! npm run build; then
        print_error "Web interface build failed"
        exit 1
    fi
    cd ..
    
    print_success "Web interface built successfully"
}

# Run tests
run_tests() {
    print_status "Running tests..."
    
    # C++ tests
    if [ -f "build/router_tests" ]; then
        print_status "Running C++ tests..."
        cd build
        if ! ./router_tests; then
            print_warning "Some C++ tests failed"
        fi
        cd ..
    fi
    
    # Rust tests
    print_status "Running Rust tests..."
    cd rust
    if ! cargo test; then
        print_warning "Some Rust tests failed"
    fi
    cd ..
    
    # Go tests
    print_status "Running Go tests..."
    cd go
    if ! go test ./...; then
        print_warning "Some Go tests failed"
    fi
    cd ..
    
    print_success "Tests completed"
}

# Main build process
main() {
    echo "╔══════════════════════════════════════════════════════════════╗"
    echo "║                Multi-Protocol Router Simulator              ║"
    echo "║                        Build Script                         ║"
    echo "╚══════════════════════════════════════════════════════════════╝"
    echo
    
    # Parse command line arguments
    RUN_TESTS=false
    CLEAN_BUILD=false
    
    while [[ $# -gt 0 ]]; do
        case $1 in
            --test)
                RUN_TESTS=true
                shift
                ;;
            --clean)
                CLEAN_BUILD=true
                shift
                ;;
            --help)
                echo "Usage: $0 [--test] [--clean] [--help]"
                echo "  --test    Run tests after building"
                echo "  --clean   Clean build directories before building"
                echo "  --help    Show this help message"
                exit 0
                ;;
            *)
                print_error "Unknown option: $1"
                exit 1
                ;;
        esac
    done
    
    # Clean build if requested
    if [ "$CLEAN_BUILD" = true ]; then
        print_status "Cleaning build directories..."
        rm -rf build/
        rm -rf rust/target/
        rm -rf web/node_modules/
        rm -rf web/dist/
        rm -rf go/bin/
    fi
    
    # Check dependencies
    check_dependencies
    
    # Build components
    build_cpp_core
    build_rust_analytics
    build_go_services
    build_web_interface
    
    # Run tests if requested
    if [ "$RUN_TESTS" = true ]; then
        run_tests
    fi
    
    print_success "Build completed successfully!"
    echo
    print_status "To run the router simulator:"
    print_status "  ./build/router_sim --config config.yaml"
    print_status
    print_status "To start the web interface:"
    print_status "  cd web && npm start"
    print_status
    print_status "To run with a scenario:"
    print_status "  ./build/router_sim --scenario scenarios/bgp_convergence.yaml"
}

# Run main function
main "$@"
