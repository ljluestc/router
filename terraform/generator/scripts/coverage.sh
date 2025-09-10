#!/bin/bash

# Coverage analysis for Terraform provider generator
set -e

echo "Coverage Analysis for Terraform Provider Generator"
echo "================================================"
echo ""

# Check if Go is installed
if ! command -v go &> /dev/null; then
    echo "Go is not installed. Please install Go first."
    exit 1
fi

# Create coverage directory
mkdir -p coverage
cd coverage

# Run tests with coverage
echo "Running tests with coverage..."
echo "============================="
echo ""

# Run Go tests with coverage
echo "Running Go tests with coverage..."
go test -coverprofile=coverage.out -covermode=count ../...
echo ""

# Generate coverage report
echo "Generating coverage report..."
go tool cover -html=coverage.out -o coverage.html
echo "✓ Coverage report generated: coverage.html"
echo ""

# Generate coverage summary
echo "Generating coverage summary..."
go tool cover -func=coverage.out
echo ""

# Generate coverage by function
echo "Generating coverage by function..."
go tool cover -func=coverage.out | grep -E "main\.go|terraform|provider" | head -20
echo ""

# Generate coverage statistics
echo "Generating coverage statistics..."
TOTAL_LINES=$(go tool cover -func=coverage.out | tail -1 | awk '{print $3}' | sed 's/%//')
COVERED_LINES=$(go tool cover -func=coverage.out | tail -1 | awk '{print $2}' | sed 's/%//')
UNCOVERED_LINES=$((100 - TOTAL_LINES))

echo "Coverage Statistics:"
echo "  Total lines: $TOTAL_LINES%"
echo "  Covered lines: $COVERED_LINES%"
echo "  Uncovered lines: $UNCOVERED_LINES%"
echo ""

# Generate coverage by file
echo "Generating coverage by file..."
go tool cover -func=coverage.out | grep -E "main\.go" | head -10
echo ""

# Generate coverage report in different formats
echo "Generating coverage reports in different formats..."
echo "=================================================="
echo ""

# HTML report
echo "HTML report:"
go tool cover -html=coverage.out -o coverage.html
echo "✓ HTML report generated: coverage.html"
echo ""

# Text report
echo "Text report:"
go tool cover -func=coverage.out > coverage.txt
echo "✓ Text report generated: coverage.txt"
echo ""

# JSON report
echo "JSON report:"
go tool cover -func=coverage.out | awk 'NR>1 {print "{\"function\":\"" $1 "\",\"statements\":" $2 ",\"coverage\":" $3 "}"}' > coverage.json
echo "✓ JSON report generated: coverage.json"
echo ""

# Generate coverage badges
echo "Generating coverage badges..."
echo "============================"
echo ""

# Generate coverage badge
if [ "$TOTAL_LINES" -gt 80 ]; then
    BADGE_COLOR="green"
elif [ "$TOTAL_LINES" -gt 60 ]; then
    BADGE_COLOR="yellow"
else
    BADGE_COLOR="red"
fi

echo "Coverage badge:"
echo "![Coverage](https://img.shields.io/badge/coverage-$TOTAL_LINES%25-$BADGE_COLOR)"
echo ""

# Generate coverage summary
echo "Coverage Summary:"
echo "================="
echo ""

# Show coverage by function
echo "Coverage by function:"
go tool cover -func=coverage.out | grep -E "main\.go" | head -10
echo ""

# Show coverage by file
echo "Coverage by file:"
go tool cover -func=coverage.out | grep -E "main\.go" | head -10
echo ""

# Show coverage trends
echo "Coverage trends:"
echo "  Current coverage: $TOTAL_LINES%"
echo "  Target coverage: 80%"
if [ "$TOTAL_LINES" -ge 80 ]; then
    echo "  Status: ✓ Target achieved"
else
    echo "  Status: ✗ Target not achieved"
fi
echo ""

# Generate coverage recommendations
echo "Coverage Recommendations:"
echo "========================="
echo ""

# Find uncovered functions
echo "Uncovered functions:"
go tool cover -func=coverage.out | grep -E "main\.go" | awk '$3 < 80 {print $1 " - " $3 "%"}' | head -10
echo ""

# Find functions with low coverage
echo "Functions with low coverage:"
go tool cover -func=coverage.out | grep -E "main\.go" | awk '$3 < 50 {print $1 " - " $3 "%"}' | head -10
echo ""

# Generate coverage report
echo "Generating coverage report..."
echo "============================"
echo ""

# Create coverage report
cat > coverage_report.md << EOF
# Coverage Report

## Summary
- **Total Coverage**: $TOTAL_LINES%
- **Covered Lines**: $COVERED_LINES%
- **Uncovered Lines**: $UNCOVERED_LINES%

## Coverage by Function
\`\`\`
$(go tool cover -func=coverage.out | grep -E "main\.go" | head -20)
\`\`\`

## Coverage Statistics
- **Target Coverage**: 80%
- **Current Status**: $([ "$TOTAL_LINES" -ge 80 ] && echo "✓ Target achieved" || echo "✗ Target not achieved")

## Recommendations
- Add tests for uncovered functions
- Improve coverage for functions with low coverage
- Focus on critical path functions

## Files
- **HTML Report**: coverage.html
- **Text Report**: coverage.txt
- **JSON Report**: coverage.json
EOF

echo "✓ Coverage report generated: coverage_report.md"
echo ""

# Show coverage files
echo "Coverage files generated:"
echo "========================="
ls -la *.html *.txt *.json *.md 2>/dev/null || echo "No coverage files found"
echo ""

# Show coverage summary
echo "Coverage Summary:"
echo "================="
echo "  HTML report: coverage.html"
echo "  Text report: coverage.txt"
echo "  JSON report: coverage.json"
echo "  Markdown report: coverage_report.md"
echo ""

echo "Coverage analysis completed successfully!"
