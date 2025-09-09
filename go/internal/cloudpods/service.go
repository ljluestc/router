package cloudpods

import (
	"bytes"
	"context"
	"encoding/json"
	"fmt"
	"io"
	"net/http"
	"time"

	"go.uber.org/zap"
	"router-sim/internal/config"
)

// Service handles CloudPods API interactions
type Service struct {
	config *config.CloudPods
	client *http.Client
	logger *zap.Logger
}

// Resource represents a CloudPods resource
type Resource struct {
	ID          string            `json:"id"`
	Name        string            `json:"name"`
	Type        string            `json:"type"`
	Status      string            `json:"status"`
	Region      string            `json:"region"`
	CreatedAt   time.Time         `json:"created_at"`
	UpdatedAt   time.Time         `json:"updated_at"`
	Tags        []string          `json:"tags"`
	Properties  map[string]interface{} `json:"properties"`
}

// Instance represents a CloudPods instance
type Instance struct {
	Resource
	CPU        int    `json:"cpu"`
	Memory     int    `json:"memory"`
	Disk       int    `json:"disk"`
	Image      string `json:"image"`
	VPC        string `json:"vpc"`
	Subnet     string `json:"subnet"`
	PublicIP   string `json:"public_ip"`
	PrivateIP  string `json:"private_ip"`
}

// Network represents a CloudPods network
type Network struct {
	Resource
	CIDR       string `json:"cidr"`
	Gateway    string `json:"gateway"`
	DNS        []string `json:"dns"`
	VPC        string `json:"vpc"`
	Type       string `json:"type"`
}

// Storage represents a CloudPods storage
type Storage struct {
	Resource
	Size       int    `json:"size"`
	Type       string `json:"type"`
	Format     string `json:"format"`
	Instance   string `json:"instance"`
	MountPoint string `json:"mount_point"`
}

// LoadBalancer represents a CloudPods load balancer
type LoadBalancer struct {
	Resource
	Type       string   `json:"type"`
	Listeners  []Listener `json:"listeners"`
	Backends   []Backend  `json:"backends"`
	HealthCheck HealthCheck `json:"health_check"`
}

// Listener represents a load balancer listener
type Listener struct {
	Protocol string `json:"protocol"`
	Port     int    `json:"port"`
	Target   string `json:"target"`
}

// Backend represents a load balancer backend
type Backend struct {
	Instance string `json:"instance"`
	Port     int    `json:"port"`
	Weight   int    `json:"weight"`
}

// HealthCheck represents a load balancer health check
type HealthCheck struct {
	Protocol    string `json:"protocol"`
	Port        int    `json:"port"`
	Path        string `json:"path"`
	Interval    int    `json:"interval"`
	Timeout     int    `json:"timeout"`
	Threshold   int    `json:"threshold"`
}

// APIResponse represents a CloudPods API response
type APIResponse struct {
	Data    interface{} `json:"data"`
	Message string      `json:"message"`
	Code    int         `json:"code"`
	Success bool        `json:"success"`
}

// NewService creates a new CloudPods service
func NewService(cfg *config.CloudPods, logger *zap.Logger) (*Service, error) {
	client := &http.Client{
		Timeout: cfg.Timeout,
	}

	return &Service{
		config: cfg,
		client: client,
		logger: logger,
	}, nil
}

// makeRequest makes an HTTP request to CloudPods API
func (s *Service) makeRequest(ctx context.Context, method, endpoint string, body interface{}) (*http.Response, error) {
	url := s.config.Endpoint + endpoint

	var reqBody io.Reader
	if body != nil {
		jsonData, err := json.Marshal(body)
		if err != nil {
			return nil, fmt.Errorf("failed to marshal request body: %w", err)
		}
		reqBody = bytes.NewBuffer(jsonData)
	}

	req, err := http.NewRequestWithContext(ctx, method, url, reqBody)
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	// Set headers
	req.Header.Set("Content-Type", "application/json")
	req.Header.Set("Accept", "application/json")
	req.Header.Set("User-Agent", "router-sim/1.0")

	// Add custom headers
	for key, value := range s.config.Headers {
		req.Header.Set(key, value)
	}

	// Add authentication
	if s.config.Username != "" && s.config.Password != "" {
		req.SetBasicAuth(s.config.Username, s.config.Password)
	}

	s.logger.Debug("Making CloudPods API request",
		zap.String("method", method),
		zap.String("url", url),
	)

	resp, err := s.client.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to make request: %w", err)
	}

	return resp, nil
}

// ListResources lists all CloudPods resources
func (s *Service) ListResources(ctx context.Context) ([]Resource, error) {
	resp, err := s.makeRequest(ctx, "GET", "/api/v1/resources", nil)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("API request failed with status: %d", resp.StatusCode)
	}

	var apiResp APIResponse
	if err := json.NewDecoder(resp.Body).Decode(&apiResp); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	if !apiResp.Success {
		return nil, fmt.Errorf("API error: %s", apiResp.Message)
	}

	resources, ok := apiResp.Data.([]interface{})
	if !ok {
		return nil, fmt.Errorf("invalid response format")
	}

	var result []Resource
	for _, item := range resources {
		itemBytes, err := json.Marshal(item)
		if err != nil {
			continue
		}

		var resource Resource
		if err := json.Unmarshal(itemBytes, &resource); err != nil {
			continue
		}

		result = append(result, resource)
	}

	return result, nil
}

// CreateResource creates a new CloudPods resource
func (s *Service) CreateResource(ctx context.Context, resource *Resource) error {
	resp, err := s.makeRequest(ctx, "POST", "/api/v1/resources", resource)
	if err != nil {
		return err
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusCreated {
		return fmt.Errorf("failed to create resource, status: %d", resp.StatusCode)
	}

	return nil
}

// GetResource gets a CloudPods resource by ID
func (s *Service) GetResource(ctx context.Context, id string) (*Resource, error) {
	resp, err := s.makeRequest(ctx, "GET", "/api/v1/resources/"+id, nil)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("failed to get resource, status: %d", resp.StatusCode)
	}

	var apiResp APIResponse
	if err := json.NewDecoder(resp.Body).Decode(&apiResp); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	if !apiResp.Success {
		return nil, fmt.Errorf("API error: %s", apiResp.Message)
	}

	resourceBytes, err := json.Marshal(apiResp.Data)
	if err != nil {
		return nil, fmt.Errorf("failed to marshal resource data: %w", err)
	}

	var resource Resource
	if err := json.Unmarshal(resourceBytes, &resource); err != nil {
		return nil, fmt.Errorf("failed to unmarshal resource: %w", err)
	}

	return &resource, nil
}

// UpdateResource updates a CloudPods resource
func (s *Service) UpdateResource(ctx context.Context, id string, resource *Resource) error {
	resp, err := s.makeRequest(ctx, "PUT", "/api/v1/resources/"+id, resource)
	if err != nil {
		return err
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return fmt.Errorf("failed to update resource, status: %d", resp.StatusCode)
	}

	return nil
}

// DeleteResource deletes a CloudPods resource
func (s *Service) DeleteResource(ctx context.Context, id string) error {
	resp, err := s.makeRequest(ctx, "DELETE", "/api/v1/resources/"+id, nil)
	if err != nil {
		return err
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return fmt.Errorf("failed to delete resource, status: %d", resp.StatusCode)
	}

	return nil
}

// ListInstances lists all CloudPods instances
func (s *Service) ListInstances(ctx context.Context) ([]Instance, error) {
	resp, err := s.makeRequest(ctx, "GET", "/api/v1/instances", nil)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("API request failed with status: %d", resp.StatusCode)
	}

	var apiResp APIResponse
	if err := json.NewDecoder(resp.Body).Decode(&apiResp); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	if !apiResp.Success {
		return nil, fmt.Errorf("API error: %s", apiResp.Message)
	}

	instances, ok := apiResp.Data.([]interface{})
	if !ok {
		return nil, fmt.Errorf("invalid response format")
	}

	var result []Instance
	for _, item := range instances {
		itemBytes, err := json.Marshal(item)
		if err != nil {
			continue
		}

		var instance Instance
		if err := json.Unmarshal(itemBytes, &instance); err != nil {
			continue
		}

		result = append(result, instance)
	}

	return result, nil
}

// ListNetworks lists all CloudPods networks
func (s *Service) ListNetworks(ctx context.Context) ([]Network, error) {
	resp, err := s.makeRequest(ctx, "GET", "/api/v1/networks", nil)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("API request failed with status: %d", resp.StatusCode)
	}

	var apiResp APIResponse
	if err := json.NewDecoder(resp.Body).Decode(&apiResp); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	if !apiResp.Success {
		return nil, fmt.Errorf("API error: %s", apiResp.Message)
	}

	networks, ok := apiResp.Data.([]interface{})
	if !ok {
		return nil, fmt.Errorf("invalid response format")
	}

	var result []Network
	for _, item := range networks {
		itemBytes, err := json.Marshal(item)
		if err != nil {
			continue
		}

		var network Network
		if err := json.Unmarshal(itemBytes, &network); err != nil {
			continue
		}

		result = append(result, network)
	}

	return result, nil
}

// ListStorages lists all CloudPods storages
func (s *Service) ListStorages(ctx context.Context) ([]Storage, error) {
	resp, err := s.makeRequest(ctx, "GET", "/api/v1/storages", nil)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("API request failed with status: %d", resp.StatusCode)
	}

	var apiResp APIResponse
	if err := json.NewDecoder(resp.Body).Decode(&apiResp); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	if !apiResp.Success {
		return nil, fmt.Errorf("API error: %s", apiResp.Message)
	}

	storages, ok := apiResp.Data.([]interface{})
	if !ok {
		return nil, fmt.Errorf("invalid response format")
	}

	var result []Storage
	for _, item := range storages {
		itemBytes, err := json.Marshal(item)
		if err != nil {
			continue
		}

		var storage Storage
		if err := json.Unmarshal(itemBytes, &storage); err != nil {
			continue
		}

		result = append(result, storage)
	}

	return result, nil
}

// ListLoadBalancers lists all CloudPods load balancers
func (s *Service) ListLoadBalancers(ctx context.Context) ([]LoadBalancer, error) {
	resp, err := s.makeRequest(ctx, "GET", "/api/v1/loadbalancers", nil)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("API request failed with status: %d", resp.StatusCode)
	}

	var apiResp APIResponse
	if err := json.NewDecoder(resp.Body).Decode(&apiResp); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	if !apiResp.Success {
		return nil, fmt.Errorf("API error: %s", apiResp.Message)
	}

	loadBalancers, ok := apiResp.Data.([]interface{})
	if !ok {
		return nil, fmt.Errorf("invalid response format")
	}

	var result []LoadBalancer
	for _, item := range loadBalancers {
		itemBytes, err := json.Marshal(item)
		if err != nil {
			continue
		}

		var lb LoadBalancer
		if err := json.Unmarshal(itemBytes, &lb); err != nil {
			continue
		}

		result = append(result, lb)
	}

	return result, nil
}

