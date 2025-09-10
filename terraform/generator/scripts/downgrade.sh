#!/bin/bash

# Downgrade Terraform provider generator
set -e

echo "Downgrading Terraform Provider Generator"
echo "======================================="
echo ""

# Check if git is available
if ! command -v git &> /dev/null; then
    echo "Git is not available. Please downgrade manually."
    exit 1
fi

# Check if we're in a git repository
if [ ! -d ".git" ]; then
    echo "Not in a git repository. Please downgrade manually."
    exit 1
fi

# Show available versions
echo "Available versions:"
git tag --sort=-version:refname | head -10

# Ask for version
read -p "Enter version to downgrade to: " VERSION
if [ -z "$VERSION" ]; then
    echo "Version is required"
    exit 1
fi

# Check if version exists
if ! git tag | grep -q "^$VERSION$"; then
    echo "Version $VERSION does not exist"
    exit 1
fi

# Ask for confirmation
read -p "Are you sure you want to downgrade to $VERSION? (y/N): " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo "Downgrade cancelled"
    exit 0
fi

# Stop running processes
echo "Stopping running processes..."
./scripts/stop.sh

# Backup current state
echo "Backing up current state..."
./scripts/backup_providers.sh

# Downgrade code
echo "Downgrading code..."
git checkout "$VERSION"

# Downgrade dependencies
echo "Downgrading dependencies..."
go mod download
go mod tidy

# Rebuild
echo "Rebuilding..."
go build -o terraform-generator main.go

# Regenerate providers
echo "Regenerating providers..."
./scripts/generate_all.sh

# Test
echo "Testing..."
./scripts/test_providers.sh

echo ""
echo "Downgrade completed successfully!"
echo "You can now run:"
echo "  ./scripts/start.sh"
