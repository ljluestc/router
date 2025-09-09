import React, { useState, useEffect, useRef } from 'react';
import {
  Box,
  Card,
  CardContent,
  Typography,
  Button,
  IconButton,
  Tooltip,
  Chip,
  Dialog,
  DialogTitle,
  DialogContent,
  DialogActions,
  TextField,
  Select,
  MenuItem,
  FormControl,
  InputLabel,
  Grid,
  Paper,
  List,
  ListItem,
  ListItemText,
  ListItemIcon,
  Divider,
  Switch,
  FormControlLabel,
  Slider,
  Alert,
  LinearProgress,
} from '@mui/material';
import {
  Refresh as RefreshIcon,
  Add as AddIcon,
  Edit as EditIcon,
  Delete as DeleteIcon,
  Visibility as VisibilityIcon,
  VisibilityOff as VisibilityOffIcon,
  Settings as SettingsIcon,
  Fullscreen as FullscreenIcon,
  ZoomIn as ZoomInIcon,
  ZoomOut as ZoomOutIcon,
  CenterFocusStrong as CenterFocusIcon,
  Cloud as CloudIcon,
  Router as RouterIcon,
  Security as SecurityIcon,
  Speed as SpeedIcon,
  Storage as StorageIcon,
  VpnKey as VpnKeyIcon,
  NetworkCheck as NetworkCheckIcon,
  Timeline as TimelineIcon,
  Assessment as AssessmentIcon,
} from '@mui/icons-material';

// Mock data for network topology
const mockTopologyData = {
  nodes: [
    {
      id: 'transit-gw-us-west-1',
      name: 'Transit GW US-West-1',
      type: 'transit',
      cloud: 'AWS',
      region: 'us-west-1',
      status: 'active',
      position: { x: 200, y: 100 },
      properties: {
        cpu: 45,
        memory: 62,
        latency: 12,
        connections: 8,
      },
    },
    {
      id: 'transit-gw-us-east-1',
      name: 'Transit GW US-East-1',
      type: 'transit',
      cloud: 'AWS',
      region: 'us-east-1',
      status: 'active',
      position: { x: 400, y: 100 },
      properties: {
        cpu: 38,
        memory: 58,
        latency: 15,
        connections: 6,
      },
    },
    {
      id: 'spoke-gw-eu-west-1',
      name: 'Spoke GW EU-West-1',
      type: 'spoke',
      cloud: 'Azure',
      region: 'eu-west-1',
      status: 'active',
      position: { x: 300, y: 200 },
      properties: {
        cpu: 52,
        memory: 71,
        latency: 18,
        connections: 3,
      },
    },
    {
      id: 'spoke-gw-asia-pacific',
      name: 'Spoke GW Asia-Pacific',
      type: 'spoke',
      cloud: 'GCP',
      region: 'asia-pacific',
      status: 'maintenance',
      position: { x: 500, y: 200 },
      properties: {
        cpu: 0,
        memory: 0,
        latency: 0,
        connections: 0,
      },
    },
    {
      id: 'vpn-gateway-1',
      name: 'VPN Gateway 1',
      type: 'vpn',
      cloud: 'AWS',
      region: 'us-west-1',
      status: 'active',
      position: { x: 100, y: 300 },
      properties: {
        users: 25,
        sessions: 18,
        bandwidth: 100,
      },
    },
    {
      id: 'load-balancer-1',
      name: 'Load Balancer 1',
      type: 'loadbalancer',
      cloud: 'Azure',
      region: 'us-east-1',
      status: 'active',
      position: { x: 600, y: 300 },
      properties: {
        backends: 5,
        health: 100,
        requests: 1250,
      },
    },
  ],
  links: [
    {
      id: 'link-1',
      source: 'transit-gw-us-west-1',
      target: 'transit-gw-us-east-1',
      type: 'transit-to-transit',
      status: 'active',
      bandwidth: 10000,
      latency: 25,
    },
    {
      id: 'link-2',
      source: 'transit-gw-us-west-1',
      target: 'spoke-gw-eu-west-1',
      type: 'transit-to-spoke',
      status: 'active',
      bandwidth: 5000,
      latency: 45,
    },
    {
      id: 'link-3',
      source: 'transit-gw-us-east-1',
      target: 'spoke-gw-asia-pacific',
      type: 'transit-to-spoke',
      status: 'maintenance',
      bandwidth: 0,
      latency: 0,
    },
    {
      id: 'link-4',
      source: 'vpn-gateway-1',
      target: 'transit-gw-us-west-1',
      type: 'vpn-to-transit',
      status: 'active',
      bandwidth: 1000,
      latency: 15,
    },
    {
      id: 'link-5',
      source: 'load-balancer-1',
      target: 'transit-gw-us-east-1',
      type: 'lb-to-transit',
      status: 'active',
      bandwidth: 2000,
      latency: 8,
    },
  ],
};

const NetworkTopology: React.FC = () => {
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const [topologyData, setTopologyData] = useState(mockTopologyData);
  const [selectedNode, setSelectedNode] = useState(null);
  const [selectedLink, setSelectedLink] = useState(null);
  const [showNodeDialog, setShowNodeDialog] = useState(false);
  const [showLinkDialog, setShowLinkDialog] = useState(false);
  const [showSettingsDialog, setShowSettingsDialog] = useState(false);
  const [loading, setLoading] = useState(false);
  const [viewSettings, setViewSettings] = useState({
    showLabels: true,
    showMetrics: true,
    showClouds: true,
    showStatus: true,
    zoom: 1,
    panX: 0,
    panY: 0,
  });

  useEffect(() => {
    drawTopology();
  }, [topologyData, viewSettings]);

  const drawTopology = () => {
    const canvas = canvasRef.current;
    if (!canvas) return;

    const ctx = canvas.getContext('2d');
    if (!ctx) return;

    // Clear canvas
    ctx.clearRect(0, 0, canvas.width, canvas.height);

    // Apply zoom and pan
    ctx.save();
    ctx.scale(viewSettings.zoom, viewSettings.zoom);
    ctx.translate(viewSettings.panX, viewSettings.panY);

    // Draw links first (so they appear behind nodes)
    topologyData.links.forEach((link) => {
      const sourceNode = topologyData.nodes.find(n => n.id === link.source);
      const targetNode = topologyData.nodes.find(n => n.id === link.target);
      
      if (sourceNode && targetNode) {
        drawLink(ctx, sourceNode, targetNode, link);
      }
    });

    // Draw nodes
    topologyData.nodes.forEach((node) => {
      drawNode(ctx, node);
    });

    ctx.restore();
  };

  const drawNode = (ctx: CanvasRenderingContext2D, node: any) => {
    const { x, y } = node.position;
    const radius = 30;
    
    // Node background
    ctx.fillStyle = getNodeColor(node.type, node.status);
    ctx.beginPath();
    ctx.arc(x, y, radius, 0, 2 * Math.PI);
    ctx.fill();
    
    // Node border
    ctx.strokeStyle = getNodeBorderColor(node.status);
    ctx.lineWidth = 2;
    ctx.stroke();
    
    // Node icon
    ctx.fillStyle = 'white';
    ctx.font = '16px Arial';
    ctx.textAlign = 'center';
    ctx.textBaseline = 'middle';
    ctx.fillText(getNodeIcon(node.type), x, y);
    
    // Node label
    if (viewSettings.showLabels) {
      ctx.fillStyle = '#333';
      ctx.font = '12px Arial';
      ctx.textAlign = 'center';
      ctx.textBaseline = 'top';
      ctx.fillText(node.name, x, y + radius + 5);
    }
    
    // Status indicator
    if (viewSettings.showStatus) {
      const statusRadius = 8;
      ctx.fillStyle = getStatusColor(node.status);
      ctx.beginPath();
      ctx.arc(x + radius - 8, y - radius + 8, statusRadius, 0, 2 * Math.PI);
      ctx.fill();
    }
  };

  const drawLink = (ctx: CanvasRenderingContext2D, sourceNode: any, targetNode: any, link: any) => {
    const { x: x1, y: y1 } = sourceNode.position;
    const { x: x2, y: y2 } = targetNode.position;
    
    // Link line
    ctx.strokeStyle = getLinkColor(link.status);
    ctx.lineWidth = getLinkWidth(link.bandwidth);
    ctx.setLineDash(link.status === 'maintenance' ? [5, 5] : []);
    ctx.beginPath();
    ctx.moveTo(x1, y1);
    ctx.lineTo(x2, y2);
    ctx.stroke();
    ctx.setLineDash([]);
    
    // Link label (bandwidth)
    if (viewSettings.showMetrics && link.bandwidth > 0) {
      const midX = (x1 + x2) / 2;
      const midY = (y1 + y2) / 2;
      
      ctx.fillStyle = '#333';
      ctx.font = '10px Arial';
      ctx.textAlign = 'center';
      ctx.textBaseline = 'middle';
      ctx.fillText(`${link.bandwidth}Mbps`, midX, midY);
    }
  };

  const getNodeColor = (type: string, status: string) => {
    const colors = {
      transit: '#1976d2',
      spoke: '#dc004e',
      vpn: '#2e7d32',
      loadbalancer: '#ed6c02',
    };
    return colors[type] || '#666';
  };

  const getNodeBorderColor = (status: string) => {
    switch (status) {
      case 'active': return '#4caf50';
      case 'maintenance': return '#ff9800';
      case 'error': return '#f44336';
      default: return '#666';
    }
  };

  const getNodeIcon = (type: string) => {
    const icons = {
      transit: 'T',
      spoke: 'S',
      vpn: 'V',
      loadbalancer: 'L',
    };
    return icons[type] || '?';
  };

  const getStatusColor = (status: string) => {
    switch (status) {
      case 'active': return '#4caf50';
      case 'maintenance': return '#ff9800';
      case 'error': return '#f44336';
      default: return '#666';
    }
  };

  const getLinkColor = (status: string) => {
    switch (status) {
      case 'active': return '#4caf50';
      case 'maintenance': return '#ff9800';
      case 'error': return '#f44336';
      default: return '#666';
    }
  };

  const getLinkWidth = (bandwidth: number) => {
    if (bandwidth === 0) return 1;
    return Math.max(1, Math.min(8, bandwidth / 1000));
  };

  const handleCanvasClick = (event: React.MouseEvent<HTMLCanvasElement>) => {
    const canvas = canvasRef.current;
    if (!canvas) return;

    const rect = canvas.getBoundingClientRect();
    const x = (event.clientX - rect.left) / viewSettings.zoom - viewSettings.panX;
    const y = (event.clientY - rect.top) / viewSettings.zoom - viewSettings.panY;

    // Check if click is on a node
    const clickedNode = topologyData.nodes.find(node => {
      const dx = x - node.position.x;
      const dy = y - node.position.y;
      return Math.sqrt(dx * dx + dy * dy) <= 30;
    });

    if (clickedNode) {
      setSelectedNode(clickedNode);
      setShowNodeDialog(true);
    } else {
      setSelectedNode(null);
    }
  };

  const handleRefresh = () => {
    setLoading(true);
    setTimeout(() => {
      setTopologyData(mockTopologyData);
      setLoading(false);
    }, 1000);
  };

  const handleZoomIn = () => {
    setViewSettings(prev => ({
      ...prev,
      zoom: Math.min(prev.zoom * 1.2, 3),
    }));
  };

  const handleZoomOut = () => {
    setViewSettings(prev => ({
      ...prev,
      zoom: Math.max(prev.zoom / 1.2, 0.5),
    }));
  };

  const handleCenterView = () => {
    setViewSettings(prev => ({
      ...prev,
      panX: 0,
      panY: 0,
      zoom: 1,
    }));
  };

  return (
    <Box>
      {/* Header */}
      <Box sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', mb: 3 }}>
        <Typography variant="h4" component="h1">
          Network Topology
        </Typography>
        <Box>
          <Tooltip title="Settings">
            <IconButton onClick={() => setShowSettingsDialog(true)}>
              <SettingsIcon />
            </IconButton>
          </Tooltip>
          <Tooltip title="Center View">
            <IconButton onClick={handleCenterView}>
              <CenterFocusIcon />
            </IconButton>
          </Tooltip>
          <Tooltip title="Zoom In">
            <IconButton onClick={handleZoomIn}>
              <ZoomInIcon />
            </IconButton>
          </Tooltip>
          <Tooltip title="Zoom Out">
            <IconButton onClick={handleZoomOut}>
              <ZoomOutIcon />
            </IconButton>
          </Tooltip>
          <Tooltip title="Refresh">
            <IconButton onClick={handleRefresh} disabled={loading}>
              <RefreshIcon />
            </IconButton>
          </Tooltip>
        </Box>
      </Box>

      {loading && <LinearProgress sx={{ mb: 2 }} />}

      <Grid container spacing={3}>
        {/* Topology Canvas */}
        <Grid item xs={12} md={9}>
          <Card>
            <CardContent>
              <Box sx={{ position: 'relative' }}>
                <canvas
                  ref={canvasRef}
                  width={800}
                  height={600}
                  style={{
                    border: '1px solid #ddd',
                    borderRadius: '4px',
                    cursor: 'pointer',
                    width: '100%',
                    height: '600px',
                  }}
                  onClick={handleCanvasClick}
                />
                
                {/* Legend */}
                <Box sx={{ position: 'absolute', top: 10, right: 10, bgcolor: 'white', p: 2, borderRadius: 1, boxShadow: 1 }}>
                  <Typography variant="subtitle2" gutterBottom>
                    Legend
                  </Typography>
                  <Box sx={{ display: 'flex', flexDirection: 'column', gap: 1 }}>
                    <Box sx={{ display: 'flex', alignItems: 'center', gap: 1 }}>
                      <Box sx={{ width: 16, height: 16, bgcolor: '#1976d2', borderRadius: '50%' }} />
                      <Typography variant="caption">Transit Gateway</Typography>
                    </Box>
                    <Box sx={{ display: 'flex', alignItems: 'center', gap: 1 }}>
                      <Box sx={{ width: 16, height: 16, bgcolor: '#dc004e', borderRadius: '50%' }} />
                      <Typography variant="caption">Spoke Gateway</Typography>
                    </Box>
                    <Box sx={{ display: 'flex', alignItems: 'center', gap: 1 }}>
                      <Box sx={{ width: 16, height: 16, bgcolor: '#2e7d32', borderRadius: '50%' }} />
                      <Typography variant="caption">VPN Gateway</Typography>
                    </Box>
                    <Box sx={{ display: 'flex', alignItems: 'center', gap: 1 }}>
                      <Box sx={{ width: 16, height: 16, bgcolor: '#ed6c02', borderRadius: '50%' }} />
                      <Typography variant="caption">Load Balancer</Typography>
                    </Box>
                  </Box>
                </Box>
              </Box>
            </CardContent>
          </Card>
        </Grid>

        {/* Node Details Panel */}
        <Grid item xs={12} md={3}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>
                Node Details
              </Typography>
              {selectedNode ? (
                <Box>
                  <Typography variant="subtitle1" gutterBottom>
                    {selectedNode.name}
                  </Typography>
                  <Chip
                    label={selectedNode.type}
                    color="primary"
                    size="small"
                    sx={{ mb: 1 }}
                  />
                  <Chip
                    label={selectedNode.status}
                    color={getStatusColor(selectedNode.status) as any}
                    size="small"
                    sx={{ mb: 2, ml: 1 }}
                  />
                  
                  <Divider sx={{ my: 2 }} />
                  
                  <Typography variant="subtitle2" gutterBottom>
                    Properties
                  </Typography>
                  <List dense>
                    <ListItem>
                      <ListItemText
                        primary="Cloud"
                        secondary={selectedNode.cloud}
                      />
                    </ListItem>
                    <ListItem>
                      <ListItemText
                        primary="Region"
                        secondary={selectedNode.region}
                      />
                    </ListItem>
                    {selectedNode.properties.cpu !== undefined && (
                      <ListItem>
                        <ListItemText
                          primary="CPU Usage"
                          secondary={`${selectedNode.properties.cpu}%`}
                        />
                      </ListItem>
                    )}
                    {selectedNode.properties.memory !== undefined && (
                      <ListItem>
                        <ListItemText
                          primary="Memory Usage"
                          secondary={`${selectedNode.properties.memory}%`}
                        />
                      </ListItem>
                    )}
                    {selectedNode.properties.latency !== undefined && (
                      <ListItem>
                        <ListItemText
                          primary="Latency"
                          secondary={`${selectedNode.properties.latency}ms`}
                        />
                      </ListItem>
                    )}
                  </List>
                </Box>
              ) : (
                <Typography variant="body2" color="text.secondary">
                  Click on a node to view details
                </Typography>
              )}
            </CardContent>
          </Card>

          {/* Statistics */}
          <Card sx={{ mt: 2 }}>
            <CardContent>
              <Typography variant="h6" gutterBottom>
                Statistics
              </Typography>
              <List dense>
                <ListItem>
                  <ListItemText
                    primary="Total Nodes"
                    secondary={topologyData.nodes.length}
                  />
                </ListItem>
                <ListItem>
                  <ListItemText
                    primary="Active Nodes"
                    secondary={topologyData.nodes.filter(n => n.status === 'active').length}
                  />
                </ListItem>
                <ListItem>
                  <ListItemText
                    primary="Total Links"
                    secondary={topologyData.links.length}
                  />
                </ListItem>
                <ListItem>
                  <ListItemText
                    primary="Active Links"
                    secondary={topologyData.links.filter(l => l.status === 'active').length}
                  />
                </ListItem>
              </List>
            </CardContent>
          </Card>
        </Grid>
      </Grid>

      {/* Node Details Dialog */}
      <Dialog
        open={showNodeDialog}
        onClose={() => setShowNodeDialog(false)}
        maxWidth="sm"
        fullWidth
      >
        <DialogTitle>Node Details</DialogTitle>
        <DialogContent>
          {selectedNode && (
            <Box>
              <Typography variant="h6" gutterBottom>
                {selectedNode.name}
              </Typography>
              <Grid container spacing={2}>
                <Grid item xs={6}>
                  <Typography variant="body2" color="text.secondary">
                    Type
                  </Typography>
                  <Typography variant="body1">
                    {selectedNode.type}
                  </Typography>
                </Grid>
                <Grid item xs={6}>
                  <Typography variant="body2" color="text.secondary">
                    Status
                  </Typography>
                  <Chip
                    label={selectedNode.status}
                    color={getStatusColor(selectedNode.status) as any}
                    size="small"
                  />
                </Grid>
                <Grid item xs={6}>
                  <Typography variant="body2" color="text.secondary">
                    Cloud
                  </Typography>
                  <Typography variant="body1">
                    {selectedNode.cloud}
                  </Typography>
                </Grid>
                <Grid item xs={6}>
                  <Typography variant="body2" color="text.secondary">
                    Region
                  </Typography>
                  <Typography variant="body1">
                    {selectedNode.region}
                  </Typography>
                </Grid>
              </Grid>
            </Box>
          )}
        </DialogContent>
        <DialogActions>
          <Button onClick={() => setShowNodeDialog(false)}>Close</Button>
        </DialogActions>
      </Dialog>

      {/* Settings Dialog */}
      <Dialog
        open={showSettingsDialog}
        onClose={() => setShowSettingsDialog(false)}
        maxWidth="sm"
        fullWidth
      >
        <DialogTitle>View Settings</DialogTitle>
        <DialogContent>
          <Box sx={{ pt: 2 }}>
            <FormControlLabel
              control={
                <Switch
                  checked={viewSettings.showLabels}
                  onChange={(e) => setViewSettings(prev => ({
                    ...prev,
                    showLabels: e.target.checked,
                  }))}
                />
              }
              label="Show Labels"
            />
            <FormControlLabel
              control={
                <Switch
                  checked={viewSettings.showMetrics}
                  onChange={(e) => setViewSettings(prev => ({
                    ...prev,
                    showMetrics: e.target.checked,
                  }))}
                />
              }
              label="Show Metrics"
            />
            <FormControlLabel
              control={
                <Switch
                  checked={viewSettings.showClouds}
                  onChange={(e) => setViewSettings(prev => ({
                    ...prev,
                    showClouds: e.target.checked,
                  }))}
                />
              }
              label="Show Cloud Info"
            />
            <FormControlLabel
              control={
                <Switch
                  checked={viewSettings.showStatus}
                  onChange={(e) => setViewSettings(prev => ({
                    ...prev,
                    showStatus: e.target.checked,
                  }))}
                />
              }
              label="Show Status Indicators"
            />
            
            <Box sx={{ mt: 3 }}>
              <Typography gutterBottom>
                Zoom Level: {Math.round(viewSettings.zoom * 100)}%
              </Typography>
              <Slider
                value={viewSettings.zoom}
                min={0.5}
                max={3}
                step={0.1}
                onChange={(_, value) => setViewSettings(prev => ({
                  ...prev,
                  zoom: value as number,
                }))}
              />
            </Box>
          </Box>
        </DialogContent>
        <DialogActions>
          <Button onClick={() => setShowSettingsDialog(false)}>Close</Button>
        </DialogActions>
      </Dialog>
    </Box>
  );
};

export default NetworkTopology;
