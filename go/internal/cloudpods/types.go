package cloudpods

import "time"

// VPC represents a Virtual Private Cloud
type VPC struct {
	ID          string    `json:"id"`
	Name        string    `json:"name"`
	CIDRBlock   string    `json:"cidr_block"`
	Region      string    `json:"region"`
	Project     string    `json:"project"`
	Status      string    `json:"status"`
	CreatedAt   time.Time `json:"created_at"`
	UpdatedAt   time.Time `json:"updated_at"`
	SubnetCount int       `json:"subnet_count"`
}

// CreateVPCRequest represents a request to create a VPC
type CreateVPCRequest struct {
	Name      string `json:"name"`
	CIDRBlock string `json:"cidr_block"`
	Region    string `json:"region"`
	Project   string `json:"project"`
}

// UpdateVPCRequest represents a request to update a VPC
type UpdateVPCRequest struct {
	Name      string `json:"name,omitempty"`
	CIDRBlock string `json:"cidr_block,omitempty"`
}

// VPCStats represents VPC statistics
type VPCStats struct {
	VPCID              string    `json:"vpc_id"`
	SubnetCount        int       `json:"subnet_count"`
	NATGatewayCount    int       `json:"nat_gateway_count"`
	LoadBalancerCount  int       `json:"load_balancer_count"`
	ServiceMeshRoutes  int       `json:"service_mesh_routes"`
	TotalTraffic       int64     `json:"total_traffic"`
	ActiveConnections  int       `json:"active_connections"`
	LastUpdated        time.Time `json:"last_updated"`
}

// Subnet represents a subnet within a VPC
type Subnet struct {
	ID               string    `json:"id"`
	VPCID            string    `json:"vpc_id"`
	Name             string    `json:"name"`
	CIDR             string    `json:"cidr"`
	AvailabilityZone string    `json:"availability_zone"`
	Status           string    `json:"status"`
	CreatedAt        time.Time `json:"created_at"`
	UpdatedAt        time.Time `json:"updated_at"`
}

// CreateSubnetRequest represents a request to create a subnet
type CreateSubnetRequest struct {
	Name             string `json:"name"`
	CIDR             string `json:"cidr"`
	AvailabilityZone string `json:"availability_zone"`
}

// UpdateSubnetRequest represents a request to update a subnet
type UpdateSubnetRequest struct {
	Name string `json:"name,omitempty"`
}

// NATGateway represents a NAT Gateway
type NATGateway struct {
	ID        string    `json:"id"`
	VPCID     string    `json:"vpc_id"`
	SubnetID  string    `json:"subnet_id"`
	Name      string    `json:"name"`
	ElasticIP string    `json:"elastic_ip"`
	Status    string    `json:"status"`
	CreatedAt time.Time `json:"created_at"`
	UpdatedAt time.Time `json:"updated_at"`
}

// CreateNATGatewayRequest represents a request to create a NAT Gateway
type CreateNATGatewayRequest struct {
	Name      string `json:"name"`
	SubnetID  string `json:"subnet_id"`
	ElasticIP string `json:"elastic_ip"`
}

// LoadBalancer represents a load balancer
type LoadBalancer struct {
	ID           string            `json:"id"`
	VPCID        string            `json:"vpc_id"`
	Name         string            `json:"name"`
	Type         string            `json:"type"` // "application" or "network"
	SubnetID     string            `json:"subnet_id"`
	TargetGroups []string          `json:"target_groups"`
	Listeners    []Listener        `json:"listeners"`
	Status       string            `json:"status"`
	CreatedAt    time.Time         `json:"created_at"`
	UpdatedAt    time.Time         `json:"updated_at"`
}

// Listener represents a load balancer listener
type Listener struct {
	Port            int    `json:"port"`
	Protocol        string `json:"protocol"`
	SSLCertificate  string `json:"ssl_certificate,omitempty"`
	DefaultAction   string `json:"default_action"`
}

// CreateLoadBalancerRequest represents a request to create a load balancer
type CreateLoadBalancerRequest struct {
	Name         string     `json:"name"`
	Type         string     `json:"type"`
	SubnetID     string     `json:"subnet_id"`
	TargetGroups []string   `json:"target_groups"`
	Listeners    []Listener `json:"listeners"`
}

// UpdateLoadBalancerRequest represents a request to update a load balancer
type UpdateLoadBalancerRequest struct {
	Name         string     `json:"name,omitempty"`
	TargetGroups []string   `json:"target_groups,omitempty"`
	Listeners    []Listener `json:"listeners,omitempty"`
}

// ServiceMeshRoute represents a service mesh route
type ServiceMeshRoute struct {
	ID          string                 `json:"id"`
	VPCID       string                 `json:"vpc_id"`
	Name        string                 `json:"name"`
	ServiceName string                 `json:"service_name"`
	ServiceIP   string                 `json:"service_ip"`
	Endpoints   []string               `json:"endpoints"`
	Policy      ServiceMeshPolicy      `json:"policy"`
	Status      string                 `json:"status"`
	CreatedAt   time.Time              `json:"created_at"`
	UpdatedAt   time.Time              `json:"updated_at"`
}

// ServiceMeshPolicy represents service mesh traffic policy
type ServiceMeshPolicy struct {
	LoadBalancing string                 `json:"load_balancing"`
	HealthCheck   *HealthCheckPolicy     `json:"health_check,omitempty"`
	CircuitBreaker *CircuitBreakerPolicy `json:"circuit_breaker,omitempty"`
}

// HealthCheckPolicy represents health check configuration
type HealthCheckPolicy struct {
	Enabled  bool          `json:"enabled"`
	Interval time.Duration `json:"interval"`
	Timeout  time.Duration `json:"timeout"`
	Path     string        `json:"path,omitempty"`
}

// CircuitBreakerPolicy represents circuit breaker configuration
type CircuitBreakerPolicy struct {
	Enabled         bool          `json:"enabled"`
	FailureThreshold int          `json:"failure_threshold"`
	Timeout         time.Duration `json:"timeout"`
}

// CreateServiceMeshRouteRequest represents a request to create a service mesh route
type CreateServiceMeshRouteRequest struct {
	Name        string            `json:"name"`
	ServiceName string            `json:"service_name"`
	ServiceIP   string            `json:"service_ip"`
	Endpoints   []string          `json:"endpoints"`
	Policy      ServiceMeshPolicy `json:"policy"`
}

// UpdateServiceMeshRouteRequest represents a request to update a service mesh route
type UpdateServiceMeshRouteRequest struct {
	Name        string             `json:"name,omitempty"`
	ServiceName string             `json:"service_name,omitempty"`
	ServiceIP   string             `json:"service_ip,omitempty"`
	Endpoints   []string           `json:"endpoints,omitempty"`
	Policy      *ServiceMeshPolicy `json:"policy,omitempty"`
}
