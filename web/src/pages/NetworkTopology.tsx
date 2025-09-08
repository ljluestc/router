import React, { useState, useEffect, useRef } from 'react';
import {
  Box,
  Typography,
  Card,
  CardContent,
  Grid,
  Button,
  IconButton,
  Tooltip,
  Paper,
  List,
  ListItem,
  ListItemText,
  ListItemIcon,
  Chip,
  Dialog,
  DialogTitle,
  DialogContent,
  DialogActions,
  TextField,
  FormControl,
  InputLabel,
  Select,
  MenuItem,
  Switch,
  FormControlLabel,
  Slider,
  Alert,
  Snackbar,
} from '@mui/material';
import {
  Add as AddIcon,
  Edit as EditIcon,
  Delete as DeleteIcon,
  Refresh as RefreshIcon,
  Visibility as ViewIcon,
  VisibilityOff as ViewOffIcon,
  Settings as SettingsIcon,
  NetworkCheck as NetworkCheckIcon,
  Router as RouterIcon,
  Cloud as CloudIcon,
  Security as SecurityIcon,
  Speed as SpeedIcon,
} from '@mui/icons-material';
import { useQuery, useMutation, useQueryClient } from '@tanstack/react-query';

interface Node {
  id: string;
  type: 'router' | 'switch' | 'host' | 'cloud' | 'gateway';
  name: string;
  x: number;
  y: number;
  status: 'active' | 'inactive' | 'error';
  properties: any;
}

interface Link {
  id: string;
  source: string;
  target: string;
  type: 'ethernet' | 'wireless' | 'vpn' | 'tunnel';
  status: 'up' | 'down' | 'degraded';
  properties: any;
}

interface NetworkTopology {
  nodes: Node[];
  links: Link[];
  metadata: {
    name: string;
    description: string;
    created_at: string;
    updated_at: string;
  };
}

const NetworkTopology: React.FC = () => {
  const [selectedNode, setSelectedNode] = useState<Node | null>(null);
  const [selectedLink, setSelectedLink] = useState<Link | null>(null);
  const [nodeDialogOpen, setNodeDialogOpen] = useState(false);
  const [linkDialogOpen, setLinkDialogOpen] = useState(false);
  const [editingNode, setEditingNode] = useState<Node | null>(null);
  const [editingLink, setEditingLink] = useState<Link | null>(null);
  const [snackbarOpen, setSnackbarOpen] = useState(false);
  const [snackbarMessage, setSnackbarMessage] = useState('');
  const [snackbarSeverity, setSnackbarSeverity] = useState<'success' | 'error'>('success');
  const [viewMode, setViewMode] = useState<'2d' | '3d'>('2d');
  const [showLabels, setShowLabels] = useState(true);
  const [showMetrics, setShowMetrics] = useState(true);

  const canvasRef = useRef<HTMLCanvasElement>(null);
  const queryClient = useQueryClient();

  const { data: topology, isLoading } = useQuery({
    queryKey: ['network-topology'],
    queryFn: async () => {
      const response = await fetch('/api/v1/network/topology');
      return response.json();
    },
  });

  const { data: interfaces } = useQuery({
    queryKey: ['network-interfaces'],
    queryFn: async () => {
      const response = await fetch('/api/v1/network/interfaces');
      return response.json();
    },
  });

  const { data: routes } = useQuery({
    queryKey: ['network-routes'],
    queryFn: async () => {
      const response = await fetch('/api/v1/network/routes');
      return response.json();
    },
  });

  const createNodeMutation = useMutation({
    mutationFn: async (nodeData: Partial<Node>) => {
      const response = await fetch('/api/v1/network/nodes', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify(nodeData),
      });
      return response.json();
    },
    onSuccess: () => {
      queryClient.invalidateQueries({ queryKey: ['network-topology'] });
      setSnackbarMessage('Node created successfully');
      setSnackbarOpen(true);
      setNodeDialogOpen(false);
    },
    onError: () => {
      setSnackbarMessage('Failed to create node');
      setSnackbarSeverity('error');
      setSnackbarOpen(true);
    },
  });

  const updateNodeMutation = useMutation({
    mutationFn: async ({ id, ...nodeData }: Partial<Node> & { id: string }) => {
      const response = await fetch(`/api/v1/network/nodes/${id}`, {
        method: 'PUT',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify(nodeData),
      });
      return response.json();
    },
    onSuccess: () => {
      queryClient.invalidateQueries({ queryKey: ['network-topology'] });
      setSnackbarMessage('Node updated successfully');
      setSnackbarOpen(true);
      setNodeDialogOpen(false);
    },
    onError: () => {
      setSnackbarMessage('Failed to update node');
      setSnackbarSeverity('error');
      setSnackbarOpen(true);
    },
  });

  const deleteNodeMutation = useMutation({
    mutationFn: async (nodeId: string) => {
      const response = await fetch(`/api/v1/network/nodes/${nodeId}`, {
        method: 'DELETE',
      });
      return response.json();
    },
    onSuccess: () => {
      queryClient.invalidateQueries({ queryKey: ['network-topology'] });
      setSnackbarMessage('Node deleted successfully');
      setSnackbarOpen(true);
    },
    onError: () => {
      setSnackbarMessage('Failed to delete node');
      setSnackbarSeverity('error');
      setSnackbarOpen(true);
    },
  });

  const createLinkMutation = useMutation({
    mutationFn: async (linkData: Partial<Link>) => {
      const response = await fetch('/api/v1/network/links', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify(linkData),
      });
      return response.json();
    },
    onSuccess: () => {
      queryClient.invalidateQueries({ queryKey: ['network-topology'] });
      setSnackbarMessage('Link created successfully');
      setSnackbarOpen(true);
      setLinkDialogOpen(false);
    },
    onError: () => {
      setSnackbarMessage('Failed to create link');
      setSnackbarSeverity('error');
      setSnackbarOpen(true);
    },
  });

  const updateLinkMutation = useMutation({
    mutationFn: async ({ id, ...linkData }: Partial<Link> & { id: string }) => {
      const response = await fetch(`/api/v1/network/links/${id}`, {
        method: 'PUT',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify(linkData),
      });
      return response.json();
    },
    onSuccess: () => {
      queryClient.invalidateQueries({ queryKey: ['network-topology'] });
      setSnackbarMessage('Link updated successfully');
      setSnackbarOpen(true);
      setLinkDialogOpen(false);
    },
    onError: () => {
      setSnackbarMessage('Failed to update link');
      setSnackbarSeverity('error');
      setSnackbarOpen(true);
    },
  });

  const deleteLinkMutation = useMutation({
    mutationFn: async (linkId: string) => {
      const response = await fetch(`/api/v1/network/links/${linkId}`, {
        method: 'DELETE',
      });
      return response.json();
    },
    onSuccess: () => {
      queryClient.invalidateQueries({ queryKey: ['network-topology'] });
      setSnackbarMessage('Link deleted successfully');
      setSnackbarOpen(true);
    },
    onError: () => {
      setSnackbarMessage('Failed to delete link');
      setSnackbarSeverity('error');
      setSnackbarOpen(true);
    },
  });

  const handleCreateNode = () => {
    setEditingNode(null);
    setNodeDialogOpen(true);
  };

  const handleEditNode = (node: Node) => {
    setEditingNode(node);
    setNodeDialogOpen(true);
  };

  const handleDeleteNode = (nodeId: string) => {
    if (window.confirm('Are you sure you want to delete this node?')) {
      deleteNodeMutation.mutate(nodeId);
    }
  };

  const handleCreateLink = () => {
    setEditingLink(null);
    setLinkDialogOpen(true);
  };

  const handleEditLink = (link: Link) => {
    setEditingLink(link);
    setLinkDialogOpen(true);
  };

  const handleDeleteLink = (linkId: string) => {
    if (window.confirm('Are you sure you want to delete this link?')) {
      deleteLinkMutation.mutate(linkId);
    }
  };

  const handleSaveNode = (nodeData: Partial<Node>) => {
    if (editingNode) {
      updateNodeMutation.mutate({ ...nodeData, id: editingNode.id });
    } else {
      createNodeMutation.mutate(nodeData);
    }
  };

  const handleSaveLink = (linkData: Partial<Link>) => {
    if (editingLink) {
      updateLinkMutation.mutate({ ...linkData, id: editingLink.id });
    } else {
      createLinkMutation.mutate(linkData);
    }
  };

  const getNodeIcon = (type: string) => {
    switch (type) {
      case 'router':
        return <RouterIcon />;
      case 'switch':
        return <NetworkCheckIcon />;
      case 'host':
        return <NetworkCheckIcon />;
      case 'cloud':
        return <CloudIcon />;
      case 'gateway':
        return <SecurityIcon />;
      default:
        return <NetworkCheckIcon />;
    }
  };

  const getNodeColor = (status: string) => {
    switch (status) {
      case 'active':
        return '#4caf50';
      case 'inactive':
        return '#9e9e9e';
      case 'error':
        return '#f44336';
      default:
        return '#9e9e9e';
    }
  };

  const getLinkColor = (status: string) => {
    switch (status) {
      case 'up':
        return '#4caf50';
      case 'down':
        return '#f44336';
      case 'degraded':
        return '#ff9800';
      default:
        return '#9e9e9e';
    }
  };

  const renderTopology = () => {
    const canvas = canvasRef.current;
    if (!canvas || !topology?.data) return;

    const ctx = canvas.getContext('2d');
    if (!ctx) return;

    // Clear canvas
    ctx.clearRect(0, 0, canvas.width, canvas.height);

    // Set canvas size
    canvas.width = canvas.offsetWidth;
    canvas.height = canvas.offsetHeight;

    // Draw links
    topology.data.links.forEach((link: Link) => {
      const sourceNode = topology.data.nodes.find((n: Node) => n.id === link.source);
      const targetNode = topology.data.nodes.find((n: Node) => n.id === link.target);
      
      if (sourceNode && targetNode) {
        ctx.strokeStyle = getLinkColor(link.status);
        ctx.lineWidth = 2;
        ctx.beginPath();
        ctx.moveTo(sourceNode.x, sourceNode.y);
        ctx.lineTo(targetNode.x, targetNode.y);
        ctx.stroke();
      }
    });

    // Draw nodes
    topology.data.nodes.forEach((node: Node) => {
      const isSelected = selectedNode?.id === node.id;
      
      // Draw node circle
      ctx.fillStyle = getNodeColor(node.status);
      ctx.strokeStyle = isSelected ? '#1976d2' : '#000';
      ctx.lineWidth = isSelected ? 3 : 1;
      ctx.beginPath();
      ctx.arc(node.x, node.y, 20, 0, 2 * Math.PI);
      ctx.fill();
      ctx.stroke();

      // Draw node label
      if (showLabels) {
        ctx.fillStyle = '#000';
        ctx.font = '12px Arial';
        ctx.textAlign = 'center';
        ctx.fillText(node.name, node.x, node.y + 35);
      }
    });
  };

  useEffect(() => {
    renderTopology();
  }, [topology, selectedNode, showLabels]);

  if (isLoading) {
    return (
      <Box display="flex" justifyContent="center" alignItems="center" minHeight="400px">
        <Typography>Loading network topology...</Typography>
      </Box>
    );
  }

  return (
    <Box>
      <Box display="flex" justifyContent="space-between" alignItems="center" mb={3}>
        <Typography variant="h4" component="h1">
          Network Topology
        </Typography>
        <Box display="flex" gap={2}>
          <Button
            variant="outlined"
            startIcon={<RefreshIcon />}
            onClick={() => queryClient.invalidateQueries({ queryKey: ['network-topology'] })}
          >
            Refresh
          </Button>
          <Button
            variant="contained"
            startIcon={<AddIcon />}
            onClick={handleCreateNode}
          >
            Add Node
          </Button>
        </Box>
      </Box>

      <Grid container spacing={3}>
        {/* Topology Canvas */}
        <Grid item xs={12} md={8}>
          <Card>
            <CardContent>
              <Box display="flex" justifyContent="space-between" alignItems="center" mb={2}>
                <Typography variant="h6">
                  Network Topology View
                </Typography>
                <Box display="flex" gap={1}>
                  <FormControlLabel
                    control={
                      <Switch
                        checked={showLabels}
                        onChange={(e) => setShowLabels(e.target.checked)}
                      />
                    }
                    label="Show Labels"
                  />
                  <FormControlLabel
                    control={
                      <Switch
                        checked={showMetrics}
                        onChange={(e) => setShowMetrics(e.target.checked)}
                      />
                    }
                    label="Show Metrics"
                  />
                </Box>
              </Box>
              <Paper
                sx={{
                  height: '500px',
                  position: 'relative',
                  overflow: 'hidden',
                  border: '1px solid #e0e0e0',
                }}
              >
                <canvas
                  ref={canvasRef}
                  style={{
                    width: '100%',
                    height: '100%',
                    cursor: 'pointer',
                  }}
                  onClick={(e) => {
                    // Handle node selection
                    const rect = canvasRef.current?.getBoundingClientRect();
                    if (rect) {
                      const x = e.clientX - rect.left;
                      const y = e.clientY - rect.top;
                      
                      // Find clicked node
                      const clickedNode = topology?.data?.nodes.find((node: Node) => {
                        const distance = Math.sqrt((node.x - x) ** 2 + (node.y - y) ** 2);
                        return distance <= 20;
                      });
                      
                      if (clickedNode) {
                        setSelectedNode(clickedNode);
                      }
                    }
                  }}
                />
              </Paper>
            </CardContent>
          </Card>
        </Grid>

        {/* Node Details */}
        <Grid item xs={12} md={4}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>
                Node Details
              </Typography>
              {selectedNode ? (
                <Box>
                  <Box display="flex" alignItems="center" gap={1} mb={2}>
                    {getNodeIcon(selectedNode.type)}
                    <Typography variant="h6">{selectedNode.name}</Typography>
                    <Chip
                      label={selectedNode.status}
                      color={selectedNode.status === 'active' ? 'success' : 'default'}
                      size="small"
                    />
                  </Box>
                  <Typography variant="body2" color="text.secondary" paragraph>
                    Type: {selectedNode.type}
                  </Typography>
                  <Typography variant="body2" color="text.secondary" paragraph>
                    Position: ({selectedNode.x}, {selectedNode.y})
                  </Typography>
                  {selectedNode.properties && (
                    <Box>
                      <Typography variant="subtitle2" gutterBottom>
                        Properties:
                      </Typography>
                      <pre style={{ fontSize: '12px', margin: 0 }}>
                        {JSON.stringify(selectedNode.properties, null, 2)}
                      </pre>
                    </Box>
                  )}
                  <Box display="flex" gap={1} mt={2}>
                    <Button
                      size="small"
                      startIcon={<EditIcon />}
                      onClick={() => handleEditNode(selectedNode)}
                    >
                      Edit
                    </Button>
                    <Button
                      size="small"
                      color="error"
                      startIcon={<DeleteIcon />}
                      onClick={() => handleDeleteNode(selectedNode.id)}
                    >
                      Delete
                    </Button>
                  </Box>
                </Box>
              ) : (
                <Typography variant="body2" color="text.secondary">
                  Click on a node to view details
                </Typography>
              )}
            </CardContent>
          </Card>
        </Grid>

        {/* Network Statistics */}
        <Grid item xs={12} md={6}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>
                Network Statistics
              </Typography>
              <Grid container spacing={2}>
                <Grid item xs={6}>
                  <Typography variant="body2" color="text.secondary">
                    Total Nodes
                  </Typography>
                  <Typography variant="h6">
                    {topology?.data?.nodes?.length || 0}
                  </Typography>
                </Grid>
                <Grid item xs={6}>
                  <Typography variant="body2" color="text.secondary">
                    Total Links
                  </Typography>
                  <Typography variant="h6">
                    {topology?.data?.links?.length || 0}
                  </Typography>
                </Grid>
                <Grid item xs={6}>
                  <Typography variant="body2" color="text.secondary">
                    Active Nodes
                  </Typography>
                  <Typography variant="h6">
                    {topology?.data?.nodes?.filter((n: Node) => n.status === 'active').length || 0}
                  </Typography>
                </Grid>
                <Grid item xs={6}>
                  <Typography variant="body2" color="text.secondary">
                    Active Links
                  </Typography>
                  <Typography variant="h6">
                    {topology?.data?.links?.filter((l: Link) => l.status === 'up').length || 0}
                  </Typography>
                </Grid>
              </Grid>
            </CardContent>
          </Card>
        </Grid>

        {/* Interface Statistics */}
        <Grid item xs={12} md={6}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>
                Interface Statistics
              </Typography>
              {interfaces?.data && (
                <List dense>
                  {interfaces.data.slice(0, 5).map((iface: any, index: number) => (
                    <ListItem key={index}>
                      <ListItemIcon>
                        <NetworkCheckIcon />
                      </ListItemIcon>
                      <ListItemText
                        primary={iface.name}
                        secondary={`${iface.status} - ${iface.type}`}
                      />
                    </ListItem>
                  ))}
                </List>
              )}
            </CardContent>
          </Card>
        </Grid>
      </Grid>

      {/* Node Dialog */}
      <Dialog
        open={nodeDialogOpen}
        onClose={() => setNodeDialogOpen(false)}
        maxWidth="sm"
        fullWidth
      >
        <DialogTitle>
          {editingNode ? 'Edit Node' : 'Create Node'}
        </DialogTitle>
        <DialogContent>
          <NodeForm
            node={editingNode}
            onSave={handleSaveNode}
            onCancel={() => setNodeDialogOpen(false)}
          />
        </DialogContent>
      </Dialog>

      {/* Link Dialog */}
      <Dialog
        open={linkDialogOpen}
        onClose={() => setLinkDialogOpen(false)}
        maxWidth="sm"
        fullWidth
      >
        <DialogTitle>
          {editingLink ? 'Edit Link' : 'Create Link'}
        </DialogTitle>
        <DialogContent>
          <LinkForm
            link={editingLink}
            onSave={handleSaveLink}
            onCancel={() => setLinkDialogOpen(false)}
            nodes={topology?.data?.nodes || []}
          />
        </DialogContent>
      </Dialog>

      {/* Snackbar for notifications */}
      <Snackbar
        open={snackbarOpen}
        autoHideDuration={6000}
        onClose={() => setSnackbarOpen(false)}
      >
        <Alert
          onClose={() => setSnackbarOpen(false)}
          severity={snackbarSeverity}
          sx={{ width: '100%' }}
        >
          {snackbarMessage}
        </Alert>
      </Snackbar>
    </Box>
  );
};

interface NodeFormProps {
  node: Node | null;
  onSave: (data: Partial<Node>) => void;
  onCancel: () => void;
}

const NodeForm: React.FC<NodeFormProps> = ({ node, onSave, onCancel }) => {
  const [formData, setFormData] = useState({
    name: node?.name || '',
    type: node?.type || 'host',
    x: node?.x || 100,
    y: node?.y || 100,
    status: node?.status || 'active',
    properties: node?.properties || {},
  });

  const handleSubmit = (e: React.FormEvent) => {
    e.preventDefault();
    onSave(formData);
  };

  return (
    <Box component="form" onSubmit={handleSubmit}>
      <Grid container spacing={2} sx={{ mt: 1 }}>
        <Grid item xs={12}>
          <TextField
            fullWidth
            label="Node Name"
            value={formData.name}
            onChange={(e) => setFormData({ ...formData, name: e.target.value })}
            required
          />
        </Grid>
        <Grid item xs={12}>
          <FormControl fullWidth>
            <InputLabel>Node Type</InputLabel>
            <Select
              value={formData.type}
              onChange={(e) => setFormData({ ...formData, type: e.target.value as any })}
            >
              <MenuItem value="host">Host</MenuItem>
              <MenuItem value="router">Router</MenuItem>
              <MenuItem value="switch">Switch</MenuItem>
              <MenuItem value="cloud">Cloud</MenuItem>
              <MenuItem value="gateway">Gateway</MenuItem>
            </Select>
          </FormControl>
        </Grid>
        <Grid item xs={6}>
          <TextField
            fullWidth
            label="X Position"
            type="number"
            value={formData.x}
            onChange={(e) => setFormData({ ...formData, x: parseInt(e.target.value) })}
          />
        </Grid>
        <Grid item xs={6}>
          <TextField
            fullWidth
            label="Y Position"
            type="number"
            value={formData.y}
            onChange={(e) => setFormData({ ...formData, y: parseInt(e.target.value) })}
          />
        </Grid>
        <Grid item xs={12}>
          <FormControl fullWidth>
            <InputLabel>Status</InputLabel>
            <Select
              value={formData.status}
              onChange={(e) => setFormData({ ...formData, status: e.target.value as any })}
            >
              <MenuItem value="active">Active</MenuItem>
              <MenuItem value="inactive">Inactive</MenuItem>
              <MenuItem value="error">Error</MenuItem>
            </Select>
          </FormControl>
        </Grid>
      </Grid>
      <DialogActions>
        <Button onClick={onCancel}>Cancel</Button>
        <Button type="submit" variant="contained">
          {node ? 'Update' : 'Create'}
        </Button>
      </DialogActions>
    </Box>
  );
};

interface LinkFormProps {
  link: Link | null;
  onSave: (data: Partial<Link>) => void;
  onCancel: () => void;
  nodes: Node[];
}

const LinkForm: React.FC<LinkFormProps> = ({ link, onSave, onCancel, nodes }) => {
  const [formData, setFormData] = useState({
    source: link?.source || '',
    target: link?.target || '',
    type: link?.type || 'ethernet',
    status: link?.status || 'up',
    properties: link?.properties || {},
  });

  const handleSubmit = (e: React.FormEvent) => {
    e.preventDefault();
    onSave(formData);
  };

  return (
    <Box component="form" onSubmit={handleSubmit}>
      <Grid container spacing={2} sx={{ mt: 1 }}>
        <Grid item xs={12}>
          <FormControl fullWidth>
            <InputLabel>Source Node</InputLabel>
            <Select
              value={formData.source}
              onChange={(e) => setFormData({ ...formData, source: e.target.value })}
            >
              {nodes.map((node) => (
                <MenuItem key={node.id} value={node.id}>
                  {node.name}
                </MenuItem>
              ))}
            </Select>
          </FormControl>
        </Grid>
        <Grid item xs={12}>
          <FormControl fullWidth>
            <InputLabel>Target Node</InputLabel>
            <Select
              value={formData.target}
              onChange={(e) => setFormData({ ...formData, target: e.target.value })}
            >
              {nodes.map((node) => (
                <MenuItem key={node.id} value={node.id}>
                  {node.name}
                </MenuItem>
              ))}
            </Select>
          </FormControl>
        </Grid>
        <Grid item xs={12}>
          <FormControl fullWidth>
            <InputLabel>Link Type</InputLabel>
            <Select
              value={formData.type}
              onChange={(e) => setFormData({ ...formData, type: e.target.value as any })}
            >
              <MenuItem value="ethernet">Ethernet</MenuItem>
              <MenuItem value="wireless">Wireless</MenuItem>
              <MenuItem value="vpn">VPN</MenuItem>
              <MenuItem value="tunnel">Tunnel</MenuItem>
            </Select>
          </FormControl>
        </Grid>
        <Grid item xs={12}>
          <FormControl fullWidth>
            <InputLabel>Status</InputLabel>
            <Select
              value={formData.status}
              onChange={(e) => setFormData({ ...formData, status: e.target.value as any })}
            >
              <MenuItem value="up">Up</MenuItem>
              <MenuItem value="down">Down</MenuItem>
              <MenuItem value="degraded">Degraded</MenuItem>
            </Select>
          </FormControl>
        </Grid>
      </Grid>
      <DialogActions>
        <Button onClick={onCancel}>Cancel</Button>
        <Button type="submit" variant="contained">
          {link ? 'Update' : 'Create'}
        </Button>
      </DialogActions>
    </Box>
  );
};

export default NetworkTopology;
