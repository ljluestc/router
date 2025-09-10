#!/bin/bash

# Restore Terraform providers from backup
set -e

echo "Restoring Terraform providers from backup..."

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

# Create output directory
mkdir -p output

# Restore CloudPods provider
echo "Restoring CloudPods provider..."
if [ -d "$BACKUP_DIR/cloudpods" ]; then
    cp -r "$BACKUP_DIR/cloudpods" output/
    echo "✓ CloudPods provider restored from $BACKUP_DIR/cloudpods"
else
    echo "✗ CloudPods provider not found in backup"
fi

# Restore Aviatrix provider
echo "Restoring Aviatrix provider..."
if [ -d "$BACKUP_DIR/aviatrix" ]; then
    cp -r "$BACKUP_DIR/aviatrix" output/
    echo "✓ Aviatrix provider restored from $BACKUP_DIR/aviatrix"
else
    echo "✗ Aviatrix provider not found in backup"
fi

# Restore Router Simulator provider
echo "Restoring Router Simulator provider..."
if [ -d "$BACKUP_DIR/router-sim" ]; then
    cp -r "$BACKUP_DIR/router-sim" output/
    echo "✓ Router Simulator provider restored from $BACKUP_DIR/router-sim"
else
    echo "✗ Router Simulator provider not found in backup"
fi

# Show backup info if available
if [ -f "$BACKUP_DIR/backup_info.txt" ]; then
    echo ""
    echo "Backup information:"
    cat "$BACKUP_DIR/backup_info.txt"
fi

echo "Restore completed successfully!"
