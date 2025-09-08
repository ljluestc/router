package handlers

import (
	"encoding/json"
	"net/http"
	"strconv"
	"time"

	"router-sim/internal/analytics"
	"router-sim/internal/aviatrix"
	"router-sim/internal/cloudpods"
)

// Handlers contains all HTTP handlers
type Handlers struct {
	analytics  *analytics.Engine
	cloudpods  *cloudpods.Client
	aviatrix   *aviatrix.Client
}

// New creates a new Handlers instance
func New(analytics *analytics.Engine, cloudpods *cloudpods.Client, aviatrix *aviatrix.Client) *Handlers {
	return &Handlers{
		analytics: analytics,
		cloudpods: cloudpods,
		aviatrix:  aviatrix,
	}
}

// HealthCheck handles health check requests
func (h *Handlers) HealthCheck(w http.ResponseWriter, r *http.Request) {
	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(http.StatusOK)
	json.NewEncoder(w).Encode(map[string]interface{}{
		"status":    "healthy",
		"timestamp": time.Now().UTC(),
		"version":   "1.0.0",
	})
}

// GetStatus handles status requests
func (h *Handlers) GetStatus(w http.ResponseWriter, r *http.Request) {
	w.Header().Set("Content-Type", "application/json")
	
	status := map[string]interface{}{
		"status":    "running",
		"timestamp": time.Now().UTC(),
		"uptime":    "2d 14h 32m",
		"protocols": map[string]bool{
			"bgp":  true,
			"ospf": true,
			"isis": false,
		},
		"interfaces": 3,
		"routes":     1247,
		"neighbors":  5,
	}

	w.WriteHeader(http.StatusOK)
	json.NewEncoder(w).Encode(status)
}

// GetMetrics handles metrics requests
func (h *Handlers) GetMetrics(w http.ResponseWriter, r *http.Request) {
	w.Header().Set("Content-Type", "application/json")
	
	metrics := map[string]interface{}{
		"packets_per_second": 15420,
		"throughput":         125.6,
		"latency":            12.3,
		"packet_loss":        0.02,
		"cpu_usage":          45,
		"memory_usage":       67,
		"timestamp":          time.Now().UTC(),
	}

	w.WriteHeader(http.StatusOK)
	json.NewEncoder(w).Encode(metrics)
}

// GetRoutes handles route requests
func (h *Handlers) GetRoutes(w http.ResponseWriter, r *http.Request) {
	w.Header().Set("Content-Type", "application/json")
	
	routes := []map[string]interface{}{
		{
			"network":     "10.0.0.0/8",
			"next_hop":    "192.168.1.2",
			"protocol":    "BGP",
			"metric":      0,
			"as_path":     "65002 65003",
			"communities": "65001:100",
			"local_pref":  100,
			"origin":      "IGP",
		},
		{
			"network":     "192.168.0.0/16",
			"next_hop":    "10.0.0.2",
			"protocol":    "OSPF",
			"metric":      10,
			"as_path":     "",
			"communities": "",
			"local_pref":  0,
			"origin":      "IGP",
		},
	}

	w.WriteHeader(http.StatusOK)
	json.NewEncoder(w).Encode(routes)
}

// GetInterfaces handles interface requests
func (h *Handlers) GetInterfaces(w http.ResponseWriter, r *http.Request) {
	w.Header().Set("Content-Type", "application/json")
	
	interfaces := []map[string]interface{}{
		{
			"name":        "eth0",
			"type":        "ethernet",
			"ip":          "192.168.1.1",
			"subnet":      "192.168.1.0/24",
			"gateway":     "192.168.1.1",
			"mtu":         1500,
			"enabled":     true,
			"description": "Primary interface",
			"status":      "UP",
		},
		{
			"name":        "eth1",
			"type":        "ethernet",
			"ip":          "10.0.0.1",
			"subnet":      "10.0.0.0/24",
			"gateway":     "10.0.0.1",
			"mtu":         1500,
			"enabled":     true,
			"description": "Secondary interface",
			"status":      "UP",
		},
	}

	w.WriteHeader(http.StatusOK)
	json.NewEncoder(w).Encode(interfaces)
}

// GetNeighbors handles neighbor requests
func (h *Handlers) GetNeighbors(w http.ResponseWriter, r *http.Request) {
	w.Header().Set("Content-Type", "application/json")
	
	neighbors := []map[string]interface{}{
		{
			"ip":          "192.168.1.2",
			"asn":         65002,
			"protocol":    "BGP",
			"state":       "Established",
			"uptime":      "2d 14h 32m",
			"description": "BGP neighbor 1",
		},
		{
			"ip":          "10.0.0.2",
			"asn":         65003,
			"protocol":    "BGP",
			"state":       "Established",
			"uptime":      "2d 14h 32m",
			"description": "BGP neighbor 2",
		},
	}

	w.WriteHeader(http.StatusOK)
	json.NewEncoder(w).Encode(neighbors)
}

// GetAnalytics handles analytics requests
func (h *Handlers) GetAnalytics(w http.ResponseWriter, r *http.Request) {
	w.Header().Set("Content-Type", "application/json")
	
	stats := h.analytics.GetStats()
	
	w.WriteHeader(http.StatusOK)
	json.NewEncoder(w).Encode(stats)
}

// GetCloudPodsResources handles CloudPods resource requests
func (h *Handlers) GetCloudPodsResources(w http.ResponseWriter, r *http.Request) {
	w.Header().Set("Content-Type", "application/json")
	
	// Get resource type from query parameter
	resourceType := r.URL.Query().Get("type")
	if resourceType == "" {
		resourceType = "vms"
	}

	var resources interface{}
	var err error

	switch resourceType {
	case "vms":
		resources, err = h.cloudpods.GetVirtualMachines(r.Context())
	case "networks":
		resources, err = h.cloudpods.GetNetworks(r.Context())
	case "loadbalancers":
		resources, err = h.cloudpods.GetLoadBalancers(r.Context())
	case "securitygroups":
		resources, err = h.cloudpods.GetSecurityGroups(r.Context())
	default:
		http.Error(w, "Invalid resource type", http.StatusBadRequest)
		return
	}

	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	w.WriteHeader(http.StatusOK)
	json.NewEncoder(w).Encode(resources)
}

// GetAviatrixGateways handles Aviatrix gateway requests
func (h *Handlers) GetAviatrixGateways(w http.ResponseWriter, r *http.Request) {
	w.Header().Set("Content-Type", "application/json")
	
	// Get gateway type from query parameter
	gatewayType := r.URL.Query().Get("type")
	if gatewayType == "" {
		gatewayType = "all"
	}

	var gateways interface{}
	var err error

	switch gatewayType {
	case "all":
		gateways, err = h.aviatrix.GetGateways(r.Context())
	case "transit":
		gateways, err = h.aviatrix.GetTransitGateways(r.Context())
	case "spoke":
		gateways, err = h.aviatrix.GetSpokeGateways(r.Context())
	case "vpn":
		gateways, err = h.aviatrix.GetVPNGateways(r.Context())
	default:
		http.Error(w, "Invalid gateway type", http.StatusBadRequest)
		return
	}

	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	w.WriteHeader(http.StatusOK)
	json.NewEncoder(w).Encode(gateways)
}

// GetAviatrixBGPNeighbors handles Aviatrix BGP neighbor requests
func (h *Handlers) GetAviatrixBGPNeighbors(w http.ResponseWriter, r *http.Request) {
	w.Header().Set("Content-Type", "application/json")
	
	gatewayName := r.URL.Query().Get("gateway")
	if gatewayName == "" {
		http.Error(w, "Gateway name is required", http.StatusBadRequest)
		return
	}

	neighbors, err := h.aviatrix.GetBGPNeighbors(r.Context(), gatewayName)
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	w.WriteHeader(http.StatusOK)
	json.NewEncoder(w).Encode(neighbors)
}

// GetAviatrixRoutes handles Aviatrix route requests
func (h *Handlers) GetAviatrixRoutes(w http.ResponseWriter, r *http.Request) {
	w.Header().Set("Content-Type", "application/json")
	
	gatewayName := r.URL.Query().Get("gateway")
	if gatewayName == "" {
		http.Error(w, "Gateway name is required", http.StatusBadRequest)
		return
	}

	routes, err := h.aviatrix.GetRoutes(r.Context(), gatewayName)
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	w.WriteHeader(http.StatusOK)
	json.NewEncoder(w).Encode(routes)
}

// GetAviatrixTopology handles Aviatrix topology requests
func (h *Handlers) GetAviatrixTopology(w http.ResponseWriter, r *http.Request) {
	w.Header().Set("Content-Type", "application/json")
	
	topology, err := h.aviatrix.GetTopology(r.Context())
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	w.WriteHeader(http.StatusOK)
	json.NewEncoder(w).Encode(topology)
}

// GetCloudPodsTopology handles CloudPods topology requests
func (h *Handlers) GetCloudPodsTopology(w http.ResponseWriter, r *http.Request) {
	w.Header().Set("Content-Type", "application/json")
	
	topology, err := h.cloudpods.GetTopology(r.Context())
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	w.WriteHeader(http.StatusOK)
	json.NewEncoder(w).Encode(topology)
}

// GetCloudPodsMetrics handles CloudPods metrics requests
func (h *Handlers) GetCloudPodsMetrics(w http.ResponseWriter, r *http.Request) {
	w.Header().Set("Content-Type", "application/json")
	
	resourceType := r.URL.Query().Get("type")
	resourceID := r.URL.Query().Get("id")
	
	if resourceType == "" || resourceID == "" {
		http.Error(w, "Resource type and ID are required", http.StatusBadRequest)
		return
	}

	metrics, err := h.cloudpods.GetResourceMetrics(r.Context(), resourceType, resourceID)
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	w.WriteHeader(http.StatusOK)
	json.NewEncoder(w).Encode(metrics)
}

// GetAviatrixGatewayStatus handles Aviatrix gateway status requests
func (h *Handlers) GetAviatrixGatewayStatus(w http.ResponseWriter, r *http.Request) {
	w.Header().Set("Content-Type", "application/json")
	
	gatewayName := r.URL.Query().Get("gateway")
	if gatewayName == "" {
		http.Error(w, "Gateway name is required", http.StatusBadRequest)
		return
	}

	status, err := h.aviatrix.GetGatewayStatus(r.Context(), gatewayName)
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	w.WriteHeader(http.StatusOK)
	json.NewEncoder(w).Encode(status)
}

// InsertPacketMetrics handles packet metrics insertion
func (h *Handlers) InsertPacketMetrics(w http.ResponseWriter, r *http.Request) {
	w.Header().Set("Content-Type", "application/json")
	
	var metrics analytics.PacketMetrics
	if err := json.NewDecoder(r.Body).Decode(&metrics); err != nil {
		http.Error(w, err.Error(), http.StatusBadRequest)
		return
	}

	if err := h.analytics.InsertPacketMetrics(metrics); err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	w.WriteHeader(http.StatusCreated)
	json.NewEncoder(w).Encode(map[string]string{"status": "created"})
}

// InsertRouteMetrics handles route metrics insertion
func (h *Handlers) InsertRouteMetrics(w http.ResponseWriter, r *http.Request) {
	w.Header().Set("Content-Type", "application/json")
	
	var metrics analytics.RouteMetrics
	if err := json.NewDecoder(r.Body).Decode(&metrics); err != nil {
		http.Error(w, err.Error(), http.StatusBadRequest)
		return
	}

	if err := h.analytics.InsertRouteMetrics(metrics); err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	w.WriteHeader(http.StatusCreated)
	json.NewEncoder(w).Encode(map[string]string{"status": "created"})
}

// InsertSystemMetrics handles system metrics insertion
func (h *Handlers) InsertSystemMetrics(w http.ResponseWriter, r *http.Request) {
	w.Header().Set("Content-Type", "application/json")
	
	var metrics analytics.SystemMetrics
	if err := json.NewDecoder(r.Body).Decode(&metrics); err != nil {
		http.Error(w, err.Error(), http.StatusBadRequest)
		return
	}

	if err := h.analytics.InsertSystemMetrics(metrics); err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	w.WriteHeader(http.StatusCreated)
	json.NewEncoder(w).Encode(map[string]string{"status": "created"})
}

// InsertTrafficFlow handles traffic flow insertion
func (h *Handlers) InsertTrafficFlow(w http.ResponseWriter, r *http.Request) {
	w.Header().Set("Content-Type", "application/json")
	
	var flow analytics.TrafficFlow
	if err := json.NewDecoder(r.Body).Decode(&flow); err != nil {
		http.Error(w, err.Error(), http.StatusBadRequest)
		return
	}

	if err := h.analytics.InsertTrafficFlow(flow); err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	w.WriteHeader(http.StatusCreated)
	json.NewEncoder(w).Encode(map[string]string{"status": "created"})
}

// GetRecentActivity handles recent activity requests
func (h *Handlers) GetRecentActivity(w http.ResponseWriter, r *http.Request) {
	w.Header().Set("Content-Type", "application/json")
	
	// Get limit from query parameter
	limitStr := r.URL.Query().Get("limit")
	limit := 10
	if limitStr != "" {
		if l, err := strconv.Atoi(limitStr); err == nil && l > 0 {
			limit = l
		}
	}

	activities := []map[string]interface{}{
		{
			"id":        "1",
			"timestamp": time.Now().Add(-5 * time.Minute).Format("2006-01-02 15:04:05"),
			"type":      "route_update",
			"message":   "BGP route 10.0.0.0/8 advertised to neighbor 192.168.1.2",
			"severity":  "info",
		},
		{
			"id":        "2",
			"timestamp": time.Now().Add(-10 * time.Minute).Format("2006-01-02 15:04:05"),
			"type":      "neighbor_change",
			"message":   "OSPF neighbor 10.0.1.2 state changed to Full",
			"severity":  "info",
		},
		{
			"id":        "3",
			"timestamp": time.Now().Add(-15 * time.Minute).Format("2006-01-02 15:04:05"),
			"type":      "interface_change",
			"message":   "Interface eth1 status changed to UP",
			"severity":  "info",
		},
		{
			"id":        "4",
			"timestamp": time.Now().Add(-20 * time.Minute).Format("2006-01-02 15:04:05"),
			"type":      "error",
			"message":   "Packet loss detected on interface eth2 (0.5%)",
			"severity":  "warning",
		},
	}

	// Limit results
	if limit < len(activities) {
		activities = activities[:limit]
	}

	w.WriteHeader(http.StatusOK)
	json.NewEncoder(w).Encode(activities)
}
