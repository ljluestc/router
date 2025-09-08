package aviatrix

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
	config     *config.AviatrixConfig
	httpClient *http.Client
	logger     *logrus.Logger
	sessionID  string
}

func NewClient(cfg *config.AviatrixConfig) (*Client, error) {
	client := &Client{
		config: cfg,
		httpClient: &http.Client{
			Timeout: cfg.Timeout,
		},
		logger: logrus.New(),
	}

	// Authenticate
	if err := client.authenticate(); err != nil {
		return nil, fmt.Errorf("failed to authenticate with Aviatrix: %w", err)
	}

	return client, nil
}

func (c *Client) authenticate() error {
	ctx, cancel := context.WithTimeout(context.Background(), 10*time.Second)
	defer cancel()

	loginData := map[string]string{
		"username": c.config.Username,
		"password": c.config.Password,
	}

	jsonData, err := json.Marshal(loginData)
	if err != nil {
		return err
	}

	req, err := http.NewRequestWithContext(ctx, "POST", c.getAPIEndpoint("login"), bytes.NewBuffer(jsonData))
	if err != nil {
		return err
	}

	req.Header.Set("Content-Type", "application/json")

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return err
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return fmt.Errorf("authentication failed with status %d", resp.StatusCode)
	}

	var result map[string]interface{}
	if err := json.NewDecoder(resp.Body).Decode(&result); err != nil {
		return err
	}

	if cid, ok := result["CID"].(string); ok {
		c.sessionID = cid
	} else {
		return fmt.Errorf("no session ID in response")
	}

	return nil
}

func (c *Client) getAPIEndpoint(action string) string {
	return fmt.Sprintf("https://%s/v1/api", c.config.ControllerIP) + "?action=" + action
}

func (c *Client) makeRequest(ctx context.Context, action string, params map[string]interface{}) (*http.Response, error) {
	// Add session ID to params
	if c.sessionID != "" {
		params["CID"] = c.sessionID
	}

	jsonData, err := json.Marshal(params)
	if err != nil {
		return nil, err
	}

	req, err := http.NewRequestWithContext(ctx, "POST", c.getAPIEndpoint(action), bytes.NewBuffer(jsonData))
	if err != nil {
		return nil, err
	}

	req.Header.Set("Content-Type", "application/json")

	return c.httpClient.Do(req)
}

// Transit Gateway Management
func (c *Client) ListTransitGateways(ctx context.Context) ([]TransitGateway, error) {
	resp, err := c.makeRequest(ctx, "list_transit_gateways", map[string]interface{}{})
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("API returned status %d", resp.StatusCode)
	}

	var result struct {
		Results []TransitGateway `json:"results"`
	}

	if err := json.NewDecoder(resp.Body).Decode(&result); err != nil {
		return nil, err
	}

	return result.Results, nil
}

func (c *Client) CreateTransitGateway(ctx context.Context, req CreateTransitGatewayRequest) (*TransitGateway, error) {
	params := map[string]interface{}{
		"cloud_type":                    req.CloudType,
		"account_name":                  req.AccountName,
		"gw_name":                       req.GWName,
		"vpc_id":                        req.VPCID,
		"vpc_reg":                       req.Region,
		"subnet":                        req.Subnet,
		"gw_size":                       req.GWSize,
		"enable_encrypt_peering":        req.EnableEncryptPeering,
		"enable_learned_cidrs_approval": req.EnableLearnedCIDRsApproval,
	}

	resp, err := c.makeRequest(ctx, "create_transit_gateway", params)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("API returned status %d", resp.StatusCode)
	}

	var result struct {
		Return bool `json:"return"`
	}

	if err := json.NewDecoder(resp.Body).Decode(&result); err != nil {
		return nil, err
	}

	if !result.Return {
		return nil, fmt.Errorf("failed to create transit gateway")
	}

	// Get the created gateway
	gateways, err := c.ListTransitGateways(ctx)
	if err != nil {
		return nil, err
	}

	for _, gw := range gateways {
		if gw.GWName == req.GWName {
			return &gw, nil
		}
	}

	return nil, fmt.Errorf("transit gateway not found after creation")
}

func (c *Client) GetTransitGateway(ctx context.Context, name string) (*TransitGateway, error) {
	gateways, err := c.ListTransitGateways(ctx)
	if err != nil {
		return nil, err
	}

	for _, gw := range gateways {
		if gw.GWName == name {
			return &gw, nil
		}
	}

	return nil, fmt.Errorf("transit gateway not found")
}

func (c *Client) UpdateTransitGateway(ctx context.Context, name string, req UpdateTransitGatewayRequest) (*TransitGateway, error) {
	// Aviatrix doesn't support direct updates, need to delete and recreate
	if err := c.DeleteTransitGateway(ctx, name); err != nil {
		return nil, err
	}

	createReq := CreateTransitGatewayRequest{
		CloudType:                  req.CloudType,
		AccountName:                req.AccountName,
		GWName:                     req.GWName,
		VPCID:                      req.VPCID,
		Region:                     req.Region,
		Subnet:                     req.Subnet,
		GWSize:                     req.GWSize,
		EnableEncryptPeering:       req.EnableEncryptPeering,
		EnableLearnedCIDRsApproval: req.EnableLearnedCIDRsApproval,
	}

	return c.CreateTransitGateway(ctx, createReq)
}

func (c *Client) DeleteTransitGateway(ctx context.Context, name string) error {
	params := map[string]interface{}{
		"gw_name": name,
	}

	resp, err := c.makeRequest(ctx, "delete_transit_gateway", params)
	if err != nil {
		return err
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return fmt.Errorf("API returned status %d", resp.StatusCode)
	}

	var result struct {
		Return bool `json:"return"`
	}

	if err := json.NewDecoder(resp.Body).Decode(&result); err != nil {
		return err
	}

	if !result.Return {
		return fmt.Errorf("failed to delete transit gateway")
	}

	return nil
}

// Spoke Gateway Management
func (c *Client) ListSpokeGateways(ctx context.Context) ([]SpokeGateway, error) {
	resp, err := c.makeRequest(ctx, "list_spoke_gateways", map[string]interface{}{})
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("API returned status %d", resp.StatusCode)
	}

	var result struct {
		Results []SpokeGateway `json:"results"`
	}

	if err := json.NewDecoder(resp.Body).Decode(&result); err != nil {
		return nil, err
	}

	return result.Results, nil
}

func (c *Client) CreateSpokeGateway(ctx context.Context, req CreateSpokeGatewayRequest) (*SpokeGateway, error) {
	params := map[string]interface{}{
		"cloud_type":     req.CloudType,
		"account_name":   req.AccountName,
		"gw_name":        req.GWName,
		"vpc_id":         req.VPCID,
		"vpc_reg":        req.Region,
		"subnet":         req.Subnet,
		"gw_size":        req.GWSize,
		"enable_encrypt": req.EnableEncrypt,
	}

	resp, err := c.makeRequest(ctx, "create_spoke_gateway", params)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("API returned status %d", resp.StatusCode)
	}

	var result struct {
		Return bool `json:"return"`
	}

	if err := json.NewDecoder(resp.Body).Decode(&result); err != nil {
		return nil, err
	}

	if !result.Return {
		return nil, fmt.Errorf("failed to create spoke gateway")
	}

	// Get the created gateway
	gateways, err := c.ListSpokeGateways(ctx)
	if err != nil {
		return nil, err
	}

	for _, gw := range gateways {
		if gw.GWName == req.GWName {
			return &gw, nil
		}
	}

	return nil, fmt.Errorf("spoke gateway not found after creation")
}

func (c *Client) GetSpokeGateway(ctx context.Context, name string) (*SpokeGateway, error) {
	gateways, err := c.ListSpokeGateways(ctx)
	if err != nil {
		return nil, err
	}

	for _, gw := range gateways {
		if gw.GWName == name {
			return &gw, nil
		}
	}

	return nil, fmt.Errorf("spoke gateway not found")
}

func (c *Client) UpdateSpokeGateway(ctx context.Context, name string, req UpdateSpokeGatewayRequest) (*SpokeGateway, error) {
	// Aviatrix doesn't support direct updates, need to delete and recreate
	if err := c.DeleteSpokeGateway(ctx, name); err != nil {
		return nil, err
	}

	createReq := CreateSpokeGatewayRequest{
		CloudType:    req.CloudType,
		AccountName:  req.AccountName,
		GWName:       req.GWName,
		VPCID:        req.VPCID,
		Region:       req.Region,
		Subnet:       req.Subnet,
		GWSize:       req.GWSize,
		EnableEncrypt: req.EnableEncrypt,
	}

	return c.CreateSpokeGateway(ctx, createReq)
}

func (c *Client) DeleteSpokeGateway(ctx context.Context, name string) error {
	params := map[string]interface{}{
		"gw_name": name,
	}

	resp, err := c.makeRequest(ctx, "delete_spoke_gateway", params)
	if err != nil {
		return err
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return fmt.Errorf("API returned status %d", resp.StatusCode)
	}

	var result struct {
		Return bool `json:"return"`
	}

	if err := json.NewDecoder(resp.Body).Decode(&result); err != nil {
		return err
	}

	if !result.Return {
		return fmt.Errorf("failed to delete spoke gateway")
	}

	return nil
}

// VPC Connection Management
func (c *Client) ListVPCConnections(ctx context.Context) ([]VPCConnection, error) {
	resp, err := c.makeRequest(ctx, "list_vpc_connections", map[string]interface{}{})
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("API returned status %d", resp.StatusCode)
	}

	var result struct {
		Results []VPCConnection `json:"results"`
	}

	if err := json.NewDecoder(resp.Body).Decode(&result); err != nil {
		return nil, err
	}

	return result.Results, nil
}

func (c *Client) CreateVPCConnection(ctx context.Context, req CreateVPCConnectionRequest) (*VPCConnection, error) {
	params := map[string]interface{}{
		"connection_name":                req.ConnectionName,
		"vpc_id":                         req.VPCID,
		"account_name":                   req.AccountName,
		"region":                         req.Region,
		"transit_gateway":                req.TransitGateway,
		"spoke_gateway":                  req.SpokeGateway,
		"connection_type":                req.ConnectionType,
		"enable_learned_cidrs_approval":  req.EnableLearnedCIDRsApproval,
	}

	if len(req.ApprovedCIDRs) > 0 {
		params["approved_cidrs"] = req.ApprovedCIDRs
	}

	resp, err := c.makeRequest(ctx, "create_vpc_connection", params)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("API returned status %d", resp.StatusCode)
	}

	var result struct {
		Return bool `json:"return"`
	}

	if err := json.NewDecoder(resp.Body).Decode(&result); err != nil {
		return nil, err
	}

	if !result.Return {
		return nil, fmt.Errorf("failed to create VPC connection")
	}

	// Get the created connection
	connections, err := c.ListVPCConnections(ctx)
	if err != nil {
		return nil, err
	}

	for _, conn := range connections {
		if conn.ConnectionName == req.ConnectionName {
			return &conn, nil
		}
	}

	return nil, fmt.Errorf("VPC connection not found after creation")
}

func (c *Client) GetVPCConnection(ctx context.Context, name string) (*VPCConnection, error) {
	connections, err := c.ListVPCConnections(ctx)
	if err != nil {
		return nil, err
	}

	for _, conn := range connections {
		if conn.ConnectionName == name {
			return &conn, nil
		}
	}

	return nil, fmt.Errorf("VPC connection not found")
}

func (c *Client) DeleteVPCConnection(ctx context.Context, name string) error {
	params := map[string]interface{}{
		"connection_name": name,
	}

	resp, err := c.makeRequest(ctx, "delete_vpc_connection", params)
	if err != nil {
		return err
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return fmt.Errorf("API returned status %d", resp.StatusCode)
	}

	var result struct {
		Return bool `json:"return"`
	}

	if err := json.NewDecoder(resp.Body).Decode(&result); err != nil {
		return err
	}

	if !result.Return {
		return fmt.Errorf("failed to delete VPC connection")
	}

	return nil
}

// Site-to-Cloud Connection Management
func (c *Client) ListSite2CloudConnections(ctx context.Context) ([]Site2CloudConnection, error) {
	resp, err := c.makeRequest(ctx, "list_site2cloud", map[string]interface{}{})
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("API returned status %d", resp.StatusCode)
	}

	var result struct {
		Results []Site2CloudConnection `json:"results"`
	}

	if err := json.NewDecoder(resp.Body).Decode(&result); err != nil {
		return nil, err
	}

	return result.Results, nil
}

func (c *Client) CreateSite2CloudConnection(ctx context.Context, req CreateSite2CloudConnectionRequest) (*Site2CloudConnection, error) {
	params := map[string]interface{}{
		"connection_name":    req.ConnectionName,
		"vpc_id":             req.VPCID,
		"remote_gateway_ip":  req.RemoteGatewayIP,
		"pre_shared_key":     req.PreSharedKey,
		"connection_type":    req.ConnectionType,
		"remote_subnet":      req.RemoteSubnet,
		"local_subnet":       req.LocalSubnet,
	}

	resp, err := c.makeRequest(ctx, "create_site2cloud", params)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("API returned status %d", resp.StatusCode)
	}

	var result struct {
		Return bool `json:"return"`
	}

	if err := json.NewDecoder(resp.Body).Decode(&result); err != nil {
		return nil, err
	}

	if !result.Return {
		return nil, fmt.Errorf("failed to create site-to-cloud connection")
	}

	// Get the created connection
	connections, err := c.ListSite2CloudConnections(ctx)
	if err != nil {
		return nil, err
	}

	for _, conn := range connections {
		if conn.ConnectionName == req.ConnectionName {
			return &conn, nil
		}
	}

	return nil, fmt.Errorf("site-to-cloud connection not found after creation")
}

func (c *Client) GetSite2CloudConnection(ctx context.Context, name string) (*Site2CloudConnection, error) {
	connections, err := c.ListSite2CloudConnections(ctx)
	if err != nil {
		return nil, err
	}

	for _, conn := range connections {
		if conn.ConnectionName == name {
			return &conn, nil
		}
	}

	return nil, fmt.Errorf("site-to-cloud connection not found")
}

func (c *Client) DeleteSite2CloudConnection(ctx context.Context, name string) error {
	params := map[string]interface{}{
		"connection_name": name,
	}

	resp, err := c.makeRequest(ctx, "delete_site2cloud", params)
	if err != nil {
		return err
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return fmt.Errorf("API returned status %d", resp.StatusCode)
	}

	var result struct {
		Return bool `json:"return"`
	}

	if err := json.NewDecoder(resp.Body).Decode(&result); err != nil {
		return err
	}

	if !result.Return {
		return fmt.Errorf("failed to delete site-to-cloud connection")
	}

	return nil
}
