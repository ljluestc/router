import React, { useState } from 'react';
import { useNavigate, useLocation } from 'react-router-dom';
import {
  Drawer,
  List,
  ListItem,
  ListItemButton,
  ListItemIcon,
  ListItemText,
  Typography,
  Box,
  Divider,
  IconButton,
  Tooltip,
} from '@mui/material';
import {
  Dashboard as DashboardIcon,
  Cloud as CloudIcon,
  Router as RouterIcon,
  Speed as SpeedIcon,
  NetworkCheck as NetworkIcon,
  Analytics as AnalyticsIcon,
  Settings as SettingsIcon,
  Menu as MenuIcon,
  Close as CloseIcon,
} from '@mui/icons-material';

const drawerWidth = 240;

const menuItems = [
  { text: 'Dashboard', icon: <DashboardIcon />, path: '/dashboard' },
  { text: 'CloudPods', icon: <CloudIcon />, path: '/cloudpods' },
  { text: 'Aviatrix', icon: <RouterIcon />, path: '/aviatrix' },
  { text: 'Router Sim', icon: <RouterIcon />, path: '/router-sim' },
  { text: 'Traffic Shaping', icon: <SpeedIcon />, path: '/traffic-shaping' },
  { text: 'Network Impairments', icon: <NetworkIcon />, path: '/network-impairments' },
  { text: 'Analytics', icon: <AnalyticsIcon />, path: '/analytics' },
  { text: 'Settings', icon: <SettingsIcon />, path: '/settings' },
];

const Navbar: React.FC = () => {
  const navigate = useNavigate();
  const location = useLocation();
  const [mobileOpen, setMobileOpen] = useState(false);

  const handleDrawerToggle = () => {
    setMobileOpen(!mobileOpen);
  };

  const handleNavigation = (path: string) => {
    navigate(path);
    setMobileOpen(false);
  };

  const drawer = (
    <Box>
      <Box sx={{ p: 2, display: 'flex', alignItems: 'center', justifyContent: 'space-between' }}>
        <Typography variant="h6" component="div" sx={{ fontWeight: 'bold', color: 'primary.main' }}>
          Router Sim
        </Typography>
        <IconButton
          onClick={handleDrawerToggle}
          sx={{ display: { xs: 'block', sm: 'none' } }}
        >
          <CloseIcon />
        </IconButton>
      </Box>
      <Divider />
      <List>
        {menuItems.map((item) => (
          <ListItem key={item.text} disablePadding>
            <ListItemButton
              selected={location.pathname === item.path}
              onClick={() => handleNavigation(item.path)}
              sx={{
                mx: 1,
                borderRadius: 2,
                '&.Mui-selected': {
                  backgroundColor: 'primary.main',
                  color: 'white',
                  '&:hover': {
                    backgroundColor: 'primary.dark',
                  },
                  '& .MuiListItemIcon-root': {
                    color: 'white',
                  },
                },
                '&:hover': {
                  backgroundColor: 'primary.light',
                  color: 'white',
                  '& .MuiListItemIcon-root': {
                    color: 'white',
                  },
                },
              }}
            >
              <ListItemIcon sx={{ minWidth: 40 }}>
                {item.icon}
              </ListItemIcon>
              <ListItemText primary={item.text} />
            </ListItemButton>
          </ListItem>
        ))}
      </List>
    </Box>
  );

  return (
    <>
      <IconButton
        color="inherit"
        aria-label="open drawer"
        edge="start"
        onClick={handleDrawerToggle}
        sx={{ 
          position: 'fixed',
          top: 16,
          left: 16,
          zIndex: 1300,
          display: { xs: 'block', sm: 'none' },
          backgroundColor: 'white',
          boxShadow: 2,
        }}
      >
        <MenuIcon />
      </IconButton>
      
      <Drawer
        variant="permanent"
        sx={{
          display: { xs: 'none', sm: 'block' },
          '& .MuiDrawer-paper': {
            boxSizing: 'border-box',
            width: drawerWidth,
            backgroundColor: 'white',
            borderRight: '1px solid #e0e0e0',
          },
        }}
        open
      >
        {drawer}
      </Drawer>
      
      <Drawer
        variant="temporary"
        open={mobileOpen}
        onClose={handleDrawerToggle}
        ModalProps={{
          keepMounted: true,
        }}
        sx={{
          display: { xs: 'block', sm: 'none' },
          '& .MuiDrawer-paper': {
            boxSizing: 'border-box',
            width: drawerWidth,
            backgroundColor: 'white',
          },
        }}
      >
        {drawer}
      </Drawer>
    </>
  );
};

export default Navbar;
