package server

import (
	"context"
	"fmt"
	"net/http"
	"time"

	"router-sim/internal/config"
	"router-sim/internal/handlers"

	"github.com/gin-gonic/gin"
	"github.com/sirupsen/logrus"
)

// Server represents the HTTP server
type Server struct {
	config   *config.Config
	handlers *handlers.Handlers
	server   *http.Server
	router   *gin.Engine
}

// NewServer creates a new server instance
func NewServer(cfg *config.Config, handlers *handlers.Handlers) *Server {
	// Set Gin mode
	if cfg.Logging.Level == "debug" {
		gin.SetMode(gin.DebugMode)
	} else {
		gin.SetMode(gin.ReleaseMode)
	}

	// Create router
	router := gin.New()

	// Add middleware
	router.Use(gin.Logger())
	router.Use(gin.Recovery())
	router.Use(corsMiddleware())
	router.Use(requestIDMiddleware())
	router.Use(loggingMiddleware())

	// Create server
	server := &http.Server{
		Addr:           cfg.GetServerAddress(),
		Handler:        router,
		ReadTimeout:    cfg.Server.ReadTimeout,
		WriteTimeout:   cfg.Server.WriteTimeout,
		IdleTimeout:    cfg.Server.IdleTimeout,
		MaxHeaderBytes: cfg.Server.MaxHeaderBytes,
	}

	return &Server{
		config:   cfg,
		handlers: handlers,
		server:   server,
		router:   router,
	}
}

// Start starts the server
func (s *Server) Start() error {
	// Setup routes
	s.setupRoutes()

	// Start server
	logrus.Infof("Starting server on %s", s.server.Addr)
	return s.server.ListenAndServe()
}

// Shutdown gracefully shuts down the server
func (s *Server) Shutdown(ctx context.Context) error {
	logrus.Info("Shutting down server...")
	return s.server.Shutdown(ctx)
}

// setupRoutes sets up all the routes
func (s *Server) setupRoutes() {
	// Health check
	s.router.GET("/health", s.handlers.HealthCheck)

	// API v1 routes
	v1 := s.router.Group("/api/v1")
	{
		// CloudPods routes
		if s.config.IsCloudPodsEnabled() {
			cloudpods := v1.Group("/cloudpods")
			{
				cloudpods.GET("/networks", s.handlers.GetCloudPodsNetworks)
				cloudpods.GET("/networks/:id", s.handlers.GetCloudPodsNetwork)
				cloudpods.POST("/networks", s.handlers.CreateCloudPodsNetwork)
				cloudpods.GET("/vms", s.handlers.GetCloudPodsVMs)
				cloudpods.GET("/loadbalancers", s.handlers.GetCloudPodsLoadBalancers)
				cloudpods.GET("/resources/:type", s.handlers.GetCloudPodsResources)
			}
		}

		// Aviatrix routes
		if s.config.IsAviatrixEnabled() {
			aviatrix := v1.Group("/aviatrix")
			{
				aviatrix.GET("/gateways", s.handlers.GetAviatrixGateways)
				aviatrix.GET("/transit-gateways", s.handlers.GetAviatrixTransitGateways)
				aviatrix.GET("/spoke-gateways", s.handlers.GetAviatrixSpokeGateways)
				aviatrix.GET("/vpn-gateways", s.handlers.GetAviatrixVPNGateways)
				aviatrix.GET("/connections", s.handlers.GetAviatrixConnections)
				aviatrix.POST("/connections", s.handlers.CreateAviatrixConnection)
				aviatrix.GET("/routes", s.handlers.GetAviatrixRoutes)
			}
		}

		// Router simulation routes
		router := v1.Group("/router")
		{
			router.GET("/status", s.handlers.GetRouterStatus)
			router.GET("/interfaces", s.handlers.GetInterfaces)
			router.GET("/routes", s.handlers.GetRoutes)
			router.POST("/routes", s.handlers.AddRoute)
			router.DELETE("/routes/:destination", s.handlers.RemoveRoute)
			router.GET("/statistics", s.handlers.GetStatistics)
			router.POST("/reset", s.handlers.ResetRouter)
		}

		// Traffic shaping routes
		traffic := v1.Group("/traffic")
		{
			traffic.GET("/shaping", s.handlers.GetTrafficShaping)
			traffic.POST("/shaping", s.handlers.UpdateTrafficShaping)
			traffic.GET("/statistics", s.handlers.GetTrafficStatistics)
		}

		// Network impairments routes
		impairments := v1.Group("/impairments")
		{
			impairments.GET("/", s.handlers.GetImpairments)
			impairments.POST("/", s.handlers.ApplyImpairments)
			impairments.DELETE("/:interface", s.handlers.ClearImpairments)
		}

		// Analytics routes
		analytics := v1.Group("/analytics")
		{
			analytics.GET("/metrics", s.handlers.GetMetrics)
			analytics.GET("/packets", s.handlers.GetPacketAnalytics)
			analytics.GET("/routing", s.handlers.GetRoutingAnalytics)
		}
	}

	// WebSocket routes
	s.router.GET("/ws", s.handlers.WebSocketHandler)

	// Static file serving
	s.router.Static("/static", "./web/static")
	s.router.StaticFile("/", "./web/index.html")
}

// corsMiddleware adds CORS headers
func corsMiddleware() gin.HandlerFunc {
	return func(c *gin.Context) {
		c.Header("Access-Control-Allow-Origin", "*")
		c.Header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS")
		c.Header("Access-Control-Allow-Headers", "Origin, Content-Type, Content-Length, Accept-Encoding, X-CSRF-Token, Authorization")
		c.Header("Access-Control-Allow-Credentials", "true")

		if c.Request.Method == "OPTIONS" {
			c.AbortWithStatus(204)
			return
		}

		c.Next()
	}
}

// requestIDMiddleware adds a request ID to each request
func requestIDMiddleware() gin.HandlerFunc {
	return func(c *gin.Context) {
		requestID := c.GetHeader("X-Request-ID")
		if requestID == "" {
			requestID = generateRequestID()
		}
		c.Header("X-Request-ID", requestID)
		c.Set("request_id", requestID)
		c.Next()
	}
}

// loggingMiddleware logs requests
func loggingMiddleware() gin.HandlerFunc {
	return func(c *gin.Context) {
		start := time.Now()
		path := c.Request.URL.Path
		raw := c.Request.URL.RawQuery

		c.Next()

		latency := time.Since(start)
		clientIP := c.ClientIP()
		method := c.Request.Method
		statusCode := c.Writer.Status()
		bodySize := c.Writer.Size()

		if raw != "" {
			path = path + "?" + raw
		}

		logrus.WithFields(logrus.Fields{
			"status":     statusCode,
			"latency":    latency,
			"client_ip":  clientIP,
			"method":     method,
			"path":       path,
			"body_size":  bodySize,
			"request_id": c.GetString("request_id"),
		}).Info("HTTP Request")
	}
}

// generateRequestID generates a unique request ID
func generateRequestID() string {
	return fmt.Sprintf("%d", time.Now().UnixNano())
}
