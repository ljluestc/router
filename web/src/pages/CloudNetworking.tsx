import React, { useState, useEffect } from 'react';
import {
  Box,
  Grid,
  Card,
  CardContent,
  Typography,
  Button,
  Chip,
  Table,
  TableBody,
  TableCell,
  TableContainer,
  TableHead,
  TableRow,
  Paper,
  Tabs,
  Tab,
  Alert,
  CircularProgress,
  IconButton,
  Tooltip,
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
  Accordion,
  AccordionSummary,
  AccordionDetails,
} from '@mui/material';
import {
  Cloud,
  Router,
  Security,
  Speed,
  NetworkCheck,
  Settings,
  Refresh,
  Add,
  Delete,
  Edit,
  Visibility,
  ExpandMore,
  CloudQueue,
  CloudSync,
  CloudDone,
  CloudOff,
  TrendingUp,
  TrendingDown,
  Warning,
  CheckCircle,
  Error,
} from '@mui/icons-material';
import { LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip as RechartsTooltip, ResponsiveContainer, PieChart, Pie, Cell, BarChart, Bar } from 'recharts';

interface CloudProvider {
  id: string;
  name: string;
  type: 'cloudpods' | 'aviatrix';
  status: 'connected' | 'disconnected' | 'error';
  region: string;
  resources: number;
  lastSync: string;
}

interface NetworkResource {
  id: string;
  name: string;
  type: 'gateway' | 'vpc' | 'subnet' | 'security-group' | 'load-balancer';
  provider: string;
  status: 'active' | 'inactive' | 'error';
  region: string;
  properties: Record<string, any>;
}

interface NetworkTopology {
  nodes: Array<{
    id: string;
    type: string;
    label: string;
    x: number;
    y: number;
    status: string;
    provider: string;
  }>;
  edges: Array<{
    source: string;
    target: string;
    type: string;
    status: string;
  }>;
}

const CloudNetworking: React.FC = () => {
  const [activeTab, setActiveTab] = useState(0);
  const [cloudProviders, setCloudProviders] = useState<CloudProvider[]>([]);
  const [networkResources, setNetworkResources] = useState<NetworkResource[]>([]);
  const [topology, setTopology] = useState<NetworkTopology>({ nodes: [], edges: [] });
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);
  const [createDialogOpen, setCreateDialogOpen] = useState(false);
  const [selectedResource, setSelectedResource] = useState<NetworkResource | null>(null);

  // Mock data for demonstration
  useEffect(() => {
    const loadData = async () => {
      setLoading(true);
      try {
        // Simulate API calls
        await new Promise(resolve => setTimeout(resolve, 1000));
        
        setCloudProviders([
          {
            id: 'cloudpods-1',
            name: 'CloudPods Main',
            type: 'cloudpods',
            status: 'connected',
            region: 'us-west-1',
            resources: 25,
            lastSync: '2024-01-15T10:30:00Z',
          },
          {
            id: 'aviatrix-1',
            name: 'Aviatrix Controller',
            type: 'aviatrix',
            status: 'connected',
            region: 'us-east-1',
            resources: 12,
            lastSync: '2024-01-15T10:25:00Z',
          },
        ]);

        setNetworkResources([
          {
            id: 'gw-1',
            name: 'transit-gw-aws-us-west-1',
            type: 'gateway',
            provider: 'aviatrix',
            status: 'active',
            region: 'us-west-1',
            properties: {
              asn: 65001,
              publicIp: '54.123.45.67',
              privateIp: '10.0.1.10',
              size: 't3.medium',
            },
          },
          {
            id: 'vpc-1',
            name: 'production-vpc',
            type: 'vpc',
            provider: 'cloudpods',
            status: 'active',
            region: 'us-west-1',
            properties: {
              cidr: '10.0.0.0/16',
              subnets: 4,
              securityGroups: 3,
            },
          },
          {
            id: 'subnet-1',
            name: 'public-subnet-1a',
            type: 'subnet',
            provider: 'cloudpods',
            status: 'active',
            region: 'us-west-1',
            properties: {
              cidr: '10.0.1.0/24',
              vpc: 'production-vpc',
              gateway: '10.0.1.1',
            },
          },
        ]);

        setTopology({
          nodes: [
            { id: 'cloudpods', type: 'provider', label: 'CloudPods', x: 100, y: 100, status: 'active', provider: 'cloudpods' },
            { id: 'aviatrix', type: 'provider', label: 'Aviatrix', x: 300, y: 100, status: 'active', provider: 'aviatrix' },
            { id: 'vpc-1', type: 'vpc', label: 'Production VPC', x: 100, y: 200, status: 'active', provider: 'cloudpods' },
            { id: 'gw-1', type: 'gateway', label: 'Transit GW', x: 300, y: 200, status: 'active', provider: 'aviatrix' },
            { id: 'subnet-1', type: 'subnet', label: 'Public Subnet', x: 100, y: 300, status: 'active', provider: 'cloudpods' },
          ],
          edges: [
            { source: 'cloudpods', target: 'vpc-1', type: 'manages', status: 'active' },
            { source: 'aviatrix', target: 'gw-1', type: 'manages', status: 'active' },
            { source: 'vpc-1', target: 'subnet-1', type: 'contains', status: 'active' },
            { source: 'gw-1', target: 'vpc-1', type: 'connects', status: 'active' },
          ],
        });

        setLoading(false);
      } catch (err) {
        setError('Failed to load cloud networking data');
        setLoading(false);
      }
    };

    loadData();
  }, []);

  const handleTabChange = (event: React.SyntheticEvent, newValue: number) => {
    setActiveTab(newValue);
  };

  const handleRefresh = () => {
    setLoading(true);
    // Simulate refresh
    setTimeout(() => setLoading(false), 1000);
  };

  const getStatusColor = (status: string) => {
    switch (status) {
      case 'active':
      case 'connected':
        return 'success';
      case 'inactive':
      case 'disconnected':
        return 'default';
      case 'error':
        return 'error';
      default:
        return 'default';
    }
  };

  const getStatusIcon = (status: string) => {
    switch (status) {
      case 'active':
      case 'connected':
        return <CheckCircle color="success" />;
      case 'inactive':
      case 'disconnected':
        return <CloudOff color="disabled" />;
      case 'error':
        return <Error color="error" />;
      default:
        return <Warning color="warning" />;
    }
  };

  const renderCloudProviders = () => (
    <Grid container spacing={3}>
      {cloudProviders.map((provider) => (
        <Grid item xs={12} md={6} key={provider.id}>
          <Card>
            <CardContent>
              <Box display="flex" alignItems="center" justifyContent="space-between" mb={2}>
                <Box display="flex" alignItems="center">
                  {provider.type === 'cloudpods' ? <CloudQueue /> : <CloudSync />}
                  <Typography variant="h6" sx={{ ml: 1 }}>
                    {provider.name}
                  </Typography>
                </Box>
                <Chip
                  icon={getStatusIcon(provider.status)}
                  label={provider.status}
                  color={getStatusColor(provider.status) as any}
                  size="small"
                />
              </Box>
              <Typography variant="body2" color="text.secondary" gutterBottom>
                Region: {provider.region}
              </Typography>
              <Typography variant="body2" color="text.secondary" gutterBottom>
                Resources: {provider.resources}
              </Typography>
              <Typography variant="body2" color="text.secondary">
                Last Sync: {new Date(provider.lastSync).toLocaleString()}
              </Typography>
              <Box mt={2} display="flex" gap={1}>
                <Button size="small" startIcon={<Refresh />} onClick={handleRefresh}>
                  Sync
                </Button>
                <Button size="small" startIcon={<Settings />}>
                  Configure
                </Button>
              </Box>
            </CardContent>
          </Card>
        </Grid>
      ))}
    </Grid>
  );

  const renderNetworkResources = () => (
    <TableContainer component={Paper}>
      <Table>
        <TableHead>
          <TableRow>
            <TableCell>Name</TableCell>
            <TableCell>Type</TableCell>
            <TableCell>Provider</TableCell>
            <TableCell>Status</TableCell>
            <TableCell>Region</TableCell>
            <TableCell>Actions</TableCell>
          </TableRow>
        </TableHead>
        <TableBody>
          {networkResources.map((resource) => (
            <TableRow key={resource.id}>
              <TableCell>
                <Box display="flex" alignItems="center">
                  {resource.type === 'gateway' && <Router />}
                  {resource.type === 'vpc' && <Cloud />}
                  {resource.type === 'subnet' && <NetworkCheck />}
                  {resource.type === 'security-group' && <Security />}
                  {resource.type === 'load-balancer' && <Speed />}
                  <Typography sx={{ ml: 1 }}>{resource.name}</Typography>
                </Box>
              </TableCell>
              <TableCell>
                <Chip label={resource.type} size="small" />
              </TableCell>
              <TableCell>
                <Chip 
                  label={resource.provider} 
                  size="small" 
                  color={resource.provider === 'cloudpods' ? 'primary' : 'secondary'}
                />
              </TableCell>
              <TableCell>
                <Chip
                  icon={getStatusIcon(resource.status)}
                  label={resource.status}
                  color={getStatusColor(resource.status) as any}
                  size="small"
                />
              </TableCell>
              <TableCell>{resource.region}</TableCell>
              <TableCell>
                <Tooltip title="View Details">
                  <IconButton size="small" onClick={() => setSelectedResource(resource)}>
                    <Visibility />
                  </IconButton>
                </Tooltip>
                <Tooltip title="Edit">
                  <IconButton size="small">
                    <Edit />
                  </IconButton>
                </Tooltip>
                <Tooltip title="Delete">
                  <IconButton size="small" color="error">
                    <Delete />
                  </IconButton>
                </Tooltip>
              </TableCell>
            </TableRow>
          ))}
        </TableBody>
      </Table>
    </TableContainer>
  );

  const renderNetworkTopology = () => (
    <Box>
      <Typography variant="h6" gutterBottom>
        Network Topology
      </Typography>
      <Box 
        sx={{ 
          border: '1px solid #e0e0e0', 
          borderRadius: 1, 
          p: 2, 
          minHeight: 400,
          position: 'relative',
          background: 'linear-gradient(45deg, #f5f5f5 25%, transparent 25%), linear-gradient(-45deg, #f5f5f5 25%, transparent 25%), linear-gradient(45deg, transparent 75%, #f5f5f5 75%), linear-gradient(-45deg, transparent 75%, #f5f5f5 75%)',
          backgroundSize: '20px 20px',
          backgroundPosition: '0 0, 0 10px, 10px -10px, -10px 0px',
        }}
      >
        {topology.nodes.map((node) => (
          <Box
            key={node.id}
            sx={{
              position: 'absolute',
              left: node.x,
              top: node.y,
              width: 120,
              height: 60,
              border: '2px solid',
              borderColor: getStatusColor(node.status) === 'success' ? 'green' : 
                          getStatusColor(node.status) === 'error' ? 'red' : 'gray',
              borderRadius: 2,
              backgroundColor: 'white',
              display: 'flex',
              flexDirection: 'column',
              alignItems: 'center',
              justifyContent: 'center',
              cursor: 'pointer',
              '&:hover': {
                boxShadow: 3,
              },
            }}
          >
            <Typography variant="caption" fontWeight="bold">
              {node.label}
            </Typography>
            <Typography variant="caption" color="text.secondary">
              {node.type}
            </Typography>
          </Box>
        ))}
        
        {/* Render edges */}
        <svg style={{ position: 'absolute', top: 0, left: 0, width: '100%', height: '100%', pointerEvents: 'none' }}>
          {topology.edges.map((edge, index) => {
            const sourceNode = topology.nodes.find(n => n.id === edge.source);
            const targetNode = topology.nodes.find(n => n.id === edge.target);
            if (!sourceNode || !targetNode) return null;
            
            return (
              <line
                key={index}
                x1={sourceNode.x + 60}
                y1={sourceNode.y + 30}
                x2={targetNode.x + 60}
                y2={targetNode.y + 30}
                stroke={getStatusColor(edge.status) === 'success' ? 'green' : 'gray'}
                strokeWidth="2"
                markerEnd="url(#arrowhead)"
              />
            );
          })}
          <defs>
            <marker id="arrowhead" markerWidth="10" markerHeight="7" refX="9" refY="3.5" orient="auto">
              <polygon points="0 0, 10 3.5, 0 7" fill="gray" />
            </marker>
          </defs>
        </svg>
      </Box>
    </Box>
  );

  const renderAnalytics = () => {
    const trafficData = [
      { time: '00:00', packets: 1200, bytes: 1500000 },
      { time: '04:00', packets: 800, bytes: 1000000 },
      { time: '08:00', packets: 2000, bytes: 2500000 },
      { time: '12:00', packets: 3500, bytes: 4000000 },
      { time: '16:00', packets: 2800, bytes: 3200000 },
      { time: '20:00', packets: 1500, bytes: 1800000 },
    ];

    const protocolData = [
      { name: 'HTTP', value: 45, color: '#8884d8' },
      { name: 'HTTPS', value: 30, color: '#82ca9d' },
      { name: 'SSH', value: 15, color: '#ffc658' },
      { name: 'DNS', value: 10, color: '#ff7300' },
    ];

    const regionData = [
      { region: 'us-west-1', resources: 15, traffic: 2500 },
      { region: 'us-east-1', resources: 12, traffic: 1800 },
      { region: 'eu-west-1', resources: 8, traffic: 1200 },
    ];

    return (
      <Grid container spacing={3}>
        <Grid item xs={12} md={8}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>
                Network Traffic Over Time
              </Typography>
              <ResponsiveContainer width="100%" height={300}>
                <LineChart data={trafficData}>
                  <CartesianGrid strokeDasharray="3 3" />
                  <XAxis dataKey="time" />
                  <YAxis />
                  <RechartsTooltip />
                  <Line type="monotone" dataKey="packets" stroke="#8884d8" strokeWidth={2} />
                  <Line type="monotone" dataKey="bytes" stroke="#82ca9d" strokeWidth={2} />
                </LineChart>
              </ResponsiveContainer>
            </CardContent>
          </Card>
        </Grid>
        
        <Grid item xs={12} md={4}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>
                Protocol Distribution
              </Typography>
              <ResponsiveContainer width="100%" height={300}>
                <PieChart>
                  <Pie
                    data={protocolData}
                    cx="50%"
                    cy="50%"
                    labelLine={false}
                    label={({ name, percent }) => `${name} ${(percent * 100).toFixed(0)}%`}
                    outerRadius={80}
                    fill="#8884d8"
                    dataKey="value"
                  >
                    {protocolData.map((entry, index) => (
                      <Cell key={`cell-${index}`} fill={entry.color} />
                    ))}
                  </Pie>
                  <RechartsTooltip />
                </PieChart>
              </ResponsiveContainer>
            </CardContent>
          </Card>
        </Grid>

        <Grid item xs={12}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>
                Resources by Region
              </Typography>
              <ResponsiveContainer width="100%" height={300}>
                <BarChart data={regionData}>
                  <CartesianGrid strokeDasharray="3 3" />
                  <XAxis dataKey="region" />
                  <YAxis />
                  <RechartsTooltip />
                  <Bar dataKey="resources" fill="#8884d8" />
                  <Bar dataKey="traffic" fill="#82ca9d" />
                </BarChart>
              </ResponsiveContainer>
            </CardContent>
          </Card>
        </Grid>
      </Grid>
    );
  };

  if (loading) {
    return (
      <Box display="flex" justifyContent="center" alignItems="center" minHeight="400px">
        <CircularProgress />
      </Box>
    );
  }

  return (
    <Box>
      <Box display="flex" justifyContent="space-between" alignItems="center" mb={3}>
        <Typography variant="h4" component="h1">
          Cloud Networking
        </Typography>
        <Box>
          <Button
            variant="contained"
            startIcon={<Add />}
            onClick={() => setCreateDialogOpen(true)}
            sx={{ mr: 1 }}
          >
            Add Resource
          </Button>
          <Button
            variant="outlined"
            startIcon={<Refresh />}
            onClick={handleRefresh}
          >
            Refresh
          </Button>
        </Box>
      </Box>

      {error && (
        <Alert severity="error" sx={{ mb: 3 }} onClose={() => setError(null)}>
          {error}
        </Alert>
      )}

      <Tabs value={activeTab} onChange={handleTabChange} sx={{ mb: 3 }}>
        <Tab label="Cloud Providers" />
        <Tab label="Network Resources" />
        <Tab label="Topology" />
        <Tab label="Analytics" />
      </Tabs>

      {activeTab === 0 && renderCloudProviders()}
      {activeTab === 1 && renderNetworkResources()}
      {activeTab === 2 && renderNetworkTopology()}
      {activeTab === 3 && renderAnalytics()}

      {/* Resource Details Dialog */}
      <Dialog open={!!selectedResource} onClose={() => setSelectedResource(null)} maxWidth="md" fullWidth>
        <DialogTitle>Resource Details</DialogTitle>
        <DialogContent>
          {selectedResource && (
            <Box>
              <Typography variant="h6" gutterBottom>
                {selectedResource.name}
              </Typography>
              <Grid container spacing={2}>
                <Grid item xs={6}>
                  <Typography variant="body2" color="text.secondary">
                    Type: {selectedResource.type}
                  </Typography>
                </Grid>
                <Grid item xs={6}>
                  <Typography variant="body2" color="text.secondary">
                    Provider: {selectedResource.provider}
                  </Typography>
                </Grid>
                <Grid item xs={6}>
                  <Typography variant="body2" color="text.secondary">
                    Status: {selectedResource.status}
                  </Typography>
                </Grid>
                <Grid item xs={6}>
                  <Typography variant="body2" color="text.secondary">
                    Region: {selectedResource.region}
                  </Typography>
                </Grid>
              </Grid>
              
              <Typography variant="h6" sx={{ mt: 3, mb: 2 }}>
                Properties
              </Typography>
              {Object.entries(selectedResource.properties).map(([key, value]) => (
                <Box key={key} display="flex" justifyContent="space-between" py={1}>
                  <Typography variant="body2" fontWeight="bold">
                    {key}:
                  </Typography>
                  <Typography variant="body2">
                    {typeof value === 'object' ? JSON.stringify(value) : String(value)}
                  </Typography>
                </Box>
              ))}
            </Box>
          )}
        </DialogContent>
        <DialogActions>
          <Button onClick={() => setSelectedResource(null)}>Close</Button>
        </DialogActions>
      </Dialog>

      {/* Create Resource Dialog */}
      <Dialog open={createDialogOpen} onClose={() => setCreateDialogOpen(false)} maxWidth="sm" fullWidth>
        <DialogTitle>Create New Resource</DialogTitle>
        <DialogContent>
          <Box sx={{ pt: 2 }}>
            <FormControl fullWidth sx={{ mb: 2 }}>
              <InputLabel>Resource Type</InputLabel>
              <Select label="Resource Type">
                <MenuItem value="gateway">Gateway</MenuItem>
                <MenuItem value="vpc">VPC</MenuItem>
                <MenuItem value="subnet">Subnet</MenuItem>
                <MenuItem value="security-group">Security Group</MenuItem>
                <MenuItem value="load-balancer">Load Balancer</MenuItem>
              </Select>
            </FormControl>
            
            <TextField
              fullWidth
              label="Resource Name"
              sx={{ mb: 2 }}
            />
            
            <FormControl fullWidth sx={{ mb: 2 }}>
              <InputLabel>Provider</InputLabel>
              <Select label="Provider">
                <MenuItem value="cloudpods">CloudPods</MenuItem>
                <MenuItem value="aviatrix">Aviatrix</MenuItem>
              </Select>
            </FormControl>
            
            <TextField
              fullWidth
              label="Region"
              sx={{ mb: 2 }}
            />
          </Box>
        </DialogContent>
        <DialogActions>
          <Button onClick={() => setCreateDialogOpen(false)}>Cancel</Button>
          <Button variant="contained">Create</Button>
        </DialogActions>
      </Dialog>
    </Box>
  );
};

export default CloudNetworking;
