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

	"router-sim/internal/config"
	"router-sim/internal/server"
	"router-sim/internal/cloudpods"
	"router-sim/internal/aviatrix"
	"router-sim/internal/analytics"
)

var (
	configFile = flag.String("config", "config.yaml", "Configuration file path")
	port       = flag.Int("port", 8080, "Server port")
	host       = flag.String("host", "0.0.0.0", "Server host")
	debug      = flag.Bool("debug", false, "Enable debug mode")
)

func main() {
	flag.Parse()

	// Load configuration
	cfg, err := config.Load(*configFile)
	if err != nil {
		log.Fatalf("Failed to load configuration: %v", err)
	}

	// Override with command line flags
	if *port != 8080 {
		cfg.API.Port = *port
	}
	if *host != "0.0.0.0" {
		cfg.API.Host = *host
	}
	if *debug {
		cfg.Log.Level = "debug"
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

	// Initialize analytics engine
	analyticsEngine, err := analytics.NewEngine(cfg.Analytics)
	if err != nil {
		log.Fatalf("Failed to initialize analytics engine: %v", err)
	}

	// Create server
	srv := server.New(cfg, cloudpodsClient, aviatrixClient, analyticsEngine)

	// Setup routes
	srv.SetupRoutes()

	// Create HTTP server
	httpServer := &http.Server{
		Addr:         fmt.Sprintf("%s:%d", cfg.API.Host, cfg.API.Port),
		Handler:      srv.Router(),
		ReadTimeout:  cfg.API.ReadTimeout,
		WriteTimeout: cfg.API.WriteTimeout,
		IdleTimeout:  cfg.API.IdleTimeout,
	}

	// Start server in goroutine
	go func() {
		log.Printf("Starting server on %s:%d", cfg.API.Host, cfg.API.Port)
		if err := httpServer.ListenAndServe(); err != nil && err != http.ErrServerClosed {
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

	if err := httpServer.Shutdown(ctx); err != nil {
		log.Fatalf("Server forced to shutdown: %v", err)
	}

	log.Println("Server exited")
}
