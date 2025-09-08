package main

import (
	"context"
	"log"
	"net/http"
	"os"
	"os/signal"
	"syscall"
	"time"

	"router-sim/internal/config"
	"router-sim/internal/handlers"
	"router-sim/internal/server"
	"router-sim/internal/cloudpods"
	"router-sim/internal/aviatrix"

	"github.com/gin-gonic/gin"
	"github.com/sirupsen/logrus"
	"github.com/spf13/cobra"
	"github.com/spf13/viper"
)

var (
	cfgFile string
	verbose bool
)

func main() {
	var rootCmd = &cobra.Command{
		Use:   "router-sim-server",
		Short: "Multi-Protocol Router Simulator API Server",
		Long:  "A high-performance API server for the Multi-Protocol Router Simulator with CloudPods and Aviatrix integration",
		Run:   runServer,
	}

	rootCmd.PersistentFlags().StringVar(&cfgFile, "config", "", "config file (default is $HOME/.router-sim.yaml)")
	rootCmd.PersistentFlags().BoolVarP(&verbose, "verbose", "v", false, "verbose output")

	if err := rootCmd.Execute(); err != nil {
		log.Fatal(err)
	}
}

func runServer(cmd *cobra.Command, args []string) {
	// Initialize configuration
	cfg, err := config.Load(cfgFile)
	if err != nil {
		log.Fatalf("Failed to load configuration: %v", err)
	}

	// Set log level
	if verbose {
		logrus.SetLevel(logrus.DebugLevel)
	} else {
		logrus.SetLevel(logrus.InfoLevel)
	}

	// Initialize CloudPods client
	cloudpodsClient := cloudpods.NewCloudPodsClient(cloudpods.CloudPodsConfig{
		BaseURL: cfg.CloudPods.BaseURL,
		APIKey:  cfg.CloudPods.APIKey,
		Timeout: cfg.CloudPods.Timeout,
	})

	// Initialize Aviatrix client
	aviatrixClient := aviatrix.NewAviatrixClient(aviatrix.AviatrixConfig{
		BaseURL: cfg.Aviatrix.BaseURL,
		APIKey:  cfg.Aviatrix.APIKey,
		Timeout: cfg.Aviatrix.Timeout,
	})

	// Initialize handlers
	handlers := handlers.NewHandlers(cloudpodsClient, aviatrixClient)

	// Initialize server
	srv := server.NewServer(cfg, handlers)

	// Start server in a goroutine
	go func() {
		logrus.Infof("Starting server on %s", cfg.Server.Address)
		if err := srv.Start(); err != nil && err != http.ErrServerClosed {
			logrus.Fatalf("Failed to start server: %v", err)
		}
	}()

	// Wait for interrupt signal to gracefully shutdown the server
	quit := make(chan os.Signal, 1)
	signal.Notify(quit, syscall.SIGINT, syscall.SIGTERM)
	<-quit
	logrus.Info("Shutting down server...")

	// Give outstanding requests a deadline for completion
	ctx, cancel := context.WithTimeout(context.Background(), 30*time.Second)
	defer cancel()

	// Attempt graceful shutdown
	if err := srv.Shutdown(ctx); err != nil {
		logrus.Fatalf("Server forced to shutdown: %v", err)
	}

	logrus.Info("Server exited")
}

func init() {
	cobra.OnInitialize(initConfig)
}

func initConfig() {
	if cfgFile != "" {
		viper.SetConfigFile(cfgFile)
	} else {
		home, err := os.UserHomeDir()
		if err != nil {
			log.Fatal(err)
		}

		viper.AddConfigPath(home)
		viper.AddConfigPath(".")
		viper.SetConfigName(".router-sim")
		viper.SetConfigType("yaml")
	}

	viper.AutomaticEnv()

	if err := viper.ReadInConfig(); err == nil {
		logrus.Infof("Using config file: %s", viper.ConfigFileUsed())
	}
}
