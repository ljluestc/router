#!/bin/bash

# Install dependencies for Terraform provider generator
set -e

echo "Installing dependencies for Terraform provider generator..."

# Check if Go is installed
if ! command -v go &> /dev/null; then
    echo "Go is not installed. Please install Go 1.21+ first."
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

# Install Go dependencies
echo "Installing Go dependencies..."
go mod download
go mod tidy
echo "✓ Go dependencies installed"

# Check if Terraform is installed
if ! command -v terraform &> /dev/null; then
    echo "Terraform is not installed. Please install Terraform 1.0+ first."
    exit 1
fi

# Check Terraform version
TF_VERSION=$(terraform version -json | jq -r '.terraform_version')
REQUIRED_TF_VERSION="1.0"
if [ "$(printf '%s\n' "$REQUIRED_TF_VERSION" "$TF_VERSION" | sort -V | head -n1)" != "$REQUIRED_TF_VERSION" ]; then
    echo "Terraform version $TF_VERSION is not supported. Please install Terraform 1.0+ first."
    exit 1
fi

echo "✓ Terraform version $TF_VERSION is supported"

# Check if jq is installed
if ! command -v jq &> /dev/null; then
    echo "jq is not installed. Please install jq first."
    exit 1
fi

echo "✓ jq is installed"

echo "All dependencies installed successfully!"
