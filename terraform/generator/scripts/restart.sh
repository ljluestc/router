#!/bin/bash

# Restart Terraform provider generator
set -e

echo "Restarting Terraform Provider Generator"
echo "======================================"
echo ""

# Stop any running processes
echo "Stopping running processes..."
if pgrep -f "terraform-generator" > /dev/null; then
    pkill -f "terraform-generator"
    echo "✓ Stopped terraform-generator processes"
else
    echo "✓ No terraform-generator processes running"
fi

# Stop monitor if running
if [ -f "monitor.pid" ]; then
    MONITOR_PID=$(cat monitor.pid)
    if kill -0 "$MONITOR_PID" 2>/dev/null; then
        kill "$MONITOR_PID"
        echo "✓ Stopped monitor process"
    fi
    rm -f monitor.pid
fi

# Clean up any temporary files
echo "Cleaning up temporary files..."
rm -f *.log
rm -f *.pid
echo "✓ Temporary files cleaned"

# Rebuild if needed
echo "Rebuilding generator..."
if [ -f "main.go" ]; then
    go build -o terraform-generator main.go
    echo "✓ Generator rebuilt"
else
    echo "✗ main.go not found"
    exit 1
fi

# Check system health
echo "Checking system health..."
if ./scripts/check_health.sh > /dev/null 2>&1; then
    echo "✓ System is healthy"
else
    echo "✗ System has issues"
    exit 1
fi

echo ""
echo "Restart completed successfully!"
echo "You can now run:"
echo "  ./scripts/generate_all.sh"
echo "  ./scripts/deploy_providers.sh"
echo "  ./scripts/monitor.sh"
