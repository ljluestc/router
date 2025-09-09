package cloudpods

import "time"

// Project represents a CloudPods project
type Project struct {
	ID          string    `json:"id"`
	Name        string    `json:"name"`
	Description string    `json:"description"`
	Region      string    `json:"region"`
	Status      string    `json:"status"`
	CreatedAt   time.Time `json:"created_at"`
	UpdatedAt   time.Time `json:"updated_at"`
}

// VPC represents a Virtual Private Cloud
type VPC struct {
	ID          string    `json:"id"`
	Name        string    `json:"name"`
	Description string    `json:"description"`
	CIDR        string    `json:"cidr"`
	Region      string    `json:"region"`
	Status      string    `json:"status"`
	CreatedAt   time.Time `json:"created_at"`
	UpdatedAt   time.Time `json:"updated_at"`
}

// Subnet represents a subnet within a VPC
type Subnet struct {
	ID          string    `json:"id"`
	Name        string    `json:"name"`
	Description string    `json:"description"`
	CIDR        string    `json:"cidr"`
	VPCID       string    `json:"vpc_id"`
	AZ          string    `json:"availability_zone"`
	Status      string    `json:"status"`
	CreatedAt   time.Time `json:"created_at"`
	UpdatedAt   time.Time `json:"updated_at"`
}

// Instance represents a virtual machine instance
type Instance struct {
	ID          string            `json:"id"`
	Name        string            `json:"name"`
	Type        string            `json:"type"`
	Status      string            `json:"status"`
	PublicIP    string            `json:"public_ip"`
	PrivateIP   string            `json:"private_ip"`
	VPCID       string            `json:"vpc_id"`
	SubnetID    string            `json:"subnet_id"`
	Tags        map[string]string `json:"tags"`
	CreatedAt   time.Time         `json:"created_at"`
	UpdatedAt   time.Time         `json:"updated_at"`
}

// LoadBalancer represents a load balancer
type LoadBalancer struct {
	ID          string    `json:"id"`
	Name        string    `json:"name"`
	Type        string    `json:"type"`
	Status      string    `json:"status"`
	PublicIP    string    `json:"public_ip"`
	VPCID       string    `json:"vpc_id"`
	SubnetID    string    `json:"subnet_id"`
	Listeners   []Listener `json:"listeners"`
	Backends    []Backend  `json:"backends"`
	CreatedAt   time.Time `json:"created_at"`
	UpdatedAt   time.Time `json:"updated_at"`
}

// Listener represents a load balancer listener
type Listener struct {
	ID           string `json:"id"`
	Protocol     string `json:"protocol"`
	Port         int    `json:"port"`
	BackendPort  int    `json:"backend_port"`
	HealthCheck  string `json:"health_check"`
}

// Backend represents a load balancer backend
type Backend struct {
	ID        string `json:"id"`
	InstanceID string `json:"instance_id"`
	IP        string `json:"ip"`
	Port      int    `json:"port"`
	Weight    int    `json:"weight"`
	Status    string `json:"status"`
}

// NATGateway represents a NAT gateway
type NATGateway struct {
	ID          string    `json:"id"`
	Name        string    `json:"name"`
	Status      string    `json:"status"`
	PublicIP    string    `json:"public_ip"`
	VPCID       string    `json:"vpc_id"`
	SubnetID    string    `json:"subnet_id"`
	CreatedAt   time.Time `json:"created_at"`
	UpdatedAt   time.Time `json:"updated_at"`
}

// NetworkTopology represents the network topology of a project
type NetworkTopology struct {
	ProjectID     string            `json:"project_id"`
	VPCs          []VPC             `json:"vpcs"`
	Subnets       []Subnet          `json:"subnets"`
	Instances     []Instance        `json:"instances"`
	LoadBalancers []LoadBalancer    `json:"loadbalancers"`
	NATGateways   []NATGateway      `json:"natgateways"`
	Connections   []Connection      `json:"connections"`
	Routes        []Route           `json:"routes"`
	SecurityGroups []SecurityGroup  `json:"security_groups"`
}

// Connection represents a network connection between resources
type Connection struct {
	ID        string `json:"id"`
	From      string `json:"from"`
	To        string `json:"to"`
	Type      string `json:"type"`
	Protocol  string `json:"protocol"`
	Port      int    `json:"port"`
	Status    string `json:"status"`
}

// Route represents a routing table entry
type Route struct {
	ID          string `json:"id"`
	Destination string `json:"destination"`
	Gateway     string `json:"gateway"`
	Interface   string `json:"interface"`
	Priority    int    `json:"priority"`
	Status      string `json:"status"`
}

// SecurityGroup represents a security group
type SecurityGroup struct {
	ID          string            `json:"id"`
	Name        string            `json:"name"`
	Description string            `json:"description"`
	Rules       []SecurityRule    `json:"rules"`
	VPCID       string            `json:"vpc_id"`
	CreatedAt   time.Time         `json:"created_at"`
	UpdatedAt   time.Time         `json:"updated_at"`
}

// SecurityRule represents a security group rule
type SecurityRule struct {
	ID          string `json:"id"`
	Direction   string `json:"direction"` // "ingress" or "egress"
	Protocol    string `json:"protocol"`
	Port        int    `json:"port"`
	Source      string `json:"source"`
	Destination string `json:"destination"`
	Action      string `json:"action"` // "allow" or "deny"
}

// CreateVPCRequest represents a request to create a VPC
type CreateVPCRequest struct {
	Name        string `json:"name"`
	Description string `json:"description"`
	CIDR        string `json:"cidr"`
	Region      string `json:"region"`
}

// CreateSubnetRequest represents a request to create a subnet
type CreateSubnetRequest struct {
	Name        string `json:"name"`
	Description string `json:"description"`
	CIDR        string `json:"cidr"`
	VPCID       string `json:"vpc_id"`
	AZ          string `json:"availability_zone"`
}

// CreateInstanceRequest represents a request to create an instance
type CreateInstanceRequest struct {
	Name     string            `json:"name"`
	Type     string            `json:"type"`
	VPCID    string            `json:"vpc_id"`
	SubnetID string            `json:"subnet_id"`
	Tags     map[string]string `json:"tags"`
}

// CreateLoadBalancerRequest represents a request to create a load balancer
type CreateLoadBalancerRequest struct {
	Name     string     `json:"name"`
	Type     string     `json:"type"`
	VPCID    string     `json:"vpc_id"`
	SubnetID string     `json:"subnet_id"`
	Listeners []Listener `json:"listeners"`
}

// CreateNATGatewayRequest represents a request to create a NAT gateway
type CreateNATGatewayRequest struct {
	Name     string `json:"name"`
	VPCID    string `json:"vpc_id"`
	SubnetID string `json:"subnet_id"`
}

// UpdateVPCRequest represents a request to update a VPC
type UpdateVPCRequest struct {
	Name        string `json:"name,omitempty"`
	Description string `json:"description,omitempty"`
}

// UpdateSubnetRequest represents a request to update a subnet
type UpdateSubnetRequest struct {
	Name        string `json:"name,omitempty"`
	Description string `json:"description,omitempty"`
}

// UpdateInstanceRequest represents a request to update an instance
type UpdateInstanceRequest struct {
	Name string            `json:"name,omitempty"`
	Tags map[string]string `json:"tags,omitempty"`
}

// ErrorResponse represents an error response from the API
type ErrorResponse struct {
	Error   string `json:"error"`
	Message string `json:"message"`
	Code    int    `json:"code"`
}

// PaginationRequest represents a pagination request
type PaginationRequest struct {
	Page     int `json:"page"`
	PageSize int `json:"page_size"`
}

// PaginationResponse represents a paginated response
type PaginationResponse struct {
	Page       int `json:"page"`
	PageSize   int `json:"page_size"`
	Total      int `json:"total"`
	TotalPages int `json:"total_pages"`
}

// ProjectsResponse represents a paginated projects response
type ProjectsResponse struct {
	PaginationResponse
	Projects []Project `json:"projects"`
}

// VPCsResponse represents a paginated VPCs response
type VPCsResponse struct {
	PaginationResponse
	VPCs []VPC `json:"vpcs"`
}

// SubnetsResponse represents a paginated subnets response
type SubnetsResponse struct {
	PaginationResponse
	Subnets []Subnet `json:"subnets"`
}

// InstancesResponse represents a paginated instances response
type InstancesResponse struct {
	PaginationResponse
	Instances []Instance `json:"instances"`
}

// LoadBalancersResponse represents a paginated load balancers response
type LoadBalancersResponse struct {
	PaginationResponse
	LoadBalancers []LoadBalancer `json:"loadbalancers"`
}

// NATGatewaysResponse represents a paginated NAT gateways response
type NATGatewaysResponse struct {
	PaginationResponse
	NATGateways []NATGateway `json:"natgateways"`
}

// SecurityGroupsResponse represents a paginated security groups response
type SecurityGroupsResponse struct {
	PaginationResponse
	SecurityGroups []SecurityGroup `json:"security_groups"`
}
