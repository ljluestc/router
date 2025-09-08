package handlers

import (
	"net/http"

	"router-sim/internal/aviatrix"
	"github.com/gin-gonic/gin"
	"github.com/sirupsen/logrus"
)

type AviatrixHandler struct {
	client *aviatrix.Client
	logger *logrus.Logger
}

func NewAviatrixHandler(client *aviatrix.Client, logger *logrus.Logger) *AviatrixHandler {
	return &AviatrixHandler{
		client: client,
		logger: logger,
	}
}

// Transit Gateway Handlers
func (h *AviatrixHandler) ListTransitGateways(c *gin.Context) {
	gateways, err := h.client.ListTransitGateways(c.Request.Context())
	if err != nil {
		h.logger.WithError(err).Error("Failed to list transit gateways")
		c.JSON(http.StatusInternalServerError, gin.H{"error": "Failed to list transit gateways"})
		return
	}

	c.JSON(http.StatusOK, gin.H{
		"status": "success",
		"data":   gateways,
	})
}

func (h *AviatrixHandler) CreateTransitGateway(c *gin.Context) {
	var req aviatrix.CreateTransitGatewayRequest
	if err := c.ShouldBindJSON(&req); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
		return
	}

	gateway, err := h.client.CreateTransitGateway(c.Request.Context(), req)
	if err != nil {
		h.logger.WithError(err).Error("Failed to create transit gateway")
		c.JSON(http.StatusInternalServerError, gin.H{"error": "Failed to create transit gateway"})
		return
	}

	c.JSON(http.StatusCreated, gin.H{
		"status": "success",
		"data":   gateway,
	})
}

func (h *AviatrixHandler) GetTransitGateway(c *gin.Context) {
	name := c.Param("name")
	if name == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "Gateway name is required"})
		return
	}

	gateway, err := h.client.GetTransitGateway(c.Request.Context(), name)
	if err != nil {
		h.logger.WithError(err).WithField("gateway_name", name).Error("Failed to get transit gateway")
		c.JSON(http.StatusInternalServerError, gin.H{"error": "Failed to get transit gateway"})
		return
	}

	c.JSON(http.StatusOK, gin.H{
		"status": "success",
		"data":   gateway,
	})
}

func (h *AviatrixHandler) UpdateTransitGateway(c *gin.Context) {
	name := c.Param("name")
	if name == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "Gateway name is required"})
		return
	}

	var req aviatrix.UpdateTransitGatewayRequest
	if err := c.ShouldBindJSON(&req); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
		return
	}

	gateway, err := h.client.UpdateTransitGateway(c.Request.Context(), name, req)
	if err != nil {
		h.logger.WithError(err).WithField("gateway_name", name).Error("Failed to update transit gateway")
		c.JSON(http.StatusInternalServerError, gin.H{"error": "Failed to update transit gateway"})
		return
	}

	c.JSON(http.StatusOK, gin.H{
		"status": "success",
		"data":   gateway,
	})
}

func (h *AviatrixHandler) DeleteTransitGateway(c *gin.Context) {
	name := c.Param("name")
	if name == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "Gateway name is required"})
		return
	}

	err := h.client.DeleteTransitGateway(c.Request.Context(), name)
	if err != nil {
		h.logger.WithError(err).WithField("gateway_name", name).Error("Failed to delete transit gateway")
		c.JSON(http.StatusInternalServerError, gin.H{"error": "Failed to delete transit gateway"})
		return
	}

	c.JSON(http.StatusNoContent, nil)
}

// Spoke Gateway Handlers
func (h *AviatrixHandler) ListSpokeGateways(c *gin.Context) {
	gateways, err := h.client.ListSpokeGateways(c.Request.Context())
	if err != nil {
		h.logger.WithError(err).Error("Failed to list spoke gateways")
		c.JSON(http.StatusInternalServerError, gin.H{"error": "Failed to list spoke gateways"})
		return
	}

	c.JSON(http.StatusOK, gin.H{
		"status": "success",
		"data":   gateways,
	})
}

func (h *AviatrixHandler) CreateSpokeGateway(c *gin.Context) {
	var req aviatrix.CreateSpokeGatewayRequest
	if err := c.ShouldBindJSON(&req); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
		return
	}

	gateway, err := h.client.CreateSpokeGateway(c.Request.Context(), req)
	if err != nil {
		h.logger.WithError(err).Error("Failed to create spoke gateway")
		c.JSON(http.StatusInternalServerError, gin.H{"error": "Failed to create spoke gateway"})
		return
	}

	c.JSON(http.StatusCreated, gin.H{
		"status": "success",
		"data":   gateway,
	})
}

func (h *AviatrixHandler) GetSpokeGateway(c *gin.Context) {
	name := c.Param("name")
	if name == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "Gateway name is required"})
		return
	}

	gateway, err := h.client.GetSpokeGateway(c.Request.Context(), name)
	if err != nil {
		h.logger.WithError(err).WithField("gateway_name", name).Error("Failed to get spoke gateway")
		c.JSON(http.StatusInternalServerError, gin.H{"error": "Failed to get spoke gateway"})
		return
	}

	c.JSON(http.StatusOK, gin.H{
		"status": "success",
		"data":   gateway,
	})
}

func (h *AviatrixHandler) UpdateSpokeGateway(c *gin.Context) {
	name := c.Param("name")
	if name == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "Gateway name is required"})
		return
	}

	var req aviatrix.UpdateSpokeGatewayRequest
	if err := c.ShouldBindJSON(&req); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
		return
	}

	gateway, err := h.client.UpdateSpokeGateway(c.Request.Context(), name, req)
	if err != nil {
		h.logger.WithError(err).WithField("gateway_name", name).Error("Failed to update spoke gateway")
		c.JSON(http.StatusInternalServerError, gin.H{"error": "Failed to update spoke gateway"})
		return
	}

	c.JSON(http.StatusOK, gin.H{
		"status": "success",
		"data":   gateway,
	})
}

func (h *AviatrixHandler) DeleteSpokeGateway(c *gin.Context) {
	name := c.Param("name")
	if name == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "Gateway name is required"})
		return
	}

	err := h.client.DeleteSpokeGateway(c.Request.Context(), name)
	if err != nil {
		h.logger.WithError(err).WithField("gateway_name", name).Error("Failed to delete spoke gateway")
		c.JSON(http.StatusInternalServerError, gin.H{"error": "Failed to delete spoke gateway"})
		return
	}

	c.JSON(http.StatusNoContent, nil)
}

// VPC Connection Handlers
func (h *AviatrixHandler) ListVPCConnections(c *gin.Context) {
	connections, err := h.client.ListVPCConnections(c.Request.Context())
	if err != nil {
		h.logger.WithError(err).Error("Failed to list VPC connections")
		c.JSON(http.StatusInternalServerError, gin.H{"error": "Failed to list VPC connections"})
		return
	}

	c.JSON(http.StatusOK, gin.H{
		"status": "success",
		"data":   connections,
	})
}

func (h *AviatrixHandler) CreateVPCConnection(c *gin.Context) {
	var req aviatrix.CreateVPCConnectionRequest
	if err := c.ShouldBindJSON(&req); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
		return
	}

	connection, err := h.client.CreateVPCConnection(c.Request.Context(), req)
	if err != nil {
		h.logger.WithError(err).Error("Failed to create VPC connection")
		c.JSON(http.StatusInternalServerError, gin.H{"error": "Failed to create VPC connection"})
		return
	}

	c.JSON(http.StatusCreated, gin.H{
		"status": "success",
		"data":   connection,
	})
}

func (h *AviatrixHandler) GetVPCConnection(c *gin.Context) {
	name := c.Param("name")
	if name == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "Connection name is required"})
		return
	}

	connection, err := h.client.GetVPCConnection(c.Request.Context(), name)
	if err != nil {
		h.logger.WithError(err).WithField("connection_name", name).Error("Failed to get VPC connection")
		c.JSON(http.StatusInternalServerError, gin.H{"error": "Failed to get VPC connection"})
		return
	}

	c.JSON(http.StatusOK, gin.H{
		"status": "success",
		"data":   connection,
	})
}

func (h *AviatrixHandler) DeleteVPCConnection(c *gin.Context) {
	name := c.Param("name")
	if name == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "Connection name is required"})
		return
	}

	err := h.client.DeleteVPCConnection(c.Request.Context(), name)
	if err != nil {
		h.logger.WithError(err).WithField("connection_name", name).Error("Failed to delete VPC connection")
		c.JSON(http.StatusInternalServerError, gin.H{"error": "Failed to delete VPC connection"})
		return
	}

	c.JSON(http.StatusNoContent, nil)
}

// Site-to-Cloud Connection Handlers
func (h *AviatrixHandler) ListSite2CloudConnections(c *gin.Context) {
	connections, err := h.client.ListSite2CloudConnections(c.Request.Context())
	if err != nil {
		h.logger.WithError(err).Error("Failed to list site-to-cloud connections")
		c.JSON(http.StatusInternalServerError, gin.H{"error": "Failed to list site-to-cloud connections"})
		return
	}

	c.JSON(http.StatusOK, gin.H{
		"status": "success",
		"data":   connections,
	})
}

func (h *AviatrixHandler) CreateSite2CloudConnection(c *gin.Context) {
	var req aviatrix.CreateSite2CloudConnectionRequest
	if err := c.ShouldBindJSON(&req); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
		return
	}

	connection, err := h.client.CreateSite2CloudConnection(c.Request.Context(), req)
	if err != nil {
		h.logger.WithError(err).Error("Failed to create site-to-cloud connection")
		c.JSON(http.StatusInternalServerError, gin.H{"error": "Failed to create site-to-cloud connection"})
		return
	}

	c.JSON(http.StatusCreated, gin.H{
		"status": "success",
		"data":   connection,
	})
}

func (h *AviatrixHandler) GetSite2CloudConnection(c *gin.Context) {
	name := c.Param("name")
	if name == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "Connection name is required"})
		return
	}

	connection, err := h.client.GetSite2CloudConnection(c.Request.Context(), name)
	if err != nil {
		h.logger.WithError(err).WithField("connection_name", name).Error("Failed to get site-to-cloud connection")
		c.JSON(http.StatusInternalServerError, gin.H{"error": "Failed to get site-to-cloud connection"})
		return
	}

	c.JSON(http.StatusOK, gin.H{
		"status": "success",
		"data":   connection,
	})
}

func (h *AviatrixHandler) DeleteSite2CloudConnection(c *gin.Context) {
	name := c.Param("name")
	if name == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "Connection name is required"})
		return
	}

	err := h.client.DeleteSite2CloudConnection(c.Request.Context(), name)
	if err != nil {
		h.logger.WithError(err).WithField("connection_name", name).Error("Failed to delete site-to-cloud connection")
		c.JSON(http.StatusInternalServerError, gin.H{"error": "Failed to delete site-to-cloud connection"})
		return
	}

	c.JSON(http.StatusNoContent, nil)
}
