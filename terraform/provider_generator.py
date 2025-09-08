#!/usr/bin/env python3
"""
Terraform Provider Generator for Router Simulator
Automatically generates Terraform providers for CloudPods and Aviatrix integration
"""

import os
import json
import yaml
import argparse
from typing import Dict, List, Any
from dataclasses import dataclass
from pathlib import Path

@dataclass
class ResourceDefinition:
    name: str
    type: str
    description: str
    attributes: List[Dict[str, Any]]
    required_attributes: List[str]
    optional_attributes: List[str]

class TerraformProviderGenerator:
    def __init__(self, output_dir: str = "terraform"):
        self.output_dir = Path(output_dir)
        self.output_dir.mkdir(exist_ok=True)
        
    def generate_cloudpods_provider(self) -> None:
        """Generate Terraform provider for CloudPods integration"""
        
        # CloudPods resource definitions
        resources = [
            ResourceDefinition(
                name="cloudpods_instance",
                type="yunionio_cloudpods_instance",
                description="CloudPods instance resource",
                attributes=[
                    {"name": "name", "type": "string", "description": "Instance name"},
                    {"name": "image_id", "type": "string", "description": "Image ID"},
                    {"name": "flavor_id", "type": "string", "description": "Flavor ID"},
                    {"name": "network_id", "type": "string", "description": "Network ID"},
                    {"name": "security_group_ids", "type": "list(string)", "description": "Security group IDs"},
                    {"name": "keypair", "type": "string", "description": "SSH keypair name"},
                    {"name": "tags", "type": "map(string)", "description": "Resource tags"},
                ],
                required_attributes=["name", "image_id", "flavor_id"],
                optional_attributes=["network_id", "security_group_ids", "keypair", "tags"]
            ),
            ResourceDefinition(
                name="cloudpods_network",
                type="yunionio_cloudpods_network",
                description="CloudPods network resource",
                attributes=[
                    {"name": "name", "type": "string", "description": "Network name"},
                    {"name": "cidr", "type": "string", "description": "Network CIDR"},
                    {"name": "vpc_id", "type": "string", "description": "VPC ID"},
                    {"name": "zone_id", "type": "string", "description": "Zone ID"},
                    {"name": "tags", "type": "map(string)", "description": "Resource tags"},
                ],
                required_attributes=["name", "cidr"],
                optional_attributes=["vpc_id", "zone_id", "tags"]
            ),
            ResourceDefinition(
                name="cloudpods_loadbalancer",
                type="yunionio_cloudpods_loadbalancer",
                description="CloudPods load balancer resource",
                attributes=[
                    {"name": "name", "type": "string", "description": "Load balancer name"},
                    {"name": "network_id", "type": "string", "description": "Network ID"},
                    {"name": "listeners", "type": "list(object)", "description": "Load balancer listeners"},
                    {"name": "backend_groups", "type": "list(object)", "description": "Backend groups"},
                    {"name": "tags", "type": "map(string)", "description": "Resource tags"},
                ],
                required_attributes=["name", "network_id"],
                optional_attributes=["listeners", "backend_groups", "tags"]
            ),
        ]
        
        self._generate_provider_schema("cloudpods", resources)
        self._generate_provider_go("cloudpods", resources)
        self._generate_examples("cloudpods", resources)
        
    def generate_aviatrix_provider(self) -> None:
        """Generate Terraform provider for Aviatrix integration"""
        
        # Aviatrix resource definitions
        resources = [
            ResourceDefinition(
                name="aviatrix_gateway",
                type="aviatrix_gateway",
                description="Aviatrix gateway resource",
                attributes=[
                    {"name": "gw_name", "type": "string", "description": "Gateway name"},
                    {"name": "cloud_type", "type": "number", "description": "Cloud type"},
                    {"name": "account_name", "type": "string", "description": "Account name"},
                    {"name": "region", "type": "string", "description": "Region"},
                    {"name": "vpc_id", "type": "string", "description": "VPC ID"},
                    {"name": "subnet", "type": "string", "description": "Subnet CIDR"},
                    {"name": "gw_size", "type": "string", "description": "Gateway size"},
                    {"name": "enable_vpn_access", "type": "bool", "description": "Enable VPN access"},
                    {"name": "enable_elb", "type": "bool", "description": "Enable ELB"},
                    {"name": "tags", "type": "map(string)", "description": "Resource tags"},
                ],
                required_attributes=["gw_name", "cloud_type", "account_name", "region", "vpc_id", "subnet", "gw_size"],
                optional_attributes=["enable_vpn_access", "enable_elb", "tags"]
            ),
            ResourceDefinition(
                name="aviatrix_transit_gateway",
                type="aviatrix_transit_gateway",
                description="Aviatrix transit gateway resource",
                attributes=[
                    {"name": "gw_name", "type": "string", "description": "Gateway name"},
                    {"name": "cloud_type", "type": "number", "description": "Cloud type"},
                    {"name": "account_name", "type": "string", "description": "Account name"},
                    {"name": "region", "type": "string", "description": "Region"},
                    {"name": "vpc_id", "type": "string", "description": "VPC ID"},
                    {"name": "subnet", "type": "string", "description": "Subnet CIDR"},
                    {"name": "gw_size", "type": "string", "description": "Gateway size"},
                    {"name": "enable_hybrid_connection", "type": "bool", "description": "Enable hybrid connection"},
                    {"name": "enable_firenet", "type": "bool", "description": "Enable FireNet"},
                    {"name": "tags", "type": "map(string)", "description": "Resource tags"},
                ],
                required_attributes=["gw_name", "cloud_type", "account_name", "region", "vpc_id", "subnet", "gw_size"],
                optional_attributes=["enable_hybrid_connection", "enable_firenet", "tags"]
            ),
            ResourceDefinition(
                name="aviatrix_spoke_gateway",
                type="aviatrix_spoke_gateway",
                description="Aviatrix spoke gateway resource",
                attributes=[
                    {"name": "gw_name", "type": "string", "description": "Gateway name"},
                    {"name": "cloud_type", "type": "number", "description": "Cloud type"},
                    {"name": "account_name", "type": "string", "description": "Account name"},
                    {"name": "region", "type": "string", "description": "Region"},
                    {"name": "vpc_id", "type": "string", "description": "VPC ID"},
                    {"name": "subnet", "type": "string", "description": "Subnet CIDR"},
                    {"name": "gw_size", "type": "string", "description": "Gateway size"},
                    {"name": "transit_gw", "type": "string", "description": "Transit gateway name"},
                    {"name": "enable_vpn_access", "type": "bool", "description": "Enable VPN access"},
                    {"name": "tags", "type": "map(string)", "description": "Resource tags"},
                ],
                required_attributes=["gw_name", "cloud_type", "account_name", "region", "vpc_id", "subnet", "gw_size"],
                optional_attributes=["transit_gw", "enable_vpn_access", "tags"]
            ),
        ]
        
        self._generate_provider_schema("aviatrix", resources)
        self._generate_provider_go("aviatrix", resources)
        self._generate_examples("aviatrix", resources)
        
    def generate_router_sim_provider(self) -> None:
        """Generate Terraform provider for Router Simulator"""
        
        # Router Simulator resource definitions
        resources = [
            ResourceDefinition(
                name="router_sim_interface",
                type="router_sim_interface",
                description="Router simulator interface resource",
                attributes=[
                    {"name": "name", "type": "string", "description": "Interface name"},
                    {"name": "type", "type": "string", "description": "Interface type"},
                    {"name": "ip_address", "type": "string", "description": "IP address with CIDR"},
                    {"name": "status", "type": "string", "description": "Interface status"},
                    {"name": "description", "type": "string", "description": "Interface description"},
                ],
                required_attributes=["name", "type", "ip_address"],
                optional_attributes=["status", "description"]
            ),
            ResourceDefinition(
                name="router_sim_protocol",
                type="router_sim_protocol",
                description="Router simulator protocol resource",
                attributes=[
                    {"name": "name", "type": "string", "description": "Protocol name"},
                    {"name": "type", "type": "string", "description": "Protocol type (bgp, ospf, isis)"},
                    {"name": "enabled", "type": "bool", "description": "Protocol enabled"},
                    {"name": "config", "type": "map(string)", "description": "Protocol configuration"},
                    {"name": "neighbors", "type": "list(object)", "description": "Protocol neighbors"},
                ],
                required_attributes=["name", "type"],
                optional_attributes=["enabled", "config", "neighbors"]
            ),
            ResourceDefinition(
                name="router_sim_traffic_shaping",
                type="router_sim_traffic_shaping",
                description="Router simulator traffic shaping resource",
                attributes=[
                    {"name": "name", "type": "string", "description": "Traffic shaper name"},
                    {"name": "type", "type": "string", "description": "Shaping type (token_bucket, wfq)"},
                    {"name": "interface", "type": "string", "description": "Target interface"},
                    {"name": "rate", "type": "string", "description": "Traffic rate"},
                    {"name": "burst_size", "type": "string", "description": "Burst size"},
                    {"name": "classes", "type": "list(object)", "description": "Traffic classes"},
                ],
                required_attributes=["name", "type", "interface"],
                optional_attributes=["rate", "burst_size", "classes"]
            ),
            ResourceDefinition(
                name="router_sim_impairment",
                type="router_sim_impairment",
                description="Router simulator network impairment resource",
                attributes=[
                    {"name": "name", "type": "string", "description": "Impairment name"},
                    {"name": "interface", "type": "string", "description": "Target interface"},
                    {"name": "type", "type": "string", "description": "Impairment type"},
                    {"name": "value", "type": "number", "description": "Impairment value"},
                    {"name": "variation", "type": "number", "description": "Impairment variation"},
                    {"name": "enabled", "type": "bool", "description": "Impairment enabled"},
                ],
                required_attributes=["name", "interface", "type"],
                optional_attributes=["value", "variation", "enabled"]
            ),
        ]
        
        self._generate_provider_schema("router_sim", resources)
        self._generate_provider_go("router_sim", resources)
        self._generate_examples("router_sim", resources)
        
    def _generate_provider_schema(self, provider_name: str, resources: List[ResourceDefinition]) -> None:
        """Generate Terraform provider schema"""
        
        provider_dir = self.output_dir / provider_name
        provider_dir.mkdir(exist_ok=True)
        
        schema = {
            "provider": {
                provider_name: {
                    "version": "~> 1.0",
                    "source": f"local/{provider_name}",
                    "configuration_aliases": [f"{provider_name}.alias"]
                }
            },
            "resource": {},
            "data_source": {}
        }
        
        for resource in resources:
            resource_schema = {
                "type": "object",
                "properties": {},
                "required": resource.required_attributes
            }
            
            for attr in resource.attributes:
                resource_schema["properties"][attr["name"]] = {
                    "type": attr["type"],
                    "description": attr["description"]
                }
            
            schema["resource"][resource.type] = resource_schema
        
        schema_file = provider_dir / "schema.json"
        with open(schema_file, 'w') as f:
            json.dump(schema, f, indent=2)
            
    def _generate_provider_go(self, provider_name: str, resources: List[ResourceDefinition]) -> None:
        """Generate Terraform provider Go code"""
        
        provider_dir = self.output_dir / provider_name
        provider_dir.mkdir(exist_ok=True)
        
        go_code = f'''package main

import (
    "context"
    "fmt"
    "github.com/hashicorp/terraform-plugin-sdk/v2/diag"
    "github.com/hashicorp/terraform-plugin-sdk/v2/helper/schema"
    "github.com/hashicorp/terraform-plugin-sdk/v2/plugin"
)

func main() {{
    plugin.Serve(&plugin.ServeOpts{{
        ProviderFunc: Provider,
    }})
}}

func Provider() *schema.Provider {{
    return &schema.Provider{{
        Schema: map[string]*schema.Schema{{
            "endpoint": {{
                Type:        schema.TypeString,
                Required:    true,
                Description: "API endpoint URL",
            }},
            "api_key": {{
                Type:        schema.TypeString,
                Required:    true,
                Description: "API key for authentication",
            }},
        }},
        ResourcesMap: map[string]*schema.Resource{{
'''
        
        for resource in resources:
            go_code += f'''            "{resource.type}": resource{resource.name.title().replace("_", "")}(),
'''
        
        go_code += '''        },
        DataSourcesMap: map[string]*schema.Resource{},
        ConfigureContextFunc: providerConfigure,
    }
}

func providerConfigure(ctx context.Context, d *schema.ResourceData) (interface{}, diag.Diagnostics) {
    endpoint := d.Get("endpoint").(string)
    apiKey := d.Get("api_key").(string)
    
    // Initialize API client
    client := &APIClient{
        Endpoint: endpoint,
        APIKey:   apiKey,
    }
    
    return client, nil
}

type APIClient struct {
    Endpoint string
    APIKey   string
}
'''
        
        for resource in resources:
            go_code += self._generate_resource_go(resource)
        
        go_file = provider_dir / "main.go"
        with open(go_file, 'w') as f:
            f.write(go_code)
            
    def _generate_resource_go(self, resource: ResourceDefinition) -> str:
        """Generate Go code for a specific resource"""
        
        resource_name = resource.name.title().replace("_", "")
        
        go_code = f'''
func resource{resource_name}() *schema.Resource {{
    return &schema.Resource{{
        CreateContext: resource{resource_name}Create,
        ReadContext:   resource{resource_name}Read,
        UpdateContext: resource{resource_name}Update,
        DeleteContext: resource{resource_name}Delete,
        Schema: map[string]*schema.Schema{{
'''
        
        for attr in resource.attributes:
            go_code += f'''            "{attr["name"]}": {{
                Type:        schema.Type{attr["type"].title().replace("_", "")},
                Required:    {str(attr["name"] in resource.required_attributes).lower()},
                Description: "{attr["description"]}",
'''
            if attr["name"] in resource.optional_attributes:
                go_code += f'''                Optional: true,
'''
            go_code += f'''            }},
'''
        
        go_code += f'''        }},
    }}
}}

func resource{resource_name}Create(ctx context.Context, d *schema.ResourceData, m interface{{}}) diag.Diagnostics {{
    client := m.(*APIClient)
    
    // Create resource via API
    id := fmt.Sprintf("%s-%%d", time.Now().Unix())
    d.SetId(id)
    
    return resource{resource_name}Read(ctx, d, m)
}}

func resource{resource_name}Read(ctx context.Context, d *schema.ResourceData, m interface{{}}) diag.Diagnostics {{
    client := m.(*APIClient)
    
    // Read resource from API
    // Implementation would call client API
    
    return nil
}}

func resource{resource_name}Update(ctx context.Context, d *schema.ResourceData, m interface{{}}) diag.Diagnostics {{
    client := m.(*APIClient)
    
    // Update resource via API
    // Implementation would call client API
    
    return resource{resource_name}Read(ctx, d, m)
}}

func resource{resource_name}Delete(ctx context.Context, d *schema.ResourceData, m interface{{}}) diag.Diagnostics {{
    client := m.(*APIClient)
    
    // Delete resource via API
    // Implementation would call client API
    
    d.SetId("")
    return nil
}}
'''
        
        return go_code
        
    def _generate_examples(self, provider_name: str, resources: List[ResourceDefinition]) -> None:
        """Generate Terraform examples"""
        
        examples_dir = self.output_dir / provider_name / "examples"
        examples_dir.mkdir(exist_ok=True)
        
        # Generate main.tf example
        main_tf = f'''terraform {{
  required_providers {{
    {provider_name} = {{
      source  = "local/{provider_name}"
      version = "~> 1.0"
    }}
  }}
}}

provider "{provider_name}" {{
  endpoint = "http://localhost:8080"
  api_key  = "your-api-key"
}}

'''
        
        for resource in resources:
            main_tf += f'''resource "{resource.type}" "example" {{
'''
            for attr in resource.required_attributes:
                main_tf += f'''  {attr} = "example-{attr}"\n'''
            for attr in resource.optional_attributes[:3]:  # Limit to first 3 optional attributes
                main_tf += f'''  {attr} = "example-{attr}"\n'''
            main_tf += f'''}}\n\n'''
        
        main_tf_file = examples_dir / "main.tf"
        with open(main_tf_file, 'w') as f:
            f.write(main_tf)
            
        # Generate variables.tf
        variables_tf = '''variable "endpoint" {
  description = "API endpoint URL"
  type        = string
  default     = "http://localhost:8080"
}

variable "api_key" {
  description = "API key for authentication"
  type        = string
  sensitive   = true
}
'''
        
        variables_tf_file = examples_dir / "variables.tf"
        with open(variables_tf_file, 'w') as f:
            f.write(variables_tf)
            
        # Generate outputs.tf
        outputs_tf = '''output "provider_info" {
  description = "Provider information"
  value = {
    endpoint = var.endpoint
    version  = "1.0.0"
  }
}
'''
        
        outputs_tf_file = examples_dir / "outputs.tf"
        with open(outputs_tf_file, 'w') as f:
            f.write(outputs_tf)

def main():
    parser = argparse.ArgumentParser(description='Generate Terraform providers for Router Simulator')
    parser.add_argument('--output-dir', default='terraform', help='Output directory for generated providers')
    parser.add_argument('--providers', nargs='+', default=['cloudpods', 'aviatrix', 'router_sim'],
                       help='Providers to generate')
    
    args = parser.parse_args()
    
    generator = TerraformProviderGenerator(args.output_dir)
    
    for provider in args.providers:
        print(f"Generating {provider} provider...")
        
        if provider == 'cloudpods':
            generator.generate_cloudpods_provider()
        elif provider == 'aviatrix':
            generator.generate_aviatrix_provider()
        elif provider == 'router_sim':
            generator.generate_router_sim_provider()
        else:
            print(f"Unknown provider: {provider}")
            continue
            
        print(f"✓ {provider} provider generated successfully")
    
    print(f"\nAll providers generated in {args.output_dir}/")
    print("To use the providers:")
    print("1. Copy the generated files to your Terraform configuration")
    print("2. Run 'terraform init' to initialize the providers")
    print("3. Run 'terraform plan' to see the execution plan")
    print("4. Run 'terraform apply' to create the resources")

if __name__ == "__main__":
    main()
