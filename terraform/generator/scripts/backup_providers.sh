#!/bin/bash

# Backup generated Terraform providers
set -e

echo "Backing up generated Terraform providers..."

# Create backup directory with timestamp
BACKUP_DIR="backup/$(date +%Y%m%d_%H%M%S)"
mkdir -p "$BACKUP_DIR"

# Backup CloudPods provider
echo "Backing up CloudPods provider..."
if [ -d "output/cloudpods" ]; then
    cp -r output/cloudpods "$BACKUP_DIR/"
    echo "✓ CloudPods provider backed up to $BACKUP_DIR/cloudpods"
else
    echo "✗ CloudPods provider not found"
fi

# Backup Aviatrix provider
echo "Backing up Aviatrix provider..."
if [ -d "output/aviatrix" ]; then
    cp -r output/aviatrix "$BACKUP_DIR/"
    echo "✓ Aviatrix provider backed up to $BACKUP_DIR/aviatrix"
else
    echo "✗ Aviatrix provider not found"
fi

# Backup Router Simulator provider
echo "Backing up Router Simulator provider..."
if [ -d "output/router-sim" ]; then
    cp -r output/router-sim "$BACKUP_DIR/"
    echo "✓ Router Simulator provider backed up to $BACKUP_DIR/router-sim"
else
    echo "✗ Router Simulator provider not found"
fi

# Create backup info file
cat > "$BACKUP_DIR/backup_info.txt" << EOF
Backup created: $(date)
Backup directory: $BACKUP_DIR
Source: output/
EOF

echo "✓ Backup info saved to $BACKUP_DIR/backup_info.txt"

echo "Backup completed successfully!"
echo "Backup location: $BACKUP_DIR"
