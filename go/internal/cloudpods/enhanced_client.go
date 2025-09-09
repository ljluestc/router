package cloudpods

import (
	"context"
	"encoding/json"
	"fmt"
	"io"
	"net/http"
	"time"

	"router-sim/internal/config"
)

// Enhanced CloudPods client with comprehensive resource management
type EnhancedClient struct {
	config     *config.CloudPodsConfig
	httpClient *http.Client
	baseURL    string
	authToken  string
}

// CloudPods resource types
type Resource struct {
	ID          string            `json:"id"`
	Name        string            `json:"name"`
	Type        string            `json:"type"`
	Status      string            `json:"status"`
	Region      string            `json:"region"`
	Zone        string            `json:"zone"`
	CreatedAt   time.Time         `json:"created_at"`
	UpdatedAt   time.Time         `json:"updated_at"`
	Tags        map[string]string `json:"tags"`
	Properties  map[string]interface{} `json:"properties"`
	Owner       string            `json:"owner"`
	Project     string            `json:"project"`
}

type Instance struct {
	Resource
	ImageID       string            `json:"image_id"`
	InstanceType  string            `json:"instance_type"`
	CPU           int               `json:"cpu"`
	Memory        int               `json:"memory"`
	Disk          int               `json:"disk"`
	PublicIP      string            `json:"public_ip"`
	PrivateIP     string            `json:"private_ip"`
	VPC           string            `json:"vpc"`
	Subnet        string            `json:"subnet"`
	SecurityGroup string            `json:"security_group"`
	KeyPair       string            `json:"key_pair"`
	UserData      string            `json:"user_data"`
	Metadata      map[string]string `json:"metadata"`
}

type Network struct {
	Resource
	CIDR          string            `json:"cidr"`
	Gateway       string            `json:"gateway"`
	DNS           []string          `json:"dns"`
	DHCP          bool              `json:"dhcp"`
	VLAN          int               `json:"vlan"`
	VXLAN         int               `json:"vxlan"`
	Subnets       []Subnet          `json:"subnets"`
	Routes        []Route           `json:"routes"`
	ACLs          []ACL             `json:"acls"`
}

type Subnet struct {
	ID          string `json:"id"`
	Name        string `json:"name"`
	CIDR        string `json:"cidr"`
	Gateway     string `json:"gateway"`
	VLAN        int    `json:"vlan"`
	VXLAN       int    `json:"vxlan"`
	DHCP        bool   `json:"dhcp"`
	DHCPStart   string `json:"dhcp_start"`
	DHCPEnd     string `json:"dhcp_end"`
	ReservedIPs []string `json:"reserved_ips"`
}

type Route struct {
	ID          string `json:"id"`
	Destination string `json:"destination"`
	NextHop     string `json:"next_hop"`
	Interface   string `json:"interface"`
	Metric      int    `json:"metric"`
	Type        string `json:"type"`
}

type ACL struct {
	ID          string `json:"id"`
	Name        string `json:"name"`
	Direction   string `json:"direction"`
	Protocol    string `json:"protocol"`
	Source      string `json:"source"`
	Destination string `json:"destination"`
	Port        int    `json:"port"`
	Action      string `json:"action"`
	Priority    int    `json:"priority"`
}

type LoadBalancer struct {
	Resource
	Type        string            `json:"type"`
	Algorithm   string            `json:"algorithm"`
	HealthCheck HealthCheck       `json:"health_check"`
	Listeners   []Listener        `json:"listeners"`
	Backends    []Backend         `json:"backend"`
	Sticky      bool              `json:"sticky"`
	SSL         SSLConfig         `json:"ssl"`
}

type HealthCheck struct {
	Protocol    string `json:"protocol"`
	Port        int    `json:"port"`
	Path        string `json:"path"`
	Interval    int    `json:"interval"`
	Timeout     int    `json:"timeout"`
	Retries     int    `json:"retries"`
	Healthy     int    `json:"healthy"`
	Unhealthy   int    `json:"unhealthy"`
}

type Listener struct {
	ID       string `json:"id"`
	Protocol string `json:"protocol"`
	Port     int    `json:"port"`
	Target   string `json:"target"`
}

type Backend struct {
	ID       string `json:"id"`
	Instance string `json:"instance"`
	Port     int    `json:"port"`
	Weight   int    `json:"weight"`
	Health   string `json:"health"`
}

type SSLConfig struct {
	Certificate string `json:"certificate"`
	PrivateKey  string `json:"private_key"`
	Protocols   []string `json:"protocols"`
	Ciphers     []string `json:"ciphers"`
}

type Storage struct {
	Resource
	Type        string            `json:"type"`
	Size        int               `json:"size"`
	Format      string            `json:"format"`
	MountPoint  string            `json:"mount_point"`
	Encrypted   bool              `json:"encrypted"`
	Snapshots   []Snapshot        `json:"snapshots"`
	Attachments []Attachment      `json:"attachments"`
}

type Snapshot struct {
	ID          string    `json:"id"`
	Name        string    `json:"name"`
	Size        int       `json:"size"`
	CreatedAt   time.Time `json:"created_at"`
	Description string    `json:"description"`
}

type Attachment struct {
	ID       string `json:"id"`
	Instance string `json:"instance"`
	Device   string `json:"device"`
	Status   string `json:"status"`
}

// API Response structures
type ListResponse struct {
	Resources []Resource `json:"resources"`
	Total     int        `json:"total"`
	Page      int        `json:"page"`
	PageSize  int        `json:"page_size"`
	NextPage  string     `json:"next_page"`
}

type InstanceListResponse struct {
	Instances []Instance `json:"instances"`
	Total     int        `json:"total"`
	Page      int        `json:"page"`
	PageSize  int        `json:"page_size"`
}

type NetworkListResponse struct {
	Networks []Network `json:"networks"`
	Total    int       `json:"total"`
	Page     int       `json:"page"`
	PageSize int       `json:"page_size"`
}

type LoadBalancerListResponse struct {
	LoadBalancers []LoadBalancer `json:"load_balancers"`
	Total         int            `json:"total"`
	Page          int            `json:"page"`
	PageSize      int            `json:"page_size"`
}

type StorageListResponse struct {
	Storages []Storage `json:"storages"`
	Total    int       `json:"total"`
	Page     int       `json:"page"`
	PageSize int       `json:"page_size"`
}

// Statistics and metrics
type CloudPodsStats struct {
	TotalInstances     int `json:"total_instances"`
	RunningInstances   int `json:"running_instances"`
	StoppedInstances   int `json:"stopped_instances"`
	TotalNetworks      int `json:"total_networks"`
	ActiveNetworks     int `json:"active_networks"`
	TotalLoadBalancers int `json:"total_load_balancers"`
	ActiveLoadBalancers int `json:"active_load_balancers"`
	TotalStorages      int `json:"total_storages"`
	UsedStorage        int `json:"used_storage"`
	AvailableStorage   int `json:"available_storage"`
}

// NewEnhancedClient creates a new enhanced CloudPods client
func NewEnhancedClient(config *config.CloudPodsConfig) *EnhancedClient {
	return &EnhancedClient{
		config: config,
		httpClient: &http.Client{
			Timeout: config.Timeout,
		},
		baseURL:   config.BaseURL,
		authToken: config.AuthToken,
	}
}

// Authentication
func (c *EnhancedClient) Authenticate(ctx context.Context) error {
	req, err := http.NewRequestWithContext(ctx, "POST", c.baseURL+"/auth/login", nil)
	if err != nil {
		return fmt.Errorf("failed to create auth request: %w", err)
	}

	req.Header.Set("Content-Type", "application/json")
	req.Header.Set("Authorization", "Bearer "+c.authToken)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return fmt.Errorf("failed to authenticate: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return fmt.Errorf("authentication failed with status: %d", resp.StatusCode)
	}

	return nil
}

// Instance management
func (c *EnhancedClient) ListInstances(ctx context.Context, filters map[string]string) ([]Instance, error) {
	req, err := http.NewRequestWithContext(ctx, "GET", c.baseURL+"/instances", nil)
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	// Add query parameters
	q := req.URL.Query()
	for key, value := range filters {
		q.Add(key, value)
	}
	req.URL.RawQuery = q.Encode()

	req.Header.Set("Authorization", "Bearer "+c.authToken)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to list instances: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("failed to list instances with status: %d", resp.StatusCode)
	}

	var response InstanceListResponse
	if err := json.NewDecoder(resp.Body).Decode(&response); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	return response.Instances, nil
}

func (c *EnhancedClient) GetInstance(ctx context.Context, id string) (*Instance, error) {
	req, err := http.NewRequestWithContext(ctx, "GET", c.baseURL+"/instances/"+id, nil)
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	req.Header.Set("Authorization", "Bearer "+c.authToken)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to get instance: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("failed to get instance with status: %d", resp.StatusCode)
	}

	var instance Instance
	if err := json.NewDecoder(resp.Body).Decode(&instance); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	return &instance, nil
}

func (c *EnhancedClient) CreateInstance(ctx context.Context, instance *Instance) (*Instance, error) {
	jsonData, err := json.Marshal(instance)
	if err != nil {
		return nil, fmt.Errorf("failed to marshal instance: %w", err)
	}

	req, err := http.NewRequestWithContext(ctx, "POST", c.baseURL+"/instances", 
		bytes.NewBuffer(jsonData))
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	req.Header.Set("Content-Type", "application/json")
	req.Header.Set("Authorization", "Bearer "+c.authToken)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to create instance: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusCreated {
		return nil, fmt.Errorf("failed to create instance with status: %d", resp.StatusCode)
	}

	var createdInstance Instance
	if err := json.NewDecoder(resp.Body).Decode(&createdInstance); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	return &createdInstance, nil
}

func (c *EnhancedClient) UpdateInstance(ctx context.Context, id string, instance *Instance) (*Instance, error) {
	jsonData, err := json.Marshal(instance)
	if err != nil {
		return nil, fmt.Errorf("failed to marshal instance: %w", err)
	}

	req, err := http.NewRequestWithContext(ctx, "PUT", c.baseURL+"/instances/"+id, 
		bytes.NewBuffer(jsonData))
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	req.Header.Set("Content-Type", "application/json")
	req.Header.Set("Authorization", "Bearer "+c.authToken)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to update instance: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("failed to update instance with status: %d", resp.StatusCode)
	}

	var updatedInstance Instance
	if err := json.NewDecoder(resp.Body).Decode(&updatedInstance); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	return &updatedInstance, nil
}

func (c *EnhancedClient) DeleteInstance(ctx context.Context, id string) error {
	req, err := http.NewRequestWithContext(ctx, "DELETE", c.baseURL+"/instances/"+id, nil)
	if err != nil {
		return fmt.Errorf("failed to create request: %w", err)
	}

	req.Header.Set("Authorization", "Bearer "+c.authToken)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return fmt.Errorf("failed to delete instance: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusNoContent {
		return fmt.Errorf("failed to delete instance with status: %d", resp.StatusCode)
	}

	return nil
}

func (c *EnhancedClient) StartInstance(ctx context.Context, id string) error {
	return c.instanceAction(ctx, id, "start")
}

func (c *EnhancedClient) StopInstance(ctx context.Context, id string) error {
	return c.instanceAction(ctx, id, "stop")
}

func (c *EnhancedClient) RestartInstance(ctx context.Context, id string) error {
	return c.instanceAction(ctx, id, "restart")
}

func (c *EnhancedClient) instanceAction(ctx context.Context, id, action string) error {
	req, err := http.NewRequestWithContext(ctx, "POST", 
		c.baseURL+"/instances/"+id+"/"+action, nil)
	if err != nil {
		return fmt.Errorf("failed to create request: %w", err)
	}

	req.Header.Set("Authorization", "Bearer "+c.authToken)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return fmt.Errorf("failed to %s instance: %w", action, err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return fmt.Errorf("failed to %s instance with status: %d", action, resp.StatusCode)
	}

	return nil
}

// Network management
func (c *EnhancedClient) ListNetworks(ctx context.Context, filters map[string]string) ([]Network, error) {
	req, err := http.NewRequestWithContext(ctx, "GET", c.baseURL+"/networks", nil)
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	q := req.URL.Query()
	for key, value := range filters {
		q.Add(key, value)
	}
	req.URL.RawQuery = q.Encode()

	req.Header.Set("Authorization", "Bearer "+c.authToken)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to list networks: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("failed to list networks with status: %d", resp.StatusCode)
	}

	var response NetworkListResponse
	if err := json.NewDecoder(resp.Body).Decode(&response); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	return response.Networks, nil
}

func (c *EnhancedClient) GetNetwork(ctx context.Context, id string) (*Network, error) {
	req, err := http.NewRequestWithContext(ctx, "GET", c.baseURL+"/networks/"+id, nil)
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	req.Header.Set("Authorization", "Bearer "+c.authToken)

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
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	return &network, nil
}

func (c *EnhancedClient) CreateNetwork(ctx context.Context, network *Network) (*Network, error) {
	jsonData, err := json.Marshal(network)
	if err != nil {
		return nil, fmt.Errorf("failed to marshal network: %w", err)
	}

	req, err := http.NewRequestWithContext(ctx, "POST", c.baseURL+"/networks", 
		bytes.NewBuffer(jsonData))
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	req.Header.Set("Content-Type", "application/json")
	req.Header.Set("Authorization", "Bearer "+c.authToken)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to create network: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusCreated {
		return nil, fmt.Errorf("failed to create network with status: %d", resp.StatusCode)
	}

	var createdNetwork Network
	if err := json.NewDecoder(resp.Body).Decode(&createdNetwork); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	return &createdNetwork, nil
}

func (c *EnhancedClient) DeleteNetwork(ctx context.Context, id string) error {
	req, err := http.NewRequestWithContext(ctx, "DELETE", c.baseURL+"/networks/"+id, nil)
	if err != nil {
		return fmt.Errorf("failed to create request: %w", err)
	}

	req.Header.Set("Authorization", "Bearer "+c.authToken)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return fmt.Errorf("failed to delete network: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusNoContent {
		return fmt.Errorf("failed to delete network with status: %d", resp.StatusCode)
	}

	return nil
}

// Load balancer management
func (c *EnhancedClient) ListLoadBalancers(ctx context.Context, filters map[string]string) ([]LoadBalancer, error) {
	req, err := http.NewRequestWithContext(ctx, "GET", c.baseURL+"/load-balancers", nil)
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	q := req.URL.Query()
	for key, value := range filters {
		q.Add(key, value)
	}
	req.URL.RawQuery = q.Encode()

	req.Header.Set("Authorization", "Bearer "+c.authToken)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to list load balancers: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("failed to list load balancers with status: %d", resp.StatusCode)
	}

	var response LoadBalancerListResponse
	if err := json.NewDecoder(resp.Body).Decode(&response); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	return response.LoadBalancers, nil
}

func (c *EnhancedClient) GetLoadBalancer(ctx context.Context, id string) (*LoadBalancer, error) {
	req, err := http.NewRequestWithContext(ctx, "GET", c.baseURL+"/load-balancers/"+id, nil)
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	req.Header.Set("Authorization", "Bearer "+c.authToken)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to get load balancer: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("failed to get load balancer with status: %d", resp.StatusCode)
	}

	var lb LoadBalancer
	if err := json.NewDecoder(resp.Body).Decode(&lb); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	return &lb, nil
}

func (c *EnhancedClient) CreateLoadBalancer(ctx context.Context, lb *LoadBalancer) (*LoadBalancer, error) {
	jsonData, err := json.Marshal(lb)
	if err != nil {
		return nil, fmt.Errorf("failed to marshal load balancer: %w", err)
	}

	req, err := http.NewRequestWithContext(ctx, "POST", c.baseURL+"/load-balancers", 
		bytes.NewBuffer(jsonData))
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	req.Header.Set("Content-Type", "application/json")
	req.Header.Set("Authorization", "Bearer "+c.authToken)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to create load balancer: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusCreated {
		return nil, fmt.Errorf("failed to create load balancer with status: %d", resp.StatusCode)
	}

	var createdLB LoadBalancer
	if err := json.NewDecoder(resp.Body).Decode(&createdLB); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	return &createdLB, nil
}

func (c *EnhancedClient) DeleteLoadBalancer(ctx context.Context, id string) error {
	req, err := http.NewRequestWithContext(ctx, "DELETE", c.baseURL+"/load-balancers/"+id, nil)
	if err != nil {
		return fmt.Errorf("failed to create request: %w", err)
	}

	req.Header.Set("Authorization", "Bearer "+c.authToken)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return fmt.Errorf("failed to delete load balancer: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusNoContent {
		return fmt.Errorf("failed to delete load balancer with status: %d", resp.StatusCode)
	}

	return nil
}

// Storage management
func (c *EnhancedClient) ListStorages(ctx context.Context, filters map[string]string) ([]Storage, error) {
	req, err := http.NewRequestWithContext(ctx, "GET", c.baseURL+"/storages", nil)
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	q := req.URL.Query()
	for key, value := range filters {
		q.Add(key, value)
	}
	req.URL.RawQuery = q.Encode()

	req.Header.Set("Authorization", "Bearer "+c.authToken)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to list storages: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("failed to list storages with status: %d", resp.StatusCode)
	}

	var response StorageListResponse
	if err := json.NewDecoder(resp.Body).Decode(&response); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	return response.Storages, nil
}

func (c *EnhancedClient) GetStorage(ctx context.Context, id string) (*Storage, error) {
	req, err := http.NewRequestWithContext(ctx, "GET", c.baseURL+"/storages/"+id, nil)
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	req.Header.Set("Authorization", "Bearer "+c.authToken)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to get storage: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("failed to get storage with status: %d", resp.StatusCode)
	}

	var storage Storage
	if err := json.NewDecoder(resp.Body).Decode(&storage); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	return &storage, nil
}

func (c *EnhancedClient) CreateStorage(ctx context.Context, storage *Storage) (*Storage, error) {
	jsonData, err := json.Marshal(storage)
	if err != nil {
		return nil, fmt.Errorf("failed to marshal storage: %w", err)
	}

	req, err := http.NewRequestWithContext(ctx, "POST", c.baseURL+"/storages", 
		bytes.NewBuffer(jsonData))
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	req.Header.Set("Content-Type", "application/json")
	req.Header.Set("Authorization", "Bearer "+c.authToken)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to create storage: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusCreated {
		return nil, fmt.Errorf("failed to create storage with status: %d", resp.StatusCode)
	}

	var createdStorage Storage
	if err := json.NewDecoder(resp.Body).Decode(&createdStorage); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	return &createdStorage, nil
}

func (c *EnhancedClient) DeleteStorage(ctx context.Context, id string) error {
	req, err := http.NewRequestWithContext(ctx, "DELETE", c.baseURL+"/storages/"+id, nil)
	if err != nil {
		return fmt.Errorf("failed to create request: %w", err)
	}

	req.Header.Set("Authorization", "Bearer "+c.authToken)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return fmt.Errorf("failed to delete storage: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusNoContent {
		return fmt.Errorf("failed to delete storage with status: %d", resp.StatusCode)
	}

	return nil
}

// Statistics and monitoring
func (c *EnhancedClient) GetStats(ctx context.Context) (*CloudPodsStats, error) {
	req, err := http.NewRequestWithContext(ctx, "GET", c.baseURL+"/stats", nil)
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	req.Header.Set("Authorization", "Bearer "+c.authToken)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to get stats: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("failed to get stats with status: %d", resp.StatusCode)
	}

	var stats CloudPodsStats
	if err := json.NewDecoder(resp.Body).Decode(&stats); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	return &stats, nil
}

// Resource search and filtering
func (c *EnhancedClient) SearchResources(ctx context.Context, query string, resourceType string) ([]Resource, error) {
	req, err := http.NewRequestWithContext(ctx, "GET", c.baseURL+"/search", nil)
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	q := req.URL.Query()
	q.Add("q", query)
	q.Add("type", resourceType)
	req.URL.RawQuery = q.Encode()

	req.Header.Set("Authorization", "Bearer "+c.authToken)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to search resources: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("failed to search resources with status: %d", resp.StatusCode)
	}

	var response ListResponse
	if err := json.NewDecoder(resp.Body).Decode(&response); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	return response.Resources, nil
}

// Bulk operations
func (c *EnhancedClient) BulkCreateInstances(ctx context.Context, instances []Instance) ([]Instance, error) {
	jsonData, err := json.Marshal(instances)
	if err != nil {
		return nil, fmt.Errorf("failed to marshal instances: %w", err)
	}

	req, err := http.NewRequestWithContext(ctx, "POST", c.baseURL+"/instances/bulk", 
		bytes.NewBuffer(jsonData))
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	req.Header.Set("Content-Type", "application/json")
	req.Header.Set("Authorization", "Bearer "+c.authToken)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to bulk create instances: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusCreated {
		return nil, fmt.Errorf("failed to bulk create instances with status: %d", resp.StatusCode)
	}

	var createdInstances []Instance
	if err := json.NewDecoder(resp.Body).Decode(&createdInstances); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	return createdInstances, nil
}

func (c *EnhancedClient) BulkDeleteInstances(ctx context.Context, ids []string) error {
	jsonData, err := json.Marshal(ids)
	if err != nil {
		return fmt.Errorf("failed to marshal ids: %w", err)
	}

	req, err := http.NewRequestWithContext(ctx, "DELETE", c.baseURL+"/instances/bulk", 
		bytes.NewBuffer(jsonData))
	if err != nil {
		return fmt.Errorf("failed to create request: %w", err)
	}

	req.Header.Set("Content-Type", "application/json")
	req.Header.Set("Authorization", "Bearer "+c.authToken)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return fmt.Errorf("failed to bulk delete instances: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusNoContent {
		return fmt.Errorf("failed to bulk delete instances with status: %d", resp.StatusCode)
	}

	return nil
}
