import React, { useState, useEffect } from 'react';
import {
  Box,
  AppBar,
  Toolbar,
  Typography,
  Drawer,
  List,
  ListItem,
  ListItemIcon,
  ListItemText,
  IconButton,
  CssBaseline,
  ThemeProvider,
  createTheme,
  useTheme,
  useMediaQuery,
  Badge,
  Avatar,
  Menu,
  MenuItem,
  Divider,
  Chip,
  LinearProgress,
  Alert,
  Snackbar,
} from '@mui/material';
import {
  Menu as MenuIcon,
  Dashboard as DashboardIcon,
  Cloud as CloudIcon,
  Router as RouterIcon,
  Analytics as AnalyticsIcon,
  Settings as SettingsIcon,
  Notifications as NotificationsIcon,
  AccountCircle as AccountCircleIcon,
  NetworkCheck as NetworkCheckIcon,
  Security as SecurityIcon,
  Speed as SpeedIcon,
  Storage as StorageIcon,
  VpnKey as VpnKeyIcon,
  Timeline as TimelineIcon,
  Assessment as AssessmentIcon,
  BugReport as BugReportIcon,
  Help as HelpIcon,
  Info as InfoIcon,
} from '@mui/icons-material';
import { BrowserRouter as Router, Routes, Route, useNavigate, useLocation } from 'react-router-dom';

// Import pages
import Dashboard from './pages/Dashboard';
import CloudPods from './pages/CloudPods';
import Aviatrix from './pages/Aviatrix';
import RouterSim from './pages/RouterSim';
import Analytics from './pages/Analytics';
import NetworkTopology from './pages/NetworkTopology';
import TrafficShaping from './pages/TrafficShaping';
import NetworkImpairments from './pages/NetworkImpairments';
import Testing from './pages/Testing';
import Settings from './pages/Settings';
import Documentation from './pages/Documentation';
import About from './pages/About';

// Import components
import LoadingSpinner from './components/LoadingSpinner';
import ErrorBoundary from './components/ErrorBoundary';
import Notification from './components/Notification';

// Theme configuration
const theme = createTheme({
  palette: {
    mode: 'light',
    primary: {
      main: '#1976d2',
      light: '#42a5f5',
      dark: '#1565c0',
    },
    secondary: {
      main: '#dc004e',
      light: '#ff5983',
      dark: '#9a0036',
    },
    background: {
      default: '#f5f5f5',
      paper: '#ffffff',
    },
    success: {
      main: '#2e7d32',
    },
    warning: {
      main: '#ed6c02',
    },
    error: {
      main: '#d32f2f',
    },
    info: {
      main: '#0288d1',
    },
  },
  typography: {
    fontFamily: '"Roboto", "Helvetica", "Arial", sans-serif',
    h1: {
      fontSize: '2.5rem',
      fontWeight: 600,
    },
    h2: {
      fontSize: '2rem',
      fontWeight: 600,
    },
    h3: {
      fontSize: '1.75rem',
      fontWeight: 600,
    },
    h4: {
      fontSize: '1.5rem',
      fontWeight: 600,
    },
    h5: {
      fontSize: '1.25rem',
      fontWeight: 600,
    },
    h6: {
      fontSize: '1rem',
      fontWeight: 600,
    },
  },
  shape: {
    borderRadius: 8,
  },
  components: {
    MuiCard: {
      styleOverrides: {
        root: {
          boxShadow: '0 2px 8px rgba(0,0,0,0.1)',
          '&:hover': {
            boxShadow: '0 4px 16px rgba(0,0,0,0.15)',
          },
        },
      },
    },
    MuiButton: {
      styleOverrides: {
        root: {
          textTransform: 'none',
          fontWeight: 600,
        },
      },
    },
  },
});

const drawerWidth = 280;

// Navigation items
const navigationItems = [
  { id: 'dashboard', label: 'Dashboard', icon: <DashboardIcon />, path: '/' },
  { id: 'cloudpods', label: 'CloudPods', icon: <CloudIcon />, path: '/cloudpods' },
  { id: 'aviatrix', label: 'Aviatrix', icon: <VpnKeyIcon />, path: '/aviatrix' },
  { id: 'router', label: 'Router Sim', icon: <RouterIcon />, path: '/router' },
  { id: 'topology', label: 'Network Topology', icon: <NetworkCheckIcon />, path: '/topology' },
  { id: 'traffic', label: 'Traffic Shaping', icon: <SpeedIcon />, path: '/traffic' },
  { id: 'impairments', label: 'Network Impairments', icon: <BugReportIcon />, path: '/impairments' },
  { id: 'analytics', label: 'Analytics', icon: <AnalyticsIcon />, path: '/analytics' },
  { id: 'testing', label: 'Testing', icon: <AssessmentIcon />, path: '/testing' },
  { id: 'settings', label: 'Settings', icon: <SettingsIcon />, path: '/settings' },
  { id: 'docs', label: 'Documentation', icon: <HelpIcon />, path: '/docs' },
  { id: 'about', label: 'About', icon: <InfoIcon />, path: '/about' },
];

// Main App component
function AppContent() {
  const [mobileOpen, setMobileOpen] = useState(false);
  const [loading, setLoading] = useState(false);
  const [notifications, setNotifications] = useState([]);
  const [anchorEl, setAnchorEl] = useState(null);
  const [user, setUser] = useState({
    name: 'Admin User',
    email: 'admin@router-sim.com',
    avatar: '/api/avatars/admin',
  });

  const theme = useTheme();
  const isMobile = useMediaQuery(theme.breakpoints.down('md'));
  const navigate = useNavigate();
  const location = useLocation();

  const handleDrawerToggle = () => {
    setMobileOpen(!mobileOpen);
  };

  const handleProfileMenuOpen = (event) => {
    setAnchorEl(event.currentTarget);
  };

  const handleProfileMenuClose = () => {
    setAnchorEl(null);
  };

  const handleLogout = () => {
    // Implement logout logic
    console.log('Logout clicked');
    handleProfileMenuClose();
  };

  const addNotification = (message, severity = 'info') => {
    const notification = {
      id: Date.now(),
      message,
      severity,
      timestamp: new Date(),
    };
    setNotifications(prev => [...prev, notification]);
  };

  const removeNotification = (id) => {
    setNotifications(prev => prev.filter(n => n.id !== id));
  };

  // Simulate loading states
  useEffect(() => {
    const handleRouteChange = () => {
      setLoading(true);
      setTimeout(() => setLoading(false), 500);
    };

    handleRouteChange();
  }, [location.pathname]);

  // Drawer content
  const drawer = (
    <Box>
      <Box sx={{ p: 2, textAlign: 'center', borderBottom: 1, borderColor: 'divider' }}>
        <Typography variant="h6" component="div" sx={{ fontWeight: 'bold', color: 'primary.main' }}>
          🚀 Router Simulator
        </Typography>
        <Typography variant="caption" color="text.secondary">
          Multi-Cloud Networking Platform
        </Typography>
      </Box>
      <List>
        {navigationItems.map((item) => (
          <ListItem
            key={item.id}
            button
            onClick={() => {
              navigate(item.path);
              if (isMobile) {
                setMobileOpen(false);
              }
            }}
            selected={location.pathname === item.path}
            sx={{
              '&.Mui-selected': {
                backgroundColor: 'primary.light',
                color: 'primary.contrastText',
                '& .MuiListItemIcon-root': {
                  color: 'primary.contrastText',
                },
              },
              '&:hover': {
                backgroundColor: 'action.hover',
              },
            }}
          >
            <ListItemIcon>
              {item.icon}
            </ListItemIcon>
            <ListItemText primary={item.label} />
          </ListItem>
        ))}
      </List>
      <Divider />
      <Box sx={{ p: 2 }}>
        <Typography variant="caption" color="text.secondary" display="block">
          System Status
        </Typography>
        <Box sx={{ display: 'flex', alignItems: 'center', mt: 1 }}>
          <Chip
            label="Online"
            color="success"
            size="small"
            sx={{ mr: 1 }}
          />
          <Typography variant="caption" color="text.secondary">
            All systems operational
          </Typography>
        </Box>
      </Box>
    </Box>
  );

  return (
    <Box sx={{ display: 'flex' }}>
      <CssBaseline />
      
      {/* App Bar */}
      <AppBar
        position="fixed"
        sx={{
          width: { md: `calc(100% - ${drawerWidth}px)` },
          ml: { md: `${drawerWidth}px` },
          zIndex: theme.zIndex.drawer + 1,
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
          
          <Typography variant="h6" noWrap component="div" sx={{ flexGrow: 1 }}>
            {navigationItems.find(item => item.path === location.pathname)?.label || 'Router Simulator'}
          </Typography>

          <Box sx={{ display: 'flex', alignItems: 'center', gap: 1 }}>
            <IconButton color="inherit">
              <Badge badgeContent={4} color="error">
                <NotificationsIcon />
              </Badge>
            </IconButton>
            
            <IconButton
              color="inherit"
              onClick={handleProfileMenuOpen}
              sx={{ p: 0 }}
            >
              <Avatar sx={{ width: 32, height: 32 }}>
                {user.name.charAt(0)}
              </Avatar>
            </IconButton>
          </Box>
        </Toolbar>
      </AppBar>

      {/* Profile Menu */}
      <Menu
        anchorEl={anchorEl}
        open={Boolean(anchorEl)}
        onClose={handleProfileMenuClose}
        onClick={handleProfileMenuClose}
        PaperProps={{
          elevation: 0,
          sx: {
            overflow: 'visible',
            filter: 'drop-shadow(0px 2px 8px rgba(0,0,0,0.32))',
            mt: 1.5,
            '& .MuiAvatar-root': {
              width: 32,
              height: 32,
              ml: -0.5,
              mr: 1,
            },
            '&:before': {
              content: '""',
              display: 'block',
              position: 'absolute',
              top: 0,
              right: 14,
              width: 10,
              height: 10,
              bgcolor: 'background.paper',
              transform: 'translateY(-50%) rotate(45deg)',
              zIndex: 0,
            },
          },
        }}
        transformOrigin={{ horizontal: 'right', vertical: 'top' }}
        anchorOrigin={{ horizontal: 'right', vertical: 'bottom' }}
      >
        <MenuItem onClick={handleProfileMenuClose}>
          <Avatar /> Profile
        </MenuItem>
        <MenuItem onClick={handleProfileMenuClose}>
          <AccountCircleIcon sx={{ mr: 1 }} /> My Account
        </MenuItem>
        <Divider />
        <MenuItem onClick={handleLogout}>
          <Typography color="error">Logout</Typography>
        </MenuItem>
      </Menu>

      {/* Navigation Drawer */}
      <Box
        component="nav"
        sx={{ width: { md: drawerWidth }, flexShrink: { md: 0 } }}
      >
        <Drawer
          variant="temporary"
          open={mobileOpen}
          onClose={handleDrawerToggle}
          ModalProps={{
            keepMounted: true, // Better open performance on mobile.
          }}
          sx={{
            display: { xs: 'block', md: 'none' },
            '& .MuiDrawer-paper': { boxSizing: 'border-box', width: drawerWidth },
          }}
        >
          {drawer}
        </Drawer>
        <Drawer
          variant="permanent"
          sx={{
            display: { xs: 'none', md: 'block' },
            '& .MuiDrawer-paper': { boxSizing: 'border-box', width: drawerWidth },
          }}
          open
        >
          {drawer}
        </Drawer>
      </Box>

      {/* Main Content */}
      <Box
        component="main"
        sx={{
          flexGrow: 1,
          p: 3,
          width: { md: `calc(100% - ${drawerWidth}px)` },
          mt: 8,
        }}
      >
        {loading && <LinearProgress sx={{ position: 'absolute', top: 0, left: 0, right: 0 }} />}
        
        <ErrorBoundary>
          <Routes>
            <Route path="/" element={<Dashboard />} />
            <Route path="/cloudpods" element={<CloudPods />} />
            <Route path="/aviatrix" element={<Aviatrix />} />
            <Route path="/router" element={<RouterSim />} />
            <Route path="/topology" element={<NetworkTopology />} />
            <Route path="/traffic" element={<TrafficShaping />} />
            <Route path="/impairments" element={<NetworkImpairments />} />
            <Route path="/analytics" element={<Analytics />} />
            <Route path="/testing" element={<Testing />} />
            <Route path="/settings" element={<Settings />} />
            <Route path="/docs" element={<Documentation />} />
            <Route path="/about" element={<About />} />
          </Routes>
        </ErrorBoundary>
      </Box>

      {/* Notifications */}
      <Snackbar
        open={notifications.length > 0}
        autoHideDuration={6000}
        onClose={() => removeNotification(notifications[0]?.id)}
        anchorOrigin={{ vertical: 'bottom', horizontal: 'right' }}
      >
        <Alert
          onClose={() => removeNotification(notifications[0]?.id)}
          severity={notifications[0]?.severity || 'info'}
          sx={{ width: '100%' }}
        >
          {notifications[0]?.message}
        </Alert>
      </Snackbar>
    </Box>
  );
}

// Main App component with theme provider
function App() {
  return (
    <ThemeProvider theme={theme}>
      <Router>
        <AppContent />
      </Router>
    </ThemeProvider>
  );
}

export default App;
