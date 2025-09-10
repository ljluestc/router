package aviatrix

import (
	"bytes"
	"context"
	"encoding/json"
	"fmt"
	"io"
	"net/http"
	"time"
)

// Client represents an Aviatrix API client
type Client struct {
	baseURL    string
	apiKey     string
	httpClient *http.Client
}

// Config represents Aviatrix configuration
type Config struct {
	BaseURL string `json:"base_url"`
	APIKey  string `json:"api_key"`
	Timeout int    `json:"timeout_seconds"`
}

// Gateway represents an Aviatrix gateway
type Gateway struct {
	ID           string            `json:"id"`
	Name         string            `json:"name"`
	Type         string            `json:"type"`
	CloudType    string            `json:"cloud_type"`
	Region       string            `json:"region"`
	VPC          string            `json:"vpc"`
	Subnet       string            `json:"subnet"`
	PublicIP     string            `json:"public_ip"`
	PrivateIP    string            `json:"private_ip"`
	State        string            `json:"state"`
	Status       string            `json:"status"`
	CreatedAt    time.Time         `json:"created_at"`
	UpdatedAt    time.Time         `json:"updated_at"`
	Properties   map[string]string `json:"properties"`
	Tags         map[string]string `json:"tags"`
}

// TransitGateway represents an Aviatrix transit gateway
type TransitGateway struct {
	ID           string   `json:"id"`
	Name         string   `json:"name"`
	CloudType    string   `json:"cloud_type"`
	Region       string   `json:"region"`
	VPC          string   `json:"vpc"`
	Subnet       string   `json:"subnet"`
	PublicIP     string   `json:"public_ip"`
	PrivateIP    string   `json:"private_ip"`
	State        string   `json:"state"`
	Status       string   `json:"status"`
	ConnectedGWs []string `json:"connected_gateways"`
	SpokeGWs     []string `json:"spoke_gateways"`
}

// SpokeGateway represents an Aviatrix spoke gateway
type SpokeGateway struct {
	ID           string   `json:"id"`
	Name         string   `json:"name"`
	CloudType    string   `json:"cloud_type"`
	Region       string   `json:"region"`
	VPC          string   `json:"vpc"`
	Subnet       string   `json:"subnet"`
	PublicIP     string   `json:"public_ip"`
	PrivateIP    string   `json:"private_ip"`
	State        string   `json:"state"`
	Status       string   `json:"status"`
	TransitGW    string   `json:"transit_gateway"`
	AttachedGWs  []string `json:"attached_gateways"`
}

// VPNGateway represents an Aviatrix VPN gateway
type VPNGateway struct {
	ID           string   `json:"id"`
	Name         string   `json:"name"`
	Type         string   `json:"type"`
	CloudType    string   `json:"cloud_type"`
	Region       string   `json:"region"`
	VPC          string   `json:"vpc"`
	Subnet       string   `json:"subnet"`
	PublicIP     string   `json:"public_ip"`
	PrivateIP    string   `json:"private_ip"`
	State        string   `json:"state"`
	Status       string   `json:"status"`
	Users        []string `json:"users"`
	Connections  []string `json:"connections"`
}

// Route represents an Aviatrix route
type Route struct {
	ID          string `json:"id"`
	Destination string `json:"destination"`
	NextHop     string `json:"next_hop"`
	Gateway     string `json:"gateway"`
	Protocol    string `json:"protocol"`
	Metric      int    `json:"metric"`
	Status      string `json:"status"`
}

// BGPNeighbor represents a BGP neighbor
type BGPNeighbor struct {
	ID           string `json:"id"`
	Gateway      string `json:"gateway"`
	NeighborIP   string `json:"neighbor_ip"`
	ASNumber     int    `json:"as_number"`
	State        string `json:"state"`
	Status       string `json:"status"`
	Uptime       int    `json:"uptime"`
	MessagesSent int    `json:"messages_sent"`
	MessagesRcvd int    `json:"messages_received"`
}

// APIResponse represents a generic API response
type APIResponse struct {
	Success bool        `json:"success"`
	Data    interface{} `json:"data,omitempty"`
	Error   string      `json:"error,omitempty"`
	Message string      `json:"message,omitempty"`
}

// NewClient creates a new Aviatrix client
func NewClient(config Config) (*Client, error) {
	if config.BaseURL == "" {
		return nil, fmt.Errorf("base URL is required")
	}

	if config.APIKey == "" {
		return nil, fmt.Errorf("API key is required")
	}

	timeout := time.Duration(config.Timeout) * time.Second
	if timeout == 0 {
		timeout = 30 * time.Second
	}

	return &Client{
		baseURL: config.BaseURL,
		apiKey:  config.APIKey,
		httpClient: &http.Client{
			Timeout: timeout,
		},
	}, nil
}

// GetStatus returns the Aviatrix service status
func (c *Client) GetStatus(ctx context.Context) (*APIResponse, error) {
	return c.makeRequest(ctx, "GET", "/status", nil)
}

// ListGateways returns all gateways
func (c *Client) ListGateways(ctx context.Context) ([]Gateway, error) {
	resp, err := c.makeRequest(ctx, "GET", "/gateways", nil)
	if err != nil {
		return nil, err
	}

	var gateways []Gateway
	if data, ok := resp.Data.([]interface{}); ok {
		for _, item := range data {
			if gatewayData, err := json.Marshal(item); err == nil {
				var gateway Gateway
				if err := json.Unmarshal(gatewayData, &gateway); err == nil {
					gateways = append(gateways, gateway)
				}
			}
		}
	}

	return gateways, nil
}

// GetGateway returns a specific gateway by ID
func (c *Client) GetGateway(ctx context.Context, id string) (*Gateway, error) {
	resp, err := c.makeRequest(ctx, "GET", fmt.Sprintf("/gateways/%s", id), nil)
	if err != nil {
		return nil, err
	}

	var gateway Gateway
	if data, ok := resp.Data.(map[string]interface{}); ok {
		if gatewayData, err := json.Marshal(data); err == nil {
			if err := json.Unmarshal(gatewayData, &gateway); err != nil {
				return nil, err
			}
		}
	}

	return &gateway, nil
}

// CreateTransitGateway creates a new transit gateway
func (c *Client) CreateTransitGateway(ctx context.Context, tgw *TransitGateway) (*APIResponse, error) {
	data, err := json.Marshal(tgw)
	if err != nil {
		return nil, err
	}

	return c.makeRequest(ctx, "POST", "/transit-gateways", bytes.NewBuffer(data))
}

// ListTransitGateways returns all transit gateways
func (c *Client) ListTransitGateways(ctx context.Context) ([]TransitGateway, error) {
	resp, err := c.makeRequest(ctx, "GET", "/transit-gateways", nil)
	if err != nil {
		return nil, err
	}

	var tgws []TransitGateway
	if data, ok := resp.Data.([]interface{}); ok {
		for _, item := range data {
			if tgwData, err := json.Marshal(item); err == nil {
				var tgw TransitGateway
				if err := json.Unmarshal(tgwData, &tgw); err == nil {
					tgws = append(tgws, tgw)
				}
			}
		}
	}

	return tgws, nil
}

// CreateSpokeGateway creates a new spoke gateway
func (c *Client) CreateSpokeGateway(ctx context.Context, sgw *SpokeGateway) (*APIResponse, error) {
	data, err := json.Marshal(sgw)
	if err != nil {
		return nil, err
	}

	return c.makeRequest(ctx, "POST", "/spoke-gateways", bytes.NewBuffer(data))
}

// ListSpokeGateways returns all spoke gateways
func (c *Client) ListSpokeGateways(ctx context.Context) ([]SpokeGateway, error) {
	resp, err := c.makeRequest(ctx, "GET", "/spoke-gateways", nil)
	if err != nil {
		return nil, err
	}

	var sgws []SpokeGateway
	if data, ok := resp.Data.([]interface{}); ok {
		for _, item := range data {
			if sgwData, err := json.Marshal(item); err == nil {
				var sgw SpokeGateway
				if err := json.Unmarshal(sgwData, &sgw); err == nil {
					sgws = append(sgws, sgw)
				}
			}
		}
	}

	return sgws, nil
}

// CreateVPNGateway creates a new VPN gateway
func (c *Client) CreateVPNGateway(ctx context.Context, vgw *VPNGateway) (*APIResponse, error) {
	data, err := json.Marshal(vgw)
	if err != nil {
		return nil, err
	}

	return c.makeRequest(ctx, "POST", "/vpn-gateways", bytes.NewBuffer(data))
}

// ListVPNGateways returns all VPN gateways
func (c *Client) ListVPNGateways(ctx context.Context) ([]VPNGateway, error) {
	resp, err := c.makeRequest(ctx, "GET", "/vpn-gateways", nil)
	if err != nil {
		return nil, err
	}

	var vgws []VPNGateway
	if data, ok := resp.Data.([]interface{}); ok {
		for _, item := range data {
			if vgwData, err := json.Marshal(item); err == nil {
				var vgw VPNGateway
				if err := json.Unmarshal(vgwData, &vgw); err == nil {
					vgws = append(vgws, vgw)
				}
			}
		}
	}

	return vgws, nil
}

// ListRoutes returns all routes
func (c *Client) ListRoutes(ctx context.Context) ([]Route, error) {
	resp, err := c.makeRequest(ctx, "GET", "/routes", nil)
	if err != nil {
		return nil, err
	}

	var routes []Route
	if data, ok := resp.Data.([]interface{}); ok {
		for _, item := range data {
			if routeData, err := json.Marshal(item); err == nil {
				var route Route
				if err := json.Unmarshal(routeData, &route); err == nil {
					routes = append(routes, route)
				}
			}
		}
	}

	return routes, nil
}

// ListBGPNeighbors returns all BGP neighbors
func (c *Client) ListBGPNeighbors(ctx context.Context) ([]BGPNeighbor, error) {
	resp, err := c.makeRequest(ctx, "GET", "/bgp/neighbors", nil)
	if err != nil {
		return nil, err
	}

	var neighbors []BGPNeighbor
	if data, ok := resp.Data.([]interface{}); ok {
		for _, item := range data {
			if neighborData, err := json.Marshal(item); err == nil {
				var neighbor BGPNeighbor
				if err := json.Unmarshal(neighborData, &neighbor); err == nil {
					neighbors = append(neighbors, neighbor)
				}
			}
		}
	}

	return neighbors, nil
}

// DeployTransitNetwork deploys a complete transit network
func (c *Client) DeployTransitNetwork(ctx context.Context, config map[string]interface{}) (*APIResponse, error) {
	data, err := json.Marshal(config)
	if err != nil {
		return nil, err
	}

	return c.makeRequest(ctx, "POST", "/deploy/transit-network", bytes.NewBuffer(data))
}

// DeploySpokeNetwork deploys a spoke network
func (c *Client) DeploySpokeNetwork(ctx context.Context, config map[string]interface{}) (*APIResponse, error) {
	data, err := json.Marshal(config)
	if err != nil {
		return nil, err
	}

	return c.makeRequest(ctx, "POST", "/deploy/spoke-network", bytes.NewBuffer(data))
}

// CleanupNetwork removes all network resources
func (c *Client) CleanupNetwork(ctx context.Context) (*APIResponse, error) {
	return c.makeRequest(ctx, "DELETE", "/cleanup", nil)
}

// GetMetrics returns Aviatrix metrics
func (c *Client) GetMetrics(ctx context.Context, gatewayID string) (map[string]interface{}, error) {
	url := "/metrics"
	if gatewayID != "" {
		url = fmt.Sprintf("/metrics?gateway=%s", gatewayID)
	}

	resp, err := c.makeRequest(ctx, "GET", url, nil)
	if err != nil {
		return nil, err
	}

	if metrics, ok := resp.Data.(map[string]interface{}); ok {
		return metrics, nil
	}

	return nil, fmt.Errorf("invalid metrics response format")
}

// makeRequest makes an HTTP request to the Aviatrix API
func (c *Client) makeRequest(ctx context.Context, method, path string, body io.Reader) (*APIResponse, error) {
	url := c.baseURL + path
	
	req, err := http.NewRequestWithContext(ctx, method, url, body)
	if err != nil {
		return nil, err
	}

	// Set headers
	req.Header.Set("Content-Type", "application/json")
	req.Header.Set("Authorization", "Bearer "+c.apiKey)
	req.Header.Set("User-Agent", "router-sim-aviatrix-client/1.0")

	// Make request
	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()

	// Read response body
	respBody, err := io.ReadAll(resp.Body)
	if err != nil {
		return nil, err
	}

	// Parse response
	var apiResp APIResponse
	if err := json.Unmarshal(respBody, &apiResp); err != nil {
		return nil, fmt.Errorf("failed to parse response: %v", err)
	}

	// Check for HTTP errors
	if resp.StatusCode >= 400 {
		return &apiResp, fmt.Errorf("API error: %s (status: %d)", apiResp.Error, resp.StatusCode)
	}

	return &apiResp, nil
}

// HealthCheck performs a health check on the Aviatrix service
func (c *Client) HealthCheck(ctx context.Context) error {
	_, err := c.GetStatus(ctx)
	return err
}

// GetGatewayStatus returns the status of a specific gateway
func (c *Client) GetGatewayStatus(ctx context.Context, gatewayID string) (map[string]interface{}, error) {
	resp, err := c.makeRequest(ctx, "GET", fmt.Sprintf("/gateways/%s/status", gatewayID), nil)
	if err != nil {
		return nil, err
	}

	if status, ok := resp.Data.(map[string]interface{}); ok {
		return status, nil
	}

	return nil, fmt.Errorf("invalid status response format")
}

// UpdateGateway updates a gateway configuration
func (c *Client) UpdateGateway(ctx context.Context, gatewayID string, updates map[string]interface{}) (*APIResponse, error) {
	data, err := json.Marshal(updates)
	if err != nil {
		return nil, err
	}

	return c.makeRequest(ctx, "PUT", fmt.Sprintf("/gateways/%s", gatewayID), bytes.NewBuffer(data))
}

// DeleteGateway deletes a gateway
func (c *Client) DeleteGateway(ctx context.Context, gatewayID string) (*APIResponse, error) {
	return c.makeRequest(ctx, "DELETE", fmt.Sprintf("/gateways/%s", gatewayID), nil)
}

// GetNetworkTopology returns the network topology
func (c *Client) GetNetworkTopology(ctx context.Context) (map[string]interface{}, error) {
	resp, err := c.makeRequest(ctx, "GET", "/topology", nil)
	if err != nil {
		return nil, err
	}

	if topology, ok := resp.Data.(map[string]interface{}); ok {
		return topology, nil
	}

	return nil, fmt.Errorf("invalid topology response format")
}