import React, { useState, useEffect } from 'react';
import {
  BrowserRouter as Router,
  Routes,
  Route,
  Navigate,
} from 'react-router-dom';
import {
  ThemeProvider,
  createTheme,
  CssBaseline,
  AppBar,
  Toolbar,
  Typography,
  Box,
  Drawer,
  List,
  ListItem,
  ListItemIcon,
  ListItemText,
  IconButton,
  useMediaQuery,
  useTheme,
} from '@mui/material';
import {
  Menu as MenuIcon,
  Dashboard as DashboardIcon,
  Router as RouterIcon,
  Cloud as CloudIcon,
  Settings as SettingsIcon,
  Analytics as AnalyticsIcon,
  NetworkCheck as NetworkCheckIcon,
  Speed as SpeedIcon,
  Security as SecurityIcon,
} from '@mui/icons-material';

// Import pages
import Dashboard from './pages/Dashboard';
import CloudPods from './pages/CloudPods';
import Aviatrix from './pages/Aviatrix';
import RouterSim from './pages/RouterSim';
import TrafficShaping from './pages/TrafficShaping';
import NetworkImpairments from './pages/NetworkImpairments';
import Analytics from './pages/Analytics';
import Settings from './pages/Settings';

// Create theme
const theme = createTheme({
  palette: {
    mode: 'light',
    primary: {
      main: '#1976d2',
    },
    secondary: {
      main: '#dc004e',
    },
    background: {
      default: '#f5f5f5',
    },
  },
  typography: {
    h4: {
      fontWeight: 600,
    },
    h6: {
      fontWeight: 500,
    },
  },
});

const drawerWidth = 240;

interface AppDrawerProps {
  open: boolean;
  onClose: () => void;
}

const AppDrawer: React.FC<AppDrawerProps> = ({ open, onClose }) => {
  const menuItems = [
    { text: 'Dashboard', icon: <DashboardIcon />, path: '/' },
    { text: 'CloudPods', icon: <CloudIcon />, path: '/cloudpods' },
    { text: 'Aviatrix', icon: <SecurityIcon />, path: '/aviatrix' },
    { text: 'Router Sim', icon: <RouterIcon />, path: '/router' },
    { text: 'Traffic Shaping', icon: <SpeedIcon />, path: '/traffic' },
    { text: 'Network Impairments', icon: <NetworkCheckIcon />, path: '/impairments' },
    { text: 'Analytics', icon: <AnalyticsIcon />, path: '/analytics' },
    { text: 'Settings', icon: <SettingsIcon />, path: '/settings' },
  ];

  return (
    <Drawer
      variant="temporary"
      open={open}
      onClose={onClose}
      sx={{
        width: drawerWidth,
        flexShrink: 0,
        '& .MuiDrawer-paper': {
          width: drawerWidth,
          boxSizing: 'border-box',
        },
      }}
    >
      <Toolbar>
        <Typography variant="h6" noWrap component="div">
          Router Sim
        </Typography>
      </Toolbar>
      <List>
        {menuItems.map((item) => (
          <ListItem
            key={item.text}
            button
            component="a"
            href={item.path}
            onClick={onClose}
          >
            <ListItemIcon>{item.icon}</ListItemIcon>
            <ListItemText primary={item.text} />
          </ListItem>
        ))}
      </List>
    </Drawer>
  );
};

const App: React.FC = () => {
  const [mobileOpen, setMobileOpen] = useState(false);
  const theme = useTheme();
  const isMobile = useMediaQuery(theme.breakpoints.down('md'));

  const handleDrawerToggle = () => {
    setMobileOpen(!mobileOpen);
  };

  return (
    <ThemeProvider theme={theme}>
      <CssBaseline />
      <Router>
        <Box sx={{ display: 'flex' }}>
          <AppBar
            position="fixed"
            sx={{
              width: { md: `calc(100% - ${drawerWidth}px)` },
              ml: { md: `${drawerWidth}px` },
            }}
          >
            <Toolbar>
              <IconButton
                color="inherit"
                aria-label="open drawer"
                edge="start"
                onClick={handleDrawerToggle}
                sx={{ mr: 2, display: { md: 'none' } }}
              >
                <MenuIcon />
              </IconButton>
              <Typography variant="h6" noWrap component="div">
                Multi-Protocol Router Simulator
              </Typography>
            </Toolbar>
          </AppBar>

          <AppDrawer open={mobileOpen} onClose={handleDrawerToggle} />

          <Box
            component="main"
            sx={{
              flexGrow: 1,
              p: 3,
              width: { md: `calc(100% - ${drawerWidth}px)` },
            }}
          >
            <Toolbar />
            <Routes>
              <Route path="/" element={<Dashboard />} />
              <Route path="/cloudpods" element={<CloudPods />} />
              <Route path="/aviatrix" element={<Aviatrix />} />
              <Route path="/router" element={<RouterSim />} />
              <Route path="/traffic" element={<TrafficShaping />} />
              <Route path="/impairments" element={<NetworkImpairments />} />
              <Route path="/analytics" element={<Analytics />} />
              <Route path="/settings" element={<Settings />} />
              <Route path="*" element={<Navigate to="/" replace />} />
            </Routes>
          </Box>
        </Box>
      </Router>
    </ThemeProvider>
  );
};

export default App;
