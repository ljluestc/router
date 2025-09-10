import React from 'react';
import {
  Box,
  Grid,
  Card,
  CardContent,
  Typography,
  Chip,
  List,
  ListItem,
  ListItemText,
  ListItemIcon,
  Button,
  Tabs,
  Tab,
} from '@mui/material';
import {
  Cloud as CloudIcon,
  Storage as StorageIcon,
  NetworkCheck as NetworkIcon,
  Security as SecurityIcon,
} from '@mui/icons-material';

const CloudPods = () => {
  const [currentTab, setCurrentTab] = React.useState(0);
  const [connectionStatus] = React.useState('connected');

  const [resources] = React.useState([
    { id: '1', name: 'web-server-1', type: 'instance', status: 'running', region: 'us-west-1', created_at: '2024-01-15T10:30:00Z', tags: ['web', 'production', 'nginx'] },
    { id: '2', name: 'db-cluster-1', type: 'database', status: 'running', region: 'us-west-1', created_at: '2024-01-10T08:15:00Z', tags: ['database', 'production', 'postgresql'] },
    { id: '3', name: 'load-balancer-1', type: 'loadbalancer', status: 'running', region: 'us-west-1', created_at: '2024-01-12T14:20:00Z', tags: ['loadbalancer', 'production', 'nginx'] },
  ]);

  const [typeDistribution] = React.useState([
    { name: 'Instances', value: 45, color: '#1976d2' },
    { name: 'Databases', value: 25, color: '#dc004e' },
    { name: 'Load Balancers', value: 20, color: '#2e7d32' },
    { name: 'Storage', value: 10, color: '#ff9800' },
  ]);

  const handleTabChange = (event: React.SyntheticEvent, newValue: number) => {
    setCurrentTab(newValue);
  };

  const getStatusColor = (status: string) => {
    switch (status.toLowerCase()) {
      case 'running':
      case 'active':
        return 'success';
      case 'stopped':
      case 'inactive':
        return 'error';
      case 'pending':
        return 'warning';
      default:
        return 'default';
    }
  };

  const getTypeIcon = (type: string) => {
    switch (type.toLowerCase()) {
      case 'instance':
        return <CloudIcon color="primary" />;
      case 'database':
        return <StorageIcon color="secondary" />;
      case 'loadbalancer':
        return <NetworkIcon color="info" />;
      default:
        return <SecurityIcon />;
    }
  };

  return (
    <Box>
      <Box sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', mb: 3 }}>
        <Typography variant="h4" component="h1">
          CloudPods Integration
        </Typography>
        <Box>
          <Button variant="outlined" sx={{ mr: 1 }}>
            Refresh
          </Button>
          <Button variant="contained">
            Deploy
          </Button>
        </Box>
      </Box>

      <Card sx={{ mb: 3 }}>
        <CardContent>
          <Typography variant="h6" gutterBottom>
            Connection Status
          </Typography>
          <Chip
            label={connectionStatus === 'connected' ? 'Connected' : 'Disconnected'}
            color={connectionStatus === 'connected' ? 'success' : 'error'}
            sx={{ mr: 2 }}
          />
          <Typography variant="body2" color="text.secondary">
            CloudPods Controller: {connectionStatus === 'connected' ? 'Online' : 'Offline'}
          </Typography>
        </CardContent>
      </Card>

      <Paper sx={{ mb: 3 }}>
        <Tabs value={currentTab} onChange={handleTabChange}>
          <Tab label="Resources" />
          <Tab label="Analytics" />
        </Tabs>
      </Paper>

      {currentTab === 0 && (
        <Grid container spacing={3}>
          {resources.map((resource) => (
            <Grid item xs={12} md={6} key={resource.id}>
              <Card>
                <CardContent>
                  <Box sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', mb: 2 }}>
                    <Typography variant="h6">
                      {resource.name}
                    </Typography>
                    <Chip
                      label={resource.status}
                      color={getStatusColor(resource.status) as any}
                      size="small"
                    />
                  </Box>
                  <List dense>
                    <ListItem>
                      <ListItemIcon>
                        {getTypeIcon(resource.type)}
                      </ListItemIcon>
                      <ListItemText primary="Type" secondary={resource.type} />
                    </ListItem>
                    <ListItem>
                      <ListItemText primary="Region" secondary={resource.region} />
                    </ListItem>
                    <ListItem>
                      <ListItemText primary="Created" secondary={new Date(resource.created_at).toLocaleDateString()} />
                    </ListItem>
                    <ListItem>
                      <ListItemText 
                        primary="Tags" 
                        secondary={resource.tags.join(', ')} 
                      />
                    </ListItem>
                  </List>
                </CardContent>
              </Card>
            </Grid>
          ))}
        </Grid>
      )}

      {currentTab === 1 && (
        <Grid container spacing={3}>
          <Grid item xs={12} md={6}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  Resource Distribution
                </Typography>
                <List>
                  {typeDistribution.map((type, index) => (
                    <ListItem key={index}>
                      <ListItemIcon>
                        <Box
                          sx={{
                            width: 16,
                            height: 16,
                            backgroundColor: type.color,
                            borderRadius: '50%',
                          }}
                        />
                      </ListItemIcon>
                      <ListItemText
                        primary={type.name}
                        secondary={`${type.value}%`}
                      />
                    </ListItem>
                  ))}
                </List>
              </CardContent>
            </Card>
          </Grid>
          <Grid item xs={12} md={6}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  Resource Statistics
                </Typography>
                <List>
                  <ListItem>
                    <ListItemText primary="Total Resources" secondary={resources.length.toString()} />
                  </ListItem>
                  <ListItem>
                    <ListItemText 
                      primary="Running" 
                      secondary={resources.filter(r => r.status === 'running').length.toString()} 
                    />
                  </ListItem>
                  <ListItem>
                    <ListItemText 
                      primary="Stopped" 
                      secondary={resources.filter(r => r.status === 'stopped').length.toString()} 
                    />
                  </ListItem>
                </List>
              </CardContent>
            </Card>
          </Grid>
        </Grid>
      )}
    </Box>
  );
};

export default CloudPods;