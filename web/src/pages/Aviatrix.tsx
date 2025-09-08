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
  LinearProgress,
} from '@mui/material';
import {
  Add as AddIcon,
  Edit as EditIcon,
  Delete as DeleteIcon,
  Refresh as RefreshIcon,
  Security as SecurityIcon,
  Cloud as CloudIcon,
  Router as RouterIcon,
  VpnKey as VpnKeyIcon,
  Speed as SpeedIcon,
  NetworkCheck as NetworkCheckIcon,
} from '@mui/icons-material';

interface AviatrixGateway {
  id: string;
  name: string;
  type: 'transit' | 'spoke' | 'edge' | 'gateway';
  cloud: 'aws' | 'azure' | 'gcp' | 'oci';
  region: string;
  status: 'running' | 'stopped' | 'pending' | 'error';
  vpc_id: string;
  subnet_id: string;
  public_ip: string;
  private_ip: string;
  created_at: string;
  tags: string[];
}

interface AviatrixConnection {
  id: string;
  name: string;
  source_gateway: string;
  destination_gateway: string;
  type: 'transit' | 'spoke' | 'vpn' | 'peering';
  status: 'active' | 'inactive' | 'pending' | 'error';
  bandwidth: number;
  latency: number;
  created_at: string;
}

interface AviatrixSecurity {
  id: string;
  name: string;
  type: 'firewall' | 'nat' | 'vpn' | 'encryption';
  status: 'active' | 'inactive' | 'pending';
  rules: number;
  hits: number;
  created_at: string;
}

const Aviatrix: React.FC = () => {
  const [gateways, setGateways] = useState<AviatrixGateway[]>([]);
  const [connections, setConnections] = useState<AviatrixConnection[]>([]);
  const [security, setSecurity] = useState<AviatrixSecurity[]>([]);
  const [loading, setLoading] = useState(true);
  const [openDialog, setOpenDialog] = useState(false);
  const [selectedGateway, setSelectedGateway] = useState<AviatrixGateway | null>(null);
  const [dialogMode, setDialogMode] = useState<'create' | 'edit'>('create');
  const [activeTab, setActiveTab] = useState(0);

  useEffect(() => {
    fetchData();
  }, []);

  const fetchData = async () => {
    try {
      setLoading(true);
      // Simulate API calls
      await new Promise(resolve => setTimeout(resolve, 1000));
      
      setGateways([
        {
          id: '1',
          name: 'transit-gateway-us-west-2',
          type: 'transit',
          cloud: 'aws',
          region: 'us-west-2',
          status: 'running',
          vpc_id: 'vpc-12345678',
          subnet_id: 'subnet-12345678',
          public_ip: '54.123.45.67',
          private_ip: '10.0.1.10',
          created_at: '2024-01-10T10:00:00Z',
          tags: ['production', 'transit'],
        },
        {
          id: '2',
          name: 'spoke-gateway-us-east-1',
          type: 'spoke',
          cloud: 'aws',
          region: 'us-east-1',
          status: 'running',
          vpc_id: 'vpc-87654321',
          subnet_id: 'subnet-87654321',
          public_ip: '54.234.56.78',
          private_ip: '10.1.1.10',
          created_at: '2024-01-12T14:30:00Z',
          tags: ['production', 'spoke'],
        },
        {
          id: '3',
          name: 'edge-gateway-azure-westus2',
          type: 'edge',
          cloud: 'azure',
          region: 'West US 2',
          status: 'pending',
          vpc_id: 'vnet-azure-123',
          subnet_id: 'subnet-azure-123',
          public_ip: '20.123.45.67',
          private_ip: '172.16.1.10',
          created_at: '2024-01-14T09:15:00Z',
          tags: ['staging', 'edge'],
        },
      ]);

      setConnections([
        {
          id: '1',
          name: 'transit-to-spoke-connection',
          source_gateway: 'transit-gateway-us-west-2',
          destination_gateway: 'spoke-gateway-us-east-1',
          type: 'transit',
          status: 'active',
          bandwidth: 1000,
          latency: 25.5,
          created_at: '2024-01-10T10:30:00Z',
        },
        {
          id: '2',
          name: 'vpn-connection-azure',
          source_gateway: 'transit-gateway-us-west-2',
          destination_gateway: 'edge-gateway-azure-westus2',
          type: 'vpn',
          status: 'pending',
          bandwidth: 100,
          latency: 45.2,
          created_at: '2024-01-14T09:30:00Z',
        },
      ]);

      setSecurity([
        {
          id: '1',
          name: 'firewall-rules-production',
          type: 'firewall',
          status: 'active',
          rules: 25,
          hits: 15420,
          created_at: '2024-01-10T10:00:00Z',
        },
        {
          id: '2',
          name: 'nat-rules-spoke-gateways',
          type: 'nat',
          status: 'active',
          rules: 8,
          hits: 8932,
          created_at: '2024-01-12T14:30:00Z',
        },
        {
          id: '3',
          name: 'vpn-encryption-policy',
          type: 'encryption',
          status: 'active',
          rules: 3,
          hits: 2156,
          created_at: '2024-01-14T09:15:00Z',
        },
      ]);
    } catch (error) {
      console.error('Failed to fetch Aviatrix data:', error);
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
        setGateways(prev => prev.filter(g => g.id !== gatewayId));
      } catch (error) {
        console.error('Failed to delete gateway:', error);
      }
    }
  };

  const getStatusColor = (status: string) => {
    switch (status.toLowerCase()) {
      case 'running':
      case 'active':
        return 'success';
      case 'stopped':
      case 'inactive':
        return 'error';
      case 'pending':
        return 'warning';
      case 'error':
        return 'error';
      default:
        return 'default';
    }
  };

  const getCloudIcon = (cloud: string) => {
    switch (cloud.toLowerCase()) {
      case 'aws':
        return '🟠';
      case 'azure':
        return '🔵';
      case 'gcp':
        return '🔴';
      case 'oci':
        return '🟣';
      default:
        return '☁️';
    }
  };

  const getTypeIcon = (type: string) => {
    switch (type.toLowerCase()) {
      case 'transit':
        return <RouterIcon />;
      case 'spoke':
        return <CloudIcon />;
      case 'edge':
        return <NetworkCheckIcon />;
      case 'gateway':
        return <VpnKeyIcon />;
      default:
        return <CloudIcon />;
    }
  };

  if (loading) {
    return (
      <Box sx={{ width: '100%', mt: 2 }}>
        <LinearProgress />
        <Typography variant="h6" sx={{ mt: 2, textAlign: 'center' }}>
          Loading Aviatrix data...
        </Typography>
      </Box>
    );
  }

  return (
    <Box>
      <Box sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', mb: 3 }}>
        <Typography variant="h4" component="h1">
          Aviatrix Cloud Networking
        </Typography>
        <Box>
          <Button
            variant="outlined"
            startIcon={<RefreshIcon />}
            onClick={fetchData}
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

      <Box sx={{ borderBottom: 1, borderColor: 'divider', mb: 3 }}>
        <Tabs value={activeTab} onChange={(e, newValue) => setActiveTab(newValue)}>
          <Tab label="Gateways" />
          <Tab label="Connections" />
          <Tab label="Security" />
          <Tab label="Analytics" />
        </Tabs>
      </Box>

      {activeTab === 0 && (
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
                <Typography variant="h6" gutterBottom>Running</Typography>
                <Typography variant="h4" color="success.main">
                  {gateways.filter(g => g.status === 'running').length}
                </Typography>
              </CardContent>
            </Card>
          </Grid>

          <Grid item xs={12} md={3}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>Pending</Typography>
                <Typography variant="h4" color="warning.main">
                  {gateways.filter(g => g.status === 'pending').length}
                </Typography>
              </CardContent>
            </Card>
          </Grid>

          <Grid item xs={12} md={3}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>Clouds</Typography>
                <Typography variant="h4">
                  {new Set(gateways.map(g => g.cloud)).size}
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
                        <TableCell>Created</TableCell>
                        <TableCell>Actions</TableCell>
                      </TableRow>
                    </TableHead>
                    <TableBody>
                      {gateways.map((gateway) => (
                        <TableRow key={gateway.id}>
                          <TableCell>
                            <Box sx={{ display: 'flex', alignItems: 'center' }}>
                              {getTypeIcon(gateway.type)}
                              <Typography variant="body2" sx={{ ml: 1 }}>
                                {gateway.name}
                              </Typography>
                            </Box>
                          </TableCell>
                          <TableCell>
                            <Chip label={gateway.type} size="small" />
                          </TableCell>
                          <TableCell>
                            <Box sx={{ display: 'flex', alignItems: 'center' }}>
                              <Typography variant="body2" sx={{ mr: 1 }}>
                                {getCloudIcon(gateway.cloud)}
                              </Typography>
                              {gateway.cloud.toUpperCase()}
                            </Box>
                          </TableCell>
                          <TableCell>{gateway.region}</TableCell>
                          <TableCell>
                            <Chip 
                              label={gateway.status} 
                              color={getStatusColor(gateway.status)}
                              size="small"
                            />
                          </TableCell>
                          <TableCell>{gateway.public_ip}</TableCell>
                          <TableCell>
                            {new Date(gateway.created_at).toLocaleDateString()}
                          </TableCell>
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

      {activeTab === 1 && (
        <Grid container spacing={3}>
          <Grid item xs={12}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  Connections
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
                      </TableRow>
                    </TableHead>
                    <TableBody>
                      {connections.map((connection) => (
                        <TableRow key={connection.id}>
                          <TableCell>{connection.name}</TableCell>
                          <TableCell>{connection.source_gateway}</TableCell>
                          <TableCell>{connection.destination_gateway}</TableCell>
                          <TableCell>
                            <Chip label={connection.type} size="small" />
                          </TableCell>
                          <TableCell>
                            <Chip 
                              label={connection.status} 
                              color={getStatusColor(connection.status)}
                              size="small"
                            />
                          </TableCell>
                          <TableCell>{connection.bandwidth} Mbps</TableCell>
                          <TableCell>{connection.latency} ms</TableCell>
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

      {activeTab === 2 && (
        <Grid container spacing={3}>
          <Grid item xs={12}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  Security Policies
                </Typography>
                <TableContainer component={Paper} sx={{ backgroundColor: 'transparent' }}>
                  <Table>
                    <TableHead>
                      <TableRow>
                        <TableCell>Name</TableCell>
                        <TableCell>Type</TableCell>
                        <TableCell>Status</TableCell>
                        <TableCell>Rules</TableCell>
                        <TableCell>Hits</TableCell>
                        <TableCell>Created</TableCell>
                      </TableRow>
                    </TableHead>
                    <TableBody>
                      {security.map((policy) => (
                        <TableRow key={policy.id}>
                          <TableCell>{policy.name}</TableCell>
                          <TableCell>
                            <Chip label={policy.type} size="small" />
                          </TableCell>
                          <TableCell>
                            <Chip 
                              label={policy.status} 
                              color={getStatusColor(policy.status)}
                              size="small"
                            />
                          </TableCell>
                          <TableCell>{policy.rules}</TableCell>
                          <TableCell>{policy.hits.toLocaleString()}</TableCell>
                          <TableCell>
                            {new Date(policy.created_at).toLocaleDateString()}
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

      {activeTab === 3 && (
        <Grid container spacing={3}>
          <Grid item xs={12} md={6}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  Network Performance
                </Typography>
                <Box sx={{ textAlign: 'center', py: 4 }}>
                  <Typography variant="h4" color="primary">
                    99.9%
                  </Typography>
                  <Typography variant="body2" color="text.secondary">
                    Uptime
                  </Typography>
                </Box>
              </CardContent>
            </Card>
          </Grid>
          <Grid item xs={12} md={6}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  Security Score
                </Typography>
                <Box sx={{ textAlign: 'center', py: 4 }}>
                  <Typography variant="h4" color="success.main">
                    95
                  </Typography>
                  <Typography variant="body2" color="text.secondary">
                    Security Score
                  </Typography>
                </Box>
              </CardContent>
            </Card>
          </Grid>
        </Grid>
      )}

      {/* Create/Edit Dialog */}
      <Dialog open={openDialog} onClose={() => setOpenDialog(false)} maxWidth="sm" fullWidth>
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
              label="Type"
              select
              defaultValue={selectedGateway?.type || 'spoke'}
              fullWidth
            >
              <MenuItem value="transit">Transit</MenuItem>
              <MenuItem value="spoke">Spoke</MenuItem>
              <MenuItem value="edge">Edge</MenuItem>
              <MenuItem value="gateway">Gateway</MenuItem>
            </TextField>
            <TextField
              label="Cloud Provider"
              select
              defaultValue={selectedGateway?.cloud || 'aws'}
              fullWidth
            >
              <MenuItem value="aws">AWS</MenuItem>
              <MenuItem value="azure">Azure</MenuItem>
              <MenuItem value="gcp">GCP</MenuItem>
              <MenuItem value="oci">OCI</MenuItem>
            </TextField>
            <TextField
              label="Region"
              defaultValue={selectedGateway?.region || ''}
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
