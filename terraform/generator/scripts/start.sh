#!/bin/bash

# Start Terraform provider generator
set -e

echo "Starting Terraform Provider Generator"
echo "===================================="
echo ""

# Check system health
echo "Checking system health..."
if ./scripts/check_health.sh > /dev/null 2>&1; then
    echo "✓ System is healthy"
else
    echo "✗ System has issues"
    exit 1
fi

# Build generator if needed
echo "Building generator..."
if [ ! -f "terraform-generator" ] || [ "main.go" -nt "terraform-generator" ]; then
    go build -o terraform-generator main.go
    echo "✓ Generator built"
else
    echo "✓ Generator is up to date"
fi

# Generate providers if not exist
echo "Checking providers..."
if [ ! -d "output" ]; then
    echo "Generating providers..."
    ./scripts/generate_all.sh
else
    echo "✓ Providers already exist"
fi

# Start monitor
echo "Starting monitor..."
./scripts/monitor.sh &
MONITOR_PID=$!
echo "✓ Monitor started (PID: $MONITOR_PID)"

echo ""
echo "Start completed successfully!"
echo "Monitor is running in the background"
echo "To stop monitoring, run: ./scripts/stop.sh"
