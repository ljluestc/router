#!/bin/bash

# Lint Terraform provider generator
set -e

echo "Linting Terraform Provider Generator"
echo "==================================="
echo ""

# Check if Go is installed
if ! command -v go &> /dev/null; then
    echo "Go is not installed. Please install Go first."
    exit 1
fi

# Check if golangci-lint is installed
if ! command -v golangci-lint &> /dev/null; then
    echo "golangci-lint is not installed. Installing..."
    go install github.com/golangci/golangci-lint/cmd/golangci-lint@latest
    echo "✓ golangci-lint installed"
fi

# Check if terraform is installed
if ! command -v terraform &> /dev/null; then
    echo "Terraform is not installed. Please install Terraform first."
    exit 1
fi

# Create lint directory
mkdir -p lint
cd lint

# Go linting
echo "Go Linting..."
echo "============"
echo ""

# Run golangci-lint
echo "Running golangci-lint..."
golangci-lint run ../... --out-format=json > go_lint.json 2>&1 || true
echo "✓ Go linting completed"
echo ""

# Run go vet
echo "Running go vet..."
go vet ../... > go_vet.txt 2>&1 || true
echo "✓ Go vet completed"
echo ""

# Run go fmt
echo "Running go fmt..."
go fmt ../... > go_fmt.txt 2>&1 || true
echo "✓ Go fmt completed"
echo ""

# Run go mod tidy
echo "Running go mod tidy..."
go mod tidy ../... > go_mod_tidy.txt 2>&1 || true
echo "✓ Go mod tidy completed"
echo ""

# Terraform linting
echo "Terraform Linting..."
echo "==================="
echo ""

# Check if terraform fmt is available
if command -v terraform &> /dev/null; then
    echo "Running terraform fmt..."
    terraform fmt -check=true ../... > terraform_fmt.txt 2>&1 || true
    echo "✓ Terraform fmt completed"
    echo ""
    
    echo "Running terraform validate..."
    terraform validate ../... > terraform_validate.txt 2>&1 || true
    echo "✓ Terraform validate completed"
    echo ""
else
    echo "⚠ Terraform not available for linting"
fi

# Shell script linting
echo "Shell Script Linting..."
echo "======================"
echo ""

# Check if shellcheck is installed
if command -v shellcheck &> /dev/null; then
    echo "Running shellcheck..."
    shellcheck ../scripts/*.sh > shellcheck.txt 2>&1 || true
    echo "✓ Shellcheck completed"
    echo ""
else
    echo "⚠ shellcheck not available for linting"
fi

# Generate lint report
echo "Generating lint report..."
echo "========================"
echo ""

# Create lint report
cat > lint_report.md << EOF
# Lint Report

## Go Linting
- **golangci-lint**: $(grep -c "issues" go_lint.json 2>/dev/null || echo "0") issues
- **go vet**: $(wc -l < go_vet.txt 2>/dev/null || echo "0") issues
- **go fmt**: $(wc -l < go_fmt.txt 2>/dev/null || echo "0") issues
- **go mod tidy**: $(wc -l < go_mod_tidy.txt 2>/dev/null || echo "0") issues

## Terraform Linting
- **terraform fmt**: $(wc -l < terraform_fmt.txt 2>/dev/null || echo "0") issues
- **terraform validate**: $(wc -l < terraform_validate.txt 2>/dev/null || echo "0") issues

## Shell Script Linting
- **shellcheck**: $(wc -l < shellcheck.txt 2>/dev/null || echo "0") issues

## Summary
- **Total Issues**: $(($(grep -c "issues" go_lint.json 2>/dev/null || echo "0") + $(wc -l < go_vet.txt 2>/dev/null || echo "0") + $(wc -l < go_fmt.txt 2>/dev/null || echo "0") + $(wc -l < go_mod_tidy.txt 2>/dev/null || echo "0") + $(wc -l < terraform_fmt.txt 2>/dev/null || echo "0") + $(wc -l < terraform_validate.txt 2>/dev/null || echo "0") + $(wc -l < shellcheck.txt 2>/dev/null || echo "0")))

## Files
- **Go Lint**: go_lint.json
- **Go Vet**: go_vet.txt
- **Go Fmt**: go_fmt.txt
- **Go Mod Tidy**: go_mod_tidy.txt
- **Terraform Fmt**: terraform_fmt.txt
- **Terraform Validate**: terraform_validate.txt
- **Shellcheck**: shellcheck.txt
EOF

echo "✓ Lint report generated: lint_report.md"
echo ""

# Show lint summary
echo "Lint Summary:"
echo "============="
echo ""

# Go lint summary
echo "Go Linting:"
echo "  golangci-lint: $(grep -c "issues" go_lint.json 2>/dev/null || echo "0") issues"
echo "  go vet: $(wc -l < go_vet.txt 2>/dev/null || echo "0") issues"
echo "  go fmt: $(wc -l < go_fmt.txt 2>/dev/null || echo "0") issues"
echo "  go mod tidy: $(wc -l < go_mod_tidy.txt 2>/dev/null || echo "0") issues"
echo ""

# Terraform lint summary
echo "Terraform Linting:"
echo "  terraform fmt: $(wc -l < terraform_fmt.txt 2>/dev/null || echo "0") issues"
echo "  terraform validate: $(wc -l < terraform_validate.txt 2>/dev/null || echo "0") issues"
echo ""

# Shell script lint summary
echo "Shell Script Linting:"
echo "  shellcheck: $(wc -l < shellcheck.txt 2>/dev/null || echo "0") issues"
echo ""

# Show lint files
echo "Lint files generated:"
echo "===================="
ls -la *.json *.txt *.md 2>/dev/null || echo "No lint files found"
echo ""

# Show lint recommendations
echo "Lint Recommendations:"
echo "===================="
echo ""

# Go recommendations
if [ -f "go_lint.json" ] && [ "$(grep -c "issues" go_lint.json 2>/dev/null || echo "0")" -gt 0 ]; then
    echo "Go Linting Issues:"
    grep -o '"message":"[^"]*"' go_lint.json | head -5
    echo ""
fi

# Terraform recommendations
if [ -f "terraform_fmt.txt" ] && [ "$(wc -l < terraform_fmt.txt 2>/dev/null || echo "0")" -gt 0 ]; then
    echo "Terraform Fmt Issues:"
    head -5 terraform_fmt.txt
    echo ""
fi

# Shell script recommendations
if [ -f "shellcheck.txt" ] && [ "$(wc -l < shellcheck.txt 2>/dev/null || echo "0")" -gt 0 ]; then
    echo "Shellcheck Issues:"
    head -5 shellcheck.txt
    echo ""
fi

echo "Linting completed successfully!"
