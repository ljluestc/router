package aviatrix

import "time"

// Gateway represents an Aviatrix gateway
type Gateway struct {
	GatewayName string            `json:"gw_name"`
	CloudType   int               `json:"cloud_type"`
	AccountName string            `json:"account_name"`
	Region      string            `json:"region"`
	VPCID       string            `json:"vpc_id"`
	Status      string            `json:"status"`
	PublicIP    string            `json:"public_ip"`
	PrivateIP   string            `json:"private_ip"`
	Tags        map[string]string `json:"tags"`
	CreatedAt   time.Time         `json:"created_at"`
	UpdatedAt   time.Time         `json:"updated_at"`
}

// TransitGateway represents an Aviatrix transit gateway
type TransitGateway struct {
	GatewayName string `json:"gw_name"`
	CloudType   int    `json:"cloud_type"`
	AccountName string `json:"account_name"`
	Region      string `json:"region"`
	VPCID       string `json:"vpc_id"`
	Status      string `json:"status"`
	PublicIP    string `json:"public_ip"`
	PrivateIP   string `json:"private_ip"`
	GatewaySize string `json:"gw_size"`
	HAEnabled   bool   `json:"ha_enabled"`
	HAGatewaySize string `json:"ha_gw_size"`
	EnableEncryption bool `json:"enable_encrypt"`
	EnableNAT   bool   `json:"enable_nat"`
	EnableVPCDNS bool  `json:"enable_vpc_dns"`
	BGPManualSpoke bool `json:"bgp_manual_spoke"`
	LocalASNumber string `json:"local_as_number"`
	BGPECMP     bool   `json:"bgp_ecmp"`
	EnableActiveMesh bool `json:"enable_active_mesh"`
	EnableLearnedCIDRsApproval bool `json:"enable_learned_cidrs_approval"`
	LearnedCIDRsApprovalMode string `json:"learned_cidrs_approval_mode"`
	EnableEncryptPeering bool `json:"enable_encrypt_peering"`
	EnablePeeringOverPrivateNetwork bool `json:"enable_peering_over_private_network"`
	EnablePeeringOverPublicNetwork bool `json:"enable_peering_over_public_network"`
	CreatedAt   time.Time `json:"created_at"`
	UpdatedAt   time.Time `json:"updated_at"`
}

// SpokeGateway represents an Aviatrix spoke gateway
type SpokeGateway struct {
	GatewayName string `json:"gw_name"`
	CloudType   int    `json:"cloud_type"`
	AccountName string `json:"account_name"`
	Region      string `json:"region"`
	VPCID       string `json:"vpc_id"`
	Status      string `json:"status"`
	PublicIP    string `json:"public_ip"`
	PrivateIP   string `json:"private_ip"`
	GatewaySize string `json:"gw_size"`
	HAEnabled   bool   `json:"ha_enabled"`
	HAGatewaySize string `json:"ha_gw_size"`
	EnableEncryption bool `json:"enable_encrypt"`
	EnableNAT   bool   `json:"enable_nat"`
	EnableVPCDNS bool  `json:"enable_vpc_dns"`
	BGPManualSpoke bool `json:"bgp_manual_spoke"`
	LocalASNumber string `json:"local_as_number"`
	BGPECMP     bool   `json:"bgp_ecmp"`
	EnableActiveMesh bool `json:"enable_active_mesh"`
	EnableLearnedCIDRsApproval bool `json:"enable_learned_cidrs_approval"`
	LearnedCIDRsApprovalMode string `json:"learned_cidrs_approval_mode"`
	EnableEncryptPeering bool `json:"enable_encrypt_peering"`
	EnablePeeringOverPrivateNetwork bool `json:"enable_peering_over_private_network"`
	EnablePeeringOverPublicNetwork bool `json:"enable_peering_over_public_network"`
	CreatedAt   time.Time `json:"created_at"`
	UpdatedAt   time.Time `json:"updated_at"`
}

// TransitGatewayPeering represents transit gateway peering
type TransitGatewayPeering struct {
	PeeringName string `json:"peering_name"`
	SourceGateway string `json:"source_gateway"`
	DestinationGateway string `json:"destination_gateway"`
	Status      string `json:"status"`
	CreatedAt   time.Time `json:"created_at"`
	UpdatedAt   time.Time `json:"updated_at"`
}

// SecurityDomain represents a security domain
type SecurityDomain struct {
	DomainName string   `json:"domain_name"`
	Description string  `json:"description"`
	Gateways   []string `json:"gateways"`
	CreatedAt  time.Time `json:"created_at"`
	UpdatedAt  time.Time `json:"updated_at"`
}

// FirewallPolicy represents a firewall policy
type FirewallPolicy struct {
	PolicyName string `json:"policy_name"`
	Source     string `json:"source"`
	Destination string `json:"destination"`
	Protocol   string `json:"protocol"`
	Port       string `json:"port"`
	Action     string `json:"action"`
	LogEnabled bool   `json:"log_enabled"`
	CreatedAt  time.Time `json:"created_at"`
	UpdatedAt  time.Time `json:"updated_at"`
}

// Metrics represents Aviatrix metrics
type Metrics struct {
	GatewayName string                 `json:"gw_name"`
	TimeRange   string                 `json:"time_range"`
	Data        map[string]interface{} `json:"data"`
	Timestamp   time.Time              `json:"timestamp"`
}

// CreateTransitGatewayRequest represents a request to create a transit gateway
type CreateTransitGatewayRequest struct {
	CloudType        int    `json:"cloud_type"`
	AccountName      string `json:"account_name"`
	Region           string `json:"region"`
	GatewayName      string `json:"gw_name"`
	VPCID            string `json:"vpc_id"`
	VPCRegion        string `json:"vpc_reg"`
	GatewaySize      string `json:"gw_size"`
	Subnet           string `json:"subnet"`
	HASubnet         string `json:"ha_subnet"`
	HAGatewaySize    string `json:"ha_gw_size"`
	EnableEncryption bool   `json:"enable_encrypt"`
	EnableNAT        bool   `json:"enable_nat"`
	EnableVPCDNS     bool   `json:"enable_vpc_dns"`
	EnableAdvertiseTransitCIDR bool `json:"enable_advertise_transit_cidr"`
	BGPManualSpoke   bool   `json:"bgp_manual_spoke"`
	PrependASPath    string `json:"prepend_as_path"`
	LocalASNumber    string `json:"local_as_number"`
	BGPECMP          bool   `json:"bgp_ecmp"`
	EnableActiveMesh bool   `json:"enable_active_mesh"`
	EnableLearnedCIDRsApproval bool `json:"enable_learned_cidrs_approval"`
	LearnedCIDRsApprovalMode string `json:"learned_cidrs_approval_mode"`
	EnableEncryptPeering bool `json:"enable_encrypt_peering"`
	EnablePeeringOverPrivateNetwork bool `json:"enable_peering_over_private_network"`
	EnablePeeringOverPublicNetwork bool `json:"enable_peering_over_public_network"`
	EnablePeeringOverPrivateNetworkForPeeringHA bool `json:"enable_peering_over_private_network_for_peering_ha"`
	EnablePeeringOverPublicNetworkForPeeringHA bool `json:"enable_peering_over_public_network_for_peering_ha"`
	EnablePeeringOverPrivateNetworkForPeeringHAGateway bool `json:"enable_peering_over_private_network_for_peering_ha_gateway"`
	EnablePeeringOverPublicNetworkForPeeringHAGateway bool `json:"enable_peering_over_public_network_for_peering_ha_gateway"`
	EnablePeeringOverPrivateNetworkForPeeringHAGateway2 bool `json:"enable_peering_over_private_network_for_peering_ha_gateway_2"`
	EnablePeeringOverPublicNetworkForPeeringHAGateway2 bool `json:"enable_peering_over_public_network_for_peering_ha_gateway_2"`
	EnablePeeringOverPrivateNetworkForPeeringHAGateway3 bool `json:"enable_peering_over_private_network_for_peering_ha_gateway_3"`
	EnablePeeringOverPublicNetworkForPeeringHAGateway3 bool `json:"enable_peering_over_public_network_for_peering_ha_gateway_3"`
	EnablePeeringOverPrivateNetworkForPeeringHAGateway4 bool `json:"enable_peering_over_private_network_for_peering_ha_gateway_4"`
	EnablePeeringOverPublicNetworkForPeeringHAGateway4 bool `json:"enable_peering_over_public_network_for_peering_ha_gateway_4"`
	EnablePeeringOverPrivateNetworkForPeeringHAGateway5 bool `json:"enable_peering_over_private_network_for_peering_ha_gateway_5"`
	EnablePeeringOverPublicNetworkForPeeringHAGateway5 bool `json:"enable_peering_over_public_network_for_peering_ha_gateway_5"`
	EnablePeeringOverPrivateNetworkForPeeringHAGateway6 bool `json:"enable_peering_over_private_network_for_peering_ha_gateway_6"`
	EnablePeeringOverPublicNetworkForPeeringHAGateway6 bool `json:"enable_peering_over_public_network_for_peering_ha_gateway_6"`
	EnablePeeringOverPrivateNetworkForPeeringHAGateway7 bool `json:"enable_peering_over_private_network_for_peering_ha_gateway_7"`
	EnablePeeringOverPublicNetworkForPeeringHAGateway7 bool `json:"enable_peering_over_public_network_for_peering_ha_gateway_7"`
	EnablePeeringOverPrivateNetworkForPeeringHAGateway8 bool `json:"enable_peering_over_private_network_for_peering_ha_gateway_8"`
	EnablePeeringOverPublicNetworkForPeeringHAGateway8 bool `json:"enable_peering_over_public_network_for_peering_ha_gateway_8"`
	EnablePeeringOverPrivateNetworkForPeeringHAGateway9 bool `json:"enable_peering_over_private_network_for_peering_ha_gateway_9"`
	EnablePeeringOverPublicNetworkForPeeringHAGateway9 bool `json:"enable_peering_over_public_network_for_peering_ha_gateway_9"`
	EnablePeeringOverPrivateNetworkForPeeringHAGateway10 bool `json:"enable_peering_over_private_network_for_peering_ha_gateway_10"`
	EnablePeeringOverPublicNetworkForPeeringHAGateway10 bool `json:"enable_peering_over_public_network_for_peering_ha_gateway_10"`
}

// AviatrixConfig represents Aviatrix configuration
type AviatrixConfig struct {
	ControllerIP string `json:"controller_ip"`
	Username     string `json:"username"`
	Password     string `json:"password"`
	Region       string `json:"region"`
}

// AviatrixService represents the Aviatrix service interface
type AviatrixService interface {
	GetGateways(ctx context.Context) ([]Gateway, error)
	GetTransitGateways(ctx context.Context) ([]TransitGateway, error)
	GetSpokeGateways(ctx context.Context) ([]SpokeGateway, error)
	GetTransitGatewayPeering(ctx context.Context) ([]TransitGatewayPeering, error)
	GetSecurityDomains(ctx context.Context) ([]SecurityDomain, error)
	GetFirewallPolicies(ctx context.Context) ([]FirewallPolicy, error)
	GetMetrics(ctx context.Context, gatewayName, timeRange string) (*Metrics, error)
}
