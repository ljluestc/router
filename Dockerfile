# Multi-Protocol Router Simulator Dockerfile
# Multi-stage build for optimized image size

# Build stage
FROM ubuntu:22.04 as builder

# Set environment variables
ENV DEBIAN_FRONTEND=noninteractive
ENV CMAKE_BUILD_TYPE=Release
ENV ENABLE_COVERAGE=OFF
ENV ENABLE_TESTS=ON

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    libpcap-dev \
    libyaml-cpp-dev \
    frr \
    iproute2 \
    net-tools \
    pkg-config \
    git \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy source code
COPY . .

# Create build directory and build
RUN mkdir build && cd build && \
    cmake -DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE \
          -DENABLE_COVERAGE=$ENABLE_COVERAGE \
          -DENABLE_TESTS=$ENABLE_TESTS \
          -DCMAKE_INSTALL_PREFIX=/usr/local \
          .. && \
    make -j$(nproc) && \
    make install

# Runtime stage
FROM ubuntu:22.04

# Set environment variables
ENV DEBIAN_FRONTEND=noninteractive
ENV PATH="/usr/local/bin:${PATH}"

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    libpcap0.8 \
    libyaml-cpp0.8 \
    frr \
    iproute2 \
    net-tools \
    iptables \
    tcpdump \
    iputils-ping \
    curl \
    && rm -rf /var/lib/apt/lists/*

# Create non-root user
RUN useradd -m -s /bin/bash router && \
    usermod -aG sudo router

# Copy built binaries from builder stage
COPY --from=builder /usr/local/bin/router_sim /usr/local/bin/
COPY --from=builder /usr/local/bin/router_sim_test /usr/local/bin/
COPY --from=builder /usr/local/include/router_sim /usr/local/include/router_sim/
COPY --from=builder /usr/local/lib/librouter_sim* /usr/local/lib/

# Copy example configurations
COPY examples/ /app/examples/
COPY docs/ /app/docs/

# Set up FRR configuration
RUN mkdir -p /etc/frr && \
    echo "hostname router-sim" > /etc/frr/zebra.conf && \
    echo "password zebra" >> /etc/frr/zebra.conf && \
    echo "enable password zebra" >> /etc/frr/zebra.conf && \
    chown -R frr:frr /etc/frr

# Create directories for runtime
RUN mkdir -p /var/log/router_sim && \
    mkdir -p /var/run/router_sim && \
    mkdir -p /etc/router_sim && \
    chown -R router:router /var/log/router_sim /var/run/router_sim /etc/router_sim

# Copy default configuration
COPY examples/basic_router.yaml /etc/router_sim/default.yaml

# Switch to non-root user
USER router

# Set working directory
WORKDIR /app

# Expose ports (if needed for web interface)
EXPOSE 8080

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=5s --retries=3 \
    CMD router_sim --version || exit 1

# Default command
CMD ["router_sim", "-c", "/etc/router_sim/default.yaml", "-i"]

# Labels
LABEL maintainer="Router Sim Team <team@router-sim.dev>"
LABEL description="Multi-Protocol Router Simulator with FRR integration"
LABEL version="1.0.0"
LABEL org.opencontainers.image.source="https://github.com/yourusername/router-sim"
LABEL org.opencontainers.image.description="Advanced network simulation with FRR integration, traffic shaping, and comprehensive testing"
LABEL org.opencontainers.image.licenses="MIT"
