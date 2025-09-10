#!/bin/bash

# Show logs for Terraform provider generator
set -e

echo "Terraform Provider Generator Logs"
echo "================================"
echo ""

# Check if log file exists
if [ -f "generator.log" ]; then
    echo "Recent logs:"
    echo "------------"
    tail -n 50 generator.log
else
    echo "No log file found"
fi

echo ""
echo "System logs:"
echo "------------"

# Check system logs
if command -v journalctl &> /dev/null; then
    echo "Recent system logs:"
    journalctl --since "1 hour ago" | grep -i terraform | tail -n 10
else
    echo "journalctl not available"
fi

echo ""
echo "Process logs:"
echo "-------------"

# Check running processes
if pgrep -f "terraform-generator" > /dev/null; then
    echo "Terraform generator processes:"
    ps aux | grep terraform-generator | grep -v grep
else
    echo "No terraform-generator processes running"
fi

echo ""
echo "Log check completed!"
