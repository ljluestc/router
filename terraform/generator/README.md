# Terraform Provider Generator

This tool automatically generates Terraform provider configurations for CloudPods, Aviatrix, and Router Simulator.

## Features

- **CloudPods Provider**: Manages VMs, networks, and load balancers
- **Aviatrix Provider**: Manages gateways, transit gateways, and spoke gateways
- **Router Simulator Provider**: Manages router instances and simulation scenarios
- **Automatic Generation**: Creates complete Terraform configurations
- **Schema Generation**: Generates provider schemas and documentation
- **Example Generation**: Creates example configurations

## Usage

### Generate CloudPods Provider

```bash
go run main.go cloudpods ./output/cloudpods
```

### Generate Aviatrix Provider

```bash
go run main.go aviatrix ./output/aviatrix
```

### Generate Router Simulator Provider

```bash
go run main.go router-sim ./output/router-sim
```

## Generated Files

Each provider generates the following files:

- `main.tf` - Main Terraform configuration
- `variables.tf` - Variable definitions
- `outputs.tf` - Output definitions
- `provider_schema.json` - Provider schema
- `examples/basic.tf` - Basic example configuration

## CloudPods Resources

- `cloudpods_vm` - Virtual machine management
- `cloudpods_network` - Network management
- `cloudpods_load_balancer` - Load balancer management

## Aviatrix Resources

- `aviatrix_gateway` - Gateway management
- `aviatrix_transit_gateway` - Transit gateway management
- `aviatrix_spoke_gateway` - Spoke gateway management

## Router Simulator Resources

- `router_sim_router` - Router instance management
- `router_sim_scenario` - Simulation scenario management

## Configuration

### CloudPods Variables

- `cloudpods_api_key` - CloudPods API key (required)
- `cloudpods_base_url` - CloudPods base URL (default: https://api.cloudpods.example.com)
- `region` - CloudPods region (default: us-west-1)

### Aviatrix Variables

- `aviatrix_api_key` - Aviatrix API key (required)
- `aviatrix_base_url` - Aviatrix base URL (default: https://api.aviatrix.example.com)
- `region` - Aviatrix region (default: us-west-1)

### Router Simulator Variables

- `router_sim_endpoint` - Router simulator endpoint (default: http://localhost:8080)
- `api_key` - API key (optional)

## Examples

### CloudPods VM

```hcl
resource "cloudpods_vm" "web_server" {
  name          = "web-server"
  instance_type = "t3.micro"
  image         = "ubuntu-20.04"
  network       = "vpc-12345"
  zone          = "us-west-1a"
  tags = {
    Environment = "production"
    Role        = "web"
  }
}
```

### Aviatrix Gateway

```hcl
resource "aviatrix_gateway" "aws_gateway" {
  name         = "aws-gateway"
  cloud_type   = "AWS"
  region       = "us-west-1"
  vpc          = "vpc-12345"
  subnet       = "subnet-12345"
  gateway_size = "t3.micro"
  tags = {
    Environment = "production"
    Cloud       = "AWS"
  }
}
```

### Router Simulator

```hcl
resource "router_sim_router" "router1" {
  name     = "router1"
  router_id = "1.1.1.1"
  hostname = "router1.example.com"
  interfaces = [
    {
      name = "eth0"
      ip   = "192.168.1.1/24"
    }
  ]
  protocols = {
    bgp = {
      asn = 65001
      neighbors = ["192.168.1.2"]
    }
  }
}
```

## Development

### Prerequisites

- Go 1.21+
- Terraform 1.0+

### Building

```bash
go build -o terraform-generator main.go
```

### Running

```bash
./terraform-generator <provider> <output_dir>
```

## License

MIT License - see LICENSE file for details.
