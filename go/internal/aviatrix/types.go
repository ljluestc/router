package aviatrix

import "time"

// Gateway represents a generic Aviatrix gateway
type Gateway struct {
	GatewayName string            `json:"gw_name"`
	CloudType   int               `json:"cloud_type"`
	VPCID       string            `json:"vpc_id"`
	VPCRegion   string            `json:"vpc_reg"`
	Status      string            `json:"status"`
	PublicIP    string            `json:"public_ip"`
	PrivateIP   string            `json:"private_ip"`
	CreatedAt   time.Time         `json:"created_at"`
	UpdatedAt   time.Time         `json:"updated_at"`
	Tags        map[string]string `json:"tags"`
}

// TransitGateway represents an Aviatrix transit gateway
type TransitGateway struct {
	GatewayName     string            `json:"gw_name"`
	CloudType       int               `json:"cloud_type"`
	AccountName     string            `json:"account_name"`
	VPCID           string            `json:"vpc_id"`
	VPCRegion       string            `json:"vpc_reg"`
	GatewaySize     string            `json:"gw_size"`
	Subnet          string            `json:"subnet"`
	Status          string            `json:"status"`
	PublicIP        string            `json:"public_ip"`
	PrivateIP       string            `json:"private_ip"`
	EnableEncrypt   bool              `json:"enable_encrypt"`
	EnableNAT       bool              `json:"enable_nat"`
	EnableHybrid    bool              `json:"enable_hybrid"`
	EnableFireNet   bool              `json:"enable_firenet"`
	EnableVPCDNS    bool              `json:"enable_vpc_dns"`
	EnableAdvertise bool              `json:"enable_advertise"`
	CreatedAt       time.Time         `json:"created_at"`
	UpdatedAt       time.Time         `json:"updated_at"`
	Tags            map[string]string `json:"tags"`
}

// SpokeGateway represents an Aviatrix spoke gateway
type SpokeGateway struct {
	GatewayName     string            `json:"gw_name"`
	CloudType       int               `json:"cloud_type"`
	AccountName     string            `json:"account_name"`
	VPCID           string            `json:"vpc_id"`
	VPCRegion       string            `json:"vpc_reg"`
	GatewaySize     string            `json:"gw_size"`
	Subnet          string            `json:"subnet"`
	Status          string            `json:"status"`
	PublicIP        string            `json:"public_ip"`
	PrivateIP       string            `json:"private_ip"`
	EnableEncrypt   bool              `json:"enable_encrypt"`
	EnableNAT       bool              `json:"enable_nat"`
	EnableHybrid    bool              `json:"enable_hybrid"`
	EnableFireNet   bool              `json:"enable_firenet"`
	EnableVPCDNS    bool              `json:"enable_vpc_dns"`
	EnableAdvertise bool              `json:"enable_advertise"`
	CreatedAt       time.Time         `json:"created_at"`
	UpdatedAt       time.Time         `json:"updated_at"`
	Tags            map[string]string `json:"tags"`
}

// CreateTransitGatewayRequest represents a request to create a transit gateway
type CreateTransitGatewayRequest struct {
	CloudType       int    `json:"cloud_type"`
	AccountName     string `json:"account_name"`
	GatewayName     string `json:"gw_name"`
	VPCID           string `json:"vpc_id"`
	VPCRegion       string `json:"vpc_reg"`
	GatewaySize     string `json:"gw_size"`
	Subnet          string `json:"subnet"`
	EnableEncryption bool  `json:"enable_encrypt"`
	EnableNAT       bool   `json:"enable_nat"`
	EnableHybrid    bool   `json:"enable_hybrid"`
	EnableFireNet   bool   `json:"enable_firenet"`
	EnableVPCDNS    bool   `json:"enable_vpc_dns"`
	EnableAdvertise bool   `json:"enable_advertise"`
}

// CreateSpokeGatewayRequest represents a request to create a spoke gateway
type CreateSpokeGatewayRequest struct {
	CloudType       int    `json:"cloud_type"`
	AccountName     string `json:"account_name"`
	GatewayName     string `json:"gw_name"`
	VPCID           string `json:"vpc_id"`
	VPCRegion       string `json:"vpc_reg"`
	GatewaySize     string `json:"gw_size"`
	Subnet          string `json:"subnet"`
	EnableEncryption bool  `json:"enable_encrypt"`
	EnableNAT       bool   `json:"enable_nat"`
	EnableHybrid    bool   `json:"enable_hybrid"`
	EnableFireNet   bool   `json:"enable_firenet"`
	EnableVPCDNS    bool   `json:"enable_vpc_dns"`
	EnableAdvertise bool   `json:"enable_advertise"`
}

// GatewayStatus represents the status of a gateway
type GatewayStatus struct {
	GatewayName string `json:"gw_name"`
	Status      string `json:"status"`
	State       string `json:"state"`
	Uptime      string `json:"uptime"`
	Version     string `json:"version"`
	LastUpdated time.Time `json:"last_updated"`
}

// GatewayMetrics represents metrics for a gateway
type GatewayMetrics struct {
	GatewayName     string  `json:"gw_name"`
	CPUUsage        float64 `json:"cpu_usage"`
	MemoryUsage     float64 `json:"memory_usage"`
	DiskUsage       float64 `json:"disk_usage"`
	NetworkIn       int64   `json:"network_in"`
	NetworkOut      int64   `json:"network_out"`
	PacketIn        int64   `json:"packet_in"`
	PacketOut       int64   `json:"packet_out"`
	PacketDrop      int64   `json:"packet_drop"`
	Latency         float64 `json:"latency"`
	Throughput      float64 `json:"throughput"`
	LastUpdated     time.Time `json:"last_updated"`
}

// TransitGatewayPeering represents a transit gateway peering
type TransitGatewayPeering struct {
	ID              string    `json:"id"`
	SourceGateway   string    `json:"source_gateway"`
	DestinationGateway string `json:"destination_gateway"`
	Status          string    `json:"status"`
	CreatedAt       time.Time `json:"created_at"`
	UpdatedAt       time.Time `json:"updated_at"`
}

// SpokeGatewayAttachment represents a spoke gateway attachment
type SpokeGatewayAttachment struct {
	ID              string    `json:"id"`
	SpokeGateway    string    `json:"spoke_gateway"`
	TransitGateway  string    `json:"transit_gateway"`
	Status          string    `json:"status"`
	CreatedAt       time.Time `json:"created_at"`
	UpdatedAt       time.Time `json:"updated_at"`
}

// FirewallPolicy represents a firewall policy
type FirewallPolicy struct {
	ID          string    `json:"id"`
	Name        string    `json:"name"`
	Description string    `json:"description"`
	Rules       []FirewallRule `json:"rules"`
	CreatedAt   time.Time `json:"created_at"`
	UpdatedAt   time.Time `json:"updated_at"`
}

// FirewallRule represents a firewall rule
type FirewallRule struct {
	ID              string `json:"id"`
	Source          string `json:"source"`
	Destination     string `json:"destination"`
	Protocol        string `json:"protocol"`
	Port            string `json:"port"`
	Action          string `json:"action"` // allow, deny
	Description     string `json:"description"`
}

// VPNUser represents a VPN user
type VPNUser struct {
	ID          string    `json:"id"`
	Username    string    `json:"username"`
	Email       string    `json:"email"`
	Status      string    `json:"status"`
	CreatedAt   time.Time `json:"created_at"`
	UpdatedAt   time.Time `json:"updated_at"`
}

// VPNGateway represents a VPN gateway
type VPNGateway struct {
	GatewayName string    `json:"gw_name"`
	CloudType   int       `json:"cloud_type"`
	VPCID       string    `json:"vpc_id"`
	VPCRegion   string    `json:"vpc_reg"`
	Status      string    `json:"status"`
	PublicIP    string    `json:"public_ip"`
	PrivateIP   string    `json:"private_ip"`
	CreatedAt   time.Time `json:"created_at"`
	UpdatedAt   time.Time `json:"updated_at"`
}

// APIError represents an Aviatrix API error
type APIError struct {
	Code    string `json:"code"`
	Message string `json:"message"`
	Details string `json:"details,omitempty"`
}

func (e *APIError) Error() string {
	return e.Message
}

// Cloud types
const (
	CloudTypeAWS = 1
	CloudTypeAzure = 2
	CloudTypeGCP = 4
	CloudTypeOCI = 16
	CloudTypeAliCloud = 8
)

// Gateway sizes
const (
	GatewaySizeT2Micro = "t2.micro"
	GatewaySizeT2Small = "t2.small"
	GatewaySizeT2Medium = "t2.medium"
	GatewaySizeT2Large = "t2.large"
	GatewaySizeT3Micro = "t3.micro"
	GatewaySizeT3Small = "t3.small"
	GatewaySizeT3Medium = "t3.medium"
	GatewaySizeT3Large = "t3.large"
	GatewaySizeT3XLarge = "t3.xlarge"
	GatewaySizeC5Large = "c5.large"
	GatewaySizeC5XLarge = "c5.xlarge"
	GatewaySizeC5nLarge = "c5n.large"
	GatewaySizeC5nXLarge = "c5n.xlarge"
)

// Gateway statuses
const (
	StatusUp = "up"
	StatusDown = "down"
	StatusCreating = "creating"
	StatusDeleting = "deleting"
	StatusUpgrading = "upgrading"
	StatusMaintenance = "maintenance"
)
