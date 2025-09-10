import React, { useState, useEffect } from 'react';
import {
  Box,
  Grid,
  Card,
  CardContent,
  Typography,
  LinearProgress,
  Chip,
  List,
  ListItem,
  ListItemText,
  ListItemIcon,
  IconButton,
  Paper,
} from '@mui/material';
import {
  Router as RouterIcon,
  Cloud as CloudIcon,
  Speed as SpeedIcon,
  NetworkCheck as NetworkIcon,
  TrendingUp as TrendingUpIcon,
  Warning as WarningIcon,
  CheckCircle as CheckCircleIcon,
  Refresh as RefreshIcon,
} from '@mui/icons-material';
import { LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip, ResponsiveContainer, PieChart, Pie, Cell } from 'recharts';

interface SystemStats {
  totalRouters: number;
  activeConnections: number;
  trafficThroughput: number;
  packetLoss: number;
  latency: number;
  cpuUsage: number;
  memoryUsage: number;
  uptime: string;
}

interface TrafficData {
  time: string;
  throughput: number;
  latency: number;
  packetLoss: number;
}

interface ProtocolData {
  name: string;
  value: number;
  color: string;
}

const Dashboard: React.FC = () => {
  const [stats, setStats] = useState<SystemStats>({
    totalRouters: 0,
    activeConnections: 0,
    trafficThroughput: 0,
    packetLoss: 0,
    latency: 0,
    cpuUsage: 0,
    memoryUsage: 0,
    uptime: '0d 0h 0m',
  });

  const [trafficData, setTrafficData] = useState<TrafficData[]>([]);
  const [protocolData, setProtocolData] = useState<ProtocolData[]>([]);
  const [recentEvents, setRecentEvents] = useState<string[]>([]);
  const [loading, setLoading] = useState(true);

  useEffect(() => {
    fetchDashboardData();
    const interval = setInterval(fetchDashboardData, 5000);
    return () => clearInterval(interval);
  }, []);

  const fetchDashboardData = async () => {
    try {
      setLoading(true);
      
      // Simulate API calls - replace with actual endpoints
      const [statsRes, trafficRes, protocolRes, eventsRes] = await Promise.all([
        fetch('/api/v1/dashboard/stats'),
        fetch('/api/v1/dashboard/traffic'),
        fetch('/api/v1/dashboard/protocols'),
        fetch('/api/v1/dashboard/events'),
      ]);

      if (statsRes.ok) {
        const statsData = await statsRes.json();
        setStats(statsData);
      } else {
        // Mock data for demo
        setStats({
          totalRouters: 12,
          activeConnections: 8,
          trafficThroughput: 2.4,
          packetLoss: 0.1,
          latency: 45,
          cpuUsage: 65,
          memoryUsage: 78,
          uptime: '15d 8h 32m',
        });
      }

      if (trafficRes.ok) {
        const trafficData = await trafficRes.json();
        setTrafficData(trafficData);
      } else {
        // Mock traffic data
        const now = new Date();
        const mockData = Array.from({ length: 24 }, (_, i) => ({
          time: new Date(now.getTime() - (23 - i) * 60 * 60 * 1000).toLocaleTimeString(),
          throughput: Math.random() * 5 + 1,
          latency: Math.random() * 100 + 20,
          packetLoss: Math.random() * 2,
        }));
        setTrafficData(mockData);
      }

      if (protocolRes.ok) {
        const protocolData = await protocolRes.json();
        setProtocolData(protocolData);
      } else {
        // Mock protocol data
        setProtocolData([
          { name: 'BGP', value: 45, color: '#8884d8' },
          { name: 'OSPF', value: 30, color: '#82ca9d' },
          { name: 'ISIS', value: 15, color: '#ffc658' },
          { name: 'Static', value: 10, color: '#ff7300' },
        ]);
      }

      if (eventsRes.ok) {
        const eventsData = await eventsRes.json();
        setRecentEvents(eventsData);
      } else {
        // Mock events
        setRecentEvents([
          'BGP session established with AS65001',
          'Traffic shaping policy applied to interface eth0',
          'Network impairment simulation started',
          'CloudPods resource created: web-server-01',
          'Aviatrix gateway status changed to active',
        ]);
      }
    } catch (error) {
      console.error('Failed to fetch dashboard data:', error);
    } finally {
      setLoading(false);
    }
  };

  const StatCard: React.FC<{ title: string; value: string | number; icon: React.ReactNode; color?: string; progress?: number }> = ({
    title,
    value,
    icon,
    color = 'primary',
    progress,
  }) => (
    <Card>
      <CardContent>
        <Box sx={{ display: 'flex', alignItems: 'center', mb: 2 }}>
          <Box sx={{ color: `${color}.main`, mr: 1 }}>
            {icon}
          </Box>
          <Typography variant="h6" color="textSecondary">
            {title}
          </Typography>
        </Box>
        <Typography variant="h4" component="div" sx={{ mb: 1 }}>
          {value}
        </Typography>
        {progress !== undefined && (
          <LinearProgress
            variant="determinate"
            value={progress}
            sx={{ height: 8, borderRadius: 4 }}
          />
        )}
      </CardContent>
    </Card>
  );

  return (
    <Box>
      <Box sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', mb: 3 }}>
        <Typography variant="h4" component="h1">
          Dashboard
        </Typography>
        <IconButton onClick={fetchDashboardData} disabled={loading}>
          <RefreshIcon />
        </IconButton>
      </Box>

      <Grid container spacing={3}>
        {/* Stats Cards */}
        <Grid item xs={12} sm={6} md={3}>
          <StatCard
            title="Total Routers"
            value={stats.totalRouters}
            icon={<RouterIcon />}
            color="primary"
          />
        </Grid>
        <Grid item xs={12} sm={6} md={3}>
          <StatCard
            title="Active Connections"
            value={stats.activeConnections}
            icon={<NetworkIcon />}
            color="success"
          />
        </Grid>
        <Grid item xs={12} sm={6} md={3}>
          <StatCard
            title="Traffic Throughput"
            value={`${stats.trafficThroughput} Gbps`}
            icon={<SpeedIcon />}
            color="info"
          />
        </Grid>
        <Grid item xs={12} sm={6} md={3}>
          <StatCard
            title="System Uptime"
            value={stats.uptime}
            icon={<CheckCircleIcon />}
            color="secondary"
          />
        </Grid>

        {/* Performance Metrics */}
        <Grid item xs={12} sm={6} md={3}>
          <StatCard
            title="CPU Usage"
            value={`${stats.cpuUsage}%`}
            icon={<TrendingUpIcon />}
            color="warning"
            progress={stats.cpuUsage}
          />
        </Grid>
        <Grid item xs={12} sm={6} md={3}>
          <StatCard
            title="Memory Usage"
            value={`${stats.memoryUsage}%`}
            icon={<TrendingUpIcon />}
            color="error"
            progress={stats.memoryUsage}
          />
        </Grid>
        <Grid item xs={12} sm={6} md={3}>
          <StatCard
            title="Packet Loss"
            value={`${stats.packetLoss}%`}
            icon={<WarningIcon />}
            color={stats.packetLoss > 1 ? 'error' : 'success'}
          />
        </Grid>
        <Grid item xs={12} sm={6} md={3}>
          <StatCard
            title="Latency"
            value={`${stats.latency} ms`}
            icon={<NetworkIcon />}
            color={stats.latency > 100 ? 'warning' : 'success'}
          />
        </Grid>

        {/* Traffic Chart */}
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
                  <Line type="monotone" dataKey="throughput" stroke="#8884d8" strokeWidth={2} />
                  <Line type="monotone" dataKey="latency" stroke="#82ca9d" strokeWidth={2} />
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

        {/* Recent Events */}
        <Grid item xs={12}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>
                Recent Events
              </Typography>
              <List>
                {recentEvents.map((event, index) => (
                  <ListItem key={index} divider={index < recentEvents.length - 1}>
                    <ListItemIcon>
                      <CheckCircleIcon color="success" />
                    </ListItemIcon>
                    <ListItemText primary={event} />
                  </ListItem>
                ))}
              </List>
            </CardContent>
          </Card>
        </Grid>
      </Grid>
    </Box>
  );
};

export default Dashboard;
