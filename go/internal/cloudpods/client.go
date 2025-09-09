package cloudpods

import (
	"bytes"
	"context"
	"encoding/json"
	"fmt"
	"io"
	"net/http"
	"time"
)

// Client represents a CloudPods API client
type Client struct {
	baseURL    string
	httpClient *http.Client
	username   string
	password   string
	token      string
}

// NewClient creates a new CloudPods client
func NewClient(baseURL, username, password string) *Client {
	return &Client{
		baseURL: baseURL,
		httpClient: &http.Client{
			Timeout: 30 * time.Second,
		},
		username: username,
		password: password,
	}
}

// Authenticate authenticates with CloudPods and retrieves a token
func (c *Client) Authenticate(ctx context.Context) error {
	authReq := map[string]string{
		"username": c.username,
		"password": c.password,
	}

	jsonData, err := json.Marshal(authReq)
	if err != nil {
		return fmt.Errorf("failed to marshal auth request: %w", err)
	}

	req, err := http.NewRequestWithContext(ctx, "POST", c.baseURL+"/api/v1/auth", bytes.NewBuffer(jsonData))
	if err != nil {
		return fmt.Errorf("failed to create auth request: %w", err)
	}

	req.Header.Set("Content-Type", "application/json")

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return fmt.Errorf("failed to authenticate: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return fmt.Errorf("authentication failed with status: %d", resp.StatusCode)
	}

	var authResp struct {
		Token string `json:"token"`
	}

	if err := json.NewDecoder(resp.Body).Decode(&authResp); err != nil {
		return fmt.Errorf("failed to decode auth response: %w", err)
	}

	c.token = authResp.Token
	return nil
}

// GetVMs retrieves all VMs from CloudPods
func (c *Client) GetVMs(ctx context.Context) ([]VM, error) {
	req, err := c.newAuthenticatedRequest(ctx, "GET", "/api/v1/vms", nil)
	if err != nil {
		return nil, err
	}

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to get VMs: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("failed to get VMs with status: %d", resp.StatusCode)
	}

	var vmsResp struct {
		VMs []VM `json:"vms"`
	}

	if err := json.NewDecoder(resp.Body).Decode(&vmsResp); err != nil {
		return nil, fmt.Errorf("failed to decode VMs response: %w", err)
	}

	return vmsResp.VMs, nil
}

// GetVM retrieves a specific VM by ID
func (c *Client) GetVM(ctx context.Context, vmID string) (*VM, error) {
	req, err := c.newAuthenticatedRequest(ctx, "GET", "/api/v1/vms/"+vmID, nil)
	if err != nil {
		return nil, err
	}

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to get VM: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("failed to get VM with status: %d", resp.StatusCode)
	}

	var vm VM
	if err := json.NewDecoder(resp.Body).Decode(&vm); err != nil {
		return nil, fmt.Errorf("failed to decode VM response: %w", err)
	}

	return &vm, nil
}

// GetNetworks retrieves all networks from CloudPods
func (c *Client) GetNetworks(ctx context.Context) ([]Network, error) {
	req, err := c.newAuthenticatedRequest(ctx, "GET", "/api/v1/networks", nil)
	if err != nil {
		return nil, err
	}

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to get networks: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("failed to get networks with status: %d", resp.StatusCode)
	}

	var networksResp struct {
		Networks []Network `json:"networks"`
	}

	if err := json.NewDecoder(resp.Body).Decode(&networksResp); err != nil {
		return nil, fmt.Errorf("failed to decode networks response: %w", err)
	}

	return networksResp.Networks, nil
}

// GetNetwork retrieves a specific network by ID
func (c *Client) GetNetwork(ctx context.Context, networkID string) (*Network, error) {
	req, err := c.newAuthenticatedRequest(ctx, "GET", "/api/v1/networks/"+networkID, nil)
	if err != nil {
		return nil, err
	}

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to get network: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("failed to get network with status: %d", resp.StatusCode)
	}

	var network Network
	if err := json.NewDecoder(resp.Body).Decode(&network); err != nil {
		return nil, fmt.Errorf("failed to decode network response: %w", err)
	}

	return &network, nil
}

// CreateVM creates a new VM in CloudPods
func (c *Client) CreateVM(ctx context.Context, vmReq CreateVMRequest) (*VM, error) {
	jsonData, err := json.Marshal(vmReq)
	if err != nil {
		return nil, fmt.Errorf("failed to marshal VM request: %w", err)
	}

	req, err := c.newAuthenticatedRequest(ctx, "POST", "/api/v1/vms", bytes.NewBuffer(jsonData))
	if err != nil {
		return nil, err
	}

	req.Header.Set("Content-Type", "application/json")

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to create VM: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusCreated {
		return nil, fmt.Errorf("failed to create VM with status: %d", resp.StatusCode)
	}

	var vm VM
	if err := json.NewDecoder(resp.Body).Decode(&vm); err != nil {
		return nil, fmt.Errorf("failed to decode VM response: %w", err)
	}

	return &vm, nil
}

// DeleteVM deletes a VM from CloudPods
func (c *Client) DeleteVM(ctx context.Context, vmID string) error {
	req, err := c.newAuthenticatedRequest(ctx, "DELETE", "/api/v1/vms/"+vmID, nil)
	if err != nil {
		return err
	}

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return fmt.Errorf("failed to delete VM: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusNoContent {
		return fmt.Errorf("failed to delete VM with status: %d", resp.StatusCode)
	}

	return nil
}

// GetVMStatus retrieves the status of a VM
func (c *Client) GetVMStatus(ctx context.Context, vmID string) (*VMStatus, error) {
	req, err := c.newAuthenticatedRequest(ctx, "GET", "/api/v1/vms/"+vmID+"/status", nil)
	if err != nil {
		return nil, err
	}

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to get VM status: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("failed to get VM status with status: %d", resp.StatusCode)
	}

	var status VMStatus
	if err := json.NewDecoder(resp.Body).Decode(&status); err != nil {
		return nil, fmt.Errorf("failed to decode VM status response: %w", err)
	}

	return &status, nil
}

// GetMetrics retrieves metrics for a specific resource
func (c *Client) GetMetrics(ctx context.Context, resourceType, resourceID string) (*Metrics, error) {
	req, err := c.newAuthenticatedRequest(ctx, "GET", 
		fmt.Sprintf("/api/v1/metrics/%s/%s", resourceType, resourceID), nil)
	if err != nil {
		return nil, err
	}

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to get metrics: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("failed to get metrics with status: %d", resp.StatusCode)
	}

	var metrics Metrics
	if err := json.NewDecoder(resp.Body).Decode(&metrics); err != nil {
		return nil, fmt.Errorf("failed to decode metrics response: %w", err)
	}

	return &metrics, nil
}

// newAuthenticatedRequest creates a new HTTP request with authentication headers
func (c *Client) newAuthenticatedRequest(ctx context.Context, method, path string, body io.Reader) (*http.Request, error) {
	req, err := http.NewRequestWithContext(ctx, method, c.baseURL+path, body)
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	if c.token != "" {
		req.Header.Set("Authorization", "Bearer "+c.token)
	}

	return req, nil
}

// VM represents a CloudPods virtual machine
type VM struct {
	ID          string            `json:"id"`
	Name        string            `json:"name"`
	Status      string            `json:"status"`
	CPU         int               `json:"cpu"`
	Memory      int64             `json:"memory"`
	Disk        int64             `json:"disk"`
	IPAddress   string            `json:"ip_address"`
	NetworkID   string            `json:"network_id"`
	CreatedAt   time.Time         `json:"created_at"`
	UpdatedAt   time.Time         `json:"updated_at"`
	Tags        map[string]string `json:"tags"`
}

// Network represents a CloudPods network
type Network struct {
	ID          string    `json:"id"`
	Name        string    `json:"name"`
	CIDR        string    `json:"cidr"`
	Gateway     string    `json:"gateway"`
	VLAN        int       `json:"vlan"`
	CreatedAt   time.Time `json:"created_at"`
	UpdatedAt   time.Time `json:"updated_at"`
}

// CreateVMRequest represents a request to create a VM
type CreateVMRequest struct {
	Name      string `json:"name"`
	CPU       int    `json:"cpu"`
	Memory    int64  `json:"memory"`
	Disk      int64  `json:"disk"`
	NetworkID string `json:"network_id"`
	ImageID   string `json:"image_id"`
}

// VMStatus represents the status of a VM
type VMStatus struct {
	ID       string `json:"id"`
	Status   string `json:"status"`
	CPUUsage float64 `json:"cpu_usage"`
	MemUsage float64 `json:"mem_usage"`
	DiskUsage float64 `json:"disk_usage"`
	Uptime   int64  `json:"uptime"`
}

// Metrics represents resource metrics
type Metrics struct {
	ResourceID   string                 `json:"resource_id"`
	ResourceType string                 `json:"resource_type"`
	Timestamp    time.Time              `json:"timestamp"`
	CPU          float64                `json:"cpu"`
	Memory       float64                `json:"memory"`
	Disk         float64                `json:"disk"`
	Network      NetworkMetrics         `json:"network"`
	Custom       map[string]interface{} `json:"custom"`
}

// NetworkMetrics represents network-specific metrics
type NetworkMetrics struct {
	BytesIn    int64 `json:"bytes_in"`
	BytesOut   int64 `json:"bytes_out"`
	PacketsIn  int64 `json:"packets_in"`
	PacketsOut int64 `json:"packets_out"`
	Errors     int64 `json:"errors"`
}
