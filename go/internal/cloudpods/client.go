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

// CloudPodsClient represents a client for CloudPods API
type CloudPodsClient struct {
	BaseURL    string
	Username   string
	Password   string
	Token      string
	HTTPClient *http.Client
}

// NewCloudPodsClient creates a new CloudPods client
func NewCloudPodsClient(baseURL, username, password string) *CloudPodsClient {
	return &CloudPodsClient{
		BaseURL:  baseURL,
		Username: username,
		Password: password,
		HTTPClient: &http.Client{
			Timeout: 30 * time.Second,
		},
	}
}

// Authenticate authenticates with CloudPods and retrieves a token
func (c *CloudPodsClient) Authenticate(ctx context.Context) error {
	authReq := map[string]string{
		"username": c.Username,
		"password": c.Password,
	}

	jsonData, err := json.Marshal(authReq)
	if err != nil {
		return fmt.Errorf("failed to marshal auth request: %w", err)
	}

	req, err := http.NewRequestWithContext(ctx, "POST", c.BaseURL+"/api/v1/auth", bytes.NewBuffer(jsonData))
	if err != nil {
		return fmt.Errorf("failed to create auth request: %w", err)
	}

	req.Header.Set("Content-Type", "application/json")

	resp, err := c.HTTPClient.Do(req)
	if err != nil {
		return fmt.Errorf("failed to send auth request: %w", err)
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

	c.Token = authResp.Token
	return nil
}

// GetRegions retrieves all available regions
func (c *CloudPodsClient) GetRegions(ctx context.Context) ([]Region, error) {
	req, err := c.createAuthenticatedRequest(ctx, "GET", "/api/v1/regions", nil)
	if err != nil {
		return nil, err
	}

	resp, err := c.HTTPClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to get regions: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("failed to get regions with status: %d", resp.StatusCode)
	}

	var regionsResp struct {
		Regions []Region `json:"regions"`
	}

	if err := json.NewDecoder(resp.Body).Decode(&regionsResp); err != nil {
		return nil, fmt.Errorf("failed to decode regions response: %w", err)
	}

	return regionsResp.Regions, nil
}

// GetZones retrieves zones for a specific region
func (c *CloudPodsClient) GetZones(ctx context.Context, regionID string) ([]Zone, error) {
	req, err := c.createAuthenticatedRequest(ctx, "GET", fmt.Sprintf("/api/v1/regions/%s/zones", regionID), nil)
	if err != nil {
		return nil, err
	}

	resp, err := c.HTTPClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to get zones: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("failed to get zones with status: %d", resp.StatusCode)
	}

	var zonesResp struct {
		Zones []Zone `json:"zones"`
	}

	if err := json.NewDecoder(resp.Body).Decode(&zonesResp); err != nil {
		return nil, fmt.Errorf("failed to decode zones response: %w", err)
	}

	return zonesResp.Zones, nil
}

// GetVPCs retrieves VPCs for a specific region
func (c *CloudPodsClient) GetVPCs(ctx context.Context, regionID string) ([]VPC, error) {
	req, err := c.createAuthenticatedRequest(ctx, "GET", fmt.Sprintf("/api/v1/regions/%s/vpcs", regionID), nil)
	if err != nil {
		return nil, err
	}

	resp, err := c.HTTPClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to get VPCs: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("failed to get VPCs with status: %d", resp.StatusCode)
	}

	var vpcsResp struct {
		VPCs []VPC `json:"vpcs"`
	}

	if err := json.NewDecoder(resp.Body).Decode(&vpcsResp); err != nil {
		return nil, fmt.Errorf("failed to decode VPCs response: %w", err)
	}

	return vpcsResp.VPCs, nil
}

// GetSubnets retrieves subnets for a specific VPC
func (c *CloudPodsClient) GetSubnets(ctx context.Context, vpcID string) ([]Subnet, error) {
	req, err := c.createAuthenticatedRequest(ctx, "GET", fmt.Sprintf("/api/v1/vpcs/%s/subnets", vpcID), nil)
	if err != nil {
		return nil, err
	}

	resp, err := c.HTTPClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to get subnets: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("failed to get subnets with status: %d", resp.StatusCode)
	}

	var subnetsResp struct {
		Subnets []Subnet `json:"subnets"`
	}

	if err := json.NewDecoder(resp.Body).Decode(&subnetsResp); err != nil {
		return nil, fmt.Errorf("failed to decode subnets response: %w", err)
	}

	return subnetsResp.Subnets, nil
}

// GetInstances retrieves instances for a specific region
func (c *CloudPodsClient) GetInstances(ctx context.Context, regionID string) ([]Instance, error) {
	req, err := c.createAuthenticatedRequest(ctx, "GET", fmt.Sprintf("/api/v1/regions/%s/instances", regionID), nil)
	if err != nil {
		return nil, err
	}

	resp, err := c.HTTPClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to get instances: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("failed to get instances with status: %d", resp.StatusCode)
	}

	var instancesResp struct {
		Instances []Instance `json:"instances"`
	}

	if err := json.NewDecoder(resp.Body).Decode(&instancesResp); err != nil {
		return nil, fmt.Errorf("failed to decode instances response: %w", err)
	}

	return instancesResp.Instances, nil
}

// CreateInstance creates a new instance
func (c *CloudPodsClient) CreateInstance(ctx context.Context, req CreateInstanceRequest) (*Instance, error) {
	jsonData, err := json.Marshal(req)
	if err != nil {
		return nil, fmt.Errorf("failed to marshal create instance request: %w", err)
	}

	httpReq, err := c.createAuthenticatedRequest(ctx, "POST", "/api/v1/instances", bytes.NewBuffer(jsonData))
	if err != nil {
		return nil, err
	}

	httpReq.Header.Set("Content-Type", "application/json")

	resp, err := c.HTTPClient.Do(httpReq)
	if err != nil {
		return nil, fmt.Errorf("failed to create instance: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusCreated {
		return nil, fmt.Errorf("failed to create instance with status: %d", resp.StatusCode)
	}

	var instance Instance
	if err := json.NewDecoder(resp.Body).Decode(&instance); err != nil {
		return nil, fmt.Errorf("failed to decode create instance response: %w", err)
	}

	return &instance, nil
}

// DeleteInstance deletes an instance
func (c *CloudPodsClient) DeleteInstance(ctx context.Context, instanceID string) error {
	req, err := c.createAuthenticatedRequest(ctx, "DELETE", fmt.Sprintf("/api/v1/instances/%s", instanceID), nil)
	if err != nil {
		return err
	}

	resp, err := c.HTTPClient.Do(req)
	if err != nil {
		return fmt.Errorf("failed to delete instance: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusNoContent {
		return fmt.Errorf("failed to delete instance with status: %d", resp.StatusCode)
	}

	return nil
}

// GetNetworks retrieves network information
func (c *CloudPodsClient) GetNetworks(ctx context.Context, regionID string) ([]Network, error) {
	req, err := c.createAuthenticatedRequest(ctx, "GET", fmt.Sprintf("/api/v1/regions/%s/networks", regionID), nil)
	if err != nil {
		return nil, err
	}

	resp, err := c.HTTPClient.Do(req)
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

// createAuthenticatedRequest creates an authenticated HTTP request
func (c *CloudPodsClient) createAuthenticatedRequest(ctx context.Context, method, path string, body io.Reader) (*http.Request, error) {
	req, err := http.NewRequestWithContext(ctx, method, c.BaseURL+path, body)
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	if c.Token != "" {
		req.Header.Set("Authorization", "Bearer "+c.Token)
	}

	return req, nil
}
