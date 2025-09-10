package main

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"log"
	"os"
	"path/filepath"
	"strings"
	"text/template"
)

// TerraformProvider represents a Terraform provider configuration
type TerraformProvider struct {
	Name        string                 `json:"name"`
	Version     string                 `json:"version"`
	Description string                 `json:"description"`
	Resources   []TerraformResource    `json:"resources"`
	DataSources []TerraformDataSource  `json:"data_sources"`
	Variables   []TerraformVariable    `json:"variables"`
	Outputs     []TerraformOutput      `json:"outputs"`
	Providers   map[string]interface{} `json:"providers"`
}

// TerraformResource represents a Terraform resource
type TerraformResource struct {
	Name        string                 `json:"name"`
	Type        string                 `json:"type"`
	Description string                 `json:"description"`
	Arguments   []TerraformArgument    `json:"arguments"`
	Attributes  []TerraformAttribute   `json:"attributes"`
	Required    []string               `json:"required"`
	Optional    []string               `json:"optional"`
	Computed    []string               `json:"computed"`
	DependsOn   []string               `json:"depends_on"`
	Lifecycle   map[string]interface{} `json:"lifecycle"`
}

// TerraformDataSource represents a Terraform data source
type TerraformDataSource struct {
	Name        string               `json:"name"`
	Type        string               `json:"type"`
	Description string               `json:"description"`
	Arguments   []TerraformArgument  `json:"arguments"`
	Attributes  []TerraformAttribute `json:"attributes"`
}

// TerraformArgument represents a Terraform argument
type TerraformArgument struct {
	Name        string      `json:"name"`
	Type        string      `json:"type"`
	Description string      `json:"description"`
	Required    bool        `json:"required"`
	Optional    bool        `json:"optional"`
	Computed    bool        `json:"computed"`
	Default     interface{} `json:"default"`
	Validation  []string    `json:"validation"`
}

// TerraformAttribute represents a Terraform attribute
type TerraformAttribute struct {
	Name        string `json:"name"`
	Type        string `json:"type"`
	Description string `json:"description"`
	Computed    bool   `json:"computed"`
}

// TerraformVariable represents a Terraform variable
type TerraformVariable struct {
	Name        string      `json:"name"`
	Type        string      `json:"type"`
	Description string      `json:"description"`
	Default     interface{} `json:"default"`
	Validation  []string    `json:"validation"`
}

// TerraformOutput represents a Terraform output
type TerraformOutput struct {
	Name        string `json:"name"`
	Description string `json:"description"`
	Value       string `json:"value"`
	Sensitive   bool   `json:"sensitive"`
}

// CloudPodsProvider generates Terraform provider for CloudPods
func CloudPodsProvider() *TerraformProvider {
	return &TerraformProvider{
		Name:        "cloudpods",
		Version:     "1.0.0",
		Description: "Terraform provider for CloudPods cloud platform",
		Resources: []TerraformResource{
			{
				Name:        "cloudpods_vm",
				Type:        "cloudpods_vm",
				Description: "Manages a CloudPods virtual machine",
				Arguments: []TerraformArgument{
					{Name: "name", Type: "string", Description: "Name of the VM", Required: true},
					{Name: "instance_type", Type: "string", Description: "Instance type", Required: true},
					{Name: "image", Type: "string", Description: "Image ID", Required: true},
					{Name: "network", Type: "string", Description: "Network ID", Required: true},
					{Name: "zone", Type: "string", Description: "Availability zone", Optional: true},
					{Name: "tags", Type: "map(string)", Description: "Tags", Optional: true},
				},
				Attributes: []TerraformAttribute{
					{Name: "id", Type: "string", Description: "VM ID", Computed: true},
					{Name: "ip_address", Type: "string", Description: "Private IP address", Computed: true},
					{Name: "public_ip", Type: "string", Description: "Public IP address", Computed: true},
					{Name: "status", Type: "string", Description: "VM status", Computed: true},
					{Name: "created_at", Type: "string", Description: "Creation timestamp", Computed: true},
				},
				Required: []string{"name", "instance_type", "image", "network"},
				Optional: []string{"zone", "tags"},
				Computed: []string{"id", "ip_address", "public_ip", "status", "created_at"},
			},
			{
				Name:        "cloudpods_network",
				Type:        "cloudpods_network",
				Description: "Manages a CloudPods network",
				Arguments: []TerraformArgument{
					{Name: "name", Type: "string", Description: "Network name", Required: true},
					{Name: "cidr", Type: "string", Description: "CIDR block", Required: true},
					{Name: "vpc", Type: "string", Description: "VPC ID", Required: true},
					{Name: "description", Type: "string", Description: "Network description", Optional: true},
					{Name: "tags", Type: "map(string)", Description: "Tags", Optional: true},
				},
				Attributes: []TerraformAttribute{
					{Name: "id", Type: "string", Description: "Network ID", Computed: true},
					{Name: "status", Type: "string", Description: "Network status", Computed: true},
					{Name: "created_at", Type: "string", Description: "Creation timestamp", Computed: true},
				},
				Required: []string{"name", "cidr", "vpc"},
				Optional: []string{"description", "tags"},
				Computed: []string{"id", "status", "created_at"},
			},
			{
				Name:        "cloudpods_load_balancer",
				Type:        "cloudpods_load_balancer",
				Description: "Manages a CloudPods load balancer",
				Arguments: []TerraformArgument{
					{Name: "name", Type: "string", Description: "Load balancer name", Required: true},
					{Name: "type", Type: "string", Description: "Load balancer type", Required: true},
					{Name: "vms", Type: "list(string)", Description: "Target VMs", Required: true},
					{Name: "listeners", Type: "list(object)", Description: "Listener configuration", Optional: true},
					{Name: "health_check", Type: "string", Description: "Health check configuration", Optional: true},
					{Name: "tags", Type: "map(string)", Description: "Tags", Optional: true},
				},
				Attributes: []TerraformAttribute{
					{Name: "id", Type: "string", Description: "Load balancer ID", Computed: true},
					{Name: "status", Type: "string", Description: "Load balancer status", Computed: true},
					{Name: "endpoint", Type: "string", Description: "Load balancer endpoint", Computed: true},
					{Name: "created_at", Type: "string", Description: "Creation timestamp", Computed: true},
				},
				Required: []string{"name", "type", "vms"},
				Optional: []string{"listeners", "health_check", "tags"},
				Computed: []string{"id", "status", "endpoint", "created_at"},
			},
		},
		DataSources: []TerraformDataSource{
			{
				Name:        "cloudpods_vm",
				Type:        "cloudpods_vm",
				Description: "Get information about a CloudPods VM",
				Arguments: []TerraformArgument{
					{Name: "id", Type: "string", Description: "VM ID", Required: true},
				},
				Attributes: []TerraformAttribute{
					{Name: "name", Type: "string", Description: "VM name", Computed: true},
					{Name: "instance_type", Type: "string", Description: "Instance type", Computed: true},
					{Name: "ip_address", Type: "string", Description: "Private IP address", Computed: true},
					{Name: "public_ip", Type: "string", Description: "Public IP address", Computed: true},
					{Name: "status", Type: "string", Description: "VM status", Computed: true},
				},
			},
			{
				Name:        "cloudpods_network",
				Type:        "cloudpods_network",
				Description: "Get information about a CloudPods network",
				Arguments: []TerraformArgument{
					{Name: "id", Type: "string", Description: "Network ID", Required: true},
				},
				Attributes: []TerraformAttribute{
					{Name: "name", Type: "string", Description: "Network name", Computed: true},
					{Name: "cidr", Type: "string", Description: "CIDR block", Computed: true},
					{Name: "status", Type: "string", Description: "Network status", Computed: true},
				},
			},
		},
		Variables: []TerraformVariable{
			{Name: "cloudpods_api_key", Type: "string", Description: "CloudPods API key", Required: true},
			{Name: "cloudpods_base_url", Type: "string", Description: "CloudPods base URL", Default: "https://api.cloudpods.example.com"},
			{Name: "region", Type: "string", Description: "CloudPods region", Default: "us-west-1"},
		},
		Outputs: []TerraformOutput{
			{Name: "vm_ids", Description: "List of VM IDs", Value: "[for vm in cloudpods_vm.vms : vm.id]"},
			{Name: "network_ids", Description: "List of network IDs", Value: "[for net in cloudpods_network.networks : net.id]"},
		},
		Providers: map[string]interface{}{
			"cloudpods": map[string]interface{}{
				"api_key":   "${var.cloudpods_api_key}",
				"base_url":  "${var.cloudpods_base_url}",
				"region":    "${var.region}",
			},
		},
	}
}

// AviatrixProvider generates Terraform provider for Aviatrix
func AviatrixProvider() *TerraformProvider {
	return &TerraformProvider{
		Name:        "aviatrix",
		Version:     "1.0.0",
		Description: "Terraform provider for Aviatrix multi-cloud networking",
		Resources: []TerraformResource{
			{
				Name:        "aviatrix_gateway",
				Type:        "aviatrix_gateway",
				Description: "Manages an Aviatrix gateway",
				Arguments: []TerraformArgument{
					{Name: "name", Type: "string", Description: "Gateway name", Required: true},
					{Name: "cloud_type", Type: "string", Description: "Cloud type", Required: true},
					{Name: "region", Type: "string", Description: "Region", Required: true},
					{Name: "vpc", Type: "string", Description: "VPC ID", Required: true},
					{Name: "subnet", Type: "string", Description: "Subnet ID", Required: true},
					{Name: "gateway_size", Type: "string", Description: "Gateway size", Optional: true, Default: "t3.micro"},
					{Name: "tags", Type: "map(string)", Description: "Tags", Optional: true},
				},
				Attributes: []TerraformAttribute{
					{Name: "id", Type: "string", Description: "Gateway ID", Computed: true},
					{Name: "public_ip", Type: "string", Description: "Public IP address", Computed: true},
					{Name: "private_ip", Type: "string", Description: "Private IP address", Computed: true},
					{Name: "state", Type: "string", Description: "Gateway state", Computed: true},
					{Name: "status", Type: "string", Description: "Gateway status", Computed: true},
				},
				Required: []string{"name", "cloud_type", "region", "vpc", "subnet"},
				Optional: []string{"gateway_size", "tags"},
				Computed: []string{"id", "public_ip", "private_ip", "state", "status"},
			},
			{
				Name:        "aviatrix_transit_gateway",
				Type:        "aviatrix_transit_gateway",
				Description: "Manages an Aviatrix transit gateway",
				Arguments: []TerraformArgument{
					{Name: "name", Type: "string", Description: "Transit gateway name", Required: true},
					{Name: "cloud_type", Type: "string", Description: "Cloud type", Required: true},
					{Name: "region", Type: "string", Description: "Region", Required: true},
					{Name: "vpc", Type: "string", Description: "VPC ID", Required: true},
					{Name: "subnet", Type: "string", Description: "Subnet ID", Required: true},
					{Name: "gateway_size", Type: "string", Description: "Gateway size", Optional: true, Default: "t3.medium"},
					{Name: "enable_ha", Type: "bool", Description: "Enable HA", Optional: true, Default: false},
					{Name: "tags", Type: "map(string)", Description: "Tags", Optional: true},
				},
				Attributes: []TerraformAttribute{
					{Name: "id", Type: "string", Description: "Transit gateway ID", Computed: true},
					{Name: "public_ip", Type: "string", Description: "Public IP address", Computed: true},
					{Name: "private_ip", Type: "string", Description: "Private IP address", Computed: true},
					{Name: "state", Type: "string", Description: "Transit gateway state", Computed: true},
					{Name: "status", Type: "string", Description: "Transit gateway status", Computed: true},
				},
				Required: []string{"name", "cloud_type", "region", "vpc", "subnet"},
				Optional: []string{"gateway_size", "enable_ha", "tags"},
				Computed: []string{"id", "public_ip", "private_ip", "state", "status"},
			},
			{
				Name:        "aviatrix_spoke_gateway",
				Type:        "aviatrix_spoke_gateway",
				Description: "Manages an Aviatrix spoke gateway",
				Arguments: []TerraformArgument{
					{Name: "name", Type: "string", Description: "Spoke gateway name", Required: true},
					{Name: "cloud_type", Type: "string", Description: "Cloud type", Required: true},
					{Name: "region", Type: "string", Description: "Region", Required: true},
					{Name: "vpc", Type: "string", Description: "VPC ID", Required: true},
					{Name: "subnet", Type: "string", Description: "Subnet ID", Required: true},
					{Name: "transit_gateway", Type: "string", Description: "Transit gateway name", Required: true},
					{Name: "gateway_size", Type: "string", Description: "Gateway size", Optional: true, Default: "t3.small"},
					{Name: "tags", Type: "map(string)", Description: "Tags", Optional: true},
				},
				Attributes: []TerraformAttribute{
					{Name: "id", Type: "string", Description: "Spoke gateway ID", Computed: true},
					{Name: "public_ip", Type: "string", Description: "Public IP address", Computed: true},
					{Name: "private_ip", Type: "string", Description: "Private IP address", Computed: true},
					{Name: "state", Type: "string", Description: "Spoke gateway state", Computed: true},
					{Name: "status", Type: "string", Description: "Spoke gateway status", Computed: true},
				},
				Required: []string{"name", "cloud_type", "region", "vpc", "subnet", "transit_gateway"},
				Optional: []string{"gateway_size", "tags"},
				Computed: []string{"id", "public_ip", "private_ip", "state", "status"},
			},
		},
		DataSources: []TerraformDataSource{
			{
				Name:        "aviatrix_gateway",
				Type:        "aviatrix_gateway",
				Description: "Get information about an Aviatrix gateway",
				Arguments: []TerraformArgument{
					{Name: "name", Type: "string", Description: "Gateway name", Required: true},
				},
				Attributes: []TerraformAttribute{
					{Name: "id", Type: "string", Description: "Gateway ID", Computed: true},
					{Name: "public_ip", Type: "string", Description: "Public IP address", Computed: true},
					{Name: "private_ip", Type: "string", Description: "Private IP address", Computed: true},
					{Name: "state", Type: "string", Description: "Gateway state", Computed: true},
					{Name: "status", Type: "string", Description: "Gateway status", Computed: true},
				},
			},
		},
		Variables: []TerraformVariable{
			{Name: "aviatrix_api_key", Type: "string", Description: "Aviatrix API key", Required: true},
			{Name: "aviatrix_base_url", Type: "string", Description: "Aviatrix base URL", Default: "https://api.aviatrix.example.com"},
			{Name: "region", Type: "string", Description: "Aviatrix region", Default: "us-west-1"},
		},
		Outputs: []TerraformOutput{
			{Name: "gateway_ids", Description: "List of gateway IDs", Value: "[for gw in aviatrix_gateway.gateways : gw.id]"},
			{Name: "transit_gateway_ids", Description: "List of transit gateway IDs", Value: "[for tgw in aviatrix_transit_gateway.transit_gateways : tgw.id]"},
		},
		Providers: map[string]interface{}{
			"aviatrix": map[string]interface{}{
				"api_key":  "${var.aviatrix_api_key}",
				"base_url": "${var.aviatrix_base_url}",
				"region":   "${var.region}",
			},
		},
	}
}

// RouterSimProvider generates Terraform provider for Router Simulator
func RouterSimProvider() *TerraformProvider {
	return &TerraformProvider{
		Name:        "router-sim",
		Version:     "1.0.0",
		Description: "Terraform provider for Router Simulator",
		Resources: []TerraformResource{
			{
				Name:        "router_sim_router",
				Type:        "router_sim_router",
				Description: "Manages a router simulator instance",
				Arguments: []TerraformArgument{
					{Name: "name", Type: "string", Description: "Router name", Required: true},
					{Name: "router_id", Type: "string", Description: "Router ID", Required: true},
					{Name: "hostname", Type: "string", Description: "Router hostname", Optional: true},
					{Name: "interfaces", Type: "list(object)", Description: "Router interfaces", Optional: true},
					{Name: "protocols", Type: "map(object)", Description: "Routing protocols", Optional: true},
					{Name: "traffic_shaping", Type: "object", Description: "Traffic shaping configuration", Optional: true},
					{Name: "netem_impairments", Type: "list(object)", Description: "Network impairments", Optional: true},
				},
				Attributes: []TerraformAttribute{
					{Name: "id", Type: "string", Description: "Router ID", Computed: true},
					{Name: "status", Type: "string", Description: "Router status", Computed: true},
					{Name: "routes", Type: "list(object)", Description: "Routing table", Computed: true},
					{Name: "neighbors", Type: "list(object)", Description: "Protocol neighbors", Computed: true},
				},
				Required: []string{"name", "router_id"},
				Optional: []string{"hostname", "interfaces", "protocols", "traffic_shaping", "netem_impairments"},
				Computed: []string{"id", "status", "routes", "neighbors"},
			},
			{
				Name:        "router_sim_scenario",
				Type:        "router_sim_scenario",
				Description: "Manages a router simulation scenario",
				Arguments: []TerraformArgument{
					{Name: "name", Type: "string", Description: "Scenario name", Required: true},
					{Name: "description", Type: "string", Description: "Scenario description", Optional: true},
					{Name: "routers", Type: "list(string)", Description: "Router names", Required: true},
					{Name: "duration", Type: "number", Description: "Simulation duration in seconds", Optional: true, Default: 3600},
					{Name: "config", Type: "object", Description: "Scenario configuration", Required: true},
				},
				Attributes: []TerraformAttribute{
					{Name: "id", Type: "string", Description: "Scenario ID", Computed: true},
					{Name: "status", Type: "string", Description: "Scenario status", Computed: true},
					{Name: "results", Type: "object", Description: "Simulation results", Computed: true},
				},
				Required: []string{"name", "routers", "config"},
				Optional: []string{"description", "duration"},
				Computed: []string{"id", "status", "results"},
			},
		},
		DataSources: []TerraformDataSource{
			{
				Name:        "router_sim_router",
				Type:        "router_sim_router",
				Description: "Get information about a router simulator instance",
				Arguments: []TerraformArgument{
					{Name: "name", Type: "string", Description: "Router name", Required: true},
				},
				Attributes: []TerraformAttribute{
					{Name: "id", Type: "string", Description: "Router ID", Computed: true},
					{Name: "status", Type: "string", Description: "Router status", Computed: true},
					{Name: "routes", Type: "list(object)", Description: "Routing table", Computed: true},
					{Name: "neighbors", Type: "list(object)", Description: "Protocol neighbors", Computed: true},
				},
			},
		},
		Variables: []TerraformVariable{
			{Name: "router_sim_endpoint", Type: "string", Description: "Router simulator endpoint", Default: "http://localhost:8080"},
			{Name: "api_key", Type: "string", Description: "API key", Optional: true},
		},
		Outputs: []TerraformOutput{
			{Name: "router_ids", Description: "List of router IDs", Value: "[for router in router_sim_router.routers : router.id]"},
			{Name: "scenario_ids", Description: "List of scenario IDs", Value: "[for scenario in router_sim_scenario.scenarios : scenario.id]"},
		},
		Providers: map[string]interface{}{
			"router-sim": map[string]interface{}{
				"endpoint": "${var.router_sim_endpoint}",
				"api_key":  "${var.api_key}",
			},
		},
	}
}

// GenerateTerraformFiles generates Terraform configuration files
func GenerateTerraformFiles(provider *TerraformProvider, outputDir string) error {
	// Create output directory
	if err := os.MkdirAll(outputDir, 0755); err != nil {
		return fmt.Errorf("failed to create output directory: %v", err)
	}

	// Generate main.tf
	if err := generateMainTF(provider, outputDir); err != nil {
		return fmt.Errorf("failed to generate main.tf: %v", err)
	}

	// Generate variables.tf
	if err := generateVariablesTF(provider, outputDir); err != nil {
		return fmt.Errorf("failed to generate variables.tf: %v", err)
	}

	// Generate outputs.tf
	if err := generateOutputsTF(provider, outputDir); err != nil {
		return fmt.Errorf("failed to generate outputs.tf: %v", err)
	}

	// Generate provider schema
	if err := generateProviderSchema(provider, outputDir); err != nil {
		return fmt.Errorf("failed to generate provider schema: %v", err)
	}

	// Generate examples
	if err := generateExamples(provider, outputDir); err != nil {
		return fmt.Errorf("failed to generate examples: %v", err)
	}

	return nil
}

// generateMainTF generates the main.tf file
func generateMainTF(provider *TerraformProvider, outputDir string) error {
	tmpl := `terraform {
  required_providers {
    {{.Name}} = {
      source  = "local/{{.Name}}"
      version = "{{.Version}}"
    }
  }
}

{{range $key, $value := .Providers}}
provider "{{$key}}" {
{{range $k, $v := $value}}
  {{$k}} = {{$v}}
{{end}}
}
{{end}}

{{range .Resources}}
resource "{{.Type}}" "{{.Name}}" {
  {{range .Arguments}}
  {{if .Required}}
  {{.Name}} = var.{{.Name}}
  {{else if .Optional}}
  {{.Name}} = var.{{.Name}}
  {{end}}
  {{end}}
}
{{end}}

{{range .DataSources}}
data "{{.Type}}" "{{.Name}}" {
  {{range .Arguments}}
  {{if .Required}}
  {{.Name}} = var.{{.Name}}
  {{end}}
  {{end}}
}
{{end}}
`

	t, err := template.New("main").Parse(tmpl)
	if err != nil {
		return err
	}

	file, err := os.Create(filepath.Join(outputDir, "main.tf"))
	if err != nil {
		return err
	}
	defer file.Close()

	return t.Execute(file, provider)
}

// generateVariablesTF generates the variables.tf file
func generateVariablesTF(provider *TerraformProvider, outputDir string) error {
	tmpl := `{{range .Variables}}
variable "{{.Name}}" {
  type        = {{.Type}}
  description = "{{.Description}}"
  {{if .Default}}
  default     = {{.Default}}
  {{end}}
  {{if .Validation}}
  validation {
    {{range .Validation}}
    {{.}}
    {{end}}
  }
  {{end}}
}
{{end}}
`

	t, err := template.New("variables").Parse(tmpl)
	if err != nil {
		return err
	}

	file, err := os.Create(filepath.Join(outputDir, "variables.tf"))
	if err != nil {
		return err
	}
	defer file.Close()

	return t.Execute(file, provider)
}

// generateOutputsTF generates the outputs.tf file
func generateOutputsTF(provider *TerraformProvider, outputDir string) error {
	tmpl := `{{range .Outputs}}
output "{{.Name}}" {
  description = "{{.Description}}"
  value       = {{.Value}}
  {{if .Sensitive}}
  sensitive   = true
  {{end}}
}
{{end}}
`

	t, err := template.New("outputs").Parse(tmpl)
	if err != nil {
		return err
	}

	file, err := os.Create(filepath.Join(outputDir, "outputs.tf"))
	if err != nil {
		return err
	}
	defer file.Close()

	return t.Execute(file, provider)
}

// generateProviderSchema generates the provider schema
func generateProviderSchema(provider *TerraformProvider, outputDir string) error {
	schema := map[string]interface{}{
		"provider": map[string]interface{}{
			"name":    provider.Name,
			"version": provider.Version,
			"resources": func() map[string]interface{} {
				resources := make(map[string]interface{})
				for _, resource := range provider.Resources {
					resources[resource.Type] = resource
				}
				return resources
			}(),
			"data_sources": func() map[string]interface{} {
				dataSources := make(map[string]interface{})
				for _, dataSource := range provider.DataSources {
					dataSources[dataSource.Type] = dataSource
				}
				return dataSources
			}(),
		},
	}

	data, err := json.MarshalIndent(schema, "", "  ")
	if err != nil {
		return err
	}

	return ioutil.WriteFile(filepath.Join(outputDir, "provider_schema.json"), data, 0644)
}

// generateExamples generates example configurations
func generateExamples(provider *TerraformProvider, outputDir string) error {
	examplesDir := filepath.Join(outputDir, "examples")
	if err := os.MkdirAll(examplesDir, 0755); err != nil {
		return err
	}

	// Generate basic example
	basicExample := fmt.Sprintf(`# Basic %s example
terraform {
  required_providers {
    %s = {
      source  = "local/%s"
      version = "%s"
    }
  }
}

provider "%s" {
  # Add provider configuration here
}

# Add resource examples here
`, provider.Name, provider.Name, provider.Name, provider.Version, provider.Name)

	return ioutil.WriteFile(filepath.Join(examplesDir, "basic.tf"), []byte(basicExample), 0644)
}

func main() {
	if len(os.Args) < 2 {
		log.Fatal("Usage: go run main.go <provider> <output_dir>")
	}

	providerName := os.Args[1]
	outputDir := os.Args[2]

	var provider *TerraformProvider

	switch strings.ToLower(providerName) {
	case "cloudpods":
		provider = CloudPodsProvider()
	case "aviatrix":
		provider = AviatrixProvider()
	case "router-sim":
		provider = RouterSimProvider()
	default:
		log.Fatalf("Unknown provider: %s", providerName)
	}

	if err := GenerateTerraformFiles(provider, outputDir); err != nil {
		log.Fatalf("Failed to generate Terraform files: %v", err)
	}

	fmt.Printf("Successfully generated Terraform provider for %s in %s\n", providerName, outputDir)
}
