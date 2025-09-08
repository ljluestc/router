#!/bin/bash

# Router Simulator Environment Activation Script

# Set environment variables
export ROUTER_SIM_ROOT="$(pwd)"
export PATH="$ROUTER_SIM_ROOT/bin:$PATH"
export LD_LIBRARY_PATH="$ROUTER_SIM_ROOT/lib:$LD_LIBRARY_PATH"
export PKG_CONFIG_PATH="$ROUTER_SIM_ROOT/lib/pkgconfig:$PKG_CONFIG_PATH"

# Go environment
export GOPATH="$ROUTER_SIM_ROOT/go"
export GOBIN="$ROUTER_SIM_ROOT/bin"
export GO111MODULE=on

# Rust environment
export CARGO_HOME="$ROUTER_SIM_ROOT/.cargo"
export RUST_LOG=info

# Build directories
export BUILD_DIR="$ROUTER_SIM_ROOT/build"
export INSTALL_DIR="$ROUTER_SIM_ROOT/install"

# Create necessary directories
mkdir -p "$BUILD_DIR" "$INSTALL_DIR" "$ROUTER_SIM_ROOT/bin" "$ROUTER_SIM_ROOT/lib"

# Set up aliases
alias build="pixi run build"
alias test="pixi run test"
alias clean="pixi run clean"
alias go-build="pixi run go-build"
alias rust-build="pixi run rust-build"
alias web-build="pixi run web-build"

echo "Router Simulator environment activated!"
echo "Available commands: build, test, clean, go-build, rust-build, web-build"
