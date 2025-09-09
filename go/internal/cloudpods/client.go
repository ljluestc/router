package cloudpods

import (
	"context"
	"encoding/json"
	"fmt"
	"net/http"
	"time"
)

// Client represents a CloudPods API client
type Client struct {
	baseURL    string
	httpClient *http.Client
	auth       AuthConfig
}

// AuthConfig holds authentication configuration
type AuthConfig struct {
	Type     string `json:"type"`
	Username string `json:"username"`
	Password string `json:"password"`
	Token    string `json:"token"`
}

// Config holds CloudPods client configuration
type Config struct {
	BaseURL string     `json:"base_url"`
	Timeout time.Duration `json:"timeout"`
	Auth    AuthConfig `json:"auth"`
	Region  string     `json:"region"`
}

// NewClient creates a new CloudPods client
func NewClient(config Config) *Client {
	return &Client{
		baseURL: config.BaseURL,
		httpClient: &http.Client{
			Timeout: config.Timeout,
		},
		auth: config.Auth,
	}
}

// VPC represents a CloudPods VPC
type VPC struct {
	ID          string            `json:"id"`
	Name        string            `json:"name"`
	CIDR        string            `json:"cidr"`
	Region      string            `json:"region"`
	Status      string            `json:"status"`
	CreatedAt   time.Time         `json:"created_at"`
	UpdatedAt   time.Time         `json:"updated_at"`
	Tags        map[string]string `json:"tags"`
	Subnets     []Subnet          `json:"subnets"`
	Gateways    []Gateway         `json:"gateways"`
}

// Subnet represents a CloudPods subnet
type Subnet struct {
	ID        string            `json:"id"`
	Name      string            `json:"name"`
	CIDR      string            `json:"cidr"`
	VPCID     string            `json:"vpc_id"`
	Zone      string            `json:"zone"`
	Status    string            `json:"status"`
	CreatedAt time.Time         `json:"created_at"`
	Tags      map[string]string `json:"tags"`
}

// Gateway represents a CloudPods gateway
type Gateway struct {
	ID        string            `json:"id"`
	Name      string            `json:"name"`
	Type      string            `json:"type"`
	VPCID     string            `json:"vpc_id"`
	SubnetID  string            `json:"subnet_id"`
	IPAddress string            `json:"ip_address"`
	Status    string            `json:"status"`
	CreatedAt time.Time         `json:"created_at"`
	Tags      map[string]string `json:"tags"`
}

// Route represents a CloudPods route
type Route struct {
	ID          string    `json:"id"`
	Destination string    `json:"destination"`
	NextHop     string    `json:"next_hop"`
	VPCID       string    `json:"vpc_id"`
	Type        string    `json:"type"`
	Status      string    `json:"status"`
	CreatedAt   time.Time `json:"created_at"`
}

// SecurityGroup represents a CloudPods security group
type SecurityGroup struct {
	ID          string            `json:"id"`
	Name        string            `json:"name"`
	Description string            `json:"description"`
	VPCID       string            `json:"vpc_id"`
	Rules       []SecurityRule    `json:"rules"`
	Status      string            `json:"status"`
	CreatedAt   time.Time         `json:"created_at"`
	Tags        map[string]string `json:"tags"`
}

// SecurityRule represents a CloudPods security group rule
type SecurityRule struct {
	ID            string `json:"id"`
	Direction     string `json:"direction"`
	Protocol      string `json:"protocol"`
	PortRange     string `json:"port_range"`
	Source        string `json:"source"`
	Destination   string `json:"destination"`
	Action        string `json:"action"`
	Priority      int    `json:"priority"`
	Description   string `json:"description"`
}

// ListVPCs retrieves all VPCs
func (c *Client) ListVPCs(ctx context.Context) ([]VPC, error) {
	req, err := http.NewRequestWithContext(ctx, "GET", c.baseURL+"/api/v1/vpcs", nil)
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	c.setAuthHeaders(req)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to execute request: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("API request failed with status: %d", resp.StatusCode)
	}

	var result struct {
		VPCs []VPC `json:"vpcs"`
	}

	if err := json.NewDecoder(resp.Body).Decode(&result); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	return result.VPCs, nil
}

// GetVPC retrieves a specific VPC by ID
func (c *Client) GetVPC(ctx context.Context, vpcID string) (*VPC, error) {
	req, err := http.NewRequestWithContext(ctx, "GET", c.baseURL+"/api/v1/vpcs/"+vpcID, nil)
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	c.setAuthHeaders(req)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to execute request: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("API request failed with status: %d", resp.StatusCode)
	}

	var vpc VPC
	if err := json.NewDecoder(resp.Body).Decode(&vpc); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	return &vpc, nil
}

// CreateVPC creates a new VPC
func (c *Client) CreateVPC(ctx context.Context, vpc VPC) (*VPC, error) {
	jsonData, err := json.Marshal(vpc)
	if err != nil {
		return nil, fmt.Errorf("failed to marshal VPC: %w", err)
	}

	req, err := http.NewRequestWithContext(ctx, "POST", c.baseURL+"/api/v1/vpcs", 
		bytes.NewBuffer(jsonData))
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	req.Header.Set("Content-Type", "application/json")
	c.setAuthHeaders(req)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to execute request: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusCreated {
		return nil, fmt.Errorf("API request failed with status: %d", resp.StatusCode)
	}

	var createdVPC VPC
	if err := json.NewDecoder(resp.Body).Decode(&createdVPC); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	return &createdVPC, nil
}

// DeleteVPC deletes a VPC
func (c *Client) DeleteVPC(ctx context.Context, vpcID string) error {
	req, err := http.NewRequestWithContext(ctx, "DELETE", c.baseURL+"/api/v1/vpcs/"+vpcID, nil)
	if err != nil {
		return fmt.Errorf("failed to create request: %w", err)
	}

	c.setAuthHeaders(req)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return fmt.Errorf("failed to execute request: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusNoContent {
		return fmt.Errorf("API request failed with status: %d", resp.StatusCode)
	}

	return nil
}

// ListSubnets retrieves all subnets for a VPC
func (c *Client) ListSubnets(ctx context.Context, vpcID string) ([]Subnet, error) {
	req, err := http.NewRequestWithContext(ctx, "GET", 
		c.baseURL+"/api/v1/vpcs/"+vpcID+"/subnets", nil)
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	c.setAuthHeaders(req)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to execute request: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("API request failed with status: %d", resp.StatusCode)
	}

	var result struct {
		Subnets []Subnet `json:"subnets"`
	}

	if err := json.NewDecoder(resp.Body).Decode(&result); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	return result.Subnets, nil
}

// ListRoutes retrieves all routes for a VPC
func (c *Client) ListRoutes(ctx context.Context, vpcID string) ([]Route, error) {
	req, err := http.NewRequestWithContext(ctx, "GET", 
		c.baseURL+"/api/v1/vpcs/"+vpcID+"/routes", nil)
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	c.setAuthHeaders(req)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to execute request: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("API request failed with status: %d", resp.StatusCode)
	}

	var result struct {
		Routes []Route `json:"routes"`
	}

	if err := json.NewDecoder(resp.Body).Decode(&result); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	return result.Routes, nil
}

// CreateRoute creates a new route
func (c *Client) CreateRoute(ctx context.Context, vpcID string, route Route) (*Route, error) {
	jsonData, err := json.Marshal(route)
	if err != nil {
		return nil, fmt.Errorf("failed to marshal route: %w", err)
	}

	req, err := http.NewRequestWithContext(ctx, "POST", 
		c.baseURL+"/api/v1/vpcs/"+vpcID+"/routes", bytes.NewBuffer(jsonData))
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	req.Header.Set("Content-Type", "application/json")
	c.setAuthHeaders(req)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to execute request: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusCreated {
		return nil, fmt.Errorf("API request failed with status: %d", resp.StatusCode)
	}

	var createdRoute Route
	if err := json.NewDecoder(resp.Body).Decode(&createdRoute); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	return &createdRoute, nil
}

// DeleteRoute deletes a route
func (c *Client) DeleteRoute(ctx context.Context, vpcID, routeID string) error {
	req, err := http.NewRequestWithContext(ctx, "DELETE", 
		c.baseURL+"/api/v1/vpcs/"+vpcID+"/routes/"+routeID, nil)
	if err != nil {
		return fmt.Errorf("failed to create request: %w", err)
	}

	c.setAuthHeaders(req)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return fmt.Errorf("failed to execute request: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusNoContent {
		return fmt.Errorf("API request failed with status: %d", resp.StatusCode)
	}

	return nil
}

// ListSecurityGroups retrieves all security groups for a VPC
func (c *Client) ListSecurityGroups(ctx context.Context, vpcID string) ([]SecurityGroup, error) {
	req, err := http.NewRequestWithContext(ctx, "GET", 
		c.baseURL+"/api/v1/vpcs/"+vpcID+"/security-groups", nil)
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	c.setAuthHeaders(req)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to execute request: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("API request failed with status: %d", resp.StatusCode)
	}

	var result struct {
		SecurityGroups []SecurityGroup `json:"security_groups"`
	}

	if err := json.NewDecoder(resp.Body).Decode(&result); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	return result.SecurityGroups, nil
}

// setAuthHeaders sets authentication headers based on auth type
func (c *Client) setAuthHeaders(req *http.Request) {
	switch c.auth.Type {
	case "basic":
		req.SetBasicAuth(c.auth.Username, c.auth.Password)
	case "token":
		req.Header.Set("Authorization", "Bearer "+c.auth.Token)
	case "api-key":
		req.Header.Set("X-API-Key", c.auth.Token)
	}
}
