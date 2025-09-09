package aviatrix

import (
	"bytes"
	"encoding/json"
	"fmt"
	"io"
	"net/http"
	"time"
)

// Client represents the Aviatrix API client
type Client struct {
	baseURL    string
	username   string
	password   string
	httpClient *http.Client
}

// NewClient creates a new Aviatrix API client
func NewClient(baseURL, username, password string) *Client {
	return &Client{
		baseURL:  baseURL,
		username: username,
		password: password,
		httpClient: &http.Client{
			Timeout: 30 * time.Second,
		},
	}
}

// makeRequest makes an HTTP request to the Aviatrix API
func (c *Client) makeRequest(method, endpoint string, body interface{}) (*http.Response, error) {
	var reqBody io.Reader
	if body != nil {
		jsonBody, err := json.Marshal(body)
		if err != nil {
			return nil, fmt.Errorf("failed to marshal request body: %w", err)
		}
		reqBody = bytes.NewBuffer(jsonBody)
	}

	req, err := http.NewRequest(method, c.baseURL+endpoint, reqBody)
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	req.Header.Set("Content-Type", "application/json")
	req.SetBasicAuth(c.username, c.password)

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to make request: %w", err)
	}

	return resp, nil
}

// CreateGateway creates a gateway using the API client
func (c *Client) CreateGateway(req GatewayRequest) (*Gateway, error) {
	resp, err := c.makeRequest("POST", "/api/v1/gateways", req)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()

	var gateway Gateway
	if err := json.NewDecoder(resp.Body).Decode(&gateway); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	return &gateway, nil
}

// CreateTransitGateway creates a transit gateway using the API client
func (c *Client) CreateTransitGateway(req TransitGatewayRequest) (*TransitGateway, error) {
	resp, err := c.makeRequest("POST", "/api/v1/transit-gateways", req)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()

	var transitGW TransitGateway
	if err := json.NewDecoder(resp.Body).Decode(&transitGW); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	return &transitGW, nil
}

// CreateSpokeGateway creates a spoke gateway using the API client
func (c *Client) CreateSpokeGateway(req SpokeGatewayRequest) (*SpokeGateway, error) {
	resp, err := c.makeRequest("POST", "/api/v1/spoke-gateways", req)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()

	var spokeGW SpokeGateway
	if err := json.NewDecoder(resp.Body).Decode(&spokeGW); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	return &spokeGW, nil
}

// CreateTransitConnection creates a transit connection using the API client
func (c *Client) CreateTransitConnection(req TransitConnectionRequest) (*TransitConnection, error) {
	resp, err := c.makeRequest("POST", "/api/v1/transit-connections", req)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()

	var connection TransitConnection
	if err := json.NewDecoder(resp.Body).Decode(&connection); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	return &connection, nil
}

// ListTransitConnections lists transit connections using the API client
func (c *Client) ListTransitConnections() ([]TransitConnection, error) {
	resp, err := c.makeRequest("GET", "/api/v1/transit-connections", nil)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()

	var result struct {
		Connections []TransitConnection `json:"connections"`
	}
	if err := json.NewDecoder(resp.Body).Decode(&result); err != nil {
		return nil, fmt.Errorf("failed to decode response: %w", err)
	}

	return result.Connections, nil
}

// DeleteTransitConnection deletes a transit connection using the API client
func (c *Client) DeleteTransitConnection(connectionID string) error {
	resp, err := c.makeRequest("DELETE", fmt.Sprintf("/api/v1/transit-connections/%s", connectionID), nil)
	if err != nil {
		return err
	}
	defer resp.Body.Close()

	if resp.StatusCode >= 400 {
		return fmt.Errorf("failed to delete transit connection: status %d", resp.StatusCode)
	}

	return nil
}
