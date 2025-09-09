package config

import (
	"fmt"
	"time"

	"github.com/spf13/viper"
)

// Config represents the application configuration
type Config struct {
	Server    ServerConfig    `mapstructure:"server"`
	Analytics AnalyticsConfig `mapstructure:"analytics"`
	Aviatrix  AviatrixConfig  `mapstructure:"aviatrix"`
	CloudPods CloudPodsConfig `mapstructure:"cloudpods"`
	Router    RouterConfig    `mapstructure:"router"`
}

// ServerConfig represents the HTTP server configuration
type ServerConfig struct {
	Port         int           `mapstructure:"port"`
	Host         string        `mapstructure:"host"`
	ReadTimeout  time.Duration `mapstructure:"read_timeout"`
	WriteTimeout time.Duration `mapstructure:"write_timeout"`
	IdleTimeout  time.Duration `mapstructure:"idle_timeout"`
	EnableCORS   bool          `mapstructure:"enable_cors"`
	EnablePprof  bool          `mapstructure:"enable_pprof"`
}

// AnalyticsConfig represents the analytics service configuration
type AnalyticsConfig struct {
	ClickHouse ClickHouseConfig `mapstructure:"clickhouse"`
	Prometheus PrometheusConfig `mapstructure:"prometheus"`
	Enabled    bool             `mapstructure:"enabled"`
}

// ClickHouseConfig represents ClickHouse configuration
type ClickHouseConfig struct {
	Host     string `mapstructure:"host"`
	Port     int    `mapstructure:"port"`
	Database string `mapstructure:"database"`
	Username string `mapstructure:"username"`
	Password string `mapstructure:"password"`
	SSL      bool   `mapstructure:"ssl"`
}

// PrometheusConfig represents Prometheus configuration
type PrometheusConfig struct {
	Enabled bool   `mapstructure:"enabled"`
	Path    string `mapstructure:"path"`
	Port    int    `mapstructure:"port"`
}

// AviatrixConfig represents Aviatrix integration configuration
type AviatrixConfig struct {
	Controller ControllerConfig `mapstructure:"controller"`
	CoPilot    CoPilotConfig    `mapstructure:"copilot"`
	Enabled    bool             `mapstructure:"enabled"`
	Timeout    time.Duration    `mapstructure:"timeout"`
}

// ControllerConfig represents Aviatrix Controller configuration
type ControllerConfig struct {
	Host     string `mapstructure:"host"`
	Port     int    `mapstructure:"port"`
	Username string `mapstructure:"username"`
	Password string `mapstructure:"password"`
	SSL      bool   `mapstructure:"ssl"`
}

// CoPilotConfig represents Aviatrix CoPilot configuration
type CoPilotConfig struct {
	Host     string `mapstructure:"host"`
	Port     int    `mapstructure:"port"`
	Username string `mapstructure:"username"`
	Password string `mapstructure:"password"`
	SSL      bool   `mapstructure:"ssl"`
}

// CloudPodsConfig represents CloudPods integration configuration
type CloudPodsConfig struct {
	API      APIConfig      `mapstructure:"api"`
	Auth     AuthConfig     `mapstructure:"auth"`
	Enabled  bool           `mapstructure:"enabled"`
	Timeout  time.Duration  `mapstructure:"timeout"`
	Region   string         `mapstructure:"region"`
	Projects []ProjectConfig `mapstructure:"projects"`
}

// APIConfig represents API configuration
type APIConfig struct {
	BaseURL string `mapstructure:"base_url"`
	Version string `mapstructure:"version"`
	Timeout time.Duration `mapstructure:"timeout"`
}

// AuthConfig represents authentication configuration
type AuthConfig struct {
	Type     string `mapstructure:"type"` // "basic", "oauth2", "api_key"
	Username string `mapstructure:"username"`
	Password string `mapstructure:"password"`
	APIKey   string `mapstructure:"api_key"`
	Token    string `mapstructure:"token"`
}

// ProjectConfig represents CloudPods project configuration
type ProjectConfig struct {
	ID          string `mapstructure:"id"`
	Name        string `mapstructure:"name"`
	Description string `mapstructure:"description"`
	Region      string `mapstructure:"region"`
}

// RouterConfig represents router simulation configuration
type RouterConfig struct {
	Interfaces    []InterfaceConfig    `mapstructure:"interfaces"`
	Protocols     []ProtocolConfig     `mapstructure:"protocols"`
	TrafficShaping TrafficShapingConfig `mapstructure:"traffic_shaping"`
	Impairments   []ImpairmentConfig   `mapstructure:"impairments"`
}

// InterfaceConfig represents network interface configuration
type InterfaceConfig struct {
	Name      string `mapstructure:"name"`
	IPAddress string `mapstructure:"ip_address"`
	Netmask   string `mapstructure:"netmask"`
	Enabled   bool   `mapstructure:"enabled"`
	MTU       int    `mapstructure:"mtu"`
}

// ProtocolConfig represents routing protocol configuration
type ProtocolConfig struct {
	Type       string                 `mapstructure:"type"` // "bgp", "ospf", "isis"
	Name       string                 `mapstructure:"name"`
	Enabled    bool                   `mapstructure:"enabled"`
	Parameters map[string]interface{} `mapstructure:"parameters"`
}

// TrafficShapingConfig represents traffic shaping configuration
type TrafficShapingConfig struct {
	Algorithm string                 `mapstructure:"algorithm"`
	Rate      int64                  `mapstructure:"rate"`      // bits per second
	BurstSize int64                  `mapstructure:"burst_size"` // bytes
	Enabled   bool                   `mapstructure:"enabled"`
	Parameters map[string]interface{} `mapstructure:"parameters"`
}

// ImpairmentConfig represents network impairment configuration
type ImpairmentConfig struct {
	Interface string                 `mapstructure:"interface"`
	Type      string                 `mapstructure:"type"` // "delay", "loss", "bandwidth", "reorder"
	Enabled   bool                   `mapstructure:"enabled"`
	Parameters map[string]interface{} `mapstructure:"parameters"`
}

// Load loads configuration from file
func Load(filename string) (*Config, error) {
	viper.SetConfigFile(filename)
	viper.SetConfigType("yaml")

	// Set default values
	setDefaults()

	// Read config file
	if err := viper.ReadInConfig(); err != nil {
		return nil, fmt.Errorf("failed to read config file: %w", err)
	}

	// Unmarshal config
	var config Config
	if err := viper.Unmarshal(&config); err != nil {
		return nil, fmt.Errorf("failed to unmarshal config: %w", err)
	}

	// Validate config
	if err := validate(&config); err != nil {
		return nil, fmt.Errorf("config validation failed: %w", err)
	}

	return &config, nil
}

// setDefaults sets default configuration values
func setDefaults() {
	// Server defaults
	viper.SetDefault("server.port", 8080)
	viper.SetDefault("server.host", "0.0.0.0")
	viper.SetDefault("server.read_timeout", "30s")
	viper.SetDefault("server.write_timeout", "30s")
	viper.SetDefault("server.idle_timeout", "120s")
	viper.SetDefault("server.enable_cors", true)
	viper.SetDefault("server.enable_pprof", false)

	// Analytics defaults
	viper.SetDefault("analytics.enabled", true)
	viper.SetDefault("analytics.clickhouse.host", "localhost")
	viper.SetDefault("analytics.clickhouse.port", 9000)
	viper.SetDefault("analytics.clickhouse.database", "router_sim")
	viper.SetDefault("analytics.clickhouse.username", "default")
	viper.SetDefault("analytics.clickhouse.password", "")
	viper.SetDefault("analytics.clickhouse.ssl", false)
	viper.SetDefault("analytics.prometheus.enabled", true)
	viper.SetDefault("analytics.prometheus.path", "/metrics")
	viper.SetDefault("analytics.prometheus.port", 9090)

	// Aviatrix defaults
	viper.SetDefault("aviatrix.enabled", false)
	viper.SetDefault("aviatrix.timeout", "30s")
	viper.SetDefault("aviatrix.controller.port", 443)
	viper.SetDefault("aviatrix.controller.ssl", true)
	viper.SetDefault("aviatrix.copilot.port", 443)
	viper.SetDefault("aviatrix.copilot.ssl", true)

	// CloudPods defaults
	viper.SetDefault("cloudpods.enabled", false)
	viper.SetDefault("cloudpods.timeout", "30s")
	viper.SetDefault("cloudpods.api.version", "v1")
	viper.SetDefault("cloudpods.api.timeout", "30s")
	viper.SetDefault("cloudpods.auth.type", "basic")

	// Router defaults
	viper.SetDefault("router.traffic_shaping.algorithm", "token_bucket")
	viper.SetDefault("router.traffic_shaping.rate", 1000000) // 1 Mbps
	viper.SetDefault("router.traffic_shaping.burst_size", 10000) // 10 KB
	viper.SetDefault("router.traffic_shaping.enabled", true)
}

// validate validates the configuration
func validate(config *Config) error {
	// Validate server config
	if config.Server.Port <= 0 || config.Server.Port > 65535 {
		return fmt.Errorf("invalid server port: %d", config.Server.Port)
	}

	// Validate analytics config
	if config.Analytics.Enabled {
		if config.Analytics.ClickHouse.Host == "" {
			return fmt.Errorf("clickhouse host is required when analytics is enabled")
		}
		if config.Analytics.ClickHouse.Port <= 0 || config.Analytics.ClickHouse.Port > 65535 {
			return fmt.Errorf("invalid clickhouse port: %d", config.Analytics.ClickHouse.Port)
		}
	}

	// Validate Aviatrix config
	if config.Aviatrix.Enabled {
		if config.Aviatrix.Controller.Host == "" {
			return fmt.Errorf("aviatrix controller host is required when aviatrix is enabled")
		}
		if config.Aviatrix.Controller.Username == "" {
			return fmt.Errorf("aviatrix controller username is required")
		}
		if config.Aviatrix.Controller.Password == "" {
			return fmt.Errorf("aviatrix controller password is required")
		}
	}

	// Validate CloudPods config
	if config.CloudPods.Enabled {
		if config.CloudPods.API.BaseURL == "" {
			return fmt.Errorf("cloudpods api base url is required when cloudpods is enabled")
		}
		if config.CloudPods.Auth.Username == "" {
			return fmt.Errorf("cloudpods username is required")
		}
		if config.CloudPods.Auth.Password == "" {
			return fmt.Errorf("cloudpods password is required")
		}
	}

	return nil
}