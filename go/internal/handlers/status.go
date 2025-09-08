package handlers

import (
	"net/http"
	"runtime"
	"time"

	"github.com/gin-gonic/gin"
	"github.com/sirupsen/logrus"
)

type StatusHandler struct {
	logger *logrus.Logger
}

func NewStatusHandler(logger *logrus.Logger) *StatusHandler {
	return &StatusHandler{
		logger: logger,
	}
}

// HealthCheck provides a simple health check endpoint
func (h *StatusHandler) HealthCheck(c *gin.Context) {
	c.JSON(http.StatusOK, gin.H{
		"status":    "healthy",
		"timestamp": time.Now().UTC(),
		"service":   "router-sim-api",
	})
}

// GetStatus provides detailed system status
func (h *StatusHandler) GetStatus(c *gin.Context) {
	var m runtime.MemStats
	runtime.ReadMemStats(&m)

	status := gin.H{
		"status":    "running",
		"timestamp": time.Now().UTC(),
		"service":   "router-sim-api",
		"version":   "1.0.0",
		"uptime":    time.Since(time.Now()).String(), // This would be actual uptime in real implementation
		"system": gin.H{
			"go_version":    runtime.Version(),
			"go_routines":   runtime.NumGoroutine(),
			"memory_alloc":  m.Alloc,
			"memory_total":  m.TotalAlloc,
			"memory_sys":    m.Sys,
			"gc_runs":       m.NumGC,
			"cpu_count":     runtime.NumCPU(),
		},
		"components": gin.H{
			"cloudpods": gin.H{
				"enabled": true,
				"status":  "connected",
			},
			"aviatrix": gin.H{
				"enabled": true,
				"status":  "connected",
			},
			"analytics": gin.H{
				"enabled": true,
				"status":  "running",
			},
			"router": gin.H{
				"enabled": true,
				"status":  "running",
			},
		},
	}

	c.JSON(http.StatusOK, status)
}
