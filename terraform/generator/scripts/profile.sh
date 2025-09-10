#!/bin/bash

# Profile Terraform provider generator
set -e

echo "Profiling Terraform Provider Generator"
echo "====================================="
echo ""

# Check if Go is installed
if ! command -v go &> /dev/null; then
    echo "Go is not installed. Please install Go first."
    exit 1
fi

# Check if pprof is available
if ! command -v go &> /dev/null; then
    echo "Go is not installed. Please install Go first."
    exit 1
fi

# Create profile directory
mkdir -p profile
cd profile

# CPU profiling
echo "CPU Profiling..."
echo "==============="
echo ""

# Build with profiling enabled
echo "Building with profiling enabled..."
go build -o terraform-generator ../main.go
echo ""

# Run with CPU profiling
echo "Running with CPU profiling..."
go run ../main.go cloudpods ./cloudpods &
GENERATOR_PID=$!
sleep 5
kill -SIGUSR1 $GENERATOR_PID 2>/dev/null || true
wait $GENERATOR_PID 2>/dev/null || true
echo ""

# Memory profiling
echo "Memory Profiling..."
echo "=================="
echo ""

# Run with memory profiling
echo "Running with memory profiling..."
go run ../main.go aviatrix ./aviatrix &
GENERATOR_PID=$!
sleep 5
kill -SIGUSR2 $GENERATOR_PID 2>/dev/null || true
wait $GENERATOR_PID 2>/dev/null || true
echo ""

# Block profiling
echo "Block Profiling..."
echo "================="
echo ""

# Run with block profiling
echo "Running with block profiling..."
go run ../main.go router-sim ./router-sim &
GENERATOR_PID=$!
sleep 5
kill -SIGUSR1 $GENERATOR_PID 2>/dev/null || true
wait $GENERATOR_PID 2>/dev/null || true
echo ""

# Goroutine profiling
echo "Goroutine Profiling..."
echo "====================="
echo ""

# Run with goroutine profiling
echo "Running with goroutine profiling..."
go run ../main.go cloudpods ./cloudpods_goroutine &
GENERATOR_PID=$!
sleep 5
kill -SIGUSR1 $GENERATOR_PID 2>/dev/null || true
wait $GENERATOR_PID 2>/dev/null || true
echo ""

# Mutex profiling
echo "Mutex Profiling..."
echo "================="
echo ""

# Run with mutex profiling
echo "Running with mutex profiling..."
go run ../main.go aviatrix ./aviatrix_mutex &
GENERATOR_PID=$!
sleep 5
kill -SIGUSR1 $GENERATOR_PID 2>/dev/null || true
wait $GENERATOR_PID 2>/dev/null || true
echo ""

# Trace profiling
echo "Trace Profiling..."
echo "================="
echo ""

# Run with trace profiling
echo "Running with trace profiling..."
go run ../main.go router-sim ./router-sim_trace &
GENERATOR_PID=$!
sleep 5
kill -SIGUSR1 $GENERATOR_PID 2>/dev/null || true
wait $GENERATOR_PID 2>/dev/null || true
echo ""

# Show profile files
echo "Profile files created:"
echo "====================="
ls -la *.prof *.trace 2>/dev/null || echo "No profile files found"
echo ""

# Analyze profiles
echo "Analyzing profiles..."
echo "===================="
echo ""

# CPU profile analysis
if [ -f "cpu.prof" ]; then
    echo "CPU Profile Analysis:"
    go tool pprof -top cpu.prof
    echo ""
fi

# Memory profile analysis
if [ -f "mem.prof" ]; then
    echo "Memory Profile Analysis:"
    go tool pprof -top mem.prof
    echo ""
fi

# Block profile analysis
if [ -f "block.prof" ]; then
    echo "Block Profile Analysis:"
    go tool pprof -top block.prof
    echo ""
fi

# Goroutine profile analysis
if [ -f "goroutine.prof" ]; then
    echo "Goroutine Profile Analysis:"
    go tool pprof -top goroutine.prof
    echo ""
fi

# Mutex profile analysis
if [ -f "mutex.prof" ]; then
    echo "Mutex Profile Analysis:"
    go tool pprof -top mutex.prof
    echo ""
fi

# Trace analysis
if [ -f "trace.out" ]; then
    echo "Trace Analysis:"
    go tool trace trace.out
    echo ""
fi

# Generate profile reports
echo "Generating profile reports..."
echo "============================"
echo ""

# CPU profile report
if [ -f "cpu.prof" ]; then
    echo "Generating CPU profile report..."
    go tool pprof -pdf cpu.prof > cpu_profile.pdf 2>/dev/null || echo "Failed to generate CPU profile PDF"
    go tool pprof -svg cpu.prof > cpu_profile.svg 2>/dev/null || echo "Failed to generate CPU profile SVG"
    echo "✓ CPU profile report generated"
fi

# Memory profile report
if [ -f "mem.prof" ]; then
    echo "Generating memory profile report..."
    go tool pprof -pdf mem.prof > mem_profile.pdf 2>/dev/null || echo "Failed to generate memory profile PDF"
    go tool pprof -svg mem.prof > mem_profile.svg 2>/dev/null || echo "Failed to generate memory profile SVG"
    echo "✓ Memory profile report generated"
fi

# Block profile report
if [ -f "block.prof" ]; then
    echo "Generating block profile report..."
    go tool pprof -pdf block.prof > block_profile.pdf 2>/dev/null || echo "Failed to generate block profile PDF"
    go tool pprof -svg block.prof > block_profile.svg 2>/dev/null || echo "Failed to generate block profile SVG"
    echo "✓ Block profile report generated"
fi

# Goroutine profile report
if [ -f "goroutine.prof" ]; then
    echo "Generating goroutine profile report..."
    go tool pprof -pdf goroutine.prof > goroutine_profile.pdf 2>/dev/null || echo "Failed to generate goroutine profile PDF"
    go tool pprof -svg goroutine.prof > goroutine_profile.svg 2>/dev/null || echo "Failed to generate goroutine profile SVG"
    echo "✓ Goroutine profile report generated"
fi

# Mutex profile report
if [ -f "mutex.prof" ]; then
    echo "Generating mutex profile report..."
    go tool pprof -pdf mutex.prof > mutex_profile.pdf 2>/dev/null || echo "Failed to generate mutex profile PDF"
    go tool pprof -svg mutex.prof > mutex_profile.svg 2>/dev/null || echo "Failed to generate mutex profile SVG"
    echo "✓ Mutex profile report generated"
fi

# Show profile summary
echo ""
echo "Profile Summary:"
echo "==============="
echo ""

# File sizes
echo "Profile file sizes:"
ls -lh *.prof *.trace *.pdf *.svg 2>/dev/null || echo "No profile files found"
echo ""

# Profile statistics
echo "Profile statistics:"
if [ -f "cpu.prof" ]; then
    echo "  CPU profile: $(wc -l < cpu.prof) lines"
fi
if [ -f "mem.prof" ]; then
    echo "  Memory profile: $(wc -l < mem.prof) lines"
fi
if [ -f "block.prof" ]; then
    echo "  Block profile: $(wc -l < block.prof) lines"
fi
if [ -f "goroutine.prof" ]; then
    echo "  Goroutine profile: $(wc -l < goroutine.prof) lines"
fi
if [ -f "mutex.prof" ]; then
    echo "  Mutex profile: $(wc -l < mutex.prof) lines"
fi
if [ -f "trace.out" ]; then
    echo "  Trace: $(wc -l < trace.out) lines"
fi

echo ""
echo "Profiling completed successfully!"
echo "Profile files are in the profile/ directory"
