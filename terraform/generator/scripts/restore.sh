#!/bin/bash

# Restore Terraform provider generator from backup
set -e

echo "Restoring Terraform Provider Generator"
echo "====================================="
echo ""

# Check if backup directory is provided
if [ -z "$1" ]; then
    echo "Usage: $0 <backup_directory>"
    echo "Available backups:"
    ls -la backup/ 2>/dev/null || echo "No backups found"
    exit 1
fi

BACKUP_DIR="$1"

# Check if backup directory exists
if [ ! -d "$BACKUP_DIR" ]; then
    echo "Backup directory $BACKUP_DIR does not exist"
    exit 1
fi

# Show backup info if available
if [ -f "$BACKUP_DIR/backup_info.txt" ]; then
    echo "Backup information:"
    cat "$BACKUP_DIR/backup_info.txt"
    echo ""
fi

# Ask for confirmation
read -p "Do you want to restore from this backup? (y/N): " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo "Restore cancelled"
    exit 0
fi

# Stop running processes
echo "Stopping running processes..."
./scripts/stop.sh

# Restore source code
echo "Restoring source code..."
if [ -d "$BACKUP_DIR/source" ]; then
    cp -r "$BACKUP_DIR/source"/* .
    echo "✓ Source code restored"
else
    echo "⚠ No source code to restore"
fi

# Restore generated providers
echo "Restoring generated providers..."
if [ -d "$BACKUP_DIR/output" ]; then
    cp -r "$BACKUP_DIR/output" .
    echo "✓ Generated providers restored"
else
    echo "⚠ No generated providers to restore"
fi

# Restore configuration
echo "Restoring configuration..."
if [ -d "$BACKUP_DIR/config" ]; then
    cp -r "$BACKUP_DIR/config" .
    echo "✓ Configuration restored"
else
    echo "⚠ No configuration to restore"
fi

# Restore logs
echo "Restoring logs..."
if [ -f "$BACKUP_DIR/generator.log" ]; then
    cp "$BACKUP_DIR/generator.log" .
    echo "✓ Logs restored"
else
    echo "⚠ No logs to restore"
fi

# Rebuild
echo "Rebuilding..."
go build -o terraform-generator main.go
echo "✓ Generator rebuilt"

# Test
echo "Testing..."
./scripts/test_providers.sh

echo ""
echo "Restore completed successfully!"
echo "You can now run:"
echo "  ./scripts/start.sh"
