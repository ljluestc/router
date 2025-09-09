package handlers

import (
	"net/http"

	"router-sim/internal/analytics"
	"router-sim/internal/aviatrix"
	"router-sim/internal/cloudpods"
	"router-sim/internal/server"

	"github.com/gin-gonic/gin"
)

// Handlers contains all HTTP handlers
type Handlers struct {
	analytics  *analytics.Service
	aviatrix   *aviatrix.Service
	cloudpods  *cloudpods.Service
}

// NewHandlers creates a new handlers instance
func NewHandlers(analytics *analytics.Service, aviatrix *aviatrix.Service, cloudpods *cloudpods.Service) *Handlers {
	return &Handlers{
		analytics: analytics,
		aviatrix:  aviatrix,
		cloudpods: cloudpods,
	}
}

// SetupRoutes sets up all HTTP routes
func (h *Handlers) SetupRoutes(r *gin.Engine) {
	// API routes
	api := r.Group("/api/v1")
	{
		// Router status and monitoring
		api.GET("/status", server.GetRouterStatus)
		api.GET("/interfaces", server.GetInterfaces)
		api.GET("/routes", server.GetRoutes)
		api.GET("/protocols", server.GetProtocols)
		api.POST("/protocols/:name/start", server.StartProtocol)
		api.POST("/protocols/:name/stop", server.StopProtocol)
		
		// Traffic shaping
		api.GET("/traffic-shaping", server.GetTrafficShaping)
		api.PUT("/traffic-shaping", server.UpdateTrafficShaping)
		
		// Network impairments
		api.GET("/impairments", server.GetImpairments)
		api.PUT("/impairments", server.UpdateImpairments)
		
		// CloudPods integration
		cloudpods := api.Group("/cloudpods")
		{
			cloudpods.GET("/projects", h.GetCloudPodsProjects)
			cloudpods.GET("/networks", h.GetCloudPodsNetworks)
			cloudpods.GET("/instances", h.GetCloudPodsInstances)
		}
		
		// Aviatrix integration
		aviatrix := api.Group("/aviatrix")
		{
			aviatrix.GET("/gateways", h.GetAviatrixGateways)
			aviatrix.GET("/transit-gateways", h.GetAviatrixTransitGateways)
			aviatrix.GET("/spoke-gateways", h.GetAviatrixSpokeGateways)
		}
		
		// Analytics
		analytics := api.Group("/analytics")
		{
			analytics.GET("/metrics", h.GetAnalyticsMetrics)
			analytics.GET("/traffic", h.GetTrafficAnalytics)
			analytics.GET("/performance", h.GetPerformanceAnalytics)
		}
	}
	
	// Health check
	r.GET("/health", func(c *gin.Context) {
		c.JSON(http.StatusOK, gin.H{"status": "healthy"})
	})
}

// CloudPods handlers
func (h *Handlers) GetCloudPodsProjects(c *gin.Context) {
	projects := []gin.H{
		{"id": "proj-1", "name": "Production", "region": "us-west-1", "status": "active"},
		{"id": "proj-2", "name": "Development", "region": "us-east-1", "status": "active"},
		{"id": "proj-3", "name": "Testing", "region": "eu-west-1", "status": "inactive"},
	}
	c.JSON(http.StatusOK, gin.H{"projects": projects})
}

func (h *Handlers) GetCloudPodsNetworks(c *gin.Context) {
	networks := []gin.H{
		{"id": "net-1", "name": "VPC-Prod", "cidr": "10.0.0.0/16", "region": "us-west-1", "status": "active"},
		{"id": "net-2", "name": "VPC-Dev", "cidr": "10.1.0.0/16", "region": "us-east-1", "status": "active"},
		{"id": "net-3", "name": "VPC-Test", "cidr": "10.2.0.0/16", "region": "eu-west-1", "status": "inactive"},
	}
	c.JSON(http.StatusOK, gin.H{"networks": networks})
}

func (h *Handlers) GetCloudPodsInstances(c *gin.Context) {
	instances := []gin.H{
		{"id": "inst-1", "name": "Web-Server-1", "type": "t3.medium", "status": "running", "ip": "10.0.1.10"},
		{"id": "inst-2", "name": "DB-Server-1", "type": "t3.large", "status": "running", "ip": "10.0.2.10"},
		{"id": "inst-3", "name": "App-Server-1", "type": "t3.small", "status": "stopped", "ip": "10.0.3.10"},
	}
	c.JSON(http.StatusOK, gin.H{"instances": instances})
}

// Aviatrix handlers
func (h *Handlers) GetAviatrixGateways(c *gin.Context) {
	gateways := []gin.H{
		{"name": "gw-aws-us-west-1", "cloud": "AWS", "region": "us-west-1", "status": "up", "type": "transit"},
		{"name": "gw-aws-us-east-1", "cloud": "AWS", "region": "us-east-1", "status": "up", "type": "transit"},
		{"name": "gw-azure-westus", "cloud": "Azure", "region": "westus", "status": "up", "type": "spoke"},
	}
	c.JSON(http.StatusOK, gin.H{"gateways": gateways})
}

func (h *Handlers) GetAviatrixTransitGateways(c *gin.Context) {
	transitGateways := []gin.H{
		{"name": "transit-gw-1", "cloud": "AWS", "region": "us-west-1", "status": "up", "asn": 65001},
		{"name": "transit-gw-2", "cloud": "AWS", "region": "us-east-1", "status": "up", "asn": 65002},
	}
	c.JSON(http.StatusOK, gin.H{"transit_gateways": transitGateways})
}

func (h *Handlers) GetAviatrixSpokeGateways(c *gin.Context) {
	spokeGateways := []gin.H{
		{"name": "spoke-gw-1", "cloud": "Azure", "region": "westus", "status": "up", "vpc_id": "vpc-12345"},
		{"name": "spoke-gw-2", "cloud": "GCP", "region": "us-central1", "status": "up", "vpc_id": "vpc-67890"},
	}
	c.JSON(http.StatusOK, gin.H{"spoke_gateways": spokeGateways})
}

// Analytics handlers
func (h *Handlers) GetAnalyticsMetrics(c *gin.Context) {
	metrics := gin.H{
		"cpu_usage": 45.2,
		"memory_usage": 67.8,
		"network_rx": 1024000,
		"network_tx": 2048000,
		"packet_loss": 0.001,
		"latency": 12.5,
	}
	c.JSON(http.StatusOK, metrics)
}

func (h *Handlers) GetTrafficAnalytics(c *gin.Context) {
	traffic := gin.H{
		"total_packets": 1000000,
		"total_bytes": 1000000000,
		"protocols": []gin.H{
			{"name": "TCP", "packets": 800000, "bytes": 800000000},
			{"name": "UDP", "packets": 150000, "bytes": 150000000},
			{"name": "ICMP", "packets": 50000, "bytes": 50000000},
		},
		"interfaces": []gin.H{
			{"name": "eth0", "rx_packets": 500000, "tx_packets": 500000},
			{"name": "eth1", "rx_packets": 300000, "tx_packets": 200000},
		},
	}
	c.JSON(http.StatusOK, traffic)
}

func (h *Handlers) GetPerformanceAnalytics(c *gin.Context) {
	performance := gin.H{
		"throughput": gin.H{
			"current": 1000,
			"average": 950,
			"peak": 1200,
		},
		"latency": gin.H{
			"current": 12.5,
			"average": 15.2,
			"p95": 25.0,
			"p99": 50.0,
		},
		"errors": gin.H{
			"total": 100,
			"rate": 0.01,
			"by_type": []gin.H{
				{"type": "timeout", "count": 50},
				{"type": "connection_refused", "count": 30},
				{"type": "other", "count": 20},
			},
		},
	}
	c.JSON(http.StatusOK, performance)
}

