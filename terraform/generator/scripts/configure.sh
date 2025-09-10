#!/bin/bash

# Configure Terraform provider generator
set -e

echo "Configuring Terraform Provider Generator"
echo "======================================="
echo ""

# Create config directory
mkdir -p config

# CloudPods configuration
echo "Configuring CloudPods..."
cat > config/cloudpods.conf << EOF
# CloudPods Configuration
CLOUDPODS_API_KEY=""
CLOUDPODS_BASE_URL="https://api.cloudpods.example.com"
CLOUDPODS_REGION="us-west-1"
CLOUDPODS_TIMEOUT="30s"
CLOUDPODS_RETRIES="3"
EOF

# Aviatrix configuration
echo "Configuring Aviatrix..."
cat > config/aviatrix.conf << EOF
# Aviatrix Configuration
AVIATRIX_API_KEY=""
AVIATRIX_BASE_URL="https://api.aviatrix.example.com"
AVIATRIX_REGION="us-west-1"
AVIATRIX_TIMEOUT="30s"
AVIATRIX_RETRIES="3"
EOF

# Router Simulator configuration
echo "Configuring Router Simulator..."
cat > config/router-sim.conf << EOF
# Router Simulator Configuration
ROUTER_SIM_ENDPOINT="http://localhost:8080"
ROUTER_SIM_API_KEY=""
ROUTER_SIM_TIMEOUT="30s"
ROUTER_SIM_RETRIES="3"
EOF

# Global configuration
echo "Configuring global settings..."
cat > config/global.conf << EOF
# Global Configuration
LOG_LEVEL="info"
LOG_FILE="generator.log"
BACKUP_ENABLED="true"
BACKUP_RETENTION_DAYS="30"
MONITOR_ENABLED="true"
MONITOR_INTERVAL="30s"
EOF

# Environment configuration
echo "Configuring environment..."
cat > config/env.conf << EOF
# Environment Configuration
ENVIRONMENT="development"
DEBUG="false"
VERBOSE="false"
DRY_RUN="false"
EOF

echo "✓ Configuration files created in config/"

# Ask for API keys
echo ""
echo "Please configure your API keys:"
echo ""

# CloudPods API key
read -p "CloudPods API Key (optional): " CLOUDPODS_API_KEY
if [ ! -z "$CLOUDPODS_API_KEY" ]; then
    sed -i "s/CLOUDPODS_API_KEY=\"\"/CLOUDPODS_API_KEY=\"$CLOUDPODS_API_KEY\"/" config/cloudpods.conf
fi

# Aviatrix API key
read -p "Aviatrix API Key (optional): " AVIATRIX_API_KEY
if [ ! -z "$AVIATRIX_API_KEY" ]; then
    sed -i "s/AVIATRIX_API_KEY=\"\"/AVIATRIX_API_KEY=\"$AVIATRIX_API_KEY\"/" config/aviatrix.conf
fi

# Router Simulator API key
read -p "Router Simulator API Key (optional): " ROUTER_SIM_API_KEY
if [ ! -z "$ROUTER_SIM_API_KEY" ]; then
    sed -i "s/ROUTER_SIM_API_KEY=\"\"/ROUTER_SIM_API_KEY=\"$ROUTER_SIM_API_KEY\"/" config/router-sim.conf
fi

# Ask for endpoints
echo ""
echo "Please configure your endpoints:"
echo ""

# CloudPods endpoint
read -p "CloudPods Base URL (default: https://api.cloudpods.example.com): " CLOUDPODS_BASE_URL
if [ ! -z "$CLOUDPODS_BASE_URL" ]; then
    sed -i "s|CLOUDPODS_BASE_URL=\"https://api.cloudpods.example.com\"|CLOUDPODS_BASE_URL=\"$CLOUDPODS_BASE_URL\"|" config/cloudpods.conf
fi

# Aviatrix endpoint
read -p "Aviatrix Base URL (default: https://api.aviatrix.example.com): " AVIATRIX_BASE_URL
if [ ! -z "$AVIATRIX_BASE_URL" ]; then
    sed -i "s|AVIATRIX_BASE_URL=\"https://api.aviatrix.example.com\"|AVIATRIX_BASE_URL=\"$AVIATRIX_BASE_URL\"|" config/aviatrix.conf
fi

# Router Simulator endpoint
read -p "Router Simulator Endpoint (default: http://localhost:8080): " ROUTER_SIM_ENDPOINT
if [ ! -z "$ROUTER_SIM_ENDPOINT" ]; then
    sed -i "s|ROUTER_SIM_ENDPOINT=\"http://localhost:8080\"|ROUTER_SIM_ENDPOINT=\"$ROUTER_SIM_ENDPOINT\"|" config/router-sim.conf
fi

# Ask for regions
echo ""
echo "Please configure your regions:"
echo ""

# CloudPods region
read -p "CloudPods Region (default: us-west-1): " CLOUDPODS_REGION
if [ ! -z "$CLOUDPODS_REGION" ]; then
    sed -i "s/CLOUDPODS_REGION=\"us-west-1\"/CLOUDPODS_REGION=\"$CLOUDPODS_REGION\"/" config/cloudpods.conf
fi

# Aviatrix region
read -p "Aviatrix Region (default: us-west-1): " AVIATRIX_REGION
if [ ! -z "$AVIATRIX_REGION" ]; then
    sed -i "s/AVIATRIX_REGION=\"us-west-1\"/AVIATRIX_REGION=\"$AVIATRIX_REGION\"/" config/aviatrix.conf
fi

echo ""
echo "Configuration completed successfully!"
echo ""
echo "Configuration files:"
echo "  - config/cloudpods.conf"
echo "  - config/aviatrix.conf"
echo "  - config/router-sim.conf"
echo "  - config/global.conf"
echo "  - config/env.conf"
echo ""
echo "You can now run:"
echo "  ./scripts/start.sh"
