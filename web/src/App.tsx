import React, { useState, useEffect } from 'react';
import {
  ThemeProvider,
  createTheme,
  CssBaseline,
  AppBar,
  Toolbar,
  Typography,
  Container,
  Box,
  Tabs,
  Tab,
  Paper,
  IconButton,
  Menu,
  MenuItem,
  Chip,
  Badge,
  Tooltip,
} from '@mui/material';
import {
  Router as RouterIcon,
  Cloud as CloudIcon,
  NetworkCheck as NetworkIcon,
  Analytics as AnalyticsIcon,
  Settings as SettingsIcon,
  Notifications as NotificationsIcon,
  AccountCircle as AccountIcon,
  Dashboard as DashboardIcon,
  Timeline as TimelineIcon,
  Storage as StorageIcon,
} from '@mui/icons-material';
import { BrowserRouter as Router, Routes, Route, Navigate } from 'react-router-dom';

// Import pages
import Dashboard from './pages/Dashboard';
import RouterSimulation from './pages/RouterSimulation';
import CloudNetworking from './pages/CloudNetworking';
import CloudPods from './pages/CloudPods';
import AviatrixIntegration from './pages/AviatrixIntegration';
import Analytics from './pages/Analytics';
import NetworkTopology from './pages/NetworkTopology';
import TrafficShaping from './pages/TrafficShaping';
import NetworkImpairments from './pages/NetworkImpairments';

// Create theme
const theme = createTheme({
  palette: {
    mode: 'dark',
    primary: {
      main: '#1976d2',
    },
    secondary: {
      main: '#dc004e',
    },
    background: {
      default: '#0a0a0a',
      paper: '#1a1a1a',
    },
  },
  typography: {
    fontFamily: '"Roboto", "Helvetica", "Arial", sans-serif',
    h4: {
      fontWeight: 600,
    },
    h6: {
      fontWeight: 500,
    },
  },
  components: {
    MuiPaper: {
      styleOverrides: {
        root: {
          backgroundImage: 'none',
        },
      },
    },
  },
});

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
      id={`simple-tabpanel-${index}`}
      aria-labelledby={`simple-tab-${index}`}
      {...other}
    >
      {value === index && <Box sx={{ p: 3 }}>{children}</Box>}
    </div>
  );
}

function App() {
  const [currentTab, setCurrentTab] = useState(0);
  const [anchorEl, setAnchorEl] = useState<null | HTMLElement>(null);
  const [notifications, setNotifications] = useState(5);
  const [systemStatus, setSystemStatus] = useState('online');

  const handleTabChange = (event: React.SyntheticEvent, newValue: number) => {
    setCurrentTab(newValue);
  };

  const handleMenuOpen = (event: React.MouseEvent<HTMLElement>) => {
    setAnchorEl(event.currentTarget);
  };

  const handleMenuClose = () => {
    setAnchorEl(null);
  };

  const tabs = [
    { label: 'Dashboard', icon: <DashboardIcon />, component: <Dashboard /> },
    { label: 'Router Simulation', icon: <RouterIcon />, component: <RouterSimulation /> },
    { label: 'Cloud Networking', icon: <CloudIcon />, component: <CloudNetworking /> },
    { label: 'CloudPods', icon: <StorageIcon />, component: <CloudPods /> },
    { label: 'Aviatrix', icon: <NetworkIcon />, component: <AviatrixIntegration /> },
    { label: 'Analytics', icon: <AnalyticsIcon />, component: <Analytics /> },
    { label: 'Topology', icon: <TimelineIcon />, component: <NetworkTopology /> },
    { label: 'Traffic Shaping', icon: <NetworkIcon />, component: <TrafficShaping /> },
    { label: 'Impairments', icon: <SettingsIcon />, component: <NetworkImpairments /> },
  ];

  return (
    <ThemeProvider theme={theme}>
      <CssBaseline />
      <Router>
        <Box sx={{ flexGrow: 1 }}>
          <AppBar position="static" elevation={0}>
            <Toolbar>
              <RouterIcon sx={{ mr: 2 }} />
              <Typography variant="h6" component="div" sx={{ flexGrow: 1 }}>
                Multi-Protocol Router Simulator
              </Typography>
              
              <Box sx={{ display: 'flex', alignItems: 'center', gap: 2 }}>
                <Chip
                  label={systemStatus}
                  color={systemStatus === 'online' ? 'success' : 'error'}
                  size="small"
                />
                
                <Tooltip title="Notifications">
                  <IconButton color="inherit">
                    <Badge badgeContent={notifications} color="error">
                      <NotificationsIcon />
                    </Badge>
                  </IconButton>
                </Tooltip>
                
                <Tooltip title="Account">
                  <IconButton
                    color="inherit"
                    onClick={handleMenuOpen}
                  >
                    <AccountIcon />
                  </IconButton>
                </Tooltip>
              </Box>
            </Toolbar>
          </AppBar>

          <Container maxWidth="xl" sx={{ mt: 2 }}>
            <Paper elevation={1} sx={{ mb: 2 }}>
              <Tabs
                value={currentTab}
                onChange={handleTabChange}
                variant="scrollable"
                scrollButtons="auto"
                sx={{ borderBottom: 1, borderColor: 'divider' }}
              >
                {tabs.map((tab, index) => (
                  <Tab
                    key={index}
                    label={tab.label}
                    icon={tab.icon}
                    iconPosition="start"
                  />
                ))}
              </Tabs>
            </Paper>

            <Box sx={{ minHeight: '70vh' }}>
              {tabs.map((tab, index) => (
                <TabPanel key={index} value={currentTab} index={index}>
                  {tab.component}
                </TabPanel>
              ))}
            </Box>
          </Container>

          <Menu
            anchorEl={anchorEl}
            open={Boolean(anchorEl)}
            onClose={handleMenuClose}
          >
            <MenuItem onClick={handleMenuClose}>Profile</MenuItem>
            <MenuItem onClick={handleMenuClose}>Settings</MenuItem>
            <MenuItem onClick={handleMenuClose}>Logout</MenuItem>
          </Menu>
        </Box>
      </Router>
    </ThemeProvider>
  );
}

export default App;
