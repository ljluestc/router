#!/bin/bash

# Update Terraform providers
set -e

echo "Updating Terraform providers..."

# Check if terraform is installed
if ! command -v terraform &> /dev/null; then
    echo "Terraform is not installed. Please install Terraform first."
    exit 1
fi

# Update CloudPods provider
echo "Updating CloudPods provider..."
cd output/cloudpods
if [ -f "main.tf" ]; then
    echo "Updating CloudPods Terraform..."
    terraform init -upgrade
    echo "✓ CloudPods provider updated"
else
    echo "✗ CloudPods main.tf missing"
    exit 1
fi
cd ../..

# Update Aviatrix provider
echo "Updating Aviatrix provider..."
cd output/aviatrix
if [ -f "main.tf" ]; then
    echo "Updating Aviatrix Terraform..."
    terraform init -upgrade
    echo "✓ Aviatrix provider updated"
else
    echo "✗ Aviatrix main.tf missing"
    exit 1
fi
cd ../..

# Update Router Simulator provider
echo "Updating Router Simulator provider..."
cd output/router-sim
if [ -f "main.tf" ]; then
    echo "Updating Router Simulator Terraform..."
    terraform init -upgrade
    echo "✓ Router Simulator provider updated"
else
    echo "✗ Router Simulator main.tf missing"
    exit 1
fi
cd ../..

echo "All providers updated successfully!"
