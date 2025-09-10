package cloudpods

import (
	"context"
	"encoding/json"
	"fmt"
	"net/http"
	"time"

	"router-sim/internal/config"
)

// Client represents a CloudPods API client
type Client struct {
	config *config.CloudPodsConfig
	client *http.Client
}

// NewClient creates a new CloudPods client
func NewClient(config *config.CloudPodsConfig) *Client {
	return &Client{
		config: config,
		client: &http.Client{
			Timeout: config.Timeout,
		},
	}
}

// CloudPods API Response structures
type CloudPodsResponse struct {
	Data    interface{} `json:"data"`
	Total   int         `json:"total"`
	Success bool        `json:"success"`
	Message string      `json:"message"`
}

type CloudPodsResource struct {
	ID          string                 `json:"id"`
	Name        string                 `json:"name"`
	Type        string                 `json:"type"`
	Status      string                 `json:"status"`
	Region      string                 `json:"region"`
	Zone        string                 `json:"zone"`
	CreatedAt   time.Time              `json:"created_at"`
	UpdatedAt   time.Time              `json:"updated_at"`
	Properties  map[string]interface{} `json:"properties"`
	Tags        map[string]string      `json:"tags"`
}

type CloudPodsVM struct {
	CloudPodsResource
	CPU        int    `json:"cpu"`
	Memory     int    `json:"memory"`
	Disk       int    `json:"disk"`
	Image      string `json:"image"`
	VPC        string `json:"vpc"`
	Subnet     string `json:"subnet"`
	PublicIP   string `json:"public_ip"`
	PrivateIP  string `json:"private_ip"`
	SSHKey     string `json:"ssh_key"`
}

type CloudPodsVPC struct {
	CloudPodsResource
	CIDR       string   `json:"cidr"`
	Subnets    []string `json:"subnets"`
	Gateways   []string `json:"gateways"`
	Routes     []string `json:"routes"`
	SecurityGroups []string `json:"security_groups"`
}

type CloudPodsSubnet struct {
	CloudPodsResource
	VPC        string `json:"vpc"`
	CIDR       string `json:"cidr"`
	Gateway    string `json:"gateway"`
	DHCP       bool   `json:"dhcp"`
	DNS        []string `json:"dns"`
}

type CloudPodsSecurityGroup struct {
	CloudPodsResource
	Rules      []SecurityGroupRule `json:"rules"`
	VPC        string              `json:"vpc"`
}

type SecurityGroupRule struct {
	Direction    string `json:"direction"`
	Protocol     string `json:"protocol"`
	PortRange    string `json:"port_range"`
	Source       string `json:"source"`
	Destination  string `json:"destination"`
	Action       string `json:"action"`
	Priority     int    `json:"priority"`
}

type CloudPodsLoadBalancer struct {
	CloudPodsResource
	VPC         string   `json:"vpc"`
	Subnet      string   `json:"subnet"`
	Listeners   []string `json:"listeners"`
	Backends    []string `json:"backends"`
	HealthCheck string   `json:"health_check"`
}

// GetVMs retrieves all VMs from CloudPods
func (c *Client) GetVMs(ctx context.Context) ([]CloudPodsVM, error) {
	url := fmt.Sprintf("%s/api/v1/vms", c.config.API.BaseURL)
	
	req, err := http.NewRequestWithContext(ctx, "GET", url, nil)
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	// Add authentication headers
	req.SetBasicAuth(c.config.Auth.Username, c.config.Auth.Password)
	req.Header.Set("Content-Type", "application/json")

	resp, err := c.client.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to execute request: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("API request failed with status: %d", resp.StatusCode)
	}

	var response CloudPodsResponse
	if err := json.NewDecoder(resp.Body).Decode(&response); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	if !response.Success {
		return nil, fmt.Errorf("API returned error: %s", response.Message)
	}

	// Convert data to VMs
	vms := make([]CloudPodsVM, 0)
	if data, ok := response.Data.([]interface{}); ok {
		for _, item := range data {
			if itemMap, ok := item.(map[string]interface{}); ok {
				var vm CloudPodsVM
				if err := mapToStruct(itemMap, &vm); err == nil {
					vms = append(vms, vm)
				}
			}
		}
	}

	return vms, nil
}

// GetVPCs retrieves all VPCs from CloudPods
func (c *Client) GetVPCs(ctx context.Context) ([]CloudPodsVPC, error) {
	url := fmt.Sprintf("%s/api/v1/vpcs", c.config.API.BaseURL)
	
	req, err := http.NewRequestWithContext(ctx, "GET", url, nil)
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	req.SetBasicAuth(c.config.Auth.Username, c.config.Auth.Password)
	req.Header.Set("Content-Type", "application/json")

	resp, err := c.client.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to execute request: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("API request failed with status: %d", resp.StatusCode)
	}

	var response CloudPodsResponse
	if err := json.NewDecoder(resp.Body).Decode(&response); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	if !response.Success {
		return nil, fmt.Errorf("API returned error: %s", response.Message)
	}

	vpcs := make([]CloudPodsVPC, 0)
	if data, ok := response.Data.([]interface{}); ok {
		for _, item := range data {
			if itemMap, ok := item.(map[string]interface{}); ok {
				var vpc CloudPodsVPC
				if err := mapToStruct(itemMap, &vpc); err == nil {
					vpcs = append(vpcs, vpc)
				}
			}
		}
	}

	return vpcs, nil
}

// GetSubnets retrieves all subnets from CloudPods
func (c *Client) GetSubnets(ctx context.Context) ([]CloudPodsSubnet, error) {
	url := fmt.Sprintf("%s/api/v1/subnets", c.config.API.BaseURL)
	
	req, err := http.NewRequestWithContext(ctx, "GET", url, nil)
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	req.SetBasicAuth(c.config.Auth.Username, c.config.Auth.Password)
	req.Header.Set("Content-Type", "application/json")

	resp, err := c.client.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to execute request: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("API request failed with status: %d", resp.StatusCode)
	}

	var response CloudPodsResponse
	if err := json.NewDecoder(resp.Body).Decode(&response); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	if !response.Success {
		return nil, fmt.Errorf("API returned error: %s", response.Message)
	}

	subnets := make([]CloudPodsSubnet, 0)
	if data, ok := response.Data.([]interface{}); ok {
		for _, item := range data {
			if itemMap, ok := item.(map[string]interface{}); ok {
				var subnet CloudPodsSubnet
				if err := mapToStruct(itemMap, &subnet); err == nil {
					subnets = append(subnets, subnet)
				}
			}
		}
	}

	return subnets, nil
}

// GetSecurityGroups retrieves all security groups from CloudPods
func (c *Client) GetSecurityGroups(ctx context.Context) ([]CloudPodsSecurityGroup, error) {
	url := fmt.Sprintf("%s/api/v1/security-groups", c.config.API.BaseURL)
	
	req, err := http.NewRequestWithContext(ctx, "GET", url, nil)
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	req.SetBasicAuth(c.config.Auth.Username, c.config.Auth.Password)
	req.Header.Set("Content-Type", "application/json")

	resp, err := c.client.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to execute request: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("API request failed with status: %d", resp.StatusCode)
	}

	var response CloudPodsResponse
	if err := json.NewDecoder(resp.Body).Decode(&response); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	if !response.Success {
		return nil, fmt.Errorf("API returned error: %s", response.Message)
	}

	securityGroups := make([]CloudPodsSecurityGroup, 0)
	if data, ok := response.Data.([]interface{}); ok {
		for _, item := range data {
			if itemMap, ok := item.(map[string]interface{}); ok {
				var sg CloudPodsSecurityGroup
				if err := mapToStruct(itemMap, &sg); err == nil {
					securityGroups = append(securityGroups, sg)
				}
			}
		}
	}

	return securityGroups, nil
}

// GetLoadBalancers retrieves all load balancers from CloudPods
func (c *Client) GetLoadBalancers(ctx context.Context) ([]CloudPodsLoadBalancer, error) {
	url := fmt.Sprintf("%s/api/v1/load-balancers", c.config.API.BaseURL)
	
	req, err := http.NewRequestWithContext(ctx, "GET", url, nil)
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	req.SetBasicAuth(c.config.Auth.Username, c.config.Auth.Password)
	req.Header.Set("Content-Type", "application/json")

	resp, err := c.client.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to execute request: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("API request failed with status: %d", resp.StatusCode)
	}

	var response CloudPodsResponse
	if err := json.NewDecoder(resp.Body).Decode(&response); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	if !response.Success {
		return nil, fmt.Errorf("API returned error: %s", response.Message)
	}

	loadBalancers := make([]CloudPodsLoadBalancer, 0)
	if data, ok := response.Data.([]interface{}); ok {
		for _, item := range data {
			if itemMap, ok := item.(map[string]interface{}); ok {
				var lb CloudPodsLoadBalancer
				if err := mapToStruct(itemMap, &lb); err == nil {
					loadBalancers = append(loadBalancers, lb)
				}
			}
		}
	}

	return loadBalancers, nil
}

// CreateVM creates a new VM in CloudPods
func (c *Client) CreateVM(ctx context.Context, vm CloudPodsVM) (*CloudPodsVM, error) {
	url := fmt.Sprintf("%s/api/v1/vms", c.config.API.BaseURL)
	
	vmData, err := json.Marshal(vm)
	if err != nil {
		return nil, fmt.Errorf("failed to marshal VM data: %w", err)
	}

	req, err := http.NewRequestWithContext(ctx, "POST", url, bytes.NewBuffer(vmData))
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	req.SetBasicAuth(c.config.Auth.Username, c.config.Auth.Password)
	req.Header.Set("Content-Type", "application/json")

	resp, err := c.client.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to execute request: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusCreated {
		return nil, fmt.Errorf("API request failed with status: %d", resp.StatusCode)
	}

	var response CloudPodsResponse
	if err := json.NewDecoder(resp.Body).Decode(&response); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	if !response.Success {
		return nil, fmt.Errorf("API returned error: %s", response.Message)
	}

	var createdVM CloudPodsVM
	if data, ok := response.Data.(map[string]interface{}); ok {
		if err := mapToStruct(data, &createdVM); err != nil {
			return nil, fmt.Errorf("failed to convert response data: %w", err)
		}
	}

	return &createdVM, nil
}

// DeleteVM deletes a VM from CloudPods
func (c *Client) DeleteVM(ctx context.Context, vmID string) error {
	url := fmt.Sprintf("%s/api/v1/vms/%s", c.config.API.BaseURL, vmID)
	
	req, err := http.NewRequestWithContext(ctx, "DELETE", url, nil)
	if err != nil {
		return fmt.Errorf("failed to create request: %w", err)
	}

	req.SetBasicAuth(c.config.Auth.Username, c.config.Auth.Password)
	req.Header.Set("Content-Type", "application/json")

	resp, err := c.client.Do(req)
	if err != nil {
		return fmt.Errorf("failed to execute request: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return fmt.Errorf("API request failed with status: %d", resp.StatusCode)
	}

	return nil
}

// GetResourceMetrics retrieves metrics for a specific resource
func (c *Client) GetResourceMetrics(ctx context.Context, resourceID string, metricType string) (map[string]interface{}, error) {
	url := fmt.Sprintf("%s/api/v1/metrics/%s/%s", c.config.API.BaseURL, resourceID, metricType)
	
	req, err := http.NewRequestWithContext(ctx, "GET", url, nil)
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	req.SetBasicAuth(c.config.Auth.Username, c.config.Auth.Password)
	req.Header.Set("Content-Type", "application/json")

	resp, err := c.client.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to execute request: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("API request failed with status: %d", resp.StatusCode)
	}

	var response CloudPodsResponse
	if err := json.NewDecoder(resp.Body).Decode(&response); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	if !response.Success {
		return nil, fmt.Errorf("API returned error: %s", response.Message)
	}

	if data, ok := response.Data.(map[string]interface{}); ok {
		return data, nil
	}

	return nil, fmt.Errorf("invalid response data format")
}

// Helper function to convert map to struct
func mapToStruct(data map[string]interface{}, target interface{}) error {
	jsonData, err := json.Marshal(data)
	if err != nil {
		return err
	}
	return json.Unmarshal(jsonData, target)
}
