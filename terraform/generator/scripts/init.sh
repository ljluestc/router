#!/bin/bash

# Initialize Terraform provider generator
set -e

echo "Initializing Terraform Provider Generator"
echo "========================================"
echo ""

# Check if already initialized
if [ -f ".initialized" ]; then
    echo "Generator is already initialized"
    echo "To reinitialize, run: rm .initialized && ./scripts/init.sh"
    exit 0
fi

# Install dependencies
echo "Installing dependencies..."
./scripts/install_dependencies.sh

# Configure
echo "Configuring..."
./scripts/configure.sh

# Generate providers
echo "Generating providers..."
./scripts/generate_all.sh

# Test
echo "Testing..."
./scripts/test_providers.sh

# Create initialization marker
echo "Creating initialization marker..."
touch .initialized

echo ""
echo "Initialization completed successfully!"
echo ""
echo "You can now run:"
echo "  ./scripts/start.sh        - Start the generator"
echo "  ./scripts/status.sh       - Show status"
echo "  ./scripts/help.sh         - Show help"
echo ""
echo "For more information, see README.md"
