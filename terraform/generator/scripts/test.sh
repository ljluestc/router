#!/bin/bash

# Test Terraform provider generator
set -e

echo "Testing Terraform Provider Generator"
echo "==================================="
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

# Create test directory
mkdir -p test
cd test

# Test 1: Basic functionality
echo "Test 1: Basic functionality"
echo "=========================="
echo ""

# Build generator
echo "Building generator..."
go build -o terraform-generator ../main.go
echo "✓ Generator built successfully"
echo ""

# Test 2: Provider generation
echo "Test 2: Provider generation"
echo "=========================="
echo ""

# Generate CloudPods provider
echo "Generating CloudPods provider..."
time ../terraform-generator cloudpods ./cloudpods
if [ -d "cloudpods" ]; then
    echo "✓ CloudPods provider generated"
else
    echo "✗ CloudPods provider generation failed"
    exit 1
fi
echo ""

# Generate Aviatrix provider
echo "Generating Aviatrix provider..."
time ../terraform-generator aviatrix ./aviatrix
if [ -d "aviatrix" ]; then
    echo "✓ Aviatrix provider generated"
else
    echo "✗ Aviatrix provider generation failed"
    exit 1
fi
echo ""

# Generate Router Simulator provider
echo "Generating Router Simulator provider..."
time ../terraform-generator router-sim ./router-sim
if [ -d "router-sim" ]; then
    echo "✓ Router Simulator provider generated"
else
    echo "✗ Router Simulator provider generation failed"
    exit 1
fi
echo ""

# Test 3: File validation
echo "Test 3: File validation"
echo "======================"
echo ""

# Check required files for each provider
for provider in cloudpods aviatrix router-sim; do
    echo "Checking $provider provider files..."
    
    # Check main.tf
    if [ -f "$provider/main.tf" ]; then
        echo "  ✓ main.tf exists"
    else
        echo "  ✗ main.tf missing"
        exit 1
    fi
    
    # Check variables.tf
    if [ -f "$provider/variables.tf" ]; then
        echo "  ✓ variables.tf exists"
    else
        echo "  ✗ variables.tf missing"
        exit 1
    fi
    
    # Check outputs.tf
    if [ -f "$provider/outputs.tf" ]; then
        echo "  ✓ outputs.tf exists"
    else
        echo "  ✗ outputs.tf missing"
        exit 1
    fi
    
    # Check provider_schema.json
    if [ -f "$provider/provider_schema.json" ]; then
        echo "  ✓ provider_schema.json exists"
    else
        echo "  ✗ provider_schema.json missing"
        exit 1
    fi
    
    # Check examples/basic.tf
    if [ -f "$provider/examples/basic.tf" ]; then
        echo "  ✓ examples/basic.tf exists"
    else
        echo "  ✗ examples/basic.tf missing"
        exit 1
    fi
    
    echo ""
done

# Test 4: Terraform validation
echo "Test 4: Terraform validation"
echo "==========================="
echo ""

# Validate CloudPods provider
echo "Validating CloudPods provider..."
terraform -chdir=cloudpods init
terraform -chdir=cloudpods validate
echo "✓ CloudPods provider is valid"
echo ""

# Validate Aviatrix provider
echo "Validating Aviatrix provider..."
terraform -chdir=aviatrix init
terraform -chdir=aviatrix validate
echo "✓ Aviatrix provider is valid"
echo ""

# Validate Router Simulator provider
echo "Validating Router Simulator provider..."
terraform -chdir=router-sim init
terraform -chdir=router-sim validate
echo "✓ Router Simulator provider is valid"
echo ""

# Test 5: Performance testing
echo "Test 5: Performance testing"
echo "=========================="
echo ""

# Test generation speed
echo "Testing generation speed..."
time (
    ../terraform-generator cloudpods ./cloudpods_perf &
    ../terraform-generator aviatrix ./aviatrix_perf &
    ../terraform-generator router-sim ./router-sim_perf &
    wait
)
echo "✓ Performance test completed"
echo ""

# Test 6: Error handling
echo "Test 6: Error handling"
echo "====================="
echo ""

# Test invalid provider
echo "Testing invalid provider..."
if ../terraform-generator invalid-provider ./invalid 2>/dev/null; then
    echo "✗ Invalid provider should have failed"
    exit 1
else
    echo "✓ Invalid provider correctly failed"
fi
echo ""

# Test invalid output directory
echo "Testing invalid output directory..."
if ../terraform-generator cloudpods /invalid/path 2>/dev/null; then
    echo "✗ Invalid output directory should have failed"
    exit 1
else
    echo "✓ Invalid output directory correctly failed"
fi
echo ""

# Test 7: Concurrent generation
echo "Test 7: Concurrent generation"
echo "============================"
echo ""

# Test concurrent generation
echo "Testing concurrent generation..."
time (
    ../terraform-generator cloudpods ./cloudpods_concurrent &
    ../terraform-generator aviatrix ./aviatrix_concurrent &
    ../terraform-generator router-sim ./router-sim_concurrent &
    wait
)
echo "✓ Concurrent generation test completed"
echo ""

# Test 8: Memory usage
echo "Test 8: Memory usage"
echo "==================="
echo ""

# Test memory usage
echo "Testing memory usage..."
/usr/bin/time -v ../terraform-generator cloudpods ./cloudpods_memory 2>&1 | grep -E "(Maximum resident set size|Average total memory use)" || echo "Memory usage information not available"
echo ""

# Test 9: Disk usage
echo "Test 9: Disk usage"
echo "================="
echo ""

# Test disk usage
echo "Testing disk usage..."
du -sh cloudpods aviatrix router-sim
echo ""

# Test 10: Cleanup
echo "Test 10: Cleanup"
echo "==============="
echo ""

# Cleanup test files
echo "Cleaning up test files..."
rm -rf cloudpods aviatrix router-sim cloudpods_perf aviatrix_perf router-sim_perf cloudpods_concurrent aviatrix_concurrent router-sim_concurrent cloudpods_memory
rm -f terraform-generator
cd ..
rm -rf test

echo "✓ Test cleanup completed"
echo ""

# Test summary
echo "Test Summary"
echo "==========="
echo "✓ All tests passed successfully!"
echo ""
echo "Tested features:"
echo "  - Basic functionality"
echo "  - Provider generation"
echo "  - File validation"
echo "  - Terraform validation"
echo "  - Performance testing"
echo "  - Error handling"
echo "  - Concurrent generation"
echo "  - Memory usage"
echo "  - Disk usage"
echo "  - Cleanup"
echo ""
echo "Testing completed successfully!"
