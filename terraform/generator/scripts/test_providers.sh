#!/bin/bash

# Test generated Terraform providers
set -e

echo "Testing generated Terraform providers..."

# Test CloudPods provider
echo "Testing CloudPods provider..."
cd output/cloudpods
if [ -f "main.tf" ]; then
    echo "✓ main.tf exists"
else
    echo "✗ main.tf missing"
    exit 1
fi

if [ -f "variables.tf" ]; then
    echo "✓ variables.tf exists"
else
    echo "✗ variables.tf missing"
    exit 1
fi

if [ -f "outputs.tf" ]; then
    echo "✓ outputs.tf exists"
else
    echo "✗ outputs.tf missing"
    exit 1
fi

if [ -f "provider_schema.json" ]; then
    echo "✓ provider_schema.json exists"
else
    echo "✗ provider_schema.json missing"
    exit 1
fi

if [ -f "examples/basic.tf" ]; then
    echo "✓ examples/basic.tf exists"
else
    echo "✗ examples/basic.tf missing"
    exit 1
fi

cd ../..

# Test Aviatrix provider
echo "Testing Aviatrix provider..."
cd output/aviatrix
if [ -f "main.tf" ]; then
    echo "✓ main.tf exists"
else
    echo "✗ main.tf missing"
    exit 1
fi

if [ -f "variables.tf" ]; then
    echo "✓ variables.tf exists"
else
    echo "✗ variables.tf missing"
    exit 1
fi

if [ -f "outputs.tf" ]; then
    echo "✓ outputs.tf exists"
else
    echo "✗ outputs.tf missing"
    exit 1
fi

if [ -f "provider_schema.json" ]; then
    echo "✓ provider_schema.json exists"
else
    echo "✗ provider_schema.json missing"
    exit 1
fi

if [ -f "examples/basic.tf" ]; then
    echo "✓ examples/basic.tf exists"
else
    echo "✗ examples/basic.tf missing"
    exit 1
fi

cd ../..

# Test Router Simulator provider
echo "Testing Router Simulator provider..."
cd output/router-sim
if [ -f "main.tf" ]; then
    echo "✓ main.tf exists"
else
    echo "✗ main.tf missing"
    exit 1
fi

if [ -f "variables.tf" ]; then
    echo "✓ variables.tf exists"
else
    echo "✗ variables.tf missing"
    exit 1
fi

if [ -f "outputs.tf" ]; then
    echo "✓ outputs.tf exists"
else
    echo "✗ outputs.tf missing"
    exit 1
fi

if [ -f "provider_schema.json" ]; then
    echo "✓ provider_schema.json exists"
else
    echo "✗ provider_schema.json missing"
    exit 1
fi

if [ -f "examples/basic.tf" ]; then
    echo "✓ examples/basic.tf exists"
else
    echo "✗ examples/basic.tf missing"
    exit 1
fi

cd ../..

echo "All providers tested successfully!"
