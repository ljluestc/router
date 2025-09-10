#!/bin/bash

# Cleanup generated Terraform providers
set -e

echo "Cleaning up generated Terraform providers..."

# Remove output directory
if [ -d "output" ]; then
    echo "Removing output directory..."
    rm -rf output
    echo "✓ Output directory removed"
else
    echo "✓ Output directory does not exist"
fi

# Remove build artifacts
if [ -f "terraform-generator" ]; then
    echo "Removing terraform-generator binary..."
    rm -f terraform-generator
    echo "✓ terraform-generator binary removed"
else
    echo "✓ terraform-generator binary does not exist"
fi

# Remove Terraform state files
echo "Removing Terraform state files..."
find . -name "*.tfstate" -type f -delete 2>/dev/null || true
find . -name "*.tfstate.*" -type f -delete 2>/dev/null || true
find . -name ".terraform" -type d -exec rm -rf {} + 2>/dev/null || true
find . -name ".terraform.lock.hcl" -type f -delete 2>/dev/null || true
echo "✓ Terraform state files removed"

echo "Cleanup completed successfully!"
