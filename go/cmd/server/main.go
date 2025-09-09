package main

import (
	"context"
	"flag"
	"fmt"
	"log"
	"net/http"
	"os"
	"os/signal"
	"syscall"
	"time"

	"github.com/gin-gonic/gin"
	"github.com/gin-contrib/cors"
	"github.com/prometheus/client_golang/prometheus/promhttp"
	"go.uber.org/zap"
	"go.uber.org/zap/zapcore"

	"router-sim/internal/config"
	"router-sim/internal/server"
	"router-sim/internal/cloudpods"
	"router-sim/internal/aviatrix"
	"router-sim/internal/analytics"
)

var (
	version   = "dev"
	buildTime = "unknown"
	gitCommit = "unknown"
)

func main() {
	var (
		configFile = flag.String("config", "config.yaml", "Configuration file path")
		port       = flag.Int("port", 8080, "Server port")
		host       = flag.String("host", "0.0.0.0", "Server host")
		verbose    = flag.Bool("verbose", false, "Enable verbose logging")
		versionFlag = flag.Bool("version", false, "Show version information")
	)
	flag.Parse()

	if *versionFlag {
		fmt.Printf("Router Simulator Server\n")
		fmt.Printf("Version: %s\n", version)
		fmt.Printf("Build Time: %s\n", buildTime)
		fmt.Printf("Git Commit: %s\n", gitCommit)
		return
	}

	// Load configuration
	cfg, err := config.Load(*configFile)
	if err != nil {
		log.Fatalf("Failed to load configuration: %v", err)
	}

	// Override config with command line flags
	if *port != 8080 {
		cfg.API.Port = *port
	}
	if *host != "0.0.0.0" {
		cfg.API.Host = *host
	}
	if *verbose {
		cfg.Log.Level = "debug"
	}

	// Initialize logger
	logger, err := initLogger(cfg)
	if err != nil {
		log.Fatalf("Failed to initialize logger: %v", err)
	}
	defer logger.Sync()

	logger.Info("Starting Router Simulator Server",
		zap.String("version", version),
		zap.String("build_time", buildTime),
		zap.String("git_commit", gitCommit),
	)

	// Initialize services
	cloudPodsService, err := cloudpods.NewService(cfg.CloudPods, logger)
	if err != nil {
		logger.Fatal("Failed to initialize CloudPods service", zap.Error(err))
	}

	aviatrixService, err := aviatrix.NewService(cfg.Aviatrix, logger)
	if err != nil {
		logger.Fatal("Failed to initialize Aviatrix service", zap.Error(err))
	}

	analyticsService, err := analytics.NewService(cfg.Monitoring, logger)
	if err != nil {
		logger.Fatal("Failed to initialize analytics service", zap.Error(err))
	}

	// Initialize HTTP server
	router := setupRouter(cfg, cloudPodsService, aviatrixService, analyticsService, logger)

	// Create HTTP server
	srv := &http.Server{
		Addr:         fmt.Sprintf("%s:%d", cfg.API.Host, cfg.API.Port),
		Handler:      router,
		ReadTimeout:  cfg.API.ReadTimeout,
		WriteTimeout: cfg.API.WriteTimeout,
		IdleTimeout:  cfg.API.IdleTimeout,
	}

	// Start server in goroutine
	go func() {
		logger.Info("Starting HTTP server",
			zap.String("host", cfg.API.Host),
			zap.Int("port", cfg.API.Port),
		)
		if err := srv.ListenAndServe(); err != nil && err != http.ErrServerClosed {
			logger.Fatal("Failed to start HTTP server", zap.Error(err))
		}
	}()

	// Start metrics server
	go func() {
		metricsAddr := fmt.Sprintf("%s:%d", cfg.API.Host, cfg.Monitoring.Prometheus.Port)
		logger.Info("Starting metrics server", zap.String("addr", metricsAddr))
		metricsMux := http.NewServeMux()
		metricsMux.Handle("/metrics", promhttp.Handler())
		if err := http.ListenAndServe(metricsAddr, metricsMux); err != nil && err != http.ErrServerClosed {
			logger.Fatal("Failed to start metrics server", zap.Error(err))
		}
	}()

	// Wait for interrupt signal to gracefully shutdown the server
	quit := make(chan os.Signal, 1)
	signal.Notify(quit, syscall.SIGINT, syscall.SIGTERM)
	<-quit
	logger.Info("Shutting down server...")

	// Give outstanding requests 30 seconds to complete
	ctx, cancel := context.WithTimeout(context.Background(), 30*time.Second)
	defer cancel()

	if err := srv.Shutdown(ctx); err != nil {
		logger.Fatal("Server forced to shutdown", zap.Error(err))
	}

	logger.Info("Server exited")
}

func initLogger(cfg *config.Config) (*zap.Logger, error) {
	var config zap.Config
	if cfg.Log.JSON {
		config = zap.NewProductionConfig()
	} else {
		config = zap.NewDevelopmentConfig()
	}

	config.Level = zap.NewAtomicLevelAt(parseLogLevel(cfg.Log.Level))
	config.OutputPaths = []string{"stdout"}
	if cfg.Log.File != "" {
		config.OutputPaths = append(config.OutputPaths, cfg.Log.File)
	}

	return config.Build()
}

func parseLogLevel(level string) zapcore.Level {
	switch level {
	case "debug":
		return zapcore.DebugLevel
	case "info":
		return zapcore.InfoLevel
	case "warn":
		return zapcore.WarnLevel
	case "error":
		return zapcore.ErrorLevel
	default:
		return zapcore.InfoLevel
	}
}

func setupRouter(cfg *config.Config, cloudPodsService *cloudpods.Service, aviatrixService *aviatrix.Service, analyticsService *analytics.Service, logger *zap.Logger) *gin.Engine {
	// Set Gin mode
	if cfg.Log.Level == "debug" {
		gin.SetMode(gin.DebugMode)
	} else {
		gin.SetMode(gin.ReleaseMode)
	}

	router := gin.New()

	// Middleware
	router.Use(gin.Logger())
	router.Use(gin.Recovery())
	router.Use(cors.New(cors.Config{
		AllowOrigins:     cfg.API.CORS.AllowOrigins,
		AllowMethods:     cfg.API.CORS.AllowMethods,
		AllowHeaders:     cfg.API.CORS.AllowHeaders,
		ExposeHeaders:    []string{"Content-Length"},
		AllowCredentials: true,
		MaxAge:           12 * time.Hour,
	}))

	// Health check
	router.GET("/health", func(c *gin.Context) {
		c.JSON(http.StatusOK, gin.H{
			"status":    "healthy",
			"timestamp": time.Now().UTC(),
			"version":   version,
		})
	})

	// API routes
	v1 := router.Group("/api/v1")
	{
		// CloudPods routes
		cloudpodsGroup := v1.Group("/cloudpods")
		{
			cloudpodsGroup.GET("/resources", cloudPodsService.ListResources)
			cloudpodsGroup.POST("/resources", cloudPodsService.CreateResource)
			cloudpodsGroup.GET("/resources/:id", cloudPodsService.GetResource)
			cloudpodsGroup.PUT("/resources/:id", cloudPodsService.UpdateResource)
			cloudpodsGroup.DELETE("/resources/:id", cloudPodsService.DeleteResource)
			cloudpodsGroup.GET("/instances", cloudPodsService.ListInstances)
			cloudpodsGroup.GET("/networks", cloudPodsService.ListNetworks)
			cloudpodsGroup.GET("/storages", cloudPodsService.ListStorages)
			cloudpodsGroup.GET("/loadbalancers", cloudPodsService.ListLoadBalancers)
		}

		// Aviatrix routes
		aviatrixGroup := v1.Group("/aviatrix")
		{
			aviatrixGroup.GET("/gateways", aviatrixService.ListGateways)
			aviatrixGroup.POST("/gateways", aviatrixService.CreateGateway)
			aviatrixGroup.GET("/gateways/:id", aviatrixService.GetGateway)
			aviatrixGroup.DELETE("/gateways/:id", aviatrixService.DeleteGateway)
			aviatrixGroup.GET("/transit-gateways", aviatrixService.ListTransitGateways)
			aviatrixGroup.POST("/transit-gateways", aviatrixService.CreateTransitGateway)
			aviatrixGroup.GET("/spoke-gateways", aviatrixService.ListSpokeGateways)
			aviatrixGroup.POST("/spoke-gateways", aviatrixService.CreateSpokeGateway)
		}

		// Router simulation routes
		routerGroup := v1.Group("/router")
		{
			routerGroup.GET("/status", server.GetRouterStatus)
			routerGroup.GET("/interfaces", server.GetInterfaces)
			routerGroup.GET("/routes", server.GetRoutes)
			routerGroup.GET("/protocols", server.GetProtocols)
			routerGroup.POST("/protocols/:name/start", server.StartProtocol)
			routerGroup.POST("/protocols/:name/stop", server.StopProtocol)
			routerGroup.GET("/traffic-shaping", server.GetTrafficShaping)
			routerGroup.POST("/traffic-shaping", server.UpdateTrafficShaping)
			routerGroup.GET("/impairments", server.GetImpairments)
			routerGroup.POST("/impairments", server.UpdateImpairments)
		}

		// Analytics routes
		analyticsGroup := v1.Group("/analytics")
		{
			analyticsGroup.GET("/metrics", analyticsService.GetMetrics)
			analyticsGroup.GET("/dashboard", analyticsService.GetDashboard)
			analyticsGroup.GET("/reports", analyticsService.GetReports)
		}
	}

	// Static files for web UI
	router.Static("/static", "./web/dist")
	router.StaticFile("/", "./web/dist/index.html")
	router.StaticFile("/favicon.ico", "./web/dist/favicon.ico")

	// Catch-all for SPA routing
	router.NoRoute(func(c *gin.Context) {
		c.File("./web/dist/index.html")
	})

	return router
}

