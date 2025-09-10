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
  Alert,
  LinearProgress,
  Tabs,
  Tab,
} from '@mui/material';
import {
  PlayArrow as PlayIcon,
  Stop as StopIcon,
  Pause as PauseIcon,
  Refresh as RefreshIcon,
  Router as RouterIcon,
  NetworkCheck as NetworkIcon,
  Speed as SpeedIcon,
  Settings as SettingsIcon,
  Add as AddIcon,
  Edit as EditIcon,
  Delete as DeleteIcon,
} from '@mui/icons-material';
import { LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip, ResponsiveContainer, AreaChart, Area } from 'recharts';

interface RouterNode {
  id: string;
  name: string;
  type: 'core' | 'edge' | 'access';
  status: 'running' | 'stopped' | 'error';
  protocols: string[];
  interfaces: RouterInterface[];
  cpuUsage: number;
  memoryUsage: number;
  uptime: string;
  lastUpdate: string;
}

interface RouterInterface {
  name: string;
  status: 'up' | 'down' | 'admin-down';
  ipAddress: string;
  bandwidth: number;
  utilization: number;
  packetsIn: number;
  packetsOut: number;
  errors: number;
}

interface SimulationScenario {
  id: string;
  name: string;
  description: string;
  protocols: string[];
  duration: number;
  status: 'idle' | 'running' | 'paused' | 'completed';
  progress: number;
  startTime?: string;
  endTime?: string;
}

interface ProtocolStats {
  name: string;
  sessions: number;
  routes: number;
  updates: number;
  errors: number;
  convergenceTime: number;
}

const RouterSim: React.FC = () => {
  const [routers, setRouters] = useState<RouterNode[]>([]);
  const [scenarios, setScenarios] = useState<SimulationScenario[]>([]);
  const [protocolStats, setProtocolStats] = useState<ProtocolStats[]>([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);
  const [tabValue, setTabValue] = useState(0);
  const [openDialog, setOpenDialog] = useState(false);
  const [selectedScenario, setSelectedScenario] = useState<SimulationScenario | null>(null);
  const [dialogMode, setDialogMode] = useState<'create' | 'edit'>('create');
  const [simulationRunning, setSimulationRunning] = useState(false);

  useEffect(() => {
    fetchRouterData();
    const interval = setInterval(fetchRouterData, 5000);
    return () => clearInterval(interval);
  }, []);

  const fetchRouterData = async () => {
    try {
      setLoading(true);
      setError(null);

      const [routersRes, scenariosRes, statsRes] = await Promise.all([
        fetch('/api/v1/router-sim/routers'),
        fetch('/api/v1/router-sim/scenarios'),
        fetch('/api/v1/router-sim/protocols'),
      ]);

      if (routersRes.ok) {
        const routersData = await routersRes.json();
        setRouters(routersData.routers || []);
      } else {
        // Mock data for demo
        setRouters([
          {
            id: '1',
            name: 'Core-Router-01',
            type: 'core',
            status: 'running',
            protocols: ['BGP', 'OSPF', 'ISIS'],
            interfaces: [
              {
                name: 'GigabitEthernet0/0/0',
                status: 'up',
                ipAddress: '192.168.1.1/24',
                bandwidth: 1000,
                utilization: 45,
                packetsIn: 1234567,
                packetsOut: 987654,
                errors: 0,
              },
              {
                name: 'GigabitEthernet0/0/1',
                status: 'up',
                ipAddress: '192.168.2.1/24',
                bandwidth: 1000,
                utilization: 32,
                packetsIn: 876543,
                packetsOut: 654321,
                errors: 2,
              },
            ],
            cpuUsage: 65,
            memoryUsage: 78,
            uptime: '15d 8h 32m',
            lastUpdate: new Date().toISOString(),
          },
          {
            id: '2',
            name: 'Edge-Router-01',
            type: 'edge',
            status: 'running',
            protocols: ['BGP', 'OSPF'],
            interfaces: [
              {
                name: 'GigabitEthernet0/0/0',
                status: 'up',
                ipAddress: '10.0.1.1/24',
                bandwidth: 1000,
                utilization: 28,
                packetsIn: 456789,
                packetsOut: 321654,
                errors: 0,
              },
            ],
            cpuUsage: 42,
            memoryUsage: 56,
            uptime: '12d 4h 15m',
            lastUpdate: new Date().toISOString(),
          },
          {
            id: '3',
            name: 'Access-Router-01',
            type: 'access',
            status: 'stopped',
            protocols: ['OSPF'],
            interfaces: [
              {
                name: 'GigabitEthernet0/0/0',
                status: 'down',
                ipAddress: '172.16.1.1/24',
                bandwidth: 100,
                utilization: 0,
                packetsIn: 0,
                packetsOut: 0,
                errors: 0,
              },
            ],
            cpuUsage: 0,
            memoryUsage: 12,
            uptime: '0d 0h 0m',
            lastUpdate: new Date().toISOString(),
          },
        ]);
      }

      if (scenariosRes.ok) {
        const scenariosData = await scenariosRes.json();
        setScenarios(scenariosData.scenarios || []);
      } else {
        // Mock scenarios data
        setScenarios([
          {
            id: '1',
            name: 'BGP Convergence Test',
            description: 'Test BGP convergence after link failure',
            protocols: ['BGP'],
            duration: 300,
            status: 'idle',
            progress: 0,
          },
          {
            id: '2',
            name: 'OSPF LSA Flooding',
            description: 'Simulate OSPF LSA flooding across network',
            protocols: ['OSPF'],
            duration: 180,
            status: 'running',
            progress: 45,
            startTime: new Date(Date.now() - 120000).toISOString(),
          },
          {
            id: '3',
            name: 'ISIS Multi-Area',
            description: 'Test ISIS multi-area configuration',
            protocols: ['ISIS'],
            duration: 600,
            status: 'completed',
            progress: 100,
            startTime: new Date(Date.now() - 600000).toISOString(),
            endTime: new Date().toISOString(),
          },
        ]);
      }

      if (statsRes.ok) {
        const statsData = await statsRes.json();
        setProtocolStats(statsData.protocols || []);
      } else {
        // Mock protocol stats
        setProtocolStats([
          {
            name: 'BGP',
            sessions: 8,
            routes: 1250,
            updates: 45,
            errors: 2,
            convergenceTime: 12.5,
          },
          {
            name: 'OSPF',
            sessions: 12,
            routes: 890,
            updates: 23,
            errors: 0,
            convergenceTime: 8.2,
          },
          {
            name: 'ISIS',
            sessions: 6,
            routes: 567,
            updates: 18,
            errors: 1,
            convergenceTime: 15.8,
          },
        ]);
      }
    } catch (error) {
      console.error('Failed to fetch router data:', error);
      setError('Failed to fetch router simulation data. Please check your connection.');
    } finally {
      setLoading(false);
    }
  };

  const handleStartSimulation = async (scenarioId: string) => {
    try {
      await fetch(`/api/v1/router-sim/scenarios/${scenarioId}/start`, {
        method: 'POST',
      });
      setSimulationRunning(true);
      fetchRouterData();
    } catch (error) {
      console.error('Failed to start simulation:', error);
    }
  };

  const handleStopSimulation = async (scenarioId: string) => {
    try {
      await fetch(`/api/v1/router-sim/scenarios/${scenarioId}/stop`, {
        method: 'POST',
      });
      setSimulationRunning(false);
      fetchRouterData();
    } catch (error) {
      console.error('Failed to stop simulation:', error);
    }
  };

  const handleCreateScenario = () => {
    setSelectedScenario(null);
    setDialogMode('create');
    setOpenDialog(true);
  };

  const handleEditScenario = (scenario: SimulationScenario) => {
    setSelectedScenario(scenario);
    setDialogMode('edit');
    setOpenDialog(true);
  };

  const getStatusColor = (status: string) => {
    switch (status.toLowerCase()) {
      case 'running':
      case 'up':
        return 'success';
      case 'stopped':
      case 'down':
        return 'error';
      case 'paused':
      case 'admin-down':
        return 'warning';
      case 'completed':
        return 'info';
      default:
        return 'default';
    }
  };

  const getRouterTypeIcon = (type: string) => {
    switch (type.toLowerCase()) {
      case 'core':
        return <RouterIcon color="primary" />;
      case 'edge':
        return <NetworkIcon color="secondary" />;
      case 'access':
        return <SpeedIcon color="info" />;
      default:
        return <RouterIcon />;
    }
  };

  if (loading && routers.length === 0) {
    return (
      <Box sx={{ p: 3 }}>
        <Typography variant="h4" gutterBottom>Router Simulation</Typography>
        <LinearProgress />
      </Box>
    );
  }

  return (
    <Box>
      <Box sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', mb: 3 }}>
        <Typography variant="h4" component="h1">
          Router Simulation
        </Typography>
        <Box>
          <Button
            variant="outlined"
            startIcon={<RefreshIcon />}
            onClick={fetchRouterData}
            sx={{ mr: 1 }}
          >
            Refresh
          </Button>
          <Button
            variant="contained"
            startIcon={<AddIcon />}
            onClick={handleCreateScenario}
          >
            New Scenario
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
          <Tab label="Routers" />
          <Tab label="Scenarios" />
          <Tab label="Protocols" />
          <Tab label="Performance" />
        </Tabs>
      </Box>

      {/* Routers Tab */}
      {tabValue === 0 && (
        <Grid container spacing={3}>
          {/* Summary Cards */}
          <Grid item xs={12} md={3}>
            <Card>
              <CardContent>
                <Box sx={{ display: 'flex', alignItems: 'center', mb: 2 }}>
                  <RouterIcon color="primary" sx={{ mr: 1 }} />
                  <Typography variant="h6">Total Routers</Typography>
                </Box>
                <Typography variant="h4">{routers.length}</Typography>
              </CardContent>
            </Card>
          </Grid>

          <Grid item xs={12} md={3}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>Running</Typography>
                <Typography variant="h4" color="success.main">
                  {routers.filter(r => r.status === 'running').length}
                </Typography>
              </CardContent>
            </Card>
          </Grid>

          <Grid item xs={12} md={3}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>Core</Typography>
                <Typography variant="h4" color="primary.main">
                  {routers.filter(r => r.type === 'core').length}
                </Typography>
              </CardContent>
            </Card>
          </Grid>

          <Grid item xs={12} md={3}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>Edge</Typography>
                <Typography variant="h4" color="secondary.main">
                  {routers.filter(r => r.type === 'edge').length}
                </Typography>
              </CardContent>
            </Card>
          </Grid>

          {/* Routers Table */}
          <Grid item xs={12}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  Router Status
                </Typography>
                <TableContainer component={Paper} sx={{ backgroundColor: 'transparent' }}>
                  <Table>
                    <TableHead>
                      <TableRow>
                        <TableCell>Name</TableCell>
                        <TableCell>Type</TableCell>
                        <TableCell>Status</TableCell>
                        <TableCell>Protocols</TableCell>
                        <TableCell>CPU</TableCell>
                        <TableCell>Memory</TableCell>
                        <TableCell>Uptime</TableCell>
                        <TableCell>Actions</TableCell>
                      </TableRow>
                    </TableHead>
                    <TableBody>
                      {routers.map((router) => (
                        <TableRow key={router.id}>
                          <TableCell>
                            <Box sx={{ display: 'flex', alignItems: 'center' }}>
                              <Box sx={{ mr: 1 }}>
                                {getRouterTypeIcon(router.type)}
                              </Box>
                              {router.name}
                            </Box>
                          </TableCell>
                          <TableCell>
                            <Chip 
                              label={router.type} 
                              color={router.type === 'core' ? 'primary' : router.type === 'edge' ? 'secondary' : 'default'}
                              size="small" 
                            />
                          </TableCell>
                          <TableCell>
                            <Chip 
                              label={router.status} 
                              color={getStatusColor(router.status)}
                              size="small"
                            />
                          </TableCell>
                          <TableCell>
                            <Box sx={{ display: 'flex', gap: 0.5, flexWrap: 'wrap' }}>
                              {router.protocols.map((protocol) => (
                                <Chip key={protocol} label={protocol} size="small" variant="outlined" />
                              ))}
                            </Box>
                          </TableCell>
                          <TableCell>
                            <Box sx={{ display: 'flex', alignItems: 'center' }}>
                              <Typography variant="body2" sx={{ mr: 1 }}>
                                {router.cpuUsage}%
                              </Typography>
                              <LinearProgress
                                variant="determinate"
                                value={router.cpuUsage}
                                sx={{ width: 60, height: 8, borderRadius: 4 }}
                              />
                            </Box>
                          </TableCell>
                          <TableCell>
                            <Box sx={{ display: 'flex', alignItems: 'center' }}>
                              <Typography variant="body2" sx={{ mr: 1 }}>
                                {router.memoryUsage}%
                              </Typography>
                              <LinearProgress
                                variant="determinate"
                                value={router.memoryUsage}
                                sx={{ width: 60, height: 8, borderRadius: 4 }}
                              />
                            </Box>
                          </TableCell>
                          <TableCell>{router.uptime}</TableCell>
                          <TableCell>
                            <IconButton size="small">
                              <SettingsIcon />
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

      {/* Scenarios Tab */}
      {tabValue === 1 && (
        <Grid container spacing={3}>
          <Grid item xs={12}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  Simulation Scenarios
                </Typography>
                <TableContainer component={Paper} sx={{ backgroundColor: 'transparent' }}>
                  <Table>
                    <TableHead>
                      <TableRow>
                        <TableCell>Name</TableCell>
                        <TableCell>Description</TableCell>
                        <TableCell>Protocols</TableCell>
                        <TableCell>Duration</TableCell>
                        <TableCell>Status</TableCell>
                        <TableCell>Progress</TableCell>
                        <TableCell>Actions</TableCell>
                      </TableRow>
                    </TableHead>
                    <TableBody>
                      {scenarios.map((scenario) => (
                        <TableRow key={scenario.id}>
                          <TableCell>{scenario.name}</TableCell>
                          <TableCell>{scenario.description}</TableCell>
                          <TableCell>
                            <Box sx={{ display: 'flex', gap: 0.5, flexWrap: 'wrap' }}>
                              {scenario.protocols.map((protocol) => (
                                <Chip key={protocol} label={protocol} size="small" variant="outlined" />
                              ))}
                            </Box>
                          </TableCell>
                          <TableCell>{scenario.duration}s</TableCell>
                          <TableCell>
                            <Chip 
                              label={scenario.status} 
                              color={getStatusColor(scenario.status)}
                              size="small"
                            />
                          </TableCell>
                          <TableCell>
                            <Box sx={{ display: 'flex', alignItems: 'center' }}>
                              <Typography variant="body2" sx={{ mr: 1 }}>
                                {scenario.progress}%
                              </Typography>
                              <LinearProgress
                                variant="determinate"
                                value={scenario.progress}
                                sx={{ width: 100, height: 8, borderRadius: 4 }}
                              />
                            </Box>
                          </TableCell>
                          <TableCell>
                            <IconButton
                              size="small"
                              onClick={() => scenario.status === 'running' 
                                ? handleStopSimulation(scenario.id)
                                : handleStartSimulation(scenario.id)
                              }
                            >
                              {scenario.status === 'running' ? <StopIcon /> : <PlayIcon />}
                            </IconButton>
                            <IconButton
                              size="small"
                              onClick={() => handleEditScenario(scenario)}
                            >
                              <EditIcon />
                            </IconButton>
                            <IconButton
                              size="small"
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

      {/* Protocols Tab */}
      {tabValue === 2 && (
        <Grid container spacing={3}>
          <Grid item xs={12}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  Protocol Statistics
                </Typography>
                <TableContainer component={Paper} sx={{ backgroundColor: 'transparent' }}>
                  <Table>
                    <TableHead>
                      <TableRow>
                        <TableCell>Protocol</TableCell>
                        <TableCell>Sessions</TableCell>
                        <TableCell>Routes</TableCell>
                        <TableCell>Updates</TableCell>
                        <TableCell>Errors</TableCell>
                        <TableCell>Convergence Time</TableCell>
                      </TableRow>
                    </TableHead>
                    <TableBody>
                      {protocolStats.map((stat) => (
                        <TableRow key={stat.name}>
                          <TableCell>
                            <Chip 
                              label={stat.name} 
                              color="primary"
                              size="small" 
                            />
                          </TableCell>
                          <TableCell>{stat.sessions}</TableCell>
                          <TableCell>{stat.routes}</TableCell>
                          <TableCell>{stat.updates}</TableCell>
                          <TableCell>
                            <Chip 
                              label={stat.errors} 
                              color={stat.errors > 0 ? 'error' : 'success'}
                              size="small" 
                            />
                          </TableCell>
                          <TableCell>{stat.convergenceTime}s</TableCell>
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

      {/* Performance Tab */}
      {tabValue === 3 && (
        <Grid container spacing={3}>
          <Grid item xs={12}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  Performance Metrics
                </Typography>
                <Box sx={{ height: 400 }}>
                  <ResponsiveContainer width="100%" height="100%">
                    <AreaChart data={[
                      { time: '00:00', cpu: 45, memory: 60, throughput: 2.1 },
                      { time: '04:00', cpu: 38, memory: 58, throughput: 1.8 },
                      { time: '08:00', cpu: 65, memory: 78, throughput: 2.5 },
                      { time: '12:00', cpu: 72, memory: 82, throughput: 2.8 },
                      { time: '16:00', cpu: 58, memory: 70, throughput: 2.2 },
                      { time: '20:00', cpu: 42, memory: 65, throughput: 2.0 },
                    ]}>
                      <CartesianGrid strokeDasharray="3 3" />
                      <XAxis dataKey="time" />
                      <YAxis />
                      <Tooltip />
                      <Area type="monotone" dataKey="cpu" stackId="1" stroke="#8884d8" fill="#8884d8" />
                      <Area type="monotone" dataKey="memory" stackId="1" stroke="#82ca9d" fill="#82ca9d" />
                    </AreaChart>
                  </ResponsiveContainer>
                </Box>
              </CardContent>
            </Card>
          </Grid>
        </Grid>
      )}

      {/* Create/Edit Scenario Dialog */}
      <Dialog open={openDialog} onClose={() => setOpenDialog(false)} maxWidth="md" fullWidth>
        <DialogTitle>
          {dialogMode === 'create' ? 'Create Scenario' : 'Edit Scenario'}
        </DialogTitle>
        <DialogContent>
          <Box sx={{ display: 'flex', flexDirection: 'column', gap: 2, mt: 1 }}>
            <TextField
              label="Scenario Name"
              defaultValue={selectedScenario?.name || ''}
              fullWidth
            />
            <TextField
              label="Description"
              multiline
              rows={3}
              defaultValue={selectedScenario?.description || ''}
              fullWidth
            />
            <TextField
              label="Duration (seconds)"
              type="number"
              defaultValue={selectedScenario?.duration || 300}
              fullWidth
            />
            <Box>
              <Typography variant="subtitle2" gutterBottom>
                Protocols
              </Typography>
              <FormControlLabel control={<Switch defaultChecked />} label="BGP" />
              <FormControlLabel control={<Switch />} label="OSPF" />
              <FormControlLabel control={<Switch />} label="ISIS" />
            </Box>
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

export default RouterSim;
