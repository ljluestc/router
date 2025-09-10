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
  Switch,
  FormControlLabel,
  Slider,
  Alert,
  LinearProgress,
  Tabs,
  Tab,
} from '@mui/material';
import {
  Add as AddIcon,
  Edit as EditIcon,
  Delete as DeleteIcon,
  Refresh as RefreshIcon,
  Speed as SpeedIcon,
  Queue as QueueIcon,
  Settings as SettingsIcon,
  PlayArrow as PlayIcon,
  Stop as StopIcon,
} from '@mui/icons-material';
import { LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip, ResponsiveContainer, BarChart, Bar } from 'recharts';

interface TrafficPolicy {
  id: string;
  name: string;
  type: 'token-bucket' | 'wfq' | 'cbq' | 'htb';
  interface: string;
  bandwidth: number;
  burstSize: number;
  priority: number;
  status: 'active' | 'inactive' | 'error';
  packetsProcessed: number;
  bytesProcessed: number;
  drops: number;
  createdAt: string;
}

interface QueueConfig {
  id: string;
  name: string;
  weight: number;
  bandwidth: number;
  priority: number;
  packetsInQueue: number;
  maxQueueSize: number;
  drops: number;
}

interface TrafficStats {
  timestamp: string;
  throughput: number;
  latency: number;
  packetLoss: number;
  queueUtilization: number;
}

const TrafficShaping: React.FC = () => {
  const [policies, setPolicies] = useState<TrafficPolicy[]>([]);
  const [queues, setQueues] = useState<QueueConfig[]>([]);
  const [stats, setStats] = useState<TrafficStats[]>([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);
  const [tabValue, setTabValue] = useState(0);
  const [openDialog, setOpenDialog] = useState(false);
  const [selectedPolicy, setSelectedPolicy] = useState<TrafficPolicy | null>(null);
  const [dialogMode, setDialogMode] = useState<'create' | 'edit'>('create');

  useEffect(() => {
    fetchTrafficData();
    const interval = setInterval(fetchTrafficData, 5000);
    return () => clearInterval(interval);
  }, []);

  const fetchTrafficData = async () => {
    try {
      setLoading(true);
      setError(null);

      const [policiesRes, queuesRes, statsRes] = await Promise.all([
        fetch('/api/v1/traffic-shaping/policies'),
        fetch('/api/v1/traffic-shaping/queues'),
        fetch('/api/v1/traffic-shaping/stats'),
      ]);

      if (policiesRes.ok) {
        const policiesData = await policiesRes.json();
        setPolicies(policiesData.policies || []);
      } else {
        // Mock data for demo
        setPolicies([
          {
            id: '1',
            name: 'High-Priority-Voice',
            type: 'token-bucket',
            interface: 'GigabitEthernet0/0/0',
            bandwidth: 100,
            burstSize: 1000,
            priority: 1,
            status: 'active',
            packetsProcessed: 1234567,
            bytesProcessed: 987654321,
            drops: 0,
            createdAt: '2024-01-15T10:30:00Z',
          },
          {
            id: '2',
            name: 'Bulk-Data-Transfer',
            type: 'wfq',
            interface: 'GigabitEthernet0/0/1',
            bandwidth: 500,
            burstSize: 5000,
            priority: 3,
            status: 'active',
            packetsProcessed: 2345678,
            bytesProcessed: 1876543210,
            drops: 45,
            createdAt: '2024-01-16T14:20:00Z',
          },
          {
            id: '3',
            name: 'Web-Traffic',
            type: 'cbq',
            interface: 'GigabitEthernet0/0/2',
            bandwidth: 200,
            burstSize: 2000,
            priority: 2,
            status: 'inactive',
            packetsProcessed: 0,
            bytesProcessed: 0,
            drops: 0,
            createdAt: '2024-01-17T09:15:00Z',
          },
        ]);
      }

      if (queuesRes.ok) {
        const queuesData = await queuesRes.json();
        setQueues(queuesData.queues || []);
      } else {
        // Mock queues data
        setQueues([
          {
            id: '1',
            name: 'Voice-Queue',
            weight: 10,
            bandwidth: 100,
            priority: 1,
            packetsInQueue: 5,
            maxQueueSize: 100,
            drops: 0,
          },
          {
            id: '2',
            name: 'Video-Queue',
            weight: 8,
            bandwidth: 200,
            priority: 2,
            packetsInQueue: 12,
            maxQueueSize: 200,
            drops: 2,
          },
          {
            id: '3',
            name: 'Data-Queue',
            weight: 5,
            bandwidth: 300,
            priority: 3,
            packetsInQueue: 25,
            maxQueueSize: 500,
            drops: 8,
          },
          {
            id: '4',
            name: 'Bulk-Queue',
            weight: 2,
            bandwidth: 400,
            priority: 4,
            packetsInQueue: 45,
            maxQueueSize: 1000,
            drops: 15,
          },
        ]);
      }

      if (statsRes.ok) {
        const statsData = await statsRes.json();
        setStats(statsData.stats || []);
      } else {
        // Mock stats data
        const now = new Date();
        const mockStats = Array.from({ length: 24 }, (_, i) => ({
          timestamp: new Date(now.getTime() - (23 - i) * 60 * 60 * 1000).toLocaleTimeString(),
          throughput: Math.random() * 5 + 1,
          latency: Math.random() * 50 + 10,
          packetLoss: Math.random() * 2,
          queueUtilization: Math.random() * 100,
        }));
        setStats(mockStats);
      }
    } catch (error) {
      console.error('Failed to fetch traffic data:', error);
      setError('Failed to fetch traffic shaping data. Please check your connection.');
    } finally {
      setLoading(false);
    }
  };

  const handleCreatePolicy = () => {
    setSelectedPolicy(null);
    setDialogMode('create');
    setOpenDialog(true);
  };

  const handleEditPolicy = (policy: TrafficPolicy) => {
    setSelectedPolicy(policy);
    setDialogMode('edit');
    setOpenDialog(true);
  };

  const handleDeletePolicy = async (policyId: string) => {
    if (window.confirm('Are you sure you want to delete this policy?')) {
      try {
        await fetch(`/api/v1/traffic-shaping/policies/${policyId}`, {
          method: 'DELETE',
        });
        fetchTrafficData();
      } catch (error) {
        console.error('Failed to delete policy:', error);
      }
    }
  };

  const handleTogglePolicy = async (policyId: string, status: string) => {
    try {
      await fetch(`/api/v1/traffic-shaping/policies/${policyId}`, {
        method: 'PATCH',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({
          status: status === 'active' ? 'inactive' : 'active',
        }),
      });
      fetchTrafficData();
    } catch (error) {
      console.error('Failed to toggle policy:', error);
    }
  };

  const getStatusColor = (status: string) => {
    switch (status.toLowerCase()) {
      case 'active':
        return 'success';
      case 'inactive':
        return 'default';
      case 'error':
        return 'error';
      default:
        return 'default';
    }
  };

  const getTypeIcon = (type: string) => {
    switch (type.toLowerCase()) {
      case 'token-bucket':
        return <SpeedIcon />;
      case 'wfq':
      case 'cbq':
      case 'htb':
        return <QueueIcon />;
      default:
        return <SettingsIcon />;
    }
  };

  if (loading && policies.length === 0) {
    return (
      <Box sx={{ p: 3 }}>
        <Typography variant="h4" gutterBottom>Traffic Shaping</Typography>
        <LinearProgress />
      </Box>
    );
  }

  return (
    <Box>
      <Box sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', mb: 3 }}>
        <Typography variant="h4" component="h1">
          Traffic Shaping
        </Typography>
        <Box>
          <Button
            variant="outlined"
            startIcon={<RefreshIcon />}
            onClick={fetchTrafficData}
            sx={{ mr: 1 }}
          >
            Refresh
          </Button>
          <Button
            variant="contained"
            startIcon={<AddIcon />}
            onClick={handleCreatePolicy}
          >
            New Policy
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
          <Tab label="Policies" />
          <Tab label="Queues" />
          <Tab label="Statistics" />
          <Tab label="Performance" />
        </Tabs>
      </Box>

      {/* Policies Tab */}
      {tabValue === 0 && (
        <Grid container spacing={3}>
          {/* Summary Cards */}
          <Grid item xs={12} md={3}>
            <Card>
              <CardContent>
                <Box sx={{ display: 'flex', alignItems: 'center', mb: 2 }}>
                  <SpeedIcon color="primary" sx={{ mr: 1 }} />
                  <Typography variant="h6">Total Policies</Typography>
                </Box>
                <Typography variant="h4">{policies.length}</Typography>
              </CardContent>
            </Card>
          </Grid>

          <Grid item xs={12} md={3}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>Active</Typography>
                <Typography variant="h4" color="success.main">
                  {policies.filter(p => p.status === 'active').length}
                </Typography>
              </CardContent>
            </Card>
          </Grid>

          <Grid item xs={12} md={3}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>Total Bandwidth</Typography>
                <Typography variant="h4" color="primary.main">
                  {policies.reduce((sum, p) => sum + p.bandwidth, 0)} Mbps
                </Typography>
              </CardContent>
            </Card>
          </Grid>

          <Grid item xs={12} md={3}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>Total Drops</Typography>
                <Typography variant="h4" color="error.main">
                  {policies.reduce((sum, p) => sum + p.drops, 0)}
                </Typography>
              </CardContent>
            </Card>
          </Grid>

          {/* Policies Table */}
          <Grid item xs={12}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  Traffic Policies
                </Typography>
                <TableContainer component={Paper} sx={{ backgroundColor: 'transparent' }}>
                  <Table>
                    <TableHead>
                      <TableRow>
                        <TableCell>Name</TableCell>
                        <TableCell>Type</TableCell>
                        <TableCell>Interface</TableCell>
                        <TableCell>Bandwidth</TableCell>
                        <TableCell>Priority</TableCell>
                        <TableCell>Status</TableCell>
                        <TableCell>Packets Processed</TableCell>
                        <TableCell>Drops</TableCell>
                        <TableCell>Actions</TableCell>
                      </TableRow>
                    </TableHead>
                    <TableBody>
                      {policies.map((policy) => (
                        <TableRow key={policy.id}>
                          <TableCell>
                            <Box sx={{ display: 'flex', alignItems: 'center' }}>
                              <Box sx={{ mr: 1 }}>
                                {getTypeIcon(policy.type)}
                              </Box>
                              {policy.name}
                            </Box>
                          </TableCell>
                          <TableCell>
                            <Chip 
                              label={policy.type.toUpperCase()} 
                              color="primary"
                              size="small" 
                            />
                          </TableCell>
                          <TableCell>{policy.interface}</TableCell>
                          <TableCell>{policy.bandwidth} Mbps</TableCell>
                          <TableCell>
                            <Chip 
                              label={`P${policy.priority}`} 
                              color={policy.priority === 1 ? 'error' : policy.priority === 2 ? 'warning' : 'default'}
                              size="small" 
                            />
                          </TableCell>
                          <TableCell>
                            <Chip 
                              label={policy.status} 
                              color={getStatusColor(policy.status)}
                              size="small"
                            />
                          </TableCell>
                          <TableCell>{policy.packetsProcessed.toLocaleString()}</TableCell>
                          <TableCell>
                            <Chip 
                              label={policy.drops} 
                              color={policy.drops > 0 ? 'error' : 'success'}
                              size="small" 
                            />
                          </TableCell>
                          <TableCell>
                            <IconButton
                              size="small"
                              onClick={() => handleTogglePolicy(policy.id, policy.status)}
                            >
                              {policy.status === 'active' ? <StopIcon /> : <PlayIcon />}
                            </IconButton>
                            <IconButton
                              size="small"
                              onClick={() => handleEditPolicy(policy)}
                            >
                              <EditIcon />
                            </IconButton>
                            <IconButton
                              size="small"
                              onClick={() => handleDeletePolicy(policy.id)}
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

      {/* Queues Tab */}
      {tabValue === 1 && (
        <Grid container spacing={3}>
          <Grid item xs={12}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  Queue Configuration
                </Typography>
                <TableContainer component={Paper} sx={{ backgroundColor: 'transparent' }}>
                  <Table>
                    <TableHead>
                      <TableRow>
                        <TableCell>Queue Name</TableCell>
                        <TableCell>Weight</TableCell>
                        <TableCell>Bandwidth</TableCell>
                        <TableCell>Priority</TableCell>
                        <TableCell>Packets in Queue</TableCell>
                        <TableCell>Max Queue Size</TableCell>
                        <TableCell>Utilization</TableCell>
                        <TableCell>Drops</TableCell>
                      </TableRow>
                    </TableHead>
                    <TableBody>
                      {queues.map((queue) => (
                        <TableRow key={queue.id}>
                          <TableCell>
                            <Box sx={{ display: 'flex', alignItems: 'center' }}>
                              <QueueIcon sx={{ mr: 1 }} />
                              {queue.name}
                            </Box>
                          </TableCell>
                          <TableCell>{queue.weight}</TableCell>
                          <TableCell>{queue.bandwidth} Mbps</TableCell>
                          <TableCell>
                            <Chip 
                              label={`P${queue.priority}`} 
                              color={queue.priority === 1 ? 'error' : queue.priority === 2 ? 'warning' : 'default'}
                              size="small" 
                            />
                          </TableCell>
                          <TableCell>{queue.packetsInQueue}</TableCell>
                          <TableCell>{queue.maxQueueSize}</TableCell>
                          <TableCell>
                            <Box sx={{ display: 'flex', alignItems: 'center' }}>
                              <Typography variant="body2" sx={{ mr: 1 }}>
                                {Math.round((queue.packetsInQueue / queue.maxQueueSize) * 100)}%
                              </Typography>
                              <LinearProgress
                                variant="determinate"
                                value={(queue.packetsInQueue / queue.maxQueueSize) * 100}
                                sx={{ width: 100, height: 8, borderRadius: 4 }}
                              />
                            </Box>
                          </TableCell>
                          <TableCell>
                            <Chip 
                              label={queue.drops} 
                              color={queue.drops > 0 ? 'error' : 'success'}
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
      )}

      {/* Statistics Tab */}
      {tabValue === 2 && (
        <Grid container spacing={3}>
          <Grid item xs={12} md={6}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  Bandwidth Distribution
                </Typography>
                <Box sx={{ height: 300 }}>
                  <ResponsiveContainer width="100%" height="100%">
                    <BarChart data={policies.map(p => ({ name: p.name, bandwidth: p.bandwidth }))}>
                      <CartesianGrid strokeDasharray="3 3" />
                      <XAxis dataKey="name" />
                      <YAxis />
                      <Tooltip />
                      <Bar dataKey="bandwidth" fill="#8884d8" />
                    </BarChart>
                  </ResponsiveContainer>
                </Box>
              </CardContent>
            </Card>
          </Grid>

          <Grid item xs={12} md={6}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  Queue Utilization
                </Typography>
                <Box sx={{ height: 300 }}>
                  <ResponsiveContainer width="100%" height="100%">
                    <BarChart data={queues.map(q => ({ 
                      name: q.name, 
                      utilization: Math.round((q.packetsInQueue / q.maxQueueSize) * 100) 
                    }))}>
                      <CartesianGrid strokeDasharray="3 3" />
                      <XAxis dataKey="name" />
                      <YAxis />
                      <Tooltip />
                      <Bar dataKey="utilization" fill="#82ca9d" />
                    </BarChart>
                  </ResponsiveContainer>
                </Box>
              </CardContent>
            </Card>
          </Grid>
        </Grid>
      )}

      {/* Performance Tab */}
      {tabValue === 3 && (
        <Grid container spacing={3}>
          <Grid item xs={12}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  Traffic Performance Over Time
                </Typography>
                <Box sx={{ height: 400 }}>
                  <ResponsiveContainer width="100%" height="100%">
                    <LineChart data={stats}>
                      <CartesianGrid strokeDasharray="3 3" />
                      <XAxis dataKey="timestamp" />
                      <YAxis />
                      <Tooltip />
                      <Line type="monotone" dataKey="throughput" stroke="#8884d8" strokeWidth={2} />
                      <Line type="monotone" dataKey="latency" stroke="#82ca9d" strokeWidth={2} />
                      <Line type="monotone" dataKey="packetLoss" stroke="#ffc658" strokeWidth={2} />
                      <Line type="monotone" dataKey="queueUtilization" stroke="#ff7300" strokeWidth={2} />
                    </LineChart>
                  </ResponsiveContainer>
                </Box>
              </CardContent>
            </Card>
          </Grid>
        </Grid>
      )}

      {/* Create/Edit Policy Dialog */}
      <Dialog open={openDialog} onClose={() => setOpenDialog(false)} maxWidth="md" fullWidth>
        <DialogTitle>
          {dialogMode === 'create' ? 'Create Policy' : 'Edit Policy'}
        </DialogTitle>
        <DialogContent>
          <Box sx={{ display: 'flex', flexDirection: 'column', gap: 2, mt: 1 }}>
            <TextField
              label="Policy Name"
              defaultValue={selectedPolicy?.name || ''}
              fullWidth
            />
            <TextField
              label="Policy Type"
              select
              defaultValue={selectedPolicy?.type || 'token-bucket'}
              fullWidth
            >
              <MenuItem value="token-bucket">Token Bucket</MenuItem>
              <MenuItem value="wfq">Weighted Fair Queuing</MenuItem>
              <MenuItem value="cbq">Class-Based Queuing</MenuItem>
              <MenuItem value="htb">Hierarchical Token Bucket</MenuItem>
            </TextField>
            <TextField
              label="Interface"
              defaultValue={selectedPolicy?.interface || ''}
              fullWidth
            />
            <Box>
              <Typography variant="subtitle2" gutterBottom>
                Bandwidth (Mbps)
              </Typography>
              <Slider
                defaultValue={selectedPolicy?.bandwidth || 100}
                min={1}
                max={1000}
                step={1}
                valueLabelDisplay="auto"
                valueLabelFormat={(value) => `${value} Mbps`}
              />
            </Box>
            <Box>
              <Typography variant="subtitle2" gutterBottom>
                Burst Size (packets)
              </Typography>
              <Slider
                defaultValue={selectedPolicy?.burstSize || 1000}
                min={100}
                max={10000}
                step={100}
                valueLabelDisplay="auto"
                valueLabelFormat={(value) => `${value} packets`}
              />
            </Box>
            <TextField
              label="Priority"
              type="number"
              defaultValue={selectedPolicy?.priority || 1}
              inputProps={{ min: 1, max: 10 }}
              fullWidth
            />
            <FormControlLabel
              control={<Switch defaultChecked={selectedPolicy?.status === 'active'} />}
              label="Active"
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

export default TrafficShaping;
