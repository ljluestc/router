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
} from '@mui/material';
import {
  Add as AddIcon,
  Edit as EditIcon,
  Delete as DeleteIcon,
  PlayArrow as PlayIcon,
  Stop as StopIcon,
  Pause as PauseIcon,
  Refresh as RefreshIcon,
  Speed as SpeedIcon,
  NetworkCheck as NetworkCheckIcon,
  TrendingUp as TrendingUpIcon,
} from '@mui/icons-material';
import { useQuery, useMutation, useQueryClient } from '@tanstack/react-query';

interface TrafficRule {
  id: string;
  name: string;
  description: string;
  type: 'token_bucket' | 'wfq' | 'netem' | 'tc';
  status: 'active' | 'inactive' | 'paused';
  priority: number;
  interface: string;
  source_ip: string;
  destination_ip: string;
  protocol: string;
  port: number;
  bandwidth: number;
  burst_size: number;
  latency: number;
  jitter: number;
  packet_loss: number;
  created_at: string;
  updated_at: string;
}

interface TrafficStats {
  total_packets: number;
  total_bytes: number;
  packets_per_second: number;
  bytes_per_second: number;
  dropped_packets: number;
  dropped_bytes: number;
  average_latency: number;
  average_jitter: number;
  packet_loss_rate: number;
}

const TrafficShaping: React.FC = () => {
  const [selectedRule, setSelectedRule] = useState<TrafficRule | null>(null);
  const [ruleDialogOpen, setRuleDialogOpen] = useState(false);
  const [editingRule, setEditingRule] = useState<TrafficRule | null>(null);
  const [snackbarOpen, setSnackbarOpen] = useState(false);
  const [snackbarMessage, setSnackbarMessage] = useState('');
  const [snackbarSeverity, setSnackbarSeverity] = useState<'success' | 'error'>('success');

  const queryClient = useQueryClient();

  const { data: rules, isLoading } = useQuery({
    queryKey: ['traffic-rules'],
    queryFn: async () => {
      const response = await fetch('/api/v1/traffic/rules');
      return response.json();
    },
  });

  const { data: stats } = useQuery({
    queryKey: ['traffic-stats'],
    queryFn: async () => {
      const response = await fetch('/api/v1/traffic/stats');
      return response.json();
    },
  });

  const { data: interfaces } = useQuery({
    queryKey: ['traffic-interfaces'],
    queryFn: async () => {
      const response = await fetch('/api/v1/traffic/interfaces');
      return response.json();
    },
  });

  const createRuleMutation = useMutation({
    mutationFn: async (ruleData: Partial<TrafficRule>) => {
      const response = await fetch('/api/v1/traffic/rules', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify(ruleData),
      });
      return response.json();
    },
    onSuccess: () => {
      queryClient.invalidateQueries({ queryKey: ['traffic-rules'] });
      setSnackbarMessage('Traffic rule created successfully');
      setSnackbarOpen(true);
      setRuleDialogOpen(false);
    },
    onError: () => {
      setSnackbarMessage('Failed to create traffic rule');
      setSnackbarSeverity('error');
      setSnackbarOpen(true);
    },
  });

  const updateRuleMutation = useMutation({
    mutationFn: async ({ id, ...ruleData }: Partial<TrafficRule> & { id: string }) => {
      const response = await fetch(`/api/v1/traffic/rules/${id}`, {
        method: 'PUT',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify(ruleData),
      });
      return response.json();
    },
    onSuccess: () => {
      queryClient.invalidateQueries({ queryKey: ['traffic-rules'] });
      setSnackbarMessage('Traffic rule updated successfully');
      setSnackbarOpen(true);
      setRuleDialogOpen(false);
    },
    onError: () => {
      setSnackbarMessage('Failed to update traffic rule');
      setSnackbarSeverity('error');
      setSnackbarOpen(true);
    },
  });

  const deleteRuleMutation = useMutation({
    mutationFn: async (ruleId: string) => {
      const response = await fetch(`/api/v1/traffic/rules/${ruleId}`, {
        method: 'DELETE',
      });
      return response.json();
    },
    onSuccess: () => {
      queryClient.invalidateQueries({ queryKey: ['traffic-rules'] });
      setSnackbarMessage('Traffic rule deleted successfully');
      setSnackbarOpen(true);
    },
    onError: () => {
      setSnackbarMessage('Failed to delete traffic rule');
      setSnackbarSeverity('error');
      setSnackbarOpen(true);
    },
  });

  const startRuleMutation = useMutation({
    mutationFn: async (ruleId: string) => {
      const response = await fetch(`/api/v1/traffic/rules/${ruleId}/start`, {
        method: 'POST',
      });
      return response.json();
    },
    onSuccess: () => {
      queryClient.invalidateQueries({ queryKey: ['traffic-rules'] });
      setSnackbarMessage('Traffic rule started successfully');
      setSnackbarOpen(true);
    },
    onError: () => {
      setSnackbarMessage('Failed to start traffic rule');
      setSnackbarSeverity('error');
      setSnackbarOpen(true);
    },
  });

  const stopRuleMutation = useMutation({
    mutationFn: async (ruleId: string) => {
      const response = await fetch(`/api/v1/traffic/rules/${ruleId}/stop`, {
        method: 'POST',
      });
      return response.json();
    },
    onSuccess: () => {
      queryClient.invalidateQueries({ queryKey: ['traffic-rules'] });
      setSnackbarMessage('Traffic rule stopped successfully');
      setSnackbarOpen(true);
    },
    onError: () => {
      setSnackbarMessage('Failed to stop traffic rule');
      setSnackbarSeverity('error');
      setSnackbarOpen(true);
    },
  });

  const pauseRuleMutation = useMutation({
    mutationFn: async (ruleId: string) => {
      const response = await fetch(`/api/v1/traffic/rules/${ruleId}/pause`, {
        method: 'POST',
      });
      return response.json();
    },
    onSuccess: () => {
      queryClient.invalidateQueries({ queryKey: ['traffic-rules'] });
      setSnackbarMessage('Traffic rule paused successfully');
      setSnackbarOpen(true);
    },
    onError: () => {
      setSnackbarMessage('Failed to pause traffic rule');
      setSnackbarSeverity('error');
      setSnackbarOpen(true);
    },
  });

  const handleCreateRule = () => {
    setEditingRule(null);
    setRuleDialogOpen(true);
  };

  const handleEditRule = (rule: TrafficRule) => {
    setEditingRule(rule);
    setRuleDialogOpen(true);
  };

  const handleDeleteRule = (ruleId: string) => {
    if (window.confirm('Are you sure you want to delete this traffic rule?')) {
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

  const handleSaveRule = (ruleData: Partial<TrafficRule>) => {
    if (editingRule) {
      updateRuleMutation.mutate({ ...ruleData, id: editingRule.id });
    } else {
      createRuleMutation.mutate(ruleData);
    }
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

  const getRuleTypeLabel = (type: string) => {
    switch (type) {
      case 'token_bucket':
        return 'Token Bucket';
      case 'wfq':
        return 'Weighted Fair Queuing';
      case 'netem':
        return 'Network Emulation';
      case 'tc':
        return 'Traffic Control';
      default:
        return type;
    }
  };

  if (isLoading) {
    return (
      <Box display="flex" justifyContent="center" alignItems="center" minHeight="400px">
        <Typography>Loading traffic rules...</Typography>
      </Box>
    );
  }

  return (
    <Box>
      <Box display="flex" justifyContent="space-between" alignItems="center" mb={3}>
        <Typography variant="h4" component="h1">
          Traffic Shaping
        </Typography>
        <Box display="flex" gap={2}>
          <Button
            variant="outlined"
            startIcon={<RefreshIcon />}
            onClick={() => queryClient.invalidateQueries({ queryKey: ['traffic-rules'] })}
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

      <Grid container spacing={3}>
        {/* Traffic Statistics */}
        <Grid item xs={12} md={6}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>
                Traffic Statistics
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
                      Total Bytes
                    </Typography>
                    <Typography variant="h6">
                      {(stats.data.total_bytes / 1024 / 1024).toFixed(2)} MB
                    </Typography>
                  </Grid>
                  <Grid item xs={6}>
                    <Typography variant="body2" color="text.secondary">
                      Packets/sec
                    </Typography>
                    <Typography variant="h6">
                      {stats.data.packets_per_second?.toFixed(0)}
                    </Typography>
                  </Grid>
                  <Grid item xs={6}>
                    <Typography variant="body2" color="text.secondary">
                      Bytes/sec
                    </Typography>
                    <Typography variant="h6">
                      {(stats.data.bytes_per_second / 1024).toFixed(0)} KB/s
                    </Typography>
                  </Grid>
                  <Grid item xs={6}>
                    <Typography variant="body2" color="text.secondary">
                      Dropped Packets
                    </Typography>
                    <Typography variant="h6" color="error">
                      {stats.data.dropped_packets?.toLocaleString()}
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
                      Average Latency
                    </Typography>
                    <Typography variant="h6">
                      {stats.data.average_latency?.toFixed(2)} ms
                    </Typography>
                    <LinearProgress
                      variant="determinate"
                      value={Math.min((stats.data.average_latency / 100) * 100, 100)}
                      sx={{ mt: 1 }}
                    />
                  </Grid>
                  <Grid item xs={12}>
                    <Typography variant="body2" color="text.secondary">
                      Average Jitter
                    </Typography>
                    <Typography variant="h6">
                      {stats.data.average_jitter?.toFixed(2)} ms
                    </Typography>
                    <LinearProgress
                      variant="determinate"
                      value={Math.min((stats.data.average_jitter / 50) * 100, 100)}
                      sx={{ mt: 1 }}
                    />
                  </Grid>
                  <Grid item xs={12}>
                    <Typography variant="body2" color="text.secondary">
                      Dropped Bytes
                    </Typography>
                    <Typography variant="h6" color="error">
                      {(stats.data.dropped_bytes / 1024 / 1024).toFixed(2)} MB
                    </Typography>
                    <LinearProgress
                      variant="determinate"
                      value={Math.min((stats.data.dropped_bytes / stats.data.total_bytes) * 100, 100)}
                      sx={{ mt: 1 }}
                    />
                  </Grid>
                </Grid>
              )}
            </CardContent>
          </Card>
        </Grid>

        {/* Traffic Rules */}
        <Grid item xs={12}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>
                Traffic Rules
              </Typography>
              {rules?.data && rules.data.length > 0 ? (
                <List>
                  {rules.data.map((rule: TrafficRule) => (
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
                                label={getRuleTypeLabel(rule.type)}
                                size="small"
                                variant="outlined"
                              />
                              <Chip
                                label={`Priority: ${rule.priority}`}
                                size="small"
                                variant="outlined"
                              />
                              <Chip
                                label={`Bandwidth: ${rule.bandwidth} Mbps`}
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
                    No traffic rules configured. Create your first rule to get started.
                  </Typography>
                </Box>
              )}
            </CardContent>
          </Card>
        </Grid>
      </Grid>

      {/* Rule Dialog */}
      <Dialog
        open={ruleDialogOpen}
        onClose={() => setRuleDialogOpen(false)}
        maxWidth="md"
        fullWidth
      >
        <DialogTitle>
          {editingRule ? 'Edit Traffic Rule' : 'Create Traffic Rule'}
        </DialogTitle>
        <DialogContent>
          <TrafficRuleForm
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

interface TrafficRuleFormProps {
  rule: TrafficRule | null;
  onSave: (data: Partial<TrafficRule>) => void;
  onCancel: () => void;
  interfaces: any[];
}

const TrafficRuleForm: React.FC<TrafficRuleFormProps> = ({ rule, onSave, onCancel, interfaces }) => {
  const [formData, setFormData] = useState({
    name: rule?.name || '',
    description: rule?.description || '',
    type: rule?.type || 'token_bucket',
    status: rule?.status || 'inactive',
    priority: rule?.priority || 1,
    interface: rule?.interface || '',
    source_ip: rule?.source_ip || '',
    destination_ip: rule?.destination_ip || '',
    protocol: rule?.protocol || 'tcp',
    port: rule?.port || 80,
    bandwidth: rule?.bandwidth || 100,
    burst_size: rule?.burst_size || 1000,
    latency: rule?.latency || 0,
    jitter: rule?.jitter || 0,
    packet_loss: rule?.packet_loss || 0,
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
            <InputLabel>Rule Type</InputLabel>
            <Select
              value={formData.type}
              onChange={(e) => setFormData({ ...formData, type: e.target.value as any })}
            >
              <MenuItem value="token_bucket">Token Bucket</MenuItem>
              <MenuItem value="wfq">Weighted Fair Queuing</MenuItem>
              <MenuItem value="netem">Network Emulation</MenuItem>
              <MenuItem value="tc">Traffic Control</MenuItem>
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
            label="Bandwidth (Mbps)"
            type="number"
            value={formData.bandwidth}
            onChange={(e) => setFormData({ ...formData, bandwidth: parseInt(e.target.value) })}
          />
        </Grid>
        <Grid item xs={12} md={6}>
          <TextField
            fullWidth
            label="Burst Size (bytes)"
            type="number"
            value={formData.burst_size}
            onChange={(e) => setFormData({ ...formData, burst_size: parseInt(e.target.value) })}
          />
        </Grid>
        <Grid item xs={12} md={6}>
          <TextField
            fullWidth
            label="Latency (ms)"
            type="number"
            value={formData.latency}
            onChange={(e) => setFormData({ ...formData, latency: parseInt(e.target.value) })}
          />
        </Grid>
        <Grid item xs={12} md={6}>
          <TextField
            fullWidth
            label="Jitter (ms)"
            type="number"
            value={formData.jitter}
            onChange={(e) => setFormData({ ...formData, jitter: parseInt(e.target.value) })}
          />
        </Grid>
        <Grid item xs={12} md={6}>
          <TextField
            fullWidth
            label="Packet Loss (%)"
            type="number"
            value={formData.packet_loss}
            onChange={(e) => setFormData({ ...formData, packet_loss: parseInt(e.target.value) })}
          />
        </Grid>
        <Grid item xs={12} md={6}>
          <TextField
            fullWidth
            label="Priority"
            type="number"
            value={formData.priority}
            onChange={(e) => setFormData({ ...formData, priority: parseInt(e.target.value) })}
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

export default TrafficShaping;
