import React from 'react';
import { BrowserRouter as Router, Routes, Route, Navigate } from 'react-router-dom';
import { ThemeProvider, createTheme } from '@mui/material/styles';
import CssBaseline from '@mui/material/CssBaseline';
import { Box, AppBar, Toolbar, Typography, Container, Tabs, Tab } from '@mui/material';
import { Cloud as CloudIcon, Router as RouterIcon, Analytics as AnalyticsIcon } from '@mui/icons-material';
import CloudNetworking from './pages/CloudNetworking';
import CloudPods from './pages/CloudPods';

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
      {value === index && (
        <Box sx={{ p: 3 }}>
          {children}
        </Box>
      )}
    </div>
  );
}

function App() {
  const [value, setValue] = React.useState(0);

  const handleChange = (event: React.SyntheticEvent, newValue: number) => {
    setValue(newValue);
  };

  return (
    <ThemeProvider theme={theme}>
      <CssBaseline />
      <Router>
        <Box sx={{ flexGrow: 1 }}>
          <AppBar position="static" sx={{ backgroundColor: '#1a1a1a', borderBottom: '1px solid #333' }}>
            <Toolbar>
              <RouterIcon sx={{ mr: 2 }} />
              <Typography variant="h6" component="div" sx={{ flexGrow: 1 }}>
                Router Simulator - Multi-Cloud Networking Platform
              </Typography>
            </Toolbar>
            <Tabs 
              value={value} 
              onChange={handleChange} 
              aria-label="navigation tabs"
              sx={{ borderBottom: 1, borderColor: 'divider' }}
            >
              <Tab 
                icon={<CloudIcon />} 
                label="Cloud Networking" 
                id="tab-0"
                aria-controls="tabpanel-0"
              />
              <Tab 
                icon={<CloudIcon />} 
                label="CloudPods" 
                id="tab-1"
                aria-controls="tabpanel-1"
              />
              <Tab 
                icon={<AnalyticsIcon />} 
                label="Analytics" 
                id="tab-2"
                aria-controls="tabpanel-2"
              />
            </Tabs>
          </AppBar>
          
          <Container maxWidth="xl" sx={{ mt: 2 }}>
            <TabPanel value={value} index={0}>
              <CloudNetworking />
            </TabPanel>
            <TabPanel value={value} index={1}>
              <CloudPods />
            </TabPanel>
            <TabPanel value={value} index={2}>
              <Box sx={{ textAlign: 'center', py: 8 }}>
                <Typography variant="h4" gutterBottom>
                  Analytics Dashboard
                </Typography>
                <Typography variant="body1" color="text.secondary">
                  Real-time analytics and monitoring coming soon...
                </Typography>
              </Box>
            </TabPanel>
          </Container>
        </Box>
      </Router>
    </ThemeProvider>
  );
}

export default App;
