import React from 'react';
import {
  Box,
  Typography,
  Button,
  Container,
  Paper,
  Grid,
  Card,
  CardContent,
} from '@mui/material';
import {
  Home as HomeIcon,
  ArrowBack as ArrowBackIcon,
  Search as SearchIcon,
  Help as HelpIcon,
} from '@mui/icons-material';
import { useNavigate } from 'react-router-dom';

const NotFound: React.FC = () => {
  const navigate = useNavigate();

  const handleGoHome = () => {
    navigate('/');
  };

  const handleGoBack = () => {
    navigate(-1);
  };

  const quickLinks = [
    {
      title: 'Dashboard',
      description: 'View system overview and statistics',
      path: '/',
    },
    {
      title: 'Network Topology',
      description: 'Manage network nodes and connections',
      path: '/topology',
    },
    {
      title: 'Traffic Shaping',
      description: 'Configure traffic rules and policies',
      path: '/traffic-shaping',
    },
    {
      title: 'Analytics',
      description: 'View performance metrics and reports',
      path: '/analytics',
    },
    {
      title: 'Documentation',
      description: 'Read guides and API documentation',
      path: '/documentation',
    },
    {
      title: 'Settings',
      description: 'Configure system settings',
      path: '/settings',
    },
  ];

  return (
    <Container maxWidth="lg">
      <Box
        display="flex"
        flexDirection="column"
        alignItems="center"
        justifyContent="center"
        minHeight="80vh"
        textAlign="center"
      >
        {/* 404 Error */}
        <Typography
          variant="h1"
          component="h1"
          sx={{
            fontSize: { xs: '4rem', sm: '6rem', md: '8rem' },
            fontWeight: 'bold',
            color: 'primary.main',
            mb: 2,
          }}
        >
          404
        </Typography>

        <Typography
          variant="h4"
          component="h2"
          gutterBottom
          sx={{
            fontSize: { xs: '1.5rem', sm: '2rem', md: '2.5rem' },
            fontWeight: 'medium',
          }}
        >
          Page Not Found
        </Typography>

        <Typography
          variant="body1"
          color="text.secondary"
          paragraph
          sx={{
            fontSize: { xs: '1rem', sm: '1.1rem', md: '1.2rem' },
            maxWidth: '600px',
            mb: 4,
          }}
        >
          The page you're looking for doesn't exist or has been moved.
          Let's get you back on track!
        </Typography>

        {/* Action Buttons */}
        <Box display="flex" gap={2} mb={6} flexWrap="wrap" justifyContent="center">
          <Button
            variant="contained"
            size="large"
            startIcon={<HomeIcon />}
            onClick={handleGoHome}
            sx={{ minWidth: '160px' }}
          >
            Go Home
          </Button>
          <Button
            variant="outlined"
            size="large"
            startIcon={<ArrowBackIcon />}
            onClick={handleGoBack}
            sx={{ minWidth: '160px' }}
          >
            Go Back
          </Button>
        </Box>

        {/* Quick Links */}
        <Paper
          elevation={2}
          sx={{
            p: 4,
            width: '100%',
            maxWidth: '800px',
            borderRadius: 2,
          }}
        >
          <Typography
            variant="h5"
            component="h3"
            gutterBottom
            sx={{ mb: 3 }}
          >
            Quick Links
          </Typography>
          <Grid container spacing={2}>
            {quickLinks.map((link, index) => (
              <Grid item xs={12} sm={6} md={4} key={index}>
                <Card
                  sx={{
                    height: '100%',
                    cursor: 'pointer',
                    transition: 'all 0.2s ease-in-out',
                    '&:hover': {
                      transform: 'translateY(-2px)',
                      boxShadow: 4,
                    },
                  }}
                  onClick={() => navigate(link.path)}
                >
                  <CardContent>
                    <Typography variant="h6" component="h4" gutterBottom>
                      {link.title}
                    </Typography>
                    <Typography variant="body2" color="text.secondary">
                      {link.description}
                    </Typography>
                  </CardContent>
                </Card>
              </Grid>
            ))}
          </Grid>
        </Paper>

        {/* Help Section */}
        <Box mt={6} textAlign="center">
          <Typography variant="body2" color="text.secondary" paragraph>
            Still can't find what you're looking for?
          </Typography>
          <Box display="flex" gap={2} justifyContent="center" flexWrap="wrap">
            <Button
              variant="text"
              startIcon={<SearchIcon />}
              onClick={() => navigate('/documentation')}
            >
              Search Documentation
            </Button>
            <Button
              variant="text"
              startIcon={<HelpIcon />}
              onClick={() => navigate('/about')}
            >
              Get Help
            </Button>
          </Box>
        </Box>
      </Box>
    </Container>
  );
};

export default NotFound;
