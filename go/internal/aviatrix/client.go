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

// AviatrixClient represents a client for Aviatrix API
type AviatrixClient struct {
	baseURL    string
	apiKey     string
	httpClient *http.Client
}

// AviatrixConfig represents Aviatrix configuration
type AviatrixConfig struct {
	BaseURL string `json:"base_url"`
	APIKey  string `json:"api_key"`
	Timeout int    `json:"timeout_seconds"`
}

// AviatrixGateway represents an Aviatrix gateway
type AviatrixGateway struct {
	ID          string                 `json:"id"`
	Name        string                 `json:"name"`
	Type        string                 `json:"type"`
	Status      string                 `json:"status"`
	Region      string                 `json:"region"`
	CloudType   string                 `json:"cloud_type"`
	VPCID       string                 `json:"vpc_id"`
	SubnetID    string                 `json:"subnet_id"`
	PublicIP    string                 `json:"public_ip"`
	PrivateIP   string                 `json:"private_ip"`
	InstanceID  string                 `json:"instance_id"`
	CreatedAt   time.Time              `json:"created_at"`
	UpdatedAt   time.Time              `json:"updated_at"`
	Properties  map[string]interface{} `json:"properties"`
	Tags        map[string]string      `json:"tags"`
}

// AviatrixTransitGateway represents an Aviatrix transit gateway
type AviatrixTransitGateway struct {
	ID          string                 `json:"id"`
	Name        string                 `json:"name"`
	Status      string                 `json:"status"`
	Region      string                 `json:"region"`
	CloudType   string                 `json:"cloud_type"`
	VPCID       string                 `json:"vpc_id"`
	SubnetID    string                 `json:"subnet_id"`
	PublicIP    string                 `json:"public_ip"`
	PrivateIP   string                 `json:"private_ip"`
	InstanceID  string                 `json:"instance_id"`
	ASNumber    int                    `json:"as_number"`
	BGPEnabled  bool                   `json:"bgp_enabled"`
	CreatedAt   time.Time              `json:"created_at"`
	UpdatedAt   time.Time              `json:"updated_at"`
	Properties  map[string]interface{} `json:"properties"`
}

// AviatrixSpokeGateway represents an Aviatrix spoke gateway
type AviatrixSpokeGateway struct {
	ID          string                 `json:"id"`
	Name        string                 `json:"name"`
	Status      string                 `json:"status"`
	Region      string                 `json:"region"`
	CloudType   string                 `json:"cloud_type"`
	VPCID       string                 `json:"vpc_id"`
	SubnetID    string                 `json:"subnet_id"`
	PublicIP    string                 `json:"public_ip"`
	PrivateIP   string                 `json:"private_ip"`
	InstanceID  string                 `json:"instance_id"`
	TransitGW   string                 `json:"transit_gateway"`
	CreatedAt   time.Time              `json:"created_at"`
	UpdatedAt   time.Time              `json:"updated_at"`
	Properties  map[string]interface{} `json:"properties"`
}

// AviatrixVPNGateway represents an Aviatrix VPN gateway
type AviatrixVPNGateway struct {
	ID          string                 `json:"id"`
	Name        string                 `json:"name"`
	Status      string                 `json:"status"`
	Region      string                 `json:"region"`
	CloudType   string                 `json:"cloud_type"`
	VPCID       string                 `json:"vpc_id"`
	SubnetID    string                 `json:"subnet_id"`
	PublicIP    string                 `json:"public_ip"`
	PrivateIP   string                 `json:"private_ip"`
	InstanceID  string                 `json:"instance_id"`
	VPNType     string                 `json:"vpn_type"`
	CreatedAt   time.Time              `json:"created_at"`
	UpdatedAt   time.Time              `json:"updated_at"`
	Properties  map[string]interface{} `json:"properties"`
}

// AviatrixConnection represents an Aviatrix connection
type AviatrixConnection struct {
	ID          string                 `json:"id"`
	Name        string                 `json:"name"`
	Type        string                 `json:"type"`
	Status      string                 `json:"status"`
	SourceGW    string                 `json:"source_gateway"`
	DestGW      string                 `json:"dest_gateway"`
	Protocol    string                 `json:"protocol"`
	Port        int                    `json:"port"`
	CreatedAt   time.Time              `json:"created_at"`
	UpdatedAt   time.Time              `json:"updated_at"`
	Properties  map[string]interface{} `json:"properties"`
}

// AviatrixRoute represents an Aviatrix route
type AviatrixRoute struct {
	ID          string    `json:"id"`
	Destination string    `json:"destination"`
	Gateway     string    `json:"gateway"`
	Interface   string    `json:"interface"`
	Metric      int       `json:"metric"`
	Protocol    string    `json:"protocol"`
	Status      string    `json:"status"`
	CreatedAt   time.Time `json:"created_at"`
	UpdatedAt   time.Time `json:"updated_at"`
}

// AviatrixAPIResponse represents a generic API response
type AviatrixAPIResponse struct {
	Success bool                   `json:"success"`
	Message string                 `json:"message"`
	Data    interface{}            `json:"data"`
	Error   string                 `json:"error"`
	Meta    map[string]interface{} `json:"meta"`
}

// NewAviatrixClient creates a new Aviatrix client
func NewAviatrixClient(config AviatrixConfig) *AviatrixClient {
	timeout := time.Duration(config.Timeout) * time.Second
	if timeout == 0 {
		timeout = 30 * time.Second
	}

	return &AviatrixClient{
		baseURL: config.BaseURL,
		apiKey:  config.APIKey,
		httpClient: &http.Client{
			Timeout: timeout,
		},
	}
}

// GetGateways retrieves all gateways
func (c *AviatrixClient) GetGateways(ctx context.Context) ([]AviatrixGateway, error) {
	url := fmt.Sprintf("%s/api/v1/gateways", c.baseURL)
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

	var apiResp AviatrixAPIResponse
	if err := json.Unmarshal(body, &apiResp); err != nil {
		return nil, fmt.Errorf("failed to unmarshal response: %w", err)
	}

	if !apiResp.Success {
		return nil, fmt.Errorf("API error: %s", apiResp.Error)
	}

	// Convert data to gateways
	gatewaysData, ok := apiResp.Data.([]interface{})
	if !ok {
		return nil, fmt.Errorf("invalid response data format")
	}

	var gateways []AviatrixGateway
	for _, item := range gatewaysData {
		itemBytes, err := json.Marshal(item)
		if err != nil {
			continue
		}

		var gateway AviatrixGateway
		if err := json.Unmarshal(itemBytes, &gateway); err != nil {
			continue
		}

		gateways = append(gateways, gateway)
	}

	return gateways, nil
}

// GetTransitGateways retrieves all transit gateways
func (c *AviatrixClient) GetTransitGateways(ctx context.Context) ([]AviatrixTransitGateway, error) {
	url := fmt.Sprintf("%s/api/v1/transit-gateways", c.baseURL)
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

	var apiResp AviatrixAPIResponse
	if err := json.Unmarshal(body, &apiResp); err != nil {
		return nil, fmt.Errorf("failed to unmarshal response: %w", err)
	}

	if !apiResp.Success {
		return nil, fmt.Errorf("API error: %s", apiResp.Error)
	}

	// Convert data to transit gateways
	tgData, ok := apiResp.Data.([]interface{})
	if !ok {
		return nil, fmt.Errorf("invalid response data format")
	}

	var transitGateways []AviatrixTransitGateway
	for _, item := range tgData {
		itemBytes, err := json.Marshal(item)
		if err != nil {
			continue
		}

		var tg AviatrixTransitGateway
		if err := json.Unmarshal(itemBytes, &tg); err != nil {
			continue
		}

		transitGateways = append(transitGateways, tg)
	}

	return transitGateways, nil
}

// GetSpokeGateways retrieves all spoke gateways
func (c *AviatrixClient) GetSpokeGateways(ctx context.Context) ([]AviatrixSpokeGateway, error) {
	url := fmt.Sprintf("%s/api/v1/spoke-gateways", c.baseURL)
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

	var apiResp AviatrixAPIResponse
	if err := json.Unmarshal(body, &apiResp); err != nil {
		return nil, fmt.Errorf("failed to unmarshal response: %w", err)
	}

	if !apiResp.Success {
		return nil, fmt.Errorf("API error: %s", apiResp.Error)
	}

	// Convert data to spoke gateways
	sgData, ok := apiResp.Data.([]interface{})
	if !ok {
		return nil, fmt.Errorf("invalid response data format")
	}

	var spokeGateways []AviatrixSpokeGateway
	for _, item := range sgData {
		itemBytes, err := json.Marshal(item)
		if err != nil {
			continue
		}

		var sg AviatrixSpokeGateway
		if err := json.Unmarshal(itemBytes, &sg); err != nil {
			continue
		}

		spokeGateways = append(spokeGateways, sg)
	}

	return spokeGateways, nil
}

// GetVPNGateways retrieves all VPN gateways
func (c *AviatrixClient) GetVPNGateways(ctx context.Context) ([]AviatrixVPNGateway, error) {
	url := fmt.Sprintf("%s/api/v1/vpn-gateways", c.baseURL)
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

	var apiResp AviatrixAPIResponse
	if err := json.Unmarshal(body, &apiResp); err != nil {
		return nil, fmt.Errorf("failed to unmarshal response: %w", err)
	}

	if !apiResp.Success {
		return nil, fmt.Errorf("API error: %s", apiResp.Error)
	}

	// Convert data to VPN gateways
	vgData, ok := apiResp.Data.([]interface{})
	if !ok {
		return nil, fmt.Errorf("invalid response data format")
	}

	var vpnGateways []AviatrixVPNGateway
	for _, item := range vgData {
		itemBytes, err := json.Marshal(item)
		if err != nil {
			continue
		}

		var vg AviatrixVPNGateway
		if err := json.Unmarshal(itemBytes, &vg); err != nil {
			continue
		}

		vpnGateways = append(vpnGateways, vg)
	}

	return vpnGateways, nil
}

// GetConnections retrieves all connections
func (c *AviatrixClient) GetConnections(ctx context.Context) ([]AviatrixConnection, error) {
	url := fmt.Sprintf("%s/api/v1/connections", c.baseURL)
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

	var apiResp AviatrixAPIResponse
	if err := json.Unmarshal(body, &apiResp); err != nil {
		return nil, fmt.Errorf("failed to unmarshal response: %w", err)
	}

	if !apiResp.Success {
		return nil, fmt.Errorf("API error: %s", apiResp.Error)
	}

	// Convert data to connections
	connData, ok := apiResp.Data.([]interface{})
	if !ok {
		return nil, fmt.Errorf("invalid response data format")
	}

	var connections []AviatrixConnection
	for _, item := range connData {
		itemBytes, err := json.Marshal(item)
		if err != nil {
			continue
		}

		var conn AviatrixConnection
		if err := json.Unmarshal(itemBytes, &conn); err != nil {
			continue
		}

		connections = append(connections, conn)
	}

	return connections, nil
}

// GetRoutes retrieves all routes
func (c *AviatrixClient) GetRoutes(ctx context.Context) ([]AviatrixRoute, error) {
	url := fmt.Sprintf("%s/api/v1/routes", c.baseURL)
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

	var apiResp AviatrixAPIResponse
	if err := json.Unmarshal(body, &apiResp); err != nil {
		return nil, fmt.Errorf("failed to unmarshal response: %w", err)
	}

	if !apiResp.Success {
		return nil, fmt.Errorf("API error: %s", apiResp.Error)
	}

	// Convert data to routes
	routesData, ok := apiResp.Data.([]interface{})
	if !ok {
		return nil, fmt.Errorf("invalid response data format")
	}

	var routes []AviatrixRoute
	for _, item := range routesData {
		itemBytes, err := json.Marshal(item)
		if err != nil {
			continue
		}

		var route AviatrixRoute
		if err := json.Unmarshal(itemBytes, &route); err != nil {
			continue
		}

		routes = append(routes, route)
	}

	return routes, nil
}

// CreateConnection creates a new connection
func (c *AviatrixClient) CreateConnection(ctx context.Context, connection AviatrixConnection) (*AviatrixConnection, error) {
	url := fmt.Sprintf("%s/api/v1/connections", c.baseURL)
	
	connData, err := json.Marshal(connection)
	if err != nil {
		return nil, fmt.Errorf("failed to marshal connection: %w", err)
	}

	req, err := http.NewRequestWithContext(ctx, "POST", url, bytes.NewReader(connData))
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

	var apiResp AviatrixAPIResponse
	if err := json.Unmarshal(body, &apiResp); err != nil {
		return nil, fmt.Errorf("failed to unmarshal response: %w", err)
	}

	if !apiResp.Success {
		return nil, fmt.Errorf("API error: %s", apiResp.Error)
	}

	// Convert data to connection
	connData, err = json.Marshal(apiResp.Data)
	if err != nil {
		return nil, fmt.Errorf("failed to marshal connection data: %w", err)
	}

	var createdConn AviatrixConnection
	if err := json.Unmarshal(connData, &createdConn); err != nil {
		return nil, fmt.Errorf("failed to unmarshal connection: %w", err)
	}

	return &createdConn, nil
}

// HealthCheck performs a health check on the Aviatrix API
func (c *AviatrixClient) HealthCheck(ctx context.Context) error {
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
