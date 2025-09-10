package config

import (
	"os"
	"strconv"
	"time"
)

// Config represents the application configuration
type Config struct {
	Server    ServerConfig    `json:"server"`
	Analytics AnalyticsConfig `json:"analytics"`
	Aviatrix  AviatrixConfig  `json:"aviatrix"`
	CloudPods CloudPodsConfig `json:"cloudpods"`
}

// ServerConfig contains server configuration
type ServerConfig struct {
	Port        int    `json:"port"`
	Environment string `json:"environment"`
	Host        string `json:"host"`
}

// AnalyticsConfig contains analytics configuration
type AnalyticsConfig struct {
	ClickHouse ClickHouseConfig `json:"clickhouse"`
	Enabled    bool             `json:"enabled"`
}

// ClickHouseConfig contains ClickHouse configuration
type ClickHouseConfig struct {
	Host     string `json:"host"`
	Port     int    `json:"port"`
	Database string `json:"database"`
	Username string `json:"username"`
	Password string `json:"password"`
}

// AviatrixConfig contains Aviatrix configuration
type AviatrixConfig struct {
	ControllerURL string        `json:"controller_url"`
	Username      string        `json:"username"`
	Password      string        `json:"password"`
	Timeout       time.Duration `json:"timeout"`
}

// CloudPodsConfig contains CloudPods configuration
type CloudPodsConfig struct {
	APIURL   string        `json:"api_url"`
	Token    string        `json:"token"`
	Timeout  time.Duration `json:"timeout"`
}

// Load loads configuration from environment variables
func Load() (*Config, error) {
	config := &Config{
		Server: ServerConfig{
			Port:        getEnvInt("SERVER_PORT", 8080),
			Environment: getEnv("SERVER_ENV", "development"),
			Host:        getEnv("SERVER_HOST", "0.0.0.0"),
		},
		Analytics: AnalyticsConfig{
			ClickHouse: ClickHouseConfig{
				Host:     getEnv("CLICKHOUSE_HOST", "localhost"),
				Port:     getEnvInt("CLICKHOUSE_PORT", 9000),
				Database: getEnv("CLICKHOUSE_DATABASE", "router_sim"),
				Username: getEnv("CLICKHOUSE_USERNAME", "default"),
				Password: getEnv("CLICKHOUSE_PASSWORD", ""),
			},
			Enabled: getEnvBool("ANALYTICS_ENABLED", true),
		},
		Aviatrix: AviatrixConfig{
			ControllerURL: getEnv("AVIATRIX_CONTROLLER_URL", "https://controller.aviatrix.com"),
			Username:      getEnv("AVIATRIX_USERNAME", ""),
			Password:      getEnv("AVIATRIX_PASSWORD", ""),
			Timeout:       time.Duration(getEnvInt("AVIATRIX_TIMEOUT", 30)) * time.Second,
		},
		CloudPods: CloudPodsConfig{
			APIURL:  getEnv("CLOUDPODS_API_URL", "https://api.cloudpods.com"),
			Token:   getEnv("CLOUDPODS_TOKEN", ""),
			Timeout: time.Duration(getEnvInt("CLOUDPODS_TIMEOUT", 30)) * time.Second,
		},
	}

	return config, nil
}

// getEnv gets an environment variable with a default value
func getEnv(key, defaultValue string) string {
	if value := os.Getenv(key); value != "" {
		return value
	}
	return defaultValue
}

// getEnvInt gets an environment variable as integer with a default value
func getEnvInt(key string, defaultValue int) int {
	if value := os.Getenv(key); value != "" {
		if intValue, err := strconv.Atoi(value); err == nil {
			return intValue
		}
	}
	return defaultValue
}

// getEnvBool gets an environment variable as boolean with a default value
func getEnvBool(key string, defaultValue bool) bool {
	if value := os.Getenv(key); value != "" {
		if boolValue, err := strconv.ParseBool(value); err == nil {
			return boolValue
		}
	}
	return defaultValue
}
