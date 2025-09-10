#!/bin/bash

# Benchmark Terraform provider generator
set -e

echo "Benchmarking Terraform Provider Generator"
echo "======================================="
echo ""

# Check if Go is installed
if ! command -v go &> /dev/null; then
    echo "Go is not installed. Please install Go first."
    exit 1
fi

# Check if terraform is installed
if ! command -v terraform &> /dev/null; then
    echo "Terraform is not installed. Please install Terraform first."
    exit 1
fi

# Create benchmark directory
mkdir -p benchmark
cd benchmark

# Benchmark Go build
echo "Benchmarking Go build..."
echo "======================="
echo ""

# Clean build
echo "Clean build:"
time go build -o terraform-generator ../main.go
echo ""

# Incremental build
echo "Incremental build:"
time go build -o terraform-generator ../main.go
echo ""

# Optimized build
echo "Optimized build:"
time go build -ldflags="-s -w" -o terraform-generator ../main.go
echo ""

# Benchmark provider generation
echo "Benchmarking provider generation..."
echo "=================================="
echo ""

# CloudPods provider
echo "CloudPods provider:"
time ../terraform-generator cloudpods ./cloudpods
echo ""

# Aviatrix provider
echo "Aviatrix provider:"
time ../terraform-generator aviatrix ./aviatrix
echo ""

# Router Simulator provider
echo "Router Simulator provider:"
time ../terraform-generator router-sim ./router-sim
echo ""

# Benchmark Terraform validation
echo "Benchmarking Terraform validation..."
echo "==================================="
echo ""

# CloudPods validation
echo "CloudPods validation:"
time terraform -chdir=cloudpods init
time terraform -chdir=cloudpods validate
echo ""

# Aviatrix validation
echo "Aviatrix validation:"
time terraform -chdir=aviatrix init
time terraform -chdir=aviatrix validate
echo ""

# Router Simulator validation
echo "Router Simulator validation:"
time terraform -chdir=router-sim init
time terraform -chdir=router-sim validate
echo ""

# Benchmark file operations
echo "Benchmarking file operations..."
echo "=============================="
echo ""

# File creation
echo "File creation:"
time for i in {1..100}; do echo "test $i" > "test_$i.txt"; done
echo ""

# File deletion
echo "File deletion:"
time rm -f test_*.txt
echo ""

# Directory operations
echo "Directory operations:"
time mkdir -p test_dir
time cp -r cloudpods test_dir/
time rm -rf test_dir
echo ""

# Benchmark memory usage
echo "Benchmarking memory usage..."
echo "==========================="
echo ""

# Memory usage during build
echo "Memory usage during build:"
/usr/bin/time -v go build -o terraform-generator ../main.go 2>&1 | grep -E "(Maximum resident set size|Average total memory use)"
echo ""

# Memory usage during generation
echo "Memory usage during generation:"
/usr/bin/time -v ../terraform-generator cloudpods ./cloudpods 2>&1 | grep -E "(Maximum resident set size|Average total memory use)"
echo ""

# Benchmark disk usage
echo "Benchmarking disk usage..."
echo "========================="
echo ""

# Show disk usage
echo "Disk usage:"
du -sh cloudpods aviatrix router-sim
echo ""

# Benchmark network operations
echo "Benchmarking network operations..."
echo "================================="
echo ""

# Test network connectivity
echo "Network connectivity test:"
time ping -c 3 8.8.8.8 > /dev/null 2>&1 && echo "✓ Network is reachable" || echo "✗ Network is not reachable"
echo ""

# Test DNS resolution
echo "DNS resolution test:"
time nslookup google.com > /dev/null 2>&1 && echo "✓ DNS is working" || echo "✗ DNS is not working"
echo ""

# Benchmark concurrent operations
echo "Benchmarking concurrent operations..."
echo "===================================="
echo ""

# Concurrent provider generation
echo "Concurrent provider generation:"
time (
    ../terraform-generator cloudpods ./cloudpods_concurrent &
    ../terraform-generator aviatrix ./aviatrix_concurrent &
    ../terraform-generator router-sim ./router-sim_concurrent &
    wait
)
echo ""

# Concurrent Terraform validation
echo "Concurrent Terraform validation:"
time (
    terraform -chdir=cloudpods_concurrent init &
    terraform -chdir=aviatrix_concurrent init &
    terraform -chdir=router-sim_concurrent init &
    wait
)
echo ""

# Cleanup
echo "Cleaning up benchmark files..."
rm -rf cloudpods aviatrix router-sim cloudpods_concurrent aviatrix_concurrent router-sim_concurrent
rm -f terraform-generator
cd ..
rm -rf benchmark

echo ""
echo "Benchmark completed successfully!"
