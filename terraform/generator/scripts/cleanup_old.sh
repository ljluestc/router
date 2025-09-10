#!/bin/bash

# Cleanup old Terraform provider generator files
set -e

echo "Cleaning up old Terraform Provider Generator files"
echo "================================================"
echo ""

# Ask for confirmation
read -p "Do you want to cleanup old files? (y/N): " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo "Cleanup cancelled"
    exit 0
fi

# Clean old backups
echo "Cleaning old backups..."
if [ -d "backup" ]; then
    # Keep only last 10 backups
    BACKUP_COUNT=$(ls -1 backup/ | wc -l)
    if [ "$BACKUP_COUNT" -gt 10 ]; then
        OLD_BACKUPS=$(ls -1t backup/ | tail -n +11)
        for backup in $OLD_BACKUPS; do
            rm -rf "backup/$backup"
            echo "✓ Removed old backup: $backup"
        done
    else
        echo "✓ No old backups to remove"
    fi
else
    echo "⚠ No backup directory found"
fi

# Clean old logs
echo "Cleaning old logs..."
if [ -f "generator.log" ]; then
    LOG_SIZE=$(du -h generator.log | awk '{print $1}')
    if [ "$LOG_SIZE" != "0" ]; then
        # Keep only last 1000 lines
        tail -n 1000 generator.log > generator.log.tmp
        mv generator.log.tmp generator.log
        echo "✓ Log file trimmed to last 1000 lines"
    else
        echo "✓ Log file is empty"
    fi
else
    echo "⚠ No log file found"
fi

# Clean old temporary files
echo "Cleaning old temporary files..."
rm -f *.tmp
rm -f *.pid
rm -f *.lock
echo "✓ Temporary files cleaned"

# Clean old Terraform state files
echo "Cleaning old Terraform state files..."
find . -name "*.tfstate" -type f -delete 2>/dev/null || true
find . -name "*.tfstate.*" -type f -delete 2>/dev/null || true
find . -name ".terraform" -type d -exec rm -rf {} + 2>/dev/null || true
find . -name ".terraform.lock.hcl" -type f -delete 2>/dev/null || true
echo "✓ Terraform state files cleaned"

# Clean old build artifacts
echo "Cleaning old build artifacts..."
rm -f terraform-generator
rm -f *.exe
rm -f *.dll
rm -f *.so
rm -f *.dylib
echo "✓ Build artifacts cleaned"

# Clean old test files
echo "Cleaning old test files..."
rm -f test.out
rm -f *.test
echo "✓ Test files cleaned"

# Clean old cache files
echo "Cleaning old cache files..."
rm -rf .cache/
rm -rf cache/
echo "✓ Cache files cleaned"

# Show disk usage
echo ""
echo "Disk usage after cleanup:"
if [ -d "output" ]; then
    OUTPUT_SIZE=$(du -sh output/ | awk '{print $1}')
    echo "  Output directory: $OUTPUT_SIZE"
fi

if [ -d "backup" ]; then
    BACKUP_SIZE=$(du -sh backup/ | awk '{print $1}')
    echo "  Backup directory: $BACKUP_SIZE"
fi

if [ -f "generator.log" ]; then
    LOG_SIZE=$(du -h generator.log | awk '{print $1}')
    echo "  Log file: $LOG_SIZE"
fi

echo ""
echo "Cleanup completed successfully!"
