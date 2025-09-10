#!/bin/bash

# Validate Terraform provider generator
set -e

echo "Validating Terraform Provider Generator"
echo "======================================"
echo ""

# Check system health
echo "Checking system health..."
if ./scripts/check_health.sh > /dev/null 2>&1; then
    echo "✓ System is healthy"
else
    echo "✗ System has issues"
    exit 1
fi

# Check configuration
echo "Checking configuration..."
if [ -d "config" ]; then
    echo "✓ Configuration directory exists"
    
    # Check config files
    for conf in config/*.conf; do
        if [ -f "$conf" ]; then
            echo "✓ $(basename "$conf") exists"
        else
            echo "✗ $(basename "$conf") missing"
        fi
    done
else
    echo "⚠ Configuration directory does not exist"
    echo "Run ./scripts/configure.sh to create configuration"
fi

# Check providers
echo "Checking providers..."
if [ -d "output" ]; then
    echo "✓ Output directory exists"
    
    # Check each provider
    for provider in cloudpods aviatrix router-sim; do
        if [ -d "output/$provider" ]; then
            echo "✓ $provider provider exists"
            
            # Check required files
            for file in main.tf variables.tf outputs.tf provider_schema.json; do
                if [ -f "output/$provider/$file" ]; then
                    echo "  ✓ $file exists"
                else
                    echo "  ✗ $file missing"
                fi
            done
            
            # Check examples
            if [ -f "output/$provider/examples/basic.tf" ]; then
                echo "  ✓ examples/basic.tf exists"
            else
                echo "  ✗ examples/basic.tf missing"
            fi
        else
            echo "✗ $provider provider missing"
        fi
    done
else
    echo "✗ Output directory does not exist"
    echo "Run ./scripts/generate_all.sh to generate providers"
fi

# Check backups
echo "Checking backups..."
if [ -d "backup" ]; then
    BACKUP_COUNT=$(ls -1 backup/ | wc -l)
    echo "✓ $BACKUP_COUNT backups available"
else
    echo "⚠ No backups available"
fi

# Check logs
echo "Checking logs..."
if [ -f "generator.log" ]; then
    echo "✓ Log file exists"
    LOG_SIZE=$(du -h generator.log | awk '{print $1}')
    echo "  Log size: $LOG_SIZE"
else
    echo "⚠ No log file found"
fi

# Check processes
echo "Checking processes..."
if pgrep -f "terraform-generator" > /dev/null; then
    echo "✓ Terraform generator processes running"
else
    echo "⚠ No terraform-generator processes running"
fi

if [ -f "monitor.pid" ]; then
    MONITOR_PID=$(cat monitor.pid)
    if kill -0 "$MONITOR_PID" 2>/dev/null; then
        echo "✓ Monitor process running (PID: $MONITOR_PID)"
    else
        echo "⚠ Monitor process not running"
    fi
else
    echo "⚠ No monitor process"
fi

# Check disk usage
echo "Checking disk usage..."
if [ -d "output" ]; then
    OUTPUT_SIZE=$(du -sh output/ | awk '{print $1}')
    echo "  Output directory: $OUTPUT_SIZE"
fi

if [ -d "backup" ]; then
    BACKUP_SIZE=$(du -sh backup/ | awk '{print $1}')
    echo "  Backup directory: $BACKUP_SIZE"
fi

# Check permissions
echo "Checking permissions..."
for script in scripts/*.sh; do
    if [ -f "$script" ]; then
        if [ -x "$script" ]; then
            echo "✓ $(basename "$script") is executable"
        else
            echo "✗ $(basename "$script") is not executable"
        fi
    fi
done

echo ""
echo "Validation completed!"
