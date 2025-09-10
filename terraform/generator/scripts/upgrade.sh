#!/bin/bash

# Upgrade Terraform provider generator
set -e

echo "Upgrading Terraform Provider Generator"
echo "====================================="
echo ""

# Check if git is available
if ! command -v git &> /dev/null; then
    echo "Git is not available. Please upgrade manually."
    exit 1
fi

# Check if we're in a git repository
if [ ! -d ".git" ]; then
    echo "Not in a git repository. Please upgrade manually."
    exit 1
fi

# Check for updates
echo "Checking for updates..."
git fetch origin

# Check if there are updates
if [ "$(git rev-parse HEAD)" = "$(git rev-parse origin/main)" ]; then
    echo "✓ Already up to date"
    exit 0
fi

# Show available updates
echo "Available updates:"
git log --oneline HEAD..origin/main

# Ask for confirmation
read -p "Do you want to upgrade? (y/N): " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo "Upgrade cancelled"
    exit 0
fi

# Stop running processes
echo "Stopping running processes..."
./scripts/stop.sh

# Backup current state
echo "Backing up current state..."
./scripts/backup_providers.sh

# Upgrade code
echo "Upgrading code..."
git pull origin main

# Upgrade dependencies
echo "Upgrading dependencies..."
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
echo "Upgrade completed successfully!"
echo "You can now run:"
echo "  ./scripts/start.sh"
