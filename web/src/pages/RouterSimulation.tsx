import React from 'react';
import {
  Box,
  Grid,
  Card,
  CardContent,
  Typography,
  Table,
  TableBody,
  TableCell,
  TableContainer,
  TableHead,
  TableRow,
  Paper,
  Chip,
  Button,
  Tabs,
  Tab,
} from '@mui/material';
import {
  PlayArrow as PlayIcon,
  Pause as PauseIcon,
  Refresh as RefreshIcon,
} from '@mui/icons-material';

const RouterSimulation = () => {
  const [currentTab, setCurrentTab] = React.useState(0);
  const [isRunning, setIsRunning] = React.useState(false);

  const [routes] = React.useState([
    { prefix: '10.0.1.0/24', nextHop: '192.168.1.1', protocol: 'BGP', metric: 0, age: '2d 15h', status: 'Active' },
    { prefix: '10.0.2.0/24', nextHop: '192.168.1.2', protocol: 'OSPF', metric: 10, age: '1d 8h', status: 'Active' },
    { prefix: '10.0.3.0/24', nextHop: '192.168.1.3', protocol: 'ISIS', metric: 20, age: '3d 2h', status: 'Active' },
  ]);

  const [neighbors] = React.useState([
    { id: '1', address: '192.168.1.1', protocol: 'BGP', state: 'Established', uptime: '2d 15h', routes: 1250 },
    { id: '2', address: '192.168.1.2', protocol: 'OSPF', state: 'Full', uptime: '1d 8h', routes: 890 },
    { id: '3', address: '192.168.1.3', protocol: 'ISIS', state: 'Up', uptime: '3d 2h', routes: 1100 },
  ]);

  const handleTabChange = (event: React.SyntheticEvent, newValue: number) => {
    setCurrentTab(newValue);
  };

  const handleStartStop = () => {
    setIsRunning(!isRunning);
  };

  const getStatusColor = (status: string) => {
    switch (status.toLowerCase()) {
      case 'active':
      case 'established':
      case 'full':
      case 'up':
        return 'success';
      case 'inactive':
      case 'down':
        return 'error';
      case 'pending':
        return 'warning';
      default:
        return 'default';
    }
  };

  return (
    <Box>
      <Box sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', mb: 3 }}>
        <Typography variant="h4" component="h1">
          Router Simulation
        </Typography>
        <Box>
          <Button
            variant="contained"
            startIcon={isRunning ? <PauseIcon /> : <PlayIcon />}
            onClick={handleStartStop}
            sx={{ mr: 1 }}
          >
            {isRunning ? 'Stop' : 'Start'}
          </Button>
          <Button variant="outlined" startIcon={<RefreshIcon />}>
            Refresh
          </Button>
        </Box>
      </Box>

      <Paper sx={{ mb: 3 }}>
        <Tabs value={currentTab} onChange={handleTabChange}>
          <Tab label="Routes" />
          <Tab label="Neighbors" />
        </Tabs>
      </Paper>

      {currentTab === 0 && (
        <Card>
          <CardContent>
            <Typography variant="h6" gutterBottom>
              Routing Table
            </Typography>
            <TableContainer component={Paper} sx={{ backgroundColor: 'transparent' }}>
              <Table>
                <TableHead>
                  <TableRow>
                    <TableCell>Prefix</TableCell>
                    <TableCell>Next Hop</TableCell>
                    <TableCell>Protocol</TableCell>
                    <TableCell>Metric</TableCell>
                    <TableCell>Age</TableCell>
                    <TableCell>Status</TableCell>
                  </TableRow>
                </TableHead>
                <TableBody>
                  {routes.map((route, index) => (
                    <TableRow key={index}>
                      <TableCell>
                        <Typography variant="body2" fontFamily="monospace">
                          {route.prefix}
                        </Typography>
                      </TableCell>
                      <TableCell>
                        <Typography variant="body2" fontFamily="monospace">
                          {route.nextHop}
                        </Typography>
                      </TableCell>
                      <TableCell>
                        <Chip label={route.protocol} size="small" />
                      </TableCell>
                      <TableCell>{route.metric}</TableCell>
                      <TableCell>{route.age}</TableCell>
                      <TableCell>
                        <Chip
                          label={route.status}
                          color={getStatusColor(route.status) as any}
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
      )}

      {currentTab === 1 && (
        <Card>
          <CardContent>
            <Typography variant="h6" gutterBottom>
              BGP Neighbors
            </Typography>
            <TableContainer component={Paper} sx={{ backgroundColor: 'transparent' }}>
              <Table>
                <TableHead>
                  <TableRow>
                    <TableCell>Address</TableCell>
                    <TableCell>Protocol</TableCell>
                    <TableCell>State</TableCell>
                    <TableCell>Uptime</TableCell>
                    <TableCell>Routes</TableCell>
                  </TableRow>
                </TableHead>
                <TableBody>
                  {neighbors.map((neighbor) => (
                    <TableRow key={neighbor.id}>
                      <TableCell>
                        <Typography variant="body2" fontFamily="monospace">
                          {neighbor.address}
                        </Typography>
                      </TableCell>
                      <TableCell>
                        <Chip label={neighbor.protocol} size="small" />
                      </TableCell>
                      <TableCell>
                        <Chip
                          label={neighbor.state}
                          color={getStatusColor(neighbor.state) as any}
                          size="small"
                        />
                      </TableCell>
                      <TableCell>{neighbor.uptime}</TableCell>
                      <TableCell>{neighbor.routes}</TableCell>
                    </TableRow>
                  ))}
                </TableBody>
              </Table>
            </TableContainer>
          </CardContent>
        </Card>
      )}
    </Box>
  );
};

export default RouterSimulation;