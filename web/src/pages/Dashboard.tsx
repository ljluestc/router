import React, { useState, useEffect } from 'react';
import {
  Box,
  Grid,
  Card,
  CardContent,
  Typography,
  Button,
  Chip,
  LinearProgress,
  Table,
  TableBody,
  TableCell,
  TableContainer,
  TableHead,
  TableRow,
  Paper,
  IconButton,
  Tooltip,
} from '@mui/material';
import {
  Refresh as RefreshIcon,
  PlayArrow as PlayIcon,
  Pause as PauseIcon,
  Stop as StopIcon,
  TrendingUp as TrendingUpIcon,
  NetworkCheck as NetworkCheckIcon,
  Speed as SpeedIcon,
  Security as SecurityIcon,
} from '@mui/icons-material';

interface SystemStatus {
  status: 'running' | 'stopped' | 'starting' | 'stopping';
  uptime: string;
  protocols: {
    bgp: boolean;
    ospf: boolean;
    isis: boolean;
  };
  interfaces: number;
  routes: number;
  neighbors: number;
}

interface PerformanceMetrics {
  packetsPerSecond: number;
  throughput: number;
  latency: number;
  packetLoss: number;
  cpuUsage: number;
  memoryUsage: number;
}

interface RecentActivity {
  id: string;
  timestamp: string;
  type: 'route_update' | 'neighbor_change' | 'interface_change' | 'error';
  message: string;
  severity: 'info' | 'warning' | 'error';
}

const Dashboard: React.FC = () => {
  const [systemStatus, setSystemStatus] = useState<SystemStatus>({
    status: 'running',
    uptime: '2d 14h 32m',
    protocols: {
      bgp: true,
      ospf: true,
      isis: false,
    },
    interfaces: 3,
    routes: 1247,
    neighbors: 5,
  });

  const [metrics, setMetrics] = useState<PerformanceMetrics>({
    packetsPerSecond: 15420,
    throughput: 125.6,
    latency: 12.3,
    packetLoss: 0.02,
    cpuUsage: 45,
    memoryUsage: 67,
  });

  const [recentActivity, setRecentActivity] = useState<RecentActivity[]>([
    {
      id: '1',
      timestamp: '2024-01-15 14:32:15',
      type: 'route_update',
      message: 'BGP route 10.0.0.0/8 advertised to neighbor 192.168.1.2',
      severity: 'info',
    },
    {
      id: '2',
      timestamp: '2024-01-15 14:31:42',
      type: 'neighbor_change',
      message: 'OSPF neighbor 10.0.1.2 state changed to Full',
      severity: 'info',
    },
    {
      id: '3',
      timestamp: '2024-01-15 14:30:18',
      type: 'interface_change',
      message: 'Interface eth1 status changed to UP',
      severity: 'info',
    },
    {
      id: '4',
      timestamp: '2024-01-15 14:29:55',
      type: 'error',
      message: 'Packet loss detected on interface eth2 (0.5%)',
      severity: 'warning',
    },
  ]);

  const [isRunning, setIsRunning] = useState(true);

  useEffect(() => {
    // Simulate real-time updates
    const interval = setInterval(() => {
      setMetrics(prev => ({
        ...prev,
        packetsPerSecond: prev.packetsPerSecond + Math.floor(Math.random() * 1000 - 500),
        throughput: prev.throughput + (Math.random() * 2 - 1),
        latency: prev.latency + (Math.random() * 0.5 - 0.25),
        packetLoss: Math.max(0, prev.packetLoss + (Math.random() * 0.01 - 0.005)),
        cpuUsage: Math.max(0, Math.min(100, prev.cpuUsage + (Math.random() * 4 - 2))),
        memoryUsage: Math.max(0, Math.min(100, prev.memoryUsage + (Math.random() * 2 - 1))),
      }));
    }, 2000);

    return () => clearInterval(interval);
  }, []);

  const handleStart = () => {
    setIsRunning(true);
    setSystemStatus(prev => ({ ...prev, status: 'running' }));
  };

  const handleStop = () => {
    setIsRunning(false);
    setSystemStatus(prev => ({ ...prev, status: 'stopped' }));
  };

  const handleRefresh = () => {
    // Simulate refresh
    setSystemStatus(prev => ({
      ...prev,
      routes: prev.routes + Math.floor(Math.random() * 10 - 5),
      neighbors: prev.neighbors + Math.floor(Math.random() * 2 - 1),
    }));
  };

  const getStatusColor = (status: string) => {
    switch (status) {
      case 'running':
        return 'success';
      case 'stopped':
        return 'error';
      case 'starting':
      case 'stopping':
        return 'warning';
      default:
        return 'default';
    }
  };

  const getSeverityColor = (severity: string) => {
    switch (severity) {
      case 'info':
        return 'info';
      case 'warning':
        return 'warning';
      case 'error':
        return 'error';
      default:
        return 'default';
    }
  };

  return (
    <Box>
      <Box sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', mb: 3 }}>
        <Typography variant="h4" component="h1">
          Dashboard
        </Typography>
        <Box>
          <Button
            variant="outlined"
            startIcon={<RefreshIcon />}
            onClick={handleRefresh}
            sx={{ mr: 1 }}
          >
            Refresh
          </Button>
          {isRunning ? (
            <Button
              variant="contained"
              color="error"
              startIcon={<StopIcon />}
              onClick={handleStop}
            >
              Stop
            </Button>
          ) : (
            <Button
              variant="contained"
              color="success"
              startIcon={<PlayIcon />}
              onClick={handleStart}
            >
              Start
            </Button>
          )}
        </Box>
      </Box>

      <Grid container spacing={3}>
        {/* System Status Card */}
        <Grid item xs={12} md={6}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>
                System Status
              </Typography>
              <Box sx={{ display: 'flex', alignItems: 'center', mb: 2 }}>
                <Chip
                  label={systemStatus.status.toUpperCase()}
                  color={getStatusColor(systemStatus.status)}
                  sx={{ mr: 2 }}
                />
                <Typography variant="body2" color="text.secondary">
                  Uptime: {systemStatus.uptime}
                </Typography>
              </Box>
              <Grid container spacing={2}>
                <Grid item xs={6}>
                  <Typography variant="body2" color="text.secondary">
                    Interfaces
                  </Typography>
                  <Typography variant="h6">{systemStatus.interfaces}</Typography>
                </Grid>
                <Grid item xs={6}>
                  <Typography variant="body2" color="text.secondary">
                    Routes
                  </Typography>
                  <Typography variant="h6">{systemStatus.routes}</Typography>
                </Grid>
                <Grid item xs={6}>
                  <Typography variant="body2" color="text.secondary">
                    Neighbors
                  </Typography>
                  <Typography variant="h6">{systemStatus.neighbors}</Typography>
                </Grid>
                <Grid item xs={6}>
                  <Typography variant="body2" color="text.secondary">
                    Protocols
                  </Typography>
                  <Box>
                    {Object.entries(systemStatus.protocols).map(([protocol, enabled]) => (
                      <Chip
                        key={protocol}
                        label={protocol.toUpperCase()}
                        size="small"
                        color={enabled ? 'success' : 'default'}
                        sx={{ mr: 0.5, mb: 0.5 }}
                      />
                    ))}
                  </Box>
                </Grid>
              </Grid>
            </CardContent>
          </Card>
        </Grid>

        {/* Performance Metrics */}
        <Grid item xs={12} md={6}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>
                Performance Metrics
              </Typography>
              <Grid container spacing={2}>
                <Grid item xs={6}>
                  <Box sx={{ display: 'flex', alignItems: 'center', mb: 1 }}>
                    <TrendingUpIcon sx={{ mr: 1, color: 'primary.main' }} />
                    <Typography variant="body2" color="text.secondary">
                      Packets/sec
                    </Typography>
                  </Box>
                  <Typography variant="h6">
                    {metrics.packetsPerSecond.toLocaleString()}
                  </Typography>
                </Grid>
                <Grid item xs={6}>
                  <Box sx={{ display: 'flex', alignItems: 'center', mb: 1 }}>
                    <SpeedIcon sx={{ mr: 1, color: 'primary.main' }} />
                    <Typography variant="body2" color="text.secondary">
                      Throughput
                    </Typography>
                  </Box>
                  <Typography variant="h6">
                    {metrics.throughput.toFixed(1)} Mbps
                  </Typography>
                </Grid>
                <Grid item xs={6}>
                  <Box sx={{ display: 'flex', alignItems: 'center', mb: 1 }}>
                    <NetworkCheckIcon sx={{ mr: 1, color: 'primary.main' }} />
                    <Typography variant="body2" color="text.secondary">
                      Latency
                    </Typography>
                  </Box>
                  <Typography variant="h6">
                    {metrics.latency.toFixed(1)} ms
                  </Typography>
                </Grid>
                <Grid item xs={6}>
                  <Box sx={{ display: 'flex', alignItems: 'center', mb: 1 }}>
                    <SecurityIcon sx={{ mr: 1, color: 'primary.main' }} />
                    <Typography variant="body2" color="text.secondary">
                      Packet Loss
                    </Typography>
                  </Box>
                  <Typography variant="h6">
                    {metrics.packetLoss.toFixed(2)}%
                  </Typography>
                </Grid>
              </Grid>
            </CardContent>
          </Card>
        </Grid>

        {/* Resource Usage */}
        <Grid item xs={12} md={6}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>
                Resource Usage
              </Typography>
              <Box sx={{ mb: 2 }}>
                <Box sx={{ display: 'flex', justifyContent: 'space-between', mb: 1 }}>
                  <Typography variant="body2">CPU Usage</Typography>
                  <Typography variant="body2">{metrics.cpuUsage}%</Typography>
                </Box>
                <LinearProgress
                  variant="determinate"
                  value={metrics.cpuUsage}
                  sx={{ height: 8, borderRadius: 4 }}
                />
              </Box>
              <Box>
                <Box sx={{ display: 'flex', justifyContent: 'space-between', mb: 1 }}>
                  <Typography variant="body2">Memory Usage</Typography>
                  <Typography variant="body2">{metrics.memoryUsage}%</Typography>
                </Box>
                <LinearProgress
                  variant="determinate"
                  value={metrics.memoryUsage}
                  color="secondary"
                  sx={{ height: 8, borderRadius: 4 }}
                />
              </Box>
            </CardContent>
          </Card>
        </Grid>

        {/* Quick Actions */}
        <Grid item xs={12} md={6}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>
                Quick Actions
              </Typography>
              <Box sx={{ display: 'flex', flexDirection: 'column', gap: 1 }}>
                <Button variant="outlined" fullWidth>
                  Run Test Scenario
                </Button>
                <Button variant="outlined" fullWidth>
                  Configure Traffic Shaping
                </Button>
                <Button variant="outlined" fullWidth>
                  Apply Network Impairments
                </Button>
                <Button variant="outlined" fullWidth>
                  View Analytics
                </Button>
              </Box>
            </CardContent>
          </Card>
        </Grid>

        {/* Recent Activity */}
        <Grid item xs={12}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>
                Recent Activity
              </Typography>
              <TableContainer component={Paper} sx={{ backgroundColor: 'transparent' }}>
                <Table size="small">
                  <TableHead>
                    <TableRow>
                      <TableCell>Timestamp</TableCell>
                      <TableCell>Type</TableCell>
                      <TableCell>Message</TableCell>
                      <TableCell>Severity</TableCell>
                      <TableCell>Actions</TableCell>
                    </TableRow>
                  </TableHead>
                  <TableBody>
                    {recentActivity.map((activity) => (
                      <TableRow key={activity.id}>
                        <TableCell>{activity.timestamp}</TableCell>
                        <TableCell>
                          <Chip
                            label={activity.type.replace('_', ' ').toUpperCase()}
                            size="small"
                            variant="outlined"
                          />
                        </TableCell>
                        <TableCell>{activity.message}</TableCell>
                        <TableCell>
                          <Chip
                            label={activity.severity.toUpperCase()}
                            size="small"
                            color={getSeverityColor(activity.severity)}
                          />
                        </TableCell>
                        <TableCell>
                          <Tooltip title="View Details">
                            <IconButton size="small">
                              <RefreshIcon />
                            </IconButton>
                          </Tooltip>
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
    </Box>
  );
};

export default Dashboard;
