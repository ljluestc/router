#!/bin/bash

# Multi-Protocol Router Simulator Build Script
# This script builds all components of the router simulator

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
BUILD_TYPE=${BUILD_TYPE:-Release}
ENABLE_COVERAGE=${ENABLE_COVERAGE:-OFF}
ENABLE_CLOUDPODS=${ENABLE_CLOUDPODS:-ON}
ENABLE_AVIATRIX=${ENABLE_AVIATRIX:-ON}
ENABLE_RUST=${ENABLE_RUST:-ON}
ENABLE_GO=${ENABLE_GO:-ON}
PARALLEL_JOBS=${PARALLEL_JOBS:-$(nproc)}

# Directories
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_ROOT/build"
INSTALL_DIR="$PROJECT_ROOT/install"

echo -e "${BLUE}🌐 Multi-Protocol Router Simulator Build Script${NC}"
echo "=================================================="
echo "Build Type: $BUILD_TYPE"
echo "Coverage: $ENABLE_COVERAGE"
echo "CloudPods: $ENABLE_CLOUDPODS"
echo "Aviatrix: $ENABLE_AVIATRIX"
echo "Rust: $ENABLE_RUST"
echo "Go: $ENABLE_GO"
echo "Parallel Jobs: $PARALLEL_JOBS"
echo "=================================================="

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

# Function to check dependencies
check_dependencies() {
    echo -e "${BLUE}🔍 Checking dependencies...${NC}"
    
    # Check for required tools
    local missing_deps=()
    
    if ! command -v cmake &> /dev/null; then
        missing_deps+=("cmake")
    fi
    
    if ! command -v make &> /dev/null; then
        missing_deps+=("make")
    fi
    
    if ! command -v g++ &> /dev/null; then
        missing_deps+=("g++")
    fi
    
    if ! command -v pkg-config &> /dev/null; then
        missing_deps+=("pkg-config")
    fi
    
    if [ "$ENABLE_GO" = "ON" ] && ! command -v go &> /dev/null; then
        missing_deps+=("go")
    fi
    
    if [ "$ENABLE_RUST" = "ON" ] && ! command -v cargo &> /dev/null; then
        missing_deps+=("cargo")
    fi
    
    if [ ${#missing_deps[@]} -ne 0 ]; then
        print_error "Missing dependencies: ${missing_deps[*]}"
        echo "Please install the missing dependencies and try again."
        exit 1
    fi
    
    print_status "All dependencies found"
}

# Function to install system dependencies
install_system_deps() {
    echo -e "${BLUE}📦 Installing system dependencies...${NC}"
    
    if command -v apt-get &> /dev/null; then
        # Ubuntu/Debian
        sudo apt-get update
        sudo apt-get install -y \
            build-essential \
            cmake \
            git \
            pkg-config \
            libpcap-dev \
            libnl-3-dev \
            libzmq3-dev \
            libyaml-cpp-dev \
            libgtest-dev \
            frr \
            tc \
            golang-go \
            rustc \
            cargo \
            protobuf-compiler \
            libprotobuf-dev \
            libgrpc++-dev
    elif command -v yum &> /dev/null; then
        # CentOS/RHEL
        sudo yum install -y \
            gcc-c++ \
            cmake \
            git \
            pkgconfig \
            libpcap-devel \
            libnl3-devel \
            zeromq-devel \
            yaml-cpp-devel \
            gtest-devel \
            frr \
            iproute-tc \
            golang \
            rust \
            cargo \
            protobuf-compiler \
            protobuf-devel \
            grpc-devel
    else
        print_warning "Unknown package manager. Please install dependencies manually."
    fi
    
    print_status "System dependencies installed"
}

# Function to build C++ components
build_cpp() {
    echo -e "${BLUE}🔨 Building C++ components...${NC}"
    
    # Create build directory
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    
    # Configure with CMake
    cmake "$PROJECT_ROOT" \
        -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
        -DENABLE_COVERAGE="$ENABLE_COVERAGE" \
        -DENABLE_CLOUDPODS="$ENABLE_CLOUDPODS" \
        -DENABLE_AVIATRIX="$ENABLE_AVIATRIX" \
        -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR"
    
    # Build
    make -j"$PARALLEL_JOBS"
    
    # Install
    make install
    
    print_status "C++ components built successfully"
}

# Function to build Go components
build_go() {
    if [ "$ENABLE_GO" = "OFF" ]; then
        print_warning "Go components disabled"
        return
    fi
    
    echo -e "${BLUE}🔨 Building Go components...${NC}"
    
    cd "$PROJECT_ROOT/go"
    
    # Download dependencies
    go mod download
    
    # Build
    go build -o "$INSTALL_DIR/bin/cloudpods_client" ./cmd/cloudpods
    go build -o "$INSTALL_DIR/bin/aviatrix_client" ./cmd/aviatrix
    
    print_status "Go components built successfully"
}

# Function to build Rust components
build_rust() {
    if [ "$ENABLE_RUST" = "OFF" ]; then
        print_warning "Rust components disabled"
        return
    fi
    
    echo -e "${BLUE}🔨 Building Rust components...${NC}"
    
    cd "$PROJECT_ROOT/rust"
    
    # Build
    cargo build --release
    
    # Copy binaries
    cp target/release/router_analytics "$INSTALL_DIR/bin/"
    cp target/release/router_visualization "$INSTALL_DIR/bin/"
    
    print_status "Rust components built successfully"
}

# Function to run tests
run_tests() {
    echo -e "${BLUE}🧪 Running tests...${NC}"
    
    cd "$BUILD_DIR"
    
    # Run C++ tests
    if [ -f "router_tests" ]; then
        ./router_tests
        print_status "C++ tests passed"
    fi
    
    # Run Go tests
    if [ "$ENABLE_GO" = "ON" ]; then
        cd "$PROJECT_ROOT/go"
        go test ./...
        print_status "Go tests passed"
    fi
    
    # Run Rust tests
    if [ "$ENABLE_RUST" = "ON" ]; then
        cd "$PROJECT_ROOT/rust"
        cargo test
        print_status "Rust tests passed"
    fi
}

# Function to generate coverage report
generate_coverage() {
    if [ "$ENABLE_COVERAGE" = "OFF" ]; then
        return
    fi
    
    echo -e "${BLUE}📊 Generating coverage report...${NC}"
    
    cd "$BUILD_DIR"
    
    # Generate coverage data
    make coverage
    
    # Generate HTML report
    if command -v gcovr &> /dev/null; then
        gcovr -r "$PROJECT_ROOT" --html --html-details -o coverage_report.html
        print_status "Coverage report generated: $BUILD_DIR/coverage_report.html"
    else
        print_warning "gcovr not found. Install it to generate HTML coverage reports."
    fi
}

# Function to create installation package
create_package() {
    echo -e "${BLUE}📦 Creating installation package...${NC}"
    
    cd "$PROJECT_ROOT"
    
    # Create package directory
    PACKAGE_DIR="router-sim-$(date +%Y%m%d-%H%M%S)"
    mkdir -p "$PACKAGE_DIR"
    
    # Copy binaries
    cp -r "$INSTALL_DIR"/* "$PACKAGE_DIR/"
    
    # Copy configuration files
    cp -r scenarios "$PACKAGE_DIR/"
    cp -r docs "$PACKAGE_DIR/"
    cp README.md "$PACKAGE_DIR/"
    cp LICENSE "$PACKAGE_DIR/"
    
    # Create installation script
    cat > "$PACKAGE_DIR/install.sh" << 'EOF'
#!/bin/bash
# Installation script for Multi-Protocol Router Simulator

set -e

echo "Installing Multi-Protocol Router Simulator..."

# Copy binaries
sudo cp -r bin/* /usr/local/bin/
sudo cp -r lib/* /usr/local/lib/ 2>/dev/null || true
sudo cp -r share/* /usr/local/share/ 2>/dev/null || true

# Set permissions
sudo chmod +x /usr/local/bin/*

# Create systemd service
sudo tee /etc/systemd/system/router-sim.service > /dev/null << 'SERVICE_EOF'
[Unit]
Description=Multi-Protocol Router Simulator
After=network.target

[Service]
Type=simple
User=root
ExecStart=/usr/local/bin/router_sim
Restart=always
RestartSec=5

[Install]
WantedBy=multi-user.target
SERVICE_EOF

# Enable service
sudo systemctl daemon-reload
sudo systemctl enable router-sim

echo "Installation complete!"
echo "Start the service with: sudo systemctl start router-sim"
EOF
    
    chmod +x "$PACKAGE_DIR/install.sh"
    
    # Create tarball
    tar -czf "${PACKAGE_DIR}.tar.gz" "$PACKAGE_DIR"
    
    print_status "Package created: ${PACKAGE_DIR}.tar.gz"
}

# Function to clean build
clean_build() {
    echo -e "${BLUE}🧹 Cleaning build...${NC}"
    
    rm -rf "$BUILD_DIR"
    rm -rf "$INSTALL_DIR"
    
    if [ "$ENABLE_GO" = "ON" ]; then
        cd "$PROJECT_ROOT/go"
        go clean
    fi
    
    if [ "$ENABLE_RUST" = "ON" ]; then
        cd "$PROJECT_ROOT/rust"
        cargo clean
    fi
    
    print_status "Build cleaned"
}

# Function to show help
show_help() {
    cat << EOF
Usage: $0 [OPTIONS]

Options:
    -h, --help              Show this help message
    -c, --clean             Clean build before building
    -t, --test              Run tests after building
    -p, --package           Create installation package
    -d, --deps              Install system dependencies
    --no-go                 Disable Go components
    --no-rust               Disable Rust components
    --no-cloudpods          Disable CloudPods integration
    --no-aviatrix           Disable Aviatrix integration
    --coverage              Enable coverage reporting
    --debug                 Build in debug mode
    --jobs N                Number of parallel jobs (default: $(nproc))

Examples:
    $0                      # Build all components
    $0 --clean --test       # Clean, build, and test
    $0 --package            # Build and create package
    $0 --deps               # Install dependencies and build
    $0 --no-go --no-rust    # Build only C++ components
EOF
}

# Parse command line arguments
CLEAN=false
TEST=false
PACKAGE=false
INSTALL_DEPS=false

while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_help
            exit 0
            ;;
        -c|--clean)
            CLEAN=true
            shift
            ;;
        -t|--test)
            TEST=true
            shift
            ;;
        -p|--package)
            PACKAGE=true
            shift
            ;;
        -d|--deps)
            INSTALL_DEPS=true
            shift
            ;;
        --no-go)
            ENABLE_GO=OFF
            shift
            ;;
        --no-rust)
            ENABLE_RUST=OFF
            shift
            ;;
        --no-cloudpods)
            ENABLE_CLOUDPODS=OFF
            shift
            ;;
        --no-aviatrix)
            ENABLE_AVIATRIX=OFF
            shift
            ;;
        --coverage)
            ENABLE_COVERAGE=ON
            shift
            ;;
        --debug)
            BUILD_TYPE=Debug
            shift
            ;;
        --jobs)
            PARALLEL_JOBS="$2"
            shift 2
            ;;
        *)
            print_error "Unknown option: $1"
            show_help
            exit 1
            ;;
    esac
done

# Main build process
main() {
    # Check dependencies
    check_dependencies
    
    # Install system dependencies if requested
    if [ "$INSTALL_DEPS" = true ]; then
        install_system_deps
    fi
    
    # Clean if requested
    if [ "$CLEAN" = true ]; then
        clean_build
    fi
    
    # Build components
    build_cpp
    build_go
    build_rust
    
    # Run tests if requested
    if [ "$TEST" = true ]; then
        run_tests
    fi
    
    # Generate coverage if enabled
    generate_coverage
    
    # Create package if requested
    if [ "$PACKAGE" = true ]; then
        create_package
    fi
    
    print_status "Build completed successfully!"
    echo ""
    echo "Installation directory: $INSTALL_DIR"
    echo "Binaries:"
    ls -la "$INSTALL_DIR/bin/" 2>/dev/null || echo "No binaries found"
    echo ""
    echo "To run the router simulator:"
    echo "  sudo $INSTALL_DIR/bin/router_sim"
    echo ""
    echo "To run with a scenario:"
    echo "  sudo $INSTALL_DIR/bin/router_sim -s scenarios/cloud_networking_demo.yaml"
}

# Run main function
main "$@"
