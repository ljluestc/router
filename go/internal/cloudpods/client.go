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
	"github.com/sirupsen/logrus"
)

type Client struct {
	config     *config.CloudPodsConfig
	httpClient *http.Client
	logger     *logrus.Logger
}

func NewClient(cfg *config.CloudPodsConfig) (*Client, error) {
	client := &Client{
		config: cfg,
		httpClient: &http.Client{
			Timeout: cfg.Timeout,
		},
		logger: logrus.New(),
	}

	// Test connection
	if err := client.testConnection(); err != nil {
		return nil, fmt.Errorf("failed to connect to CloudPods: %w", err)
	}

	return client, nil
}

func (c *Client) testConnection() error {
	ctx, cancel := context.WithTimeout(context.Background(), 10*time.Second)
	defer cancel()

	req, err := http.NewRequestWithContext(ctx, "GET", c.config.Endpoint+"/api/v1/status", nil)
	if err != nil {
		return err
	}

	c.setAuthHeaders(req)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return err
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return fmt.Errorf("CloudPods API returned status %d", resp.StatusCode)
	}

	return nil
}

func (c *Client) setAuthHeaders(req *http.Request) {
	req.Header.Set("Content-Type", "application/json")
	req.Header.Set("X-CloudPods-Username", c.config.Username)
	req.Header.Set("X-CloudPods-Password", c.config.Password)
	req.Header.Set("X-CloudPods-Region", c.config.Region)
	req.Header.Set("X-CloudPods-Project", c.config.Project)
}

func (c *Client) makeRequest(ctx context.Context, method, endpoint string, body interface{}) (*http.Response, error) {
	var reqBody io.Reader
	if body != nil {
		jsonData, err := json.Marshal(body)
		if err != nil {
			return nil, err
		}
		reqBody = bytes.NewBuffer(jsonData)
	}

	req, err := http.NewRequestWithContext(ctx, method, c.config.Endpoint+endpoint, reqBody)
	if err != nil {
		return nil, err
	}

	c.setAuthHeaders(req)

	return c.httpClient.Do(req)
}

// VPC Management
func (c *Client) ListVPCs(ctx context.Context) ([]VPC, error) {
	resp, err := c.makeRequest(ctx, "GET", "/api/v1/vpcs", nil)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("API returned status %d", resp.StatusCode)
	}

	var result struct {
		VPCs []VPC `json:"vpcs"`
	}

	if err := json.NewDecoder(resp.Body).Decode(&result); err != nil {
		return nil, err
	}

	return result.VPCs, nil
}

func (c *Client) CreateVPC(ctx context.Context, req CreateVPCRequest) (*VPC, error) {
	resp, err := c.makeRequest(ctx, "POST", "/api/v1/vpcs", req)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusCreated {
		return nil, fmt.Errorf("API returned status %d", resp.StatusCode)
	}

	var vpc VPC
	if err := json.NewDecoder(resp.Body).Decode(&vpc); err != nil {
		return nil, err
	}

	return &vpc, nil
}

func (c *Client) GetVPC(ctx context.Context, id string) (*VPC, error) {
	resp, err := c.makeRequest(ctx, "GET", "/api/v1/vpcs/"+id, nil)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("API returned status %d", resp.StatusCode)
	}

	var vpc VPC
	if err := json.NewDecoder(resp.Body).Decode(&vpc); err != nil {
		return nil, err
	}

	return &vpc, nil
}

func (c *Client) UpdateVPC(ctx context.Context, id string, req UpdateVPCRequest) (*VPC, error) {
	resp, err := c.makeRequest(ctx, "PUT", "/api/v1/vpcs/"+id, req)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("API returned status %d", resp.StatusCode)
	}

	var vpc VPC
	if err := json.NewDecoder(resp.Body).Decode(&vpc); err != nil {
		return nil, err
	}

	return &vpc, nil
}

func (c *Client) DeleteVPC(ctx context.Context, id string) error {
	resp, err := c.makeRequest(ctx, "DELETE", "/api/v1/vpcs/"+id, nil)
	if err != nil {
		return err
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusNoContent {
		return fmt.Errorf("API returned status %d", resp.StatusCode)
	}

	return nil
}

func (c *Client) GetVPCStats(ctx context.Context, id string) (*VPCStats, error) {
	resp, err := c.makeRequest(ctx, "GET", "/api/v1/vpcs/"+id+"/stats", nil)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("API returned status %d", resp.StatusCode)
	}

	var stats VPCStats
	if err := json.NewDecoder(resp.Body).Decode(&stats); err != nil {
		return nil, err
	}

	return &stats, nil
}

// Subnet Management
func (c *Client) ListSubnets(ctx context.Context, vpcID string) ([]Subnet, error) {
	resp, err := c.makeRequest(ctx, "GET", "/api/v1/vpcs/"+vpcID+"/subnets", nil)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("API returned status %d", resp.StatusCode)
	}

	var result struct {
		Subnets []Subnet `json:"subnets"`
	}

	if err := json.NewDecoder(resp.Body).Decode(&result); err != nil {
		return nil, err
	}

	return result.Subnets, nil
}

func (c *Client) CreateSubnet(ctx context.Context, vpcID string, req CreateSubnetRequest) (*Subnet, error) {
	resp, err := c.makeRequest(ctx, "POST", "/api/v1/vpcs/"+vpcID+"/subnets", req)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusCreated {
		return nil, fmt.Errorf("API returned status %d", resp.StatusCode)
	}

	var subnet Subnet
	if err := json.NewDecoder(resp.Body).Decode(&subnet); err != nil {
		return nil, err
	}

	return &subnet, nil
}

func (c *Client) GetSubnet(ctx context.Context, vpcID, subnetID string) (*Subnet, error) {
	resp, err := c.makeRequest(ctx, "GET", "/api/v1/vpcs/"+vpcID+"/subnets/"+subnetID, nil)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("API returned status %d", resp.StatusCode)
	}

	var subnet Subnet
	if err := json.NewDecoder(resp.Body).Decode(&subnet); err != nil {
		return nil, err
	}

	return &subnet, nil
}

func (c *Client) UpdateSubnet(ctx context.Context, vpcID, subnetID string, req UpdateSubnetRequest) (*Subnet, error) {
	resp, err := c.makeRequest(ctx, "PUT", "/api/v1/vpcs/"+vpcID+"/subnets/"+subnetID, req)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("API returned status %d", resp.StatusCode)
	}

	var subnet Subnet
	if err := json.NewDecoder(resp.Body).Decode(&subnet); err != nil {
		return nil, err
	}

	return &subnet, nil
}

func (c *Client) DeleteSubnet(ctx context.Context, vpcID, subnetID string) error {
	resp, err := c.makeRequest(ctx, "DELETE", "/api/v1/vpcs/"+vpcID+"/subnets/"+subnetID, nil)
	if err != nil {
		return err
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusNoContent {
		return fmt.Errorf("API returned status %d", resp.StatusCode)
	}

	return nil
}

// NAT Gateway Management
func (c *Client) ListNATGateways(ctx context.Context, vpcID string) ([]NATGateway, error) {
	resp, err := c.makeRequest(ctx, "GET", "/api/v1/vpcs/"+vpcID+"/nat-gateways", nil)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("API returned status %d", resp.StatusCode)
	}

	var result struct {
		NATGateways []NATGateway `json:"nat_gateways"`
	}

	if err := json.NewDecoder(resp.Body).Decode(&result); err != nil {
		return nil, err
	}

	return result.NATGateways, nil
}

func (c *Client) CreateNATGateway(ctx context.Context, vpcID string, req CreateNATGatewayRequest) (*NATGateway, error) {
	resp, err := c.makeRequest(ctx, "POST", "/api/v1/vpcs/"+vpcID+"/nat-gateways", req)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusCreated {
		return nil, fmt.Errorf("API returned status %d", resp.StatusCode)
	}

	var nat NATGateway
	if err := json.NewDecoder(resp.Body).Decode(&nat); err != nil {
		return nil, err
	}

	return &nat, nil
}

func (c *Client) GetNATGateway(ctx context.Context, vpcID, natID string) (*NATGateway, error) {
	resp, err := c.makeRequest(ctx, "GET", "/api/v1/vpcs/"+vpcID+"/nat-gateways/"+natID, nil)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("API returned status %d", resp.StatusCode)
	}

	var nat NATGateway
	if err := json.NewDecoder(resp.Body).Decode(&nat); err != nil {
		return nil, err
	}

	return &nat, nil
}

func (c *Client) DeleteNATGateway(ctx context.Context, vpcID, natID string) error {
	resp, err := c.makeRequest(ctx, "DELETE", "/api/v1/vpcs/"+vpcID+"/nat-gateways/"+natID, nil)
	if err != nil {
		return err
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusNoContent {
		return fmt.Errorf("API returned status %d", resp.StatusCode)
	}

	return nil
}

// Load Balancer Management
func (c *Client) ListLoadBalancers(ctx context.Context, vpcID string) ([]LoadBalancer, error) {
	resp, err := c.makeRequest(ctx, "GET", "/api/v1/vpcs/"+vpcID+"/load-balancers", nil)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("API returned status %d", resp.StatusCode)
	}

	var result struct {
		LoadBalancers []LoadBalancer `json:"load_balancers"`
	}

	if err := json.NewDecoder(resp.Body).Decode(&result); err != nil {
		return nil, err
	}

	return result.LoadBalancers, nil
}

func (c *Client) CreateLoadBalancer(ctx context.Context, vpcID string, req CreateLoadBalancerRequest) (*LoadBalancer, error) {
	resp, err := c.makeRequest(ctx, "POST", "/api/v1/vpcs/"+vpcID+"/load-balancers", req)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusCreated {
		return nil, fmt.Errorf("API returned status %d", resp.StatusCode)
	}

	var lb LoadBalancer
	if err := json.NewDecoder(resp.Body).Decode(&lb); err != nil {
		return nil, err
	}

	return &lb, nil
}

func (c *Client) GetLoadBalancer(ctx context.Context, vpcID, lbID string) (*LoadBalancer, error) {
	resp, err := c.makeRequest(ctx, "GET", "/api/v1/vpcs/"+vpcID+"/load-balancers/"+lbID, nil)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("API returned status %d", resp.StatusCode)
	}

	var lb LoadBalancer
	if err := json.NewDecoder(resp.Body).Decode(&lb); err != nil {
		return nil, err
	}

	return &lb, nil
}

func (c *Client) UpdateLoadBalancer(ctx context.Context, vpcID, lbID string, req UpdateLoadBalancerRequest) (*LoadBalancer, error) {
	resp, err := c.makeRequest(ctx, "PUT", "/api/v1/vpcs/"+vpcID+"/load-balancers/"+lbID, req)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("API returned status %d", resp.StatusCode)
	}

	var lb LoadBalancer
	if err := json.NewDecoder(resp.Body).Decode(&lb); err != nil {
		return nil, err
	}

	return &lb, nil
}

func (c *Client) DeleteLoadBalancer(ctx context.Context, vpcID, lbID string) error {
	resp, err := c.makeRequest(ctx, "DELETE", "/api/v1/vpcs/"+vpcID+"/load-balancers/"+lbID, nil)
	if err != nil {
		return err
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusNoContent {
		return fmt.Errorf("API returned status %d", resp.StatusCode)
	}

	return nil
}

// Service Mesh Management
func (c *Client) ListServiceMeshRoutes(ctx context.Context, vpcID string) ([]ServiceMeshRoute, error) {
	resp, err := c.makeRequest(ctx, "GET", "/api/v1/vpcs/"+vpcID+"/service-mesh", nil)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("API returned status %d", resp.StatusCode)
	}

	var result struct {
		Routes []ServiceMeshRoute `json:"routes"`
	}

	if err := json.NewDecoder(resp.Body).Decode(&result); err != nil {
		return nil, err
	}

	return result.Routes, nil
}

func (c *Client) CreateServiceMeshRoute(ctx context.Context, vpcID string, req CreateServiceMeshRouteRequest) (*ServiceMeshRoute, error) {
	resp, err := c.makeRequest(ctx, "POST", "/api/v1/vpcs/"+vpcID+"/service-mesh", req)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusCreated {
		return nil, fmt.Errorf("API returned status %d", resp.StatusCode)
	}

	var route ServiceMeshRoute
	if err := json.NewDecoder(resp.Body).Decode(&route); err != nil {
		return nil, err
	}

	return &route, nil
}

func (c *Client) GetServiceMeshRoute(ctx context.Context, vpcID, routeID string) (*ServiceMeshRoute, error) {
	resp, err := c.makeRequest(ctx, "GET", "/api/v1/vpcs/"+vpcID+"/service-mesh/"+routeID, nil)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("API returned status %d", resp.StatusCode)
	}

	var route ServiceMeshRoute
	if err := json.NewDecoder(resp.Body).Decode(&route); err != nil {
		return nil, err
	}

	return &route, nil
}

func (c *Client) UpdateServiceMeshRoute(ctx context.Context, vpcID, routeID string, req UpdateServiceMeshRouteRequest) (*ServiceMeshRoute, error) {
	resp, err := c.makeRequest(ctx, "PUT", "/api/v1/vpcs/"+vpcID+"/service-mesh/"+routeID, req)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("API returned status %d", resp.StatusCode)
	}

	var route ServiceMeshRoute
	if err := json.NewDecoder(resp.Body).Decode(&route); err != nil {
		return nil, err
	}

	return &route, nil
}

func (c *Client) DeleteServiceMeshRoute(ctx context.Context, vpcID, routeID string) error {
	resp, err := c.makeRequest(ctx, "DELETE", "/api/v1/vpcs/"+vpcID+"/service-mesh/"+routeID, nil)
	if err != nil {
		return err
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusNoContent {
		return fmt.Errorf("API returned status %d", resp.StatusCode)
	}

	return nil
}
