#!/bin/bash

# Optimize Terraform provider generator
set -e

echo "Optimizing Terraform Provider Generator"
echo "======================================"
echo ""

# Check system resources
echo "Checking system resources..."
TOTAL_MEM=$(free -h | awk '/^Mem:/ {print $2}')
AVAILABLE_MEM=$(free -h | awk '/^Mem:/ {print $7}')
TOTAL_DISK=$(df -h . | awk 'NR==2 {print $2}')
AVAILABLE_DISK=$(df -h . | awk 'NR==2 {print $4}')

echo "  Total memory: $TOTAL_MEM"
echo "  Available memory: $AVAILABLE_MEM"
echo "  Total disk: $TOTAL_DISK"
echo "  Available disk: $AVAILABLE_DISK"

# Optimize Go build
echo "Optimizing Go build..."
export CGO_ENABLED=0
export GOOS=linux
export GOARCH=amd64
go build -ldflags="-s -w" -o terraform-generator main.go
echo "✓ Go build optimized"

# Optimize generated providers
echo "Optimizing generated providers..."
if [ -d "output" ]; then
    for provider in output/*; do
        if [ -d "$provider" ]; then
            echo "  Optimizing $(basename "$provider")..."
            
            # Optimize Terraform files
            for tf_file in "$provider"/*.tf; do
                if [ -f "$tf_file" ]; then
                    terraform fmt "$tf_file" 2>/dev/null || true
                fi
            done
            
            # Optimize JSON files
            for json_file in "$provider"/*.json; do
                if [ -f "$json_file" ]; then
                    jq -c . "$json_file" > "$json_file.tmp" 2>/dev/null || true
                    mv "$json_file.tmp" "$json_file" 2>/dev/null || true
                fi
            done
        fi
    done
    echo "✓ Generated providers optimized"
else
    echo "⚠ No generated providers to optimize"
fi

# Optimize backups
echo "Optimizing backups..."
if [ -d "backup" ]; then
    for backup in backup/*; do
        if [ -d "$backup" ]; then
            echo "  Optimizing backup $(basename "$backup")..."
            
            # Compress large files
            find "$backup" -name "*.log" -size +1M -exec gzip {} \; 2>/dev/null || true
            find "$backup" -name "*.json" -size +1M -exec gzip {} \; 2>/dev/null || true
            
            # Remove empty directories
            find "$backup" -type d -empty -delete 2>/dev/null || true
        fi
    done
    echo "✓ Backups optimized"
else
    echo "⚠ No backups to optimize"
fi

# Optimize logs
echo "Optimizing logs..."
if [ -f "generator.log" ]; then
    LOG_SIZE=$(du -h generator.log | awk '{print $1}')
    if [ "$LOG_SIZE" != "0" ]; then
        # Compress old log entries
        if [ -f "generator.log" ] && [ "$(wc -l < generator.log)" -gt 1000 ]; then
            head -n 500 generator.log > generator.log.old
            tail -n 500 generator.log > generator.log.new
            gzip generator.log.old 2>/dev/null || true
            mv generator.log.new generator.log
            echo "✓ Log file optimized"
        else
            echo "✓ Log file is already optimized"
        fi
    else
        echo "✓ Log file is empty"
    fi
else
    echo "⚠ No log file found"
fi

# Optimize configuration
echo "Optimizing configuration..."
if [ -d "config" ]; then
    for conf in config/*.conf; do
        if [ -f "$conf" ]; then
            # Remove empty lines and comments
            grep -v '^#' "$conf" | grep -v '^$' > "$conf.tmp" 2>/dev/null || true
            mv "$conf.tmp" "$conf" 2>/dev/null || true
        fi
    done
    echo "✓ Configuration optimized"
else
    echo "⚠ No configuration to optimize"
fi

# Show optimization results
echo ""
echo "Optimization results:"
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

if [ -f "terraform-generator" ]; then
    BINARY_SIZE=$(du -h terraform-generator | awk '{print $1}')
    echo "  Binary size: $BINARY_SIZE"
fi

echo ""
echo "Optimization completed successfully!"
