#!/bin/bash

# Show status of Terraform provider generator
set -e

echo "Terraform Provider Generator Status"
echo "=================================="
echo ""

# Check system health
echo "System Health:"
if ./scripts/check_health.sh > /dev/null 2>&1; then
    echo "✓ System is healthy"
else
    echo "✗ System has issues"
fi

# Check if providers are generated
echo ""
echo "Provider Status:"
if [ -d "output/cloudpods" ]; then
    echo "✓ CloudPods provider generated"
else
    echo "✗ CloudPods provider not generated"
fi

if [ -d "output/aviatrix" ]; then
    echo "✓ Aviatrix provider generated"
else
    echo "✗ Aviatrix provider not generated"
fi

if [ -d "output/router-sim" ]; then
    echo "✓ Router Simulator provider generated"
else
    echo "✗ Router Simulator provider not generated"
fi

# Check if providers are valid
echo ""
echo "Provider Validation:"
if [ -d "output" ]; then
    if ./scripts/validate_terraform.sh > /dev/null 2>&1; then
        echo "✓ All providers are valid"
    else
        echo "✗ Some providers have validation issues"
    fi
else
    echo "✗ No providers to validate"
fi

# Check backup status
echo ""
echo "Backup Status:"
if [ -d "backup" ]; then
    BACKUP_COUNT=$(ls -1 backup/ | wc -l)
    echo "✓ $BACKUP_COUNT backups available"
    echo "  Latest backup: $(ls -t backup/ | head -n1)"
else
    echo "✗ No backups available"
fi

# Check disk usage
echo ""
echo "Disk Usage:"
if [ -d "output" ]; then
    OUTPUT_SIZE=$(du -sh output/ | awk '{print $1}')
    echo "  Output directory: $OUTPUT_SIZE"
else
    echo "  Output directory: Not found"
fi

if [ -d "backup" ]; then
    BACKUP_SIZE=$(du -sh backup/ | awk '{print $1}')
    echo "  Backup directory: $BACKUP_SIZE"
else
    echo "  Backup directory: Not found"
fi

# Check recent activity
echo ""
echo "Recent Activity:"
if [ -f "output" ]; then
    echo "  Last generated: $(stat -c %y output/ 2>/dev/null || echo 'Unknown')"
else
    echo "  Last generated: Never"
fi

if [ -d "backup" ]; then
    echo "  Last backup: $(stat -c %y backup/ 2>/dev/null || echo 'Unknown')"
else
    echo "  Last backup: Never"
fi

echo ""
echo "Status check completed!"
