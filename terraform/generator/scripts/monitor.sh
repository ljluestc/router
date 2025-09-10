#!/bin/bash

# Monitor Terraform provider generator
set -e

echo "Terraform Provider Generator Monitor"
echo "==================================="
echo ""

# Check if monitoring is already running
if [ -f "monitor.pid" ]; then
    echo "Monitor is already running (PID: $(cat monitor.pid))"
    echo "To stop monitoring, run: kill $(cat monitor.pid)"
    exit 1
fi

# Start monitoring
echo "Starting monitoring..."
echo "Press Ctrl+C to stop monitoring"
echo ""

# Create PID file
echo $$ > monitor.pid

# Cleanup function
cleanup() {
    echo ""
    echo "Stopping monitor..."
    rm -f monitor.pid
    exit 0
}

# Set trap for cleanup
trap cleanup SIGINT SIGTERM

# Monitor loop
while true; do
    echo "$(date): Checking status..."
    
    # Check system health
    if ! ./scripts/check_health.sh > /dev/null 2>&1; then
        echo "⚠ System health check failed"
    fi
    
    # Check provider status
    if [ -d "output" ]; then
        PROVIDER_COUNT=$(ls -1 output/ | wc -l)
        echo "  Providers: $PROVIDER_COUNT"
    else
        echo "  Providers: 0"
    fi
    
    # Check backup status
    if [ -d "backup" ]; then
        BACKUP_COUNT=$(ls -1 backup/ | wc -l)
        echo "  Backups: $BACKUP_COUNT"
    else
        echo "  Backups: 0"
    fi
    
    # Check disk usage
    if [ -d "output" ]; then
        OUTPUT_SIZE=$(du -sh output/ | awk '{print $1}')
        echo "  Output size: $OUTPUT_SIZE"
    fi
    
    if [ -d "backup" ]; then
        BACKUP_SIZE=$(du -sh backup/ | awk '{print $1}')
        echo "  Backup size: $BACKUP_SIZE"
    fi
    
    echo "  Status: OK"
    echo ""
    
    # Wait 30 seconds
    sleep 30
done
