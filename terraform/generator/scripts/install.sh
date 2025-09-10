#!/bin/bash

# Install Terraform provider generator
set -e

echo "Installing Terraform Provider Generator"
echo "======================================"
echo ""

# Check if Go is installed
if ! command -v go &> /dev/null; then
    echo "Go is not installed. Please install Go 1.21+ first."
    echo "Visit: https://golang.org/dl/"
    exit 1
fi

# Check Go version
GO_VERSION=$(go version | awk '{print $3}' | sed 's/go//')
REQUIRED_VERSION="1.21"
if [ "$(printf '%s\n' "$REQUIRED_VERSION" "$GO_VERSION" | sort -V | head -n1)" != "$REQUIRED_VERSION" ]; then
    echo "Go version $GO_VERSION is not supported. Please install Go 1.21+ first."
    exit 1
fi

echo "✓ Go version $GO_VERSION is supported"

# Check if Terraform is installed
if ! command -v terraform &> /dev/null; then
    echo "Terraform is not installed. Please install Terraform 1.0+ first."
    echo "Visit: https://terraform.io/downloads"
    exit 1
fi

# Check Terraform version
TF_VERSION=$(terraform version -json | jq -r '.terraform_version' 2>/dev/null || terraform version | head -n1)
REQUIRED_TF_VERSION="1.0"
if [ "$(printf '%s\n' "$REQUIRED_TF_VERSION" "$TF_VERSION" | sort -V | head -n1)" != "$REQUIRED_TF_VERSION" ]; then
    echo "Terraform version $TF_VERSION is not supported. Please install Terraform 1.0+ first."
    exit 1
fi

echo "✓ Terraform version $TF_VERSION is supported"

# Check if jq is installed
if ! command -v jq &> /dev/null; then
    echo "jq is not installed. Please install jq first."
    echo "Visit: https://stedolan.github.io/jq/"
    exit 1
fi

echo "✓ jq is installed"

# Install Go dependencies
echo "Installing Go dependencies..."
go mod download
go mod tidy
echo "✓ Go dependencies installed"

# Build generator
echo "Building generator..."
go build -o terraform-generator main.go
echo "✓ Generator built"

# Make scripts executable
echo "Making scripts executable..."
chmod +x scripts/*.sh
echo "✓ Scripts made executable"

# Create directories
echo "Creating directories..."
mkdir -p output
mkdir -p backup
echo "✓ Directories created"

# Test installation
echo "Testing installation..."
if ./scripts/check_health.sh > /dev/null 2>&1; then
    echo "✓ Installation test passed"
else
    echo "✗ Installation test failed"
    exit 1
fi

# Generate providers
echo "Generating providers..."
./scripts/generate_all.sh
echo "✓ Providers generated"

echo ""
echo "Installation completed successfully!"
echo ""
echo "You can now run:"
echo "  ./scripts/start.sh        - Start the generator"
echo "  ./scripts/help.sh         - Show help"
echo "  ./scripts/status.sh       - Show status"
echo "  ./scripts/generate_all.sh - Generate all providers"
echo ""
echo "For more information, see README.md"
