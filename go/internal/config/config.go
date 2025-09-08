package config

import (
	"fmt"
	"os"
	"time"

	"github.com/spf13/viper"
)

// Config represents the application configuration
type Config struct {
	Server    ServerConfig    `mapstructure:"server"`
	CloudPods CloudPodsConfig `mapstructure:"cloudpods"`
	Aviatrix  AviatrixConfig `mapstructure:"aviatrix"`
	Database  DatabaseConfig  `mapstructure:"database"`
	Logging   LoggingConfig   `mapstructure:"logging"`
}

// ServerConfig represents the server configuration
type ServerConfig struct {
	Address         string        `mapstructure:"address"`
	Port            int           `mapstructure:"port"`
	ReadTimeout     time.Duration `mapstructure:"read_timeout"`
	WriteTimeout    time.Duration `mapstructure:"write_timeout"`
	IdleTimeout     time.Duration `mapstructure:"idle_timeout"`
	ShutdownTimeout time.Duration `mapstructure:"shutdown_timeout"`
	MaxHeaderBytes  int           `mapstructure:"max_header_bytes"`
}

// CloudPodsConfig represents the CloudPods configuration
type CloudPodsConfig struct {
	BaseURL string `mapstructure:"base_url"`
	APIKey  string `mapstructure:"api_key"`
	Timeout int    `mapstructure:"timeout"`
	Enabled bool   `mapstructure:"enabled"`
}

// AviatrixConfig represents the Aviatrix configuration
type AviatrixConfig struct {
	BaseURL string `mapstructure:"base_url"`
	APIKey  string `mapstructure:"api_key"`
	Timeout int    `mapstructure:"timeout"`
	Enabled bool   `mapstructure:"enabled"`
}

// DatabaseConfig represents the database configuration
type DatabaseConfig struct {
	Host     string `mapstructure:"host"`
	Port     int    `mapstructure:"port"`
	Username string `mapstructure:"username"`
	Password string `mapstructure:"password"`
	Database string `mapstructure:"database"`
	SSLMode  string `mapstructure:"ssl_mode"`
}

// LoggingConfig represents the logging configuration
type LoggingConfig struct {
	Level  string `mapstructure:"level"`
	Format string `mapstructure:"format"`
	Output string `mapstructure:"output"`
}

// Load loads the configuration from file and environment variables
func Load(configFile string) (*Config, error) {
	// Set default values
	setDefaults()

	// Set config file
	if configFile != "" {
		viper.SetConfigFile(configFile)
	} else {
		viper.SetConfigName("config")
		viper.SetConfigType("yaml")
		viper.AddConfigPath(".")
		viper.AddConfigPath("./config")
		viper.AddConfigPath("/etc/router-sim")
	}

	// Enable reading from environment variables
	viper.AutomaticEnv()

	// Read config file
	if err := viper.ReadInConfig(); err != nil {
		if _, ok := err.(viper.ConfigFileNotFoundError); !ok {
			return nil, fmt.Errorf("failed to read config file: %w", err)
		}
	}

	// Unmarshal config
	var config Config
	if err := viper.Unmarshal(&config); err != nil {
		return nil, fmt.Errorf("failed to unmarshal config: %w", err)
	}

	// Validate config
	if err := validateConfig(&config); err != nil {
		return nil, fmt.Errorf("invalid configuration: %w", err)
	}

	return &config, nil
}

// setDefaults sets default configuration values
func setDefaults() {
	// Server defaults
	viper.SetDefault("server.address", "0.0.0.0")
	viper.SetDefault("server.port", 8080)
	viper.SetDefault("server.read_timeout", "30s")
	viper.SetDefault("server.write_timeout", "30s")
	viper.SetDefault("server.idle_timeout", "120s")
	viper.SetDefault("server.shutdown_timeout", "30s")
	viper.SetDefault("server.max_header_bytes", 1048576)

	// CloudPods defaults
	viper.SetDefault("cloudpods.base_url", "http://localhost:8080")
	viper.SetDefault("cloudpods.timeout", 30)
	viper.SetDefault("cloudpods.enabled", true)

	// Aviatrix defaults
	viper.SetDefault("aviatrix.base_url", "https://api.aviatrix.com")
	viper.SetDefault("aviatrix.timeout", 30)
	viper.SetDefault("aviatrix.enabled", true)

	// Database defaults
	viper.SetDefault("database.host", "localhost")
	viper.SetDefault("database.port", 5432)
	viper.SetDefault("database.username", "router_sim")
	viper.SetDefault("database.password", "password")
	viper.SetDefault("database.database", "router_sim")
	viper.SetDefault("database.ssl_mode", "disable")

	// Logging defaults
	viper.SetDefault("logging.level", "info")
	viper.SetDefault("logging.format", "json")
	viper.SetDefault("logging.output", "stdout")

	// Environment variable mappings
	viper.BindEnv("server.address", "SERVER_ADDRESS")
	viper.BindEnv("server.port", "SERVER_PORT")
	viper.BindEnv("cloudpods.base_url", "CLOUDPODS_BASE_URL")
	viper.BindEnv("cloudpods.api_key", "CLOUDPODS_API_KEY")
	viper.BindEnv("aviatrix.base_url", "AVIATRIX_BASE_URL")
	viper.BindEnv("aviatrix.api_key", "AVIATRIX_API_KEY")
	viper.BindEnv("database.host", "DB_HOST")
	viper.BindEnv("database.port", "DB_PORT")
	viper.BindEnv("database.username", "DB_USERNAME")
	viper.BindEnv("database.password", "DB_PASSWORD")
	viper.BindEnv("database.database", "DB_DATABASE")
}

// validateConfig validates the configuration
func validateConfig(config *Config) error {
	// Validate server config
	if config.Server.Address == "" {
		return fmt.Errorf("server address cannot be empty")
	}
	if config.Server.Port <= 0 || config.Server.Port > 65535 {
		return fmt.Errorf("server port must be between 1 and 65535")
	}

	// Validate CloudPods config
	if config.CloudPods.Enabled {
		if config.CloudPods.BaseURL == "" {
			return fmt.Errorf("cloudpods base_url cannot be empty when enabled")
		}
		if config.CloudPods.APIKey == "" {
			return fmt.Errorf("cloudpods api_key cannot be empty when enabled")
		}
	}

	// Validate Aviatrix config
	if config.Aviatrix.Enabled {
		if config.Aviatrix.BaseURL == "" {
			return fmt.Errorf("aviatrix base_url cannot be empty when enabled")
		}
		if config.Aviatrix.APIKey == "" {
			return fmt.Errorf("aviatrix api_key cannot be empty when enabled")
		}
	}

	// Validate database config
	if config.Database.Host == "" {
		return fmt.Errorf("database host cannot be empty")
	}
	if config.Database.Port <= 0 || config.Database.Port > 65535 {
		return fmt.Errorf("database port must be between 1 and 65535")
	}
	if config.Database.Username == "" {
		return fmt.Errorf("database username cannot be empty")
	}
	if config.Database.Database == "" {
		return fmt.Errorf("database name cannot be empty")
	}

	return nil
}

// GetServerAddress returns the full server address
func (c *Config) GetServerAddress() string {
	return fmt.Sprintf("%s:%d", c.Server.Address, c.Server.Port)
}

// IsCloudPodsEnabled returns true if CloudPods integration is enabled
func (c *Config) IsCloudPodsEnabled() bool {
	return c.CloudPods.Enabled
}

// IsAviatrixEnabled returns true if Aviatrix integration is enabled
func (c *Config) IsAviatrixEnabled() bool {
	return c.Aviatrix.Enabled
}

// GetCloudPodsAPIKey returns the CloudPods API key from environment or config
func (c *Config) GetCloudPodsAPIKey() string {
	if apiKey := os.Getenv("CLOUDPODS_API_KEY"); apiKey != "" {
		return apiKey
	}
	return c.CloudPods.APIKey
}

// GetAviatrixAPIKey returns the Aviatrix API key from environment or config
func (c *Config) GetAviatrixAPIKey() string {
	if apiKey := os.Getenv("AVIATRIX_API_KEY"); apiKey != "" {
		return apiKey
	}
	return c.Aviatrix.APIKey
}
