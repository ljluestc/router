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

	"router-sim/internal/config"

	"github.com/sirupsen/logrus"
)

// Client represents a CloudPods API client
type Client struct {
	config     config.CloudPodsConfig
	httpClient *http.Client
	baseURL    *url.URL
	authToken  string
}

// NewClient creates a new CloudPods client
func NewClient(cfg config.CloudPodsConfig) *Client {
	baseURL, _ := url.Parse(cfg.API.BaseURL)
	
	return &Client{
		config: cfg,
		httpClient: &http.Client{
			Timeout: cfg.Timeout,
		},
		baseURL: baseURL,
	}
}

// Authenticate authenticates with CloudPods API
func (c *Client) Authenticate(ctx context.Context) error {
	logrus.Info("Authenticating with CloudPods API")

	authURL := c.baseURL.ResolveReference(&url.URL{Path: "/api/v1/auth/login"})
	
	authReq := map[string]string{
		"username": c.config.Auth.Username,
		"password": c.config.Auth.Password,
	}

	jsonData, err := json.Marshal(authReq)
	if err != nil {
		return fmt.Errorf("failed to marshal auth request: %w", err)
	}

	req, err := http.NewRequestWithContext(ctx, "POST", authURL.String(), bytes.NewBuffer(jsonData))
	if err != nil {
		return fmt.Errorf("failed to create auth request: %w", err)
	}

	req.Header.Set("Content-Type", "application/json")
	req.Header.Set("Accept", "application/json")

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return fmt.Errorf("failed to send auth request: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		body, _ := io.ReadAll(resp.Body)
		return fmt.Errorf("authentication failed: %s", string(body))
	}

	var authResp struct {
		Token string `json:"token"`
	}

	if err := json.NewDecoder(resp.Body).Decode(&authResp); err != nil {
		return fmt.Errorf("failed to decode auth response: %w", err)
	}

	c.authToken = authResp.Token
	logrus.Info("Successfully authenticated with CloudPods API")

	return nil
}

// GetProjects retrieves all projects
func (c *Client) GetProjects(ctx context.Context) ([]Project, error) {
	logrus.Debug("Retrieving CloudPods projects")

	projectsURL := c.baseURL.ResolveReference(&url.URL{Path: "/api/v1/projects"})
	
	req, err := http.NewRequestWithContext(ctx, "GET", projectsURL.String(), nil)
	if err != nil {
		return nil, fmt.Errorf("failed to create projects request: %w", err)
	}

	req.Header.Set("Authorization", "Bearer "+c.authToken)
	req.Header.Set("Accept", "application/json")

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to send projects request: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		body, _ := io.ReadAll(resp.Body)
		return nil, fmt.Errorf("failed to get projects: %s", string(body))
	}

	var projectsResp struct {
		Projects []Project `json:"projects"`
	}

	if err := json.NewDecoder(resp.Body).Decode(&projectsResp); err != nil {
		return nil, fmt.Errorf("failed to decode projects response: %w", err)
	}

	logrus.WithField("count", len(projectsResp.Projects)).Debug("Retrieved CloudPods projects")
	return projectsResp.Projects, nil
}

// GetProject retrieves a specific project by ID
func (c *Client) GetProject(ctx context.Context, projectID string) (*Project, error) {
	logrus.WithField("project_id", projectID).Debug("Retrieving CloudPods project")

	projectURL := c.baseURL.ResolveReference(&url.URL{Path: fmt.Sprintf("/api/v1/projects/%s", projectID)})
	
	req, err := http.NewRequestWithContext(ctx, "GET", projectURL.String(), nil)
	if err != nil {
		return nil, fmt.Errorf("failed to create project request: %w", err)
	}

	req.Header.Set("Authorization", "Bearer "+c.authToken)
	req.Header.Set("Accept", "application/json")

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to send project request: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		body, _ := io.ReadAll(resp.Body)
		return nil, fmt.Errorf("failed to get project: %s", string(body))
	}

	var project Project
	if err := json.NewDecoder(resp.Body).Decode(&project); err != nil {
		return nil, fmt.Errorf("failed to decode project response: %w", err)
	}

	logrus.WithField("project_id", projectID).Debug("Retrieved CloudPods project")
	return &project, nil
}

// GetVPCs retrieves VPCs for a project
func (c *Client) GetVPCs(ctx context.Context, projectID string) ([]VPC, error) {
	logrus.WithField("project_id", projectID).Debug("Retrieving CloudPods VPCs")

	vpcsURL := c.baseURL.ResolveReference(&url.URL{Path: fmt.Sprintf("/api/v1/projects/%s/vpcs", projectID)})
	
	req, err := http.NewRequestWithContext(ctx, "GET", vpcsURL.String(), nil)
	if err != nil {
		return nil, fmt.Errorf("failed to create VPCs request: %w", err)
	}

	req.Header.Set("Authorization", "Bearer "+c.authToken)
	req.Header.Set("Accept", "application/json")

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to send VPCs request: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		body, _ := io.ReadAll(resp.Body)
		return nil, fmt.Errorf("failed to get VPCs: %s", string(body))
	}

	var vpcsResp struct {
		VPCs []VPC `json:"vpcs"`
	}

	if err := json.NewDecoder(resp.Body).Decode(&vpcsResp); err != nil {
		return nil, fmt.Errorf("failed to decode VPCs response: %w", err)
	}

	logrus.WithField("project_id", projectID).WithField("count", len(vpcsResp.VPCs)).Debug("Retrieved CloudPods VPCs")
	return vpcsResp.VPCs, nil
}

// GetSubnets retrieves subnets for a VPC
func (c *Client) GetSubnets(ctx context.Context, projectID, vpcID string) ([]Subnet, error) {
	logrus.WithField("project_id", projectID).WithField("vpc_id", vpcID).Debug("Retrieving CloudPods subnets")

	subnetsURL := c.baseURL.ResolveReference(&url.URL{Path: fmt.Sprintf("/api/v1/projects/%s/vpcs/%s/subnets", projectID, vpcID)})
	
	req, err := http.NewRequestWithContext(ctx, "GET", subnetsURL.String(), nil)
	if err != nil {
		return nil, fmt.Errorf("failed to create subnets request: %w", err)
	}

	req.Header.Set("Authorization", "Bearer "+c.authToken)
	req.Header.Set("Accept", "application/json")

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to send subnets request: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		body, _ := io.ReadAll(resp.Body)
		return nil, fmt.Errorf("failed to get subnets: %s", string(body))
	}

	var subnetsResp struct {
		Subnets []Subnet `json:"subnets"`
	}

	if err := json.NewDecoder(resp.Body).Decode(&subnetsResp); err != nil {
		return nil, fmt.Errorf("failed to decode subnets response: %w", err)
	}

	logrus.WithField("project_id", projectID).WithField("vpc_id", vpcID).WithField("count", len(subnetsResp.Subnets)).Debug("Retrieved CloudPods subnets")
	return subnetsResp.Subnets, nil
}

// GetInstances retrieves instances for a project
func (c *Client) GetInstances(ctx context.Context, projectID string) ([]Instance, error) {
	logrus.WithField("project_id", projectID).Debug("Retrieving CloudPods instances")

	instancesURL := c.baseURL.ResolveReference(&url.URL{Path: fmt.Sprintf("/api/v1/projects/%s/instances", projectID)})
	
	req, err := http.NewRequestWithContext(ctx, "GET", instancesURL.String(), nil)
	if err != nil {
		return nil, fmt.Errorf("failed to create instances request: %w", err)
	}

	req.Header.Set("Authorization", "Bearer "+c.authToken)
	req.Header.Set("Accept", "application/json")

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to send instances request: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		body, _ := io.ReadAll(resp.Body)
		return nil, fmt.Errorf("failed to get instances: %s", string(body))
	}

	var instancesResp struct {
		Instances []Instance `json:"instances"`
	}

	if err := json.NewDecoder(resp.Body).Decode(&instancesResp); err != nil {
		return nil, fmt.Errorf("failed to decode instances response: %w", err)
	}

	logrus.WithField("project_id", projectID).WithField("count", len(instancesResp.Instances)).Debug("Retrieved CloudPods instances")
	return instancesResp.Instances, nil
}

// GetLoadBalancers retrieves load balancers for a project
func (c *Client) GetLoadBalancers(ctx context.Context, projectID string) ([]LoadBalancer, error) {
	logrus.WithField("project_id", projectID).Debug("Retrieving CloudPods load balancers")

	lbURL := c.baseURL.ResolveReference(&url.URL{Path: fmt.Sprintf("/api/v1/projects/%s/loadbalancers", projectID)})
	
	req, err := http.NewRequestWithContext(ctx, "GET", lbURL.String(), nil)
	if err != nil {
		return nil, fmt.Errorf("failed to create load balancers request: %w", err)
	}

	req.Header.Set("Authorization", "Bearer "+c.authToken)
	req.Header.Set("Accept", "application/json")

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to send load balancers request: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		body, _ := io.ReadAll(resp.Body)
		return nil, fmt.Errorf("failed to get load balancers: %s", string(body))
	}

	var lbResp struct {
		LoadBalancers []LoadBalancer `json:"loadbalancers"`
	}

	if err := json.NewDecoder(resp.Body).Decode(&lbResp); err != nil {
		return nil, fmt.Errorf("failed to decode load balancers response: %w", err)
	}

	logrus.WithField("project_id", projectID).WithField("count", len(lbResp.LoadBalancers)).Debug("Retrieved CloudPods load balancers")
	return lbResp.LoadBalancers, nil
}

// GetNATGateways retrieves NAT gateways for a project
func (c *Client) GetNATGateways(ctx context.Context, projectID string) ([]NATGateway, error) {
	logrus.WithField("project_id", projectID).Debug("Retrieving CloudPods NAT gateways")

	natURL := c.baseURL.ResolveReference(&url.URL{Path: fmt.Sprintf("/api/v1/projects/%s/natgateways", projectID)})
	
	req, err := http.NewRequestWithContext(ctx, "GET", natURL.String(), nil)
	if err != nil {
		return nil, fmt.Errorf("failed to create NAT gateways request: %w", err)
	}

	req.Header.Set("Authorization", "Bearer "+c.authToken)
	req.Header.Set("Accept", "application/json")

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to send NAT gateways request: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		body, _ := io.ReadAll(resp.Body)
		return nil, fmt.Errorf("failed to get NAT gateways: %s", string(body))
	}

	var natResp struct {
		NATGateways []NATGateway `json:"natgateways"`
	}

	if err := json.NewDecoder(resp.Body).Decode(&natResp); err != nil {
		return nil, fmt.Errorf("failed to decode NAT gateways response: %w", err)
	}

	logrus.WithField("project_id", projectID).WithField("count", len(natResp.NATGateways)).Debug("Retrieved CloudPods NAT gateways")
	return natResp.NATGateways, nil
}

// GetNetworkTopology retrieves network topology for a project
func (c *Client) GetNetworkTopology(ctx context.Context, projectID string) (*NetworkTopology, error) {
	logrus.WithField("project_id", projectID).Debug("Retrieving CloudPods network topology")

	topologyURL := c.baseURL.ResolveReference(&url.URL{Path: fmt.Sprintf("/api/v1/projects/%s/network-topology", projectID)})
	
	req, err := http.NewRequestWithContext(ctx, "GET", topologyURL.String(), nil)
	if err != nil {
		return nil, fmt.Errorf("failed to create topology request: %w", err)
	}

	req.Header.Set("Authorization", "Bearer "+c.authToken)
	req.Header.Set("Accept", "application/json")

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to send topology request: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		body, _ := io.ReadAll(resp.Body)
		return nil, fmt.Errorf("failed to get network topology: %s", string(body))
	}

	var topology NetworkTopology
	if err := json.NewDecoder(resp.Body).Decode(&topology); err != nil {
		return nil, fmt.Errorf("failed to decode topology response: %w", err)
	}

	logrus.WithField("project_id", projectID).Debug("Retrieved CloudPods network topology")
	return &topology, nil
}

// CreateVPC creates a new VPC
func (c *Client) CreateVPC(ctx context.Context, projectID string, vpcReq CreateVPCRequest) (*VPC, error) {
	logrus.WithField("project_id", projectID).WithField("vpc_name", vpcReq.Name).Debug("Creating CloudPods VPC")

	vpcURL := c.baseURL.ResolveReference(&url.URL{Path: fmt.Sprintf("/api/v1/projects/%s/vpcs", projectID)})
	
	jsonData, err := json.Marshal(vpcReq)
	if err != nil {
		return nil, fmt.Errorf("failed to marshal VPC request: %w", err)
	}

	req, err := http.NewRequestWithContext(ctx, "POST", vpcURL.String(), bytes.NewBuffer(jsonData))
	if err != nil {
		return nil, fmt.Errorf("failed to create VPC request: %w", err)
	}

	req.Header.Set("Authorization", "Bearer "+c.authToken)
	req.Header.Set("Content-Type", "application/json")
	req.Header.Set("Accept", "application/json")

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to send VPC request: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusCreated {
		body, _ := io.ReadAll(resp.Body)
		return nil, fmt.Errorf("failed to create VPC: %s", string(body))
	}

	var vpc VPC
	if err := json.NewDecoder(resp.Body).Decode(&vpc); err != nil {
		return nil, fmt.Errorf("failed to decode VPC response: %w", err)
	}

	logrus.WithField("project_id", projectID).WithField("vpc_id", vpc.ID).Debug("Created CloudPods VPC")
	return &vpc, nil
}

// DeleteVPC deletes a VPC
func (c *Client) DeleteVPC(ctx context.Context, projectID, vpcID string) error {
	logrus.WithField("project_id", projectID).WithField("vpc_id", vpcID).Debug("Deleting CloudPods VPC")

	vpcURL := c.baseURL.ResolveReference(&url.URL{Path: fmt.Sprintf("/api/v1/projects/%s/vpcs/%s", projectID, vpcID)})
	
	req, err := http.NewRequestWithContext(ctx, "DELETE", vpcURL.String(), nil)
	if err != nil {
		return fmt.Errorf("failed to create VPC delete request: %w", err)
	}

	req.Header.Set("Authorization", "Bearer "+c.authToken)
	req.Header.Set("Accept", "application/json")

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return fmt.Errorf("failed to send VPC delete request: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusNoContent {
		body, _ := io.ReadAll(resp.Body)
		return fmt.Errorf("failed to delete VPC: %s", string(body))
	}

	logrus.WithField("project_id", projectID).WithField("vpc_id", vpcID).Debug("Deleted CloudPods VPC")
	return nil
}
