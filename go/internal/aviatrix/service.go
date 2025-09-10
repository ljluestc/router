package aviatrix

import (
	"context"
	"net/http"
	"time"

	"github.com/gin-gonic/gin"
	"go.uber.org/zap"
)

// Service represents the Aviatrix service
type Service struct {
	client *Client
	logger *zap.Logger
}

// NewService creates a new Aviatrix service
func NewService(config *config.AviatrixConfig, logger *zap.Logger) (*Service, error) {
	client := NewClient(config)
	
	return &Service{
		client: client,
		logger: logger,
	}, nil
}

// ListGateways handles GET /api/v1/aviatrix/gateways
func (s *Service) ListGateways(c *gin.Context) {
	ctx, cancel := context.WithTimeout(context.Background(), 30*time.Second)
	defer cancel()

	gateways, err := s.client.GetGateways(ctx)
	if err != nil {
		s.logger.Error("Failed to get Aviatrix gateways", zap.Error(err))
		c.JSON(http.StatusInternalServerError, gin.H{
			"error": "Failed to retrieve gateways",
		})
		return
	}

	c.JSON(http.StatusOK, gin.H{
		"gateways": gateways,
		"count":    len(gateways),
	})
}

// CreateGateway handles POST /api/v1/aviatrix/gateways
func (s *Service) CreateGateway(c *gin.Context) {
	var request struct {
		Name   string `json:"name" binding:"required"`
		Cloud  string `json:"cloud" binding:"required"`
		Region string `json:"region" binding:"required"`
		Type   string `json:"type" binding:"required"`
		ASN    int    `json:"asn"`
	}

	if err := c.ShouldBindJSON(&request); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{
			"error": "Invalid request body",
		})
		return
	}

	// Mock gateway creation
	gateway := map[string]interface{}{
		"id":          "gw-" + time.Now().Format("20060102150405"),
		"name":        request.Name,
		"cloud":       request.Cloud,
		"region":      request.Region,
		"type":        request.Type,
		"status":      "creating",
		"asn":         request.ASN,
		"created_at":  time.Now().Format(time.RFC3339),
	}

	s.logger.Info("Created Aviatrix gateway", 
		zap.String("name", request.Name),
		zap.String("cloud", request.Cloud))

	c.JSON(http.StatusCreated, gin.H{
		"gateway": gateway,
	})
}

// GetGateway handles GET /api/v1/aviatrix/gateways/:id
func (s *Service) GetGateway(c *gin.Context) {
	gatewayID := c.Param("id")
	
	// Mock gateway retrieval
	gateway := map[string]interface{}{
		"id":          gatewayID,
		"name":        "aviatrix-gateway-" + gatewayID,
		"cloud":       "AWS",
		"region":      "us-west-1",
		"type":        "transit",
		"status":      "up",
		"asn":         65001,
		"created_at":  "2024-01-01T00:00:00Z",
	}

	c.JSON(http.StatusOK, gin.H{
		"gateway": gateway,
	})
}

// DeleteGateway handles DELETE /api/v1/aviatrix/gateways/:id
func (s *Service) DeleteGateway(c *gin.Context) {
	gatewayID := c.Param("id")
	
	s.logger.Info("Deleted Aviatrix gateway", zap.String("id", gatewayID))
	
	c.JSON(http.StatusOK, gin.H{
		"message": "Gateway deleted successfully",
		"id":      gatewayID,
	})
}

// ListTransitGateways handles GET /api/v1/aviatrix/transit-gateways
func (s *Service) ListTransitGateways(c *gin.Context) {
	ctx, cancel := context.WithTimeout(context.Background(), 30*time.Second)
	defer cancel()

	gateways, err := s.client.GetTransitGateways(ctx)
	if err != nil {
		s.logger.Error("Failed to get Aviatrix transit gateways", zap.Error(err))
		c.JSON(http.StatusInternalServerError, gin.H{
			"error": "Failed to retrieve transit gateways",
		})
		return
	}

	c.JSON(http.StatusOK, gin.H{
		"transit_gateways": gateways,
		"count":           len(gateways),
	})
}

// CreateTransitGateway handles POST /api/v1/aviatrix/transit-gateways
func (s *Service) CreateTransitGateway(c *gin.Context) {
	var request struct {
		Name   string `json:"name" binding:"required"`
		Cloud  string `json:"cloud" binding:"required"`
		Region string `json:"region" binding:"required"`
		ASN    int    `json:"asn" binding:"required"`
	}

	if err := c.ShouldBindJSON(&request); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{
			"error": "Invalid request body",
		})
		return
	}

	// Mock transit gateway creation
	gateway := map[string]interface{}{
		"id":          "tgw-" + time.Now().Format("20060102150405"),
		"name":        request.Name,
		"cloud":       request.Cloud,
		"region":      request.Region,
		"type":        "transit",
		"status":      "creating",
		"asn":         request.ASN,
		"created_at":  time.Now().Format(time.RFC3339),
	}

	s.logger.Info("Created Aviatrix transit gateway", 
		zap.String("name", request.Name),
		zap.String("cloud", request.Cloud))

	c.JSON(http.StatusCreated, gin.H{
		"transit_gateway": gateway,
	})
}

// ListSpokeGateways handles GET /api/v1/aviatrix/spoke-gateways
func (s *Service) ListSpokeGateways(c *gin.Context) {
	ctx, cancel := context.WithTimeout(context.Background(), 30*time.Second)
	defer cancel()

	gateways, err := s.client.GetSpokeGateways(ctx)
	if err != nil {
		s.logger.Error("Failed to get Aviatrix spoke gateways", zap.Error(err))
		c.JSON(http.StatusInternalServerError, gin.H{
			"error": "Failed to retrieve spoke gateways",
		})
		return
	}

	c.JSON(http.StatusOK, gin.H{
		"spoke_gateways": gateways,
		"count":         len(gateways),
	})
}

// CreateSpokeGateway handles POST /api/v1/aviatrix/spoke-gateways
func (s *Service) CreateSpokeGateway(c *gin.Context) {
	var request struct {
		Name   string `json:"name" binding:"required"`
		Cloud  string `json:"cloud" binding:"required"`
		Region string `json:"region" binding:"required"`
		VPCID  string `json:"vpc_id" binding:"required"`
	}

	if err := c.ShouldBindJSON(&request); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{
			"error": "Invalid request body",
		})
		return
	}

	// Mock spoke gateway creation
	gateway := map[string]interface{}{
		"id":          "sgw-" + time.Now().Format("20060102150405"),
		"name":        request.Name,
		"cloud":       request.Cloud,
		"region":      request.Region,
		"type":        "spoke",
		"status":      "creating",
		"vpc_id":      request.VPCID,
		"created_at":  time.Now().Format(time.RFC3339),
	}

	s.logger.Info("Created Aviatrix spoke gateway", 
		zap.String("name", request.Name),
		zap.String("cloud", request.Cloud))

	c.JSON(http.StatusCreated, gin.H{
		"spoke_gateway": gateway,
	})
}
