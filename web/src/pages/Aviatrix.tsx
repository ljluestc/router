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
  Tabs,
  Tab,
  Alert,
  LinearProgress,
} from '@mui/material';
import {
  Add as AddIcon,
  Edit as EditIcon,
  Delete as DeleteIcon,
  Refresh as RefreshIcon,
  Router as RouterIcon,
  Cloud as CloudIcon,
  Speed as SpeedIcon,
  Security as SecurityIcon,
  CheckCircle as CheckCircleIcon,
  Warning as WarningIcon,
  Error as ErrorIcon,
} from '@mui/icons-material';
import { LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip, ResponsiveContainer } from 'recharts';

interface AviatrixGateway {
  id: string;
  name: string;
  type: 'transit' | 'spoke';
  cloudType: string;
  region: string;
  vpcId: string;
  subnet: string;
  gatewaySize: string;
  publicIp: string;
  privateIp: string;
  status: 'active' | 'pending' | 'error' | 'stopped';
  asn: number;
  bgpEnabled: boolean;
  connectedTransit?: boolean;
  learnedCidrs?: string[];
  advertisedCidrs?: string[];
  transitGateway?: string;
  haGateway?: string;
  created_at: string;
}

interface AviatrixConnection {
  id: string;
  name: string;
  sourceGateway: string;
  destGateway: string;
  status: 'connected' | 'connecting' | 'disconnected' | 'error';
  type: 'transit-to-transit' | 'transit-to-spoke' | 'spoke-to-spoke';
  bandwidth: number;
  latency: number;
  packetLoss: number;
}

interface AviatrixRoute {
  destination: string;
  nextHop: string;
  protocol: string;
  metric: number;
  asPath: number[];
  age: string;
}

const Aviatrix: React.FC = () => {
  const [gateways, setGateways] = useState<AviatrixGateway[]>([]);
  const [connections, setConnections] = useState<AviatrixConnection[]>([]);
  const [routes, setRoutes] = useState<AviatrixRoute[]>([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);
  const [tabValue, setTabValue] = useState(0);
  const [openDialog, setOpenDialog] = useState(false);
  const [selectedGateway, setSelectedGateway] = useState<AviatrixGateway | null>(null);
  const [dialogMode, setDialogMode] = useState<'create' | 'edit'>('create');

  useEffect(() => {
    fetchAviatrixData();
    const interval = setInterval(fetchAviatrixData, 10000);
    return () => clearInterval(interval);
  }, []);

  const fetchAviatrixData = async () => {
    try {
      setLoading(true);
      setError(null);

      const [gatewaysRes, connectionsRes, routesRes] = await Promise.all([
        fetch('/api/v1/aviatrix/gateways'),
        fetch('/api/v1/aviatrix/connections'),
        fetch('/api/v1/aviatrix/routes'),
      ]);

      if (gatewaysRes.ok) {
        const gatewaysData = await gatewaysRes.json();
        setGateways(gatewaysData.gateways || []);
      } else {
        // Mock data for demo
        setGateways([
          {
            id: '1',
            name: 'transit-gw-aws-us-east-1',
            type: 'transit',
            cloudType: 'AWS',
            region: 'us-east-1',
            vpcId: 'vpc-12345678',
            subnet: '10.0.0.0/24',
            gatewaySize: 't3.medium',
            publicIp: '54.123.45.67',
            privateIp: '10.0.0.10',
            status: 'active',
            asn: 65001,
            bgpEnabled: true,
            connectedTransit: true,
            learnedCidrs: ['10.1.0.0/16', '10.2.0.0/16'],
            advertisedCidrs: ['10.0.0.0/16'],
            created_at: '2024-01-15T10:30:00Z',
          },
          {
            id: '2',
            name: 'spoke-gw-azure-west-europe',
            type: 'spoke',
            cloudType: 'Azure',
            region: 'west-europe',
            vpcId: 'vnet-87654321',
            subnet: '10.1.0.0/24',
            gatewaySize: 'Standard_B2s',
            publicIp: '20.123.45.67',
            privateIp: '10.1.0.10',
            status: 'active',
            asn: 65002,
            bgpEnabled: true,
            transitGateway: 'transit-gw-aws-us-east-1',
            created_at: '2024-01-16T14:20:00Z',
          },
          {
            id: '3',
            name: 'spoke-gw-gcp-asia-southeast1',
            type: 'spoke',
            cloudType: 'GCP',
            region: 'asia-southeast1',
            vpcId: 'projects/project-123/global/networks/vpc-1',
            subnet: '10.2.0.0/24',
            gatewaySize: 'n1-standard-2',
            publicIp: '34.123.45.67',
            privateIp: '10.2.0.10',
            status: 'pending',
            asn: 65003,
            bgpEnabled: true,
            transitGateway: 'transit-gw-aws-us-east-1',
            created_at: '2024-01-17T09:15:00Z',
          },
        ]);
      }

      if (connectionsRes.ok) {
        const connectionsData = await connectionsRes.json();
        setConnections(connectionsData.connections || []);
      } else {
        // Mock connections data
        setConnections([
          {
            id: '1',
            name: 'transit-to-spoke-1',
            sourceGateway: 'transit-gw-aws-us-east-1',
            destGateway: 'spoke-gw-azure-west-europe',
            status: 'connected',
            type: 'transit-to-spoke',
            bandwidth: 1000,
            latency: 45,
            packetLoss: 0.1,
          },
          {
            id: '2',
            name: 'transit-to-spoke-2',
            sourceGateway: 'transit-gw-aws-us-east-1',
            destGateway: 'spoke-gw-gcp-asia-southeast1',
            status: 'connecting',
            type: 'transit-to-spoke',
            bandwidth: 1000,
            latency: 0,
            packetLoss: 0,
          },
        ]);
      }

      if (routesRes.ok) {
        const routesData = await routesRes.json();
        setRoutes(routesData.routes || []);
      } else {
        // Mock routes data
        setRoutes([
          {
            destination: '10.1.0.0/16',
            nextHop: '10.0.0.10',
            protocol: 'BGP',
            metric: 0,
            asPath: [65002],
            age: '2d 5h 30m',
          },
          {
            destination: '10.2.0.0/16',
            nextHop: '10.0.0.10',
            protocol: 'BGP',
            metric: 0,
            asPath: [65003],
            age: '1d 12h 15m',
          },
          {
            destination: '0.0.0.0/0',
            nextHop: '10.0.0.1',
            protocol: 'Static',
            metric: 1,
            asPath: [],
            age: '15d 8h 45m',
          },
        ]);
      }
    } catch (error) {
      console.error('Failed to fetch Aviatrix data:', error);
      setError('Failed to fetch Aviatrix data. Please check your connection.');
    } finally {
      setLoading(false);
    }
  };

  const handleCreateGateway = () => {
    setSelectedGateway(null);
    setDialogMode('create');
    setOpenDialog(true);
  };

  const handleEditGateway = (gateway: AviatrixGateway) => {
    setSelectedGateway(gateway);
    setDialogMode('edit');
    setOpenDialog(true);
  };

  const handleDeleteGateway = async (gatewayId: string) => {
    if (window.confirm('Are you sure you want to delete this gateway?')) {
      try {
        await fetch(`/api/v1/aviatrix/gateways/${gatewayId}`, {
          method: 'DELETE',
        });
        fetchAviatrixData();
      } catch (error) {
        console.error('Failed to delete gateway:', error);
      }
    }
  };

  const getStatusColor = (status: string) => {
    switch (status.toLowerCase()) {
      case 'active':
      case 'connected':
        return 'success';
      case 'pending':
      case 'connecting':
        return 'warning';
      case 'error':
      case 'disconnected':
        return 'error';
      case 'stopped':
        return 'default';
      default:
        return 'default';
    }
  };

  const getStatusIcon = (status: string) => {
    switch (status.toLowerCase()) {
      case 'active':
      case 'connected':
        return <CheckCircleIcon />;
      case 'pending':
      case 'connecting':
        return <WarningIcon />;
      case 'error':
      case 'disconnected':
        return <ErrorIcon />;
      default:
        return <WarningIcon />;
    }
  };

  const getGatewayTypeIcon = (type: string) => {
    switch (type.toLowerCase()) {
      case 'transit':
        return <RouterIcon />;
      case 'spoke':
        return <CloudIcon />;
      default:
        return <RouterIcon />;
    }
  };

  if (loading && gateways.length === 0) {
    return (
      <Box sx={{ p: 3 }}>
        <Typography variant="h4" gutterBottom>Aviatrix Integration</Typography>
        <LinearProgress />
      </Box>
    );
  }

  return (
    <Box>
      <Box sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', mb: 3 }}>
        <Typography variant="h4" component="h1">
          Aviatrix Integration
        </Typography>
        <Box>
          <Button
            variant="outlined"
            startIcon={<RefreshIcon />}
            onClick={fetchAviatrixData}
            sx={{ mr: 1 }}
          >
            Refresh
          </Button>
          <Button
            variant="contained"
            startIcon={<AddIcon />}
            onClick={handleCreateGateway}
          >
            Create Gateway
          </Button>
        </Box>
      </Box>

      {error && (
        <Alert severity="error" sx={{ mb: 3 }}>
          {error}
        </Alert>
      )}

      <Box sx={{ borderBottom: 1, borderColor: 'divider', mb: 3 }}>
        <Tabs value={tabValue} onChange={(e, newValue) => setTabValue(newValue)}>
          <Tab label="Gateways" />
          <Tab label="Connections" />
          <Tab label="Routes" />
          <Tab label="Analytics" />
        </Tabs>
      </Box>

      {/* Gateways Tab */}
      {tabValue === 0 && (
        <Grid container spacing={3}>
          {/* Summary Cards */}
          <Grid item xs={12} md={3}>
            <Card>
              <CardContent>
                <Box sx={{ display: 'flex', alignItems: 'center', mb: 2 }}>
                  <RouterIcon color="primary" sx={{ mr: 1 }} />
                  <Typography variant="h6">Total Gateways</Typography>
                </Box>
                <Typography variant="h4">{gateways.length}</Typography>
              </CardContent>
            </Card>
          </Grid>

          <Grid item xs={12} md={3}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>Active</Typography>
                <Typography variant="h4" color="success.main">
                  {gateways.filter(g => g.status === 'active').length}
                </Typography>
              </CardContent>
            </Card>
          </Grid>

          <Grid item xs={12} md={3}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>Transit</Typography>
                <Typography variant="h4" color="primary.main">
                  {gateways.filter(g => g.type === 'transit').length}
                </Typography>
              </CardContent>
            </Card>
          </Grid>

          <Grid item xs={12} md={3}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>Spoke</Typography>
                <Typography variant="h4" color="info.main">
                  {gateways.filter(g => g.type === 'spoke').length}
                </Typography>
              </CardContent>
            </Card>
          </Grid>

          {/* Gateways Table */}
          <Grid item xs={12}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  Gateways
                </Typography>
                <TableContainer component={Paper} sx={{ backgroundColor: 'transparent' }}>
                  <Table>
                    <TableHead>
                      <TableRow>
                        <TableCell>Name</TableCell>
                        <TableCell>Type</TableCell>
                        <TableCell>Cloud</TableCell>
                        <TableCell>Region</TableCell>
                        <TableCell>Status</TableCell>
                        <TableCell>Public IP</TableCell>
                        <TableCell>ASN</TableCell>
                        <TableCell>Actions</TableCell>
                      </TableRow>
                    </TableHead>
                    <TableBody>
                      {gateways.map((gateway) => (
                        <TableRow key={gateway.id}>
                          <TableCell>
                            <Box sx={{ display: 'flex', alignItems: 'center' }}>
                              <Box sx={{ mr: 1 }}>
                                {getGatewayTypeIcon(gateway.type)}
                              </Box>
                              {gateway.name}
                            </Box>
                          </TableCell>
                          <TableCell>
                            <Chip 
                              label={gateway.type} 
                              color={gateway.type === 'transit' ? 'primary' : 'secondary'}
                              size="small" 
                            />
                          </TableCell>
                          <TableCell>{gateway.cloudType}</TableCell>
                          <TableCell>{gateway.region}</TableCell>
                          <TableCell>
                            <Chip 
                              icon={getStatusIcon(gateway.status)}
                              label={gateway.status} 
                              color={getStatusColor(gateway.status)}
                              size="small"
                            />
                          </TableCell>
                          <TableCell>{gateway.publicIp}</TableCell>
                          <TableCell>{gateway.asn}</TableCell>
                          <TableCell>
                            <IconButton
                              size="small"
                              onClick={() => handleEditGateway(gateway)}
                            >
                              <EditIcon />
                            </IconButton>
                            <IconButton
                              size="small"
                              onClick={() => handleDeleteGateway(gateway.id)}
                              color="error"
                            >
                              <DeleteIcon />
                            </IconButton>
                          </TableCell>
                        </TableRow>
                      ))}
                    </TableBody>
                  </Table>
                </TableContainer>
              </CardContent>
            </Card>
          </Grid>
        </Grid>
      )}

      {/* Connections Tab */}
      {tabValue === 1 && (
        <Grid container spacing={3}>
          <Grid item xs={12}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  Gateway Connections
                </Typography>
                <TableContainer component={Paper} sx={{ backgroundColor: 'transparent' }}>
                  <Table>
                    <TableHead>
                      <TableRow>
                        <TableCell>Name</TableCell>
                        <TableCell>Source</TableCell>
                        <TableCell>Destination</TableCell>
                        <TableCell>Type</TableCell>
                        <TableCell>Status</TableCell>
                        <TableCell>Bandwidth</TableCell>
                        <TableCell>Latency</TableCell>
                        <TableCell>Packet Loss</TableCell>
                      </TableRow>
                    </TableHead>
                    <TableBody>
                      {connections.map((connection) => (
                        <TableRow key={connection.id}>
                          <TableCell>{connection.name}</TableCell>
                          <TableCell>{connection.sourceGateway}</TableCell>
                          <TableCell>{connection.destGateway}</TableCell>
                          <TableCell>
                            <Chip label={connection.type} size="small" />
                          </TableCell>
                          <TableCell>
                            <Chip 
                              icon={getStatusIcon(connection.status)}
                              label={connection.status} 
                              color={getStatusColor(connection.status)}
                              size="small"
                            />
                          </TableCell>
                          <TableCell>{connection.bandwidth} Mbps</TableCell>
                          <TableCell>{connection.latency} ms</TableCell>
                          <TableCell>{connection.packetLoss}%</TableCell>
                        </TableRow>
                      ))}
                    </TableBody>
                  </Table>
                </TableContainer>
              </CardContent>
            </Card>
          </Grid>
        </Grid>
      )}

      {/* Routes Tab */}
      {tabValue === 2 && (
        <Grid container spacing={3}>
          <Grid item xs={12}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  Routing Table
                </Typography>
                <TableContainer component={Paper} sx={{ backgroundColor: 'transparent' }}>
                  <Table>
                    <TableHead>
                      <TableRow>
                        <TableCell>Destination</TableCell>
                        <TableCell>Next Hop</TableCell>
                        <TableCell>Protocol</TableCell>
                        <TableCell>Metric</TableCell>
                        <TableCell>AS Path</TableCell>
                        <TableCell>Age</TableCell>
                      </TableRow>
                    </TableHead>
                    <TableBody>
                      {routes.map((route, index) => (
                        <TableRow key={index}>
                          <TableCell>{route.destination}</TableCell>
                          <TableCell>{route.nextHop}</TableCell>
                          <TableCell>
                            <Chip 
                              label={route.protocol} 
                              color={route.protocol === 'BGP' ? 'primary' : 'default'}
                              size="small" 
                            />
                          </TableCell>
                          <TableCell>{route.metric}</TableCell>
                          <TableCell>
                            {route.asPath.length > 0 ? route.asPath.join(' ') : '-'}
                          </TableCell>
                          <TableCell>{route.age}</TableCell>
                        </TableRow>
                      ))}
                    </TableBody>
                  </Table>
                </TableContainer>
              </CardContent>
            </Card>
          </Grid>
        </Grid>
      )}

      {/* Analytics Tab */}
      {tabValue === 3 && (
        <Grid container spacing={3}>
          <Grid item xs={12}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  Network Performance
                </Typography>
                <Box sx={{ height: 400 }}>
                  <ResponsiveContainer width="100%" height="100%">
                    <LineChart data={[
                      { time: '00:00', latency: 45, throughput: 2.1, packetLoss: 0.1 },
                      { time: '04:00', latency: 42, throughput: 1.8, packetLoss: 0.05 },
                      { time: '08:00', latency: 48, throughput: 2.5, packetLoss: 0.15 },
                      { time: '12:00', latency: 52, throughput: 2.8, packetLoss: 0.2 },
                      { time: '16:00', latency: 38, throughput: 2.2, packetLoss: 0.08 },
                      { time: '20:00', latency: 41, throughput: 2.0, packetLoss: 0.12 },
                    ]}>
                      <CartesianGrid strokeDasharray="3 3" />
                      <XAxis dataKey="time" />
                      <YAxis />
                      <Tooltip />
                      <Line type="monotone" dataKey="latency" stroke="#8884d8" strokeWidth={2} />
                      <Line type="monotone" dataKey="throughput" stroke="#82ca9d" strokeWidth={2} />
                      <Line type="monotone" dataKey="packetLoss" stroke="#ffc658" strokeWidth={2} />
                    </LineChart>
                  </ResponsiveContainer>
                </Box>
              </CardContent>
            </Card>
          </Grid>
        </Grid>
      )}

      {/* Create/Edit Gateway Dialog */}
      <Dialog open={openDialog} onClose={() => setOpenDialog(false)} maxWidth="md" fullWidth>
        <DialogTitle>
          {dialogMode === 'create' ? 'Create Gateway' : 'Edit Gateway'}
        </DialogTitle>
        <DialogContent>
          <Box sx={{ display: 'flex', flexDirection: 'column', gap: 2, mt: 1 }}>
            <TextField
              label="Gateway Name"
              defaultValue={selectedGateway?.name || ''}
              fullWidth
            />
            <TextField
              label="Gateway Type"
              select
              defaultValue={selectedGateway?.type || 'spoke'}
              fullWidth
            >
              <MenuItem value="transit">Transit Gateway</MenuItem>
              <MenuItem value="spoke">Spoke Gateway</MenuItem>
            </TextField>
            <TextField
              label="Cloud Type"
              select
              defaultValue={selectedGateway?.cloudType || 'AWS'}
              fullWidth
            >
              <MenuItem value="AWS">AWS</MenuItem>
              <MenuItem value="Azure">Azure</MenuItem>
              <MenuItem value="GCP">Google Cloud</MenuItem>
            </TextField>
            <TextField
              label="Region"
              defaultValue={selectedGateway?.region || ''}
              fullWidth
            />
            <TextField
              label="VPC ID"
              defaultValue={selectedGateway?.vpcId || ''}
              fullWidth
            />
            <TextField
              label="Subnet"
              defaultValue={selectedGateway?.subnet || ''}
              fullWidth
            />
            <TextField
              label="Gateway Size"
              defaultValue={selectedGateway?.gatewaySize || ''}
              fullWidth
            />
            <TextField
              label="ASN"
              type="number"
              defaultValue={selectedGateway?.asn || 65001}
              fullWidth
            />
          </Box>
        </DialogContent>
        <DialogActions>
          <Button onClick={() => setOpenDialog(false)}>Cancel</Button>
          <Button variant="contained">
            {dialogMode === 'create' ? 'Create' : 'Save'}
          </Button>
        </DialogActions>
      </Dialog>
    </Box>
  );
};

export default Aviatrix;
