import React, { useState } from 'react';
import {
  Box,
  Typography,
  Card,
  CardContent,
  Grid,
  TextField,
  Button,
  Switch,
  FormControlLabel,
  Divider,
  Alert,
  Snackbar,
  Tabs,
  Tab,
  Paper,
  List,
  ListItem,
  ListItemText,
  ListItemSecondaryAction,
  IconButton,
  Dialog,
  DialogTitle,
  DialogContent,
  DialogActions,
} from '@mui/material';
import {
  Save as SaveIcon,
  Refresh as RefreshIcon,
  Add as AddIcon,
  Edit as EditIcon,
  Delete as DeleteIcon,
  CloudDownload as CloudDownloadIcon,
  CloudUpload as CloudUploadIcon,
} from '@mui/icons-material';
import { useQuery, useMutation, useQueryClient } from '@tanstack/react-query';

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
      id={`settings-tabpanel-${index}`}
      aria-labelledby={`settings-tab-${index}`}
      {...other}
    >
      {value === index && <Box sx={{ p: 3 }}>{children}</Box>}
    </div>
  );
}

const Settings: React.FC = () => {
  const [tabValue, setTabValue] = useState(0);
  const [snackbarOpen, setSnackbarOpen] = useState(false);
  const [snackbarMessage, setSnackbarMessage] = useState('');
  const [configDialogOpen, setConfigDialogOpen] = useState(false);
  const [editingConfig, setEditingConfig] = useState<any>(null);

  const queryClient = useQueryClient();

  const { data: config } = useQuery({
    queryKey: ['settings-config'],
    queryFn: async () => {
      const response = await fetch('/api/v1/settings');
      return response.json();
    },
  });

  const { data: scenarios } = useQuery({
    queryKey: ['settings-scenarios'],
    queryFn: async () => {
      const response = await fetch('/api/v1/scenarios');
      return response.json();
    },
  });

  const { data: cloudpodsConfig } = useQuery({
    queryKey: ['settings-cloudpods'],
    queryFn: async () => {
      const response = await fetch('/api/v1/cloudpods/config');
      return response.json();
    },
  });

  const { data: aviatrixConfig } = useQuery({
    queryKey: ['settings-aviatrix'],
    queryFn: async () => {
      const response = await fetch('/api/v1/aviatrix/config');
      return response.json();
    },
  });

  const updateConfigMutation = useMutation({
    mutationFn: async (newConfig: any) => {
      const response = await fetch('/api/v1/settings', {
        method: 'PUT',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify(newConfig),
      });
      return response.json();
    },
    onSuccess: () => {
      queryClient.invalidateQueries({ queryKey: ['settings-config'] });
      setSnackbarMessage('Configuration updated successfully');
      setSnackbarOpen(true);
    },
  });

  const updateCloudPodsMutation = useMutation({
    mutationFn: async (newConfig: any) => {
      const response = await fetch('/api/v1/cloudpods/config', {
        method: 'PUT',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify(newConfig),
      });
      return response.json();
    },
    onSuccess: () => {
      queryClient.invalidateQueries({ queryKey: ['settings-cloudpods'] });
      setSnackbarMessage('CloudPods configuration updated successfully');
      setSnackbarOpen(true);
    },
  });

  const updateAviatrixMutation = useMutation({
    mutationFn: async (newConfig: any) => {
      const response = await fetch('/api/v1/aviatrix/config', {
        method: 'PUT',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify(newConfig),
      });
      return response.json();
    },
    onSuccess: () => {
      queryClient.invalidateQueries({ queryKey: ['settings-aviatrix'] });
      setSnackbarMessage('Aviatrix configuration updated successfully');
      setSnackbarOpen(true);
    },
  });

  const handleTabChange = (event: React.SyntheticEvent, newValue: number) => {
    setTabValue(newValue);
  };

  const handleSaveConfig = () => {
    // Implementation for saving configuration
    updateConfigMutation.mutate(config);
  };

  const handleSaveCloudPods = () => {
    // Implementation for saving CloudPods configuration
    updateCloudPodsMutation.mutate(cloudpodsConfig);
  };

  const handleSaveAviatrix = () => {
    // Implementation for saving Aviatrix configuration
    updateAviatrixMutation.mutate(aviatrixConfig);
  };

  const handleExportConfig = () => {
    const dataStr = JSON.stringify(config, null, 2);
    const dataUri = 'data:application/json;charset=utf-8,'+ encodeURIComponent(dataStr);
    const exportFileDefaultName = 'router-config.json';
    const linkElement = document.createElement('a');
    linkElement.setAttribute('href', dataUri);
    linkElement.setAttribute('download', exportFileDefaultName);
    linkElement.click();
  };

  const handleImportConfig = (event: React.ChangeEvent<HTMLInputElement>) => {
    const file = event.target.files?.[0];
    if (file) {
      const reader = new FileReader();
      reader.onload = (e) => {
        try {
          const config = JSON.parse(e.target?.result as string);
          updateConfigMutation.mutate(config);
        } catch (error) {
          setSnackbarMessage('Invalid configuration file');
          setSnackbarOpen(true);
        }
      };
      reader.readAsText(file);
    }
  };

  return (
    <Box>
      <Typography variant="h4" component="h1" gutterBottom>
        Settings
      </Typography>

      <Box sx={{ borderBottom: 1, borderColor: 'divider' }}>
        <Tabs value={tabValue} onChange={handleTabChange} aria-label="settings tabs">
          <Tab label="General" />
          <Tab label="CloudPods" />
          <Tab label="Aviatrix" />
          <Tab label="Scenarios" />
          <Tab label="Import/Export" />
        </Tabs>
      </Box>

      {/* General Settings */}
      <TabPanel value={tabValue} index={0}>
        <Grid container spacing={3}>
          <Grid item xs={12} md={6}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  API Settings
                </Typography>
                <Grid container spacing={2}>
                  <Grid item xs={12}>
                    <TextField
                      fullWidth
                      label="API Host"
                      value={config?.data?.api?.host || ''}
                      onChange={(e) => {
                        // Handle change
                      }}
                    />
                  </Grid>
                  <Grid item xs={12}>
                    <TextField
                      fullWidth
                      label="API Port"
                      type="number"
                      value={config?.data?.api?.port || ''}
                      onChange={(e) => {
                        // Handle change
                      }}
                    />
                  </Grid>
                  <Grid item xs={12}>
                    <TextField
                      fullWidth
                      label="Read Timeout"
                      value={config?.data?.api?.read_timeout || ''}
                      onChange={(e) => {
                        // Handle change
                      }}
                    />
                  </Grid>
                  <Grid item xs={12}>
                    <TextField
                      fullWidth
                      label="Write Timeout"
                      value={config?.data?.api?.write_timeout || ''}
                      onChange={(e) => {
                        // Handle change
                      }}
                    />
                  </Grid>
                </Grid>
              </CardContent>
            </Card>
          </Grid>

          <Grid item xs={12} md={6}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  Logging Settings
                </Typography>
                <Grid container spacing={2}>
                  <Grid item xs={12}>
                    <TextField
                      fullWidth
                      label="Log Level"
                      select
                      SelectProps={{ native: true }}
                      value={config?.data?.logging?.level || 'info'}
                    >
                      <option value="debug">Debug</option>
                      <option value="info">Info</option>
                      <option value="warn">Warn</option>
                      <option value="error">Error</option>
                    </TextField>
                  </Grid>
                  <Grid item xs={12}>
                    <TextField
                      fullWidth
                      label="Log File"
                      value={config?.data?.logging?.file || ''}
                      onChange={(e) => {
                        // Handle change
                      }}
                    />
                  </Grid>
                  <Grid item xs={12}>
                    <FormControlLabel
                      control={
                        <Switch
                          checked={config?.data?.logging?.console || false}
                          onChange={(e) => {
                            // Handle change
                          }}
                        />
                      }
                      label="Console Logging"
                    />
                  </Grid>
                </Grid>
              </CardContent>
            </Card>
          </Grid>

          <Grid item xs={12}>
            <Box display="flex" gap={2}>
              <Button
                variant="contained"
                startIcon={<SaveIcon />}
                onClick={handleSaveConfig}
                disabled={updateConfigMutation.isPending}
              >
                Save Configuration
              </Button>
              <Button
                variant="outlined"
                startIcon={<RefreshIcon />}
                onClick={() => queryClient.invalidateQueries({ queryKey: ['settings-config'] })}
              >
                Refresh
              </Button>
            </Box>
          </Grid>
        </Grid>
      </TabPanel>

      {/* CloudPods Settings */}
      <TabPanel value={tabValue} index={1}>
        <Grid container spacing={3}>
          <Grid item xs={12} md={6}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  CloudPods Configuration
                </Typography>
                <Grid container spacing={2}>
                  <Grid item xs={12}>
                    <TextField
                      fullWidth
                      label="Endpoint"
                      value={cloudpodsConfig?.data?.endpoint || ''}
                      onChange={(e) => {
                        // Handle change
                      }}
                    />
                  </Grid>
                  <Grid item xs={12}>
                    <TextField
                      fullWidth
                      label="Username"
                      value={cloudpodsConfig?.data?.username || ''}
                      onChange={(e) => {
                        // Handle change
                      }}
                    />
                  </Grid>
                  <Grid item xs={12}>
                    <TextField
                      fullWidth
                      label="Password"
                      type="password"
                      value={cloudpodsConfig?.data?.password || ''}
                      onChange={(e) => {
                        // Handle change
                      }}
                    />
                  </Grid>
                  <Grid item xs={12}>
                    <TextField
                      fullWidth
                      label="Region"
                      value={cloudpodsConfig?.data?.region || ''}
                      onChange={(e) => {
                        // Handle change
                      }}
                    />
                  </Grid>
                  <Grid item xs={12}>
                    <TextField
                      fullWidth
                      label="Project"
                      value={cloudpodsConfig?.data?.project || ''}
                      onChange={(e) => {
                        // Handle change
                      }}
                    />
                  </Grid>
                  <Grid item xs={12}>
                    <FormControlLabel
                      control={
                        <Switch
                          checked={cloudpodsConfig?.data?.verify_ssl || false}
                          onChange={(e) => {
                            // Handle change
                          }}
                        />
                      }
                      label="Verify SSL"
                    />
                  </Grid>
                </Grid>
              </CardContent>
            </Card>
          </Grid>

          <Grid item xs={12} md={6}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  Connection Settings
                </Typography>
                <Grid container spacing={2}>
                  <Grid item xs={12}>
                    <TextField
                      fullWidth
                      label="Timeout"
                      value={cloudpodsConfig?.data?.timeout || ''}
                      onChange={(e) => {
                        // Handle change
                      }}
                    />
                  </Grid>
                  <Grid item xs={12}>
                    <TextField
                      fullWidth
                      label="Retry Count"
                      type="number"
                      value={cloudpodsConfig?.data?.retry_count || ''}
                      onChange={(e) => {
                        // Handle change
                      }}
                    />
                  </Grid>
                  <Grid item xs={12}>
                    <TextField
                      fullWidth
                      label="Retry Delay"
                      value={cloudpodsConfig?.data?.retry_delay || ''}
                      onChange={(e) => {
                        // Handle change
                      }}
                    />
                  </Grid>
                </Grid>
              </CardContent>
            </Card>
          </Grid>

          <Grid item xs={12}>
            <Box display="flex" gap={2}>
              <Button
                variant="contained"
                startIcon={<SaveIcon />}
                onClick={handleSaveCloudPods}
                disabled={updateCloudPodsMutation.isPending}
              >
                Save CloudPods Configuration
              </Button>
              <Button
                variant="outlined"
                startIcon={<RefreshIcon />}
                onClick={() => queryClient.invalidateQueries({ queryKey: ['settings-cloudpods'] })}
              >
                Refresh
              </Button>
            </Box>
          </Grid>
        </Grid>
      </TabPanel>

      {/* Aviatrix Settings */}
      <TabPanel value={tabValue} index={2}>
        <Grid container spacing={3}>
          <Grid item xs={12} md={6}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  Aviatrix Configuration
                </Typography>
                <Grid container spacing={2}>
                  <Grid item xs={12}>
                    <TextField
                      fullWidth
                      label="Controller IP"
                      value={aviatrixConfig?.data?.controller_ip || ''}
                      onChange={(e) => {
                        // Handle change
                      }}
                    />
                  </Grid>
                  <Grid item xs={12}>
                    <TextField
                      fullWidth
                      label="Username"
                      value={aviatrixConfig?.data?.username || ''}
                      onChange={(e) => {
                        // Handle change
                      }}
                    />
                  </Grid>
                  <Grid item xs={12}>
                    <TextField
                      fullWidth
                      label="Password"
                      type="password"
                      value={aviatrixConfig?.data?.password || ''}
                      onChange={(e) => {
                        // Handle change
                      }}
                    />
                  </Grid>
                  <Grid item xs={12}>
                    <TextField
                      fullWidth
                      label="API Version"
                      value={aviatrixConfig?.data?.api_version || ''}
                      onChange={(e) => {
                        // Handle change
                      }}
                    />
                  </Grid>
                  <Grid item xs={12}>
                    <TextField
                      fullWidth
                      label="Region"
                      value={aviatrixConfig?.data?.region || ''}
                      onChange={(e) => {
                        // Handle change
                      }}
                    />
                  </Grid>
                  <Grid item xs={12}>
                    <FormControlLabel
                      control={
                        <Switch
                          checked={aviatrixConfig?.data?.verify_ssl || false}
                          onChange={(e) => {
                            // Handle change
                          }}
                        />
                      }
                      label="Verify SSL"
                    />
                  </Grid>
                </Grid>
              </CardContent>
            </Card>
          </Grid>

          <Grid item xs={12} md={6}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  Connection Settings
                </Typography>
                <Grid container spacing={2}>
                  <Grid item xs={12}>
                    <TextField
                      fullWidth
                      label="Timeout"
                      value={aviatrixConfig?.data?.timeout || ''}
                      onChange={(e) => {
                        // Handle change
                      }}
                    />
                  </Grid>
                  <Grid item xs={12}>
                    <TextField
                      fullWidth
                      label="Retry Count"
                      type="number"
                      value={aviatrixConfig?.data?.retry_count || ''}
                      onChange={(e) => {
                        // Handle change
                      }}
                    />
                  </Grid>
                  <Grid item xs={12}>
                    <TextField
                      fullWidth
                      label="Retry Delay"
                      value={aviatrixConfig?.data?.retry_delay || ''}
                      onChange={(e) => {
                        // Handle change
                      }}
                    />
                  </Grid>
                </Grid>
              </CardContent>
            </Card>
          </Grid>

          <Grid item xs={12}>
            <Box display="flex" gap={2}>
              <Button
                variant="contained"
                startIcon={<SaveIcon />}
                onClick={handleSaveAviatrix}
                disabled={updateAviatrixMutation.isPending}
              >
                Save Aviatrix Configuration
              </Button>
              <Button
                variant="outlined"
                startIcon={<RefreshIcon />}
                onClick={() => queryClient.invalidateQueries({ queryKey: ['settings-aviatrix'] })}
              >
                Refresh
              </Button>
            </Box>
          </Grid>
        </Grid>
      </TabPanel>

      {/* Scenarios Settings */}
      <TabPanel value={tabValue} index={3}>
        <Grid container spacing={3}>
          <Grid item xs={12}>
            <Card>
              <CardContent>
                <Box display="flex" justifyContent="space-between" alignItems="center" mb={2}>
                  <Typography variant="h6">
                    Available Scenarios
                  </Typography>
                  <Button
                    variant="contained"
                    startIcon={<AddIcon />}
                    onClick={() => setConfigDialogOpen(true)}
                  >
                    Add Scenario
                  </Button>
                </Box>
                {scenarios?.data && (
                  <List>
                    {scenarios.data.map((scenario: any, index: number) => (
                      <ListItem key={index}>
                        <ListItemText
                          primary={scenario.name}
                          secondary={scenario.description}
                        />
                        <ListItemSecondaryAction>
                          <IconButton
                            edge="end"
                            onClick={() => {
                              setEditingConfig(scenario);
                              setConfigDialogOpen(true);
                            }}
                          >
                            <EditIcon />
                          </IconButton>
                          <IconButton
                            edge="end"
                            onClick={() => {
                              // Handle delete
                            }}
                          >
                            <DeleteIcon />
                          </IconButton>
                        </ListItemSecondaryAction>
                      </ListItem>
                    ))}
                  </List>
                )}
              </CardContent>
            </Card>
          </Grid>
        </Grid>
      </TabPanel>

      {/* Import/Export Settings */}
      <TabPanel value={tabValue} index={4}>
        <Grid container spacing={3}>
          <Grid item xs={12} md={6}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  Export Configuration
                </Typography>
                <Typography variant="body2" color="text.secondary" paragraph>
                  Export your current configuration to a JSON file for backup or sharing.
                </Typography>
                <Button
                  variant="contained"
                  startIcon={<CloudDownloadIcon />}
                  onClick={handleExportConfig}
                >
                  Export Configuration
                </Button>
              </CardContent>
            </Card>
          </Grid>

          <Grid item xs={12} md={6}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  Import Configuration
                </Typography>
                <Typography variant="body2" color="text.secondary" paragraph>
                  Import a configuration from a JSON file to restore settings.
                </Typography>
                <input
                  accept=".json"
                  style={{ display: 'none' }}
                  id="import-config-file"
                  type="file"
                  onChange={handleImportConfig}
                />
                <label htmlFor="import-config-file">
                  <Button
                    variant="contained"
                    component="span"
                    startIcon={<CloudUploadIcon />}
                  >
                    Import Configuration
                  </Button>
                </label>
              </CardContent>
            </Card>
          </Grid>
        </Grid>
      </TabPanel>

      {/* Configuration Dialog */}
      <Dialog
        open={configDialogOpen}
        onClose={() => setConfigDialogOpen(false)}
        maxWidth="md"
        fullWidth
      >
        <DialogTitle>
          {editingConfig ? 'Edit Scenario' : 'Add Scenario'}
        </DialogTitle>
        <DialogContent>
          <Grid container spacing={2} sx={{ mt: 1 }}>
            <Grid item xs={12}>
              <TextField
                fullWidth
                label="Scenario Name"
                value={editingConfig?.name || ''}
                onChange={(e) => {
                  // Handle change
                }}
              />
            </Grid>
            <Grid item xs={12}>
              <TextField
                fullWidth
                label="Description"
                multiline
                rows={3}
                value={editingConfig?.description || ''}
                onChange={(e) => {
                  // Handle change
                }}
              />
            </Grid>
            <Grid item xs={12}>
              <TextField
                fullWidth
                label="Configuration (YAML)"
                multiline
                rows={10}
                value={editingConfig?.config || ''}
                onChange={(e) => {
                  // Handle change
                }}
              />
            </Grid>
          </Grid>
        </DialogContent>
        <DialogActions>
          <Button onClick={() => setConfigDialogOpen(false)}>
            Cancel
          </Button>
          <Button
            variant="contained"
            onClick={() => {
              // Handle save
              setConfigDialogOpen(false);
            }}
          >
            {editingConfig ? 'Update' : 'Create'}
          </Button>
        </DialogActions>
      </Dialog>

      {/* Snackbar for notifications */}
      <Snackbar
        open={snackbarOpen}
        autoHideDuration={6000}
        onClose={() => setSnackbarOpen(false)}
      >
        <Alert
          onClose={() => setSnackbarOpen(false)}
          severity="success"
          sx={{ width: '100%' }}
        >
          {snackbarMessage}
        </Alert>
      </Snackbar>
    </Box>
  );
};

export default Settings;
