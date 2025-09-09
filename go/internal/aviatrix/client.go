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
	ControllerIP string
	Username     string
	Password     string
	Token        string
	HTTPClient   *http.Client
}

// NewAviatrixClient creates a new Aviatrix client
func NewAviatrixClient(controllerIP, username, password string) *AviatrixClient {
	return &AviatrixClient{
		ControllerIP: controllerIP,
		Username:     username,
		Password:     password,
		HTTPClient: &http.Client{
			Timeout: 30 * time.Second,
		},
	}
}

// Authenticate authenticates with Aviatrix Controller
func (c *AviatrixClient) Authenticate(ctx context.Context) error {
	authReq := map[string]string{
		"action":   "login",
		"username": c.Username,
		"password": c.Password,
	}

	jsonData, err := json.Marshal(authReq)
	if err != nil {
		return fmt.Errorf("failed to marshal auth request: %w", err)
	}

	req, err := http.NewRequestWithContext(ctx, "POST", "https://"+c.ControllerIP+"/v1/api", bytes.NewBuffer(jsonData))
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
		Return bool   `json:"return"`
		Reason string `json:"reason"`
		CID    string `json:"CID"`
	}

	if err := json.NewDecoder(resp.Body).Decode(&authResp); err != nil {
		return fmt.Errorf("failed to decode auth response: %w", err)
	}

	if !authResp.Return {
		return fmt.Errorf("authentication failed: %s", authResp.Reason)
	}

	c.Token = authResp.CID
	return nil
}

// GetGateways retrieves all gateways
func (c *AviatrixClient) GetGateways(ctx context.Context) ([]Gateway, error) {
	req := map[string]string{
		"action": "list_vpcs_summary",
		"CID":    c.Token,
	}

	jsonData, err := json.Marshal(req)
	if err != nil {
		return nil, fmt.Errorf("failed to marshal request: %w", err)
	}

	httpReq, err := http.NewRequestWithContext(ctx, "POST", "https://"+c.ControllerIP+"/v1/api", bytes.NewBuffer(jsonData))
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	httpReq.Header.Set("Content-Type", "application/json")

	resp, err := c.HTTPClient.Do(httpReq)
	if err != nil {
		return nil, fmt.Errorf("failed to get gateways: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("failed to get gateways with status: %d", resp.StatusCode)
	}

	var gatewaysResp struct {
		Return  bool      `json:"return"`
		Reason  string    `json:"reason"`
		Results []Gateway `json:"results"`
	}

	if err := json.NewDecoder(resp.Body).Decode(&gatewaysResp); err != nil {
		return nil, fmt.Errorf("failed to decode gateways response: %w", err)
	}

	if !gatewaysResp.Return {
		return nil, fmt.Errorf("failed to get gateways: %s", gatewaysResp.Reason)
	}

	return gatewaysResp.Results, nil
}

// GetTransitGateways retrieves transit gateways
func (c *AviatrixClient) GetTransitGateways(ctx context.Context) ([]TransitGateway, error) {
	req := map[string]string{
		"action": "list_transit_gateways",
		"CID":    c.Token,
	}

	jsonData, err := json.Marshal(req)
	if err != nil {
		return nil, fmt.Errorf("failed to marshal request: %w", err)
	}

	httpReq, err := http.NewRequestWithContext(ctx, "POST", "https://"+c.ControllerIP+"/v1/api", bytes.NewBuffer(jsonData))
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	httpReq.Header.Set("Content-Type", "application/json")

	resp, err := c.HTTPClient.Do(httpReq)
	if err != nil {
		return nil, fmt.Errorf("failed to get transit gateways: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("failed to get transit gateways with status: %d", resp.StatusCode)
	}

	var gatewaysResp struct {
		Return  bool             `json:"return"`
		Reason  string           `json:"reason"`
		Results []TransitGateway `json:"results"`
	}

	if err := json.NewDecoder(resp.Body).Decode(&gatewaysResp); err != nil {
		return nil, fmt.Errorf("failed to decode transit gateways response: %w", err)
	}

	if !gatewaysResp.Return {
		return nil, fmt.Errorf("failed to get transit gateways: %s", gatewaysResp.Reason)
	}

	return gatewaysResp.Results, nil
}

// GetSpokeGateways retrieves spoke gateways
func (c *AviatrixClient) GetSpokeGateways(ctx context.Context) ([]SpokeGateway, error) {
	req := map[string]string{
		"action": "list_spoke_gateways",
		"CID":    c.Token,
	}

	jsonData, err := json.Marshal(req)
	if err != nil {
		return nil, fmt.Errorf("failed to marshal request: %w", err)
	}

	httpReq, err := http.NewRequestWithContext(ctx, "POST", "https://"+c.ControllerIP+"/v1/api", bytes.NewBuffer(jsonData))
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	httpReq.Header.Set("Content-Type", "application/json")

	resp, err := c.HTTPClient.Do(httpReq)
	if err != nil {
		return nil, fmt.Errorf("failed to get spoke gateways: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("failed to get spoke gateways with status: %d", resp.StatusCode)
	}

	var gatewaysResp struct {
		Return  bool           `json:"return"`
		Reason  string         `json:"reason"`
		Results []SpokeGateway `json:"results"`
	}

	if err := json.NewDecoder(resp.Body).Decode(&gatewaysResp); err != nil {
		return nil, fmt.Errorf("failed to decode spoke gateways response: %w", err)
	}

	if !gatewaysResp.Return {
		return nil, fmt.Errorf("failed to get spoke gateways: %s", gatewaysResp.Reason)
	}

	return gatewaysResp.Results, nil
}

// CreateTransitGateway creates a new transit gateway
func (c *AviatrixClient) CreateTransitGateway(ctx context.Context, req CreateTransitGatewayRequest) (*TransitGateway, error) {
	reqMap := map[string]interface{}{
		"action":           "create_transit_gateway",
		"CID":              c.Token,
		"cloud_type":       req.CloudType,
		"account_name":     req.AccountName,
		"gw_name":          req.GatewayName,
		"vpc_id":           req.VPCID,
		"vpc_reg":          req.VPCRegion,
		"gw_size":          req.GatewaySize,
		"subnet":           req.Subnet,
		"enable_encrypt":   req.EnableEncryption,
		"enable_nat":       req.EnableNAT,
		"enable_hybrid":    req.EnableHybrid,
		"enable_firenet":   req.EnableFireNet,
		"enable_vpc_dns":   req.EnableVPCDNS,
		"enable_advertise": req.EnableAdvertise,
	}

	jsonData, err := json.Marshal(reqMap)
	if err != nil {
		return nil, fmt.Errorf("failed to marshal request: %w", err)
	}

	httpReq, err := http.NewRequestWithContext(ctx, "POST", "https://"+c.ControllerIP+"/v1/api", bytes.NewBuffer(jsonData))
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	httpReq.Header.Set("Content-Type", "application/json")

	resp, err := c.HTTPClient.Do(httpReq)
	if err != nil {
		return nil, fmt.Errorf("failed to create transit gateway: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("failed to create transit gateway with status: %d", resp.StatusCode)
	}

	var createResp struct {
		Return bool   `json:"return"`
		Reason string `json:"reason"`
	}

	if err := json.NewDecoder(resp.Body).Decode(&createResp); err != nil {
		return nil, fmt.Errorf("failed to decode create response: %w", err)
	}

	if !createResp.Return {
		return nil, fmt.Errorf("failed to create transit gateway: %s", createResp.Reason)
	}

	// Return the created gateway (simplified)
	return &TransitGateway{
		GatewayName: req.GatewayName,
		CloudType:   req.CloudType,
		VPCID:       req.VPCID,
		Status:      "creating",
	}, nil
}

// DeleteTransitGateway deletes a transit gateway
func (c *AviatrixClient) DeleteTransitGateway(ctx context.Context, gatewayName string) error {
	req := map[string]string{
		"action":  "delete_transit_gateway",
		"CID":     c.Token,
		"gw_name": gatewayName,
	}

	jsonData, err := json.Marshal(req)
	if err != nil {
		return fmt.Errorf("failed to marshal request: %w", err)
	}

	httpReq, err := http.NewRequestWithContext(ctx, "POST", "https://"+c.ControllerIP+"/v1/api", bytes.NewBuffer(jsonData))
	if err != nil {
		return fmt.Errorf("failed to create request: %w", err)
	}

	httpReq.Header.Set("Content-Type", "application/json")

	resp, err := c.HTTPClient.Do(httpReq)
	if err != nil {
		return fmt.Errorf("failed to delete transit gateway: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return fmt.Errorf("failed to delete transit gateway with status: %d", resp.StatusCode)
	}

	var deleteResp struct {
		Return bool   `json:"return"`
		Reason string `json:"reason"`
	}

	if err := json.NewDecoder(resp.Body).Decode(&deleteResp); err != nil {
		return fmt.Errorf("failed to decode delete response: %w", err)
	}

	if !deleteResp.Return {
		return fmt.Errorf("failed to delete transit gateway: %s", deleteResp.Reason)
	}

	return nil
}

// GetGatewayStatus retrieves gateway status
func (c *AviatrixClient) GetGatewayStatus(ctx context.Context, gatewayName string) (*GatewayStatus, error) {
	req := map[string]string{
		"action":  "get_gateway_status",
		"CID":     c.Token,
		"gw_name": gatewayName,
	}

	jsonData, err := json.Marshal(req)
	if err != nil {
		return nil, fmt.Errorf("failed to marshal request: %w", err)
	}

	httpReq, err := http.NewRequestWithContext(ctx, "POST", "https://"+c.ControllerIP+"/v1/api", bytes.NewBuffer(jsonData))
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	httpReq.Header.Set("Content-Type", "application/json")

	resp, err := c.HTTPClient.Do(httpReq)
	if err != nil {
		return nil, fmt.Errorf("failed to get gateway status: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("failed to get gateway status with status: %d", resp.StatusCode)
	}

	var statusResp struct {
		Return  bool          `json:"return"`
		Reason  string        `json:"reason"`
		Results GatewayStatus `json:"results"`
	}

	if err := json.NewDecoder(resp.Body).Decode(&statusResp); err != nil {
		return nil, fmt.Errorf("failed to decode status response: %w", err)
	}

	if !statusResp.Return {
		return nil, fmt.Errorf("failed to get gateway status: %s", statusResp.Reason)
	}

	return &statusResp.Results, nil
}

// GetGatewayMetrics retrieves gateway metrics
func (c *AviatrixClient) GetGatewayMetrics(ctx context.Context, gatewayName string) (*GatewayMetrics, error) {
	req := map[string]string{
		"action":  "get_gateway_metrics",
		"CID":     c.Token,
		"gw_name": gatewayName,
	}

	jsonData, err := json.Marshal(req)
	if err != nil {
		return nil, fmt.Errorf("failed to marshal request: %w", err)
	}

	httpReq, err := http.NewRequestWithContext(ctx, "POST", "https://"+c.ControllerIP+"/v1/api", bytes.NewBuffer(jsonData))
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	httpReq.Header.Set("Content-Type", "application/json")

	resp, err := c.HTTPClient.Do(httpReq)
	if err != nil {
		return nil, fmt.Errorf("failed to get gateway metrics: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("failed to get gateway metrics with status: %d", resp.StatusCode)
	}

	var metricsResp struct {
		Return  bool           `json:"return"`
		Reason  string         `json:"reason"`
		Results GatewayMetrics `json:"results"`
	}

	if err := json.NewDecoder(resp.Body).Decode(&metricsResp); err != nil {
		return nil, fmt.Errorf("failed to decode metrics response: %w", err)
	}

	if !metricsResp.Return {
		return nil, fmt.Errorf("failed to get gateway metrics: %s", metricsResp.Reason)
	}

	return &metricsResp.Results, nil
}