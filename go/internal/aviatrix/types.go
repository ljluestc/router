package aviatrix

import "time"

// TransitGateway represents an Aviatrix Transit Gateway
type TransitGateway struct {
	GWName                     string    `json:"gw_name"`
	CloudType                  int       `json:"cloud_type"`
	AccountName                string    `json:"account_name"`
	VPCID                      string    `json:"vpc_id"`
	Region                     string    `json:"vpc_reg"`
	Subnet                     string    `json:"subnet"`
	GWSize                     string    `json:"gw_size"`
	EnableEncryptPeering       bool      `json:"enable_encrypt_peering"`
	EnableLearnedCIDRsApproval bool      `json:"enable_learned_cidrs_approval"`
	Status                     string    `json:"status"`
	CreatedAt                  time.Time `json:"created_at"`
}

// CreateTransitGatewayRequest represents a request to create a transit gateway
type CreateTransitGatewayRequest struct {
	CloudType                  int    `json:"cloud_type"`
	AccountName                string `json:"account_name"`
	GWName                     string `json:"gw_name"`
	VPCID                      string `json:"vpc_id"`
	Region                     string `json:"vpc_reg"`
	Subnet                     string `json:"subnet"`
	GWSize                     string `json:"gw_size"`
	EnableEncryptPeering       bool   `json:"enable_encrypt_peering"`
	EnableLearnedCIDRsApproval bool   `json:"enable_learned_cidrs_approval"`
}

// UpdateTransitGatewayRequest represents a request to update a transit gateway
type UpdateTransitGatewayRequest struct {
	CloudType                  int    `json:"cloud_type"`
	AccountName                string `json:"account_name"`
	GWName                     string `json:"gw_name"`
	VPCID                      string `json:"vpc_id"`
	Region                     string `json:"vpc_reg"`
	Subnet                     string `json:"subnet"`
	GWSize                     string `json:"gw_size"`
	EnableEncryptPeering       bool   `json:"enable_encrypt_peering"`
	EnableLearnedCIDRsApproval bool   `json:"enable_learned_cidrs_approval"`
}

// SpokeGateway represents an Aviatrix Spoke Gateway
type SpokeGateway struct {
	GWName        string    `json:"gw_name"`
	CloudType     int       `json:"cloud_type"`
	AccountName   string    `json:"account_name"`
	VPCID         string    `json:"vpc_id"`
	Region        string    `json:"vpc_reg"`
	Subnet        string    `json:"subnet"`
	GWSize        string    `json:"gw_size"`
	EnableEncrypt bool      `json:"enable_encrypt"`
	Status        string    `json:"status"`
	CreatedAt     time.Time `json:"created_at"`
}

// CreateSpokeGatewayRequest represents a request to create a spoke gateway
type CreateSpokeGatewayRequest struct {
	CloudType     int    `json:"cloud_type"`
	AccountName   string `json:"account_name"`
	GWName        string `json:"gw_name"`
	VPCID         string `json:"vpc_id"`
	Region        string `json:"vpc_reg"`
	Subnet        string `json:"subnet"`
	GWSize        string `json:"gw_size"`
	EnableEncrypt bool   `json:"enable_encrypt"`
}

// UpdateSpokeGatewayRequest represents a request to update a spoke gateway
type UpdateSpokeGatewayRequest struct {
	CloudType     int    `json:"cloud_type"`
	AccountName   string `json:"account_name"`
	GWName        string `json:"gw_name"`
	VPCID         string `json:"vpc_id"`
	Region        string `json:"vpc_reg"`
	Subnet        string `json:"subnet"`
	GWSize        string `json:"gw_size"`
	EnableEncrypt bool   `json:"enable_encrypt"`
}

// VPCConnection represents a VPC connection
type VPCConnection struct {
	ConnectionName               string    `json:"connection_name"`
	VPCID                        string    `json:"vpc_id"`
	AccountName                  string    `json:"account_name"`
	Region                       string    `json:"region"`
	TransitGateway               string    `json:"transit_gateway"`
	SpokeGateway                 string    `json:"spoke_gateway"`
	ConnectionType               string    `json:"connection_type"`
	EnableLearnedCIDRsApproval   bool      `json:"enable_learned_cidrs_approval"`
	ApprovedCIDRs                []string  `json:"approved_cidrs"`
	Status                       string    `json:"status"`
	CreatedAt                    time.Time `json:"created_at"`
}

// CreateVPCConnectionRequest represents a request to create a VPC connection
type CreateVPCConnectionRequest struct {
	ConnectionName               string   `json:"connection_name"`
	VPCID                        string   `json:"vpc_id"`
	AccountName                  string   `json:"account_name"`
	Region                       string   `json:"region"`
	TransitGateway               string   `json:"transit_gateway"`
	SpokeGateway                 string   `json:"spoke_gateway"`
	ConnectionType               string   `json:"connection_type"`
	EnableLearnedCIDRsApproval   bool     `json:"enable_learned_cidrs_approval"`
	ApprovedCIDRs                []string `json:"approved_cidrs,omitempty"`
}

// Site2CloudConnection represents a site-to-cloud connection
type Site2CloudConnection struct {
	ConnectionName   string    `json:"connection_name"`
	VPCID            string    `json:"vpc_id"`
	RemoteGatewayIP  string    `json:"remote_gateway_ip"`
	PreSharedKey     string    `json:"pre_shared_key"`
	ConnectionType   string    `json:"connection_type"`
	RemoteSubnet     string    `json:"remote_subnet"`
	LocalSubnet      string    `json:"local_subnet"`
	Status           string    `json:"status"`
	CreatedAt        time.Time `json:"created_at"`
}

// CreateSite2CloudConnectionRequest represents a request to create a site-to-cloud connection
type CreateSite2CloudConnectionRequest struct {
	ConnectionName  string `json:"connection_name"`
	VPCID           string `json:"vpc_id"`
	RemoteGatewayIP string `json:"remote_gateway_ip"`
	PreSharedKey    string `json:"pre_shared_key"`
	ConnectionType  string `json:"connection_type"`
	RemoteSubnet    string `json:"remote_subnet"`
	LocalSubnet     string `json:"local_subnet"`
}
