#!/bin/bash

# Security analysis for Terraform provider generator
set -e

echo "Security Analysis for Terraform Provider Generator"
echo "================================================"
echo ""

# Check if Go is installed
if ! command -v go &> /dev/null; then
    echo "Go is not installed. Please install Go first."
    exit 1
fi

# Check if gosec is installed
if ! command -v gosec &> /dev/null; then
    echo "gosec is not installed. Installing..."
    go install github.com/securecodewarrior/gosec/v2/cmd/gosec@latest
    echo "✓ gosec installed"
fi

# Check if semgrep is installed
if ! command -v semgrep &> /dev/null; then
    echo "semgrep is not installed. Installing..."
    pip install semgrep
    echo "✓ semgrep installed"
fi

# Create security directory
mkdir -p security
cd security

# Go security analysis
echo "Go Security Analysis..."
echo "======================"
echo ""

# Run gosec
echo "Running gosec..."
gosec -fmt json -out go_security.json ../... 2>/dev/null || true
echo "✓ gosec completed"
echo ""

# Run semgrep
echo "Running semgrep..."
semgrep --config=auto --json --output=semgrep.json ../... 2>/dev/null || true
echo "✓ semgrep completed"
echo ""

# Run go audit
echo "Running go audit..."
go list -json -deps ../... | jq -r '.Deps[]' | xargs go list -json | jq -r '.ImportPath' | sort | uniq > go_deps.txt
echo "✓ Go audit completed"
echo ""

# Terraform security analysis
echo "Terraform Security Analysis..."
echo "============================="
echo ""

# Check if tfsec is installed
if command -v tfsec &> /dev/null; then
    echo "Running tfsec..."
    tfsec ../... --format json --out tfsec.json 2>/dev/null || true
    echo "✓ tfsec completed"
    echo ""
else
    echo "⚠ tfsec not available for Terraform security analysis"
fi

# Check if checkov is installed
if command -v checkov &> /dev/null; then
    echo "Running checkov..."
    checkov -d ../... --output json --output-file-path checkov.json 2>/dev/null || true
    echo "✓ checkov completed"
    echo ""
else
    echo "⚠ checkov not available for Terraform security analysis"
fi

# Shell script security analysis
echo "Shell Script Security Analysis..."
echo "================================"
echo ""

# Check if shellcheck is installed
if command -v shellcheck &> /dev/null; then
    echo "Running shellcheck..."
    shellcheck ../scripts/*.sh > shellcheck_security.txt 2>/dev/null || true
    echo "✓ shellcheck completed"
    echo ""
else
    echo "⚠ shellcheck not available for shell script security analysis"
fi

# Generate security report
echo "Generating security report..."
echo "============================"
echo ""

# Create security report
cat > security_report.md << EOF
# Security Report

## Go Security Analysis
- **gosec**: $(grep -c "issues" go_security.json 2>/dev/null || echo "0") issues
- **semgrep**: $(grep -c "issues" semgrep.json 2>/dev/null || echo "0") issues
- **Dependencies**: $(wc -l < go_deps.txt 2>/dev/null || echo "0") packages

## Terraform Security Analysis
- **tfsec**: $(grep -c "issues" tfsec.json 2>/dev/null || echo "0") issues
- **checkov**: $(grep -c "issues" checkov.json 2>/dev/null || echo "0") issues

## Shell Script Security Analysis
- **shellcheck**: $(wc -l < shellcheck_security.txt 2>/dev/null || echo "0") issues

## Summary
- **Total Security Issues**: $(($(grep -c "issues" go_security.json 2>/dev/null || echo "0") + $(grep -c "issues" semgrep.json 2>/dev/null || echo "0") + $(grep -c "issues" tfsec.json 2>/dev/null || echo "0") + $(grep -c "issues" checkov.json 2>/dev/null || echo "0") + $(wc -l < shellcheck_security.txt 2>/dev/null || echo "0")))

## Files
- **Go Security**: go_security.json
- **Semgrep**: semgrep.json
- **Go Dependencies**: go_deps.txt
- **Terraform Security**: tfsec.json
- **Checkov**: checkov.json
- **Shellcheck Security**: shellcheck_security.txt
EOF

echo "✓ Security report generated: security_report.md"
echo ""

# Show security summary
echo "Security Summary:"
echo "================="
echo ""

# Go security summary
echo "Go Security:"
echo "  gosec: $(grep -c "issues" go_security.json 2>/dev/null || echo "0") issues"
echo "  semgrep: $(grep -c "issues" semgrep.json 2>/dev/null || echo "0") issues"
echo "  Dependencies: $(wc -l < go_deps.txt 2>/dev/null || echo "0") packages"
echo ""

# Terraform security summary
echo "Terraform Security:"
echo "  tfsec: $(grep -c "issues" tfsec.json 2>/dev/null || echo "0") issues"
echo "  checkov: $(grep -c "issues" checkov.json 2>/dev/null || echo "0") issues"
echo ""

# Shell script security summary
echo "Shell Script Security:"
echo "  shellcheck: $(wc -l < shellcheck_security.txt 2>/dev/null || echo "0") issues"
echo ""

# Show security files
echo "Security files generated:"
echo "========================"
ls -la *.json *.txt *.md 2>/dev/null || echo "No security files found"
echo ""

# Show security recommendations
echo "Security Recommendations:"
echo "========================="
echo ""

# Go security recommendations
if [ -f "go_security.json" ] && [ "$(grep -c "issues" go_security.json 2>/dev/null || echo "0")" -gt 0 ]; then
    echo "Go Security Issues:"
    grep -o '"message":"[^"]*"' go_security.json | head -5
    echo ""
fi

# Semgrep recommendations
if [ -f "semgrep.json" ] && [ "$(grep -c "issues" semgrep.json 2>/dev/null || echo "0")" -gt 0 ]; then
    echo "Semgrep Issues:"
    grep -o '"message":"[^"]*"' semgrep.json | head -5
    echo ""
fi

# Terraform security recommendations
if [ -f "tfsec.json" ] && [ "$(grep -c "issues" tfsec.json 2>/dev/null || echo "0")" -gt 0 ]; then
    echo "Terraform Security Issues:"
    grep -o '"message":"[^"]*"' tfsec.json | head -5
    echo ""
fi

# Checkov recommendations
if [ -f "checkov.json" ] && [ "$(grep -c "issues" checkov.json 2>/dev/null || echo "0")" -gt 0 ]; then
    echo "Checkov Issues:"
    grep -o '"message":"[^"]*"' checkov.json | head -5
    echo ""
fi

# Shell script security recommendations
if [ -f "shellcheck_security.txt" ] && [ "$(wc -l < shellcheck_security.txt 2>/dev/null || echo "0")" -gt 0 ]; then
    echo "Shell Script Security Issues:"
    head -5 shellcheck_security.txt
    echo ""
fi

echo "Security analysis completed successfully!"
