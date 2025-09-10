#!/bin/bash

# Validate generated Terraform configurations
set -e

echo "Validating generated Terraform configurations..."

# Check if terraform is installed
if ! command -v terraform &> /dev/null; then
    echo "Terraform is not installed. Please install Terraform first."
    exit 1
fi

# Validate CloudPods provider
echo "Validating CloudPods provider..."
cd output/cloudpods
if [ -f "main.tf" ]; then
    echo "Validating CloudPods main.tf..."
    terraform fmt -check=true main.tf
    echo "✓ CloudPods main.tf is valid"
else
    echo "✗ CloudPods main.tf missing"
    exit 1
fi

if [ -f "variables.tf" ]; then
    echo "Validating CloudPods variables.tf..."
    terraform fmt -check=true variables.tf
    echo "✓ CloudPods variables.tf is valid"
else
    echo "✗ CloudPods variables.tf missing"
    exit 1
fi

if [ -f "outputs.tf" ]; then
    echo "Validating CloudPods outputs.tf..."
    terraform fmt -check=true outputs.tf
    echo "✓ CloudPods outputs.tf is valid"
else
    echo "✗ CloudPods outputs.tf missing"
    exit 1
fi

cd ../..

# Validate Aviatrix provider
echo "Validating Aviatrix provider..."
cd output/aviatrix
if [ -f "main.tf" ]; then
    echo "Validating Aviatrix main.tf..."
    terraform fmt -check=true main.tf
    echo "✓ Aviatrix main.tf is valid"
else
    echo "✗ Aviatrix main.tf missing"
    exit 1
fi

if [ -f "variables.tf" ]; then
    echo "Validating Aviatrix variables.tf..."
    terraform fmt -check=true variables.tf
    echo "✓ Aviatrix variables.tf is valid"
else
    echo "✗ Aviatrix variables.tf missing"
    exit 1
fi

if [ -f "outputs.tf" ]; then
    echo "Validating Aviatrix outputs.tf..."
    terraform fmt -check=true outputs.tf
    echo "✓ Aviatrix outputs.tf is valid"
else
    echo "✗ Aviatrix outputs.tf missing"
    exit 1
fi

cd ../..

# Validate Router Simulator provider
echo "Validating Router Simulator provider..."
cd output/router-sim
if [ -f "main.tf" ]; then
    echo "Validating Router Simulator main.tf..."
    terraform fmt -check=true main.tf
    echo "✓ Router Simulator main.tf is valid"
else
    echo "✗ Router Simulator main.tf missing"
    exit 1
fi

if [ -f "variables.tf" ]; then
    echo "Validating Router Simulator variables.tf..."
    terraform fmt -check=true variables.tf
    echo "✓ Router Simulator variables.tf is valid"
else
    echo "✗ Router Simulator variables.tf missing"
    exit 1
fi

if [ -f "outputs.tf" ]; then
    echo "Validating Router Simulator outputs.tf..."
    terraform fmt -check=true outputs.tf
    echo "✓ Router Simulator outputs.tf is valid"
else
    echo "✗ Router Simulator outputs.tf missing"
    exit 1
fi

cd ../..

echo "All Terraform configurations validated successfully!"
