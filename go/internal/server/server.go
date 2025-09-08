package server

import (
	"net/http"
	"time"

	"router-sim/internal/config"
	"router-sim/internal/cloudpods"
	"router-sim/internal/aviatrix"
	"router-sim/internal/analytics"
	"router-sim/internal/handlers"

	"github.com/gin-gonic/gin"
	"github.com/gin-contrib/cors"
	"github.com/sirupsen/logrus"
)

type Server struct {
	config         *config.Config
	cloudpods      *cloudpods.Client
	aviatrix       *aviatrix.Client
	analytics      *analytics.Engine
	router         *gin.Engine
	logger         *logrus.Logger
}

func New(cfg *config.Config, cloudpodsClient *cloudpods.Client, aviatrixClient *aviatrix.Client, analyticsEngine *analytics.Engine) *Server {
	// Setup logging
	logger := logrus.New()
	if cfg.Log.JSON {
		logger.SetFormatter(&logrus.JSONFormatter{})
	} else {
		logger.SetFormatter(&logrus.TextFormatter{})
	}
	
	level, err := logrus.ParseLevel(cfg.Log.Level)
	if err != nil {
		level = logrus.InfoLevel
	}
	logger.SetLevel(level)

	// Setup Gin mode
	if cfg.Log.Level == "debug" {
		gin.SetMode(gin.DebugMode)
	} else {
		gin.SetMode(gin.ReleaseMode)
	}

	// Create router
	router := gin.New()
	router.Use(gin.Logger())
	router.Use(gin.Recovery())

	// Setup CORS
	if cfg.API.CORS.Enabled {
		corsConfig := cors.Config{
			AllowOrigins:     cfg.API.CORS.AllowOrigins,
			AllowMethods:     cfg.API.CORS.AllowMethods,
			AllowHeaders:     cfg.API.CORS.AllowHeaders,
			ExposeHeaders:    []string{"Content-Length"},
			AllowCredentials: true,
			MaxAge:           12 * time.Hour,
		}
		router.Use(cors.New(corsConfig))
	}

	server := &Server{
		config:    cfg,
		cloudpods: cloudpodsClient,
		aviatrix:  aviatrixClient,
		analytics: analyticsEngine,
		router:    router,
		logger:    logger,
	}

	return server
}

func (s *Server) Router() *gin.Engine {
	return s.router
}

func (s *Server) SetupRoutes() {
	// Create handlers
	statusHandler := handlers.NewStatusHandler(s.logger)
	cloudpodsHandler := handlers.NewCloudPodsHandler(s.cloudpods, s.logger)
	aviatrixHandler := handlers.NewAviatrixHandler(s.aviatrix, s.logger)
	analyticsHandler := handlers.NewAnalyticsHandler(s.analytics, s.logger)
	routingHandler := handlers.NewRoutingHandler(s.logger)

	// Health check
	s.router.GET("/health", statusHandler.HealthCheck)
	s.router.GET("/api/v1/status", statusHandler.GetStatus)

	// CloudPods routes
	cloudpodsGroup := s.router.Group("/api/v1/cloudpods")
	{
		// VPC management
		cloudpodsGroup.GET("/vpcs", cloudpodsHandler.ListVPCs)
		cloudpodsGroup.POST("/vpcs", cloudpodsHandler.CreateVPC)
		cloudpodsGroup.GET("/vpcs/:id", cloudpodsHandler.GetVPC)
		cloudpodsGroup.PUT("/vpcs/:id", cloudpodsHandler.UpdateVPC)
		cloudpodsGroup.DELETE("/vpcs/:id", cloudpodsHandler.DeleteVPC)
		cloudpodsGroup.GET("/vpcs/:id/stats", cloudpodsHandler.GetVPCStats)

		// Subnet management
		cloudpodsGroup.GET("/vpcs/:vpc_id/subnets", cloudpodsHandler.ListSubnets)
		cloudpodsGroup.POST("/vpcs/:vpc_id/subnets", cloudpodsHandler.CreateSubnet)
		cloudpodsGroup.GET("/vpcs/:vpc_id/subnets/:id", cloudpodsHandler.GetSubnet)
		cloudpodsGroup.PUT("/vpcs/:vpc_id/subnets/:id", cloudpodsHandler.UpdateSubnet)
		cloudpodsGroup.DELETE("/vpcs/:vpc_id/subnets/:id", cloudpodsHandler.DeleteSubnet)

		// NAT Gateway management
		cloudpodsGroup.GET("/vpcs/:vpc_id/nat-gateways", cloudpodsHandler.ListNATGateways)
		cloudpodsGroup.POST("/vpcs/:vpc_id/nat-gateways", cloudpodsHandler.CreateNATGateway)
		cloudpodsGroup.GET("/vpcs/:vpc_id/nat-gateways/:id", cloudpodsHandler.GetNATGateway)
		cloudpodsGroup.DELETE("/vpcs/:vpc_id/nat-gateways/:id", cloudpodsHandler.DeleteNATGateway)

		// Load Balancer management
		cloudpodsGroup.GET("/vpcs/:vpc_id/load-balancers", cloudpodsHandler.ListLoadBalancers)
		cloudpodsGroup.POST("/vpcs/:vpc_id/load-balancers", cloudpodsHandler.CreateLoadBalancer)
		cloudpodsGroup.GET("/vpcs/:vpc_id/load-balancers/:id", cloudpodsHandler.GetLoadBalancer)
		cloudpodsGroup.PUT("/vpcs/:vpc_id/load-balancers/:id", cloudpodsHandler.UpdateLoadBalancer)
		cloudpodsGroup.DELETE("/vpcs/:vpc_id/load-balancers/:id", cloudpodsHandler.DeleteLoadBalancer)

		// Service Mesh management
		cloudpodsGroup.GET("/vpcs/:vpc_id/service-mesh", cloudpodsHandler.ListServiceMeshRoutes)
		cloudpodsGroup.POST("/vpcs/:vpc_id/service-mesh", cloudpodsHandler.CreateServiceMeshRoute)
		cloudpodsGroup.GET("/vpcs/:vpc_id/service-mesh/:id", cloudpodsHandler.GetServiceMeshRoute)
		cloudpodsGroup.PUT("/vpcs/:vpc_id/service-mesh/:id", cloudpodsHandler.UpdateServiceMeshRoute)
		cloudpodsGroup.DELETE("/vpcs/:vpc_id/service-mesh/:id", cloudpodsHandler.DeleteServiceMeshRoute)
	}

	// Aviatrix routes
	aviatrixGroup := s.router.Group("/api/v1/aviatrix")
	{
		// Transit Gateway management
		aviatrixGroup.GET("/transit-gateways", aviatrixHandler.ListTransitGateways)
		aviatrixGroup.POST("/transit-gateways", aviatrixHandler.CreateTransitGateway)
		aviatrixGroup.GET("/transit-gateways/:name", aviatrixHandler.GetTransitGateway)
		aviatrixGroup.PUT("/transit-gateways/:name", aviatrixHandler.UpdateTransitGateway)
		aviatrixGroup.DELETE("/transit-gateways/:name", aviatrixHandler.DeleteTransitGateway)

		// Spoke Gateway management
		aviatrixGroup.GET("/spoke-gateways", aviatrixHandler.ListSpokeGateways)
		aviatrixGroup.POST("/spoke-gateways", aviatrixHandler.CreateSpokeGateway)
		aviatrixGroup.GET("/spoke-gateways/:name", aviatrixHandler.GetSpokeGateway)
		aviatrixGroup.PUT("/spoke-gateways/:name", aviatrixHandler.UpdateSpokeGateway)
		aviatrixGroup.DELETE("/spoke-gateways/:name", aviatrixHandler.DeleteSpokeGateway)

		// VPC Connection management
		aviatrixGroup.GET("/vpc-connections", aviatrixHandler.ListVPCConnections)
		aviatrixGroup.POST("/vpc-connections", aviatrixHandler.CreateVPCConnection)
		aviatrixGroup.GET("/vpc-connections/:name", aviatrixHandler.GetVPCConnection)
		aviatrixGroup.DELETE("/vpc-connections/:name", aviatrixHandler.DeleteVPCConnection)

		// Site-to-Cloud connections
		aviatrixGroup.GET("/site2cloud", aviatrixHandler.ListSite2CloudConnections)
		aviatrixGroup.POST("/site2cloud", aviatrixHandler.CreateSite2CloudConnection)
		aviatrixGroup.GET("/site2cloud/:name", aviatrixHandler.GetSite2CloudConnection)
		aviatrixGroup.DELETE("/site2cloud/:name", aviatrixHandler.DeleteSite2CloudConnection)
	}

	// Analytics routes
	analyticsGroup := s.router.Group("/api/v1/analytics")
	{
		analyticsGroup.GET("/traffic", analyticsHandler.GetTrafficStats)
		analyticsGroup.GET("/performance", analyticsHandler.GetPerformanceMetrics)
		analyticsGroup.GET("/routing", analyticsHandler.GetRoutingStats)
		analyticsGroup.GET("/cloudpods", analyticsHandler.GetCloudPodsStats)
		analyticsGroup.GET("/aviatrix", analyticsHandler.GetAviatrixStats)
	}

	// Routing routes
	routingGroup := s.router.Group("/api/v1/routing")
	{
		routingGroup.GET("/stats", routingHandler.GetRoutingStats)
		routingGroup.GET("/routes", routingHandler.GetRoutes)
		routingGroup.POST("/routes", routingHandler.AddRoute)
		routingGroup.DELETE("/routes/:destination", routingHandler.RemoveRoute)
		routingGroup.GET("/protocols", routingHandler.GetProtocols)
		routingGroup.POST("/protocols/:name/start", routingHandler.StartProtocol)
		routingGroup.POST("/protocols/:name/stop", routingHandler.StopProtocol)
	}
}
