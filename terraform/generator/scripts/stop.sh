#!/bin/bash

# Stop Terraform provider generator
set -e

echo "Stopping Terraform Provider Generator"
echo "===================================="
echo ""

# Stop terraform-generator processes
echo "Stopping terraform-generator processes..."
if pgrep -f "terraform-generator" > /dev/null; then
    pkill -f "terraform-generator"
    echo "✓ Stopped terraform-generator processes"
else
    echo "✓ No terraform-generator processes running"
fi

# Stop monitor process
echo "Stopping monitor process..."
if [ -f "monitor.pid" ]; then
    MONITOR_PID=$(cat monitor.pid)
    if kill -0 "$MONITOR_PID" 2>/dev/null; then
        kill "$MONITOR_PID"
        echo "✓ Stopped monitor process"
    else
        echo "✓ Monitor process already stopped"
    fi
    rm -f monitor.pid
else
    echo "✓ No monitor process running"
fi

# Stop any terraform processes
echo "Stopping terraform processes..."
if pgrep -f "terraform" > /dev/null; then
    pkill -f "terraform"
    echo "✓ Stopped terraform processes"
else
    echo "✓ No terraform processes running"
fi

# Clean up temporary files
echo "Cleaning up temporary files..."
rm -f *.log
rm -f *.pid
echo "✓ Temporary files cleaned"

echo ""
echo "Stop completed successfully!"
