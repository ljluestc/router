import React, { useState, useEffect } from 'react';
import {
  Box,
  Card,
  CardContent,
  Typography,
  Button,
  Grid,
  Chip,
  Table,
  TableBody,
  TableCell,
  TableContainer,
  TableHead,
  TableRow,
  Paper,
  IconButton,
  Dialog,
  DialogTitle,
  DialogContent,
  DialogActions,
  TextField,
  MenuItem,
  LinearProgress,
  Alert,
  Tabs,
  Tab,
} from '@mui/material';
import {
  Add as AddIcon,
  Edit as EditIcon,
  Delete as DeleteIcon,
  Refresh as RefreshIcon,
  Cloud as CloudIcon,
  Router as RouterIcon,
  Security as SecurityIcon,
  Analytics as AnalyticsIcon,
  Network as NetworkIcon,
  CheckCircle as CheckCircleIcon,
  Error as ErrorIcon,
  Warning as WarningIcon,
} from '@mui/icons-material';

interface CloudProvider {
  id: string;
  name: string;
  type: 'cloudpods' | 'aviatrix' | 'aws' | 'azure' | 'gcp';
  status: 'connected' | 'disconnected' | 'error';
  region: string;
  resources: number;
  cost: number;
  lastSync: string;
}

interface NetworkNode {
  id: string;
  type: 'gateway' | 'router' | 'switch' | 'server' | 'cloud' | 'region';
  name: string;
  status: 'active' | 'inactive' | 'error';
  position: { x: number; y: number };
  properties: Record<string, any>;
  metrics: {
    cpu: number;
    memory: number;
    network: number;
    latency: number;
  };
}

interface NetworkEdge {
  id: string;
  source: string;
  target: string;
  type: 'bgp' | 'ospf' | 'isis' | 'static' | 'cloud';
  status: 'up' | 'down' | 'degraded';
  bandwidth: number;
  latency: number;
  utilization: number;
}

interface TrafficFlow {
  id: string;
  source: string;
  destination: string;
  protocol: string;
  bytes: number;
  packets: number;
  latency: number;
  jitter: number;
  loss: number;
  timestamp: string;
}

const CloudNetworking: React.FC = () => {
  const [cloudProviders, setCloudProviders] = useState<CloudProvider[]>([
    {
      id: '1',
      name: 'CloudPods Multi-Cloud',
      type: 'cloudpods',
      status: 'connected',
      region: 'us-west-1',
      resources: 45,
      cost: 1250.50,
      lastSync: '2 minutes ago',
    },
    {
      id: '2',
      name: 'Aviatrix Controller',
      type: 'aviatrix',
      status: 'connected',
      region: 'us-east-1',
      resources: 23,
      cost: 890.25,
      lastSync: '1 minute ago',
    },
    {
      id: '3',
      name: 'AWS Cloud',
      type: 'aws',
      status: 'connected',
      region: 'us-west-2',
      resources: 67,
      cost: 2100.75,
      lastSync: '3 minutes ago',
    },
    {
      id: '4',
      name: 'Azure Cloud',
      type: 'azure',
      status: 'error',
      region: 'eastus',
      resources: 0,
      cost: 0,
      lastSync: 'Never',
    },
  ]);

  const [networkTopology, setNetworkTopology] = useState<{
    nodes: NetworkNode[];
    edges: NetworkEdge[];
  }>({
    nodes: [
      {
        id: '1',
        type: 'cloud',
        name: 'CloudPods',
        status: 'active',
        position: { x: 100, y: 100 },
        properties: { provider: 'cloudpods', region: 'us-west-1' },
        metrics: { cpu: 45, memory: 67, network: 23, latency: 12 },
      },
      {
        id: '2',
        type: 'gateway',
        name: 'Aviatrix Gateway',
        status: 'active',
        position: { x: 300, y: 100 },
        properties: { provider: 'aviatrix', region: 'us-east-1' },
        metrics: { cpu: 78, memory: 45, network: 89, latency: 8 },
      },
      {
        id: '3',
        type: 'router',
        name: 'BGP Router',
        status: 'active',
        position: { x: 500, y: 100 },
        properties: { protocol: 'bgp', asn: 65001 },
        metrics: { cpu: 34, memory: 56, network: 78, latency: 5 },
      },
      {
        id: '4',
        type: 'server',
        name: 'Web Server',
        status: 'active',
        position: { x: 700, y: 100 },
        properties: { service: 'web', port: 80 },
        metrics: { cpu: 67, memory: 78, network: 45, latency: 3 },
      },
    ],
    edges: [
      {
        id: '1',
        source: '1',
        target: '2',
        type: 'cloud',
        status: 'up',
        bandwidth: 1000,
        latency: 15,
        utilization: 45,
      },
      {
        id: '2',
        source: '2',
        target: '3',
        type: 'bgp',
        status: 'up',
        bandwidth: 10000,
        latency: 2,
        utilization: 23,
      },
      {
        id: '3',
        source: '3',
        target: '4',
        type: 'ospf',
        status: 'up',
        bandwidth: 1000,
        latency: 1,
        utilization: 67,
      },
    ],
  });

  const [trafficFlows, setTrafficFlows] = useState<TrafficFlow[]>([
    {
      id: '1',
      source: 'CloudPods',
      destination: 'Web Server',
      protocol: 'HTTP',
      bytes: 1024000,
      packets: 1500,
      latency: 25,
      jitter: 5,
      loss: 0.1,
      timestamp: '2024-01-15T10:30:00Z',
    },
    {
      id: '2',
      source: 'Aviatrix Gateway',
      destination: 'BGP Router',
      protocol: 'BGP',
      bytes: 512000,
      packets: 800,
      latency: 8,
      jitter: 2,
      loss: 0,
      timestamp: '2024-01-15T10:30:00Z',
    },
  ]);

  const [selectedProvider, setSelectedProvider] = useState<string>('all');
  const [selectedRegion, setSelectedRegion] = useState<string>('all');
  const [isLoading, setIsLoading] = useState(false);
  const [tabValue, setTabValue] = useState(0);

  const getStatusIcon = (status: string) => {
    switch (status) {
      case 'connected':
      case 'active':
      case 'up':
        return <CheckCircleIcon color="success" />;
      case 'disconnected':
      case 'inactive':
      case 'down':
        return <ErrorIcon color="error" />;
      case 'error':
        return <WarningIcon color="warning" />;
      default:
        return <ErrorIcon color="disabled" />;
    }
  };

  const getStatusColor = (status: string) => {
    switch (status) {
      case 'connected':
      case 'active':
      case 'up':
        return 'success';
      case 'disconnected':
      case 'inactive':
      case 'down':
        return 'error';
      case 'error':
        return 'warning';
      default:
        return 'default';
    }
  };

  const getProviderIcon = (type: string) => {
    switch (type) {
      case 'cloudpods':
        return <CloudIcon />;
      case 'aviatrix':
        return <SecurityIcon />;
      case 'aws':
        return <RouterIcon />;
      case 'azure':
        return <NetworkIcon />;
      case 'gcp':
        return <CloudIcon />;
      default:
        return <NetworkIcon />;
    }
  };

  const getNodeIcon = (type: string) => {
    switch (type) {
      case 'cloud':
        return <CloudIcon />;
      case 'gateway':
        return <SecurityIcon />;
      case 'router':
        return <RouterIcon />;
      case 'switch':
        return <NetworkIcon />;
      case 'server':
        return <RouterIcon />;
      case 'region':
        return <CloudIcon />;
      default:
        return <NetworkIcon />;
    }
  };

  const handleRefresh = async () => {
    setIsLoading(true);
    // Simulate API call
    await new Promise(resolve => setTimeout(resolve, 1000));
    setIsLoading(false);
  };

  const handleTabChange = (event: React.SyntheticEvent, newValue: number) => {
    setTabValue(newValue);
  };

  return (
    <Box>
      {/* Header */}
      <Box sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', mb: 3 }}>
        <Box>
          <Typography variant="h4" component="h1">
            Cloud Networking
          </Typography>
          <Typography variant="body1" color="text.secondary">
            Multi-cloud network management and monitoring
          </Typography>
        </Box>
        <Box>
          <Button
            variant="outlined"
            startIcon={<RefreshIcon />}
            onClick={handleRefresh}
            disabled={isLoading}
            sx={{ mr: 1 }}
          >
            Refresh
          </Button>
          <Button variant="contained" startIcon={<AddIcon />}>
            Add Provider
          </Button>
        </Box>
      </Box>

      {/* Stats Cards */}
      <Grid container spacing={3} sx={{ mb: 3 }}>
        <Grid item xs={12} md={3}>
          <Card>
            <CardContent>
              <Box sx={{ display: 'flex', alignItems: 'center' }}>
                <CloudIcon color="primary" sx={{ mr: 2, fontSize: 40 }} />
                <Box>
                  <Typography variant="h6" color="text.secondary">
                    Total Providers
                  </Typography>
                  <Typography variant="h4">{cloudProviders.length}</Typography>
                </Box>
              </Box>
            </CardContent>
          </Card>
        </Grid>
        <Grid item xs={12} md={3}>
          <Card>
            <CardContent>
              <Box sx={{ display: 'flex', alignItems: 'center' }}>
                <RouterIcon color="success" sx={{ mr: 2, fontSize: 40 }} />
                <Box>
                  <Typography variant="h6" color="text.secondary">
                    Active Resources
                  </Typography>
                  <Typography variant="h4">
                    {cloudProviders.reduce((sum, p) => sum + p.resources, 0)}
                  </Typography>
                </Box>
              </Box>
            </CardContent>
          </Card>
        </Grid>
        <Grid item xs={12} md={3}>
          <Card>
            <CardContent>
              <Box sx={{ display: 'flex', alignItems: 'center' }}>
                <AnalyticsIcon color="warning" sx={{ mr: 2, fontSize: 40 }} />
                <Box>
                  <Typography variant="h6" color="text.secondary">
                    Total Cost
                  </Typography>
                  <Typography variant="h4">
                    ${cloudProviders.reduce((sum, p) => sum + p.cost, 0).toFixed(2)}
                  </Typography>
                </Box>
              </Box>
            </CardContent>
          </Card>
        </Grid>
        <Grid item xs={12} md={3}>
          <Card>
            <CardContent>
              <Box sx={{ display: 'flex', alignItems: 'center' }}>
                <NetworkIcon color="info" sx={{ mr: 2, fontSize: 40 }} />
                <Box>
                  <Typography variant="h6" color="text.secondary">
                    Network Health
                  </Typography>
                  <Typography variant="h4">98.5%</Typography>
                </Box>
              </Box>
            </CardContent>
          </Card>
        </Grid>
      </Grid>

      {/* Main Content Tabs */}
      <Card>
        <Box sx={{ borderBottom: 1, borderColor: 'divider' }}>
          <Tabs value={tabValue} onChange={handleTabChange} aria-label="cloud networking tabs">
            <Tab label="Overview" />
            <Tab label="Network Topology" />
            <Tab label="Traffic Flows" />
            <Tab label="Security" />
            <Tab label="Analytics" />
          </Tabs>
        </Box>

        {/* Overview Tab */}
        {tabValue === 0 && (
          <CardContent>
            <Grid container spacing={3}>
              {/* Cloud Providers */}
              <Grid item xs={12} md={6}>
                <Typography variant="h6" gutterBottom>
                  Cloud Providers
                </Typography>
                <Box sx={{ maxHeight: 400, overflow: 'auto' }}>
                  {cloudProviders.map((provider) => (
                    <Card key={provider.id} sx={{ mb: 2 }}>
                      <CardContent>
                        <Box sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center' }}>
                          <Box sx={{ display: 'flex', alignItems: 'center' }}>
                            {getProviderIcon(provider.type)}
                            <Box sx={{ ml: 2 }}>
                              <Typography variant="subtitle1">{provider.name}</Typography>
                              <Typography variant="body2" color="text.secondary">
                                {provider.region} • {provider.resources} resources
                              </Typography>
                            </Box>
                          </Box>
                          <Box sx={{ textAlign: 'right' }}>
                            <Box sx={{ display: 'flex', alignItems: 'center', mb: 1 }}>
                              {getStatusIcon(provider.status)}
                              <Chip 
                                label={provider.status} 
                                color={getStatusColor(provider.status)}
                                size="small"
                                sx={{ ml: 1 }}
                              />
                            </Box>
                            <Typography variant="h6">
                              ${provider.cost.toFixed(2)}
                            </Typography>
                            <Typography variant="caption" color="text.secondary">
                              {provider.lastSync}
                            </Typography>
                          </Box>
                        </Box>
                      </CardContent>
                    </Card>
                  ))}
                </Box>
              </Grid>

              {/* Network Health */}
              <Grid item xs={12} md={6}>
                <Typography variant="h6" gutterBottom>
                  Network Health
                </Typography>
                <Card>
                  <CardContent>
                    <Box sx={{ mb: 2 }}>
                      <Box sx={{ display: 'flex', justifyContent: 'space-between', mb: 1 }}>
                        <Typography variant="body2">Overall Health</Typography>
                        <Typography variant="body2">98.5%</Typography>
                      </Box>
                      <LinearProgress variant="determinate" value={98.5} />
                    </Box>
                    
                    <Box sx={{ mt: 3 }}>
                      <Box sx={{ display: 'flex', justifyContent: 'space-between', mb: 1 }}>
                        <Typography variant="body2">BGP Sessions</Typography>
                        <Chip label="12/12" color="success" size="small" />
                      </Box>
                      <Box sx={{ display: 'flex', justifyContent: 'space-between', mb: 1 }}>
                        <Typography variant="body2">OSPF Neighbors</Typography>
                        <Chip label="8/8" color="success" size="small" />
                      </Box>
                      <Box sx={{ display: 'flex', justifyContent: 'space-between', mb: 1 }}>
                        <Typography variant="body2">ISIS Adjacencies</Typography>
                        <Chip label="6/6" color="success" size="small" />
                      </Box>
                      <Box sx={{ display: 'flex', justifyContent: 'space-between' }}>
                        <Typography variant="body2">Traffic Shaping</Typography>
                        <Chip label="Active" color="success" size="small" />
                      </Box>
                    </Box>
                  </CardContent>
                </Card>
              </Grid>
            </Grid>
          </CardContent>
        )}

        {/* Network Topology Tab */}
        {tabValue === 1 && (
          <CardContent>
            <Typography variant="h6" gutterBottom>
              Network Topology
            </Typography>
            <Box sx={{ border: 1, borderColor: 'divider', borderRadius: 1, p: 2, minHeight: 400, backgroundColor: 'grey.50' }}>
              <Typography variant="body2" color="text.secondary" align="center" sx={{ mt: 20 }}>
                Network topology visualization coming soon...
              </Typography>
            </Box>
          </CardContent>
        )}

        {/* Traffic Flows Tab */}
        {tabValue === 2 && (
          <CardContent>
            <Typography variant="h6" gutterBottom>
              Traffic Flows
            </Typography>
            <TableContainer component={Paper} sx={{ backgroundColor: 'transparent' }}>
              <Table>
                <TableHead>
                  <TableRow>
                    <TableCell>Source</TableCell>
                    <TableCell>Destination</TableCell>
                    <TableCell>Protocol</TableCell>
                    <TableCell>Bytes</TableCell>
                    <TableCell>Packets</TableCell>
                    <TableCell>Latency</TableCell>
                    <TableCell>Jitter</TableCell>
                    <TableCell>Loss</TableCell>
                  </TableRow>
                </TableHead>
                <TableBody>
                  {trafficFlows.map((flow) => (
                    <TableRow key={flow.id}>
                      <TableCell>{flow.source}</TableCell>
                      <TableCell>{flow.destination}</TableCell>
                      <TableCell>
                        <Chip label={flow.protocol} size="small" />
                      </TableCell>
                      <TableCell>{(flow.bytes / 1024 / 1024).toFixed(2)} MB</TableCell>
                      <TableCell>{flow.packets.toLocaleString()}</TableCell>
                      <TableCell>{flow.latency}ms</TableCell>
                      <TableCell>{flow.jitter}ms</TableCell>
                      <TableCell>
                        <Typography color={flow.loss > 0 ? 'error' : 'success'}>
                          {flow.loss}%
                        </Typography>
                      </TableCell>
                    </TableRow>
                  ))}
                </TableBody>
              </Table>
            </TableContainer>
          </CardContent>
        )}

        {/* Security Tab */}
        {tabValue === 3 && (
          <CardContent>
            <Typography variant="h6" gutterBottom>
              Security Policies
            </Typography>
            <Alert severity="info">
              Security policies and firewall rules management coming soon...
            </Alert>
          </CardContent>
        )}

        {/* Analytics Tab */}
        {tabValue === 4 && (
          <CardContent>
            <Typography variant="h6" gutterBottom>
              Analytics Dashboard
            </Typography>
            <Alert severity="info">
              Real-time analytics and monitoring charts coming soon...
            </Alert>
          </CardContent>
        )}
      </Card>
    </Box>
  );
};

export default CloudNetworking;
