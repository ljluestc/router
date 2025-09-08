# Multi-stage Dockerfile for Router Simulator
# Supports C++, Go, Rust, and Web components

# Base stage with common dependencies
FROM ubuntu:22.04 AS base

# Set environment variables
ENV DEBIAN_FRONTEND=noninteractive
ENV LANG=C.UTF-8
ENV LC_ALL=C.UTF-8

# Install system dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    pkg-config \
    git \
    curl \
    wget \
    ca-certificates \
    libpcap-dev \
    libssl-dev \
    libyaml-cpp-dev \
    libgtest-dev \
    && rm -rf /var/lib/apt/lists/*

# C++ Build stage
FROM base AS cpp-builder

WORKDIR /app

# Copy C++ source code
COPY include/ /app/include/
COPY src/ /app/src/
COPY CMakeLists.txt /app/
COPY tests/ /app/tests/

# Build C++ components
RUN mkdir build && cd build && \
    cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release && \
    ninja

# Go Build stage
FROM golang:1.21-alpine AS go-builder

WORKDIR /app

# Copy Go source code
COPY go/ /app/

# Download dependencies and build
RUN go mod download && \
    go build -o bin/cloudnet ./cmd/cloudnet

# Rust Build stage
FROM rust:1.70-slim AS rust-builder

WORKDIR /app

# Copy Rust source code
COPY rust/ /app/

# Build Rust components
RUN cargo build --release

# Web Build stage
FROM node:18-alpine AS web-builder

WORKDIR /app

# Copy web source code
COPY web/ /app/

# Install dependencies and build
RUN npm ci && \
    npm run build

# Final stage
FROM ubuntu:22.04 AS final

# Set environment variables
ENV DEBIAN_FRONTEND=noninteractive
ENV LANG=C.UTF-8
ENV LC_ALL=C.UTF-8

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    libpcap0.8 \
    libssl3 \
    libyaml-cpp0.8 \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Create app user
RUN useradd -m -s /bin/bash app

# Set working directory
WORKDIR /app

# Copy built binaries from previous stages
COPY --from=cpp-builder /app/build/routersim_tests /app/bin/
COPY --from=cpp-builder /app/build/routersim_cli /app/bin/
COPY --from=cpp-builder /app/build/routersim_demo /app/bin/

COPY --from=go-builder /app/bin/cloudnet /app/bin/

COPY --from=rust-builder /app/target/release/librouter_sim.so /app/lib/

COPY --from=web-builder /app/dist /app/web/

# Copy configuration and scripts
COPY config.yaml /app/
COPY scripts/ /app/scripts/
COPY scenarios/ /app/scenarios/

# Set permissions
RUN chmod +x /app/bin/* && \
    chmod +x /app/scripts/* && \
    chown -R app:app /app

# Switch to app user
USER app

# Expose ports
EXPOSE 8080 9090

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=5s --retries=3 \
    CMD curl -f http://localhost:8080/health || exit 1

# Default command
CMD ["/app/bin/cloudnet", "server"]