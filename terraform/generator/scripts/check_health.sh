#!/bin/bash

# Check health of Terraform provider generator
set -e

echo "Checking health of Terraform provider generator..."

# Check if Go is installed
if ! command -v go &> /dev/null; then
    echo "✗ Go is not installed"
    exit 1
else
    echo "✓ Go is installed"
fi

# Check if Terraform is installed
if ! command -v terraform &> /dev/null; then
    echo "✗ Terraform is not installed"
    exit 1
else
    echo "✓ Terraform is installed"
fi

# Check if jq is installed
if ! command -v jq &> /dev/null; then
    echo "✗ jq is not installed"
    exit 1
else
    echo "✓ jq is installed"
fi

# Check if main.go exists
if [ ! -f "main.go" ]; then
    echo "✗ main.go not found"
    exit 1
else
    echo "✓ main.go exists"
fi

# Check if go.mod exists
if [ ! -f "go.mod" ]; then
    echo "✗ go.mod not found"
    exit 1
else
    echo "✓ go.mod exists"
fi

# Check if scripts directory exists
if [ ! -d "scripts" ]; then
    echo "✗ scripts directory not found"
    exit 1
else
    echo "✓ scripts directory exists"
fi

# Check if all scripts are executable
echo "Checking script permissions..."
for script in scripts/*.sh; do
    if [ -f "$script" ]; then
        if [ -x "$script" ]; then
            echo "✓ $script is executable"
        else
            echo "✗ $script is not executable"
            exit 1
        fi
    fi
done

# Check if output directory exists
if [ ! -d "output" ]; then
    echo "⚠ output directory does not exist (run generate_all.sh to create)"
else
    echo "✓ output directory exists"
fi

# Check if backup directory exists
if [ ! -d "backup" ]; then
    echo "⚠ backup directory does not exist (run backup_providers.sh to create)"
else
    echo "✓ backup directory exists"
fi

echo "Health check completed successfully!"
