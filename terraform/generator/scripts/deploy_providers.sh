#!/bin/bash

# Deploy generated Terraform providers
set -e

echo "Deploying generated Terraform providers..."

# Check if terraform is installed
if ! command -v terraform &> /dev/null; then
    echo "Terraform is not installed. Please install Terraform first."
    exit 1
fi

# Deploy CloudPods provider
echo "Deploying CloudPods provider..."
cd output/cloudpods
if [ -f "main.tf" ]; then
    echo "Initializing CloudPods Terraform..."
    terraform init
    echo "Planning CloudPods deployment..."
    terraform plan
    echo "✓ CloudPods provider ready for deployment"
else
    echo "✗ CloudPods main.tf missing"
    exit 1
fi
cd ../..

# Deploy Aviatrix provider
echo "Deploying Aviatrix provider..."
cd output/aviatrix
if [ -f "main.tf" ]; then
    echo "Initializing Aviatrix Terraform..."
    terraform init
    echo "Planning Aviatrix deployment..."
    terraform plan
    echo "✓ Aviatrix provider ready for deployment"
else
    echo "✗ Aviatrix main.tf missing"
    exit 1
fi
cd ../..

# Deploy Router Simulator provider
echo "Deploying Router Simulator provider..."
cd output/router-sim
if [ -f "main.tf" ]; then
    echo "Initializing Router Simulator Terraform..."
    terraform init
    echo "Planning Router Simulator deployment..."
    terraform plan
    echo "✓ Router Simulator provider ready for deployment"
else
    echo "✗ Router Simulator main.tf missing"
    exit 1
fi
cd ../..

echo "All providers deployed successfully!"
echo ""
echo "To apply the configurations, run:"
echo "  cd output/cloudpods && terraform apply"
echo "  cd output/aviatrix && terraform apply"
echo "  cd output/router-sim && terraform apply"
