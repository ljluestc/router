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
)

// Client represents an Aviatrix API client
type Client struct {
	controllerURL string
	copilotURL    string
	httpClient    *http.Client
	username      string
	password      string
	CID           string
}

// NewClient creates a new Aviatrix client
func NewClient(controllerURL, copilotURL, username, password, cid string) *Client {
	// Configure HTTP client with TLS settings
	tr := &http.Transport{
		TLSClientConfig: &tls.Config{
			InsecureSkipVerify: true, // For demo purposes
		},
	}

	return &Client{
		controllerURL: controllerURL,
		copilotURL:    copilotURL,
		httpClient: &http.Client{
			Transport: tr,
			Timeout:   30 * time.Second,
		},
		username: username,
		password: password,
		CID:      cid,
	}
}

// Authenticate authenticates with Aviatrix Controller
func (c *Client) Authenticate(ctx context.Context) error {
	authReq := map[string]string{
		"action":   "login",
		"username": c.username,
		"password": c.password,
	}

	jsonData, err := json.Marshal(authReq)
	if err != nil {
		return fmt.Errorf("failed to marshal auth request: %w", err)
	}

	req, err := http.NewRequestWithContext(ctx, "POST", 
		c.controllerURL+"/v1/api", bytes.NewBuffer(jsonData))
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
		CID    string `json:"CID"`
	}

	if err := json.NewDecoder(resp.Body).Decode(&authResp); err != nil {
		return fmt.Errorf("failed to decode auth response: %w", err)
	}

	if !authResp.Return {
		return fmt.Errorf("authentication failed: %s", authResp.Reason)
	}

	c.CID = authResp.CID
	return nil
}

// Gateway represents an Aviatrix gateway
type Gateway struct {
	GatewayName    string    `json:"gw_name"`
	CloudType      int       `json:"cloud_type"`
	AccountName    string    `json:"account_name"`
	VPCID          string    `json:"vpc_id"`
	VPCRegion      string    `json:"vpc_reg"`
	GatewaySize    string    `json:"vpc_size"`
	SubnetCIDR     string    `json:"vpc_net"`
	PublicIP       string    `json:"public_ip"`
	PrivateIP      string    `json:"private_ip"`
	State          string    `json:"state"`
	CreatedAt      time.Time `json:"created_at"`
	UpdatedAt      time.Time `json:"updated_at"`
}

// GetGateways retrieves all gateways from Aviatrix
func (c *Client) GetGateways(ctx context.Context) ([]Gateway, error) {
	req := map[string]string{
		"action": "list_vpcs",
		"CID":    c.CID,
	}

	jsonData, err := json.Marshal(req)
	if err != nil {
		return nil, fmt.Errorf("failed to marshal request: %w", err)
	}

	httpReq, err := http.NewRequestWithContext(ctx, "POST", 
		c.controllerURL+"/v1/api", bytes.NewBuffer(jsonData))
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	httpReq.Header.Set("Content-Type", "application/json")

	resp, err := c.httpClient.Do(httpReq)
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