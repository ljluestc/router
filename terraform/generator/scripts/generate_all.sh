#!/bin/bash

# Generate all Terraform providers
set -e

echo "Generating all Terraform providers..."

# Create output directory
mkdir -p output

# Generate CloudPods provider
echo "Generating CloudPods provider..."
go run main.go cloudpods ./output/cloudpods

# Generate Aviatrix provider
echo "Generating Aviatrix provider..."
go run main.go aviatrix ./output/aviatrix

# Generate Router Simulator provider
echo "Generating Router Simulator provider..."
go run main.go router-sim ./output/router-sim

echo "All providers generated successfully!"
echo "Output directory: ./output"
echo ""
echo "Generated providers:"
echo "  - CloudPods: ./output/cloudpods"
echo "  - Aviatrix: ./output/aviatrix"
echo "  - Router Simulator: ./output/router-sim"
