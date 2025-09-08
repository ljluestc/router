package handlers

import (
	"net/http"
	"strconv"

	"router-sim/internal/cloudpods"
	"github.com/gin-gonic/gin"
	"github.com/sirupsen/logrus"
)

type CloudPodsHandler struct {
	client *cloudpods.Client
	logger *logrus.Logger
}

func NewCloudPodsHandler(client *cloudpods.Client, logger *logrus.Logger) *CloudPodsHandler {
	return &CloudPodsHandler{
		client: client,
		logger: logger,
	}
}

// VPC Handlers
func (h *CloudPodsHandler) ListVPCs(c *gin.Context) {
	vpcs, err := h.client.ListVPCs(c.Request.Context())
	if err != nil {
		h.logger.WithError(err).Error("Failed to list VPCs")
		c.JSON(http.StatusInternalServerError, gin.H{"error": "Failed to list VPCs"})
		return
	}

	c.JSON(http.StatusOK, gin.H{
		"status": "success",
		"data":   vpcs,
	})
}

func (h *CloudPodsHandler) CreateVPC(c *gin.Context) {
	var req cloudpods.CreateVPCRequest
	if err := c.ShouldBindJSON(&req); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
		return
	}

	vpc, err := h.client.CreateVPC(c.Request.Context(), req)
	if err != nil {
		h.logger.WithError(err).Error("Failed to create VPC")
		c.JSON(http.StatusInternalServerError, gin.H{"error": "Failed to create VPC"})
		return
	}

	c.JSON(http.StatusCreated, gin.H{
		"status": "success",
		"data":   vpc,
	})
}

func (h *CloudPodsHandler) GetVPC(c *gin.Context) {
	id := c.Param("id")
	if id == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "VPC ID is required"})
		return
	}

	vpc, err := h.client.GetVPC(c.Request.Context(), id)
	if err != nil {
		h.logger.WithError(err).WithField("vpc_id", id).Error("Failed to get VPC")
		c.JSON(http.StatusInternalServerError, gin.H{"error": "Failed to get VPC"})
		return
	}

	c.JSON(http.StatusOK, gin.H{
		"status": "success",
		"data":   vpc,
	})
}

func (h *CloudPodsHandler) UpdateVPC(c *gin.Context) {
	id := c.Param("id")
	if id == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "VPC ID is required"})
		return
	}

	var req cloudpods.UpdateVPCRequest
	if err := c.ShouldBindJSON(&req); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
		return
	}

	vpc, err := h.client.UpdateVPC(c.Request.Context(), id, req)
	if err != nil {
		h.logger.WithError(err).WithField("vpc_id", id).Error("Failed to update VPC")
		c.JSON(http.StatusInternalServerError, gin.H{"error": "Failed to update VPC"})
		return
	}

	c.JSON(http.StatusOK, gin.H{
		"status": "success",
		"data":   vpc,
	})
}

func (h *CloudPodsHandler) DeleteVPC(c *gin.Context) {
	id := c.Param("id")
	if id == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "VPC ID is required"})
		return
	}

	err := h.client.DeleteVPC(c.Request.Context(), id)
	if err != nil {
		h.logger.WithError(err).WithField("vpc_id", id).Error("Failed to delete VPC")
		c.JSON(http.StatusInternalServerError, gin.H{"error": "Failed to delete VPC"})
		return
	}

	c.JSON(http.StatusNoContent, nil)
}

func (h *CloudPodsHandler) GetVPCStats(c *gin.Context) {
	id := c.Param("id")
	if id == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "VPC ID is required"})
		return
	}

	stats, err := h.client.GetVPCStats(c.Request.Context(), id)
	if err != nil {
		h.logger.WithError(err).WithField("vpc_id", id).Error("Failed to get VPC stats")
		c.JSON(http.StatusInternalServerError, gin.H{"error": "Failed to get VPC stats"})
		return
	}

	c.JSON(http.StatusOK, gin.H{
		"status": "success",
		"data":   stats,
	})
}

// Subnet Handlers
func (h *CloudPodsHandler) ListSubnets(c *gin.Context) {
	vpcID := c.Param("vpc_id")
	if vpcID == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "VPC ID is required"})
		return
	}

	subnets, err := h.client.ListSubnets(c.Request.Context(), vpcID)
	if err != nil {
		h.logger.WithError(err).WithField("vpc_id", vpcID).Error("Failed to list subnets")
		c.JSON(http.StatusInternalServerError, gin.H{"error": "Failed to list subnets"})
		return
	}

	c.JSON(http.StatusOK, gin.H{
		"status": "success",
		"data":   subnets,
	})
}

func (h *CloudPodsHandler) CreateSubnet(c *gin.Context) {
	vpcID := c.Param("vpc_id")
	if vpcID == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "VPC ID is required"})
		return
	}

	var req cloudpods.CreateSubnetRequest
	if err := c.ShouldBindJSON(&req); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
		return
	}

	subnet, err := h.client.CreateSubnet(c.Request.Context(), vpcID, req)
	if err != nil {
		h.logger.WithError(err).WithField("vpc_id", vpcID).Error("Failed to create subnet")
		c.JSON(http.StatusInternalServerError, gin.H{"error": "Failed to create subnet"})
		return
	}

	c.JSON(http.StatusCreated, gin.H{
		"status": "success",
		"data":   subnet,
	})
}

func (h *CloudPodsHandler) GetSubnet(c *gin.Context) {
	vpcID := c.Param("vpc_id")
	subnetID := c.Param("id")
	if vpcID == "" || subnetID == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "VPC ID and Subnet ID are required"})
		return
	}

	subnet, err := h.client.GetSubnet(c.Request.Context(), vpcID, subnetID)
	if err != nil {
		h.logger.WithError(err).WithFields(logrus.Fields{
			"vpc_id":    vpcID,
			"subnet_id": subnetID,
		}).Error("Failed to get subnet")
		c.JSON(http.StatusInternalServerError, gin.H{"error": "Failed to get subnet"})
		return
	}

	c.JSON(http.StatusOK, gin.H{
		"status": "success",
		"data":   subnet,
	})
}

func (h *CloudPodsHandler) UpdateSubnet(c *gin.Context) {
	vpcID := c.Param("vpc_id")
	subnetID := c.Param("id")
	if vpcID == "" || subnetID == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "VPC ID and Subnet ID are required"})
		return
	}

	var req cloudpods.UpdateSubnetRequest
	if err := c.ShouldBindJSON(&req); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
		return
	}

	subnet, err := h.client.UpdateSubnet(c.Request.Context(), vpcID, subnetID, req)
	if err != nil {
		h.logger.WithError(err).WithFields(logrus.Fields{
			"vpc_id":    vpcID,
			"subnet_id": subnetID,
		}).Error("Failed to update subnet")
		c.JSON(http.StatusInternalServerError, gin.H{"error": "Failed to update subnet"})
		return
	}

	c.JSON(http.StatusOK, gin.H{
		"status": "success",
		"data":   subnet,
	})
}

func (h *CloudPodsHandler) DeleteSubnet(c *gin.Context) {
	vpcID := c.Param("vpc_id")
	subnetID := c.Param("id")
	if vpcID == "" || subnetID == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "VPC ID and Subnet ID are required"})
		return
	}

	err := h.client.DeleteSubnet(c.Request.Context(), vpcID, subnetID)
	if err != nil {
		h.logger.WithError(err).WithFields(logrus.Fields{
			"vpc_id":    vpcID,
			"subnet_id": subnetID,
		}).Error("Failed to delete subnet")
		c.JSON(http.StatusInternalServerError, gin.H{"error": "Failed to delete subnet"})
		return
	}

	c.JSON(http.StatusNoContent, nil)
}

// NAT Gateway Handlers
func (h *CloudPodsHandler) ListNATGateways(c *gin.Context) {
	vpcID := c.Param("vpc_id")
	if vpcID == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "VPC ID is required"})
		return
	}

	nats, err := h.client.ListNATGateways(c.Request.Context(), vpcID)
	if err != nil {
		h.logger.WithError(err).WithField("vpc_id", vpcID).Error("Failed to list NAT gateways")
		c.JSON(http.StatusInternalServerError, gin.H{"error": "Failed to list NAT gateways"})
		return
	}

	c.JSON(http.StatusOK, gin.H{
		"status": "success",
		"data":   nats,
	})
}

func (h *CloudPodsHandler) CreateNATGateway(c *gin.Context) {
	vpcID := c.Param("vpc_id")
	if vpcID == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "VPC ID is required"})
		return
	}

	var req cloudpods.CreateNATGatewayRequest
	if err := c.ShouldBindJSON(&req); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
		return
	}

	nat, err := h.client.CreateNATGateway(c.Request.Context(), vpcID, req)
	if err != nil {
		h.logger.WithError(err).WithField("vpc_id", vpcID).Error("Failed to create NAT gateway")
		c.JSON(http.StatusInternalServerError, gin.H{"error": "Failed to create NAT gateway"})
		return
	}

	c.JSON(http.StatusCreated, gin.H{
		"status": "success",
		"data":   nat,
	})
}

func (h *CloudPodsHandler) GetNATGateway(c *gin.Context) {
	vpcID := c.Param("vpc_id")
	natID := c.Param("id")
	if vpcID == "" || natID == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "VPC ID and NAT Gateway ID are required"})
		return
	}

	nat, err := h.client.GetNATGateway(c.Request.Context(), vpcID, natID)
	if err != nil {
		h.logger.WithError(err).WithFields(logrus.Fields{
			"vpc_id": vpcID,
			"nat_id": natID,
		}).Error("Failed to get NAT gateway")
		c.JSON(http.StatusInternalServerError, gin.H{"error": "Failed to get NAT gateway"})
		return
	}

	c.JSON(http.StatusOK, gin.H{
		"status": "success",
		"data":   nat,
	})
}

func (h *CloudPodsHandler) DeleteNATGateway(c *gin.Context) {
	vpcID := c.Param("vpc_id")
	natID := c.Param("id")
	if vpcID == "" || natID == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "VPC ID and NAT Gateway ID are required"})
		return
	}

	err := h.client.DeleteNATGateway(c.Request.Context(), vpcID, natID)
	if err != nil {
		h.logger.WithError(err).WithFields(logrus.Fields{
			"vpc_id": vpcID,
			"nat_id": natID,
		}).Error("Failed to delete NAT gateway")
		c.JSON(http.StatusInternalServerError, gin.H{"error": "Failed to delete NAT gateway"})
		return
	}

	c.JSON(http.StatusNoContent, nil)
}

// Load Balancer Handlers
func (h *CloudPodsHandler) ListLoadBalancers(c *gin.Context) {
	vpcID := c.Param("vpc_id")
	if vpcID == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "VPC ID is required"})
		return
	}

	lbs, err := h.client.ListLoadBalancers(c.Request.Context(), vpcID)
	if err != nil {
		h.logger.WithError(err).WithField("vpc_id", vpcID).Error("Failed to list load balancers")
		c.JSON(http.StatusInternalServerError, gin.H{"error": "Failed to list load balancers"})
		return
	}

	c.JSON(http.StatusOK, gin.H{
		"status": "success",
		"data":   lbs,
	})
}

func (h *CloudPodsHandler) CreateLoadBalancer(c *gin.Context) {
	vpcID := c.Param("vpc_id")
	if vpcID == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "VPC ID is required"})
		return
	}

	var req cloudpods.CreateLoadBalancerRequest
	if err := c.ShouldBindJSON(&req); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
		return
	}

	lb, err := h.client.CreateLoadBalancer(c.Request.Context(), vpcID, req)
	if err != nil {
		h.logger.WithError(err).WithField("vpc_id", vpcID).Error("Failed to create load balancer")
		c.JSON(http.StatusInternalServerError, gin.H{"error": "Failed to create load balancer"})
		return
	}

	c.JSON(http.StatusCreated, gin.H{
		"status": "success",
		"data":   lb,
	})
}

func (h *CloudPodsHandler) GetLoadBalancer(c *gin.Context) {
	vpcID := c.Param("vpc_id")
	lbID := c.Param("id")
	if vpcID == "" || lbID == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "VPC ID and Load Balancer ID are required"})
		return
	}

	lb, err := h.client.GetLoadBalancer(c.Request.Context(), vpcID, lbID)
	if err != nil {
		h.logger.WithError(err).WithFields(logrus.Fields{
			"vpc_id": vpcID,
			"lb_id":  lbID,
		}).Error("Failed to get load balancer")
		c.JSON(http.StatusInternalServerError, gin.H{"error": "Failed to get load balancer"})
		return
	}

	c.JSON(http.StatusOK, gin.H{
		"status": "success",
		"data":   lb,
	})
}

func (h *CloudPodsHandler) UpdateLoadBalancer(c *gin.Context) {
	vpcID := c.Param("vpc_id")
	lbID := c.Param("id")
	if vpcID == "" || lbID == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "VPC ID and Load Balancer ID are required"})
		return
	}

	var req cloudpods.UpdateLoadBalancerRequest
	if err := c.ShouldBindJSON(&req); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
		return
	}

	lb, err := h.client.UpdateLoadBalancer(c.Request.Context(), vpcID, lbID, req)
	if err != nil {
		h.logger.WithError(err).WithFields(logrus.Fields{
			"vpc_id": vpcID,
			"lb_id":  lbID,
		}).Error("Failed to update load balancer")
		c.JSON(http.StatusInternalServerError, gin.H{"error": "Failed to update load balancer"})
		return
	}

	c.JSON(http.StatusOK, gin.H{
		"status": "success",
		"data":   lb,
	})
}

func (h *CloudPodsHandler) DeleteLoadBalancer(c *gin.Context) {
	vpcID := c.Param("vpc_id")
	lbID := c.Param("id")
	if vpcID == "" || lbID == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "VPC ID and Load Balancer ID are required"})
		return
	}

	err := h.client.DeleteLoadBalancer(c.Request.Context(), vpcID, lbID)
	if err != nil {
		h.logger.WithError(err).WithFields(logrus.Fields{
			"vpc_id": vpcID,
			"lb_id":  lbID,
		}).Error("Failed to delete load balancer")
		c.JSON(http.StatusInternalServerError, gin.H{"error": "Failed to delete load balancer"})
		return
	}

	c.JSON(http.StatusNoContent, nil)
}

// Service Mesh Handlers
func (h *CloudPodsHandler) ListServiceMeshRoutes(c *gin.Context) {
	vpcID := c.Param("vpc_id")
	if vpcID == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "VPC ID is required"})
		return
	}

	routes, err := h.client.ListServiceMeshRoutes(c.Request.Context(), vpcID)
	if err != nil {
		h.logger.WithError(err).WithField("vpc_id", vpcID).Error("Failed to list service mesh routes")
		c.JSON(http.StatusInternalServerError, gin.H{"error": "Failed to list service mesh routes"})
		return
	}

	c.JSON(http.StatusOK, gin.H{
		"status": "success",
		"data":   routes,
	})
}

func (h *CloudPodsHandler) CreateServiceMeshRoute(c *gin.Context) {
	vpcID := c.Param("vpc_id")
	if vpcID == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "VPC ID is required"})
		return
	}

	var req cloudpods.CreateServiceMeshRouteRequest
	if err := c.ShouldBindJSON(&req); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
		return
	}

	route, err := h.client.CreateServiceMeshRoute(c.Request.Context(), vpcID, req)
	if err != nil {
		h.logger.WithError(err).WithField("vpc_id", vpcID).Error("Failed to create service mesh route")
		c.JSON(http.StatusInternalServerError, gin.H{"error": "Failed to create service mesh route"})
		return
	}

	c.JSON(http.StatusCreated, gin.H{
		"status": "success",
		"data":   route,
	})
}

func (h *CloudPodsHandler) GetServiceMeshRoute(c *gin.Context) {
	vpcID := c.Param("vpc_id")
	routeID := c.Param("id")
	if vpcID == "" || routeID == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "VPC ID and Route ID are required"})
		return
	}

	route, err := h.client.GetServiceMeshRoute(c.Request.Context(), vpcID, routeID)
	if err != nil {
		h.logger.WithError(err).WithFields(logrus.Fields{
			"vpc_id":   vpcID,
			"route_id": routeID,
		}).Error("Failed to get service mesh route")
		c.JSON(http.StatusInternalServerError, gin.H{"error": "Failed to get service mesh route"})
		return
	}

	c.JSON(http.StatusOK, gin.H{
		"status": "success",
		"data":   route,
	})
}

func (h *CloudPodsHandler) UpdateServiceMeshRoute(c *gin.Context) {
	vpcID := c.Param("vpc_id")
	routeID := c.Param("id")
	if vpcID == "" || routeID == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "VPC ID and Route ID are required"})
		return
	}

	var req cloudpods.UpdateServiceMeshRouteRequest
	if err := c.ShouldBindJSON(&req); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
		return
	}

	route, err := h.client.UpdateServiceMeshRoute(c.Request.Context(), vpcID, routeID, req)
	if err != nil {
		h.logger.WithError(err).WithFields(logrus.Fields{
			"vpc_id":   vpcID,
			"route_id": routeID,
		}).Error("Failed to update service mesh route")
		c.JSON(http.StatusInternalServerError, gin.H{"error": "Failed to update service mesh route"})
		return
	}

	c.JSON(http.StatusOK, gin.H{
		"status": "success",
		"data":   route,
	})
}

func (h *CloudPodsHandler) DeleteServiceMeshRoute(c *gin.Context) {
	vpcID := c.Param("vpc_id")
	routeID := c.Param("id")
	if vpcID == "" || routeID == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "VPC ID and Route ID are required"})
		return
	}

	err := h.client.DeleteServiceMeshRoute(c.Request.Context(), vpcID, routeID)
	if err != nil {
		h.logger.WithError(err).WithFields(logrus.Fields{
			"vpc_id":   vpcID,
			"route_id": routeID,
		}).Error("Failed to delete service mesh route")
		c.JSON(http.StatusInternalServerError, gin.H{"error": "Failed to delete service mesh route"})
		return
	}

	c.JSON(http.StatusNoContent, nil)
}
