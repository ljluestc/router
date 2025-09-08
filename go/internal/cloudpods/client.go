package cloudpods

import (
	"bytes"
	"context"
	"encoding/json"
	"fmt"
	"io"
	"net/http"
	"time"

	"router-sim/internal/config"
)

// Client represents a CloudPods client
type Client struct {
	config     config.CloudPodsConfig
	httpClient *http.Client
	baseURL    string
}

// NewClient creates a new CloudPods client
func NewClient(config config.CloudPodsConfig) (*Client, error) {
	httpClient := &http.Client{
		Timeout: config.Timeout,
	}

	baseURL := fmt.Sprintf("%s/api/v1", config.Endpoint)

	return &Client{
		config:     config,
		httpClient: httpClient,
		baseURL:    baseURL,
	}, nil
}

// CloudResource represents a cloud resource
type CloudResource struct {
	ID          string            `json:"id"`
	Name        string            `json:"name"`
	Type        string            `json:"type"`
	Status      string            `json:"status"`
	Region      string            `json:"region"`
	Zone        string            `json:"zone"`
	Provider    string            `json:"provider"`
	CreatedAt   time.Time         `json:"created_at"`
	UpdatedAt   time.Time         `json:"updated_at"`
	Tags        map[string]string `json:"tags"`
	Properties  map[string]interface{} `json:"properties"`
}

// VirtualMachine represents a virtual machine
type VirtualMachine struct {
	CloudResource
	CPU        int    `json:"cpu"`
	Memory     int    `json:"memory"`
	Disk       int    `json:"disk"`
	OS         string `json:"os"`
	IPAddress  string `json:"ip_address"`
	State      string `json:"state"`
	PowerState string `json:"power_state"`
}

// Network represents a network
type Network struct {
	CloudResource
	CIDR       string `json:"cidr"`
	Gateway    string `json:"gateway"`
	VLAN       int    `json:"vlan"`
	VPC        string `json:"vpc"`
	Subnet     string `json:"subnet"`
}

// LoadBalancer represents a load balancer
type LoadBalancer struct {
	CloudResource
	Type        string   `json:"type"`
	Algorithm   string   `json:"algorithm"`
	Ports       []Port   `json:"ports"`
	Backends    []string `json:"backends"`
	HealthCheck HealthCheck `json:"health_check"`
}

// Port represents a port configuration
type Port struct {
	Protocol string `json:"protocol"`
	Port     int    `json:"port"`
	Target   int    `json:"target"`
}

// HealthCheck represents health check configuration
type HealthCheck struct {
	Protocol string `json:"protocol"`
	Path     string `json:"path"`
	Port     int    `json:"port"`
	Interval int    `json:"interval"`
	Timeout  int    `json:"timeout"`
	Retries  int    `json:"retries"`
}

// SecurityGroup represents a security group
type SecurityGroup struct {
	CloudResource
	Rules []SecurityRule `json:"rules"`
}

// SecurityRule represents a security rule
type SecurityRule struct {
	Direction   string `json:"direction"`
	Protocol    string `json:"protocol"`
	Port        int    `json:"port"`
	Source      string `json:"source"`
	Destination string `json:"destination"`
	Action      string `json:"action"`
	Priority    int    `json:"priority"`
}

// GetVirtualMachines retrieves virtual machines
func (c *Client) GetVirtualMachines(ctx context.Context) ([]VirtualMachine, error) {
	url := fmt.Sprintf("%s/vms", c.baseURL)
	
	req, err := http.NewRequestWithContext(ctx, "GET", url, nil)
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	c.setHeaders(req)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to execute request: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("request failed with status: %d", resp.StatusCode)
	}

	var result struct {
		Data []VirtualMachine `json:"data"`
	}

	if err := json.NewDecoder(resp.Body).Decode(&result); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	return result.Data, nil
}

// GetNetworks retrieves networks
func (c *Client) GetNetworks(ctx context.Context) ([]Network, error) {
	url := fmt.Sprintf("%s/networks", c.baseURL)
	
	req, err := http.NewRequestWithContext(ctx, "GET", url, nil)
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	c.setHeaders(req)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to execute request: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("request failed with status: %d", resp.StatusCode)
	}

	var result struct {
		Data []Network `json:"data"`
	}

	if err := json.NewDecoder(resp.Body).Decode(&result); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	return result.Data, nil
}

// GetLoadBalancers retrieves load balancers
func (c *Client) GetLoadBalancers(ctx context.Context) ([]LoadBalancer, error) {
	url := fmt.Sprintf("%s/loadbalancers", c.baseURL)
	
	req, err := http.NewRequestWithContext(ctx, "GET", url, nil)
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	c.setHeaders(req)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to execute request: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("request failed with status: %d", resp.StatusCode)
	}

	var result struct {
		Data []LoadBalancer `json:"data"`
	}

	if err := json.NewDecoder(resp.Body).Decode(&result); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	return result.Data, nil
}

// GetSecurityGroups retrieves security groups
func (c *Client) GetSecurityGroups(ctx context.Context) ([]SecurityGroup, error) {
	url := fmt.Sprintf("%s/securitygroups", c.baseURL)
	
	req, err := http.NewRequestWithContext(ctx, "GET", url, nil)
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	c.setHeaders(req)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to execute request: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("request failed with status: %d", resp.StatusCode)
	}

	var result struct {
		Data []SecurityGroup `json:"data"`
	}

	if err := json.NewDecoder(resp.Body).Decode(&result); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	return result.Data, nil
}

// CreateVirtualMachine creates a virtual machine
func (c *Client) CreateVirtualMachine(ctx context.Context, vm VirtualMachine) (*VirtualMachine, error) {
	url := fmt.Sprintf("%s/vms", c.baseURL)
	
	body, err := json.Marshal(vm)
	if err != nil {
		return nil, fmt.Errorf("failed to marshal request: %w", err)
	}

	req, err := http.NewRequestWithContext(ctx, "POST", url, bytes.NewBuffer(body))
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	c.setHeaders(req)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to execute request: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusCreated {
		return nil, fmt.Errorf("request failed with status: %d", resp.StatusCode)
	}

	var result VirtualMachine
	if err := json.NewDecoder(resp.Body).Decode(&result); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	return &result, nil
}

// UpdateVirtualMachine updates a virtual machine
func (c *Client) UpdateVirtualMachine(ctx context.Context, id string, vm VirtualMachine) (*VirtualMachine, error) {
	url := fmt.Sprintf("%s/vms/%s", c.baseURL, id)
	
	body, err := json.Marshal(vm)
	if err != nil {
		return nil, fmt.Errorf("failed to marshal request: %w", err)
	}

	req, err := http.NewRequestWithContext(ctx, "PUT", url, bytes.NewBuffer(body))
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	c.setHeaders(req)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to execute request: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("request failed with status: %d", resp.StatusCode)
	}

	var result VirtualMachine
	if err := json.NewDecoder(resp.Body).Decode(&result); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	return &result, nil
}

// DeleteVirtualMachine deletes a virtual machine
func (c *Client) DeleteVirtualMachine(ctx context.Context, id string) error {
	url := fmt.Sprintf("%s/vms/%s", c.baseURL, id)
	
	req, err := http.NewRequestWithContext(ctx, "DELETE", url, nil)
	if err != nil {
		return fmt.Errorf("failed to create request: %w", err)
	}

	c.setHeaders(req)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return fmt.Errorf("failed to execute request: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusNoContent {
		return fmt.Errorf("request failed with status: %d", resp.StatusCode)
	}

	return nil
}

// GetResourceMetrics retrieves resource metrics
func (c *Client) GetResourceMetrics(ctx context.Context, resourceType, resourceID string) (map[string]interface{}, error) {
	url := fmt.Sprintf("%s/resources/%s/%s/metrics", c.baseURL, resourceType, resourceID)
	
	req, err := http.NewRequestWithContext(ctx, "GET", url, nil)
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	c.setHeaders(req)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to execute request: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("request failed with status: %d", resp.StatusCode)
	}

	var result map[string]interface{}
	if err := json.NewDecoder(resp.Body).Decode(&result); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	return result, nil
}

// GetTopology retrieves network topology
func (c *Client) GetTopology(ctx context.Context) (map[string]interface{}, error) {
	url := fmt.Sprintf("%s/topology", c.baseURL)
	
	req, err := http.NewRequestWithContext(ctx, "GET", url, nil)
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	c.setHeaders(req)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to execute request: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("request failed with status: %d", resp.StatusCode)
	}

	var result map[string]interface{}
	if err := json.NewDecoder(resp.Body).Decode(&result); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	return result, nil
}

// setHeaders sets common headers for requests
func (c *Client) setHeaders(req *http.Request) {
	req.Header.Set("Content-Type", "application/json")
	req.Header.Set("Accept", "application/json")
	req.Header.Set("User-Agent", "router-sim/1.0.0")
	
	// Add custom headers from config
	for key, value := range c.config.Headers {
		req.Header.Set(key, value)
	}
}
