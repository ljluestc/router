#!/bin/bash

# Reset Terraform provider generator
set -e

echo "Resetting Terraform Provider Generator"
echo "====================================="
echo ""

# Ask for confirmation
read -p "Are you sure you want to reset? This will remove all generated files and configurations. (y/N): " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo "Reset cancelled"
    exit 0
fi

# Stop running processes
echo "Stopping running processes..."
./scripts/stop.sh

# Remove generated files
echo "Removing generated files..."
rm -rf output/
rm -rf backup/
rm -rf config/
rm -f terraform-generator
rm -f *.log
rm -f *.pid
rm -f .initialized
echo "✓ Generated files removed"

# Clean Go modules
echo "Cleaning Go modules..."
go clean -modcache
echo "✓ Go modules cleaned"

# Rebuild
echo "Rebuilding..."
go build -o terraform-generator main.go
echo "✓ Generator rebuilt"

# Make scripts executable
echo "Making scripts executable..."
chmod +x scripts/*.sh
echo "✓ Scripts made executable"

echo ""
echo "Reset completed successfully!"
echo ""
echo "To reinitialize, run:"
echo "  ./scripts/init.sh"
