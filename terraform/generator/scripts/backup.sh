#!/bin/bash

# Backup Terraform provider generator
set -e

echo "Backing up Terraform Provider Generator"
echo "======================================"
echo ""

# Create backup directory with timestamp
BACKUP_DIR="backup/$(date +%Y%m%d_%H%M%S)"
mkdir -p "$BACKUP_DIR"

# Backup source code
echo "Backing up source code..."
cp -r . "$BACKUP_DIR/source/"
echo "✓ Source code backed up to $BACKUP_DIR/source"

# Backup generated providers
echo "Backing up generated providers..."
if [ -d "output" ]; then
    cp -r output "$BACKUP_DIR/"
    echo "✓ Generated providers backed up to $BACKUP_DIR/output"
else
    echo "⚠ No generated providers to backup"
fi

# Backup configuration
echo "Backing up configuration..."
if [ -d "config" ]; then
    cp -r config "$BACKUP_DIR/"
    echo "✓ Configuration backed up to $BACKUP_DIR/config"
else
    echo "⚠ No configuration to backup"
fi

# Backup logs
echo "Backing up logs..."
if [ -f "generator.log" ]; then
    cp generator.log "$BACKUP_DIR/"
    echo "✓ Logs backed up to $BACKUP_DIR/generator.log"
else
    echo "⚠ No logs to backup"
fi

# Create backup info file
cat > "$BACKUP_DIR/backup_info.txt" << EOF
Backup created: $(date)
Backup directory: $BACKUP_DIR
Source: $(pwd)
Git commit: $(git rev-parse HEAD 2>/dev/null || echo "Not a git repository")
Git branch: $(git branch --show-current 2>/dev/null || echo "Not a git repository")
System: $(uname -s) $(uname -m)
Go version: $(go version 2>/dev/null || echo "Go not installed")
Terraform version: $(terraform version -json | jq -r '.terraform_version' 2>/dev/null || echo "Terraform not installed")
EOF

echo "✓ Backup info saved to $BACKUP_DIR/backup_info.txt"

# Create backup archive
echo "Creating backup archive..."
cd "$BACKUP_DIR"
tar -czf "../$(basename "$BACKUP_DIR").tar.gz" .
cd ..
echo "✓ Backup archive created: $(basename "$BACKUP_DIR").tar.gz"

# Show backup size
BACKUP_SIZE=$(du -sh "$BACKUP_DIR" | awk '{print $1}')
echo "✓ Backup size: $BACKUP_SIZE"

echo ""
echo "Backup completed successfully!"
echo "Backup location: $BACKUP_DIR"
echo "Backup archive: $(basename "$BACKUP_DIR").tar.gz"
