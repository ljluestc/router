import React, { useState, useEffect } from 'react';
import {
  Box,
  Card,
  CardContent,
  Typography,
  Grid,
  Chip,
  Table,
  TableBody,
  TableCell,
  TableContainer,
  TableHead,
  TableRow,
  Paper,
  IconButton,
  TextField,
  InputAdornment,
} from '@mui/material';
import {
  Search as SearchIcon,
  Refresh as RefreshIcon,
  FilterList as FilterIcon,
} from '@mui/icons-material';
import { LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip, ResponsiveContainer, BarChart, Bar } from 'recharts';

interface NetworkFlow {
  flow_id: string;
  source_ip: string;
  dest_ip: string;
  protocol: string;
  source_port: number;
  dest_port: number;
  packet_count: number;
  byte_count: number;
  duration_ms: number;
  status: string;
  start_time: string;
}

const Flows: React.FC = () => {
  const [flows, setFlows] = useState<NetworkFlow[]>([]);
  const [loading, setLoading] = useState(true);
  const [searchTerm, setSearchTerm] = useState('');

  useEffect(() => {
    fetchFlows();
    const interval = setInterval(fetchFlows, 5000); // Update every 5 seconds
    return () => clearInterval(interval);
  }, []);

  const fetchFlows = async () => {
    try {
      setLoading(true);
      const response = await fetch('/api/v1/monitoring/flows');
      const data = await response.json();
      setFlows(data.flows || []);
    } catch (error) {
      console.error('Failed to fetch flows:', error);
    } finally {
      setLoading(false);
    }
  };

  const filteredFlows = flows.filter(flow =>
    flow.source_ip.toLowerCase().includes(searchTerm.toLowerCase()) ||
    flow.dest_ip.toLowerCase().includes(searchTerm.toLowerCase()) ||
    flow.protocol.toLowerCase().includes(searchTerm.toLowerCase())
  );

  const getStatusColor = (status: string) => {
    switch (status.toLowerCase()) {
      case 'active':
        return 'success';
      case 'idle':
        return 'warning';
      case 'dropped':
        return 'error';
      case 'completed':
        return 'default';
      default:
        return 'default';
    }
  };

  const getProtocolColor = (protocol: string) => {
    switch (protocol.toLowerCase()) {
      case 'tcp':
        return 'primary';
      case 'udp':
        return 'secondary';
      case 'icmp':
        return 'warning';
      default:
        return 'default';
    }
  };

  // Mock data for charts
  const flowStats = [
    { time: '00:00', active: 10, completed: 5, dropped: 1 },
    { time: '04:00', active: 15, completed: 8, dropped: 2 },
    { time: '08:00', active: 25, completed: 12, dropped: 1 },
    { time: '12:00', active: 30, completed: 18, dropped: 3 },
    { time: '16:00', active: 28, completed: 15, dropped: 2 },
    { time: '20:00', active: 20, completed: 10, dropped: 1 },
  ];

  const protocolStats = [
    { protocol: 'TCP', count: 45, percentage: 60 },
    { protocol: 'UDP', count: 25, percentage: 33 },
    { protocol: 'ICMP', count: 5, percentage: 7 },
  ];

  return (
    <Box>
      <Box sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', mb: 3 }}>
        <Typography variant="h4" component="h1">
          Network Flows
        </Typography>
        <Box>
          <TextField
            placeholder="Search flows..."
            value={searchTerm}
            onChange={(e) => setSearchTerm(e.target.value)}
            InputProps={{
              startAdornment: (
                <InputAdornment position="start">
                  <SearchIcon />
                </InputAdornment>
              ),
            }}
            sx={{ mr: 2, width: 300 }}
          />
          <IconButton onClick={fetchFlows}>
            <RefreshIcon />
          </IconButton>
          <IconButton>
            <FilterIcon />
          </IconButton>
        </Box>
      </Box>

      <Grid container spacing={3}>
        {/* Summary Cards */}
        <Grid item xs={12} md={3}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>Active Flows</Typography>
              <Typography variant="h4" color="success.main">
                {flows.filter(f => f.status === 'active').length}
              </Typography>
            </CardContent>
          </Card>
        </Grid>

        <Grid item xs={12} md={3}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>Total Packets</Typography>
              <Typography variant="h4">
                {flows.reduce((sum, flow) => sum + flow.packet_count, 0).toLocaleString()}
              </Typography>
            </CardContent>
          </Card>
        </Grid>

        <Grid item xs={12} md={3}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>Total Bytes</Typography>
              <Typography variant="h4">
                {(flows.reduce((sum, flow) => sum + flow.byte_count, 0) / 1024 / 1024).toFixed(1)} MB
              </Typography>
            </CardContent>
          </Card>
        </Grid>

        <Grid item xs={12} md={3}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>Dropped Flows</Typography>
              <Typography variant="h4" color="error.main">
                {flows.filter(f => f.status === 'dropped').length}
              </Typography>
            </CardContent>
          </Card>
        </Grid>

        {/* Charts */}
        <Grid item xs={12} md={8}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>
                Flow Statistics Over Time
              </Typography>
              <ResponsiveContainer width="100%" height={300}>
                <LineChart data={flowStats}>
                  <CartesianGrid strokeDasharray="3 3" />
                  <XAxis dataKey="time" />
                  <YAxis />
                  <Tooltip />
                  <Line type="monotone" dataKey="active" stroke="#4caf50" strokeWidth={2} />
                  <Line type="monotone" dataKey="completed" stroke="#2196f3" strokeWidth={2} />
                  <Line type="monotone" dataKey="dropped" stroke="#f44336" strokeWidth={2} />
                </LineChart>
              </ResponsiveContainer>
            </CardContent>
          </Card>
        </Grid>

        <Grid item xs={12} md={4}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>
                Protocol Distribution
              </Typography>
              <ResponsiveContainer width="100%" height={300}>
                <BarChart data={protocolStats}>
                  <CartesianGrid strokeDasharray="3 3" />
                  <XAxis dataKey="protocol" />
                  <YAxis />
                  <Tooltip />
                  <Bar dataKey="count" fill="#1976d2" />
                </BarChart>
              </ResponsiveContainer>
            </CardContent>
          </Card>
        </Grid>

        {/* Flows Table */}
        <Grid item xs={12}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>
                Active Flows
              </Typography>
              <TableContainer component={Paper} sx={{ backgroundColor: 'transparent' }}>
                <Table>
                  <TableHead>
                    <TableRow>
                      <TableCell>Flow ID</TableCell>
                      <TableCell>Source</TableCell>
                      <TableCell>Destination</TableCell>
                      <TableCell>Protocol</TableCell>
                      <TableCell>Packets</TableCell>
                      <TableCell>Bytes</TableCell>
                      <TableCell>Duration</TableCell>
                      <TableCell>Status</TableCell>
                    </TableRow>
                  </TableHead>
                  <TableBody>
                    {filteredFlows.map((flow) => (
                      <TableRow key={flow.flow_id}>
                        <TableCell>
                          <Typography variant="body2" fontFamily="monospace">
                            {flow.flow_id}
                          </Typography>
                        </TableCell>
                        <TableCell>
                          <Typography variant="body2">
                            {flow.source_ip}:{flow.source_port}
                          </Typography>
                        </TableCell>
                        <TableCell>
                          <Typography variant="body2">
                            {flow.dest_ip}:{flow.dest_port}
                          </Typography>
                        </TableCell>
                        <TableCell>
                          <Chip 
                            label={flow.protocol} 
                            color={getProtocolColor(flow.protocol)}
                            size="small"
                          />
                        </TableCell>
                        <TableCell>
                          {flow.packet_count.toLocaleString()}
                        </TableCell>
                        <TableCell>
                          {(flow.byte_count / 1024).toFixed(1)} KB
                        </TableCell>
                        <TableCell>
                          {(flow.duration_ms / 1000).toFixed(1)}s
                        </TableCell>
                        <TableCell>
                          <Chip 
                            label={flow.status} 
                            color={getStatusColor(flow.status)}
                            size="small"
                          />
                        </TableCell>
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

export default Flows;
