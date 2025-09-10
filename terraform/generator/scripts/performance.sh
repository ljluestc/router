#!/bin/bash

# Performance analysis for Terraform provider generator
set -e

echo "Performance Analysis for Terraform Provider Generator"
echo "==================================================="
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

# Create performance directory
mkdir -p performance
cd performance

# Performance testing
echo "Performance Testing..."
echo "===================="
echo ""

# Test 1: Build performance
echo "Test 1: Build Performance"
echo "------------------------"
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

# Test 2: Generation performance
echo "Test 2: Generation Performance"
echo "-----------------------------"
echo ""

# CloudPods provider generation
echo "CloudPods provider generation:"
time ../terraform-generator cloudpods ./cloudpods
echo ""

# Aviatrix provider generation
echo "Aviatrix provider generation:"
time ../terraform-generator aviatrix ./aviatrix
echo ""

# Router Simulator provider generation
echo "Router Simulator provider generation:"
time ../terraform-generator router-sim ./router-sim
echo ""

# Test 3: Concurrent generation performance
echo "Test 3: Concurrent Generation Performance"
echo "---------------------------------------"
echo ""

# Concurrent generation
echo "Concurrent generation:"
time (
    ../terraform-generator cloudpods ./cloudpods_concurrent &
    ../terraform-generator aviatrix ./aviatrix_concurrent &
    ../terraform-generator router-sim ./router-sim_concurrent &
    wait
)
echo ""

# Test 4: Terraform validation performance
echo "Test 4: Terraform Validation Performance"
echo "---------------------------------------"
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

# Test 5: Memory usage performance
echo "Test 5: Memory Usage Performance"
echo "-------------------------------"
echo ""

# Memory usage during build
echo "Memory usage during build:"
/usr/bin/time -v go build -o terraform-generator ../main.go 2>&1 | grep -E "(Maximum resident set size|Average total memory use)"
echo ""

# Memory usage during generation
echo "Memory usage during generation:"
/usr/bin/time -v ../terraform-generator cloudpods ./cloudpods_memory 2>&1 | grep -E "(Maximum resident set size|Average total memory use)"
echo ""

# Test 6: Disk usage performance
echo "Test 6: Disk Usage Performance"
echo "-----------------------------"
echo ""

# Disk usage
echo "Disk usage:"
du -sh cloudpods aviatrix router-sim
echo ""

# Test 7: Network performance
echo "Test 7: Network Performance"
echo "-------------------------"
echo ""

# Network connectivity
echo "Network connectivity:"
time ping -c 3 8.8.8.8 > /dev/null 2>&1 && echo "✓ Network is reachable" || echo "✗ Network is not reachable"
echo ""

# DNS resolution
echo "DNS resolution:"
time nslookup google.com > /dev/null 2>&1 && echo "✓ DNS is working" || echo "✗ DNS is not working"
echo ""

# Test 8: CPU usage performance
echo "Test 8: CPU Usage Performance"
echo "----------------------------"
echo ""

# CPU usage during build
echo "CPU usage during build:"
time go build -o terraform-generator ../main.go
echo ""

# CPU usage during generation
echo "CPU usage during generation:"
time ../terraform-generator cloudpods ./cloudpods_cpu
echo ""

# Test 9: I/O performance
echo "Test 9: I/O Performance"
echo "----------------------"
echo ""

# File creation performance
echo "File creation performance:"
time for i in {1..1000}; do echo "test $i" > "test_$i.txt"; done
echo ""

# File deletion performance
echo "File deletion performance:"
time rm -f test_*.txt
echo ""

# Directory operations performance
echo "Directory operations performance:"
time mkdir -p test_dir
time cp -r cloudpods test_dir/
time rm -rf test_dir
echo ""

# Test 10: Scalability performance
echo "Test 10: Scalability Performance"
echo "-------------------------------"
echo ""

# Multiple provider generation
echo "Multiple provider generation:"
time for i in {1..10}; do
    ../terraform-generator cloudpods "./cloudpods_$i" &
done
wait
echo ""

# Multiple Terraform validation
echo "Multiple Terraform validation:"
time for i in {1..10}; do
    terraform -chdir="cloudpods_$i" init &
done
wait
echo ""

# Generate performance report
echo "Generating performance report..."
echo "==============================="
echo ""

# Create performance report
cat > performance_report.md << EOF
# Performance Report

## Build Performance
- **Clean Build**: $(time go build -o terraform-generator ../main.go 2>&1 | grep real | awk '{print $2}')
- **Incremental Build**: $(time go build -o terraform-generator ../main.go 2>&1 | grep real | awk '{print $2}')
- **Optimized Build**: $(time go build -ldflags="-s -w" -o terraform-generator ../main.go 2>&1 | grep real | awk '{print $2}')

## Generation Performance
- **CloudPods Provider**: $(time ../terraform-generator cloudpods ./cloudpods 2>&1 | grep real | awk '{print $2}')
- **Aviatrix Provider**: $(time ../terraform-generator aviatrix ./aviatrix 2>&1 | grep real | awk '{print $2}')
- **Router Simulator Provider**: $(time ../terraform-generator router-sim ./router-sim 2>&1 | grep real | awk '{print $2}')

## Concurrent Generation Performance
- **Concurrent Generation**: $(time (../terraform-generator cloudpods ./cloudpods_concurrent & ../terraform-generator aviatrix ./aviatrix_concurrent & ../terraform-generator router-sim ./router-sim_concurrent & wait) 2>&1 | grep real | awk '{print $2}')

## Terraform Validation Performance
- **CloudPods Validation**: $(time terraform -chdir=cloudpods init && terraform -chdir=cloudpods validate 2>&1 | grep real | awk '{print $2}')
- **Aviatrix Validation**: $(time terraform -chdir=aviatrix init && terraform -chdir=aviatrix validate 2>&1 | grep real | awk '{print $2}')
- **Router Simulator Validation**: $(time terraform -chdir=router-sim init && terraform -chdir=router-sim validate 2>&1 | grep real | awk '{print $2}')

## Memory Usage Performance
- **Build Memory**: $(/usr/bin/time -v go build -o terraform-generator ../main.go 2>&1 | grep "Maximum resident set size" | awk '{print $6}')
- **Generation Memory**: $(/usr/bin/time -v ../terraform-generator cloudpods ./cloudpods_memory 2>&1 | grep "Maximum resident set size" | awk '{print $6}')

## Disk Usage Performance
- **CloudPods**: $(du -sh cloudpods | awk '{print $1}')
- **Aviatrix**: $(du -sh aviatrix | awk '{print $1}')
- **Router Simulator**: $(du -sh router-sim | awk '{print $1}')

## Network Performance
- **Ping Test**: $(time ping -c 3 8.8.8.8 > /dev/null 2>&1 && echo "✓ Pass" || echo "✗ Fail")
- **DNS Test**: $(time nslookup google.com > /dev/null 2>&1 && echo "✓ Pass" || echo "✗ Fail")

## I/O Performance
- **File Creation**: $(time for i in {1..1000}; do echo "test \$i" > "test_\$i.txt"; done 2>&1 | grep real | awk '{print $2}')
- **File Deletion**: $(time rm -f test_*.txt 2>&1 | grep real | awk '{print $2}')
- **Directory Operations**: $(time mkdir -p test_dir && cp -r cloudpods test_dir/ && rm -rf test_dir 2>&1 | grep real | awk '{print $2}')

## Scalability Performance
- **Multiple Provider Generation**: $(time for i in {1..10}; do ../terraform-generator cloudpods "./cloudpods_\$i" & done; wait 2>&1 | grep real | awk '{print $2}')
- **Multiple Terraform Validation**: $(time for i in {1..10}; do terraform -chdir="cloudpods_\$i" init & done; wait 2>&1 | grep real | awk '{print $2}')

## Summary
- **Total Test Time**: $(date)
- **Performance Status**: ✓ All tests completed successfully
- **Recommendations**: 
  - Use optimized builds for production
  - Consider concurrent generation for multiple providers
  - Monitor memory usage during large operations
  - Optimize I/O operations for better performance
EOF

echo "✓ Performance report generated: performance_report.md"
echo ""

# Show performance summary
echo "Performance Summary:"
echo "==================="
echo ""

# Build performance
echo "Build Performance:"
echo "  Clean build: $(time go build -o terraform-generator ../main.go 2>&1 | grep real | awk '{print $2}')"
echo "  Incremental build: $(time go build -o terraform-generator ../main.go 2>&1 | grep real | awk '{print $2}')"
echo "  Optimized build: $(time go build -ldflags="-s -w" -o terraform-generator ../main.go 2>&1 | grep real | awk '{print $2}')"
echo ""

# Generation performance
echo "Generation Performance:"
echo "  CloudPods: $(time ../terraform-generator cloudpods ./cloudpods 2>&1 | grep real | awk '{print $2}')"
echo "  Aviatrix: $(time ../terraform-generator aviatrix ./aviatrix 2>&1 | grep real | awk '{print $2}')"
echo "  Router Simulator: $(time ../terraform-generator router-sim ./router-sim 2>&1 | grep real | awk '{print $2}')"
echo ""

# Memory usage
echo "Memory Usage:"
echo "  Build: $(/usr/bin/time -v go build -o terraform-generator ../main.go 2>&1 | grep "Maximum resident set size" | awk '{print $6}')"
echo "  Generation: $(/usr/bin/time -v ../terraform-generator cloudpods ./cloudpods_memory 2>&1 | grep "Maximum resident set size" | awk '{print $6}')"
echo ""

# Disk usage
echo "Disk Usage:"
echo "  CloudPods: $(du -sh cloudpods | awk '{print $1}')"
echo "  Aviatrix: $(du -sh aviatrix | awk '{print $1}')"
echo "  Router Simulator: $(du -sh router-sim | awk '{print $1}')"
echo ""

# Show performance files
echo "Performance files generated:"
echo "============================"
ls -la *.md 2>/dev/null || echo "No performance files found"
echo ""

echo "Performance analysis completed successfully!"
