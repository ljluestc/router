import React, { useState, useEffect } from 'react';
import {
  Box,
  Card,
  CardContent,
  Typography,
  Grid,
  Table,
  TableBody,
  TableCell,
  TableContainer,
  TableHead,
  TableRow,
  Paper,
  Chip,
  Button,
  Dialog,
  DialogTitle,
  DialogContent,
  DialogActions,
  TextField,
  FormControl,
  InputLabel,
  Select,
  MenuItem,
  Tabs,
  Tab,
  Alert,
  CircularProgress,
  IconButton,
  Tooltip,
} from '@mui/material';
import {
  Cloud,
  Router,
  Security,
  Speed,
  Storage,
  NetworkCheck,
  Add,
  Edit,
  Delete,
  Refresh,
  Visibility,
  Settings,
} from '@mui/icons-material';
import { LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip as RechartsTooltip, ResponsiveContainer, PieChart, Pie, Cell } from 'recharts';

interface VPCRoutingStats {
  vpc_id: string;
  subnet_count: number;
  nat_gateway_count: number;
  load_balancer_count: number;
  service_mesh_routes: number;
  total_routes: number;
  active_routes: number;
}

interface SubnetInfo {
  subnet_id: string;
  cidr: string;
  availability_zone: string;
  created_at: number;
}

interface NATGatewayInfo {
  nat_id: string;
  subnet_id: string;
  elastic_ip: string;
  state: string;
  created_at: number;
}

interface LoadBalancerInfo {
  lb_id: string;
  subnet_id: string;
  target_groups: string[];
  state: string;
  created_at: number;
}

interface ServiceMeshRoute {
  service_name: string;
  service_ip: string;
  endpoints: string[];
  state: string;
  created_at: number;
}

interface TabPanelProps {
  children?: React.ReactNode;
  index: number;
  value: number;
}

function TabPanel(props: TabPanelProps) {
  const { children, value, index, ...other } = props;

  return (
    <div
      role="tabpanel"
      hidden={value !== index}
      id={`cloudpods-tabpanel-${index}`}
      aria-labelledby={`cloudpods-tab-${index}`}
      {...other}
    >
      {value === index && (
        <Box sx={{ p: 3 }}>
          {children}
        </Box>
      )}
    </div>
  );
}

const CloudPods: React.FC = () => {
  const [tabValue, setTabValue] = useState(0);
  const [vpcStats, setVpcStats] = useState<VPCRoutingStats | null>(null);
  const [subnets, setSubnets] = useState<SubnetInfo[]>([]);
  const [natGateways, setNatGateways] = useState<NATGatewayInfo[]>([]);
  const [loadBalancers, setLoadBalancers] = useState<LoadBalancerInfo[]>([]);
  const [serviceMeshRoutes, setServiceMeshRoutes] = useState<ServiceMeshRoute[]>([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);
  const [openDialog, setOpenDialog] = useState(false);
  const [dialogType, setDialogType] = useState<'subnet' | 'nat' | 'lb' | 'service'>('subnet');
  const [formData, setFormData] = useState<any>({});

  const fetchData = async () => {
    try {
      setLoading(true);
      setError(null);

      const [vpcResponse, subnetsResponse, natResponse, lbResponse, serviceResponse] = await Promise.all([
        fetch('/api/cloudpods/vpc/stats'),
        fetch('/api/cloudpods/vpc/subnets'),
        fetch('/api/cloudpods/vpc/nat-gateways'),
        fetch('/api/cloudpods/vpc/load-balancers'),
        fetch('/api/cloudpods/vpc/service-mesh'),
      ]);

      if (!vpcResponse.ok || !subnetsResponse.ok || !natResponse.ok || !lbResponse.ok || !serviceResponse.ok) {
        throw new Error('Failed to fetch CloudPods data');
      }

      const [vpcData, subnetsData, natData, lbData, serviceData] = await Promise.all([
        vpcResponse.json(),
        subnetsResponse.json(),
        natResponse.json(),
        lbResponse.json(),
        serviceResponse.json(),
      ]);

      setVpcStats(vpcData);
      setSubnets(subnetsData);
      setNatGateways(natData);
      setLoadBalancers(lbData);
      setServiceMeshRoutes(serviceData);
    } catch (err) {
      setError(err instanceof Error ? err.message : 'An error occurred');
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    fetchData();
    const interval = setInterval(fetchData, 30000); // Refresh every 30 seconds
    return () => clearInterval(interval);
  }, []);

  const handleTabChange = (event: React.SyntheticEvent, newValue: number) => {
    setTabValue(newValue);
  };

  const handleOpenDialog = (type: 'subnet' | 'nat' | 'lb' | 'service') => {
    setDialogType(type);
    setFormData({});
    setOpenDialog(true);
  };

  const handleCloseDialog = () => {
    setOpenDialog(false);
    setFormData({});
  };

  const handleSubmit = async () => {
    try {
      const endpoint = `/api/cloudpods/vpc/${dialogType === 'subnet' ? 'subnets' : 
        dialogType === 'nat' ? 'nat-gateways' : 
        dialogType === 'lb' ? 'load-balancers' : 'service-mesh'}`;
      
      const response = await fetch(endpoint, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify(formData),
      });

      if (!response.ok) {
        throw new Error('Failed to create resource');
      }

      handleCloseDialog();
      fetchData();
    } catch (err) {
      setError(err instanceof Error ? err.message : 'An error occurred');
    }
  };

  const getStateColor = (state: string) => {
    switch (state.toLowerCase()) {
      case 'active':
      case 'available':
        return 'success';
      case 'pending':
        return 'warning';
      case 'failed':
      case 'error':
        return 'error';
      default:
        return 'default';
    }
  };

  const trafficData = [
    { name: '00:00', packets: 1200, bytes: 1500000 },
    { name: '04:00', packets: 800, bytes: 1000000 },
    { name: '08:00', packets: 2000, bytes: 2500000 },
    { name: '12:00', packets: 3500, bytes: 4000000 },
    { name: '16:00', packets: 2800, bytes: 3200000 },
    { name: '20:00', packets: 1800, bytes: 2200000 },
  ];

  const protocolData = [
    { name: 'HTTP', value: 45, color: '#8884d8' },
    { name: 'HTTPS', value: 30, color: '#82ca9d' },
    { name: 'TCP', value: 15, color: '#ffc658' },
    { name: 'UDP', value: 10, color: '#ff7300' },
  ];

  if (loading) {
    return (
      <Box display="flex" justifyContent="center" alignItems="center" minHeight="400px">
        <CircularProgress />
      </Box>
    );
  }

  return (
    <Box sx={{ width: '100%' }}>
      <Box sx={{ borderBottom: 1, borderColor: 'divider' }}>
        <Tabs value={tabValue} onChange={handleTabChange} aria-label="CloudPods tabs">
          <Tab label="Overview" icon={<Cloud />} />
          <Tab label="VPC Routing" icon={<Router />} />
          <Tab label="NAT Gateways" icon={<Security />} />
          <Tab label="Load Balancers" icon={<Speed />} />
          <Tab label="Service Mesh" icon={<NetworkCheck />} />
        </Tabs>
      </Box>

      {error && (
        <Alert severity="error" sx={{ mb: 2 }}>
          {error}
        </Alert>
      )}

      <TabPanel value={tabValue} index={0}>
        <Grid container spacing={3}>
          <Grid item xs={12} md={6}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  VPC Statistics
                </Typography>
                {vpcStats && (
                  <Grid container spacing={2}>
                    <Grid item xs={6}>
                      <Typography variant="body2" color="textSecondary">
                        Subnets
                      </Typography>
                      <Typography variant="h4">
                        {vpcStats.subnet_count}
                      </Typography>
                    </Grid>
                    <Grid item xs={6}>
                      <Typography variant="body2" color="textSecondary">
                        NAT Gateways
                      </Typography>
                      <Typography variant="h4">
                        {vpcStats.nat_gateway_count}
                      </Typography>
                    </Grid>
                    <Grid item xs={6}>
                      <Typography variant="body2" color="textSecondary">
                        Load Balancers
                      </Typography>
                      <Typography variant="h4">
                        {vpcStats.load_balancer_count}
                      </Typography>
                    </Grid>
                    <Grid item xs={6}>
                      <Typography variant="body2" color="textSecondary">
                        Service Routes
                      </Typography>
                      <Typography variant="h4">
                        {vpcStats.service_mesh_routes}
                      </Typography>
                    </Grid>
                  </Grid>
                )}
              </CardContent>
            </Card>
          </Grid>

          <Grid item xs={12} md={6}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  Traffic Distribution
                </Typography>
                <ResponsiveContainer width="100%" height={200}>
                  <PieChart>
                    <Pie
                      data={protocolData}
                      cx="50%"
                      cy="50%"
                      outerRadius={80}
                      dataKey="value"
                      label={({ name, percent }) => `${name} ${(percent * 100).toFixed(0)}%`}
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
                  Traffic Over Time
                </Typography>
                <ResponsiveContainer width="100%" height={300}>
                  <LineChart data={trafficData}>
                    <CartesianGrid strokeDasharray="3 3" />
                    <XAxis dataKey="name" />
                    <YAxis />
                    <RechartsTooltip />
                    <Line type="monotone" dataKey="packets" stroke="#8884d8" strokeWidth={2} />
                    <Line type="monotone" dataKey="bytes" stroke="#82ca9d" strokeWidth={2} />
                  </LineChart>
                </ResponsiveContainer>
              </CardContent>
            </Card>
          </Grid>
        </Grid>
      </TabPanel>

      <TabPanel value={tabValue} index={1}>
        <Box sx={{ mb: 2, display: 'flex', justifyContent: 'space-between', alignItems: 'center' }}>
          <Typography variant="h6">VPC Routing Table</Typography>
          <Box>
            <Button
              variant="contained"
              startIcon={<Add />}
              onClick={() => handleOpenDialog('subnet')}
              sx={{ mr: 1 }}
            >
              Add Subnet
            </Button>
            <IconButton onClick={fetchData}>
              <Refresh />
            </IconButton>
          </Box>
        </Box>

        <TableContainer component={Paper}>
          <Table>
            <TableHead>
              <TableRow>
                <TableCell>Subnet ID</TableCell>
                <TableCell>CIDR</TableCell>
                <TableCell>Availability Zone</TableCell>
                <TableCell>Created</TableCell>
                <TableCell>Actions</TableCell>
              </TableRow>
            </TableHead>
            <TableBody>
              {subnets.map((subnet) => (
                <TableRow key={subnet.subnet_id}>
                  <TableCell>{subnet.subnet_id}</TableCell>
                  <TableCell>{subnet.cidr}</TableCell>
                  <TableCell>{subnet.availability_zone}</TableCell>
                  <TableCell>
                    {new Date(subnet.created_at).toLocaleString()}
                  </TableCell>
                  <TableCell>
                    <Tooltip title="View Details">
                      <IconButton size="small">
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
      </TabPanel>

      <TabPanel value={tabValue} index={2}>
        <Box sx={{ mb: 2, display: 'flex', justifyContent: 'space-between', alignItems: 'center' }}>
          <Typography variant="h6">NAT Gateways</Typography>
          <Box>
            <Button
              variant="contained"
              startIcon={<Add />}
              onClick={() => handleOpenDialog('nat')}
              sx={{ mr: 1 }}
            >
              Add NAT Gateway
            </Button>
            <IconButton onClick={fetchData}>
              <Refresh />
            </IconButton>
          </Box>
        </Box>

        <TableContainer component={Paper}>
          <Table>
            <TableHead>
              <TableRow>
                <TableCell>NAT ID</TableCell>
                <TableCell>Subnet ID</TableCell>
                <TableCell>Elastic IP</TableCell>
                <TableCell>State</TableCell>
                <TableCell>Created</TableCell>
                <TableCell>Actions</TableCell>
              </TableRow>
            </TableHead>
            <TableBody>
              {natGateways.map((nat) => (
                <TableRow key={nat.nat_id}>
                  <TableCell>{nat.nat_id}</TableCell>
                  <TableCell>{nat.subnet_id}</TableCell>
                  <TableCell>{nat.elastic_ip}</TableCell>
                  <TableCell>
                    <Chip
                      label={nat.state}
                      color={getStateColor(nat.state) as any}
                      size="small"
                    />
                  </TableCell>
                  <TableCell>
                    {new Date(nat.created_at).toLocaleString()}
                  </TableCell>
                  <TableCell>
                    <Tooltip title="View Details">
                      <IconButton size="small">
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
      </TabPanel>

      <TabPanel value={tabValue} index={3}>
        <Box sx={{ mb: 2, display: 'flex', justifyContent: 'space-between', alignItems: 'center' }}>
          <Typography variant="h6">Load Balancers</Typography>
          <Box>
            <Button
              variant="contained"
              startIcon={<Add />}
              onClick={() => handleOpenDialog('lb')}
              sx={{ mr: 1 }}
            >
              Add Load Balancer
            </Button>
            <IconButton onClick={fetchData}>
              <Refresh />
            </IconButton>
          </Box>
        </Box>

        <TableContainer component={Paper}>
          <Table>
            <TableHead>
              <TableRow>
                <TableCell>LB ID</TableCell>
                <TableCell>Subnet ID</TableCell>
                <TableCell>Target Groups</TableCell>
                <TableCell>State</TableCell>
                <TableCell>Created</TableCell>
                <TableCell>Actions</TableCell>
              </TableRow>
            </TableHead>
            <TableBody>
              {loadBalancers.map((lb) => (
                <TableRow key={lb.lb_id}>
                  <TableCell>{lb.lb_id}</TableCell>
                  <TableCell>{lb.subnet_id}</TableCell>
                  <TableCell>
                    {lb.target_groups.map((tg, index) => (
                      <Chip key={index} label={tg} size="small" sx={{ mr: 0.5 }} />
                    ))}
                  </TableCell>
                  <TableCell>
                    <Chip
                      label={lb.state}
                      color={getStateColor(lb.state) as any}
                      size="small"
                    />
                  </TableCell>
                  <TableCell>
                    {new Date(lb.created_at).toLocaleString()}
                  </TableCell>
                  <TableCell>
                    <Tooltip title="View Details">
                      <IconButton size="small">
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
      </TabPanel>

      <TabPanel value={tabValue} index={4}>
        <Box sx={{ mb: 2, display: 'flex', justifyContent: 'space-between', alignItems: 'center' }}>
          <Typography variant="h6">Service Mesh Routes</Typography>
          <Box>
            <Button
              variant="contained"
              startIcon={<Add />}
              onClick={() => handleOpenDialog('service')}
              sx={{ mr: 1 }}
            >
              Add Service Route
            </Button>
            <IconButton onClick={fetchData}>
              <Refresh />
            </IconButton>
          </Box>
        </Box>

        <TableContainer component={Paper}>
          <Table>
            <TableHead>
              <TableRow>
                <TableCell>Service Name</TableCell>
                <TableCell>Service IP</TableCell>
                <TableCell>Endpoints</TableCell>
                <TableCell>State</TableCell>
                <TableCell>Created</TableCell>
                <TableCell>Actions</TableCell>
              </TableRow>
            </TableHead>
            <TableBody>
              {serviceMeshRoutes.map((route) => (
                <TableRow key={route.service_name}>
                  <TableCell>{route.service_name}</TableCell>
                  <TableCell>{route.service_ip}</TableCell>
                  <TableCell>
                    {route.endpoints.map((endpoint, index) => (
                      <Chip key={index} label={endpoint} size="small" sx={{ mr: 0.5 }} />
                    ))}
                  </TableCell>
                  <TableCell>
                    <Chip
                      label={route.state}
                      color={getStateColor(route.state) as any}
                      size="small"
                    />
                  </TableCell>
                  <TableCell>
                    {new Date(route.created_at).toLocaleString()}
                  </TableCell>
                  <TableCell>
                    <Tooltip title="View Details">
                      <IconButton size="small">
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
      </TabPanel>

      <Dialog open={openDialog} onClose={handleCloseDialog} maxWidth="sm" fullWidth>
        <DialogTitle>
          Add {dialogType === 'subnet' ? 'Subnet' : 
                dialogType === 'nat' ? 'NAT Gateway' : 
                dialogType === 'lb' ? 'Load Balancer' : 'Service Route'}
        </DialogTitle>
        <DialogContent>
          <Box sx={{ pt: 1 }}>
            {dialogType === 'subnet' && (
              <>
                <TextField
                  fullWidth
                  label="Subnet ID"
                  value={formData.subnet_id || ''}
                  onChange={(e) => setFormData({...formData, subnet_id: e.target.value})}
                  sx={{ mb: 2 }}
                />
                <TextField
                  fullWidth
                  label="CIDR"
                  value={formData.cidr || ''}
                  onChange={(e) => setFormData({...formData, cidr: e.target.value})}
                  sx={{ mb: 2 }}
                />
                <TextField
                  fullWidth
                  label="Availability Zone"
                  value={formData.availability_zone || ''}
                  onChange={(e) => setFormData({...formData, availability_zone: e.target.value})}
                />
              </>
            )}
            {dialogType === 'nat' && (
              <>
                <TextField
                  fullWidth
                  label="NAT ID"
                  value={formData.nat_id || ''}
                  onChange={(e) => setFormData({...formData, nat_id: e.target.value})}
                  sx={{ mb: 2 }}
                />
                <TextField
                  fullWidth
                  label="Subnet ID"
                  value={formData.subnet_id || ''}
                  onChange={(e) => setFormData({...formData, subnet_id: e.target.value})}
                  sx={{ mb: 2 }}
                />
                <TextField
                  fullWidth
                  label="Elastic IP"
                  value={formData.elastic_ip || ''}
                  onChange={(e) => setFormData({...formData, elastic_ip: e.target.value})}
                />
              </>
            )}
            {dialogType === 'lb' && (
              <>
                <TextField
                  fullWidth
                  label="Load Balancer ID"
                  value={formData.lb_id || ''}
                  onChange={(e) => setFormData({...formData, lb_id: e.target.value})}
                  sx={{ mb: 2 }}
                />
                <TextField
                  fullWidth
                  label="Subnet ID"
                  value={formData.subnet_id || ''}
                  onChange={(e) => setFormData({...formData, subnet_id: e.target.value})}
                  sx={{ mb: 2 }}
                />
                <TextField
                  fullWidth
                  label="Target Groups (comma-separated)"
                  value={formData.target_groups || ''}
                  onChange={(e) => setFormData({...formData, target_groups: e.target.value.split(',')})}
                />
              </>
            )}
            {dialogType === 'service' && (
              <>
                <TextField
                  fullWidth
                  label="Service Name"
                  value={formData.service_name || ''}
                  onChange={(e) => setFormData({...formData, service_name: e.target.value})}
                  sx={{ mb: 2 }}
                />
                <TextField
                  fullWidth
                  label="Service IP"
                  value={formData.service_ip || ''}
                  onChange={(e) => setFormData({...formData, service_ip: e.target.value})}
                  sx={{ mb: 2 }}
                />
                <TextField
                  fullWidth
                  label="Endpoints (comma-separated)"
                  value={formData.endpoints || ''}
                  onChange={(e) => setFormData({...formData, endpoints: e.target.value.split(',')})}
                />
              </>
            )}
          </Box>
        </DialogContent>
        <DialogActions>
          <Button onClick={handleCloseDialog}>Cancel</Button>
          <Button onClick={handleSubmit} variant="contained">
            Create
          </Button>
        </DialogActions>
      </Dialog>
    </Box>
  );
};

export default CloudPods;