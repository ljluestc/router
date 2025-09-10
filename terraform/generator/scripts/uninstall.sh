#!/bin/bash

# Uninstall Terraform provider generator
set -e

echo "Uninstalling Terraform Provider Generator"
echo "========================================"
echo ""

# Ask for confirmation
read -p "Are you sure you want to uninstall? (y/N): " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo "Uninstall cancelled"
    exit 0
fi

# Stop running processes
echo "Stopping running processes..."
./scripts/stop.sh

# Remove generated files
echo "Removing generated files..."
rm -rf output/
rm -rf backup/
rm -f terraform-generator
rm -f *.log
rm -f *.pid
echo "✓ Generated files removed"

# Ask about source code
read -p "Do you want to remove source code? (y/N): " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    echo "Removing source code..."
    cd ..
    rm -rf terraform/generator/
    echo "✓ Source code removed"
    echo ""
    echo "Uninstall completed successfully!"
    echo "All files have been removed."
else
    echo "Source code preserved"
    echo ""
    echo "Uninstall completed successfully!"
    echo "Source code has been preserved. You can remove it manually if needed."
fi
