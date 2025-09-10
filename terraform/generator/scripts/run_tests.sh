#!/bin/bash

# Run all tests for Terraform provider generator
set -e

echo "Running all tests for Terraform provider generator..."

# Run Go tests
echo "Running Go tests..."
go test -v ./...

# Generate all providers
echo "Generating all providers..."
./scripts/generate_all.sh

# Test generated providers
echo "Testing generated providers..."
./scripts/test_providers.sh

# Validate Terraform configurations
echo "Validating Terraform configurations..."
./scripts/validate_terraform.sh

echo "All tests completed successfully!"
