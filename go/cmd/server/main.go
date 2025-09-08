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

	"router-sim/internal/analytics"
	"router-sim/internal/aviatrix"
	"router-sim/internal/cloudpods"
	"router-sim/internal/config"
	"router-sim/internal/handlers"
	"router-sim/internal/server"
)

func main() {
	var (
		configPath = flag.String("config", "config.yaml", "Path to configuration file")
		port       = flag.Int("port", 8080, "Port to listen on")
		host       = flag.String("host", "0.0.0.0", "Host to bind to")
	)
	flag.Parse()

	// Load configuration
	cfg, err := config.Load(*configPath)
	if err != nil {
		log.Fatalf("Failed to load configuration: %v", err)
	}

	// Override config with command line flags
	cfg.API.Port = *port
	cfg.API.Host = *host

	// Initialize analytics engine
	analyticsEngine, err := analytics.NewEngine(cfg.Analytics)
	if err != nil {
		log.Fatalf("Failed to initialize analytics engine: %v", err)
	}

	// Initialize cloud providers
	cloudpodsClient, err := cloudpods.NewClient(cfg.CloudPods)
	if err != nil {
		log.Fatalf("Failed to initialize CloudPods client: %v", err)
	}

	aviatrixClient, err := aviatrix.NewClient(cfg.Aviatrix)
	if err != nil {
		log.Fatalf("Failed to initialize Aviatrix client: %v", err)
	}

	// Initialize handlers
	handlers := handlers.New(analyticsEngine, cloudpodsClient, aviatrixClient)

	// Create HTTP server
	srv := server.New(cfg, handlers)

	// Start server in a goroutine
	go func() {
		log.Printf("Starting server on %s:%d", cfg.API.Host, cfg.API.Port)
		if err := srv.Start(); err != nil && err != http.ErrServerClosed {
			log.Fatalf("Failed to start server: %v", err)
		}
	}()

	// Wait for interrupt signal to gracefully shutdown the server
	quit := make(chan os.Signal, 1)
	signal.Notify(quit, syscall.SIGINT, syscall.SIGTERM)
	<-quit
	log.Println("Shutting down server...")

	// Give outstanding requests 30 seconds to complete
	ctx, cancel := context.WithTimeout(context.Background(), 30*time.Second)
	defer cancel()

	if err := srv.Shutdown(ctx); err != nil {
		log.Fatalf("Server forced to shutdown: %v", err)
	}

	log.Println("Server exited")
}
