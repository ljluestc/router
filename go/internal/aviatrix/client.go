package aviatrix

import (
	"bytes"
	"context"
	"crypto/tls"
	"encoding/json"
	"fmt"
	"io"
	"net/http"
	"time"

	"router-sim/internal/config"
)

// Client represents an Aviatrix client
type Client struct {
	config     config.AviatrixConfig
	httpClient *http.Client
	baseURL    string
	token      string
}

// NewClient creates a new Aviatrix client
func NewClient(config config.AviatrixConfig) (*Client, error) {
	// Create HTTP client with custom TLS config
	httpClient := &http.Client{
		Timeout: config.Timeout,
		Transport: &http.Transport{
			TLSClientConfig: &tls.Config{
				InsecureSkipVerify: !config.VerifySSL,
			},
		},
	}

	baseURL := fmt.Sprintf("https://%s/v1/api", config.ControllerIP)

	client := &Client{
		config:     config,
		httpClient: httpClient,
		baseURL:    baseURL,
	}

	// Authenticate and get token
	if err := client.authenticate(); err != nil {
		return nil, fmt.Errorf("failed to authenticate: %w", err)
	}

	return client, nil
}

// Gateway represents an Aviatrix gateway
type Gateway struct {
	CloudType     int    `json:"cloud_type"`
	GwName        string `json:"gw_name"`
	GwSize        string `json:"gw_size"`
	VpcID         string `json:"vpc_id"`
	Subnet        string `json:"subnet"`
	ElasticIP     string `json:"elastic_ip"`
	PublicIP      string `json:"public_ip"`
	PrivateIP     string `json:"private_ip"`
	State         string `json:"state"`
	Status        string `json:"status"`
	EnableEIP     bool   `json:"enable_eip"`
	EnableNat     bool   `json:"enable_nat"`
	EnableVpcDns  bool   `json:"enable_vpc_dns"`
	EnableLdap    bool   `json:"enable_ldap"`
	EnableLdapSsl bool   `json:"enable_ldap_ssl"`
	VpcRegion     string `json:"vpc_region"`
	VpcSize       string `json:"vpc_size"`
	VpcNet        string `json:"vpc_net"`
	VpcCIDR       string `json:"vpc_cidr"`
	VpcState      string `json:"vpc_state"`
	VpcID2        string `json:"vpc_id2"`
	VpcRegion2    string `json:"vpc_region2"`
	VpcSize2      string `json:"vpc_size2"`
	VpcNet2       string `json:"vpc_net2"`
	VpcCIDR2      string `json:"vpc_cidr2"`
	VpcState2     string `json:"vpc_state2"`
	VpcID3        string `json:"vpc_id3"`
	VpcRegion3    string `json:"vpc_region3"`
	VpcSize3      string `json:"vpc_size3"`
	VpcNet3       string `json:"vpc_net3"`
	VpcCIDR3      string `json:"vpc_cidr3"`
	VpcState3     string `json:"vpc_state3"`
	VpcID4        string `json:"vpc_id4"`
	VpcRegion4    string `json:"vpc_region4"`
	VpcSize4      string `json:"vpc_size4"`
	VpcNet4       string `json:"vpc_net4"`
	VpcCIDR4      string `json:"vpc_cidr4"`
	VpcState4     string `json:"vpc_state4"`
	VpcID5        string `json:"vpc_id5"`
	VpcRegion5    string `json:"vpc_region5"`
	VpcSize5      string `json:"vpc_size5"`
	VpcNet5       string `json:"vpc_net5"`
	VpcCIDR5      string `json:"vpc_cidr5"`
	VpcState5     string `json:"vpc_state5"`
	VpcID6        string `json:"vpc_id6"`
	VpcRegion6    string `json:"vpc_region6"`
	VpcSize6      string `json:"vpc_size6"`
	VpcNet6       string `json:"vpc_net6"`
	VpcCIDR6      string `json:"vpc_cidr6"`
	VpcState6     string `json:"vpc_state6"`
	VpcID7        string `json:"vpc_id7"`
	VpcRegion7    string `json:"vpc_region7"`
	VpcSize7      string `json:"vpc_size7"`
	VpcNet7       string `json:"vpc_net7"`
	VpcCIDR7      string `json:"vpc_cidr7"`
	VpcState7     string `json:"vpc_state7"`
	VpcID8        string `json:"vpc_id8"`
	VpcRegion8    string `json:"vpc_region8"`
	VpcSize8      string `json:"vpc_size8"`
	VpcNet8       string `json:"vpc_net8"`
	VpcCIDR8      string `json:"vpc_cidr8"`
	VpcState8     string `json:"vpc_state8"`
	VpcID9        string `json:"vpc_id9"`
	VpcRegion9    string `json:"vpc_region9"`
	VpcSize9      string `json:"vpc_size9"`
	VpcNet9       string `json:"vpc_net9"`
	VpcCIDR9      string `json:"vpc_cidr9"`
	VpcState9     string `json:"vpc_state9"`
	VpcID10       string `json:"vpc_id10"`
	VpcRegion10   string `json:"vpc_region10"`
	VpcSize10     string `json:"vpc_size10"`
	VpcNet10      string `json:"vpc_net10"`
	VpcCIDR10     string `json:"vpc_cidr10"`
	VpcState10    string `json:"vpc_state10"`
	VpcID11       string `json:"vpc_id11"`
	VpcRegion11   string `json:"vpc_region11"`
	VpcSize11     string `json:"vpc_size11"`
	VpcNet11      string `json:"vpc_net11"`
	VpcCIDR11     string `json:"vpc_cidr11"`
	VpcState11    string `json:"vpc_state11"`
	VpcID12       string `json:"vpc_id12"`
	VpcRegion12   string `json:"vpc_region12"`
	VpcSize12     string `json:"vpc_size12"`
	VpcNet12      string `json:"vpc_net12"`
	VpcCIDR12     string `json:"vpc_cidr12"`
	VpcState12    string `json:"vpc_state12"`
	VpcID13       string `json:"vpc_id13"`
	VpcRegion13   string `json:"vpc_region13"`
	VpcSize13     string `json:"vpc_size13"`
	VpcNet13      string `json:"vpc_net13"`
	VpcCIDR13     string `json:"vpc_cidr13"`
	VpcState13    string `json:"vpc_state13"`
	VpcID14       string `json:"vpc_id14"`
	VpcRegion14   string `json:"vpc_region14"`
	VpcSize14     string `json:"vpc_size14"`
	VpcNet14      string `json:"vpc_net14"`
	VpcCIDR14     string `json:"vpc_cidr14"`
	VpcState14    string `json:"vpc_state14"`
	VpcID15       string `json:"vpc_id15"`
	VpcRegion15   string `json:"vpc_region15"`
	VpcSize15     string `json:"vpc_size15"`
	VpcNet15      string `json:"vpc_net15"`
	VpcCIDR15     string `json:"vpc_cidr15"`
	VpcState15    string `json:"vpc_state15"`
	VpcID16       string `json:"vpc_id16"`
	VpcRegion16   string `json:"vpc_region16"`
	VpcSize16     string `json:"vpc_size16"`
	VpcNet16      string `json:"vpc_net16"`
	VpcCIDR16     string `json:"vpc_cidr16"`
	VpcState16    string `json:"vpc_state16"`
	VpcID17       string `json:"vpc_id17"`
	VpcRegion17   string `json:"vpc_region17"`
	VpcSize17     string `json:"vpc_size17"`
	VpcNet17      string `json:"vpc_net17"`
	VpcCIDR17     string `json:"vpc_cidr17"`
	VpcState17    string `json:"vpc_state18"`
	VpcID18       string `json:"vpc_id18"`
	VpcRegion18   string `json:"vpc_region18"`
	VpcSize18     string `json:"vpc_size18"`
	VpcNet18      string `json:"vpc_net18"`
	VpcCIDR18     string `json:"vpc_cidr18"`
	VpcState18    string `json:"vpc_state18"`
	VpcID19       string `json:"vpc_id19"`
	VpcRegion19   string `json:"vpc_region19"`
	VpcSize19     string `json:"vpc_size19"`
	VpcNet19      string `json:"vpc_net19"`
	VpcCIDR19     string `json:"vpc_cidr19"`
	VpcState19    string `json:"vpc_state19"`
	VpcID20       string `json:"vpc_id20"`
	VpcRegion20   string `json:"vpc_region20"`
	VpcSize20     string `json:"vpc_size20"`
	VpcNet20      string `json:"vpc_net20"`
	VpcCIDR20     string `json:"vpc_cidr20"`
	VpcState20    string `json:"vpc_state20"`
}

// TransitGateway represents an Aviatrix transit gateway
type TransitGateway struct {
	CloudType     int    `json:"cloud_type"`
	GwName        string `json:"gw_name"`
	GwSize        string `json:"gw_size"`
	VpcID         string `json:"vpc_id"`
	Subnet        string `json:"subnet"`
	ElasticIP     string `json:"elastic_ip"`
	PublicIP      string `json:"public_ip"`
	PrivateIP     string `json:"private_ip"`
	State         string `json:"state"`
	Status        string `json:"status"`
	EnableEIP     bool   `json:"enable_eip"`
	EnableNat     bool   `json:"enable_nat"`
	EnableVpcDns  bool   `json:"enable_vpc_dns"`
	EnableLdap    bool   `json:"enable_ldap"`
	EnableLdapSsl bool   `json:"enable_ldap_ssl"`
	VpcRegion     string `json:"vpc_region"`
	VpcSize       string `json:"vpc_size"`
	VpcNet        string `json:"vpc_net"`
	VpcCIDR       string `json:"vpc_cidr"`
	VpcState      string `json:"vpc_state"`
}

// SpokeGateway represents an Aviatrix spoke gateway
type SpokeGateway struct {
	CloudType     int    `json:"cloud_type"`
	GwName        string `json:"gw_name"`
	GwSize        string `json:"gw_size"`
	VpcID         string `json:"vpc_id"`
	Subnet        string `json:"subnet"`
	ElasticIP     string `json:"elastic_ip"`
	PublicIP      string `json:"public_ip"`
	PrivateIP     string `json:"private_ip"`
	State         string `json:"state"`
	Status        string `json:"status"`
	EnableEIP     bool   `json:"enable_eip"`
	EnableNat     bool   `json:"enable_nat"`
	EnableVpcDns  bool   `json:"enable_vpc_dns"`
	EnableLdap    bool   `json:"enable_ldap"`
	EnableLdapSsl bool   `json:"enable_ldap_ssl"`
	VpcRegion     string `json:"vpc_region"`
	VpcSize       string `json:"vpc_size"`
	VpcNet        string `json:"vpc_net"`
	VpcCIDR       string `json:"vpc_cidr"`
	VpcState      string `json:"vpc_state"`
}

// VPNGateway represents an Aviatrix VPN gateway
type VPNGateway struct {
	CloudType     int    `json:"cloud_type"`
	GwName        string `json:"gw_name"`
	GwSize        string `json:"gw_size"`
	VpcID         string `json:"vpc_id"`
	Subnet        string `json:"subnet"`
	ElasticIP     string `json:"elastic_ip"`
	PublicIP      string `json:"public_ip"`
	PrivateIP     string `json:"private_ip"`
	State         string `json:"state"`
	Status        string `json:"status"`
	EnableEIP     bool   `json:"enable_eip"`
	EnableNat     bool   `json:"enable_nat"`
	EnableVpcDns  bool   `json:"enable_vpc_dns"`
	EnableLdap    bool   `json:"enable_ldap"`
	EnableLdapSsl bool   `json:"enable_ldap_ssl"`
	VpcRegion     string `json:"vpc_region"`
	VpcSize       string `json:"vpc_size"`
	VpcNet        string `json:"vpc_net"`
	VpcCIDR       string `json:"vpc_cidr"`
	VpcState      string `json:"vpc_state"`
}

// BGPNeighbor represents a BGP neighbor
type BGPNeighbor struct {
	IP          string `json:"ip"`
	ASN         int    `json:"asn"`
	Password    string `json:"password"`
	Description string `json:"description"`
	Enabled     bool   `json:"enabled"`
}

// Route represents a route
type Route struct {
	Network     string `json:"network"`
	NextHop     string `json:"next_hop"`
	Protocol    string `json:"protocol"`
	Metric      int    `json:"metric"`
	ASPath      string `json:"as_path"`
	Communities string `json:"communities"`
	LocalPref   int    `json:"local_pref"`
	Origin      string `json:"origin"`
}

// authenticate authenticates with Aviatrix controller
func (c *Client) authenticate() error {
	url := fmt.Sprintf("%s/login", c.baseURL)
	
	authData := map[string]string{
		"action":   "login",
		"username": c.config.Username,
		"password": c.config.Password,
	}

	body, err := json.Marshal(authData)
	if err != nil {
		return fmt.Errorf("failed to marshal auth data: %w", err)
	}

	req, err := http.NewRequest("POST", url, bytes.NewBuffer(body))
	if err != nil {
		return fmt.Errorf("failed to create request: %w", err)
	}

	req.Header.Set("Content-Type", "application/json")

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return fmt.Errorf("failed to execute request: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return fmt.Errorf("authentication failed with status: %d", resp.StatusCode)
	}

	var result map[string]interface{}
	if err := json.NewDecoder(resp.Body).Decode(&result); err != nil {
		return fmt.Errorf("failed to decode response: %w", err)
	}

	// Extract token from response
	if token, ok := result["CID"].(string); ok {
		c.token = token
	} else {
		return fmt.Errorf("failed to extract token from response")
	}

	return nil
}

// GetGateways retrieves all gateways
func (c *Client) GetGateways(ctx context.Context) ([]Gateway, error) {
	url := fmt.Sprintf("%s/list-gateways", c.baseURL)
	
	req, err := http.NewRequestWithContext(ctx, "GET", url, nil)
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	c.setHeaders(req)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to execute request: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("request failed with status: %d", resp.StatusCode)
	}

	var result struct {
		Data []Gateway `json:"data"`
	}

	if err := json.NewDecoder(resp.Body).Decode(&result); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	return result.Data, nil
}

// GetTransitGateways retrieves transit gateways
func (c *Client) GetTransitGateways(ctx context.Context) ([]TransitGateway, error) {
	url := fmt.Sprintf("%s/list-transit-gateways", c.baseURL)
	
	req, err := http.NewRequestWithContext(ctx, "GET", url, nil)
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	c.setHeaders(req)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to execute request: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("request failed with status: %d", resp.StatusCode)
	}

	var result struct {
		Data []TransitGateway `json:"data"`
	}

	if err := json.NewDecoder(resp.Body).Decode(&result); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	return result.Data, nil
}

// GetSpokeGateways retrieves spoke gateways
func (c *Client) GetSpokeGateways(ctx context.Context) ([]SpokeGateway, error) {
	url := fmt.Sprintf("%s/list-spoke-gateways", c.baseURL)
	
	req, err := http.NewRequestWithContext(ctx, "GET", url, nil)
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	c.setHeaders(req)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to execute request: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("request failed with status: %d", resp.StatusCode)
	}

	var result struct {
		Data []SpokeGateway `json:"data"`
	}

	if err := json.NewDecoder(resp.Body).Decode(&result); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	return result.Data, nil
}

// GetVPNGateways retrieves VPN gateways
func (c *Client) GetVPNGateways(ctx context.Context) ([]VPNGateway, error) {
	url := fmt.Sprintf("%s/list-vpn-gateways", c.baseURL)
	
	req, err := http.NewRequestWithContext(ctx, "GET", url, nil)
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	c.setHeaders(req)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to execute request: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("request failed with status: %d", resp.StatusCode)
	}

	var result struct {
		Data []VPNGateway `json:"data"`
	}

	if err := json.NewDecoder(resp.Body).Decode(&result); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	return result.Data, nil
}

// GetBGPNeighbors retrieves BGP neighbors
func (c *Client) GetBGPNeighbors(ctx context.Context, gatewayName string) ([]BGPNeighbor, error) {
	url := fmt.Sprintf("%s/get-bgp-neighbor", c.baseURL)
	
	params := map[string]string{
		"action":      "get_bgp_neighbor",
		"gateway_name": gatewayName,
	}

	body, err := json.Marshal(params)
	if err != nil {
		return nil, fmt.Errorf("failed to marshal request: %w", err)
	}

	req, err := http.NewRequestWithContext(ctx, "POST", url, bytes.NewBuffer(body))
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	c.setHeaders(req)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to execute request: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("request failed with status: %d", resp.StatusCode)
	}

	var result struct {
		Data []BGPNeighbor `json:"data"`
	}

	if err := json.NewDecoder(resp.Body).Decode(&result); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	return result.Data, nil
}

// GetRoutes retrieves routes
func (c *Client) GetRoutes(ctx context.Context, gatewayName string) ([]Route, error) {
	url := fmt.Sprintf("%s/get-routes", c.baseURL)
	
	params := map[string]string{
		"action":       "get_routes",
		"gateway_name": gatewayName,
	}

	body, err := json.Marshal(params)
	if err != nil {
		return nil, fmt.Errorf("failed to marshal request: %w", err)
	}

	req, err := http.NewRequestWithContext(ctx, "POST", url, bytes.NewBuffer(body))
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	c.setHeaders(req)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to execute request: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("request failed with status: %d", resp.StatusCode)
	}

	var result struct {
		Data []Route `json:"data"`
	}

	if err := json.NewDecoder(resp.Body).Decode(&result); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	return result.Data, nil
}

// GetGatewayStatus retrieves gateway status
func (c *Client) GetGatewayStatus(ctx context.Context, gatewayName string) (map[string]interface{}, error) {
	url := fmt.Sprintf("%s/get-gateway-status", c.baseURL)
	
	params := map[string]string{
		"action":       "get_gateway_status",
		"gateway_name": gatewayName,
	}

	body, err := json.Marshal(params)
	if err != nil {
		return nil, fmt.Errorf("failed to marshal request: %w", err)
	}

	req, err := http.NewRequestWithContext(ctx, "POST", url, bytes.NewBuffer(body))
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	c.setHeaders(req)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to execute request: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("request failed with status: %d", resp.StatusCode)
	}

	var result map[string]interface{}
	if err := json.NewDecoder(resp.Body).Decode(&result); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	return result, nil
}

// GetTopology retrieves network topology
func (c *Client) GetTopology(ctx context.Context) (map[string]interface{}, error) {
	url := fmt.Sprintf("%s/get-topology", c.baseURL)
	
	req, err := http.NewRequestWithContext(ctx, "GET", url, nil)
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	c.setHeaders(req)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to execute request: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("request failed with status: %d", resp.StatusCode)
	}

	var result map[string]interface{}
	if err := json.NewDecoder(resp.Body).Decode(&result); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	return result, nil
}

// setHeaders sets common headers for requests
func (c *Client) setHeaders(req *http.Request) {
	req.Header.Set("Content-Type", "application/json")
	req.Header.Set("Accept", "application/json")
	req.Header.Set("User-Agent", "router-sim/1.0.0")
	
	if c.token != "" {
		req.Header.Set("Authorization", "Bearer "+c.token)
	}
}
