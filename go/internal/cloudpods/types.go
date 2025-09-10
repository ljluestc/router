package cloudpods

import "time"

// Region represents a CloudPods region
type Region struct {
	ID          string    `json:"id"`
	Name        string    `json:"name"`
	Description string    `json:"description"`
	Status      string    `json:"status"`
	CreatedAt   time.Time `json:"created_at"`
	UpdatedAt   time.Time `json:"updated_at"`
}

// Zone represents a CloudPods availability zone
type Zone struct {
	ID          string    `json:"id"`
	Name        string    `json:"name"`
	RegionID    string    `json:"region_id"`
	Description string    `json:"description"`
	Status      string    `json:"status"`
	CreatedAt   time.Time `json:"created_at"`
	UpdatedAt   time.Time `json:"updated_at"`
}

// VPC represents a CloudPods VPC
type VPC struct {
	ID          string    `json:"id"`
	Name        string    `json:"name"`
	RegionID    string    `json:"region_id"`
	CIDR        string    `json:"cidr"`
	Description string    `json:"description"`
	Status      string    `json:"status"`
	CreatedAt   time.Time `json:"created_at"`
	UpdatedAt   time.Time `json:"updated_at"`
}

// Subnet represents a CloudPods subnet
type Subnet struct {
	ID          string    `json:"id"`
	Name        string    `json:"name"`
	VPCID       string    `json:"vpc_id"`
	CIDR        string    `json:"cidr"`
	ZoneID      string    `json:"zone_id"`
	Description string    `json:"description"`
	Status      string    `json:"status"`
	CreatedAt   time.Time `json:"created_at"`
	UpdatedAt   time.Time `json:"updated_at"`
}

// Instance represents a CloudPods instance
type Instance struct {
	ID          string            `json:"id"`
	Name        string            `json:"name"`
	RegionID    string            `json:"region_id"`
	ZoneID      string            `json:"zone_id"`
	VPCID       string            `json:"vpc_id"`
	SubnetID    string            `json:"subnet_id"`
	InstanceType string           `json:"instance_type"`
	ImageID     string            `json:"image_id"`
	Status      string            `json:"status"`
	PublicIP    string            `json:"public_ip"`
	PrivateIP   string            `json:"private_ip"`
	Tags        map[string]string `json:"tags"`
	CreatedAt   time.Time         `json:"created_at"`
	UpdatedAt   time.Time         `json:"updated_at"`
}

// CreateInstanceRequest represents a request to create an instance
type CreateInstanceRequest struct {
	Name         string            `json:"name"`
	RegionID     string            `json:"region_id"`
	ZoneID       string            `json:"zone_id"`
	VPCID        string            `json:"vpc_id"`
	SubnetID     string            `json:"subnet_id"`
	InstanceType string            `json:"instance_type"`
	ImageID      string            `json:"image_id"`
	Tags         map[string]string `json:"tags,omitempty"`
}

// Network represents a CloudPods network
type Network struct {
	ID          string    `json:"id"`
	Name        string    `json:"name"`
	RegionID    string    `json:"region_id"`
	Type        string    `json:"type"`
	Description string    `json:"description"`
	Status      string    `json:"status"`
	CreatedAt   time.Time `json:"created_at"`
	UpdatedAt   time.Time `json:"updated_at"`
}

// LoadBalancer represents a CloudPods load balancer
type LoadBalancer struct {
	ID          string    `json:"id"`
	Name        string    `json:"name"`
	RegionID    string    `json:"region_id"`
	VPCID       string    `json:"vpc_id"`
	Type        string    `json:"type"`
	Status      string    `json:"status"`
	PublicIP    string    `json:"public_ip"`
	PrivateIP   string    `json:"private_ip"`
	Port        int       `json:"port"`
	Protocol    string    `json:"protocol"`
	CreatedAt   time.Time `json:"created_at"`
	UpdatedAt   time.Time `json:"updated_at"`
}

// SecurityGroup represents a CloudPods security group
type SecurityGroup struct {
	ID          string              `json:"id"`
	Name        string              `json:"name"`
	RegionID    string              `json:"region_id"`
	VPCID       string              `json:"vpc_id"`
	Description string              `json:"description"`
	Rules       []SecurityGroupRule `json:"rules"`
	CreatedAt   time.Time           `json:"created_at"`
	UpdatedAt   time.Time           `json:"updated_at"`
}

// SecurityGroupRule represents a security group rule
type SecurityGroupRule struct {
	ID              string `json:"id"`
	Direction       string `json:"direction"`
	Protocol        string `json:"protocol"`
	PortRange       string `json:"port_range"`
	Source          string `json:"source"`
	Destination     string `json:"destination"`
	Action          string `json:"action"`
	Description     string `json:"description"`
}

// Metrics represents CloudPods metrics
type Metrics struct {
	ResourceType string                 `json:"resource_type"`
	ResourceID   string                 `json:"resource_id"`
	TimeRange    string                 `json:"time_range"`
	Data         map[string]interface{} `json:"data"`
	Timestamp    time.Time              `json:"timestamp"`
}

// CloudPodsConfig represents CloudPods configuration
type CloudPodsConfig struct {
	BaseURL  string `json:"base_url"`
	Username string `json:"username"`
	Password string `json:"password"`
	Region   string `json:"region"`
}

// CloudPodsService represents the CloudPods service interface
type CloudPodsService interface {
	GetRegions(ctx context.Context) ([]Region, error)
	GetZones(ctx context.Context, regionID string) ([]Zone, error)
	GetVPCs(ctx context.Context, regionID string) ([]VPC, error)
	GetSubnets(ctx context.Context, vpcID string) ([]Subnet, error)
	GetInstances(ctx context.Context, regionID string) ([]Instance, error)
	CreateInstance(ctx context.Context, req CreateInstanceRequest) (*Instance, error)
	DeleteInstance(ctx context.Context, instanceID string) error
	GetNetworks(ctx context.Context, regionID string) ([]Network, error)
	GetLoadBalancers(ctx context.Context, regionID string) ([]LoadBalancer, error)
	GetSecurityGroups(ctx context.Context, regionID string) ([]SecurityGroup, error)
	GetMetrics(ctx context.Context, resourceType, resourceID, timeRange string) (*Metrics, error)
}
