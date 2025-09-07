#!/bin/bash

# Multi-Protocol Router Simulator Build Script
# This script builds the router simulator with all dependencies

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
BUILD_TYPE=${BUILD_TYPE:-Release}
BUILD_DIR=${BUILD_DIR:-build}
INSTALL_PREFIX=${INSTALL_PREFIX:-/usr/local}
ENABLE_COVERAGE=${ENABLE_COVERAGE:-OFF}
ENABLE_TESTS=${ENABLE_TESTS:-ON}
ENABLE_DOCS=${ENABLE_DOCS:-OFF}

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
    
    # Check for required commands
    if ! command_exists cmake; then
        missing_deps+=("cmake")
    fi
    
    if ! command_exists make; then
        missing_deps+=("make")
    fi
    
    if ! command_exists g++; then
        missing_deps+=("g++")
    fi
    
    # Check for required libraries
    if ! pkg-config --exists libpcap; then
        missing_deps+=("libpcap-dev")
    fi
    
    if ! pkg-config --exists yaml-cpp; then
        missing_deps+=("libyaml-cpp-dev")
    fi
    
    if ! command_exists tc; then
        missing_deps+=("iproute2")
    fi
    
    if ! command_exists vtysh; then
        missing_deps+=("frr")
    fi
    
    if [ ${#missing_deps[@]} -ne 0 ]; then
        print_error "Missing dependencies: ${missing_deps[*]}"
        print_status "Install them with:"
        print_status "sudo apt-get install -y ${missing_deps[*]}"
        exit 1
    fi
    
    print_success "All dependencies found"
}

# Function to install dependencies
install_dependencies() {
    print_status "Installing dependencies..."
    
    if command_exists apt-get; then
        sudo apt-get update
        sudo apt-get install -y \
            build-essential \
            cmake \
            libpcap-dev \
            libyaml-cpp-dev \
            frr \
            iproute2 \
            net-tools \
            pkg-config
    elif command_exists yum; then
        sudo yum install -y \
            gcc-c++ \
            cmake \
            libpcap-devel \
            yaml-cpp-devel \
            frr \
            iproute \
            net-tools \
            pkgconfig
    elif command_exists brew; then
        brew install cmake libpcap yaml-cpp
    else
        print_warning "Unknown package manager. Please install dependencies manually."
    fi
}

# Function to create build directory
create_build_dir() {
    print_status "Creating build directory..."
    
    if [ -d "$BUILD_DIR" ]; then
        print_warning "Build directory exists. Cleaning..."
        rm -rf "$BUILD_DIR"
    fi
    
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
}

# Function to configure with CMake
configure_cmake() {
    print_status "Configuring with CMake..."
    
    local cmake_args=(
        -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
        -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX"
        -DENABLE_COVERAGE="$ENABLE_COVERAGE"
        -DENABLE_TESTS="$ENABLE_TESTS"
        -DENABLE_DOCS="$ENABLE_DOCS"
    )
    
    if [ "$ENABLE_COVERAGE" = "ON" ]; then
        cmake_args+=(-DCMAKE_CXX_FLAGS="--coverage")
    fi
    
    cmake "${cmake_args[@]}" ..
    
    print_success "CMake configuration complete"
}

# Function to build the project
build_project() {
    print_status "Building project..."
    
    local jobs=$(nproc)
    if [ "$jobs" -gt 8 ]; then
        jobs=8
    fi
    
    make -j"$jobs"
    
    print_success "Build complete"
}

# Function to run tests
run_tests() {
    if [ "$ENABLE_TESTS" = "ON" ]; then
        print_status "Running tests..."
        
        if [ -f "router_sim_test" ]; then
            ./router_sim_test
            print_success "Tests passed"
        else
            print_warning "Test executable not found"
        fi
    fi
}

# Function to install the project
install_project() {
    print_status "Installing project..."
    
    sudo make install
    
    print_success "Installation complete"
}

# Function to generate coverage report
generate_coverage() {
    if [ "$ENABLE_COVERAGE" = "ON" ]; then
        print_status "Generating coverage report..."
        
        if command_exists gcov; then
            gcov -r .
            if command_exists lcov; then
                lcov --capture --directory . --output-file coverage.info
                lcov --remove coverage.info '/usr/*' --output-file coverage.info
                lcov --list coverage.info
            fi
        fi
        
        print_success "Coverage report generated"
    fi
}

# Function to create package
create_package() {
    print_status "Creating package..."
    
    if command_exists cpack; then
        cpack
        print_success "Package created"
    else
        print_warning "CPack not available"
    fi
}

# Function to clean up
cleanup() {
    print_status "Cleaning up..."
    
    if [ -d "$BUILD_DIR" ]; then
        rm -rf "$BUILD_DIR"
    fi
    
    print_success "Cleanup complete"
}

# Function to show help
show_help() {
    echo "Multi-Protocol Router Simulator Build Script"
    echo ""
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  -h, --help              Show this help message"
    echo "  -d, --deps              Install dependencies"
    echo "  -c, --clean             Clean build directory"
    echo "  -t, --test              Run tests"
    echo "  -i, --install           Install the project"
    echo "  -p, --package           Create package"
    echo "  -g, --coverage          Enable coverage reporting"
    echo "  --build-type TYPE       Set build type (Debug, Release, RelWithDebInfo)"
    echo "  --build-dir DIR         Set build directory"
    echo "  --install-prefix PREFIX Set install prefix"
    echo "  --no-tests              Disable tests"
    echo "  --docs                  Enable documentation"
    echo ""
    echo "Environment variables:"
    echo "  BUILD_TYPE              Build type (default: Release)"
    echo "  BUILD_DIR               Build directory (default: build)"
    echo "  INSTALL_PREFIX          Install prefix (default: /usr/local)"
    echo "  ENABLE_COVERAGE         Enable coverage (default: OFF)"
    echo "  ENABLE_TESTS            Enable tests (default: ON)"
    echo "  ENABLE_DOCS             Enable docs (default: OFF)"
}

# Main function
main() {
    local install_deps=false
    local clean_build=false
    local run_tests_flag=false
    local install_flag=false
    local package_flag=false
    local coverage_flag=false
    local no_tests=false
    local docs_flag=false
    
    # Parse command line arguments
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                show_help
                exit 0
                ;;
            -d|--deps)
                install_deps=true
                shift
                ;;
            -c|--clean)
                clean_build=true
                shift
                ;;
            -t|--test)
                run_tests_flag=true
                shift
                ;;
            -i|--install)
                install_flag=true
                shift
                ;;
            -p|--package)
                package_flag=true
                shift
                ;;
            -g|--coverage)
                coverage_flag=true
                ENABLE_COVERAGE=ON
                shift
                ;;
            --build-type)
                BUILD_TYPE="$2"
                shift 2
                ;;
            --build-dir)
                BUILD_DIR="$2"
                shift 2
                ;;
            --install-prefix)
                INSTALL_PREFIX="$2"
                shift 2
                ;;
            --no-tests)
                no_tests=true
                ENABLE_TESTS=OFF
                shift
                ;;
            --docs)
                docs_flag=true
                ENABLE_DOCS=ON
                shift
                ;;
            *)
                print_error "Unknown option: $1"
                show_help
                exit 1
                ;;
        esac
    done
    
    # Execute actions
    if [ "$install_deps" = true ]; then
        install_dependencies
    fi
    
    if [ "$clean_build" = true ]; then
        cleanup
        exit 0
    fi
    
    check_dependencies
    create_build_dir
    configure_cmake
    build_project
    
    if [ "$run_tests_flag" = true ] || [ "$ENABLE_TESTS" = "ON" ]; then
        run_tests
    fi
    
    if [ "$install_flag" = true ]; then
        install_project
    fi
    
    if [ "$package_flag" = true ]; then
        create_package
    fi
    
    if [ "$coverage_flag" = true ]; then
        generate_coverage
    fi
    
    print_success "Build process completed successfully!"
    print_status "Build directory: $BUILD_DIR"
    print_status "Build type: $BUILD_TYPE"
    print_status "Install prefix: $INSTALL_PREFIX"
}

# Run main function with all arguments
main "$@"
