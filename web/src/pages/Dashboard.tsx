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
} from '@mui/material';
import {
  Router as RouterIcon,
  Cloud as CloudIcon,
  NetworkCheck as NetworkIcon,
  Storage as StorageIcon,
  Timeline as TimelineIcon,
} from '@mui/icons-material';
import {
  LineChart,
  Line,
  XAxis,
  YAxis,
  CartesianGrid,
  Tooltip,
  ResponsiveContainer,
  PieChart,
  Pie,
  Cell,
} from 'recharts';

const Dashboard = () => {
  const [systemStatus] = React.useState('online');
  const [services] = React.useState([
    { name: 'aviatrix', status: 'connected', gateways: 4 },
    { name: 'cloudpods', status: 'connected', resources: 12 },
    { name: 'router', status: 'running', routes: 1250 },
  ]);

  const [trafficData] = React.useState([
    { time: '00:00', bgp: 120, ospf: 95, isis: 80 },
    { time: '01:00', bgp: 135, ospf: 110, isis: 90 },
    { time: '02:00', bgp: 98, ospf: 78, isis: 70 },
    { time: '03:00', bgp: 145, ospf: 120, isis: 100 },
    { time: '04:00', bgp: 167, ospf: 140, isis: 115 },
    { time: '05:00', bgp: 189, ospf: 155, isis: 130 },
  ]);

  const [protocolData] = React.useState([
    { name: 'BGP', value: 45, color: '#1976d2' },
    { name: 'OSPF', value: 30, color: '#dc004e' },
    { name: 'ISIS', value: 25, color: '#2e7d32' },
  ]);

  const getStatusColor = (status: string) => {
    switch (status.toLowerCase()) {
      case 'connected':
      case 'running':
        return 'success';
      case 'disconnected':
      case 'stopped':
        return 'error';
      case 'pending':
        return 'warning';
      default:
        return 'default';
    }
  };

  const getServiceIcon = (service: string) => {
    switch (service.toLowerCase()) {
      case 'aviatrix':
        return <NetworkIcon color="primary" />;
      case 'cloudpods':
        return <StorageIcon color="secondary" />;
      case 'router':
        return <RouterIcon color="info" />;
      default:
        return <CloudIcon />;
    }
  };

  return (
    <Box>
      <Typography variant="h4" component="h1" gutterBottom>
        Router Simulator Dashboard
      </Typography>

      <Grid container spacing={3}>
        {/* System Status */}
        <Grid item xs={12}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>
                System Status
              </Typography>
              <Chip
                label={systemStatus}
                color={systemStatus === 'online' ? 'success' : 'error'}
                sx={{ mr: 2 }}
              />
              <Typography variant="body2" color="text.secondary">
                All systems operational
              </Typography>
            </CardContent>
          </Card>
        </Grid>

        {/* Services */}
        <Grid item xs={12} md={6}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>
                Services
              </Typography>
              <List>
                {services.map((service) => (
                  <ListItem key={service.name}>
                    <ListItemIcon>
                      {getServiceIcon(service.name)}
                    </ListItemIcon>
                    <ListItemText
                      primary={service.name.toUpperCase()}
                      secondary={`${service.gateways || service.resources || service.routes} ${service.gateways ? 'gateways' : service.resources ? 'resources' : 'routes'}`}
                    />
                    <Chip
                      label={service.status}
                      color={getStatusColor(service.status) as any}
                      size="small"
                    />
                  </ListItem>
                ))}
              </List>
            </CardContent>
          </Card>
        </Grid>

        {/* Traffic Analytics */}
        <Grid item xs={12} md={6}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>
                Traffic Analytics
              </Typography>
              <ResponsiveContainer width="100%" height={300}>
                <LineChart data={trafficData}>
                  <CartesianGrid strokeDasharray="3 3" />
                  <XAxis dataKey="time" />
                  <YAxis />
                  <Tooltip />
                  <Line type="monotone" dataKey="bgp" stroke="#1976d2" strokeWidth={2} name="BGP" />
                  <Line type="monotone" dataKey="ospf" stroke="#dc004e" strokeWidth={2} name="OSPF" />
                  <Line type="monotone" dataKey="isis" stroke="#2e7d32" strokeWidth={2} name="ISIS" />
                </LineChart>
              </ResponsiveContainer>
            </CardContent>
          </Card>
        </Grid>

        {/* Protocol Distribution */}
        <Grid item xs={12} md={6}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>
                Protocol Distribution
              </Typography>
              <ResponsiveContainer width="100%" height={300}>
                <PieChart>
                  <Pie
                    data={protocolData}
                    cx="50%"
                    cy="50%"
                    innerRadius={60}
                    outerRadius={100}
                    paddingAngle={5}
                    dataKey="value"
                  >
                    {protocolData.map((entry, index) => (
                      <Cell key={`cell-${index}`} fill={entry.color} />
                    ))}
                  </Pie>
                  <Tooltip />
                </PieChart>
              </ResponsiveContainer>
            </CardContent>
          </Card>
        </Grid>
      </Grid>
    </Box>
  );
};

export default Dashboard;