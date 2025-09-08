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
  Alert,
  Snackbar,
  Paper,
  Divider,
} from '@mui/material';
import {
  PlayArrow as PlayIcon,
  Stop as StopIcon,
  Edit as EditIcon,
  Delete as DeleteIcon,
  Add as AddIcon,
  Refresh as RefreshIcon,
  Download as DownloadIcon,
  Upload as UploadIcon,
  Visibility as ViewIcon,
} from '@mui/icons-material';
import { useQuery, useMutation, useQueryClient } from '@tanstack/react-query';

interface Scenario {
  id: string;
  name: string;
  description: string;
  status: 'stopped' | 'running' | 'paused' | 'error';
  config: any;
  created_at: string;
  updated_at: string;
}

const Scenarios: React.FC = () => {
  const [selectedScenario, setSelectedScenario] = useState<Scenario | null>(null);
  const [dialogOpen, setDialogOpen] = useState(false);
  const [editingScenario, setEditingScenario] = useState<Scenario | null>(null);
  const [snackbarOpen, setSnackbarOpen] = useState(false);
  const [snackbarMessage, setSnackbarMessage] = useState('');
  const [snackbarSeverity, setSnackbarSeverity] = useState<'success' | 'error'>('success');

  const queryClient = useQueryClient();

  const { data: scenarios, isLoading } = useQuery({
    queryKey: ['scenarios'],
    queryFn: async () => {
      const response = await fetch('/api/v1/scenarios');
      return response.json();
    },
  });

  const { data: scenarioTemplates } = useQuery({
    queryKey: ['scenario-templates'],
    queryFn: async () => {
      const response = await fetch('/api/v1/scenarios/templates');
      return response.json();
    },
  });

  const startScenarioMutation = useMutation({
    mutationFn: async (scenarioId: string) => {
      const response = await fetch(`/api/v1/scenarios/${scenarioId}/start`, {
        method: 'POST',
      });
      return response.json();
    },
    onSuccess: () => {
      queryClient.invalidateQueries({ queryKey: ['scenarios'] });
      setSnackbarMessage('Scenario started successfully');
      setSnackbarOpen(true);
    },
    onError: () => {
      setSnackbarMessage('Failed to start scenario');
      setSnackbarSeverity('error');
      setSnackbarOpen(true);
    },
  });

  const stopScenarioMutation = useMutation({
    mutationFn: async (scenarioId: string) => {
      const response = await fetch(`/api/v1/scenarios/${scenarioId}/stop`, {
        method: 'POST',
      });
      return response.json();
    },
    onSuccess: () => {
      queryClient.invalidateQueries({ queryKey: ['scenarios'] });
      setSnackbarMessage('Scenario stopped successfully');
      setSnackbarOpen(true);
    },
    onError: () => {
      setSnackbarMessage('Failed to stop scenario');
      setSnackbarSeverity('error');
      setSnackbarOpen(true);
    },
  });

  const deleteScenarioMutation = useMutation({
    mutationFn: async (scenarioId: string) => {
      const response = await fetch(`/api/v1/scenarios/${scenarioId}`, {
        method: 'DELETE',
      });
      return response.json();
    },
    onSuccess: () => {
      queryClient.invalidateQueries({ queryKey: ['scenarios'] });
      setSnackbarMessage('Scenario deleted successfully');
      setSnackbarOpen(true);
    },
    onError: () => {
      setSnackbarMessage('Failed to delete scenario');
      setSnackbarSeverity('error');
      setSnackbarOpen(true);
    },
  });

  const createScenarioMutation = useMutation({
    mutationFn: async (scenarioData: Partial<Scenario>) => {
      const response = await fetch('/api/v1/scenarios', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify(scenarioData),
      });
      return response.json();
    },
    onSuccess: () => {
      queryClient.invalidateQueries({ queryKey: ['scenarios'] });
      setSnackbarMessage('Scenario created successfully');
      setSnackbarOpen(true);
      setDialogOpen(false);
    },
    onError: () => {
      setSnackbarMessage('Failed to create scenario');
      setSnackbarSeverity('error');
      setSnackbarOpen(true);
    },
  });

  const updateScenarioMutation = useMutation({
    mutationFn: async ({ id, ...scenarioData }: Partial<Scenario> & { id: string }) => {
      const response = await fetch(`/api/v1/scenarios/${id}`, {
        method: 'PUT',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify(scenarioData),
      });
      return response.json();
    },
    onSuccess: () => {
      queryClient.invalidateQueries({ queryKey: ['scenarios'] });
      setSnackbarMessage('Scenario updated successfully');
      setSnackbarOpen(true);
      setDialogOpen(false);
    },
    onError: () => {
      setSnackbarMessage('Failed to update scenario');
      setSnackbarSeverity('error');
      setSnackbarOpen(true);
    },
  });

  const handleStartScenario = (scenarioId: string) => {
    startScenarioMutation.mutate(scenarioId);
  };

  const handleStopScenario = (scenarioId: string) => {
    stopScenarioMutation.mutate(scenarioId);
  };

  const handleDeleteScenario = (scenarioId: string) => {
    if (window.confirm('Are you sure you want to delete this scenario?')) {
      deleteScenarioMutation.mutate(scenarioId);
    }
  };

  const handleEditScenario = (scenario: Scenario) => {
    setEditingScenario(scenario);
    setDialogOpen(true);
  };

  const handleCreateScenario = () => {
    setEditingScenario(null);
    setDialogOpen(true);
  };

  const handleSaveScenario = (scenarioData: Partial<Scenario>) => {
    if (editingScenario) {
      updateScenarioMutation.mutate({ ...scenarioData, id: editingScenario.id });
    } else {
      createScenarioMutation.mutate(scenarioData);
    }
  };

  const handleExportScenario = (scenario: Scenario) => {
    const dataStr = JSON.stringify(scenario, null, 2);
    const dataUri = 'data:application/json;charset=utf-8,'+ encodeURIComponent(dataStr);
    const exportFileDefaultName = `${scenario.name}-scenario.json`;
    const linkElement = document.createElement('a');
    linkElement.setAttribute('href', dataUri);
    linkElement.setAttribute('download', exportFileDefaultName);
    linkElement.click();
  };

  const handleImportScenario = (event: React.ChangeEvent<HTMLInputElement>) => {
    const file = event.target.files?.[0];
    if (file) {
      const reader = new FileReader();
      reader.onload = (e) => {
        try {
          const scenario = JSON.parse(e.target?.result as string);
          createScenarioMutation.mutate(scenario);
        } catch (error) {
          setSnackbarMessage('Invalid scenario file');
          setSnackbarSeverity('error');
          setSnackbarOpen(true);
        }
      };
      reader.readAsText(file);
    }
  };

  const getStatusColor = (status: string) => {
    switch (status) {
      case 'running':
        return 'success';
      case 'stopped':
        return 'default';
      case 'paused':
        return 'warning';
      case 'error':
        return 'error';
      default:
        return 'default';
    }
  };

  const getStatusIcon = (status: string) => {
    switch (status) {
      case 'running':
        return <PlayIcon />;
      case 'stopped':
        return <StopIcon />;
      case 'paused':
        return <StopIcon />;
      case 'error':
        return <StopIcon />;
      default:
        return <StopIcon />;
    }
  };

  if (isLoading) {
    return (
      <Box display="flex" justifyContent="center" alignItems="center" minHeight="400px">
        <Typography>Loading scenarios...</Typography>
      </Box>
    );
  }

  return (
    <Box>
      <Box display="flex" justifyContent="space-between" alignItems="center" mb={3}>
        <Typography variant="h4" component="h1">
          Scenarios
        </Typography>
        <Box display="flex" gap={2}>
          <input
            accept=".json"
            style={{ display: 'none' }}
            id="import-scenario-file"
            type="file"
            onChange={handleImportScenario}
          />
          <label htmlFor="import-scenario-file">
            <Button
              variant="outlined"
              component="span"
              startIcon={<UploadIcon />}
            >
              Import
            </Button>
          </label>
          <Button
            variant="contained"
            startIcon={<AddIcon />}
            onClick={handleCreateScenario}
          >
            Create Scenario
          </Button>
        </Box>
      </Box>

      <Grid container spacing={3}>
        {/* Scenarios List */}
        <Grid item xs={12} md={8}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>
                Available Scenarios
              </Typography>
              {scenarios?.data && scenarios.data.length > 0 ? (
                <List>
                  {scenarios.data.map((scenario: Scenario) => (
                    <ListItem key={scenario.id} divider>
                      <ListItemText
                        primary={scenario.name}
                        secondary={
                          <Box>
                            <Typography variant="body2" color="text.secondary">
                              {scenario.description}
                            </Typography>
                            <Box display="flex" gap={1} mt={1}>
                              <Chip
                                icon={getStatusIcon(scenario.status)}
                                label={scenario.status}
                                color={getStatusColor(scenario.status) as any}
                                size="small"
                              />
                              <Chip
                                label={`Created: ${new Date(scenario.created_at).toLocaleDateString()}`}
                                size="small"
                                variant="outlined"
                              />
                            </Box>
                          </Box>
                        }
                      />
                      <ListItemSecondaryAction>
                        <Box display="flex" gap={1}>
                          {scenario.status === 'running' ? (
                            <IconButton
                              onClick={() => handleStopScenario(scenario.id)}
                              color="error"
                            >
                              <StopIcon />
                            </IconButton>
                          ) : (
                            <IconButton
                              onClick={() => handleStartScenario(scenario.id)}
                              color="primary"
                            >
                              <PlayIcon />
                            </IconButton>
                          )}
                          <IconButton
                            onClick={() => handleEditScenario(scenario)}
                            color="primary"
                          >
                            <EditIcon />
                          </IconButton>
                          <IconButton
                            onClick={() => handleExportScenario(scenario)}
                            color="primary"
                          >
                            <DownloadIcon />
                          </IconButton>
                          <IconButton
                            onClick={() => handleDeleteScenario(scenario.id)}
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
                    No scenarios available. Create your first scenario to get started.
                  </Typography>
                </Box>
              )}
            </CardContent>
          </Card>
        </Grid>

        {/* Scenario Templates */}
        <Grid item xs={12} md={4}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>
                Scenario Templates
              </Typography>
              {scenarioTemplates?.data && scenarioTemplates.data.length > 0 ? (
                <List>
                  {scenarioTemplates.data.map((template: any, index: number) => (
                    <ListItem key={index} button>
                      <ListItemText
                        primary={template.name}
                        secondary={template.description}
                      />
                    </ListItem>
                  ))}
                </List>
              ) : (
                <Typography variant="body2" color="text.secondary">
                  No templates available
                </Typography>
              )}
            </CardContent>
          </Card>
        </Grid>
      </Grid>

      {/* Scenario Dialog */}
      <Dialog
        open={dialogOpen}
        onClose={() => setDialogOpen(false)}
        maxWidth="md"
        fullWidth
      >
        <DialogTitle>
          {editingScenario ? 'Edit Scenario' : 'Create Scenario'}
        </DialogTitle>
        <DialogContent>
          <ScenarioForm
            scenario={editingScenario}
            onSave={handleSaveScenario}
            onCancel={() => setDialogOpen(false)}
            templates={scenarioTemplates?.data || []}
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

interface ScenarioFormProps {
  scenario: Scenario | null;
  onSave: (data: Partial<Scenario>) => void;
  onCancel: () => void;
  templates: any[];
}

const ScenarioForm: React.FC<ScenarioFormProps> = ({ scenario, onSave, onCancel, templates }) => {
  const [formData, setFormData] = useState({
    name: scenario?.name || '',
    description: scenario?.description || '',
    config: scenario?.config || {},
    template: '',
  });

  const handleSubmit = (e: React.FormEvent) => {
    e.preventDefault();
    onSave(formData);
  };

  const handleTemplateChange = (templateId: string) => {
    const template = templates.find(t => t.id === templateId);
    if (template) {
      setFormData({
        ...formData,
        template: templateId,
        config: template.config,
        description: template.description,
      });
    }
  };

  return (
    <Box component="form" onSubmit={handleSubmit}>
      <Grid container spacing={2} sx={{ mt: 1 }}>
        <Grid item xs={12}>
          <TextField
            fullWidth
            label="Scenario Name"
            value={formData.name}
            onChange={(e) => setFormData({ ...formData, name: e.target.value })}
            required
          />
        </Grid>
        <Grid item xs={12}>
          <TextField
            fullWidth
            label="Description"
            multiline
            rows={3}
            value={formData.description}
            onChange={(e) => setFormData({ ...formData, description: e.target.value })}
          />
        </Grid>
        <Grid item xs={12}>
          <FormControl fullWidth>
            <InputLabel>Template</InputLabel>
            <Select
              value={formData.template}
              onChange={(e) => handleTemplateChange(e.target.value)}
            >
              <MenuItem value="">None</MenuItem>
              {templates.map((template) => (
                <MenuItem key={template.id} value={template.id}>
                  {template.name}
                </MenuItem>
              ))}
            </Select>
          </FormControl>
        </Grid>
        <Grid item xs={12}>
          <TextField
            fullWidth
            label="Configuration (YAML)"
            multiline
            rows={10}
            value={typeof formData.config === 'string' ? formData.config : JSON.stringify(formData.config, null, 2)}
            onChange={(e) => {
              try {
                const config = JSON.parse(e.target.value);
                setFormData({ ...formData, config });
              } catch {
                setFormData({ ...formData, config: e.target.value });
              }
            }}
          />
        </Grid>
      </Grid>
      <DialogActions>
        <Button onClick={onCancel}>Cancel</Button>
        <Button type="submit" variant="contained">
          {scenario ? 'Update' : 'Create'}
        </Button>
      </DialogActions>
    </Box>
  );
};

export default Scenarios;
