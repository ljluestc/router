package aviatrix

import (
	"bytes"
	"context"
	"encoding/json"
	"fmt"
	"net/http"
	"time"
)

// Client represents an Aviatrix API client
type Client struct {
	controllerIP string
	username     string
	password     string
	httpClient   *http.Client
	sessionID    string
}

// NewClient creates a new Aviatrix client
func NewClient(controllerIP, username, password string) *Client {
	return &Client{
		controllerIP: controllerIP,
		username:     username,
		password:     password,
		httpClient: &http.Client{
			Timeout: 30 * time.Second,
		},
	}
}

// Authenticate authenticates with Aviatrix Controller
func (c *Client) Authenticate(ctx context.Context) error {
	authURL := fmt.Sprintf("https://%s/v1/api", c.controllerIP)
	
	authReq := map[string]string{
		"action":   "login",
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
		Return bool   `json:"return"`
		Reason string `json:"reason"`
		CID    string `json:"cid"`
	}
	
	if err := json.NewDecoder(resp.Body).Decode(&authResp); err != nil {
		return fmt.Errorf("failed to decode auth response: %w", err)
	}
	
	if !authResp.Return {
		return fmt.Errorf("authentication failed: %s", authResp.Reason)
	}
	
	c.sessionID = authResp.CID
	return nil
}

// makeRequest makes an authenticated request to Aviatrix API
func (c *Client) makeRequest(ctx context.Context, action string, params map[string]interface{}) (*http.Response, error) {
	if c.sessionID == "" {
		if err := c.Authenticate(ctx); err != nil {
			return nil, fmt.Errorf("authentication required: %w", err)
		}
	}
	
	apiURL := fmt.Sprintf("https://%s/v1/api", c.controllerIP)
	
	requestData := map[string]interface{}{
		"action": action,
		"CID":    c.sessionID,
	}
	
	// Merge additional parameters
	for k, v := range params {
		requestData[k] = v
	}
	
	jsonData, err := json.Marshal(requestData)
	if err != nil {
		return nil, fmt.Errorf("failed to marshal request: %w", err)
	}
	
	req, err := http.NewRequestWithContext(ctx, "POST", apiURL, bytes.NewBuffer(jsonData))
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}
	
	req.Header.Set("Content-Type", "application/json")
	
	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to make request: %w", err)
	}
	
	return resp, nil
}

// GetGateways retrieves all gateways
func (c *Client) GetGateways(ctx context.Context) ([]Gateway, error) {
	resp, err := c.makeRequest(ctx, "list_vpcs_summary", nil)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()
	
	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("failed to get gateways: status %d", resp.StatusCode)
	}
	
	var result struct {
		Return  bool      `json:"return"`
		Reason  string    `json:"reason"`
		Results []Gateway `json:"results"`
	}
	
	if err := json.NewDecoder(resp.Body).Decode(&result); err != nil {
		return nil, fmt.Errorf("failed to decode gateways: %w", err)
	}
	
	if !result.Return {
		return nil, fmt.Errorf("failed to get gateways: %s", result.Reason)
	}
	
	return result.Results, nil
}

// GetTransitGateways retrieves transit gateways
func (c *Client) GetTransitGateways(ctx context.Context) ([]TransitGateway, error) {
	resp, err := c.makeRequest(ctx, "list_transit_gateways", nil)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()
	
	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("failed to get transit gateways: status %d", resp.StatusCode)
	}
	
	var result struct {
		Return  bool            `json:"return"`
		Reason  string          `json:"reason"`
		Results []TransitGateway `json:"results"`
	}
	
	if err := json.NewDecoder(resp.Body).Decode(&result); err != nil {
		return nil, fmt.Errorf("failed to decode transit gateways: %w", err)
	}
	
	if !result.Return {
		return nil, fmt.Errorf("failed to get transit gateways: %s", result.Reason)
	}
	
	return result.Results, nil
}

// GetSpokeGateways retrieves spoke gateways
func (c *Client) GetSpokeGateways(ctx context.Context) ([]SpokeGateway, error) {
	resp, err := c.makeRequest(ctx, "list_spoke_gateways", nil)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()
	
	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("failed to get spoke gateways: status %d", resp.StatusCode)
	}
	
	var result struct {
		Return  bool           `json:"return"`
		Reason  string         `json:"reason"`
		Results []SpokeGateway `json:"results"`
	}
	
	if err := json.NewDecoder(resp.Body).Decode(&result); err != nil {
		return nil, fmt.Errorf("failed to decode spoke gateways: %w", err)
	}
	
	if !result.Return {
		return nil, fmt.Errorf("failed to get spoke gateways: %s", result.Reason)
	}
	
	return result.Results, nil
}

// GetMetrics retrieves Aviatrix metrics
func (c *Client) GetMetrics(ctx context.Context, gatewayName string, timeRange string) (*Metrics, error) {
	params := map[string]interface{}{
		"gw_name":    gatewayName,
		"time_range": timeRange,
	}
	
	resp, err := c.makeRequest(ctx, "get_gateway_metrics", params)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()
	
	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("failed to get metrics: status %d", resp.StatusCode)
	}
	
	var result struct {
		Return  bool    `json:"return"`
		Reason  string  `json:"reason"`
		Results Metrics `json:"results"`
	}
	
	if err := json.NewDecoder(resp.Body).Decode(&result); err != nil {
		return nil, fmt.Errorf("failed to decode metrics: %w", err)
	}
	
	if !result.Return {
		return nil, fmt.Errorf("failed to get metrics: %s", result.Reason)
	}
	
	return &result.Results, nil
}