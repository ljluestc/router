import React, { useState, useEffect } from 'react';
import {
  Box,
  Card,
  CardContent,
  Typography,
  Grid,
  TextField,
  Button,
  Switch,
  FormControlLabel,
  Divider,
  Alert,
  Snackbar,
} from '@mui/material';
import {
  Save as SaveIcon,
  Refresh as RefreshIcon,
  Cloud as CloudIcon,
  Security as SecurityIcon,
  Settings as SettingsIcon,
} from '@mui/icons-material';

interface Settings {
  cloudpods: {
    endpoint: string;
    username: string;
    password: string;
    region: string;
  };
  aviatrix: {
    endpoint: string;
    username: string;
    password: string;
    verify_ssl: boolean;
  };
  monitoring: {
    prometheus_enabled: boolean;
    prometheus_port: number;
    metrics_path: string;
    log_level: string;
  };
  router: {
    max_flows: number;
    flow_timeout: number;
    packet_buffer_size: number;
    debug_enabled: boolean;
  };
}

const Settings: React.FC = () => {
  const [settings, setSettings] = useState<Settings>({
    cloudpods: {
      endpoint: 'https://api.cloudpods.org',
      username: 'admin',
      password: '',
      region: 'us-west-2',
    },
    aviatrix: {
      endpoint: 'https://api.aviatrix.com',
      username: 'admin',
      password: '',
      verify_ssl: true,
    },
    monitoring: {
      prometheus_enabled: true,
      prometheus_port: 9090,
      metrics_path: '/metrics',
      log_level: 'info',
    },
    router: {
      max_flows: 10000,
      flow_timeout: 300,
      packet_buffer_size: 1000,
      debug_enabled: false,
    },
  });
  const [loading, setLoading] = useState(false);
  const [saved, setSaved] = useState(false);

  useEffect(() => {
    loadSettings();
  }, []);

  const loadSettings = async () => {
    try {
      setLoading(true);
      const response = await fetch('/api/v1/settings');
      const data = await response.json();
      setSettings(data.settings || settings);
    } catch (error) {
      console.error('Failed to load settings:', error);
    } finally {
      setLoading(false);
    }
  };

  const saveSettings = async () => {
    try {
      setLoading(true);
      await fetch('/api/v1/settings', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify(settings),
      });
      setSaved(true);
    } catch (error) {
      console.error('Failed to save settings:', error);
    } finally {
      setLoading(false);
    }
  };

  const handleInputChange = (section: keyof Settings, field: string, value: any) => {
    setSettings(prev => ({
      ...prev,
      [section]: {
        ...prev[section],
        [field]: value,
      },
    }));
  };

  const handleSwitchChange = (section: keyof Settings, field: string, checked: boolean) => {
    handleInputChange(section, field, checked);
  };

  return (
    <Box>
      <Box sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', mb: 3 }}>
        <Typography variant="h4" component="h1">
          Settings
        </Typography>
        <Box>
          <Button
            variant="outlined"
            startIcon={<RefreshIcon />}
            onClick={loadSettings}
            sx={{ mr: 1 }}
          >
            Refresh
          </Button>
          <Button
            variant="contained"
            startIcon={<SaveIcon />}
            onClick={saveSettings}
            disabled={loading}
          >
            Save Settings
          </Button>
        </Box>
      </Box>

      <Grid container spacing={3}>
        {/* CloudPods Settings */}
        <Grid item xs={12} md={6}>
          <Card>
            <CardContent>
              <Box sx={{ display: 'flex', alignItems: 'center', mb: 2 }}>
                <CloudIcon color="primary" sx={{ mr: 1 }} />
                <Typography variant="h6">CloudPods Configuration</Typography>
              </Box>
              <Grid container spacing={2}>
                <Grid item xs={12}>
                  <TextField
                    label="API Endpoint"
                    value={settings.cloudpods.endpoint}
                    onChange={(e) => handleInputChange('cloudpods', 'endpoint', e.target.value)}
                    fullWidth
                  />
                </Grid>
                <Grid item xs={12} sm={6}>
                  <TextField
                    label="Username"
                    value={settings.cloudpods.username}
                    onChange={(e) => handleInputChange('cloudpods', 'username', e.target.value)}
                    fullWidth
                  />
                </Grid>
                <Grid item xs={12} sm={6}>
                  <TextField
                    label="Password"
                    type="password"
                    value={settings.cloudpods.password}
                    onChange={(e) => handleInputChange('cloudpods', 'password', e.target.value)}
                    fullWidth
                  />
                </Grid>
                <Grid item xs={12}>
                  <TextField
                    label="Region"
                    value={settings.cloudpods.region}
                    onChange={(e) => handleInputChange('cloudpods', 'region', e.target.value)}
                    fullWidth
                  />
                </Grid>
              </Grid>
            </CardContent>
          </Card>
        </Grid>

        {/* Aviatrix Settings */}
        <Grid item xs={12} md={6}>
          <Card>
            <CardContent>
              <Box sx={{ display: 'flex', alignItems: 'center', mb: 2 }}>
                <SecurityIcon color="primary" sx={{ mr: 1 }} />
                <Typography variant="h6">Aviatrix Configuration</Typography>
              </Box>
              <Grid container spacing={2}>
                <Grid item xs={12}>
                  <TextField
                    label="API Endpoint"
                    value={settings.aviatrix.endpoint}
                    onChange={(e) => handleInputChange('aviatrix', 'endpoint', e.target.value)}
                    fullWidth
                  />
                </Grid>
                <Grid item xs={12} sm={6}>
                  <TextField
                    label="Username"
                    value={settings.aviatrix.username}
                    onChange={(e) => handleInputChange('aviatrix', 'username', e.target.value)}
                    fullWidth
                  />
                </Grid>
                <Grid item xs={12} sm={6}>
                  <TextField
                    label="Password"
                    type="password"
                    value={settings.aviatrix.password}
                    onChange={(e) => handleInputChange('aviatrix', 'password', e.target.value)}
                    fullWidth
                  />
                </Grid>
                <Grid item xs={12}>
                  <FormControlLabel
                    control={
                      <Switch
                        checked={settings.aviatrix.verify_ssl}
                        onChange={(e) => handleSwitchChange('aviatrix', 'verify_ssl', e.target.checked)}
                      />
                    }
                    label="Verify SSL"
                  />
                </Grid>
              </Grid>
            </CardContent>
          </Card>
        </Grid>

        {/* Monitoring Settings */}
        <Grid item xs={12} md={6}>
          <Card>
            <CardContent>
              <Box sx={{ display: 'flex', alignItems: 'center', mb: 2 }}>
                <SettingsIcon color="primary" sx={{ mr: 1 }} />
                <Typography variant="h6">Monitoring Configuration</Typography>
              </Box>
              <Grid container spacing={2}>
                <Grid item xs={12}>
                  <FormControlLabel
                    control={
                      <Switch
                        checked={settings.monitoring.prometheus_enabled}
                        onChange={(e) => handleSwitchChange('monitoring', 'prometheus_enabled', e.target.checked)}
                      />
                    }
                    label="Enable Prometheus"
                  />
                </Grid>
                <Grid item xs={12} sm={6}>
                  <TextField
                    label="Prometheus Port"
                    type="number"
                    value={settings.monitoring.prometheus_port}
                    onChange={(e) => handleInputChange('monitoring', 'prometheus_port', parseInt(e.target.value))}
                    fullWidth
                  />
                </Grid>
                <Grid item xs={12} sm={6}>
                  <TextField
                    label="Metrics Path"
                    value={settings.monitoring.metrics_path}
                    onChange={(e) => handleInputChange('monitoring', 'metrics_path', e.target.value)}
                    fullWidth
                  />
                </Grid>
                <Grid item xs={12}>
                  <TextField
                    label="Log Level"
                    select
                    value={settings.monitoring.log_level}
                    onChange={(e) => handleInputChange('monitoring', 'log_level', e.target.value)}
                    fullWidth
                  >
                    <option value="debug">Debug</option>
                    <option value="info">Info</option>
                    <option value="warn">Warning</option>
                    <option value="error">Error</option>
                  </TextField>
                </Grid>
              </Grid>
            </CardContent>
          </Card>
        </Grid>

        {/* Router Settings */}
        <Grid item xs={12} md={6}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>Router Configuration</Typography>
              <Grid container spacing={2}>
                <Grid item xs={12} sm={6}>
                  <TextField
                    label="Max Flows"
                    type="number"
                    value={settings.router.max_flows}
                    onChange={(e) => handleInputChange('router', 'max_flows', parseInt(e.target.value))}
                    fullWidth
                  />
                </Grid>
                <Grid item xs={12} sm={6}>
                  <TextField
                    label="Flow Timeout (seconds)"
                    type="number"
                    value={settings.router.flow_timeout}
                    onChange={(e) => handleInputChange('router', 'flow_timeout', parseInt(e.target.value))}
                    fullWidth
                  />
                </Grid>
                <Grid item xs={12} sm={6}>
                  <TextField
                    label="Packet Buffer Size"
                    type="number"
                    value={settings.router.packet_buffer_size}
                    onChange={(e) => handleInputChange('router', 'packet_buffer_size', parseInt(e.target.value))}
                    fullWidth
                  />
                </Grid>
                <Grid item xs={12} sm={6}>
                  <FormControlLabel
                    control={
                      <Switch
                        checked={settings.router.debug_enabled}
                        onChange={(e) => handleSwitchChange('router', 'debug_enabled', e.target.checked)}
                      />
                    }
                    label="Debug Mode"
                  />
                </Grid>
              </Grid>
            </CardContent>
          </Card>
        </Grid>

        {/* System Information */}
        <Grid item xs={12}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>System Information</Typography>
              <Grid container spacing={2}>
                <Grid item xs={12} sm={6} md={3}>
                  <Typography variant="body2" color="text.secondary">
                    Version
                  </Typography>
                  <Typography variant="body1">1.0.0</Typography>
                </Grid>
                <Grid item xs={12} sm={6} md={3}>
                  <Typography variant="body2" color="text.secondary">
                    Build Date
                  </Typography>
                  <Typography variant="body1">
                    {new Date().toLocaleDateString()}
                  </Typography>
                </Grid>
                <Grid item xs={12} sm={6} md={3}>
                  <Typography variant="body2" color="text.secondary">
                    Go Version
                  </Typography>
                  <Typography variant="body1">1.21.5</Typography>
                </Grid>
                <Grid item xs={12} sm={6} md={3}>
                  <Typography variant="body2" color="text.secondary">
                    Rust Version
                  </Typography>
                  <Typography variant="body1">1.70.0</Typography>
                </Grid>
              </Grid>
            </CardContent>
          </Card>
        </Grid>
      </Grid>

      <Snackbar
        open={saved}
        autoHideDuration={3000}
        onClose={() => setSaved(false)}
      >
        <Alert onClose={() => setSaved(false)} severity="success">
          Settings saved successfully!
        </Alert>
      </Snackbar>
    </Box>
  );
};

export default Settings;
