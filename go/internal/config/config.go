package config

import (
	"time"
)

// Config represents the application configuration
type Config struct {
	API       API       `yaml:"api"`
	Log       Log       `yaml:"log"`
	CloudPods CloudPods `yaml:"cloudpods"`
	Aviatrix  Aviatrix  `yaml:"aviatrix"`
	Monitoring Monitoring `yaml:"monitoring"`
}

// API configuration
type API struct {
	Host         string        `yaml:"host"`
	Port         int           `yaml:"port"`
	ReadTimeout  time.Duration `yaml:"read_timeout"`
	WriteTimeout time.Duration `yaml:"write_timeout"`
	IdleTimeout  time.Duration `yaml:"idle_timeout"`
	CORS         CORS         `yaml:"cors"`
}

// CORS configuration
type CORS struct {
	AllowOrigins []string `yaml:"allow_origins"`
	AllowMethods []string `yaml:"allow_methods"`
	AllowHeaders []string `yaml:"allow_headers"`
}

// Log configuration
type Log struct {
	Level string `yaml:"level"`
	File  string `yaml:"file"`
	JSON  bool   `yaml:"json"`
}

// CloudPods configuration
type CloudPods struct {
	Endpoint string `yaml:"endpoint"`
	Username string `yaml:"username"`
	Password string `yaml:"password"`
	Region   string `yaml:"region"`
}

// Aviatrix configuration
type Aviatrix struct {
	ControllerIP string `yaml:"controller_ip"`
	Username     string `yaml:"username"`
	Password     string `yaml:"password"`
	Region       string `yaml:"region"`
}

// Monitoring configuration
type Monitoring struct {
	Prometheus Prometheus `yaml:"prometheus"`
	ClickHouse ClickHouse `yaml:"clickhouse"`
}

// Prometheus configuration
type Prometheus struct {
	Port int `yaml:"port"`
}

// ClickHouse configuration
type ClickHouse struct {
	Host     string `yaml:"host"`
	Port     int    `yaml:"port"`
	Database string `yaml:"database"`
	Username string `yaml:"username"`
	Password string `yaml:"password"`
}

// Load loads configuration from file
func Load(filename string) (*Config, error) {
	// Default configuration
	config := &Config{
		API: API{
			Host:         "0.0.0.0",
			Port:         8080,
			ReadTimeout:  30 * time.Second,
			WriteTimeout: 30 * time.Second,
			IdleTimeout:  120 * time.Second,
			CORS: CORS{
				AllowOrigins: []string{"*"},
				AllowMethods: []string{"GET", "POST", "PUT", "DELETE", "OPTIONS"},
				AllowHeaders: []string{"Origin", "Content-Type", "Accept", "Authorization"},
			},
		},
		Log: Log{
			Level: "info",
			File:  "",
			JSON:  false,
		},
		CloudPods: CloudPods{
			Endpoint: "http://localhost:8081",
			Username: "admin",
			Password: "admin",
			Region:   "us-west-1",
		},
		Aviatrix: Aviatrix{
			ControllerIP: "localhost",
			Username:     "admin",
			Password:     "admin",
			Region:       "us-west-1",
		},
		Monitoring: Monitoring{
			Prometheus: Prometheus{
				Port: 9090,
			},
			ClickHouse: ClickHouse{
				Host:     "localhost",
				Port:     9000,
				Database: "router_sim",
				Username: "default",
				Password: "",
			},
		},
	}

	return config, nil
}
