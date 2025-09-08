package handlers

import (
	"net/http"

	"router-sim/internal/analytics"
	"github.com/gin-gonic/gin"
	"github.com/sirupsen/logrus"
)

type AnalyticsHandler struct {
	engine *analytics.Engine
	logger *logrus.Logger
}

func NewAnalyticsHandler(engine *analytics.Engine, logger *logrus.Logger) *AnalyticsHandler {
	return &AnalyticsHandler{
		engine: engine,
		logger: logger,
	}
}

func (h *AnalyticsHandler) GetTrafficStats(c *gin.Context) {
	stats, err := h.engine.GetTrafficStats(c.Request.Context())
	if err != nil {
		h.logger.WithError(err).Error("Failed to get traffic stats")
		c.JSON(http.StatusInternalServerError, gin.H{"error": "Failed to get traffic stats"})
		return
	}

	c.JSON(http.StatusOK, gin.H{
		"status": "success",
		"data":   stats,
	})
}

func (h *AnalyticsHandler) GetPerformanceMetrics(c *gin.Context) {
	metrics, err := h.engine.GetPerformanceMetrics(c.Request.Context())
	if err != nil {
		h.logger.WithError(err).Error("Failed to get performance metrics")
		c.JSON(http.StatusInternalServerError, gin.H{"error": "Failed to get performance metrics"})
		return
	}

	c.JSON(http.StatusOK, gin.H{
		"status": "success",
		"data":   metrics,
	})
}

func (h *AnalyticsHandler) GetRoutingStats(c *gin.Context) {
	stats, err := h.engine.GetRoutingStats(c.Request.Context())
	if err != nil {
		h.logger.WithError(err).Error("Failed to get routing stats")
		c.JSON(http.StatusInternalServerError, gin.H{"error": "Failed to get routing stats"})
		return
	}

	c.JSON(http.StatusOK, gin.H{
		"status": "success",
		"data":   stats,
	})
}

func (h *AnalyticsHandler) GetCloudPodsStats(c *gin.Context) {
	stats, err := h.engine.GetCloudPodsStats(c.Request.Context())
	if err != nil {
		h.logger.WithError(err).Error("Failed to get CloudPods stats")
		c.JSON(http.StatusInternalServerError, gin.H{"error": "Failed to get CloudPods stats"})
		return
	}

	c.JSON(http.StatusOK, gin.H{
		"status": "success",
		"data":   stats,
	})
}

func (h *AnalyticsHandler) GetAviatrixStats(c *gin.Context) {
	stats, err := h.engine.GetAviatrixStats(c.Request.Context())
	if err != nil {
		h.logger.WithError(err).Error("Failed to get Aviatrix stats")
		c.JSON(http.StatusInternalServerError, gin.H{"error": "Failed to get Aviatrix stats"})
		return
	}

	c.JSON(http.StatusOK, gin.H{
		"status": "success",
		"data":   stats,
	})
}
