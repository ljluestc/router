package server

import (
	"context"
	"fmt"
	"net/http"
	"time"

	"router-sim/internal/config"
	"router-sim/internal/handlers"
)

// Server represents the HTTP server
type Server struct {
	config   *config.Config
	handlers *handlers.Handlers
	server   *http.Server
}

// New creates a new Server instance
func New(cfg *config.Config, h *handlers.Handlers) *Server {
	mux := http.NewServeMux()

	// Health check endpoint
	mux.HandleFunc("/health", h.HealthCheck)

	// API endpoints
	mux.HandleFunc("/api/v1/status", h.GetStatus)
	mux.HandleFunc("/api/v1/metrics", h.GetMetrics)
	mux.HandleFunc("/api/v1/routes", h.GetRoutes)
	mux.HandleFunc("/api/v1/interfaces", h.GetInterfaces)
	mux.HandleFunc("/api/v1/neighbors", h.GetNeighbors)
	mux.HandleFunc("/api/v1/analytics", h.GetAnalytics)
	mux.HandleFunc("/api/v1/activity", h.GetRecentActivity)

	// CloudPods endpoints
	mux.HandleFunc("/api/v1/cloudpods/resources", h.GetCloudPodsResources)
	mux.HandleFunc("/api/v1/cloudpods/topology", h.GetCloudPodsTopology)
	mux.HandleFunc("/api/v1/cloudpods/metrics", h.GetCloudPodsMetrics)

	// Aviatrix endpoints
	mux.HandleFunc("/api/v1/aviatrix/gateways", h.GetAviatrixGateways)
	mux.HandleFunc("/api/v1/aviatrix/bgp-neighbors", h.GetAviatrixBGPNeighbors)
	mux.HandleFunc("/api/v1/aviatrix/routes", h.GetAviatrixRoutes)
	mux.HandleFunc("/api/v1/aviatrix/topology", h.GetAviatrixTopology)
	mux.HandleFunc("/api/v1/aviatrix/gateway-status", h.GetAviatrixGatewayStatus)

	// Analytics insertion endpoints
	mux.HandleFunc("/api/v1/analytics/packet-metrics", h.InsertPacketMetrics)
	mux.HandleFunc("/api/v1/analytics/route-metrics", h.InsertRouteMetrics)
	mux.HandleFunc("/api/v1/analytics/system-metrics", h.InsertSystemMetrics)
	mux.HandleFunc("/api/v1/analytics/traffic-flow", h.InsertTrafficFlow)

	// CORS middleware
	handler := corsMiddleware(mux, cfg.API.CORS)

	server := &http.Server{
		Addr:         fmt.Sprintf("%s:%d", cfg.API.Host, cfg.API.Port),
		Handler:      handler,
		ReadTimeout:  cfg.API.ReadTimeout,
		WriteTimeout: cfg.API.WriteTimeout,
		IdleTimeout:  cfg.API.IdleTimeout,
	}

	return &Server{
		config:   cfg,
		handlers: h,
		server:   server,
	}
}

// Start starts the server
func (s *Server) Start() error {
	return s.server.ListenAndServe()
}

// Shutdown gracefully shuts down the server
func (s *Server) Shutdown(ctx context.Context) error {
	return s.server.Shutdown(ctx)
}

// corsMiddleware adds CORS headers to responses
func corsMiddleware(next http.Handler, cors config.CORSConfig) http.Handler {
	return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		if cors.Enabled {
			// Set CORS headers
			for _, origin := range cors.AllowOrigins {
				if origin == "*" || origin == r.Header.Get("Origin") {
					w.Header().Set("Access-Control-Allow-Origin", origin)
					break
				}
			}

			w.Header().Set("Access-Control-Allow-Methods", joinStrings(cors.AllowMethods, ", "))
			w.Header().Set("Access-Control-Allow-Headers", joinStrings(cors.AllowHeaders, ", "))
			w.Header().Set("Access-Control-Allow-Credentials", "true")

			// Handle preflight requests
			if r.Method == "OPTIONS" {
				w.WriteHeader(http.StatusOK)
				return
			}
		}

		next.ServeHTTP(w, r)
	})
}

// joinStrings joins a slice of strings with a separator
func joinStrings(strs []string, sep string) string {
	if len(strs) == 0 {
		return ""
	}
	if len(strs) == 1 {
		return strs[0]
	}

	result := strs[0]
	for i := 1; i < len(strs); i++ {
		result += sep + strs[i]
	}
	return result
}
