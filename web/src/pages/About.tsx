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
  Divider,
  Link,
  Avatar,
  Button,
} from '@mui/material';
import {
  Code as CodeIcon,
  BugReport as BugReportIcon,
  Security as SecurityIcon,
  Speed as SpeedIcon,
  NetworkCheck as NetworkCheckIcon,
  Cloud as CloudIcon,
  Storage as StorageIcon,
  Memory as MemoryIcon,
  GitHub as GitHubIcon,
  Email as EmailIcon,
  Language as LanguageIcon,
  Build as BuildIcon,
} from '@mui/icons-material';

const About: React.FC = () => {
  const features = [
    {
      icon: <NetworkCheckIcon />,
      title: 'Multi-Protocol Support',
      description: 'BGP, OSPF, IS-IS routing protocols with FRR integration',
    },
    {
      icon: <SpeedIcon />,
      title: 'Traffic Shaping',
      description: 'Token Bucket, WFQ, and Traffic Control (tc) implementation',
    },
    {
      icon: <BugReportIcon />,
      title: 'Network Impairments',
      description: 'Latency, jitter, packet loss simulation using netem',
    },
    {
      icon: <CloudIcon />,
      title: 'Cloud Networking',
      description: 'CloudPods and Aviatrix integration for multi-cloud scenarios',
    },
    {
      icon: <StorageIcon />,
      title: 'Analytics Engine',
      description: 'ClickHouse integration for time-series data and analytics',
    },
    {
      icon: <CodeIcon />,
      title: 'Multi-Language API',
      description: 'C++, Go, Rust, and REST API for comprehensive integration',
    },
  ];

  const technologies = [
    { name: 'C++', version: '17', description: 'Core router implementation' },
    { name: 'Go', version: '1.21', description: 'API server and cloud integrations' },
    { name: 'Rust', version: '1.70', description: 'High-performance analytics engine' },
    { name: 'React', version: '18', description: 'Modern web interface' },
    { name: 'TypeScript', version: '5.0', description: 'Type-safe web development' },
    { name: 'FRR', version: '8.5', description: 'Free Range Routing protocol stack' },
    { name: 'ClickHouse', version: '23.8', description: 'Time-series database' },
    { name: 'Docker', version: '24.0', description: 'Containerization platform' },
  ];

  const contributors = [
    {
      name: 'Development Team',
      role: 'Core Development',
      avatar: <Avatar><CodeIcon /></Avatar>,
    },
    {
      name: 'CloudPods Community',
      role: 'Cloud Integration',
      avatar: <Avatar><CloudIcon /></Avatar>,
    },
    {
      name: 'Aviatrix Community',
      role: 'Multi-Cloud Networking',
      avatar: <Avatar><NetworkCheckIcon /></Avatar>,
    },
    {
      name: 'Open Source Contributors',
      role: 'Community Support',
      avatar: <Avatar><GitHubIcon /></Avatar>,
    },
  ];

  const licenses = [
    {
      name: 'MIT License',
      description: 'Main project license',
      url: 'https://opensource.org/licenses/MIT',
    },
    {
      name: 'GPL v2',
      description: 'FRR routing stack',
      url: 'https://www.gnu.org/licenses/gpl-2.0.html',
    },
    {
      name: 'Apache 2.0',
      description: 'ClickHouse database',
      url: 'https://www.apache.org/licenses/LICENSE-2.0',
    },
  ];

  return (
    <Box>
      <Typography variant="h4" component="h1" gutterBottom>
        About Multi-Protocol Router Simulator
      </Typography>

      <Grid container spacing={3}>
        {/* Project Overview */}
        <Grid item xs={12}>
          <Card>
            <CardContent>
              <Typography variant="h5" gutterBottom>
                Project Overview
              </Typography>
              <Typography variant="body1" paragraph>
                The Multi-Protocol Router Simulator is a comprehensive network simulation platform
                designed for testing, development, and education in network routing and traffic management.
                It provides a realistic environment for simulating complex network topologies,
                routing protocols, and traffic patterns.
              </Typography>
              <Typography variant="body1" paragraph>
                Built with modern technologies and designed for scalability, the simulator supports
                multiple routing protocols, traffic shaping algorithms, network impairment simulation,
                and cloud networking integrations. It serves as both a learning tool for network
                engineers and a testing platform for network applications.
              </Typography>
            </CardContent>
          </Card>
        </Grid>

        {/* Key Features */}
        <Grid item xs={12}>
          <Card>
            <CardContent>
              <Typography variant="h5" gutterBottom>
                Key Features
              </Typography>
              <Grid container spacing={2}>
                {features.map((feature, index) => (
                  <Grid item xs={12} md={6} key={index}>
                    <Paper sx={{ p: 2, height: '100%' }}>
                      <Box display="flex" alignItems="center" mb={1}>
                        <Box mr={2} color="primary.main">
                          {feature.icon}
                        </Box>
                        <Typography variant="h6">
                          {feature.title}
                        </Typography>
                      </Box>
                      <Typography variant="body2" color="text.secondary">
                        {feature.description}
                      </Typography>
                    </Paper>
                  </Grid>
                ))}
              </Grid>
            </CardContent>
          </Card>
        </Grid>

        {/* Technology Stack */}
        <Grid item xs={12} md={6}>
          <Card>
            <CardContent>
              <Typography variant="h5" gutterBottom>
                Technology Stack
              </Typography>
              <List>
                {technologies.map((tech, index) => (
                  <ListItem key={index}>
                    <ListItemIcon>
                      <BuildIcon />
                    </ListItemIcon>
                    <ListItemText
                      primary={`${tech.name} ${tech.version}`}
                      secondary={tech.description}
                    />
                  </ListItem>
                ))}
              </List>
            </CardContent>
          </Card>
        </Grid>

        {/* Architecture */}
        <Grid item xs={12} md={6}>
          <Card>
            <CardContent>
              <Typography variant="h5" gutterBottom>
                Architecture
              </Typography>
              <Typography variant="body2" paragraph>
                The simulator follows a modular architecture with clear separation of concerns:
              </Typography>
              <List dense>
                <ListItem>
                  <ListItemText
                    primary="C++ Core"
                    secondary="Router implementation, protocol handling, traffic shaping"
                  />
                </ListItem>
                <ListItem>
                  <ListItemText
                    primary="Go API Server"
                    secondary="REST API, cloud integrations, configuration management"
                  />
                </ListItem>
                <ListItem>
                  <ListItemText
                    primary="Rust Analytics"
                    secondary="High-performance packet processing and analytics"
                  />
                </ListItem>
                <ListItem>
                  <ListItemText
                    primary="React Web UI"
                    secondary="Modern web interface for monitoring and control"
                  />
                </ListItem>
                <ListItem>
                  <ListItemText
                    primary="ClickHouse Database"
                    secondary="Time-series data storage and analytics"
                  />
                </ListItem>
              </List>
            </CardContent>
          </Card>
        </Grid>

        {/* Contributors */}
        <Grid item xs={12} md={6}>
          <Card>
            <CardContent>
              <Typography variant="h5" gutterBottom>
                Contributors
              </Typography>
              <List>
                {contributors.map((contributor, index) => (
                  <ListItem key={index}>
                    <ListItemIcon>
                      {contributor.avatar}
                    </ListItemIcon>
                    <ListItemText
                      primary={contributor.name}
                      secondary={contributor.role}
                    />
                  </ListItem>
                ))}
              </List>
            </CardContent>
          </Card>
        </Grid>

        {/* Licenses */}
        <Grid item xs={12} md={6}>
          <Card>
            <CardContent>
              <Typography variant="h5" gutterBottom>
                Licenses
              </Typography>
              <List>
                {licenses.map((license, index) => (
                  <ListItem key={index}>
                    <ListItemText
                      primary={license.name}
                      secondary={license.description}
                    />
                    <Link href={license.url} target="_blank" rel="noopener">
                      View License
                    </Link>
                  </ListItem>
                ))}
              </List>
            </CardContent>
          </Card>
        </Grid>

        {/* Version Information */}
        <Grid item xs={12}>
          <Card>
            <CardContent>
              <Typography variant="h5" gutterBottom>
                Version Information
              </Typography>
              <Grid container spacing={2}>
                <Grid item xs={12} md={4}>
                  <Paper sx={{ p: 2, textAlign: 'center' }}>
                    <Typography variant="h6" color="primary">
                      v1.0.0
                    </Typography>
                    <Typography variant="body2" color="text.secondary">
                      Current Version
                    </Typography>
                  </Paper>
                </Grid>
                <Grid item xs={12} md={4}>
                  <Paper sx={{ p: 2, textAlign: 'center' }}>
                    <Typography variant="h6" color="success.main">
                      Stable
                    </Typography>
                    <Typography variant="body2" color="text.secondary">
                      Release Status
                    </Typography>
                  </Paper>
                </Grid>
                <Grid item xs={12} md={4}>
                  <Paper sx={{ p: 2, textAlign: 'center' }}>
                    <Typography variant="h6" color="info.main">
                      2024
                    </Typography>
                    <Typography variant="body2" color="text.secondary">
                      Release Year
                    </Typography>
                  </Paper>
                </Grid>
              </Grid>
            </CardContent>
          </Card>
        </Grid>

        {/* Getting Started */}
        <Grid item xs={12}>
          <Card>
            <CardContent>
              <Typography variant="h5" gutterBottom>
                Getting Started
              </Typography>
              <Typography variant="body1" paragraph>
                Ready to start using the Multi-Protocol Router Simulator? Here are some quick links:
              </Typography>
              <Box display="flex" gap={2} flexWrap="wrap">
                <Button
                  variant="contained"
                  startIcon={<GitHubIcon />}
                  href="https://github.com/your-org/router-sim"
                  target="_blank"
                  rel="noopener"
                >
                  View on GitHub
                </Button>
                <Button
                  variant="outlined"
                  startIcon={<LanguageIcon />}
                  href="/documentation"
                >
                  Read Documentation
                </Button>
                <Button
                  variant="outlined"
                  startIcon={<EmailIcon />}
                  href="mailto:support@router-sim.com"
                >
                  Contact Support
                </Button>
              </Box>
            </CardContent>
          </Card>
        </Grid>

        {/* System Requirements */}
        <Grid item xs={12}>
          <Card>
            <CardContent>
              <Typography variant="h5" gutterBottom>
                System Requirements
              </Typography>
              <Grid container spacing={2}>
                <Grid item xs={12} md={6}>
                  <Typography variant="h6" gutterBottom>
                    Minimum Requirements
                  </Typography>
                  <List dense>
                    <ListItem>
                      <ListItemText
                        primary="CPU"
                        secondary="2 cores, 2.0 GHz"
                      />
                    </ListItem>
                    <ListItem>
                      <ListItemText
                        primary="RAM"
                        secondary="4 GB"
                      />
                    </ListItem>
                    <ListItem>
                      <ListItemText
                        primary="Storage"
                        secondary="10 GB free space"
                      />
                    </ListItem>
                    <ListItem>
                      <ListItemText
                        primary="OS"
                        secondary="Linux (Ubuntu 20.04+), macOS, Windows 10+"
                      />
                    </ListItem>
                  </List>
                </Grid>
                <Grid item xs={12} md={6}>
                  <Typography variant="h6" gutterBottom>
                    Recommended Requirements
                  </Typography>
                  <List dense>
                    <ListItem>
                      <ListItemText
                        primary="CPU"
                        secondary="4+ cores, 3.0+ GHz"
                      />
                    </ListItem>
                    <ListItem>
                      <ListItemText
                        primary="RAM"
                        secondary="8+ GB"
                      />
                    </ListItem>
                    <ListItem>
                      <ListItemText
                        primary="Storage"
                        secondary="50+ GB free space"
                      />
                    </ListItem>
                    <ListItem>
                      <ListItemText
                        primary="Network"
                        secondary="Gigabit Ethernet"
                      />
                    </ListItem>
                  </List>
                </Grid>
              </Grid>
            </CardContent>
          </Card>
        </Grid>
      </Grid>
    </Box>
  );
};

export default About;
