#!/bin/bash

# Router Simulator Demo Script
set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

print_header() {
    echo -e "${BLUE}================================================${NC}"
    echo -e "${BLUE}  Router Simulator Multi-Cloud Demo${NC}"
    echo -e "${BLUE}  C++ | Rust | Go | TypeScript | ClickHouse${NC}"
    echo -e "${BLUE}================================================${NC}"
}

print_step() {
    echo -e "${GREEN}[STEP]${NC} $1"
}

print_info() {
    echo -e "${CYAN}[INFO]${NC} $1"
}

check_dependencies() {
    print_step "Checking dependencies..."
    local missing_deps=()
    
    command -v cmake >/dev/null 2>&1 || missing_deps+=("cmake")
    command -v cargo >/dev/null 2>&1 || missing_deps+=("cargo")
    command -v go >/dev/null 2>&1 || missing_deps+=("go")
    command -v node >/dev/null 2>&1 || missing_deps+=("node")
    command -v docker >/dev/null 2>&1 || missing_deps+=("docker")
    
    if [ ${#missing_deps[@]} -ne 0 ]; then
        echo -e "${RED}[ERROR]${NC} Missing dependencies: ${missing_deps[*]}"
        exit 1
    fi
    print_info "All dependencies found ✓"
}

build_all() {
    print_step "Building all components..."
    
    # Build C++
    print_info "Building C++ router simulator..."
    mkdir -p build && cd build
    cmake .. -DCMAKE_BUILD_TYPE=Release
    make -j$(nproc)
    cd ..
    
    # Build Rust
    print_info "Building Rust analytics..."
    cd rust && cargo build --release && cd ..
    
    # Build Go
    print_info "Building Go services..."
    cd go && go build -o bin/cloudpods-service ./cmd/cloudpods && cd ..
    
    # Build Web
    print_info "Building web UI..."
    cd web && npm install && npm run build && cd ..
    
    print_info "All components built ✓"
}

start_services() {
    print_step "Starting services..."
    
    # Start ClickHouse
    docker run -d --name clickhouse-demo -p 9000:9000 clickhouse/clickhouse-server:latest
    
    # Start web server
    cd web && python3 -m http.server 8080 &
    WEB_PID=$!
    cd ..
    
    print_info "Services started ✓"
    echo "Web UI: http://localhost:8080"
    echo "ClickHouse: http://localhost:9000"
}

main() {
    print_header
    check_dependencies
    build_all
    start_services
    
    print_info "Demo is running! Press Ctrl+C to stop."
    wait
}

main "$@"