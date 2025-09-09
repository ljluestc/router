package aviatrix

import (
	"bytes"
	"context"
	"encoding/json"
	"fmt"
	"net/http"
	"time"

	"router-sim/internal/config"
)

// Enhanced Aviatrix client with comprehensive gateway and network management
type EnhancedClient struct {
	config     *config.AviatrixConfig
	httpClient *http.Client
	baseURL    string
	apiKey     string
	username   string
	password   string
}

// Aviatrix gateway types
type Gateway struct {
	ID           string            `json:"id"`
	Name         string            `json:"name"`
	Type         string            `json:"type"` // transit, spoke, vpn, etc.
	Cloud        string            `json:"cloud"` // aws, azure, gcp, oci
	Region       string            `json:"region"`
	Status       string            `json:"status"`
	PublicIP     string            `json:"public_ip"`
	PrivateIP    string            `json:"private_ip"`
	InstanceSize string            `json:"instance_size"`
	VPC          string            `json:"vpc"`
	Subnet       string            `json:"subnet"`
	ASN          int               `json:"asn"`
	CreatedAt    time.Time         `json:"created_at"`
	UpdatedAt    time.Time         `json:"updated_at"`
	Tags         map[string]string `json:"tags"`
	Properties   map[string]interface{} `json:"properties"`
}

type TransitGateway struct {
	Gateway
	EnableActiveMesh bool     `json:"enable_active_mesh"`
	EnableSegmentation bool   `json:"enable_segmentation"`
	ConnectedTransitGWs []string `json:"connected_transit_gws"`
	ConnectedSpokeGWs  []string  `json:"connected_spoke_gws"`
	BGPEnabled        bool      `json:"bgp_enabled"`
	BGPASN            int       `json:"bgp_asn"`
	BGPNeighbors      []BGPNeighbor `json:"bgp_neighbors"`
}

type SpokeGateway struct {
	Gateway
	TransitGateway string   `json:"transit_gateway"`
	BGPEnabled     bool     `json:"bgp_enabled"`
	BGPASN         int      `json:"bgp_asn"`
	BGPNeighbors   []BGPNeighbor `json:"bgp_neighbors"`
	LearnedCIDRs   []string `json:"learned_cidrs"`
	AdvertisedCIDRs []string `json:"advertised_cidrs"`
}

type VPNGateway struct {
	Gateway
	VPNType        string   `json:"vpn_type"` // user, geo
	VPNProtocol    string   `json:"vpn_protocol"` // openvpn, ipsec
	VPNPort        int      `json:"vpn_port"`
	VPNUsers       []VPNUser `json:"vpn_users"`
	AllowedCIDRs   []string `json:"allowed_cidrs"`
	SplitTunnel    bool     `json:"split_tunnel"`
	EnableDualStack bool    `json:"enable_dual_stack"`
}

type BGPNeighbor struct {
	ID          string `json:"id"`
	IP          string `json:"ip"`
	ASN         int    `json:"asn"`
	Status      string `json:"status"`
	Uptime      int    `json:"uptime"`
	MessagesSent int   `json:"messages_sent"`
	MessagesReceived int `json:"messages_received"`
	RoutesReceived int  `json:"routes_received"`
	RoutesAdvertised int `json:"routes_advertised"`
}

type VPNUser struct {
	ID       string `json:"id"`
	Username string `json:"username"`
	Email    string `json:"email"`
	Status   string `json:"status"`
	CreatedAt time.Time `json:"created_at"`
	LastLogin time.Time `json:"last_login"`
}

// Network and connectivity
type TransitNetwork struct {
	ID          string `json:"id"`
	Name        string `json:"name"`
	Cloud       string `json:"cloud"`
	Region      string `json:"region"`
	CIDR        string `json:"cidr"`
	Gateway     string `json:"gateway"`
	Status      string `json:"status"`
	CreatedAt   time.Time `json:"created_at"`
	UpdatedAt   time.Time `json:"updated_at"`
}

type SpokeNetwork struct {
	ID          string `json:"id"`
	Name        string `json:"name"`
	Cloud       string `json:"cloud"`
	Region      string `json:"region"`
	CIDR        string `json:"cidr"`
	Gateway     string `json:"gateway"`
	TransitGW   string `json:"transit_gw"`
	Status      string `json:"status"`
	CreatedAt   time.Time `json:"created_at"`
	UpdatedAt   time.Time `json:"updated_at"`
}

type Connection struct {
	ID          string `json:"id"`
	Source      string `json:"source"`
	Destination string `json:"destination"`
	Type        string `json:"type"` // transit-to-transit, transit-to-spoke, etc.
	Status      string `json:"status"`
	Bandwidth   int    `json:"bandwidth"`
	Latency     int    `json:"latency"`
	CreatedAt   time.Time `json:"created_at"`
	UpdatedAt   time.Time `json:"updated_at"`
}

// Routing and policies
type Route struct {
	ID          string `json:"id"`
	Destination string `json:"destination"`
	NextHop     string `json:"next_hop"`
	Interface   string `json:"interface"`
	Protocol    string `json:"protocol"`
	Metric      int    `json:"metric"`
	Status      string `json:"status"`
	LearnedAt   time.Time `json:"learned_at"`
}

type RoutingPolicy struct {
	ID          string `json:"id"`
	Name        string `json:"name"`
	Description string `json:"description"`
	Rules       []RoutingRule `json:"rules"`
	Enabled     bool   `json:"enabled"`
	CreatedAt   time.Time `json:"created_at"`
	UpdatedAt   time.Time `json:"updated_at"`
}

type RoutingRule struct {
	ID          string `json:"id"`
	Priority    int    `json:"priority"`
	Match       MatchCriteria `json:"match"`
	Action      ActionCriteria `json:"action"`
	Enabled     bool   `json:"enabled"`
}

type MatchCriteria struct {
	SourceCIDR      string `json:"source_cidr"`
	DestinationCIDR string `json:"destination_cidr"`
	Protocol        string `json:"protocol"`
	Port            int    `json:"port"`
	ASPath          string `json:"as_path"`
	Community       string `json:"community"`
}

type ActionCriteria struct {
	Action      string `json:"action"` // permit, deny, modify
	NextHop     string `json:"next_hop"`
	LocalPref   int    `json:"local_pref"`
	MED         int    `json:"med"`
	Community   string `json:"community"`
	ASPath      string `json:"as_path"`
}

// Security and policies
type SecurityPolicy struct {
	ID          string `json:"id"`
	Name        string `json:"name"`
	Description string `json:"description"`
	Rules       []SecurityRule `json:"rules"`
	Enabled     bool   `json:"enabled"`
	CreatedAt   time.Time `json:"created_at"`
	UpdatedAt   time.Time `json:"updated_at"`
}

type SecurityRule struct {
	ID          string `json:"id"`
	Priority    int    `json:"priority"`
	Source      string `json:"source"`
	Destination string `json:"destination"`
	Protocol    string `json:"protocol"`
	Port        int    `json:"port"`
	Action      string `json:"action"` // allow, deny
	Enabled     bool   `json:"enabled"`
}

// Monitoring and analytics
type GatewayMetrics struct {
	GatewayID      string    `json:"gateway_id"`
	Timestamp      time.Time `json:"timestamp"`
	CPUUsage       float64   `json:"cpu_usage"`
	MemoryUsage    float64   `json:"memory_usage"`
	NetworkIn      int64     `json:"network_in"`
	NetworkOut     int64     `json:"network_out"`
	PacketsIn      int64     `json:"packets_in"`
	PacketsOut     int64     `json:"packets_out"`
	Latency        float64   `json:"latency"`
	Throughput     float64   `json:"throughput"`
	ActiveTunnels  int       `json:"active_tunnels"`
	BGPNeighbors   int       `json:"bgp_neighbors"`
	RoutesLearned  int       `json:"routes_learned"`
}

type NetworkTopology struct {
	Nodes []TopologyNode `json:"nodes"`
	Links []TopologyLink `json:"links"`
}

type TopologyNode struct {
	ID       string `json:"id"`
	Name     string `json:"name"`
	Type     string `json:"type"`
	Cloud    string `json:"cloud"`
	Region   string `json:"region"`
	Status   string `json:"status"`
	Position Position `json:"position"`
}

type TopologyLink struct {
	Source   string `json:"source"`
	Target   string `json:"target"`
	Type     string `json:"type"`
	Status   string `json:"status"`
	Bandwidth int   `json:"bandwidth"`
	Latency  int    `json:"latency"`
}

type Position struct {
	X float64 `json:"x"`
	Y float64 `json:"y"`
}

// API Response structures
type GatewayListResponse struct {
	Gateways []Gateway `json:"gateways"`
	Total    int       `json:"total"`
	Page     int       `json:"page"`
	PageSize int       `json:"page_size"`
}

type TransitGatewayListResponse struct {
	TransitGateways []TransitGateway `json:"transit_gateways"`
	Total           int              `json:"total"`
	Page            int              `json:"page"`
	PageSize        int              `json:"page_size"`
}

type SpokeGatewayListResponse struct {
	SpokeGateways []SpokeGateway `json:"spoke_gateways"`
	Total         int            `json:"total"`
	Page          int            `json:"page"`
	PageSize      int            `json:"page_size"`
}

type VPNGatewayListResponse struct {
	VPNGateways []VPNGateway `json:"vpn_gateways"`
	Total       int          `json:"total"`
	Page        int          `json:"page"`
	PageSize    int          `json:"page_size"`
}

type RouteListResponse struct {
	Routes []Route `json:"routes"`
	Total  int     `json:"total"`
	Page   int     `json:"page"`
	PageSize int   `json:"page_size"`
}

// Statistics
type AviatrixStats struct {
	TotalGateways       int `json:"total_gateways"`
	TransitGateways     int `json:"transit_gateways"`
	SpokeGateways       int `json:"spoke_gateways"`
	VPNGateways         int `json:"vpn_gateways"`
	ActiveGateways      int `json:"active_gateways"`
	TotalConnections    int `json:"total_connections"`
	ActiveConnections   int `json:"active_connections"`
	TotalRoutes         int `json:"total_routes"`
	LearnedRoutes       int `json:"learned_routes"`
	AdvertisedRoutes    int `json:"advertised_routes"`
	BGPNeighbors        int `json:"bgp_neighbors"`
	EstablishedBGP      int `json:"established_bgp"`
	VPNUsers            int `json:"vpn_users"`
	ActiveVPNUsers      int `json:"active_vpn_users"`
}

// NewEnhancedClient creates a new enhanced Aviatrix client
func NewEnhancedClient(config *config.AviatrixConfig) *EnhancedClient {
	return &EnhancedClient{
		config: config,
		httpClient: &http.Client{
			Timeout: config.Timeout,
		},
		baseURL:  config.BaseURL,
		apiKey:   config.APIKey,
		username: config.Username,
		password: config.Password,
	}
}

// Authentication
func (c *EnhancedClient) Authenticate(ctx context.Context) error {
	authData := map[string]string{
		"username": c.username,
		"password": c.password,
	}

	jsonData, err := json.Marshal(authData)
	if err != nil {
		return fmt.Errorf("failed to marshal auth data: %w", err)
	}

	req, err := http.NewRequestWithContext(ctx, "POST", c.baseURL+"/auth/login", 
		bytes.NewBuffer(jsonData))
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

	// Extract API key from response
	var authResponse struct {
		APIKey string `json:"api_key"`
	}
	if err := json.NewDecoder(resp.Body).Decode(&authResponse); err != nil {
		return fmt.Errorf("failed to decode auth response: %w", err)
	}

	c.apiKey = authResponse.APIKey
	return nil
}

// Gateway management
func (c *EnhancedClient) ListGateways(ctx context.Context, filters map[string]string) ([]Gateway, error) {
	req, err := http.NewRequestWithContext(ctx, "GET", c.baseURL+"/gateways", nil)
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	q := req.URL.Query()
	for key, value := range filters {
		q.Add(key, value)
	}
	req.URL.RawQuery = q.Encode()

	req.Header.Set("Authorization", "Bearer "+c.apiKey)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to list gateways: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("failed to list gateways with status: %d", resp.StatusCode)
	}

	var response GatewayListResponse
	if err := json.NewDecoder(resp.Body).Decode(&response); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	return response.Gateways, nil
}

func (c *EnhancedClient) GetGateway(ctx context.Context, id string) (*Gateway, error) {
	req, err := http.NewRequestWithContext(ctx, "GET", c.baseURL+"/gateways/"+id, nil)
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	req.Header.Set("Authorization", "Bearer "+c.apiKey)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to get gateway: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("failed to get gateway with status: %d", resp.StatusCode)
	}

	var gateway Gateway
	if err := json.NewDecoder(resp.Body).Decode(&gateway); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	return &gateway, nil
}

func (c *EnhancedClient) CreateTransitGateway(ctx context.Context, gw *TransitGateway) (*TransitGateway, error) {
	jsonData, err := json.Marshal(gw)
	if err != nil {
		return nil, fmt.Errorf("failed to marshal gateway: %w", err)
	}

	req, err := http.NewRequestWithContext(ctx, "POST", c.baseURL+"/transit-gateways", 
		bytes.NewBuffer(jsonData))
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	req.Header.Set("Content-Type", "application/json")
	req.Header.Set("Authorization", "Bearer "+c.apiKey)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to create transit gateway: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusCreated {
		return nil, fmt.Errorf("failed to create transit gateway with status: %d", resp.StatusCode)
	}

	var createdGW TransitGateway
	if err := json.NewDecoder(resp.Body).Decode(&createdGW); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	return &createdGW, nil
}

func (c *EnhancedClient) CreateSpokeGateway(ctx context.Context, gw *SpokeGateway) (*SpokeGateway, error) {
	jsonData, err := json.Marshal(gw)
	if err != nil {
		return nil, fmt.Errorf("failed to marshal gateway: %w", err)
	}

	req, err := http.NewRequestWithContext(ctx, "POST", c.baseURL+"/spoke-gateways", 
		bytes.NewBuffer(jsonData))
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	req.Header.Set("Content-Type", "application/json")
	req.Header.Set("Authorization", "Bearer "+c.apiKey)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to create spoke gateway: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusCreated {
		return nil, fmt.Errorf("failed to create spoke gateway with status: %d", resp.StatusCode)
	}

	var createdGW SpokeGateway
	if err := json.NewDecoder(resp.Body).Decode(&createdGW); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	return &createdGW, nil
}

func (c *EnhancedClient) CreateVPNGateway(ctx context.Context, gw *VPNGateway) (*VPNGateway, error) {
	jsonData, err := json.Marshal(gw)
	if err != nil {
		return nil, fmt.Errorf("failed to marshal gateway: %w", err)
	}

	req, err := http.NewRequestWithContext(ctx, "POST", c.baseURL+"/vpn-gateways", 
		bytes.NewBuffer(jsonData))
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	req.Header.Set("Content-Type", "application/json")
	req.Header.Set("Authorization", "Bearer "+c.apiKey)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to create VPN gateway: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusCreated {
		return nil, fmt.Errorf("failed to create VPN gateway with status: %d", resp.StatusCode)
	}

	var createdGW VPNGateway
	if err := json.NewDecoder(resp.Body).Decode(&createdGW); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	return &createdGW, nil
}

func (c *EnhancedClient) UpdateGateway(ctx context.Context, id string, gw *Gateway) (*Gateway, error) {
	jsonData, err := json.Marshal(gw)
	if err != nil {
		return nil, fmt.Errorf("failed to marshal gateway: %w", err)
	}

	req, err := http.NewRequestWithContext(ctx, "PUT", c.baseURL+"/gateways/"+id, 
		bytes.NewBuffer(jsonData))
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	req.Header.Set("Content-Type", "application/json")
	req.Header.Set("Authorization", "Bearer "+c.apiKey)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to update gateway: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("failed to update gateway with status: %d", resp.StatusCode)
	}

	var updatedGW Gateway
	if err := json.NewDecoder(resp.Body).Decode(&updatedGW); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	return &updatedGW, nil
}

func (c *EnhancedClient) DeleteGateway(ctx context.Context, id string) error {
	req, err := http.NewRequestWithContext(ctx, "DELETE", c.baseURL+"/gateways/"+id, nil)
	if err != nil {
		return fmt.Errorf("failed to create request: %w", err)
	}

	req.Header.Set("Authorization", "Bearer "+c.apiKey)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return fmt.Errorf("failed to delete gateway: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusNoContent {
		return fmt.Errorf("failed to delete gateway with status: %d", resp.StatusCode)
	}

	return nil
}

// Gateway actions
func (c *EnhancedClient) StartGateway(ctx context.Context, id string) error {
	return c.gatewayAction(ctx, id, "start")
}

func (c *EnhancedClient) StopGateway(ctx context.Context, id string) error {
	return c.gatewayAction(ctx, id, "stop")
}

func (c *EnhancedClient) RestartGateway(ctx context.Context, id string) error {
	return c.gatewayAction(ctx, id, "restart")
}

func (c *EnhancedClient) ResizeGateway(ctx context.Context, id string, newSize string) error {
	actionData := map[string]string{
		"instance_size": newSize,
	}

	jsonData, err := json.Marshal(actionData)
	if err != nil {
		return fmt.Errorf("failed to marshal action data: %w", err)
	}

	req, err := http.NewRequestWithContext(ctx, "POST", c.baseURL+"/gateways/"+id+"/resize", 
		bytes.NewBuffer(jsonData))
	if err != nil {
		return fmt.Errorf("failed to create request: %w", err)
	}

	req.Header.Set("Content-Type", "application/json")
	req.Header.Set("Authorization", "Bearer "+c.apiKey)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return fmt.Errorf("failed to resize gateway: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return fmt.Errorf("failed to resize gateway with status: %d", resp.StatusCode)
	}

	return nil
}

func (c *EnhancedClient) gatewayAction(ctx context.Context, id, action string) error {
	req, err := http.NewRequestWithContext(ctx, "POST", 
		c.baseURL+"/gateways/"+id+"/"+action, nil)
	if err != nil {
		return fmt.Errorf("failed to create request: %w", err)
	}

	req.Header.Set("Authorization", "Bearer "+c.apiKey)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return fmt.Errorf("failed to %s gateway: %w", action, err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return fmt.Errorf("failed to %s gateway with status: %d", action, resp.StatusCode)
	}

	return nil
}

// Connection management
func (c *EnhancedClient) CreateConnection(ctx context.Context, conn *Connection) (*Connection, error) {
	jsonData, err := json.Marshal(conn)
	if err != nil {
		return nil, fmt.Errorf("failed to marshal connection: %w", err)
	}

	req, err := http.NewRequestWithContext(ctx, "POST", c.baseURL+"/connections", 
		bytes.NewBuffer(jsonData))
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	req.Header.Set("Content-Type", "application/json")
	req.Header.Set("Authorization", "Bearer "+c.apiKey)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to create connection: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusCreated {
		return nil, fmt.Errorf("failed to create connection with status: %d", resp.StatusCode)
	}

	var createdConn Connection
	if err := json.NewDecoder(resp.Body).Decode(&createdConn); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	return &createdConn, nil
}

func (c *EnhancedClient) DeleteConnection(ctx context.Context, id string) error {
	req, err := http.NewRequestWithContext(ctx, "DELETE", c.baseURL+"/connections/"+id, nil)
	if err != nil {
		return fmt.Errorf("failed to create request: %w", err)
	}

	req.Header.Set("Authorization", "Bearer "+c.apiKey)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return fmt.Errorf("failed to delete connection: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusNoContent {
		return fmt.Errorf("failed to delete connection with status: %d", resp.StatusCode)
	}

	return nil
}

// Routing management
func (c *EnhancedClient) ListRoutes(ctx context.Context, gatewayID string) ([]Route, error) {
	req, err := http.NewRequestWithContext(ctx, "GET", c.baseURL+"/routes", nil)
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	q := req.URL.Query()
	if gatewayID != "" {
		q.Add("gateway_id", gatewayID)
	}
	req.URL.RawQuery = q.Encode()

	req.Header.Set("Authorization", "Bearer "+c.apiKey)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to list routes: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("failed to list routes with status: %d", resp.StatusCode)
	}

	var response RouteListResponse
	if err := json.NewDecoder(resp.Body).Decode(&response); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	return response.Routes, nil
}

func (c *EnhancedClient) CreateRoutingPolicy(ctx context.Context, policy *RoutingPolicy) (*RoutingPolicy, error) {
	jsonData, err := json.Marshal(policy)
	if err != nil {
		return nil, fmt.Errorf("failed to marshal policy: %w", err)
	}

	req, err := http.NewRequestWithContext(ctx, "POST", c.baseURL+"/routing-policies", 
		bytes.NewBuffer(jsonData))
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	req.Header.Set("Content-Type", "application/json")
	req.Header.Set("Authorization", "Bearer "+c.apiKey)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to create routing policy: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusCreated {
		return nil, fmt.Errorf("failed to create routing policy with status: %d", resp.StatusCode)
	}

	var createdPolicy RoutingPolicy
	if err := json.NewDecoder(resp.Body).Decode(&createdPolicy); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	return &createdPolicy, nil
}

// Monitoring and analytics
func (c *EnhancedClient) GetGatewayMetrics(ctx context.Context, gatewayID string, duration string) ([]GatewayMetrics, error) {
	req, err := http.NewRequestWithContext(ctx, "GET", c.baseURL+"/gateways/"+gatewayID+"/metrics", nil)
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	q := req.URL.Query()
	q.Add("duration", duration)
	req.URL.RawQuery = q.Encode()

	req.Header.Set("Authorization", "Bearer "+c.apiKey)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to get gateway metrics: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("failed to get gateway metrics with status: %d", resp.StatusCode)
	}

	var metrics []GatewayMetrics
	if err := json.NewDecoder(resp.Body).Decode(&metrics); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	return metrics, nil
}

func (c *EnhancedClient) GetNetworkTopology(ctx context.Context) (*NetworkTopology, error) {
	req, err := http.NewRequestWithContext(ctx, "GET", c.baseURL+"/topology", nil)
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	req.Header.Set("Authorization", "Bearer "+c.apiKey)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to get network topology: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("failed to get network topology with status: %d", resp.StatusCode)
	}

	var topology NetworkTopology
	if err := json.NewDecoder(resp.Body).Decode(&topology); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	return &topology, nil
}

// Statistics
func (c *EnhancedClient) GetStats(ctx context.Context) (*AviatrixStats, error) {
	req, err := http.NewRequestWithContext(ctx, "GET", c.baseURL+"/stats", nil)
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	req.Header.Set("Authorization", "Bearer "+c.apiKey)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to get stats: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("failed to get stats with status: %d", resp.StatusCode)
	}

	var stats AviatrixStats
	if err := json.NewDecoder(resp.Body).Decode(&stats); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	return &stats, nil
}

// VPN user management
func (c *EnhancedClient) CreateVPNUser(ctx context.Context, user *VPNUser) (*VPNUser, error) {
	jsonData, err := json.Marshal(user)
	if err != nil {
		return nil, fmt.Errorf("failed to marshal user: %w", err)
	}

	req, err := http.NewRequestWithContext(ctx, "POST", c.baseURL+"/vpn-users", 
		bytes.NewBuffer(jsonData))
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	req.Header.Set("Content-Type", "application/json")
	req.Header.Set("Authorization", "Bearer "+c.apiKey)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to create VPN user: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusCreated {
		return nil, fmt.Errorf("failed to create VPN user with status: %d", resp.StatusCode)
	}

	var createdUser VPNUser
	if err := json.NewDecoder(resp.Body).Decode(&createdUser); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	return &createdUser, nil
}

func (c *EnhancedClient) DeleteVPNUser(ctx context.Context, id string) error {
	req, err := http.NewRequestWithContext(ctx, "DELETE", c.baseURL+"/vpn-users/"+id, nil)
	if err != nil {
		return fmt.Errorf("failed to create request: %w", err)
	}

	req.Header.Set("Authorization", "Bearer "+c.apiKey)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return fmt.Errorf("failed to delete VPN user: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusNoContent {
		return fmt.Errorf("failed to delete VPN user with status: %d", resp.StatusCode)
	}

	return nil
}
