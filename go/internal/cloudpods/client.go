package cloudpods

import (
	"context"
	"encoding/json"
	"fmt"
	"io"
	"net/http"
	"time"
)

// CloudPodsClient represents a client for CloudPods API
type CloudPodsClient struct {
	baseURL    string
	apiKey     string
	httpClient *http.Client
}

// CloudPodsConfig represents CloudPods configuration
type CloudPodsConfig struct {
	BaseURL string `json:"base_url"`
	APIKey  string `json:"api_key"`
	Timeout int    `json:"timeout_seconds"`
}

// CloudPodsResource represents a CloudPods resource
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

// CloudPodsNetwork represents a CloudPods network
type CloudPodsNetwork struct {
	ID          string   `json:"id"`
	Name        string   `json:"name"`
	CIDR        string   `json:"cidr"`
	Gateway     string   `json:"gateway"`
	DNS         []string `json:"dns"`
	VLAN        int      `json:"vlan"`
	Status      string   `json:"status"`
	Description string   `json:"description"`
}

// CloudPodsVM represents a CloudPods virtual machine
type CloudPodsVM struct {
	ID          string            `json:"id"`
	Name        string            `json:"name"`
	Status      string            `json:"status"`
	CPU         int               `json:"cpu"`
	Memory      int64             `json:"memory"`
	Disk        int64             `json:"disk"`
	Image       string            `json:"image"`
	Network     string            `json:"network"`
	IPAddress   string            `json:"ip_address"`
	CreatedAt   time.Time         `json:"created_at"`
	UpdatedAt   time.Time         `json:"updated_at"`
	Properties  map[string]interface{} `json:"properties"`
}

// CloudPodsLoadBalancer represents a CloudPods load balancer
type CloudPodsLoadBalancer struct {
	ID          string   `json:"id"`
	Name        string   `json:"name"`
	Status      string   `json:"status"`
	Type        string   `json:"type"`
	Algorithm   string   `json:"algorithm"`
	Ports       []int    `json:"ports"`
	Backends    []string `json:"backends"`
	HealthCheck string   `json:"health_check"`
	CreatedAt   time.Time `json:"created_at"`
	UpdatedAt   time.Time `json:"updated_at"`
}

// CloudPodsAPIResponse represents a generic API response
type CloudPodsAPIResponse struct {
	Success bool                   `json:"success"`
	Message string                 `json:"message"`
	Data    interface{}            `json:"data"`
	Error   string                 `json:"error"`
	Meta    map[string]interface{} `json:"meta"`
}

// NewCloudPodsClient creates a new CloudPods client
func NewCloudPodsClient(config CloudPodsConfig) *CloudPodsClient {
	timeout := time.Duration(config.Timeout) * time.Second
	if timeout == 0 {
		timeout = 30 * time.Second
	}

	return &CloudPodsClient{
		baseURL: config.BaseURL,
		apiKey:  config.APIKey,
		httpClient: &http.Client{
			Timeout: timeout,
		},
	}
}

// GetNetworks retrieves all networks
func (c *CloudPodsClient) GetNetworks(ctx context.Context) ([]CloudPodsNetwork, error) {
	url := fmt.Sprintf("%s/api/v1/networks", c.baseURL)
	req, err := http.NewRequestWithContext(ctx, "GET", url, nil)
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	req.Header.Set("Authorization", "Bearer "+c.apiKey)
	req.Header.Set("Content-Type", "application/json")

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to execute request: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("API request failed with status: %d", resp.StatusCode)
	}

	body, err := io.ReadAll(resp.Body)
	if err != nil {
		return nil, fmt.Errorf("failed to read response body: %w", err)
	}

	var apiResp CloudPodsAPIResponse
	if err := json.Unmarshal(body, &apiResp); err != nil {
		return nil, fmt.Errorf("failed to unmarshal response: %w", err)
	}

	if !apiResp.Success {
		return nil, fmt.Errorf("API error: %s", apiResp.Error)
	}

	// Convert data to networks
	networksData, ok := apiResp.Data.([]interface{})
	if !ok {
		return nil, fmt.Errorf("invalid response data format")
	}

	var networks []CloudPodsNetwork
	for _, item := range networksData {
		itemBytes, err := json.Marshal(item)
		if err != nil {
			continue
		}

		var network CloudPodsNetwork
		if err := json.Unmarshal(itemBytes, &network); err != nil {
			continue
		}

		networks = append(networks, network)
	}

	return networks, nil
}

// GetNetwork retrieves a specific network by ID
func (c *CloudPodsClient) GetNetwork(ctx context.Context, networkID string) (*CloudPodsNetwork, error) {
	url := fmt.Sprintf("%s/api/v1/networks/%s", c.baseURL, networkID)
	req, err := http.NewRequestWithContext(ctx, "GET", url, nil)
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	req.Header.Set("Authorization", "Bearer "+c.apiKey)
	req.Header.Set("Content-Type", "application/json")

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to execute request: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("API request failed with status: %d", resp.StatusCode)
	}

	body, err := io.ReadAll(resp.Body)
	if err != nil {
		return nil, fmt.Errorf("failed to read response body: %w", err)
	}

	var apiResp CloudPodsAPIResponse
	if err := json.Unmarshal(body, &apiResp); err != nil {
		return nil, fmt.Errorf("failed to unmarshal response: %w", err)
	}

	if !apiResp.Success {
		return nil, fmt.Errorf("API error: %s", apiResp.Error)
	}

	// Convert data to network
	networkData, err := json.Marshal(apiResp.Data)
	if err != nil {
		return nil, fmt.Errorf("failed to marshal network data: %w", err)
	}

	var network CloudPodsNetwork
	if err := json.Unmarshal(networkData, &network); err != nil {
		return nil, fmt.Errorf("failed to unmarshal network: %w", err)
	}

	return &network, nil
}

// CreateNetwork creates a new network
func (c *CloudPodsClient) CreateNetwork(ctx context.Context, network CloudPodsNetwork) (*CloudPodsNetwork, error) {
	url := fmt.Sprintf("%s/api/v1/networks", c.baseURL)
	
	networkData, err := json.Marshal(network)
	if err != nil {
		return nil, fmt.Errorf("failed to marshal network: %w", err)
	}

	req, err := http.NewRequestWithContext(ctx, "POST", url, bytes.NewReader(networkData))
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	req.Header.Set("Authorization", "Bearer "+c.apiKey)
	req.Header.Set("Content-Type", "application/json")

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to execute request: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusCreated {
		return nil, fmt.Errorf("API request failed with status: %d", resp.StatusCode)
	}

	body, err := io.ReadAll(resp.Body)
	if err != nil {
		return nil, fmt.Errorf("failed to read response body: %w", err)
	}

	var apiResp CloudPodsAPIResponse
	if err := json.Unmarshal(body, &apiResp); err != nil {
		return nil, fmt.Errorf("failed to unmarshal response: %w", err)
	}

	if !apiResp.Success {
		return nil, fmt.Errorf("API error: %s", apiResp.Error)
	}

	// Convert data to network
	networkData, err := json.Marshal(apiResp.Data)
	if err != nil {
		return nil, fmt.Errorf("failed to marshal network data: %w", err)
	}

	var createdNetwork CloudPodsNetwork
	if err := json.Unmarshal(networkData, &createdNetwork); err != nil {
		return nil, fmt.Errorf("failed to unmarshal network: %w", err)
	}

	return &createdNetwork, nil
}

// GetVMs retrieves all virtual machines
func (c *CloudPodsClient) GetVMs(ctx context.Context) ([]CloudPodsVM, error) {
	url := fmt.Sprintf("%s/api/v1/vms", c.baseURL)
	req, err := http.NewRequestWithContext(ctx, "GET", url, nil)
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	req.Header.Set("Authorization", "Bearer "+c.apiKey)
	req.Header.Set("Content-Type", "application/json")

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to execute request: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("API request failed with status: %d", resp.StatusCode)
	}

	body, err := io.ReadAll(resp.Body)
	if err != nil {
		return nil, fmt.Errorf("failed to read response body: %w", err)
	}

	var apiResp CloudPodsAPIResponse
	if err := json.Unmarshal(body, &apiResp); err != nil {
		return nil, fmt.Errorf("failed to unmarshal response: %w", err)
	}

	if !apiResp.Success {
		return nil, fmt.Errorf("API error: %s", apiResp.Error)
	}

	// Convert data to VMs
	vmsData, ok := apiResp.Data.([]interface{})
	if !ok {
		return nil, fmt.Errorf("invalid response data format")
	}

	var vms []CloudPodsVM
	for _, item := range vmsData {
		itemBytes, err := json.Marshal(item)
		if err != nil {
			continue
		}

		var vm CloudPodsVM
		if err := json.Unmarshal(itemBytes, &vm); err != nil {
			continue
		}

		vms = append(vms, vm)
	}

	return vms, nil
}

// GetLoadBalancers retrieves all load balancers
func (c *CloudPodsClient) GetLoadBalancers(ctx context.Context) ([]CloudPodsLoadBalancer, error) {
	url := fmt.Sprintf("%s/api/v1/loadbalancers", c.baseURL)
	req, err := http.NewRequestWithContext(ctx, "GET", url, nil)
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	req.Header.Set("Authorization", "Bearer "+c.apiKey)
	req.Header.Set("Content-Type", "application/json")

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to execute request: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("API request failed with status: %d", resp.StatusCode)
	}

	body, err := io.ReadAll(resp.Body)
	if err != nil {
		return nil, fmt.Errorf("failed to read response body: %w", err)
	}

	var apiResp CloudPodsAPIResponse
	if err := json.Unmarshal(body, &apiResp); err != nil {
		return nil, fmt.Errorf("failed to unmarshal response: %w", err)
	}

	if !apiResp.Success {
		return nil, fmt.Errorf("API error: %s", apiResp.Error)
	}

	// Convert data to load balancers
	lbData, ok := apiResp.Data.([]interface{})
	if !ok {
		return nil, fmt.Errorf("invalid response data format")
	}

	var loadBalancers []CloudPodsLoadBalancer
	for _, item := range lbData {
		itemBytes, err := json.Marshal(item)
		if err != nil {
			continue
		}

		var lb CloudPodsLoadBalancer
		if err := json.Unmarshal(itemBytes, &lb); err != nil {
			continue
		}

		loadBalancers = append(loadBalancers, lb)
	}

	return loadBalancers, nil
}

// GetResources retrieves all resources of a specific type
func (c *CloudPodsClient) GetResources(ctx context.Context, resourceType string) ([]CloudPodsResource, error) {
	url := fmt.Sprintf("%s/api/v1/resources/%s", c.baseURL, resourceType)
	req, err := http.NewRequestWithContext(ctx, "GET", url, nil)
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	req.Header.Set("Authorization", "Bearer "+c.apiKey)
	req.Header.Set("Content-Type", "application/json")

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to execute request: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("API request failed with status: %d", resp.StatusCode)
	}

	body, err := io.ReadAll(resp.Body)
	if err != nil {
		return nil, fmt.Errorf("failed to read response body: %w", err)
	}

	var apiResp CloudPodsAPIResponse
	if err := json.Unmarshal(body, &apiResp); err != nil {
		return nil, fmt.Errorf("failed to unmarshal response: %w", err)
	}

	if !apiResp.Success {
		return nil, fmt.Errorf("API error: %s", apiResp.Error)
	}

	// Convert data to resources
	resourcesData, ok := apiResp.Data.([]interface{})
	if !ok {
		return nil, fmt.Errorf("invalid response data format")
	}

	var resources []CloudPodsResource
	for _, item := range resourcesData {
		itemBytes, err := json.Marshal(item)
		if err != nil {
			continue
		}

		var resource CloudPodsResource
		if err := json.Unmarshal(itemBytes, &resource); err != nil {
			continue
		}

		resources = append(resources, resource)
	}

	return resources, nil
}

// HealthCheck performs a health check on the CloudPods API
func (c *CloudPodsClient) HealthCheck(ctx context.Context) error {
	url := fmt.Sprintf("%s/api/v1/health", c.baseURL)
	req, err := http.NewRequestWithContext(ctx, "GET", url, nil)
	if err != nil {
		return fmt.Errorf("failed to create request: %w", err)
	}

	req.Header.Set("Authorization", "Bearer "+c.apiKey)
	req.Header.Set("Content-Type", "application/json")

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return fmt.Errorf("failed to execute request: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return fmt.Errorf("health check failed with status: %d", resp.StatusCode)
	}

	return nil
}
