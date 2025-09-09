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

// UpdateInstanceRequest represents a request to update an instance
type UpdateInstanceRequest struct {
	Name   string            `json:"name,omitempty"`
	Tags   map[string]string `json:"tags,omitempty"`
}

// SecurityGroup represents a CloudPods security group
type SecurityGroup struct {
	ID          string              `json:"id"`
	Name        string              `json:"name"`
	Description string              `json:"description"`
	Rules       []SecurityGroupRule `json:"rules"`
	CreatedAt   time.Time           `json:"created_at"`
	UpdatedAt   time.Time           `json:"updated_at"`
}

// SecurityGroupRule represents a security group rule
type SecurityGroupRule struct {
	ID              string `json:"id"`
	Direction       string `json:"direction"` // ingress or egress
	Protocol        string `json:"protocol"`  // tcp, udp, icmp, etc.
	PortRangeMin    int    `json:"port_range_min"`
	PortRangeMax    int    `json:"port_range_max"`
	SourceGroupID   string `json:"source_group_id,omitempty"`
	SourceCIDR      string `json:"source_cidr,omitempty"`
	DestinationCIDR string `json:"destination_cidr,omitempty"`
}

// LoadBalancer represents a CloudPods load balancer
type LoadBalancer struct {
	ID          string    `json:"id"`
	Name        string    `json:"name"`
	RegionID    string    `json:"region_id"`
	VPCID       string    `json:"vpc_id"`
	SubnetID    string    `json:"subnet_id"`
	Type        string    `json:"type"`
	Status      string    `json:"status"`
	PublicIP    string    `json:"public_ip"`
	PrivateIP   string    `json:"private_ip"`
	CreatedAt   time.Time `json:"created_at"`
	UpdatedAt   time.Time `json:"updated_at"`
}

// Database represents a CloudPods database
type Database struct {
	ID          string    `json:"id"`
	Name        string    `json:"name"`
	RegionID    string    `json:"region_id"`
	VPCID       string    `json:"vpc_id"`
	SubnetID    string    `json:"subnet_id"`
	Engine      string    `json:"engine"`
	Version     string    `json:"version"`
	Status      string    `json:"status"`
	Endpoint    string    `json:"endpoint"`
	Port        int       `json:"port"`
	CreatedAt   time.Time `json:"created_at"`
	UpdatedAt   time.Time `json:"updated_at"`
}

// Storage represents a CloudPods storage
type Storage struct {
	ID          string    `json:"id"`
	Name        string    `json:"name"`
	RegionID    string    `json:"region_id"`
	ZoneID      string    `json:"zone_id"`
	Type        string    `json:"type"`
	Size        int64     `json:"size"` // in GB
	Status      string    `json:"status"`
	CreatedAt   time.Time `json:"created_at"`
	UpdatedAt   time.Time `json:"updated_at"`
}

// APIError represents a CloudPods API error
type APIError struct {
	Code    string `json:"code"`
	Message string `json:"message"`
	Details string `json:"details,omitempty"`
}

func (e *APIError) Error() string {
	return e.Message
}

// Pagination represents pagination parameters
type Pagination struct {
	Page     int `json:"page"`
	PageSize int `json:"page_size"`
	Total    int `json:"total"`
}

// ListResponse represents a paginated list response
type ListResponse struct {
	Items      interface{} `json:"items"`
	Pagination Pagination  `json:"pagination"`
}
