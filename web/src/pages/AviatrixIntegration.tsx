import React from 'react';
import {
  Box,
  Grid,
  Card,
  CardContent,
  Typography,
  Button,
  Table,
  TableBody,
  TableCell,
  TableContainer,
  TableHead,
  TableRow,
  Paper,
  Chip,
  Alert,
} from '@mui/material';
import {
  Cloud as CloudIcon,
  Router as RouterIcon,
  NetworkCheck as NetworkIcon,
  Add as AddIcon,
  Refresh as RefreshIcon,
} from '@mui/icons-material';

const AviatrixIntegration = () => {
  const [gateways] = React.useState([
    {
      id: '1',
      name: 'gw-aws-us-west-1',
      cloud: 'AWS',
      region: 'us-west-1',
      status: 'up',
      type: 'transit',
      publicIp: '54.123.45.67',
      privateIp: '10.0.1.100',
      uptime: '2d 15h 30m',
      connections: 8,
    },
    {
      id: '2',
      name: 'gw-aws-us-east-1',
      cloud: 'AWS',
      region: 'us-east-1',
      status: 'up',
      type: 'transit',
      publicIp: '54.234.56.78',
      privateIp: '10.0.2.100',
      uptime: '1d 8h 45m',
      connections: 6,
    },
    {
      id: '3',
      name: 'gw-azure-westus',
      cloud: 'Azure',
      region: 'westus',
      status: 'up',
      type: 'spoke',
      publicIp: '20.123.45.67',
      privateIp: '10.1.1.100',
      uptime: '3d 2h 15m',
      connections: 4,
    },
  ]);

  const [connectionStatus] = React.useState('connected');

  const getStatusColor = (status: string) => {
    switch (status.toLowerCase()) {
      case 'up':
      case 'active':
      case 'connected':
        return 'success';
      case 'down':
      case 'inactive':
      case 'disconnected':
        return 'error';
      case 'pending':
      case 'initializing':
        return 'warning';
      default:
        return 'default';
    }
  };

  const getCloudIcon = (cloud: string) => {
    switch (cloud.toLowerCase()) {
      case 'aws':
        return <CloudIcon color="warning" />;
      case 'azure':
        return <CloudIcon color="info" />;
      case 'gcp':
        return <CloudIcon color="primary" />;
      default:
        return <CloudIcon />;
    }
  };

  const getGatewayTypeIcon = (type: string) => {
    switch (type.toLowerCase()) {
      case 'transit':
        return <RouterIcon color="primary" />;
      case 'spoke':
        return <NetworkIcon color="secondary" />;
      default:
        return <RouterIcon />;
    }
  };

  return (
    <Box>
      <Box sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', mb: 3 }}>
        <Typography variant="h4" component="h1">
          Aviatrix Integration
        </Typography>
        <Box>
          <Button variant="outlined" startIcon={<RefreshIcon />} sx={{ mr: 1 }}>
            Refresh
          </Button>
          <Button variant="contained" startIcon={<AddIcon />}>
            Add Gateway
          </Button>
        </Box>
      </Box>

      <Alert
        severity={connectionStatus === 'connected' ? 'success' : 'error'}
        sx={{ mb: 3 }}
      >
        Aviatrix Controller: {connectionStatus === 'connected' ? 'Connected' : 'Disconnected'}
      </Alert>

      <Grid container spacing={3}>
        <Grid item xs={12}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>
                Aviatrix Gateways
              </Typography>
              <TableContainer component={Paper} sx={{ backgroundColor: 'transparent' }}>
                <Table>
                  <TableHead>
                    <TableRow>
                      <TableCell>Name</TableCell>
                      <TableCell>Cloud</TableCell>
                      <TableCell>Region</TableCell>
                      <TableCell>Type</TableCell>
                      <TableCell>Status</TableCell>
                      <TableCell>Public IP</TableCell>
                      <TableCell>Private IP</TableCell>
                      <TableCell>Uptime</TableCell>
                      <TableCell>Connections</TableCell>
                    </TableRow>
                  </TableHead>
                  <TableBody>
                    {gateways.map((gateway) => (
                      <TableRow key={gateway.id}>
                        <TableCell>
                          <Box sx={{ display: 'flex', alignItems: 'center' }}>
                            {getGatewayTypeIcon(gateway.type)}
                            <Typography variant="body2" sx={{ ml: 1 }}>
                              {gateway.name}
                            </Typography>
                          </Box>
                        </TableCell>
                        <TableCell>
                          <Box sx={{ display: 'flex', alignItems: 'center' }}>
                            {getCloudIcon(gateway.cloud)}
                            <Typography variant="body2" sx={{ ml: 1 }}>
                              {gateway.cloud}
                            </Typography>
                          </Box>
                        </TableCell>
                        <TableCell>{gateway.region}</TableCell>
                        <TableCell>
                          <Chip label={gateway.type} size="small" />
                        </TableCell>
                        <TableCell>
                          <Chip
                            label={gateway.status}
                            color={getStatusColor(gateway.status) as any}
                            size="small"
                          />
                        </TableCell>
                        <TableCell>
                          <Typography variant="body2" fontFamily="monospace">
                            {gateway.publicIp}
                          </Typography>
                        </TableCell>
                        <TableCell>
                          <Typography variant="body2" fontFamily="monospace">
                            {gateway.privateIp}
                          </Typography>
                        </TableCell>
                        <TableCell>{gateway.uptime}</TableCell>
                        <TableCell>{gateway.connections}</TableCell>
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

export default AviatrixIntegration;
