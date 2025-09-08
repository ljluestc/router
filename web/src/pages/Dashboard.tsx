import React, { useState, useEffect } from 'react';
import {
  Grid,
  Card,
  CardContent,
  Typography,
  Box,
  Chip,
  LinearProgress,
} from '@mui/material';
import {
  Router as RouterIcon,
  Cloud as CloudIcon,
  Security as SecurityIcon,
  Timeline as TimelineIcon,
} from '@mui/icons-material';
import { LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip, ResponsiveContainer } from 'recharts';

interface SystemStatus {
  router: {
    status: string;
    uptime: string;
    interfaces: number;
    routes: number;
  };
  cloudpods: {
    status: string;
    resources: number;
    instances: number;
  };
  aviatrix: {
    status: string;
    gateways: number;
    connections: number;
  };
  monitoring: {
    packets_processed: number;
    packets_dropped: number;
    active_flows: number;
  };
}

const Dashboard: React.FC = () => {
  const [systemStatus, setSystemStatus] = useState<SystemStatus | null>(null);
  const [loading, setLoading] = useState(true);

  useEffect(() => {
    fetchSystemStatus();
    const interval = setInterval(fetchSystemStatus, 5000); // Update every 5 seconds
    return () => clearInterval(interval);
  }, []);

  const fetchSystemStatus = async () => {
    try {
      const response = await fetch('/api/v1/health');
      const data = await response.json();
      setSystemStatus(data);
      setLoading(false);
    } catch (error) {
      console.error('Failed to fetch system status:', error);
      setLoading(false);
    }
  };

  const getStatusColor = (status: string) => {
    switch (status.toLowerCase()) {
      case 'healthy':
      case 'up':
      case 'running':
        return 'success';
      case 'warning':
        return 'warning';
      case 'error':
      case 'down':
        return 'error';
      default:
        return 'default';
    }
  };

  const mockData = [
    { time: '00:00', packets: 1000, flows: 50 },
    { time: '04:00', packets: 1200, flows: 45 },
    { time: '08:00', packets: 1500, flows: 60 },
    { time: '12:00', packets: 2000, flows: 80 },
    { time: '16:00', packets: 1800, flows: 70 },
    { time: '20:00', packets: 1600, flows: 65 },
  ];

  if (loading) {
    return (
      <Box sx={{ width: '100%' }}>
        <LinearProgress />
      </Box>
    );
  }

  return (
    <Box>
      <Typography variant="h4" component="h1" gutterBottom>
        Dashboard
      </Typography>
      
      <Grid container spacing={3}>
        {/* System Status Cards */}
        <Grid item xs={12} md={3}>
          <Card>
            <CardContent>
              <Box sx={{ display: 'flex', alignItems: 'center', mb: 2 }}>
                <RouterIcon color="primary" sx={{ mr: 1 }} />
                <Typography variant="h6">Router</Typography>
              </Box>
              <Chip 
                label={systemStatus?.router?.status || 'Unknown'} 
                color={getStatusColor(systemStatus?.router?.status || '')}
                size="small"
              />
              <Typography variant="body2" sx={{ mt: 1 }}>
                Uptime: {systemStatus?.router?.uptime || 'N/A'}
              </Typography>
              <Typography variant="body2">
                Interfaces: {systemStatus?.router?.interfaces || 0}
              </Typography>
              <Typography variant="body2">
                Routes: {systemStatus?.router?.routes || 0}
              </Typography>
            </CardContent>
          </Card>
        </Grid>

        <Grid item xs={12} md={3}>
          <Card>
            <CardContent>
              <Box sx={{ display: 'flex', alignItems: 'center', mb: 2 }}>
                <CloudIcon color="primary" sx={{ mr: 1 }} />
                <Typography variant="h6">CloudPods</Typography>
              </Box>
              <Chip 
                label={systemStatus?.cloudpods?.status || 'Unknown'} 
                color={getStatusColor(systemStatus?.cloudpods?.status || '')}
                size="small"
              />
              <Typography variant="body2" sx={{ mt: 1 }}>
                Resources: {systemStatus?.cloudpods?.resources || 0}
              </Typography>
              <Typography variant="body2">
                Instances: {systemStatus?.cloudpods?.instances || 0}
              </Typography>
            </CardContent>
          </Card>
        </Grid>

        <Grid item xs={12} md={3}>
          <Card>
            <CardContent>
              <Box sx={{ display: 'flex', alignItems: 'center', mb: 2 }}>
                <SecurityIcon color="primary" sx={{ mr: 1 }} />
                <Typography variant="h6">Aviatrix</Typography>
              </Box>
              <Chip 
                label={systemStatus?.aviatrix?.status || 'Unknown'} 
                color={getStatusColor(systemStatus?.aviatrix?.status || '')}
                size="small"
              />
              <Typography variant="body2" sx={{ mt: 1 }}>
                Gateways: {systemStatus?.aviatrix?.gateways || 0}
              </Typography>
              <Typography variant="body2">
                Connections: {systemStatus?.aviatrix?.connections || 0}
              </Typography>
            </CardContent>
          </Card>
        </Grid>

        <Grid item xs={12} md={3}>
          <Card>
            <CardContent>
              <Box sx={{ display: 'flex', alignItems: 'center', mb: 2 }}>
                <TimelineIcon color="primary" sx={{ mr: 1 }} />
                <Typography variant="h6">Monitoring</Typography>
              </Box>
              <Typography variant="body2">
                Packets: {systemStatus?.monitoring?.packets_processed || 0}
              </Typography>
              <Typography variant="body2">
                Dropped: {systemStatus?.monitoring?.packets_dropped || 0}
              </Typography>
              <Typography variant="body2">
                Flows: {systemStatus?.monitoring?.active_flows || 0}
              </Typography>
            </CardContent>
          </Card>
        </Grid>

        {/* Performance Charts */}
        <Grid item xs={12} md={8}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>
                Network Performance
              </Typography>
              <ResponsiveContainer width="100%" height={300}>
                <LineChart data={mockData}>
                  <CartesianGrid strokeDasharray="3 3" />
                  <XAxis dataKey="time" />
                  <YAxis />
                  <Tooltip />
                  <Line type="monotone" dataKey="packets" stroke="#1976d2" strokeWidth={2} />
                  <Line type="monotone" dataKey="flows" stroke="#dc004e" strokeWidth={2} />
                </LineChart>
              </ResponsiveContainer>
            </CardContent>
          </Card>
        </Grid>

        {/* Quick Actions */}
        <Grid item xs={12} md={4}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>
                Quick Actions
              </Typography>
              <Box sx={{ display: 'flex', flexDirection: 'column', gap: 1 }}>
                <Chip label="Start Router" color="primary" clickable />
                <Chip label="Stop Router" color="secondary" clickable />
                <Chip label="Refresh Status" color="default" clickable />
                <Chip label="View Logs" color="default" clickable />
              </Box>
            </CardContent>
          </Card>
        </Grid>
      </Grid>
    </Box>
  );
};

export default Dashboard;
