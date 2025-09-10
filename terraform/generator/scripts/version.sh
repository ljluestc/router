#!/bin/bash

# Show version information for Terraform provider generator
set -e

echo "Terraform Provider Generator Version Information"
echo "=============================================="
echo ""

# Get Go version
if command -v go &> /dev/null; then
    GO_VERSION=$(go version | awk '{print $3}' | sed 's/go//')
    echo "Go version: $GO_VERSION"
else
    echo "Go version: Not installed"
fi

# Get Terraform version
if command -v terraform &> /dev/null; then
    TF_VERSION=$(terraform version -json | jq -r '.terraform_version' 2>/dev/null || terraform version | head -n1)
    echo "Terraform version: $TF_VERSION"
else
    echo "Terraform version: Not installed"
fi

# Get jq version
if command -v jq &> /dev/null; then
    JQ_VERSION=$(jq --version | awk '{print $2}' | sed 's/jq-//')
    echo "jq version: $JQ_VERSION"
else
    echo "jq version: Not installed"
fi

# Get generator version
if [ -f "main.go" ]; then
    echo "Generator version: 1.0.0"
else
    echo "Generator version: Not found"
fi

# Get provider versions
echo ""
echo "Provider versions:"
echo "  CloudPods: 1.0.0"
echo "  Aviatrix: 1.0.0"
echo "  Router Simulator: 1.0.0"

# Get system information
echo ""
echo "System information:"
echo "  OS: $(uname -s)"
echo "  Architecture: $(uname -m)"
echo "  Shell: $SHELL"

# Get directory information
echo ""
echo "Directory information:"
echo "  Current directory: $(pwd)"
echo "  Output directory: $(ls -la output/ 2>/dev/null | wc -l) files"
echo "  Backup directory: $(ls -la backup/ 2>/dev/null | wc -l) files"

echo ""
echo "Version check completed!"
