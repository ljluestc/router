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
	controllerURL string
	copilotURL    string
	httpClient    *http.Client
	auth          AuthConfig
}

// AuthConfig holds authentication configuration
type AuthConfig struct {
	ControllerUsername string `json:"controller_username"`
	ControllerPassword string `json:"controller_password"`
	CopilotUsername    string `json:"copilot_username"`
	CopilotPassword    string `json:"copilot_password"`
	APIKey             string `json:"api_key"`
}

// Config holds Aviatrix client configuration
type Config struct {
	ControllerURL string        `json:"controller_url"`
	CopilotURL    string        `json:"copilot_url"`
	Timeout       time.Duration `json:"timeout"`
	Auth          AuthConfig    `json:"auth"`
}

// NewClient creates a new Aviatrix client
func NewClient(config Config) *Client {
	return &Client{
		controllerURL: config.ControllerURL,
		copilotURL:    config.CopilotURL,
		httpClient: &http.Client{
			Timeout: config.Timeout,
		},
		auth: config.Auth,
	}
}