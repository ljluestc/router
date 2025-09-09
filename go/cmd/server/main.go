package main

import (
	"context"
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

	"github.com/gin-gonic/gin"
	"github.com/sirupsen/logrus"
	"github.com/spf13/cobra"
	"github.com/spf13/viper"
)

var (
	version = "1.0.0"
	cfgFile string
	verbose bool
)

func main() {
	var rootCmd = &cobra.Command{
		Use:   "router-sim-server",
		Short: "Multi-Protocol Router Simulator Server",
		Long: `A comprehensive router simulation framework with CloudPods and Aviatrix integration.
Built with C++, Go, Rust, and modern web technologies.`,
		Run: runServer,
	}

	rootCmd.PersistentFlags().StringVar(&cfgFile, "config", "config.yaml", "config file")
	rootCmd.PersistentFlags().BoolVar(&verbose, "verbose", false, "verbose output")

	if err := rootCmd.Execute(); err != nil {
		log.Fatal(err)
	}
}

func runServer(cmd *cobra.Command, args []string) {
	// Initialize configuration
	config, err := config.Load(cfgFile)
	if err != nil {
		log.Fatalf("Failed to load config: %v", err)
	}

	// Set log level
	if verbose {
		logrus.SetLevel(logrus.DebugLevel)
	} else {
		logrus.SetLevel(logrus.InfoLevel)
	}

	logrus.SetFormatter(&logrus.JSONFormatter{})
	logrus.WithField("version", version).Info("Starting Router Simulator Server")

	// Initialize services
	analyticsService := analytics.NewService(config.Analytics)
	aviatrixService := aviatrix.NewService(config.Aviatrix)
	cloudpodsService := cloudpods.NewService(config.CloudPods)

	// Initialize handlers
	handlers := handlers.NewHandlers(analyticsService, aviatrixService, cloudpodsService)

	// Create server
	srv := server.New(config.Server, handlers)

	// Start server in goroutine
	go func() {
		logrus.WithField("port", config.Server.Port).Info("Starting HTTP server")
		if err := srv.Start(); err != nil && err != http.ErrServerClosed {
			logrus.Fatalf("Failed to start server: %v", err)
		}
	}()

	// Wait for interrupt signal
	quit := make(chan os.Signal, 1)
	signal.Notify(quit, syscall.SIGINT, syscall.SIGTERM)
	<-quit

	logrus.Info("Shutting down server...")

	// Graceful shutdown
	ctx, cancel := context.WithTimeout(context.Background(), 30*time.Second)
	defer cancel()

	if err := srv.Shutdown(ctx); err != nil {
		logrus.Fatalf("Server forced to shutdown: %v", err)
	}

	logrus.Info("Server exited")
}