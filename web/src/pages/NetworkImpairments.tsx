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
  NetworkCheck as NetworkIcon,
  Speed as SpeedIcon,
  Settings as SettingsIcon,
  PlayArrow as PlayIcon,
  Stop as StopIcon,
  Warning as WarningIcon,
} from '@mui/icons-material';
import { LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip, ResponsiveContainer, AreaChart, Area } from 'recharts';

interface ImpairmentRule {
  id: string;
  name: string;
  type: 'delay' | 'loss' | 'duplicate' | 'reorder' | 'corrupt' | 'bandwidth';
  interface: string;
  enabled: boolean;
  parameters: {
    delay?: number;
    loss?: number;
    duplicate?: number;
    reorder?: number;
    corrupt?: number;
    bandwidth?: number;
    jitter?: number;
  };
  duration: number;
  packetsAffected: number;
  bytesAffected: number;
  createdAt: string;
}

interface ImpairmentStats {
  timestamp: string;
  delay: number;
  loss: number;
  duplicate: number;
  reorder: number;
  corrupt: number;
  bandwidth: number;
}

const NetworkImpairments: React.FC = () => {
  const [rules, setRules] = useState<ImpairmentRule[]>([]);
  const [stats, setStats] = useState<ImpairmentStats[]>([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);
  const [tabValue, setTabValue] = useState(0);
  const [openDialog, setOpenDialog] = useState(false);
  const [selectedRule, setSelectedRule] = useState<ImpairmentRule | null>(null);
  const [dialogMode, setDialogMode] = useState<'create' | 'edit'>('create');

  useEffect(() => {
    fetchImpairmentData();
    const interval = setInterval(fetchImpairmentData, 5000);
    return () => clearInterval(interval);
  }, []);

  const fetchImpairmentData = async () => {
    try {
      setLoading(true);
      setError(null);

      const [rulesRes, statsRes] = await Promise.all([
        fetch('/api/v1/network-impairments/rules'),
        fetch('/api/v1/network-impairments/stats'),
      ]);

      if (rulesRes.ok) {
        const rulesData = await rulesRes.json();
        setRules(rulesData.rules || []);
      } else {
        // Mock data for demo
        setRules([
          {
            id: '1',
            name: 'High Latency Simulation',
            type: 'delay',
            interface: 'GigabitEthernet0/0/0',
            enabled: true,
            parameters: {
              delay: 100,
              jitter: 20,
            },
            duration: 300,
            packetsAffected: 1234567,
            bytesAffected: 987654321,
            createdAt: '2024-01-15T10:30:00Z',
          },
          {
            id: '2',
            name: 'Packet Loss Test',
            type: 'loss',
            interface: 'GigabitEthernet0/0/1',
            enabled: true,
            parameters: {
              loss: 5,
            },
            duration: 180,
            packetsAffected: 2345678,
            bytesAffected: 1876543210,
            createdAt: '2024-01-16T14:20:00Z',
          },
          {
            id: '3',
            name: 'Bandwidth Limitation',
            type: 'bandwidth',
            interface: 'GigabitEthernet0/0/2',
            enabled: false,
            parameters: {
              bandwidth: 50,
            },
            duration: 600,
            packetsAffected: 0,
            bytesAffected: 0,
            createdAt: '2024-01-17T09:15:00Z',
          },
          {
            id: '4',
            name: 'Packet Duplication',
            type: 'duplicate',
            interface: 'GigabitEthernet0/0/3',
            enabled: true,
            parameters: {
              duplicate: 2,
            },
            duration: 120,
            packetsAffected: 456789,
            bytesAffected: 365432100,
            createdAt: '2024-01-18T11:45:00Z',
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
          delay: Math.random() * 200 + 10,
          loss: Math.random() * 10,
          duplicate: Math.random() * 5,
          reorder: Math.random() * 3,
          corrupt: Math.random() * 2,
          bandwidth: Math.random() * 100 + 50,
        }));
        setStats(mockStats);
      }
    } catch (error) {
      console.error('Failed to fetch impairment data:', error);
      setError('Failed to fetch network impairment data. Please check your connection.');
    } finally {
      setLoading(false);
    }
  };

  const handleCreateRule = () => {
    setSelectedRule(null);
    setDialogMode('create');
    setOpenDialog(true);
  };

  const handleEditRule = (rule: ImpairmentRule) => {
    setSelectedRule(rule);
    setDialogMode('edit');
    setOpenDialog(true);
  };

  const handleDeleteRule = async (ruleId: string) => {
    if (window.confirm('Are you sure you want to delete this rule?')) {
      try {
        await fetch(`/api/v1/network-impairments/rules/${ruleId}`, {
          method: 'DELETE',
        });
        fetchImpairmentData();
      } catch (error) {
        console.error('Failed to delete rule:', error);
      }
    }
  };

  const handleToggleRule = async (ruleId: string, enabled: boolean) => {
    try {
      await fetch(`/api/v1/network-impairments/rules/${ruleId}`, {
        method: 'PATCH',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({
          enabled: !enabled,
        }),
      });
      fetchImpairmentData();
    } catch (error) {
      console.error('Failed to toggle rule:', error);
    }
  };

  const getTypeIcon = (type: string) => {
    switch (type.toLowerCase()) {
      case 'delay':
        return <SpeedIcon />;
      case 'loss':
      case 'duplicate':
      case 'reorder':
      case 'corrupt':
        return <WarningIcon />;
      case 'bandwidth':
        return <NetworkIcon />;
      default:
        return <SettingsIcon />;
    }
  };

  const getTypeColor = (type: string) => {
    switch (type.toLowerCase()) {
      case 'delay':
        return 'warning';
      case 'loss':
        return 'error';
      case 'duplicate':
        return 'info';
      case 'reorder':
        return 'secondary';
      case 'corrupt':
        return 'error';
      case 'bandwidth':
        return 'primary';
      default:
        return 'default';
    }
  };

  if (loading && rules.length === 0) {
    return (
      <Box sx={{ p: 3 }}>
        <Typography variant="h4" gutterBottom>Network Impairments</Typography>
        <LinearProgress />
      </Box>
    );
  }

  return (
    <Box>
      <Box sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', mb: 3 }}>
        <Typography variant="h4" component="h1">
          Network Impairments
        </Typography>
        <Box>
          <Button
            variant="outlined"
            startIcon={<RefreshIcon />}
            onClick={fetchImpairmentData}
            sx={{ mr: 1 }}
          >
            Refresh
          </Button>
          <Button
            variant="contained"
            startIcon={<AddIcon />}
            onClick={handleCreateRule}
          >
            New Rule
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
          <Tab label="Rules" />
          <Tab label="Statistics" />
          <Tab label="Performance" />
        </Tabs>
      </Box>

      {/* Rules Tab */}
      {tabValue === 0 && (
        <Grid container spacing={3}>
          {/* Summary Cards */}
          <Grid item xs={12} md={3}>
            <Card>
              <CardContent>
                <Box sx={{ display: 'flex', alignItems: 'center', mb: 2 }}>
                  <NetworkIcon color="primary" sx={{ mr: 1 }} />
                  <Typography variant="h6">Total Rules</Typography>
                </Box>
                <Typography variant="h4">{rules.length}</Typography>
              </CardContent>
            </Card>
          </Grid>

          <Grid item xs={12} md={3}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>Active</Typography>
                <Typography variant="h4" color="success.main">
                  {rules.filter(r => r.enabled).length}
                </Typography>
              </CardContent>
            </Card>
          </Grid>

          <Grid item xs={12} md={3}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>Packets Affected</Typography>
                <Typography variant="h4" color="warning.main">
                  {rules.reduce((sum, r) => sum + r.packetsAffected, 0).toLocaleString()}
                </Typography>
              </CardContent>
            </Card>
          </Grid>

          <Grid item xs={12} md={3}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>Bytes Affected</Typography>
                <Typography variant="h4" color="error.main">
                  {(rules.reduce((sum, r) => sum + r.bytesAffected, 0) / 1024 / 1024).toFixed(1)} MB
                </Typography>
              </CardContent>
            </Card>
          </Grid>

          {/* Rules Table */}
          <Grid item xs={12}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  Impairment Rules
                </Typography>
                <TableContainer component={Paper} sx={{ backgroundColor: 'transparent' }}>
                  <Table>
                    <TableHead>
                      <TableRow>
                        <TableCell>Name</TableCell>
                        <TableCell>Type</TableCell>
                        <TableCell>Interface</TableCell>
                        <TableCell>Parameters</TableCell>
                        <TableCell>Duration</TableCell>
                        <TableCell>Status</TableCell>
                        <TableCell>Packets Affected</TableCell>
                        <TableCell>Actions</TableCell>
                      </TableRow>
                    </TableHead>
                    <TableBody>
                      {rules.map((rule) => (
                        <TableRow key={rule.id}>
                          <TableCell>
                            <Box sx={{ display: 'flex', alignItems: 'center' }}>
                              <Box sx={{ mr: 1 }}>
                                {getTypeIcon(rule.type)}
                              </Box>
                              {rule.name}
                            </Box>
                          </TableCell>
                          <TableCell>
                            <Chip 
                              label={rule.type.toUpperCase()} 
                              color={getTypeColor(rule.type)}
                              size="small" 
                            />
                          </TableCell>
                          <TableCell>{rule.interface}</TableCell>
                          <TableCell>
                            <Box sx={{ display: 'flex', flexDirection: 'column', gap: 0.5 }}>
                              {Object.entries(rule.parameters).map(([key, value]) => (
                                <Typography key={key} variant="caption">
                                  {key}: {value}
                                  {key === 'delay' || key === 'jitter' ? 'ms' : 
                                   key === 'loss' || key === 'duplicate' || key === 'reorder' || key === 'corrupt' ? '%' :
                                   key === 'bandwidth' ? 'Mbps' : ''}
                                </Typography>
                              ))}
                            </Box>
                          </TableCell>
                          <TableCell>{rule.duration}s</TableCell>
                          <TableCell>
                            <Chip 
                              label={rule.enabled ? 'Active' : 'Inactive'} 
                              color={rule.enabled ? 'success' : 'default'}
                              size="small"
                            />
                          </TableCell>
                          <TableCell>{rule.packetsAffected.toLocaleString()}</TableCell>
                          <TableCell>
                            <IconButton
                              size="small"
                              onClick={() => handleToggleRule(rule.id, rule.enabled)}
                            >
                              {rule.enabled ? <StopIcon /> : <PlayIcon />}
                            </IconButton>
                            <IconButton
                              size="small"
                              onClick={() => handleEditRule(rule)}
                            >
                              <EditIcon />
                            </IconButton>
                            <IconButton
                              size="small"
                              onClick={() => handleDeleteRule(rule.id)}
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

      {/* Statistics Tab */}
      {tabValue === 1 && (
        <Grid container spacing={3}>
          <Grid item xs={12} md={6}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  Impairment Types Distribution
                </Typography>
                <Box sx={{ height: 300 }}>
                  <ResponsiveContainer width="100%" height="100%">
                    <AreaChart data={[
                      { type: 'Delay', count: rules.filter(r => r.type === 'delay').length },
                      { type: 'Loss', count: rules.filter(r => r.type === 'loss').length },
                      { type: 'Duplicate', count: rules.filter(r => r.type === 'duplicate').length },
                      { type: 'Reorder', count: rules.filter(r => r.type === 'reorder').length },
                      { type: 'Corrupt', count: rules.filter(r => r.type === 'corrupt').length },
                      { type: 'Bandwidth', count: rules.filter(r => r.type === 'bandwidth').length },
                    ]}>
                      <CartesianGrid strokeDasharray="3 3" />
                      <XAxis dataKey="type" />
                      <YAxis />
                      <Tooltip />
                      <Area type="monotone" dataKey="count" stroke="#8884d8" fill="#8884d8" />
                    </AreaChart>
                  </ResponsiveContainer>
                </Box>
              </CardContent>
            </Card>
          </Grid>

          <Grid item xs={12} md={6}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  Packets Affected by Type
                </Typography>
                <Box sx={{ height: 300 }}>
                  <ResponsiveContainer width="100%" height="100%">
                    <AreaChart data={rules.map(r => ({ 
                      name: r.name, 
                      packets: r.packetsAffected 
                    }))}>
                      <CartesianGrid strokeDasharray="3 3" />
                      <XAxis dataKey="name" />
                      <YAxis />
                      <Tooltip />
                      <Area type="monotone" dataKey="packets" stroke="#82ca9d" fill="#82ca9d" />
                    </AreaChart>
                  </ResponsiveContainer>
                </Box>
              </CardContent>
            </Card>
          </Grid>
        </Grid>
      )}

      {/* Performance Tab */}
      {tabValue === 2 && (
        <Grid container spacing={3}>
          <Grid item xs={12}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  Impairment Metrics Over Time
                </Typography>
                <Box sx={{ height: 400 }}>
                  <ResponsiveContainer width="100%" height="100%">
                    <LineChart data={stats}>
                      <CartesianGrid strokeDasharray="3 3" />
                      <XAxis dataKey="timestamp" />
                      <YAxis />
                      <Tooltip />
                      <Line type="monotone" dataKey="delay" stroke="#8884d8" strokeWidth={2} />
                      <Line type="monotone" dataKey="loss" stroke="#82ca9d" strokeWidth={2} />
                      <Line type="monotone" dataKey="duplicate" stroke="#ffc658" strokeWidth={2} />
                      <Line type="monotone" dataKey="reorder" stroke="#ff7300" strokeWidth={2} />
                      <Line type="monotone" dataKey="corrupt" stroke="#ff0000" strokeWidth={2} />
                      <Line type="monotone" dataKey="bandwidth" stroke="#00ff00" strokeWidth={2} />
                    </LineChart>
                  </ResponsiveContainer>
                </Box>
              </CardContent>
            </Card>
          </Grid>
        </Grid>
      )}

      {/* Create/Edit Rule Dialog */}
      <Dialog open={openDialog} onClose={() => setOpenDialog(false)} maxWidth="md" fullWidth>
        <DialogTitle>
          {dialogMode === 'create' ? 'Create Rule' : 'Edit Rule'}
        </DialogTitle>
        <DialogContent>
          <Box sx={{ display: 'flex', flexDirection: 'column', gap: 2, mt: 1 }}>
            <TextField
              label="Rule Name"
              defaultValue={selectedRule?.name || ''}
              fullWidth
            />
            <TextField
              label="Impairment Type"
              select
              defaultValue={selectedRule?.type || 'delay'}
              fullWidth
            >
              <MenuItem value="delay">Delay</MenuItem>
              <MenuItem value="loss">Packet Loss</MenuItem>
              <MenuItem value="duplicate">Packet Duplication</MenuItem>
              <MenuItem value="reorder">Packet Reordering</MenuItem>
              <MenuItem value="corrupt">Packet Corruption</MenuItem>
              <MenuItem value="bandwidth">Bandwidth Limitation</MenuItem>
            </TextField>
            <TextField
              label="Interface"
              defaultValue={selectedRule?.interface || ''}
              fullWidth
            />
            <TextField
              label="Duration (seconds)"
              type="number"
              defaultValue={selectedRule?.duration || 300}
              fullWidth
            />
            <Box>
              <Typography variant="subtitle2" gutterBottom>
                Impairment Parameters
              </Typography>
              <Box sx={{ display: 'flex', flexDirection: 'column', gap: 2 }}>
                <Box>
                  <Typography variant="body2" gutterBottom>
                    Delay (ms)
                  </Typography>
                  <Slider
                    defaultValue={selectedRule?.parameters?.delay || 0}
                    min={0}
                    max={1000}
                    step={10}
                    valueLabelDisplay="auto"
                    valueLabelFormat={(value) => `${value} ms`}
                  />
                </Box>
                <Box>
                  <Typography variant="body2" gutterBottom>
                    Loss (%)
                  </Typography>
                  <Slider
                    defaultValue={selectedRule?.parameters?.loss || 0}
                    min={0}
                    max={50}
                    step={0.1}
                    valueLabelDisplay="auto"
                    valueLabelFormat={(value) => `${value}%`}
                  />
                </Box>
                <Box>
                  <Typography variant="body2" gutterBottom>
                    Bandwidth (Mbps)
                  </Typography>
                  <Slider
                    defaultValue={selectedRule?.parameters?.bandwidth || 100}
                    min={1}
                    max={1000}
                    step={1}
                    valueLabelDisplay="auto"
                    valueLabelFormat={(value) => `${value} Mbps`}
                  />
                </Box>
              </Box>
            </Box>
            <FormControlLabel
              control={<Switch defaultChecked={selectedRule?.enabled} />}
              label="Enabled"
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

export default NetworkImpairments;
