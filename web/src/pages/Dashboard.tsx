import React, { useState, useEffect } from 'react';
import {
  Box,
  Grid,
  Card,
  CardContent,
  Typography,
  LinearProgress,
  Chip,
  IconButton,
  Tooltip,
  Alert,
  Button,
  Dialog,
  DialogTitle,
  DialogContent,
  DialogActions,
  List,
  ListItem,
  ListItemText,
  ListItemIcon,
  Divider,
  Paper,
  Table,
  TableBody,
  TableCell,
  TableContainer,
  TableHead,
  TableRow,
  Avatar,
  Badge,
} from '@mui/material';
import {
  Refresh as RefreshIcon,
  TrendingUp as TrendingUpIcon,
  TrendingDown as TrendingDownIcon,
  Warning as WarningIcon,
  CheckCircle as CheckCircleIcon,
  Error as ErrorIcon,
  Info as InfoIcon,
  Cloud as CloudIcon,
  Router as RouterIcon,
  Security as SecurityIcon,
  Speed as SpeedIcon,
  Storage as StorageIcon,
  Timeline as TimelineIcon,
  Assessment as AssessmentIcon,
  NetworkCheck as NetworkCheckIcon,
  VpnKey as VpnKeyIcon,
  BugReport as BugReportIcon,
  Settings as SettingsIcon,
} from '@mui/icons-material';
import {
  LineChart,
  Line,
  AreaChart,
  Area,
  BarChart,
  Bar,
  PieChart,
  Pie,
  Cell,
  XAxis,
  YAxis,
  CartesianGrid,
  Tooltip as RechartsTooltip,
  Legend,
  ResponsiveContainer,
} from 'recharts';

// Mock data
const mockSystemStats = {
  totalGateways: 12,
  activeGateways: 10,
  totalConnections: 45,
  activeConnections: 42,
  totalRoutes: 1250,
  learnedRoutes: 1180,
  totalBandwidth: 10000, // Mbps
  usedBandwidth: 7500,
  latency: 15, // ms
  packetLoss: 0.02, // %
  uptime: 99.9, // %
};

const mockRecentActivity = [
  {
    id: 1,
    type: 'gateway',
    action: 'Gateway started',
    target: 'transit-gw-us-west-1',
    timestamp: new Date(Date.now() - 5 * 60 * 1000),
    status: 'success',
  },
  {
    id: 2,
    type: 'connection',
    action: 'Connection established',
    target: 'transit-gw-us-west-1 ↔ spoke-gw-us-east-1',
    timestamp: new Date(Date.now() - 15 * 60 * 1000),
    status: 'success',
  },
  {
    id: 3,
    type: 'route',
    action: 'Route learned',
    target: '10.0.0.0/8 via 192.168.1.1',
    timestamp: new Date(Date.now() - 30 * 60 * 1000),
    status: 'info',
  },
  {
    id: 4,
    type: 'alert',
    action: 'High latency detected',
    target: 'transit-gw-eu-west-1',
    timestamp: new Date(Date.now() - 45 * 60 * 1000),
    status: 'warning',
  },
  {
    id: 5,
    type: 'maintenance',
    action: 'Scheduled maintenance completed',
    target: 'System',
    timestamp: new Date(Date.now() - 2 * 60 * 60 * 1000),
    status: 'success',
  },
];

const mockTrafficData = [
  { time: '00:00', packets: 1200, bytes: 1500000, latency: 12 },
  { time: '04:00', packets: 800, bytes: 1000000, latency: 15 },
  { time: '08:00', packets: 2500, bytes: 3200000, latency: 18 },
  { time: '12:00', packets: 3200, bytes: 4100000, latency: 14 },
  { time: '16:00', packets: 2800, bytes: 3600000, latency: 16 },
  { time: '20:00', packets: 1900, bytes: 2400000, latency: 13 },
];

const mockGatewayData = [
  { name: 'transit-gw-us-west-1', status: 'active', cpu: 45, memory: 62, latency: 12 },
  { name: 'transit-gw-us-east-1', status: 'active', cpu: 38, memory: 58, latency: 15 },
  { name: 'spoke-gw-eu-west-1', status: 'active', cpu: 52, memory: 71, latency: 18 },
  { name: 'spoke-gw-asia-pacific', status: 'maintenance', cpu: 0, memory: 0, latency: 0 },
];

const mockProtocolData = [
  { name: 'BGP', routes: 850, neighbors: 8, status: 'active' },
  { name: 'OSPF', routes: 320, neighbors: 12, status: 'active' },
  { name: 'ISIS', routes: 180, neighbors: 6, status: 'active' },
  { name: 'Static', routes: 50, neighbors: 0, status: 'active' },
];

const mockCloudData = [
  { name: 'AWS', value: 45, color: '#FF9900' },
  { name: 'Azure', value: 30, color: '#0078D4' },
  { name: 'GCP', value: 20, color: '#4285F4' },
  { name: 'OCI', value: 5, color: '#FF0000' },
];

const Dashboard: React.FC = () => {
  const [loading, setLoading] = useState(true);
  const [systemStats, setSystemStats] = useState(mockSystemStats);
  const [recentActivity, setRecentActivity] = useState(mockRecentActivity);
  const [showActivityDialog, setShowActivityDialog] = useState(false);

  useEffect(() => {
    // Simulate loading
    const timer = setTimeout(() => {
      setLoading(false);
    }, 1000);

    return () => clearTimeout(timer);
  }, []);

  const handleRefresh = () => {
    setLoading(true);
    setTimeout(() => {
      setSystemStats(mockSystemStats);
      setRecentActivity(mockRecentActivity);
      setLoading(false);
    }, 1000);
  };

  const getStatusColor = (status: string) => {
    switch (status) {
      case 'active':
      case 'success':
        return 'success';
      case 'warning':
        return 'warning';
      case 'error':
        return 'error';
      case 'info':
        return 'info';
      default:
        return 'default';
    }
  };

  const getStatusIcon = (status: string) => {
    switch (status) {
      case 'active':
      case 'success':
        return <CheckCircleIcon color="success" />;
      case 'warning':
        return <WarningIcon color="warning" />;
      case 'error':
        return <ErrorIcon color="error" />;
      case 'info':
        return <InfoIcon color="info" />;
      default:
        return <InfoIcon />;
    }
  };

  if (loading) {
    return (
      <Box sx={{ width: '100%', mt: 2 }}>
        <LinearProgress />
        <Typography variant="h6" sx={{ mt: 2, textAlign: 'center' }}>
          Loading dashboard...
        </Typography>
      </Box>
    );
  }

  return (
    <Box>
      {/* Header */}
      <Box sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', mb: 3 }}>
        <Typography variant="h4" component="h1">
          Dashboard
        </Typography>
        <Box>
          <Tooltip title="Refresh Data">
            <IconButton onClick={handleRefresh} disabled={loading}>
              <RefreshIcon />
            </IconButton>
          </Tooltip>
        </Box>
      </Box>

      {/* System Status Alert */}
      <Alert severity="success" sx={{ mb: 3 }}>
        All systems are operational. Last health check: {new Date().toLocaleTimeString()}
      </Alert>

      {/* Key Metrics Cards */}
      <Grid container spacing={3} sx={{ mb: 3 }}>
        <Grid item xs={12} sm={6} md={3}>
          <Card>
            <CardContent>
              <Box sx={{ display: 'flex', alignItems: 'center', mb: 2 }}>
                <RouterIcon color="primary" sx={{ mr: 1 }} />
                <Typography variant="h6">Gateways</Typography>
              </Box>
              <Typography variant="h4" color="primary">
                {systemStats.activeGateways}/{systemStats.totalGateways}
              </Typography>
              <Typography variant="body2" color="text.secondary">
                Active Gateways
              </Typography>
              <Box sx={{ display: 'flex', alignItems: 'center', mt: 1 }}>
                <TrendingUpIcon color="success" sx={{ fontSize: 16, mr: 0.5 }} />
                <Typography variant="caption" color="success.main">
                  +2 this week
                </Typography>
              </Box>
            </CardContent>
          </Card>
        </Grid>

        <Grid item xs={12} sm={6} md={3}>
          <Card>
            <CardContent>
              <Box sx={{ display: 'flex', alignItems: 'center', mb: 2 }}>
                <NetworkCheckIcon color="primary" sx={{ mr: 1 }} />
                <Typography variant="h6">Connections</Typography>
              </Box>
              <Typography variant="h4" color="primary">
                {systemStats.activeConnections}
              </Typography>
              <Typography variant="body2" color="text.secondary">
                Active Connections
              </Typography>
              <Box sx={{ display: 'flex', alignItems: 'center', mt: 1 }}>
                <TrendingUpIcon color="success" sx={{ fontSize: 16, mr: 0.5 }} />
                <Typography variant="caption" color="success.main">
                  +5 today
                </Typography>
              </Box>
            </CardContent>
          </Card>
        </Grid>

        <Grid item xs={12} sm={6} md={3}>
          <Card>
            <CardContent>
              <Box sx={{ display: 'flex', alignItems: 'center', mb: 2 }}>
                <TimelineIcon color="primary" sx={{ mr: 1 }} />
                <Typography variant="h6">Routes</Typography>
              </Box>
              <Typography variant="h4" color="primary">
                {systemStats.learnedRoutes.toLocaleString()}
              </Typography>
              <Typography variant="body2" color="text.secondary">
                Learned Routes
              </Typography>
              <Box sx={{ display: 'flex', alignItems: 'center', mt: 1 }}>
                <TrendingUpIcon color="success" sx={{ fontSize: 16, mr: 0.5 }} />
                <Typography variant="caption" color="success.main">
                  +{systemStats.learnedRoutes - 1100} today
                </Typography>
              </Box>
            </CardContent>
          </Card>
        </Grid>

        <Grid item xs={12} sm={6} md={3}>
          <Card>
            <CardContent>
              <Box sx={{ display: 'flex', alignItems: 'center', mb: 2 }}>
                <SpeedIcon color="primary" sx={{ mr: 1 }} />
                <Typography variant="h6">Latency</Typography>
              </Box>
              <Typography variant="h4" color="primary">
                {systemStats.latency}ms
              </Typography>
              <Typography variant="body2" color="text.secondary">
                Average Latency
              </Typography>
              <Box sx={{ display: 'flex', alignItems: 'center', mt: 1 }}>
                <TrendingDownIcon color="success" sx={{ fontSize: 16, mr: 0.5 }} />
                <Typography variant="caption" color="success.main">
                  -2ms from yesterday
                </Typography>
              </Box>
            </CardContent>
          </Card>
        </Grid>
      </Grid>

      {/* Charts Row */}
      <Grid container spacing={3} sx={{ mb: 3 }}>
        {/* Traffic Chart */}
        <Grid item xs={12} md={8}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>
                Network Traffic (24h)
              </Typography>
              <ResponsiveContainer width="100%" height={300}>
                <AreaChart data={mockTrafficData}>
                  <CartesianGrid strokeDasharray="3 3" />
                  <XAxis dataKey="time" />
                  <YAxis yAxisId="left" />
                  <YAxis yAxisId="right" orientation="right" />
                  <RechartsTooltip />
                  <Legend />
                  <Area
                    yAxisId="left"
                    type="monotone"
                    dataKey="packets"
                    stackId="1"
                    stroke="#1976d2"
                    fill="#1976d2"
                    fillOpacity={0.6}
                    name="Packets/sec"
                  />
                  <Area
                    yAxisId="right"
                    type="monotone"
                    dataKey="latency"
                    stroke="#dc004e"
                    fill="#dc004e"
                    fillOpacity={0.6}
                    name="Latency (ms)"
                  />
                </AreaChart>
              </ResponsiveContainer>
            </CardContent>
          </Card>
        </Grid>

        {/* Cloud Distribution */}
        <Grid item xs={12} md={4}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>
                Cloud Distribution
              </Typography>
              <ResponsiveContainer width="100%" height={300}>
                <PieChart>
                  <Pie
                    data={mockCloudData}
                    cx="50%"
                    cy="50%"
                    labelLine={false}
                    label={({ name, percent }) => `${name} ${(percent * 100).toFixed(0)}%`}
                    outerRadius={80}
                    fill="#8884d8"
                    dataKey="value"
                  >
                    {mockCloudData.map((entry, index) => (
                      <Cell key={`cell-${index}`} fill={entry.color} />
                    ))}
                  </Pie>
                  <RechartsTooltip />
                </PieChart>
              </ResponsiveContainer>
            </CardContent>
          </Card>
        </Grid>
      </Grid>

      {/* Tables Row */}
      <Grid container spacing={3}>
        {/* Gateway Status */}
        <Grid item xs={12} md={6}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>
                Gateway Status
              </Typography>
              <TableContainer>
                <Table size="small">
                  <TableHead>
                    <TableRow>
                      <TableCell>Gateway</TableCell>
                      <TableCell>Status</TableCell>
                      <TableCell>CPU</TableCell>
                      <TableCell>Memory</TableCell>
                      <TableCell>Latency</TableCell>
                    </TableRow>
                  </TableHead>
                  <TableBody>
                    {mockGatewayData.map((gateway) => (
                      <TableRow key={gateway.name}>
                        <TableCell>
                          <Box sx={{ display: 'flex', alignItems: 'center' }}>
                            <Avatar sx={{ width: 24, height: 24, mr: 1, bgcolor: 'primary.main' }}>
                              <RouterIcon sx={{ fontSize: 16 }} />
                            </Avatar>
                            {gateway.name}
                          </Box>
                        </TableCell>
                        <TableCell>
                          <Chip
                            label={gateway.status}
                            color={getStatusColor(gateway.status)}
                            size="small"
                          />
                        </TableCell>
                        <TableCell>
                          <Box sx={{ display: 'flex', alignItems: 'center' }}>
                            <LinearProgress
                              variant="determinate"
                              value={gateway.cpu}
                              sx={{ width: 60, mr: 1 }}
                            />
                            <Typography variant="caption">{gateway.cpu}%</Typography>
                          </Box>
                        </TableCell>
                        <TableCell>
                          <Box sx={{ display: 'flex', alignItems: 'center' }}>
                            <LinearProgress
                              variant="determinate"
                              value={gateway.memory}
                              sx={{ width: 60, mr: 1 }}
                            />
                            <Typography variant="caption">{gateway.memory}%</Typography>
                          </Box>
                        </TableCell>
                        <TableCell>
                          {gateway.latency > 0 ? `${gateway.latency}ms` : '-'}
                        </TableCell>
                      </TableRow>
                    ))}
                  </TableBody>
                </Table>
              </TableContainer>
            </CardContent>
          </Card>
        </Grid>

        {/* Protocol Status */}
        <Grid item xs={12} md={6}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>
                Protocol Status
              </Typography>
              <TableContainer>
                <Table size="small">
                  <TableHead>
                    <TableRow>
                      <TableCell>Protocol</TableCell>
                      <TableCell>Routes</TableCell>
                      <TableCell>Neighbors</TableCell>
                      <TableCell>Status</TableCell>
                    </TableRow>
                  </TableHead>
                  <TableBody>
                    {mockProtocolData.map((protocol) => (
                      <TableRow key={protocol.name}>
                        <TableCell>
                          <Box sx={{ display: 'flex', alignItems: 'center' }}>
                            <Avatar sx={{ width: 24, height: 24, mr: 1, bgcolor: 'secondary.main' }}>
                              <SecurityIcon sx={{ fontSize: 16 }} />
                            </Avatar>
                            {protocol.name}
                          </Box>
                        </TableCell>
                        <TableCell>{protocol.routes}</TableCell>
                        <TableCell>{protocol.neighbors}</TableCell>
                        <TableCell>
                          <Chip
                            label={protocol.status}
                            color={getStatusColor(protocol.status)}
                            size="small"
                          />
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

      {/* Recent Activity */}
      <Card sx={{ mt: 3 }}>
        <CardContent>
          <Box sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', mb: 2 }}>
            <Typography variant="h6">
              Recent Activity
            </Typography>
            <Button
              size="small"
              onClick={() => setShowActivityDialog(true)}
            >
              View All
            </Button>
          </Box>
          <List>
            {recentActivity.slice(0, 5).map((activity) => (
              <React.Fragment key={activity.id}>
                <ListItem>
                  <ListItemIcon>
                    {getStatusIcon(activity.status)}
                  </ListItemIcon>
                  <ListItemText
                    primary={activity.action}
                    secondary={`${activity.target} • ${activity.timestamp.toLocaleString()}`}
                  />
                </ListItem>
                <Divider />
              </React.Fragment>
            ))}
          </List>
        </CardContent>
      </Card>

      {/* Activity Dialog */}
      <Dialog
        open={showActivityDialog}
        onClose={() => setShowActivityDialog(false)}
        maxWidth="md"
        fullWidth
      >
        <DialogTitle>All Activity</DialogTitle>
        <DialogContent>
          <List>
            {recentActivity.map((activity) => (
              <React.Fragment key={activity.id}>
                <ListItem>
                  <ListItemIcon>
                    {getStatusIcon(activity.status)}
                  </ListItemIcon>
                  <ListItemText
                    primary={activity.action}
                    secondary={`${activity.target} • ${activity.timestamp.toLocaleString()}`}
                  />
                </ListItem>
                <Divider />
              </React.Fragment>
            ))}
          </List>
        </DialogContent>
        <DialogActions>
          <Button onClick={() => setShowActivityDialog(false)}>Close</Button>
        </DialogActions>
      </Dialog>
    </Box>
  );
};

export default Dashboard;
