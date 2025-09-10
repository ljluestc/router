package main

import (
	"context"
	"encoding/json"
	"fmt"
	"log"
	"net/http"
	"os"
	"os/signal"
	"syscall"
	"time"

	"github.com/gin-gonic/gin"
	"github.com/gorilla/websocket"
	"router-sim/internal/analytics"
	"router-sim/internal/aviatrix"
	"router-sim/internal/cloudpods"
	"router-sim/internal/config"
	"router-sim/internal/handlers"
)

var upgrader = websocket.Upgrader{
	CheckOrigin: func(r *http.Request) bool {
		return true // Allow all origins in development
	},
}

func main() {
	// Load configuration
	cfg, err := config.Load()
	if err != nil {
		log.Fatalf("Failed to load configuration: %v", err)
	}

	// Initialize analytics engine
	analyticsEngine, err := analytics.NewEngine(cfg.Analytics)
	if err != nil {
		log.Fatalf("Failed to initialize analytics engine: %v", err)
	}

	// Initialize CloudPods client
	cloudpodsClient, err := cloudpods.NewClient(cfg.CloudPods)
	if err != nil {
		log.Fatalf("Failed to initialize CloudPods client: %v", err)
	}

	// Initialize Aviatrix client
	aviatrixClient, err := aviatrix.NewClient(cfg.Aviatrix)
	if err != nil {
		log.Fatalf("Failed to initialize Aviatrix client: %v", err)
	}

	// Initialize handlers
	handlers := handlers.New(analyticsEngine, aviatrixClient, cloudpodsClient)

	// Setup Gin router
	if cfg.Server.Environment == "production" {
		gin.SetMode(gin.ReleaseMode)
	}

	router := gin.Default()

	// CORS middleware
	router.Use(func(c *gin.Context) {
		c.Header("Access-Control-Allow-Origin", "*")
		c.Header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS")
		c.Header("Access-Control-Allow-Headers", "Origin, Content-Type, Accept, Authorization")
		
		if c.Request.Method == "OPTIONS" {
			c.AbortWithStatus(204)
			return
		}
		
		c.Next()
	})

	// API routes
	api := router.Group("/api/v1")
	{
		// Analytics routes
		api.GET("/analytics/metrics", handlers.GetMetrics)
		api.GET("/analytics/dashboard", handlers.GetDashboard)
		api.POST("/analytics/query", handlers.QueryAnalytics)
		
		// CloudPods routes
		api.GET("/cloudpods/status", handlers.GetCloudPodsStatus)
		api.GET("/cloudpods/resources", handlers.GetCloudPodsResources)
		api.POST("/cloudpods/deploy", handlers.DeployCloudPods)
		api.DELETE("/cloudpods/cleanup", handlers.CleanupCloudPods)
		
		// Aviatrix routes
		api.GET("/aviatrix/status", handlers.GetAviatrixStatus)
		api.GET("/aviatrix/gateways", handlers.GetAviatrixGateways)
		api.POST("/aviatrix/deploy", handlers.DeployAviatrix)
		api.DELETE("/aviatrix/cleanup", handlers.CleanupAviatrix)
		
		// Router simulation routes
		api.GET("/router/status", handlers.GetRouterStatus)
		api.GET("/router/routes", handlers.GetRoutes)
		api.GET("/router/neighbors", handlers.GetNeighbors)
		api.POST("/router/scenario", handlers.LoadScenario)
		api.POST("/router/impairment", handlers.ApplyImpairment)
		
		// Testing routes
		api.POST("/test/capture", handlers.StartCapture)
		api.POST("/test/compare", handlers.ComparePCAPs)
		api.GET("/test/results", handlers.GetTestResults)
	}

	// WebSocket endpoint for real-time updates
	router.GET("/ws", func(c *gin.Context) {
		handleWebSocket(c, analyticsEngine)
	})

	// Serve static files
	router.Static("/static", "./web/dist")
	router.StaticFile("/", "./web/dist/index.html")

	// Start server
	server := &http.Server{
		Addr:    fmt.Sprintf(":%d", cfg.Server.Port),
		Handler: router,
	}

	// Start server in goroutine
	go func() {
		log.Printf("Starting server on port %d", cfg.Server.Port)
		if err := server.ListenAndServe(); err != nil && err != http.ErrServerClosed {
			log.Fatalf("Failed to start server: %v", err)
		}
	}()

	// Wait for interrupt signal
	quit := make(chan os.Signal, 1)
	signal.Notify(quit, syscall.SIGINT, syscall.SIGTERM)
	<-quit

	log.Println("Shutting down server...")

	// Graceful shutdown
	ctx, cancel := context.WithTimeout(context.Background(), 30*time.Second)
	defer cancel()

	if err := server.Shutdown(ctx); err != nil {
		log.Fatalf("Server forced to shutdown: %v", err)
	}

	log.Println("Server exited")
}

func handleWebSocket(c *gin.Context, analyticsEngine *analytics.Engine) {
	conn, err := upgrader.Upgrade(c.Writer, c.Request, nil)
	if err != nil {
		log.Printf("WebSocket upgrade error: %v", err)
		return
	}
	defer conn.Close()

	// Create a channel for analytics updates
	updates := make(chan analytics.MetricUpdate, 100)
	defer close(updates)

	// Subscribe to analytics updates
	analyticsEngine.Subscribe(updates)

	// Send updates to WebSocket client
	for update := range updates {
		data, err := json.Marshal(update)
		if err != nil {
			log.Printf("Error marshaling update: %v", err)
			continue
		}

		if err := conn.WriteMessage(websocket.TextMessage, data); err != nil {
			log.Printf("WebSocket write error: %v", err)
			break
		}
	}

	// Unsubscribe from analytics updates
	analyticsEngine.Unsubscribe(updates)
}