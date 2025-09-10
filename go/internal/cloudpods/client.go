package cloudpods

import (
	"bytes"
	"context"
	"encoding/json"
	"fmt"
	"io"
	"net/http"
	"net/url"
	"time"
)

// Client represents a CloudPods API client
type Client struct {
	baseURL    string
	username   string
	password   string
	httpClient *http.Client
	token      string
}

// NewClient creates a new CloudPods client
func NewClient(baseURL, username, password string) *Client {
	return &Client{
		baseURL:  baseURL,
		username: username,
		password: password,
		httpClient: &http.Client{
			Timeout: 30 * time.Second,
		},
	}
}

// Authenticate authenticates with CloudPods and retrieves a token
func (c *Client) Authenticate(ctx context.Context) error {
	authURL := fmt.Sprintf("%s/api/v1/auth", c.baseURL)
	
	authReq := map[string]string{
		"username": c.username,
		"password": c.password,
	}
	
	jsonData, err := json.Marshal(authReq)
	if err != nil {
		return fmt.Errorf("failed to marshal auth request: %w", err)
	}
	
	req, err := http.NewRequestWithContext(ctx, "POST", authURL, bytes.NewBuffer(jsonData))
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

// makeRequest makes an authenticated request to CloudPods API
func (c *Client) makeRequest(ctx context.Context, method, endpoint string, body interface{}) (*http.Response, error) {
	if c.token == "" {
		if err := c.Authenticate(ctx); err != nil {
			return nil, fmt.Errorf("authentication required: %w", err)
		}
	}
	
	url := fmt.Sprintf("%s%s", c.baseURL, endpoint)
	
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
	
	req.Header.Set("Authorization", "Bearer "+c.token)
	req.Header.Set("Content-Type", "application/json")
	
	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to make request: %w", err)
	}
	
	// If unauthorized, try to re-authenticate once
	if resp.StatusCode == http.StatusUnauthorized {
		resp.Body.Close()
		if err := c.Authenticate(ctx); err != nil {
			return nil, fmt.Errorf("re-authentication failed: %w", err)
		}
		
		// Retry the request
		req.Header.Set("Authorization", "Bearer "+c.token)
		resp, err = c.httpClient.Do(req)
		if err != nil {
			return nil, fmt.Errorf("failed to retry request: %w", err)
		}
	}
	
	return resp, nil
}

// GetRegions retrieves all available regions
func (c *Client) GetRegions(ctx context.Context) ([]Region, error) {
	resp, err := c.makeRequest(ctx, "GET", "/api/v1/regions", nil)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()
	
	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("failed to get regions: status %d", resp.StatusCode)
	}
	
	var regions []Region
	if err := json.NewDecoder(resp.Body).Decode(&regions); err != nil {
		return nil, fmt.Errorf("failed to decode regions: %w", err)
	}
	
	return regions, nil
}

// GetZones retrieves zones for a specific region
func (c *Client) GetZones(ctx context.Context, regionID string) ([]Zone, error) {
	endpoint := fmt.Sprintf("/api/v1/regions/%s/zones", regionID)
	resp, err := c.makeRequest(ctx, "GET", endpoint, nil)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()
	
	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("failed to get zones: status %d", resp.StatusCode)
	}
	
	var zones []Zone
	if err := json.NewDecoder(resp.Body).Decode(&zones); err != nil {
		return nil, fmt.Errorf("failed to decode zones: %w", err)
	}
	
	return zones, nil
}

// GetVPCs retrieves VPCs for a specific region
func (c *Client) GetVPCs(ctx context.Context, regionID string) ([]VPC, error) {
	endpoint := fmt.Sprintf("/api/v1/regions/%s/vpcs", regionID)
	resp, err := c.makeRequest(ctx, "GET", endpoint, nil)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()
	
	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("failed to get VPCs: status %d", resp.StatusCode)
	}
	
	var vpcs []VPC
	if err := json.NewDecoder(resp.Body).Decode(&vpcs); err != nil {
		return nil, fmt.Errorf("failed to decode VPCs: %w", err)
	}
	
	return vpcs, nil
}

// GetSubnets retrieves subnets for a specific VPC
func (c *Client) GetSubnets(ctx context.Context, vpcID string) ([]Subnet, error) {
	endpoint := fmt.Sprintf("/api/v1/vpcs/%s/subnets", vpcID)
	resp, err := c.makeRequest(ctx, "GET", endpoint, nil)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()
	
	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("failed to get subnets: status %d", resp.StatusCode)
	}
	
	var subnets []Subnet
	if err := json.NewDecoder(resp.Body).Decode(&subnets); err != nil {
		return nil, fmt.Errorf("failed to decode subnets: %w", err)
	}
	
	return subnets, nil
}

// GetInstances retrieves instances for a specific region
func (c *Client) GetInstances(ctx context.Context, regionID string) ([]Instance, error) {
	endpoint := fmt.Sprintf("/api/v1/regions/%s/instances", regionID)
	resp, err := c.makeRequest(ctx, "GET", endpoint, nil)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()
	
	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("failed to get instances: status %d", resp.StatusCode)
	}
	
	var instances []Instance
	if err := json.NewDecoder(resp.Body).Decode(&instances); err != nil {
		return nil, fmt.Errorf("failed to decode instances: %w", err)
	}
	
	return instances, nil
}

// CreateInstance creates a new instance
func (c *Client) CreateInstance(ctx context.Context, req CreateInstanceRequest) (*Instance, error) {
	resp, err := c.makeRequest(ctx, "POST", "/api/v1/instances", req)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()
	
	if resp.StatusCode != http.StatusCreated {
		return nil, fmt.Errorf("failed to create instance: status %d", resp.StatusCode)
	}
	
	var instance Instance
	if err := json.NewDecoder(resp.Body).Decode(&instance); err != nil {
		return nil, fmt.Errorf("failed to decode instance: %w", err)
	}
	
	return &instance, nil
}

// DeleteInstance deletes an instance
func (c *Client) DeleteInstance(ctx context.Context, instanceID string) error {
	endpoint := fmt.Sprintf("/api/v1/instances/%s", instanceID)
	resp, err := c.makeRequest(ctx, "DELETE", endpoint, nil)
	if err != nil {
		return err
	}
	defer resp.Body.Close()
	
	if resp.StatusCode != http.StatusOK {
		return fmt.Errorf("failed to delete instance: status %d", resp.StatusCode)
	}
	
	return nil
}

// GetNetworks retrieves network information
func (c *Client) GetNetworks(ctx context.Context, regionID string) ([]Network, error) {
	endpoint := fmt.Sprintf("/api/v1/regions/%s/networks", regionID)
	resp, err := c.makeRequest(ctx, "GET", endpoint, nil)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()
	
	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("failed to get networks: status %d", resp.StatusCode)
	}
	
	var networks []Network
	if err := json.NewDecoder(resp.Body).Decode(&networks); err != nil {
		return nil, fmt.Errorf("failed to decode networks: %w", err)
	}
	
	return networks, nil
}

// GetLoadBalancers retrieves load balancers for a region
func (c *Client) GetLoadBalancers(ctx context.Context, regionID string) ([]LoadBalancer, error) {
	endpoint := fmt.Sprintf("/api/v1/regions/%s/loadbalancers", regionID)
	resp, err := c.makeRequest(ctx, "GET", endpoint, nil)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()
	
	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("failed to get load balancers: status %d", resp.StatusCode)
	}
	
	var loadBalancers []LoadBalancer
	if err := json.NewDecoder(resp.Body).Decode(&loadBalancers); err != nil {
		return nil, fmt.Errorf("failed to decode load balancers: %w", err)
	}
	
	return loadBalancers, nil
}

// GetSecurityGroups retrieves security groups for a region
func (c *Client) GetSecurityGroups(ctx context.Context, regionID string) ([]SecurityGroup, error) {
	endpoint := fmt.Sprintf("/api/v1/regions/%s/securitygroups", regionID)
	resp, err := c.makeRequest(ctx, "GET", endpoint, nil)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()
	
	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("failed to get security groups: status %d", resp.StatusCode)
	}
	
	var securityGroups []SecurityGroup
	if err := json.NewDecoder(resp.Body).Decode(&securityGroups); err != nil {
		return nil, fmt.Errorf("failed to decode security groups: %w", err)
	}
	
	return securityGroups, nil
}

// GetMetrics retrieves metrics for a specific resource
func (c *Client) GetMetrics(ctx context.Context, resourceType, resourceID string, timeRange string) (*Metrics, error) {
	endpoint := fmt.Sprintf("/api/v1/metrics/%s/%s", resourceType, resourceID)
	
	params := url.Values{}
	params.Set("time_range", timeRange)
	
	fullURL := fmt.Sprintf("%s?%s", endpoint, params.Encode())
	resp, err := c.makeRequest(ctx, "GET", fullURL, nil)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()
	
	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("failed to get metrics: status %d", resp.StatusCode)
	}
	
	var metrics Metrics
	if err := json.NewDecoder(resp.Body).Decode(&metrics); err != nil {
		return nil, fmt.Errorf("failed to decode metrics: %w", err)
	}
	
	return &metrics, nil
}
