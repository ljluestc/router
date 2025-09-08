package config

import (
	"time"

	"github.com/spf13/viper"
)

// Config represents the application configuration
type Config struct {
	API       APIConfig       `mapstructure:"api"`
	Log       LogConfig       `mapstructure:"log"`
	CloudPods CloudPodsConfig `mapstructure:"cloudpods"`
	Aviatrix  AviatrixConfig  `mapstructure:"aviatrix"`
	Analytics AnalyticsConfig `mapstructure:"analytics"`
	Router    RouterConfig    `mapstructure:"router"`
}

// APIConfig represents API server configuration
type APIConfig struct {
	Port         int           `mapstructure:"port"`
	Host         string        `mapstructure:"host"`
	ReadTimeout  time.Duration `mapstructure:"read_timeout"`
	WriteTimeout time.Duration `mapstructure:"write_timeout"`
	IdleTimeout  time.Duration `mapstructure:"idle_timeout"`
	CORS         CORSConfig    `mapstructure:"cors"`
}

// CORSConfig represents CORS configuration
type CORSConfig struct {
	Enabled      bool     `mapstructure:"enabled"`
	AllowOrigins []string `mapstructure:"allow_origins"`
	AllowMethods []string `mapstructure:"allow_methods"`
	AllowHeaders []string `mapstructure:"allow_headers"`
}

// LogConfig represents logging configuration
type LogConfig struct {
	Level  string `mapstructure:"level"`
	JSON   bool   `mapstructure:"json"`
	File   string `mapstructure:"file"`
	Console bool  `mapstructure:"console"`
}

// CloudPodsConfig represents CloudPods integration configuration
type CloudPodsConfig struct {
	Endpoint     string            `mapstructure:"endpoint"`
	Username     string            `mapstructure:"username"`
	Password     string            `mapstructure:"password"`
	Region       string            `mapstructure:"region"`
	Project      string            `mapstructure:"project"`
	VerifySSL    bool              `mapstructure:"verify_ssl"`
	Timeout      time.Duration     `mapstructure:"timeout"`
	Headers      map[string]string `mapstructure:"headers"`
	RetryCount   int               `mapstructure:"retry_count"`
	RetryDelay   time.Duration     `mapstructure:"retry_delay"`
}

// AviatrixConfig represents Aviatrix integration configuration
type AviatrixConfig struct {
	ControllerIP string            `mapstructure:"controller_ip"`
	Username     string            `mapstructure:"username"`
	Password     string            `mapstructure:"password"`
	APIVersion   string            `mapstructure:"api_version"`
	VerifySSL    bool              `mapstructure:"verify_ssl"`
	Timeout      time.Duration     `mapstructure:"timeout"`
	Region       string            `mapstructure:"region"`
	RetryCount   int               `mapstructure:"retry_count"`
	RetryDelay   time.Duration     `mapstructure:"retry_delay"`
}

// AnalyticsConfig represents analytics configuration
type AnalyticsConfig struct {
	ClickHouse ClickHouseConfig `mapstructure:"clickhouse"`
	Prometheus PrometheusConfig `mapstructure:"prometheus"`
}

// ClickHouseConfig represents ClickHouse configuration
type ClickHouseConfig struct {
	Host     string        `mapstructure:"host"`
	Port     int           `mapstructure:"port"`
	Database string        `mapstructure:"database"`
	Username string        `mapstructure:"username"`
	Password string        `mapstructure:"password"`
	Timeout  time.Duration `mapstructure:"timeout"`
}

// PrometheusConfig represents Prometheus configuration
type PrometheusConfig struct {
	Port     int           `mapstructure:"port"`
	Path     string        `mapstructure:"path"`
	Interval time.Duration `mapstructure:"interval"`
}

// RouterConfig represents router configuration
type RouterConfig struct {
	Interfaces     []InterfaceConfig     `mapstructure:"interfaces"`
	Protocols      []ProtocolConfig      `mapstructure:"protocols"`
	TrafficShaping TrafficShapingConfig  `mapstructure:"traffic_shaping"`
	Impairments    ImpairmentsConfig     `mapstructure:"impairments"`
}

// InterfaceConfig represents network interface configuration
type InterfaceConfig struct {
	Name      string `mapstructure:"name"`
	IPAddress string `mapstructure:"ip_address"`
	SubnetMask string `mapstructure:"subnet_mask"`
	MTU       int    `mapstructure:"mtu"`
	Enabled   bool   `mapstructure:"enabled"`
}

// ProtocolConfig represents routing protocol configuration
type ProtocolConfig struct {
	Type    string            `mapstructure:"type"`
	Enabled bool              `mapstructure:"enabled"`
	Params  map[string]string `mapstructure:"parameters"`
}

// TrafficShapingConfig represents traffic shaping configuration
type TrafficShapingConfig struct {
	Enabled     bool     `mapstructure:"enabled"`
	Algorithms  []string `mapstructure:"algorithms"`
	RateLimit   int64    `mapstructure:"rate_limit"`
	BurstSize   int64    `mapstructure:"burst_size"`
	QueueSize   int      `mapstructure:"queue_size"`
}

// ImpairmentsConfig represents network impairments configuration
type ImpairmentsConfig struct {
	Enabled     bool    `mapstructure:"enabled"`
	Delay       int     `mapstructure:"delay"`
	Jitter      int     `mapstructure:"jitter"`
	PacketLoss  float64 `mapstructure:"packet_loss"`
	Bandwidth   int64   `mapstructure:"bandwidth"`
}

// Load loads configuration from file
func Load(configFile string) (*Config, error) {
	viper.SetConfigFile(configFile)
	viper.SetConfigType("yaml")

	// Set default values
	setDefaults()

	// Read config file
	if err := viper.ReadInConfig(); err != nil {
		return nil, err
	}

	var config Config
	if err := viper.Unmarshal(&config); err != nil {
		return nil, err
	}

	return &config, nil
}

// setDefaults sets default configuration values
func setDefaults() {
	// API defaults
	viper.SetDefault("api.port", 8080)
	viper.SetDefault("api.host", "0.0.0.0")
	viper.SetDefault("api.read_timeout", "30s")
	viper.SetDefault("api.write_timeout", "30s")
	viper.SetDefault("api.idle_timeout", "120s")
	viper.SetDefault("api.cors.enabled", true)
	viper.SetDefault("api.cors.allow_origins", []string{"*"})
	viper.SetDefault("api.cors.allow_methods", []string{"GET", "POST", "PUT", "DELETE", "OPTIONS"})
	viper.SetDefault("api.cors.allow_headers", []string{"*"})

	// Log defaults
	viper.SetDefault("log.level", "info")
	viper.SetDefault("log.json", false)
	viper.SetDefault("log.file", "")
	viper.SetDefault("log.console", true)

	// CloudPods defaults
	viper.SetDefault("cloudpods.endpoint", "https://localhost:8080")
	viper.SetDefault("cloudpods.username", "admin")
	viper.SetDefault("cloudpods.password", "")
	viper.SetDefault("cloudpods.region", "default")
	viper.SetDefault("cloudpods.project", "default")
	viper.SetDefault("cloudpods.verify_ssl", true)
	viper.SetDefault("cloudpods.timeout", "30s")
	viper.SetDefault("cloudpods.retry_count", 3)
	viper.SetDefault("cloudpods.retry_delay", "5s")

	// Aviatrix defaults
	viper.SetDefault("aviatrix.controller_ip", "localhost")
	viper.SetDefault("aviatrix.username", "admin")
	viper.SetDefault("aviatrix.password", "")
	viper.SetDefault("aviatrix.api_version", "v1")
	viper.SetDefault("aviatrix.verify_ssl", true)
	viper.SetDefault("aviatrix.timeout", "30s")
	viper.SetDefault("aviatrix.region", "us-west-1")
	viper.SetDefault("aviatrix.retry_count", 3)
	viper.SetDefault("aviatrix.retry_delay", "5s")

	// Analytics defaults
	viper.SetDefault("analytics.clickhouse.host", "localhost")
	viper.SetDefault("analytics.clickhouse.port", 9000)
	viper.SetDefault("analytics.clickhouse.database", "router_analytics")
	viper.SetDefault("analytics.clickhouse.username", "default")
	viper.SetDefault("analytics.clickhouse.password", "")
	viper.SetDefault("analytics.clickhouse.timeout", "30s")
	viper.SetDefault("analytics.prometheus.port", 9090)
	viper.SetDefault("analytics.prometheus.path", "/metrics")
	viper.SetDefault("analytics.prometheus.interval", "15s")

	// Router defaults
	viper.SetDefault("router.traffic_shaping.enabled", false)
	viper.SetDefault("router.traffic_shaping.algorithms", []string{"token_bucket", "wfq"})
	viper.SetDefault("router.traffic_shaping.rate_limit", 1000000000) // 1 Gbps
	viper.SetDefault("router.traffic_shaping.burst_size", 1000000)    // 1 MB
	viper.SetDefault("router.traffic_shaping.queue_size", 1000)
	viper.SetDefault("router.impairments.enabled", false)
	viper.SetDefault("router.impairments.delay", 0)
	viper.SetDefault("router.impairments.jitter", 0)
	viper.SetDefault("router.impairments.packet_loss", 0.0)
	viper.SetDefault("router.impairments.bandwidth", 0)
}
