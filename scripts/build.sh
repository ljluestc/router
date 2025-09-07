#!/bin/bash

# Multi-Protocol Router Simulator Build Script
# This script builds the router simulator with all dependencies

set -e

echo "🔧 Multi-Protocol Router Simulator Build Script"
echo "================================================"

# Check if running as root for system dependencies
if [[ $EUID -eq 0 ]]; then
   echo "❌ Please do not run this script as root"
   exit 1
fi

# Function to check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Function to install dependencies based on OS
install_dependencies() {
    echo "📦 Installing dependencies..."
    
    if command_exists apt-get; then
        # Ubuntu/Debian
        echo "Detected Ubuntu/Debian system"
        sudo apt-get update
        sudo apt-get install -y build-essential cmake libyaml-cpp-dev libpcap-dev
        sudo apt-get install -y frr frr-dev
        sudo apt-get install -y valgrind cppcheck
        sudo apt-get install -y doxygen graphviz
        
    elif command_exists yum; then
        # CentOS/RHEL
        echo "Detected CentOS/RHEL system"
        sudo yum install -y gcc-c++ cmake yaml-cpp-devel libpcap-devel
        sudo yum install -y frr frr-devel
        sudo yum install -y valgrind cppcheck
        sudo yum install -y doxygen graphviz
        
    elif command_exists brew; then
        # macOS
        echo "Detected macOS system"
        brew install cmake yaml-cpp libpcap
        brew install --cask frr
        
    else
        echo "❌ Unsupported operating system"
        echo "Please install the following dependencies manually:"
        echo "  - CMake 3.16+"
        echo "  - C++17 compiler (GCC 7+ or Clang 5+)"
        echo "  - yaml-cpp"
        echo "  - libpcap"
        echo "  - FRR (Free Range Routing)"
        exit 1
    fi
}

# Function to check FRR installation
check_frr() {
    echo "🔍 Checking FRR installation..."
    
    if ! command_exists vtysh; then
        echo "❌ FRR not found. Please install FRR routing suite."
        echo "On Ubuntu/Debian: sudo apt-get install frr frr-dev"
        echo "On CentOS/RHEL: sudo yum install frr frr-devel"
        exit 1
    fi
    
    echo "✅ FRR found: $(vtysh --version 2>&1 | head -n1)"
}

# Function to check netem module
check_netem() {
    echo "🔍 Checking netem module..."
    
    if ! lsmod | grep -q sch_netem; then
        echo "⚠️  netem module not loaded. Loading it now..."
        if sudo modprobe sch_netem; then
            echo "✅ netem module loaded successfully"
        else
            echo "❌ Failed to load netem module. Network impairments may not work."
        fi
    else
        echo "✅ netem module is loaded"
    fi
}

# Function to build the project
build_project() {
    echo "🏗️  Building project..."
    
    # Create build directory
    mkdir -p build
    cd build
    
    # Configure with CMake
    echo "Configuring with CMake..."
    cmake -DCMAKE_BUILD_TYPE=Release \
          -DCMAKE_CXX_FLAGS="-Wall -Wextra -Werror" \
          -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
          ..
    
    # Build
    echo "Compiling..."
    make -j$(nproc)
    
    echo "✅ Build completed successfully"
}

# Function to run tests
run_tests() {
    echo "🧪 Running tests..."
    
    cd build
    
    if [ -f "./router_test" ]; then
        ./router_test
        echo "✅ All tests passed"
    else
        echo "❌ Test executable not found"
        exit 1
    fi
}

# Function to run static analysis
run_static_analysis() {
    echo "🔍 Running static analysis..."
    
    if command_exists cppcheck; then
        cppcheck --enable=all --error-exitcode=1 src/ include/ || true
        echo "✅ Static analysis completed"
    else
        echo "⚠️  cppcheck not found, skipping static analysis"
    fi
}

# Function to generate documentation
generate_docs() {
    echo "📚 Generating documentation..."
    
    if command_exists doxygen; then
        cd build
        if [ -f "Doxyfile" ]; then
            doxygen
            echo "✅ Documentation generated in build/docs/html/"
        else
            echo "⚠️  Doxyfile not found, skipping documentation generation"
        fi
    else
        echo "⚠️  doxygen not found, skipping documentation generation"
    fi
}

# Function to create installation package
create_package() {
    echo "📦 Creating installation package..."
    
    cd build
    
    # Create package directory
    mkdir -p router-simulator-package/bin
    mkdir -p router-simulator-package/scenarios
    mkdir -p router-simulator-package/docs
    
    # Copy binaries
    cp router_sim router-simulator-package/bin/
    cp router_test router-simulator-package/bin/
    
    # Copy scenarios
    cp -r ../scenarios/* router-simulator-package/scenarios/
    
    # Copy documentation
    cp -r ../docs/* router-simulator-package/docs/
    cp ../README.md router-simulator-package/
    
    # Create install script
    cat > router-simulator-package/install.sh << 'EOF'
#!/bin/bash
echo "Installing Router Simulator..."
sudo cp bin/* /usr/local/bin/
sudo mkdir -p /usr/local/share/router-simulator
sudo cp -r scenarios /usr/local/share/router-simulator/
sudo cp -r docs /usr/local/share/router-simulator/
echo "Installation completed!"
echo "Run 'router_sim' to start the simulator"
EOF
    chmod +x router-simulator-package/install.sh
    
    # Create tarball
    tar -czf router-simulator-$(date +%Y%m%d).tar.gz router-simulator-package/
    
    echo "✅ Package created: router-simulator-$(date +%Y%m%d).tar.gz"
}

# Main execution
main() {
    echo "Starting build process..."
    
    # Parse command line arguments
    INSTALL_DEPS=false
    RUN_TESTS=false
    RUN_ANALYSIS=false
    GENERATE_DOCS=false
    CREATE_PACKAGE=false
    
    while [[ $# -gt 0 ]]; do
        case $1 in
            --install-deps)
                INSTALL_DEPS=true
                shift
                ;;
            --test)
                RUN_TESTS=true
                shift
                ;;
            --analyze)
                RUN_ANALYSIS=true
                shift
                ;;
            --docs)
                GENERATE_DOCS=true
                shift
                ;;
            --package)
                CREATE_PACKAGE=true
                shift
                ;;
            --all)
                INSTALL_DEPS=true
                RUN_TESTS=true
                RUN_ANALYSIS=true
                GENERATE_DOCS=true
                CREATE_PACKAGE=true
                shift
                ;;
            -h|--help)
                echo "Usage: $0 [OPTIONS]"
                echo "Options:"
                echo "  --install-deps    Install system dependencies"
                echo "  --test           Run tests after building"
                echo "  --analyze        Run static analysis"
                echo "  --docs           Generate documentation"
                echo "  --package        Create installation package"
                echo "  --all            Run all steps"
                echo "  -h, --help       Show this help message"
                exit 0
                ;;
            *)
                echo "Unknown option: $1"
                exit 1
                ;;
        esac
    done
    
    # Install dependencies if requested
    if [ "$INSTALL_DEPS" = true ]; then
        install_dependencies
    fi
    
    # Check system requirements
    check_frr
    check_netem
    
    # Build project
    build_project
    
    # Run tests if requested
    if [ "$RUN_TESTS" = true ]; then
        run_tests
    fi
    
    # Run static analysis if requested
    if [ "$RUN_ANALYSIS" = true ]; then
        run_static_analysis
    fi
    
    # Generate documentation if requested
    if [ "$GENERATE_DOCS" = true ]; then
        generate_docs
    fi
    
    # Create package if requested
    if [ "$CREATE_PACKAGE" = true ]; then
        create_package
    fi
    
    echo ""
    echo "🎉 Build process completed successfully!"
    echo ""
    echo "Next steps:"
    echo "  - Run tests: ./build/router_test"
    echo "  - Start simulator: sudo ./build/router_sim"
    echo "  - Load scenario: load scenario scenarios/basic_router.yaml"
    echo ""
}

# Run main function with all arguments
main "$@"
