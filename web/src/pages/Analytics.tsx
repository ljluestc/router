import React from 'react';
import {
  Box,
  Typography,
  Card,
  CardContent,
  Grid,
  Paper,
  List,
  ListItem,
  ListItemText,
  ListItemIcon,
  Chip,
} from '@mui/material';
import {
  TrendingUp as TrendingUpIcon,
  Speed as SpeedIcon,
  Memory as MemoryIcon,
  NetworkCheck as NetworkCheckIcon,
} from '@mui/icons-material';
import { useQuery } from '@tanstack/react-query';
import { LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip, ResponsiveContainer, BarChart, Bar, PieChart, Pie, Cell } from 'recharts';

const Analytics: React.FC = () => {
  const { data: trafficStats } = useQuery({
    queryKey: ['analytics-traffic'],
    queryFn: async () => {
      const response = await fetch('/api/v1/analytics/traffic');
      return response.json();
    },
  });

  const { data: performanceMetrics } = useQuery({
    queryKey: ['analytics-performance'],
    queryFn: async () => {
      const response = await fetch('/api/v1/analytics/performance');
      return response.json();
    },
  });

  const { data: routingStats } = useQuery({
    queryKey: ['analytics-routing'],
    queryFn: async () => {
      const response = await fetch('/api/v1/analytics/routing');
      return response.json();
    },
  });

  const { data: cloudpodsStats } = useQuery({
    queryKey: ['analytics-cloudpods'],
    queryFn: async () => {
      const response = await fetch('/api/v1/analytics/cloudpods');
      return response.json();
    },
  });

  const { data: aviatrixStats } = useQuery({
    queryKey: ['analytics-aviatrix'],
    queryFn: async () => {
      const response = await fetch('/api/v1/analytics/aviatrix');
      return response.json();
    },
  });

  // Mock data for charts
  const trafficData = [
    { time: '00:00', packets: 1000, bytes: 1000000 },
    { time: '01:00', packets: 1200, bytes: 1200000 },
    { time: '02:00', packets: 800, bytes: 800000 },
    { time: '03:00', packets: 1500, bytes: 1500000 },
    { time: '04:00', packets: 2000, bytes: 2000000 },
    { time: '05:00', packets: 1800, bytes: 1800000 },
  ];

  const protocolData = [
    { name: 'TCP', value: 60, color: '#1976d2' },
    { name: 'UDP', value: 25, color: '#dc004e' },
    { name: 'ICMP', value: 10, color: '#2e7d32' },
    { name: 'Other', value: 5, color: '#ed6c02' },
  ];

  const performanceData = [
    { metric: 'CPU', value: 25.5 },
    { metric: 'Memory', value: 45.2 },
    { metric: 'Network', value: 78.1 },
    { metric: 'Disk', value: 12.3 },
  ];

  return (
    <Box>
      <Typography variant="h4" component="h1" gutterBottom>
        Analytics Dashboard
      </Typography>

      <Grid container spacing={3}>
        {/* Traffic Statistics */}
        <Grid item xs={12} md={6}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>
                Traffic Statistics
              </Typography>
              {trafficStats?.data && (
                <Box>
                  <Grid container spacing={2}>
                    <Grid item xs={6}>
                      <Typography variant="body2" color="text.secondary">
                        Total Packets
                      </Typography>
                      <Typography variant="h6">
                        {trafficStats.data.total_packets?.toLocaleString()}
                      </Typography>
                    </Grid>
                    <Grid item xs={6}>
                      <Typography variant="body2" color="text.secondary">
                        Total Bytes
                      </Typography>
                      <Typography variant="h6">
                        {(trafficStats.data.total_bytes / 1024 / 1024).toFixed(2)} MB
                      </Typography>
                    </Grid>
                    <Grid item xs={6}>
                      <Typography variant="body2" color="text.secondary">
                        Packets/sec
                      </Typography>
                      <Typography variant="h6">
                        {trafficStats.data.packets_per_second?.toFixed(0)}
                      </Typography>
                    </Grid>
                    <Grid item xs={6}>
                      <Typography variant="body2" color="text.secondary">
                        Bytes/sec
                      </Typography>
                      <Typography variant="h6">
                        {(trafficStats.data.bytes_per_second / 1024).toFixed(0)} KB/s
                      </Typography>
                    </Grid>
                  </Grid>
                </Box>
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
              {performanceMetrics?.data && (
                <Box>
                  <Grid container spacing={2}>
                    <Grid item xs={6}>
                      <Typography variant="body2" color="text.secondary">
                        CPU Usage
                      </Typography>
                      <Typography variant="h6">
                        {performanceMetrics.data.cpu_usage}%
                      </Typography>
                    </Grid>
                    <Grid item xs={6}>
                      <Typography variant="body2" color="text.secondary">
                        Memory Usage
                      </Typography>
                      <Typography variant="h6">
                        {performanceMetrics.data.memory_usage} MB
                      </Typography>
                    </Grid>
                    <Grid item xs={6}>
                      <Typography variant="body2" color="text.secondary">
                        Processing Rate
                      </Typography>
                      <Typography variant="h6">
                        {performanceMetrics.data.packet_processing_rate} pps
                      </Typography>
                    </Grid>
                    <Grid item xs={6}>
                      <Typography variant="body2" color="text.secondary">
                        Average Latency
                      </Typography>
                      <Typography variant="h6">
                        {performanceMetrics.data.average_latency} ms
                      </Typography>
                    </Grid>
                  </Grid>
                </Box>
              )}
            </CardContent>
          </Card>
        </Grid>

        {/* Traffic Over Time Chart */}
        <Grid item xs={12} md={8}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>
                Traffic Over Time
              </Typography>
              <ResponsiveContainer width="100%" height={300}>
                <LineChart data={trafficData}>
                  <CartesianGrid strokeDasharray="3 3" />
                  <XAxis dataKey="time" />
                  <YAxis />
                  <Tooltip />
                  <Line
                    type="monotone"
                    dataKey="packets"
                    stroke="#1976d2"
                    strokeWidth={2}
                    name="Packets"
                  />
                  <Line
                    type="monotone"
                    dataKey="bytes"
                    stroke="#dc004e"
                    strokeWidth={2}
                    name="Bytes"
                  />
                </LineChart>
              </ResponsiveContainer>
            </CardContent>
          </Card>
        </Grid>

        {/* Protocol Distribution */}
        <Grid item xs={12} md={4}>
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
                    labelLine={false}
                    label={({ name, percent }) => `${name} ${(percent * 100).toFixed(0)}%`}
                    outerRadius={80}
                    fill="#8884d8"
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

        {/* Performance Metrics Chart */}
        <Grid item xs={12} md={6}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>
                Performance Metrics
              </Typography>
              <ResponsiveContainer width="100%" height={300}>
                <BarChart data={performanceData}>
                  <CartesianGrid strokeDasharray="3 3" />
                  <XAxis dataKey="metric" />
                  <YAxis />
                  <Tooltip />
                  <Bar dataKey="value" fill="#1976d2" />
                </BarChart>
              </ResponsiveContainer>
            </CardContent>
          </Card>
        </Grid>

        {/* Top Flows */}
        <Grid item xs={12} md={6}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>
                Top Flows
              </Typography>
              {trafficStats?.data?.top_flows && (
                <List>
                  {trafficStats.data.top_flows.slice(0, 5).map((flow: any, index: number) => (
                    <ListItem key={index}>
                      <ListItemIcon>
                        <NetworkCheckIcon />
                      </ListItemIcon>
                      <ListItemText
                        primary={flow.flow_id}
                        secondary={`${flow.count} packets`}
                      />
                    </ListItem>
                  ))}
                </List>
              )}
            </CardContent>
          </Card>
        </Grid>

        {/* CloudPods Statistics */}
        <Grid item xs={12} md={6}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>
                CloudPods Statistics
              </Typography>
              {cloudpodsStats?.data && (
                <Box>
                  <Grid container spacing={2}>
                    <Grid item xs={6}>
                      <Typography variant="body2" color="text.secondary">
                        VPCs
                      </Typography>
                      <Typography variant="h6">
                        {cloudpodsStats.data.vpc_count}
                      </Typography>
                    </Grid>
                    <Grid item xs={6}>
                      <Typography variant="body2" color="text.secondary">
                        NAT Gateways
                      </Typography>
                      <Typography variant="h6">
                        {cloudpodsStats.data.nat_gateway_count}
                      </Typography>
                    </Grid>
                    <Grid item xs={6}>
                      <Typography variant="body2" color="text.secondary">
                        Load Balancers
                      </Typography>
                      <Typography variant="h6">
                        {cloudpodsStats.data.load_balancer_count}
                      </Typography>
                    </Grid>
                    <Grid item xs={6}>
                      <Typography variant="body2" color="text.secondary">
                        Service Mesh Routes
                      </Typography>
                      <Typography variant="h6">
                        {cloudpodsStats.data.service_mesh_count}
                      </Typography>
                    </Grid>
                  </Grid>
                </Box>
              )}
            </CardContent>
          </Card>
        </Grid>

        {/* Aviatrix Statistics */}
        <Grid item xs={12} md={6}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>
                Aviatrix Statistics
              </Typography>
              {aviatrixStats?.data && (
                <Box>
                  <Grid container spacing={2}>
                    <Grid item xs={6}>
                      <Typography variant="body2" color="text.secondary">
                        Transit Gateways
                      </Typography>
                      <Typography variant="h6">
                        {aviatrixStats.data.transit_gateways}
                      </Typography>
                    </Grid>
                    <Grid item xs={6}>
                      <Typography variant="body2" color="text.secondary">
                        Spoke Gateways
                      </Typography>
                      <Typography variant="h6">
                        {aviatrixStats.data.spoke_gateways}
                      </Typography>
                    </Grid>
                    <Grid item xs={6}>
                      <Typography variant="body2" color="text.secondary">
                        VPC Connections
                      </Typography>
                      <Typography variant="h6">
                        {aviatrixStats.data.vpc_connections}
                      </Typography>
                    </Grid>
                    <Grid item xs={6}>
                      <Typography variant="body2" color="text.secondary">
                        Site-to-Cloud
                      </Typography>
                      <Typography variant="h6">
                        {aviatrixStats.data.site2cloud_connections}
                      </Typography>
                    </Grid>
                  </Grid>
                </Box>
              )}
            </CardContent>
          </Card>
        </Grid>
      </Grid>
    </Box>
  );
};

export default Analytics;
