package config

import (
	"fmt"
	"os"
	"time"

	"gopkg.in/yaml.v3"
)

// Config represents the application configuration
type Config struct {
	API       APIConfig       `yaml:"api"`
	Log       LogConfig       `yaml:"log"`
	CloudPods CloudPodsConfig `yaml:"cloudpods"`
	Aviatrix  AviatrixConfig  `yaml:"aviatrix"`
	Analytics AnalyticsConfig `yaml:"analytics"`
	Router    RouterConfig    `yaml:"router"`
	Monitoring MonitoringConfig `yaml:"monitoring"`
}

// APIConfig represents API server configuration
type APIConfig struct {
	Port         int           `yaml:"port"`
	Host         string        `yaml:"host"`
	ReadTimeout  time.Duration `yaml:"read_timeout"`
	WriteTimeout time.Duration `yaml:"write_timeout"`
	IdleTimeout  time.Duration `yaml:"idle_timeout"`
	CORS         CORSConfig    `yaml:"cors"`
}

// CORSConfig represents CORS configuration
type CORSConfig struct {
	Enabled      bool     `yaml:"enabled"`
	AllowOrigins []string `yaml:"allow_origins"`
	AllowMethods []string `yaml:"allow_methods"`
	AllowHeaders []string `yaml:"allow_headers"`
}

// LogConfig represents logging configuration
type LogConfig struct {
	Level string `yaml:"level"`
	JSON  bool   `yaml:"json"`
	File  string `yaml:"file"`
}

// CloudPodsConfig represents CloudPods integration configuration
type CloudPodsConfig struct {
	Endpoint    string            `yaml:"endpoint"`
	Username    string            `yaml:"username"`
	Password    string            `yaml:"password"`
	Region      string            `yaml:"region"`
	Project     string            `yaml:"project"`
	VerifySSL   bool              `yaml:"verify_ssl"`
	Timeout     time.Duration     `yaml:"timeout"`
	Headers     map[string]string `yaml:"headers"`
	RetryCount  int               `yaml:"retry_count"`
	RetryDelay  time.Duration     `yaml:"retry_delay"`
}

// AviatrixConfig represents Aviatrix integration configuration
type AviatrixConfig struct {
	ControllerIP string        `yaml:"controller_ip"`
	Username     string        `yaml:"username"`
	Password     string        `yaml:"password"`
	APIVersion   string        `yaml:"api_version"`
	VerifySSL    bool          `yaml:"verify_ssl"`
	Timeout      time.Duration `yaml:"timeout"`
	Region       string        `yaml:"region"`
	RetryCount   int           `yaml:"retry_count"`
	RetryDelay   time.Duration `yaml:"retry_delay"`
}

// AnalyticsConfig represents analytics configuration
type AnalyticsConfig struct {
	ClickHouse ClickHouseConfig `yaml:"clickhouse"`
	Enabled    bool             `yaml:"enabled"`
	Retention  time.Duration    `yaml:"retention"`
}

// ClickHouseConfig represents ClickHouse configuration
type ClickHouseConfig struct {
	Host            string        `yaml:"host"`
	Port            int           `yaml:"port"`
	Database        string        `yaml:"database"`
	Username        string        `yaml:"username"`
	Password        string        `yaml:"password"`
	MaxConnections  int           `yaml:"max_connections"`
	BatchSize       int           `yaml:"batch_size"`
	FlushInterval   time.Duration `yaml:"flush_interval"`
	Compression     bool          `yaml:"compression"`
	MaxQueueSize    int           `yaml:"max_queue_size"`
}

// RouterConfig represents router configuration
type RouterConfig struct {
	Interfaces     []InterfaceConfig     `yaml:"interfaces"`
	Protocols      ProtocolsConfig       `yaml:"protocols"`
	TrafficShaping TrafficShapingConfig  `yaml:"traffic_shaping"`
	Impairments    ImpairmentsConfig     `yaml:"impairments"`
}

// InterfaceConfig represents network interface configuration
type InterfaceConfig struct {
	Name        string `yaml:"name"`
	Type        string `yaml:"type"`
	IP          string `yaml:"ip"`
	Subnet      string `yaml:"subnet"`
	Gateway     string `yaml:"gateway"`
	MTU         int    `yaml:"mtu"`
	Enabled     bool   `yaml:"enabled"`
	Description string `yaml:"description"`
}

// ProtocolsConfig represents routing protocols configuration
type ProtocolsConfig struct {
	BGP  BGPConfig  `yaml:"bgp"`
	OSPF OSPFConfig `yaml:"ospf"`
	ISIS ISISConfig `yaml:"isis"`
}

// BGPConfig represents BGP configuration
type BGPConfig struct {
	Enabled   bool          `yaml:"enabled"`
	ASN       uint32        `yaml:"asn"`
	RouterID  string        `yaml:"router_id"`
	Neighbors []BGPNeighbor `yaml:"neighbors"`
	Policies  []BGPPolicy   `yaml:"policies"`
	Communities map[string]string `yaml:"communities"`
}

// BGPNeighbor represents BGP neighbor configuration
type BGPNeighbor struct {
	IP          string `yaml:"ip"`
	ASN         uint32 `yaml:"asn"`
	Password    string `yaml:"password"`
	Description string `yaml:"description"`
	Enabled     bool   `yaml:"enabled"`
}

// BGPPolicy represents BGP policy configuration
type BGPPolicy struct {
	Name        string   `yaml:"name"`
	Type        string   `yaml:"type"`
	Match       []string `yaml:"match"`
	Action      string   `yaml:"action"`
	Set         []string `yaml:"set"`
	Description string   `yaml:"description"`
}

// OSPFConfig represents OSPF configuration
type OSPFConfig struct {
	Enabled     bool         `yaml:"enabled"`
	RouterID    string       `yaml:"router_id"`
	Areas       []OSPFArea   `yaml:"areas"`
	Interfaces  []OSPFInterface `yaml:"interfaces"`
	Redistribute []string    `yaml:"redistribute"`
}

// OSPFArea represents OSPF area configuration
type OSPFArea struct {
	ID      int    `yaml:"id"`
	Type    string `yaml:"type"`
	Stub    bool   `yaml:"stub"`
	NSSA    bool   `yaml:"nssa"`
	Summary bool   `yaml:"summary"`
}

// OSPFInterface represents OSPF interface configuration
type OSPFInterface struct {
	Name     string `yaml:"name"`
	Area     int    `yaml:"area"`
	Cost     int    `yaml:"cost"`
	Priority int    `yaml:"priority"`
	Enabled  bool   `yaml:"enabled"`
}

// ISISConfig represents ISIS configuration
type ISISConfig struct {
	Enabled     bool           `yaml:"enabled"`
	SystemID    string         `yaml:"system_id"`
	Level       int            `yaml:"level"`
	Interfaces  []ISISInterface `yaml:"interfaces"`
	Redistribute []string      `yaml:"redistribute"`
}

// ISISInterface represents ISIS interface configuration
type ISISInterface struct {
	Name    string `yaml:"name"`
	Level   int    `yaml:"level"`
	Cost    int    `yaml:"cost"`
	Enabled bool   `yaml:"enabled"`
}

// TrafficShapingConfig represents traffic shaping configuration
type TrafficShapingConfig struct {
	Enabled    bool              `yaml:"enabled"`
	Algorithms []ShapingAlgorithm `yaml:"algorithms"`
	Policies   []ShapingPolicy   `yaml:"policies"`
	GlobalLimit int64            `yaml:"global_limit"`
}

// ShapingAlgorithm represents traffic shaping algorithm configuration
type ShapingAlgorithm struct {
	Name       string                 `yaml:"name"`
	Type       string                 `yaml:"type"`
	Parameters map[string]interface{} `yaml:"parameters"`
	Enabled    bool                   `yaml:"enabled"`
}

// ShapingPolicy represents traffic shaping policy configuration
type ShapingPolicy struct {
	Name        string   `yaml:"name"`
	Match       []string `yaml:"match"`
	Action      string   `yaml:"action"`
	Rate        int64    `yaml:"rate"`
	Burst       int64    `yaml:"burst"`
	Priority    int      `yaml:"priority"`
	Description string   `yaml:"description"`
}

// ImpairmentsConfig represents network impairments configuration
type ImpairmentsConfig struct {
	Enabled    bool                `yaml:"enabled"`
	Algorithms []ImpairmentAlgorithm `yaml:"algorithms"`
	Policies   []ImpairmentPolicy  `yaml:"policies"`
}

// ImpairmentAlgorithm represents impairment algorithm configuration
type ImpairmentAlgorithm struct {
	Name       string                 `yaml:"name"`
	Type       string                 `yaml:"type"`
	Parameters map[string]interface{} `yaml:"parameters"`
	Enabled    bool                   `yaml:"enabled"`
}

// ImpairmentPolicy represents impairment policy configuration
type ImpairmentPolicy struct {
	Name        string                 `yaml:"name"`
	Match       []string               `yaml:"match"`
	Algorithm   string                 `yaml:"algorithm"`
	Parameters  map[string]interface{} `yaml:"parameters"`
	Enabled     bool                   `yaml:"enabled"`
	Description string                 `yaml:"description"`
}

// MonitoringConfig represents monitoring configuration
type MonitoringConfig struct {
	Enabled   bool              `yaml:"enabled"`
	Prometheus PrometheusConfig `yaml:"prometheus"`
	Metrics   MetricsConfig     `yaml:"metrics"`
	Logging   LoggingConfig     `yaml:"logging"`
	Alerting  AlertingConfig    `yaml:"alerting"`
}

// PrometheusConfig represents Prometheus configuration
type PrometheusConfig struct {
	Enabled bool   `yaml:"enabled"`
	Port    int    `yaml:"port"`
	Path    string `yaml:"path"`
}

// MetricsConfig represents metrics configuration
type MetricsConfig struct {
	Interval  time.Duration `yaml:"interval"`
	Retention time.Duration `yaml:"retention"`
	Exporters []string      `yaml:"exporters"`
	Collectors []string     `yaml:"collectors"`
}

// LoggingConfig represents logging configuration
type LoggingConfig struct {
	Level      string        `yaml:"level"`
	Format     string        `yaml:"format"`
	Outputs    []string      `yaml:"outputs"`
	Rotation   time.Duration `yaml:"rotation"`
	MaxSize    int64         `yaml:"max_size"`
	MaxBackups int           `yaml:"max_backups"`
	MaxAge     int           `yaml:"max_age"`
}

// AlertingConfig represents alerting configuration
type AlertingConfig struct {
	Enabled   bool          `yaml:"enabled"`
	Rules     []AlertRule   `yaml:"rules"`
	Receivers []string      `yaml:"receivers"`
	Webhooks  []string      `yaml:"webhooks"`
}

// AlertRule represents alert rule configuration
type AlertRule struct {
	Name        string            `yaml:"name"`
	Condition   string            `yaml:"condition"`
	Severity    string            `yaml:"severity"`
	Message     string            `yaml:"message"`
	Labels      map[string]string `yaml:"labels"`
	Annotations map[string]string `yaml:"annotations"`
}

// Load loads configuration from file
func Load(path string) (*Config, error) {
	data, err := os.ReadFile(path)
	if err != nil {
		return nil, fmt.Errorf("failed to read config file: %w", err)
	}

	var config Config
	if err := yaml.Unmarshal(data, &config); err != nil {
		return nil, fmt.Errorf("failed to parse config file: %w", err)
	}

	return &config, nil
}

// Default returns default configuration
func Default() *Config {
	return &Config{
		API: APIConfig{
			Port:         8080,
			Host:         "0.0.0.0",
			ReadTimeout:  30 * time.Second,
			WriteTimeout: 30 * time.Second,
			IdleTimeout:  120 * time.Second,
			CORS: CORSConfig{
				Enabled:      true,
				AllowOrigins: []string{"*"},
				AllowMethods: []string{"GET", "POST", "PUT", "DELETE", "OPTIONS"},
				AllowHeaders: []string{"*"},
			},
		},
		Log: LogConfig{
			Level: "info",
			JSON:  false,
			File:  "",
		},
		Analytics: AnalyticsConfig{
			ClickHouse: ClickHouseConfig{
				Host:           "localhost",
				Port:           9000,
				Database:       "router_analytics",
				Username:       "default",
				Password:       "",
				MaxConnections: 10,
				BatchSize:      1000,
				FlushInterval:  time.Second,
				Compression:    true,
				MaxQueueSize:   100000,
			},
			Enabled:   true,
			Retention: 24 * time.Hour,
		},
	}
}
