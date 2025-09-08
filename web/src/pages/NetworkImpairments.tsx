import React, { useState } from 'react';
import {
  Box,
  Typography,
  Card,
  CardContent,
  Grid,
  Button,
  List,
  ListItem,
  ListItemText,
  ListItemSecondaryAction,
  IconButton,
  Chip,
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
  Alert,
  Snackbar,
  Paper,
  Divider,
  LinearProgress,
  Tabs,
  Tab,
} from '@mui/material';
import {
  Add as AddIcon,
  Edit as EditIcon,
  Delete as DeleteIcon,
  PlayArrow as PlayIcon,
  Stop as StopIcon,
  Pause as PauseIcon,
  Refresh as RefreshIcon,
  NetworkCheck as NetworkCheckIcon,
  Speed as SpeedIcon,
  Warning as WarningIcon,
  TrendingUp as TrendingUpIcon,
} from '@mui/icons-material';
import { useQuery, useMutation, useQueryClient } from '@tanstack/react-query';

interface ImpairmentRule {
  id: string;
  name: string;
  description: string;
  type: 'latency' | 'jitter' | 'packet_loss' | 'bandwidth' | 'duplication' | 'reordering' | 'corruption';
  status: 'active' | 'inactive' | 'paused';
  interface: string;
  source_ip: string;
  destination_ip: string;
  protocol: string;
  port: number;
  value: number;
  variation: number;
  probability: number;
  created_at: string;
  updated_at: string;
}

interface ImpairmentStats {
  total_packets: number;
  impaired_packets: number;
  impairment_rate: number;
  average_latency: number;
  average_jitter: number;
  packet_loss_rate: number;
  bandwidth_utilization: number;
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
      id={`impairment-tabpanel-${index}`}
      aria-labelledby={`impairment-tab-${index}`}
      {...other}
    >
      {value === index && <Box sx={{ p: 3 }}>{children}</Box>}
    </div>
  );
}

const NetworkImpairments: React.FC = () => {
  const [selectedRule, setSelectedRule] = useState<ImpairmentRule | null>(null);
  const [ruleDialogOpen, setRuleDialogOpen] = useState(false);
  const [editingRule, setEditingRule] = useState<ImpairmentRule | null>(null);
  const [snackbarOpen, setSnackbarOpen] = useState(false);
  const [snackbarMessage, setSnackbarMessage] = useState('');
  const [snackbarSeverity, setSnackbarSeverity] = useState<'success' | 'error'>('success');
  const [tabValue, setTabValue] = useState(0);

  const queryClient = useQueryClient();

  const { data: rules, isLoading } = useQuery({
    queryKey: ['impairment-rules'],
    queryFn: async () => {
      const response = await fetch('/api/v1/impairments/rules');
      return response.json();
    },
  });

  const { data: stats } = useQuery({
    queryKey: ['impairment-stats'],
    queryFn: async () => {
      const response = await fetch('/api/v1/impairments/stats');
      return response.json();
    },
  });

  const { data: interfaces } = useQuery({
    queryKey: ['impairment-interfaces'],
    queryFn: async () => {
      const response = await fetch('/api/v1/impairments/interfaces');
      return response.json();
    },
  });

  const createRuleMutation = useMutation({
    mutationFn: async (ruleData: Partial<ImpairmentRule>) => {
      const response = await fetch('/api/v1/impairments/rules', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify(ruleData),
      });
      return response.json();
    },
    onSuccess: () => {
      queryClient.invalidateQueries({ queryKey: ['impairment-rules'] });
      setSnackbarMessage('Impairment rule created successfully');
      setSnackbarOpen(true);
      setRuleDialogOpen(false);
    },
    onError: () => {
      setSnackbarMessage('Failed to create impairment rule');
      setSnackbarSeverity('error');
      setSnackbarOpen(true);
    },
  });

  const updateRuleMutation = useMutation({
    mutationFn: async ({ id, ...ruleData }: Partial<ImpairmentRule> & { id: string }) => {
      const response = await fetch(`/api/v1/impairments/rules/${id}`, {
        method: 'PUT',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify(ruleData),
      });
      return response.json();
    },
    onSuccess: () => {
      queryClient.invalidateQueries({ queryKey: ['impairment-rules'] });
      setSnackbarMessage('Impairment rule updated successfully');
      setSnackbarOpen(true);
      setRuleDialogOpen(false);
    },
    onError: () => {
      setSnackbarMessage('Failed to update impairment rule');
      setSnackbarSeverity('error');
      setSnackbarOpen(true);
    },
  });

  const deleteRuleMutation = useMutation({
    mutationFn: async (ruleId: string) => {
      const response = await fetch(`/api/v1/impairments/rules/${ruleId}`, {
        method: 'DELETE',
      });
      return response.json();
    },
    onSuccess: () => {
      queryClient.invalidateQueries({ queryKey: ['impairment-rules'] });
      setSnackbarMessage('Impairment rule deleted successfully');
      setSnackbarOpen(true);
    },
    onError: () => {
      setSnackbarMessage('Failed to delete impairment rule');
      setSnackbarSeverity('error');
      setSnackbarOpen(true);
    },
  });

  const startRuleMutation = useMutation({
    mutationFn: async (ruleId: string) => {
      const response = await fetch(`/api/v1/impairments/rules/${ruleId}/start`, {
        method: 'POST',
      });
      return response.json();
    },
    onSuccess: () => {
      queryClient.invalidateQueries({ queryKey: ['impairment-rules'] });
      setSnackbarMessage('Impairment rule started successfully');
      setSnackbarOpen(true);
    },
    onError: () => {
      setSnackbarMessage('Failed to start impairment rule');
      setSnackbarSeverity('error');
      setSnackbarOpen(true);
    },
  });

  const stopRuleMutation = useMutation({
    mutationFn: async (ruleId: string) => {
      const response = await fetch(`/api/v1/impairments/rules/${ruleId}/stop`, {
        method: 'POST',
      });
      return response.json();
    },
    onSuccess: () => {
      queryClient.invalidateQueries({ queryKey: ['impairment-rules'] });
      setSnackbarMessage('Impairment rule stopped successfully');
      setSnackbarOpen(true);
    },
    onError: () => {
      setSnackbarMessage('Failed to stop impairment rule');
      setSnackbarSeverity('error');
      setSnackbarOpen(true);
    },
  });

  const pauseRuleMutation = useMutation({
    mutationFn: async (ruleId: string) => {
      const response = await fetch(`/api/v1/impairments/rules/${ruleId}/pause`, {
        method: 'POST',
      });
      return response.json();
    },
    onSuccess: () => {
      queryClient.invalidateQueries({ queryKey: ['impairment-rules'] });
      setSnackbarMessage('Impairment rule paused successfully');
      setSnackbarOpen(true);
    },
    onError: () => {
      setSnackbarMessage('Failed to pause impairment rule');
      setSnackbarSeverity('error');
      setSnackbarOpen(true);
    },
  });

  const handleCreateRule = () => {
    setEditingRule(null);
    setRuleDialogOpen(true);
  };

  const handleEditRule = (rule: ImpairmentRule) => {
    setEditingRule(rule);
    setRuleDialogOpen(true);
  };

  const handleDeleteRule = (ruleId: string) => {
    if (window.confirm('Are you sure you want to delete this impairment rule?')) {
      deleteRuleMutation.mutate(ruleId);
    }
  };

  const handleStartRule = (ruleId: string) => {
    startRuleMutation.mutate(ruleId);
  };

  const handleStopRule = (ruleId: string) => {
    stopRuleMutation.mutate(ruleId);
  };

  const handlePauseRule = (ruleId: string) => {
    pauseRuleMutation.mutate(ruleId);
  };

  const handleSaveRule = (ruleData: Partial<ImpairmentRule>) => {
    if (editingRule) {
      updateRuleMutation.mutate({ ...ruleData, id: editingRule.id });
    } else {
      createRuleMutation.mutate(ruleData);
    }
  };

  const handleTabChange = (event: React.SyntheticEvent, newValue: number) => {
    setTabValue(newValue);
  };

  const getStatusColor = (status: string) => {
    switch (status) {
      case 'active':
        return 'success';
      case 'inactive':
        return 'default';
      case 'paused':
        return 'warning';
      default:
        return 'default';
    }
  };

  const getStatusIcon = (status: string) => {
    switch (status) {
      case 'active':
        return <PlayIcon />;
      case 'inactive':
        return <StopIcon />;
      case 'paused':
        return <PauseIcon />;
      default:
        return <StopIcon />;
    }
  };

  const getImpairmentTypeLabel = (type: string) => {
    switch (type) {
      case 'latency':
        return 'Latency';
      case 'jitter':
        return 'Jitter';
      case 'packet_loss':
        return 'Packet Loss';
      case 'bandwidth':
        return 'Bandwidth';
      case 'duplication':
        return 'Duplication';
      case 'reordering':
        return 'Reordering';
      case 'corruption':
        return 'Corruption';
      default:
        return type;
    }
  };

  const getImpairmentIcon = (type: string) => {
    switch (type) {
      case 'latency':
        return <SpeedIcon />;
      case 'jitter':
        return <TrendingUpIcon />;
      case 'packet_loss':
        return <WarningIcon />;
      case 'bandwidth':
        return <NetworkCheckIcon />;
      case 'duplication':
        return <NetworkCheckIcon />;
      case 'reordering':
        return <NetworkCheckIcon />;
      case 'corruption':
        return <WarningIcon />;
      default:
        return <NetworkCheckIcon />;
    }
  };

  if (isLoading) {
    return (
      <Box display="flex" justifyContent="center" alignItems="center" minHeight="400px">
        <Typography>Loading impairment rules...</Typography>
      </Box>
    );
  }

  return (
    <Box>
      <Box display="flex" justifyContent="space-between" alignItems="center" mb={3}>
        <Typography variant="h4" component="h1">
          Network Impairments
        </Typography>
        <Box display="flex" gap={2}>
          <Button
            variant="outlined"
            startIcon={<RefreshIcon />}
            onClick={() => queryClient.invalidateQueries({ queryKey: ['impairment-rules'] })}
          >
            Refresh
          </Button>
          <Button
            variant="contained"
            startIcon={<AddIcon />}
            onClick={handleCreateRule}
          >
            Add Rule
          </Button>
        </Box>
      </Box>

      <Box sx={{ borderBottom: 1, borderColor: 'divider' }}>
        <Tabs value={tabValue} onChange={handleTabChange} aria-label="impairment tabs">
          <Tab label="Rules" />
          <Tab label="Statistics" />
          <Tab label="Templates" />
        </Tabs>
      </Box>

      {/* Rules Tab */}
      <TabPanel value={tabValue} index={0}>
        <Grid container spacing={3}>
          {/* Impairment Statistics */}
          <Grid item xs={12} md={6}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  Impairment Statistics
                </Typography>
                {stats?.data && (
                  <Grid container spacing={2}>
                    <Grid item xs={6}>
                      <Typography variant="body2" color="text.secondary">
                        Total Packets
                      </Typography>
                      <Typography variant="h6">
                        {stats.data.total_packets?.toLocaleString()}
                      </Typography>
                    </Grid>
                    <Grid item xs={6}>
                      <Typography variant="body2" color="text.secondary">
                        Impaired Packets
                      </Typography>
                      <Typography variant="h6" color="error">
                        {stats.data.impaired_packets?.toLocaleString()}
                      </Typography>
                    </Grid>
                    <Grid item xs={6}>
                      <Typography variant="body2" color="text.secondary">
                        Impairment Rate
                      </Typography>
                      <Typography variant="h6" color="error">
                        {(stats.data.impairment_rate * 100).toFixed(2)}%
                      </Typography>
                    </Grid>
                    <Grid item xs={6}>
                      <Typography variant="body2" color="text.secondary">
                        Average Latency
                      </Typography>
                      <Typography variant="h6">
                        {stats.data.average_latency?.toFixed(2)} ms
                      </Typography>
                    </Grid>
                    <Grid item xs={6}>
                      <Typography variant="body2" color="text.secondary">
                        Average Jitter
                      </Typography>
                      <Typography variant="h6">
                        {stats.data.average_jitter?.toFixed(2)} ms
                      </Typography>
                    </Grid>
                    <Grid item xs={6}>
                      <Typography variant="body2" color="text.secondary">
                        Packet Loss Rate
                      </Typography>
                      <Typography variant="h6" color="error">
                        {(stats.data.packet_loss_rate * 100).toFixed(2)}%
                      </Typography>
                    </Grid>
                  </Grid>
                )}
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
                {stats?.data && (
                  <Grid container spacing={2}>
                    <Grid item xs={12}>
                      <Typography variant="body2" color="text.secondary">
                        Bandwidth Utilization
                      </Typography>
                      <Typography variant="h6">
                        {(stats.data.bandwidth_utilization * 100).toFixed(2)}%
                      </Typography>
                      <LinearProgress
                        variant="determinate"
                        value={stats.data.bandwidth_utilization * 100}
                        sx={{ mt: 1 }}
                      />
                    </Grid>
                    <Grid item xs={12}>
                      <Typography variant="body2" color="text.secondary">
                        Impairment Rate
                      </Typography>
                      <Typography variant="h6">
                        {(stats.data.impairment_rate * 100).toFixed(2)}%
                      </Typography>
                      <LinearProgress
                        variant="determinate"
                        value={stats.data.impairment_rate * 100}
                        color="error"
                        sx={{ mt: 1 }}
                      />
                    </Grid>
                    <Grid item xs={12}>
                      <Typography variant="body2" color="text.secondary">
                        Packet Loss Rate
                      </Typography>
                      <Typography variant="h6">
                        {(stats.data.packet_loss_rate * 100).toFixed(2)}%
                      </Typography>
                      <LinearProgress
                        variant="determinate"
                        value={stats.data.packet_loss_rate * 100}
                        color="error"
                        sx={{ mt: 1 }}
                      />
                    </Grid>
                  </Grid>
                )}
              </CardContent>
            </Card>
          </Grid>

          {/* Impairment Rules */}
          <Grid item xs={12}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  Impairment Rules
                </Typography>
                {rules?.data && rules.data.length > 0 ? (
                  <List>
                    {rules.data.map((rule: ImpairmentRule) => (
                      <ListItem key={rule.id} divider>
                        <ListItemText
                          primary={rule.name}
                          secondary={
                            <Box>
                              <Typography variant="body2" color="text.secondary">
                                {rule.description}
                              </Typography>
                              <Box display="flex" gap={1} mt={1}>
                                <Chip
                                  icon={getStatusIcon(rule.status)}
                                  label={rule.status}
                                  color={getStatusColor(rule.status) as any}
                                  size="small"
                                />
                                <Chip
                                  icon={getImpairmentIcon(rule.type)}
                                  label={getImpairmentTypeLabel(rule.type)}
                                  size="small"
                                  variant="outlined"
                                />
                                <Chip
                                  label={`Value: ${rule.value}`}
                                  size="small"
                                  variant="outlined"
                                />
                                <Chip
                                  label={`Probability: ${(rule.probability * 100).toFixed(0)}%`}
                                  size="small"
                                  variant="outlined"
                                />
                              </Box>
                            </Box>
                          }
                        />
                        <ListItemSecondaryAction>
                          <Box display="flex" gap={1}>
                            {rule.status === 'active' ? (
                              <IconButton
                                onClick={() => handlePauseRule(rule.id)}
                                color="warning"
                              >
                                <PauseIcon />
                              </IconButton>
                            ) : rule.status === 'paused' ? (
                              <IconButton
                                onClick={() => handleStartRule(rule.id)}
                                color="primary"
                              >
                                <PlayIcon />
                              </IconButton>
                            ) : (
                              <IconButton
                                onClick={() => handleStartRule(rule.id)}
                                color="primary"
                              >
                                <PlayIcon />
                              </IconButton>
                            )}
                            <IconButton
                              onClick={() => handleEditRule(rule)}
                              color="primary"
                            >
                              <EditIcon />
                            </IconButton>
                            <IconButton
                              onClick={() => handleDeleteRule(rule.id)}
                              color="error"
                            >
                              <DeleteIcon />
                            </IconButton>
                          </Box>
                        </ListItemSecondaryAction>
                      </ListItem>
                    ))}
                  </List>
                ) : (
                  <Box textAlign="center" py={4}>
                    <Typography variant="body1" color="text.secondary">
                      No impairment rules configured. Create your first rule to get started.
                    </Typography>
                  </Box>
                )}
              </CardContent>
            </Card>
          </Grid>
        </Grid>
      </TabPanel>

      {/* Statistics Tab */}
      <TabPanel value={tabValue} index={1}>
        <Grid container spacing={3}>
          <Grid item xs={12}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  Detailed Statistics
                </Typography>
                {stats?.data && (
                  <Grid container spacing={2}>
                    <Grid item xs={12} md={4}>
                      <Paper sx={{ p: 2, textAlign: 'center' }}>
                        <Typography variant="h4" color="primary">
                          {stats.data.total_packets?.toLocaleString()}
                        </Typography>
                        <Typography variant="body2" color="text.secondary">
                          Total Packets
                        </Typography>
                      </Paper>
                    </Grid>
                    <Grid item xs={12} md={4}>
                      <Paper sx={{ p: 2, textAlign: 'center' }}>
                        <Typography variant="h4" color="error">
                          {stats.data.impaired_packets?.toLocaleString()}
                        </Typography>
                        <Typography variant="body2" color="text.secondary">
                          Impaired Packets
                        </Typography>
                      </Paper>
                    </Grid>
                    <Grid item xs={12} md={4}>
                      <Paper sx={{ p: 2, textAlign: 'center' }}>
                        <Typography variant="h4" color="warning">
                          {(stats.data.impairment_rate * 100).toFixed(2)}%
                        </Typography>
                        <Typography variant="body2" color="text.secondary">
                          Impairment Rate
                        </Typography>
                      </Paper>
                    </Grid>
                  </Grid>
                )}
              </CardContent>
            </Card>
          </Grid>
        </Grid>
      </TabPanel>

      {/* Templates Tab */}
      <TabPanel value={tabValue} index={2}>
        <Grid container spacing={3}>
          <Grid item xs={12}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  Impairment Templates
                </Typography>
                <Typography variant="body2" color="text.secondary">
                  Pre-configured impairment templates for common scenarios.
                </Typography>
                <Box mt={2}>
                  <Grid container spacing={2}>
                    <Grid item xs={12} md={4}>
                      <Paper sx={{ p: 2 }}>
                        <Typography variant="h6">High Latency</Typography>
                        <Typography variant="body2" color="text.secondary">
                          Simulates high latency network conditions
                        </Typography>
                        <Box mt={1}>
                          <Chip label="Latency: 200ms" size="small" />
                          <Chip label="Jitter: 50ms" size="small" />
                        </Box>
                      </Paper>
                    </Grid>
                    <Grid item xs={12} md={4}>
                      <Paper sx={{ p: 2 }}>
                        <Typography variant="h6">Packet Loss</Typography>
                        <Typography variant="body2" color="text.secondary">
                          Simulates packet loss conditions
                        </Typography>
                        <Box mt={1}>
                          <Chip label="Loss: 5%" size="small" />
                          <Chip label="Duplication: 2%" size="small" />
                        </Box>
                      </Paper>
                    </Grid>
                    <Grid item xs={12} md={4}>
                      <Paper sx={{ p: 2 }}>
                        <Typography variant="h6">Bandwidth Limit</Typography>
                        <Typography variant="body2" color="text.secondary">
                          Simulates limited bandwidth
                        </Typography>
                        <Box mt={1}>
                          <Chip label="Bandwidth: 10Mbps" size="small" />
                          <Chip label="Burst: 1MB" size="small" />
                        </Box>
                      </Paper>
                    </Grid>
                  </Grid>
                </Box>
              </CardContent>
            </Card>
          </Grid>
        </Grid>
      </TabPanel>

      {/* Rule Dialog */}
      <Dialog
        open={ruleDialogOpen}
        onClose={() => setRuleDialogOpen(false)}
        maxWidth="md"
        fullWidth
      >
        <DialogTitle>
          {editingRule ? 'Edit Impairment Rule' : 'Create Impairment Rule'}
        </DialogTitle>
        <DialogContent>
          <ImpairmentRuleForm
            rule={editingRule}
            onSave={handleSaveRule}
            onCancel={() => setRuleDialogOpen(false)}
            interfaces={interfaces?.data || []}
          />
        </DialogContent>
      </Dialog>

      {/* Snackbar for notifications */}
      <Snackbar
        open={snackbarOpen}
        autoHideDuration={6000}
        onClose={() => setSnackbarOpen(false)}
      >
        <Alert
          onClose={() => setSnackbarOpen(false)}
          severity={snackbarSeverity}
          sx={{ width: '100%' }}
        >
          {snackbarMessage}
        </Alert>
      </Snackbar>
    </Box>
  );
};

interface ImpairmentRuleFormProps {
  rule: ImpairmentRule | null;
  onSave: (data: Partial<ImpairmentRule>) => void;
  onCancel: () => void;
  interfaces: any[];
}

const ImpairmentRuleForm: React.FC<ImpairmentRuleFormProps> = ({ rule, onSave, onCancel, interfaces }) => {
  const [formData, setFormData] = useState({
    name: rule?.name || '',
    description: rule?.description || '',
    type: rule?.type || 'latency',
    status: rule?.status || 'inactive',
    interface: rule?.interface || '',
    source_ip: rule?.source_ip || '',
    destination_ip: rule?.destination_ip || '',
    protocol: rule?.protocol || 'tcp',
    port: rule?.port || 80,
    value: rule?.value || 0,
    variation: rule?.variation || 0,
    probability: rule?.probability || 1.0,
  });

  const handleSubmit = (e: React.FormEvent) => {
    e.preventDefault();
    onSave(formData);
  };

  return (
    <Box component="form" onSubmit={handleSubmit}>
      <Grid container spacing={2} sx={{ mt: 1 }}>
        <Grid item xs={12} md={6}>
          <TextField
            fullWidth
            label="Rule Name"
            value={formData.name}
            onChange={(e) => setFormData({ ...formData, name: e.target.value })}
            required
          />
        </Grid>
        <Grid item xs={12} md={6}>
          <TextField
            fullWidth
            label="Description"
            value={formData.description}
            onChange={(e) => setFormData({ ...formData, description: e.target.value })}
          />
        </Grid>
        <Grid item xs={12} md={6}>
          <FormControl fullWidth>
            <InputLabel>Impairment Type</InputLabel>
            <Select
              value={formData.type}
              onChange={(e) => setFormData({ ...formData, type: e.target.value as any })}
            >
              <MenuItem value="latency">Latency</MenuItem>
              <MenuItem value="jitter">Jitter</MenuItem>
              <MenuItem value="packet_loss">Packet Loss</MenuItem>
              <MenuItem value="bandwidth">Bandwidth</MenuItem>
              <MenuItem value="duplication">Duplication</MenuItem>
              <MenuItem value="reordering">Reordering</MenuItem>
              <MenuItem value="corruption">Corruption</MenuItem>
            </Select>
          </FormControl>
        </Grid>
        <Grid item xs={12} md={6}>
          <FormControl fullWidth>
            <InputLabel>Interface</InputLabel>
            <Select
              value={formData.interface}
              onChange={(e) => setFormData({ ...formData, interface: e.target.value })}
            >
              {interfaces.map((iface) => (
                <MenuItem key={iface.name} value={iface.name}>
                  {iface.name}
                </MenuItem>
              ))}
            </Select>
          </FormControl>
        </Grid>
        <Grid item xs={12} md={6}>
          <TextField
            fullWidth
            label="Source IP"
            value={formData.source_ip}
            onChange={(e) => setFormData({ ...formData, source_ip: e.target.value })}
          />
        </Grid>
        <Grid item xs={12} md={6}>
          <TextField
            fullWidth
            label="Destination IP"
            value={formData.destination_ip}
            onChange={(e) => setFormData({ ...formData, destination_ip: e.target.value })}
          />
        </Grid>
        <Grid item xs={12} md={6}>
          <FormControl fullWidth>
            <InputLabel>Protocol</InputLabel>
            <Select
              value={formData.protocol}
              onChange={(e) => setFormData({ ...formData, protocol: e.target.value })}
            >
              <MenuItem value="tcp">TCP</MenuItem>
              <MenuItem value="udp">UDP</MenuItem>
              <MenuItem value="icmp">ICMP</MenuItem>
              <MenuItem value="any">Any</MenuItem>
            </Select>
          </FormControl>
        </Grid>
        <Grid item xs={12} md={6}>
          <TextField
            fullWidth
            label="Port"
            type="number"
            value={formData.port}
            onChange={(e) => setFormData({ ...formData, port: parseInt(e.target.value) })}
          />
        </Grid>
        <Grid item xs={12} md={6}>
          <TextField
            fullWidth
            label="Value"
            type="number"
            value={formData.value}
            onChange={(e) => setFormData({ ...formData, value: parseFloat(e.target.value) })}
            helperText="Main impairment value (e.g., latency in ms, loss percentage)"
          />
        </Grid>
        <Grid item xs={12} md={6}>
          <TextField
            fullWidth
            label="Variation"
            type="number"
            value={formData.variation}
            onChange={(e) => setFormData({ ...formData, variation: parseFloat(e.target.value) })}
            helperText="Variation in the impairment value"
          />
        </Grid>
        <Grid item xs={12} md={6}>
          <TextField
            fullWidth
            label="Probability"
            type="number"
            inputProps={{ min: 0, max: 1, step: 0.01 }}
            value={formData.probability}
            onChange={(e) => setFormData({ ...formData, probability: parseFloat(e.target.value) })}
            helperText="Probability of applying the impairment (0.0 to 1.0)"
          />
        </Grid>
      </Grid>
      <DialogActions>
        <Button onClick={onCancel}>Cancel</Button>
        <Button type="submit" variant="contained">
          {rule ? 'Update' : 'Create'}
        </Button>
      </DialogActions>
    </Box>
  );
};

export default NetworkImpairments;
