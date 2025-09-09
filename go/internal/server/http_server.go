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

// HTTPServer represents the HTTP server
type HTTPServer struct {
	config   *config.ServerConfig
	handlers *handlers.Handlers
	server   *http.Server
}

// New creates a new HTTP server
func New(config *config.ServerConfig, handlers *handlers.Handlers) *HTTPServer {
	return &HTTPServer{
		config:   config,
		handlers: handlers,
	}
}

// Start starts the HTTP server
func (s *HTTPServer) Start() error {
	// Set Gin mode
	if logrus.GetLevel() == logrus.DebugLevel {
		gin.SetMode(gin.DebugMode)
	} else {
		gin.SetMode(gin.ReleaseMode)
	}

	// Create Gin router
	r := gin.New()
	
	// Add middleware
	r.Use(gin.Logger())
	r.Use(gin.Recovery())
	
	// Add CORS middleware if enabled
	if s.config.EnableCORS {
		r.Use(func(c *gin.Context) {
			c.Header("Access-Control-Allow-Origin", "*")
			c.Header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS")
			c.Header("Access-Control-Allow-Headers", "Origin, Content-Type, Accept, Authorization")
			
			if c.Request.Method == "OPTIONS" {
				c.AbortWithStatus(204)
				return
			}
			
			c.Next()
		})
	}
	
	// Setup routes
	s.handlers.SetupRoutes(r)
	
	// Create HTTP server
	s.server = &http.Server{
		Addr:         fmt.Sprintf("%s:%d", s.config.Host, s.config.Port),
		Handler:      r,
		ReadTimeout:  s.config.ReadTimeout,
		WriteTimeout: s.config.WriteTimeout,
		IdleTimeout:  s.config.IdleTimeout,
	}
	
	logrus.WithFields(logrus.Fields{
		"host": s.config.Host,
		"port": s.config.Port,
	}).Info("Starting HTTP server")
	
	// Start server
	return s.server.ListenAndServe()
}

// Shutdown gracefully shuts down the server
func (s *HTTPServer) Shutdown(ctx context.Context) error {
	logrus.Info("Shutting down HTTP server...")
	return s.server.Shutdown(ctx)
}
