import React, { useState, useEffect } from 'react';
import {
  Box,
  Card,
  CardContent,
  Typography,
  Button,
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
  Dialog,
  DialogTitle,
  DialogContent,
  DialogActions,
  TextField,
  MenuItem,
} from '@mui/material';
import {
  Add as AddIcon,
  Edit as EditIcon,
  Delete as DeleteIcon,
  Refresh as RefreshIcon,
  Cloud as CloudIcon,
} from '@mui/icons-material';

interface CloudPodsResource {
  id: string;
  name: string;
  type: string;
  status: string;
  region: string;
  created_at: string;
  tags: string[];
}

const CloudPods: React.FC = () => {
  const [resources, setResources] = useState<CloudPodsResource[]>([]);
  const [loading, setLoading] = useState(true);
  const [openDialog, setOpenDialog] = useState(false);
  const [selectedResource, setSelectedResource] = useState<CloudPodsResource | null>(null);
  const [dialogMode, setDialogMode] = useState<'create' | 'edit'>('create');

  useEffect(() => {
    fetchResources();
  }, []);

  const fetchResources = async () => {
    try {
      setLoading(true);
      const response = await fetch('/api/v1/cloudpods/resources');
      const data = await response.json();
      setResources(data.resources || []);
    } catch (error) {
      console.error('Failed to fetch CloudPods resources:', error);
    } finally {
      setLoading(false);
    }
  };

  const handleCreateResource = () => {
    setSelectedResource(null);
    setDialogMode('create');
    setOpenDialog(true);
  };

  const handleEditResource = (resource: CloudPodsResource) => {
    setSelectedResource(resource);
    setDialogMode('edit');
    setOpenDialog(true);
  };

  const handleDeleteResource = async (resourceId: string) => {
    if (window.confirm('Are you sure you want to delete this resource?')) {
      try {
        await fetch(`/api/v1/cloudpods/resources/${resourceId}`, {
          method: 'DELETE',
        });
        fetchResources();
      } catch (error) {
        console.error('Failed to delete resource:', error);
      }
    }
  };

  const handleSaveResource = async (resourceData: Partial<CloudPodsResource>) => {
    try {
      const url = dialogMode === 'create' 
        ? '/api/v1/cloudpods/resources'
        : `/api/v1/cloudpods/resources/${selectedResource?.id}`;
      
      const method = dialogMode === 'create' ? 'POST' : 'PUT';
      
      await fetch(url, {
        method,
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify(resourceData),
      });
      
      setOpenDialog(false);
      fetchResources();
    } catch (error) {
      console.error('Failed to save resource:', error);
    }
  };

  const getStatusColor = (status: string) => {
    switch (status.toLowerCase()) {
      case 'running':
      case 'active':
        return 'success';
      case 'stopped':
      case 'inactive':
        return 'error';
      case 'pending':
      case 'starting':
        return 'warning';
      default:
        return 'default';
    }
  };

  const getResourceTypeIcon = (type: string) => {
    switch (type.toLowerCase()) {
      case 'instance':
        return '🖥️';
      case 'network':
        return '🌐';
      case 'storage':
        return '💾';
      case 'loadbalancer':
        return '⚖️';
      default:
        return '☁️';
    }
  };

  return (
    <Box>
      <Box sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', mb: 3 }}>
        <Typography variant="h4" component="h1">
          CloudPods Resources
        </Typography>
        <Box>
          <Button
            variant="outlined"
            startIcon={<RefreshIcon />}
            onClick={fetchResources}
            sx={{ mr: 1 }}
          >
            Refresh
          </Button>
          <Button
            variant="contained"
            startIcon={<AddIcon />}
            onClick={handleCreateResource}
          >
            Create Resource
          </Button>
        </Box>
      </Box>

      <Grid container spacing={3}>
        {/* Summary Cards */}
        <Grid item xs={12} md={3}>
          <Card>
            <CardContent>
              <Box sx={{ display: 'flex', alignItems: 'center', mb: 2 }}>
                <CloudIcon color="primary" sx={{ mr: 1 }} />
                <Typography variant="h6">Total Resources</Typography>
              </Box>
              <Typography variant="h4">{resources.length}</Typography>
            </CardContent>
          </Card>
        </Grid>

        <Grid item xs={12} md={3}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>Running</Typography>
              <Typography variant="h4" color="success.main">
                {resources.filter(r => r.status === 'running').length}
              </Typography>
            </CardContent>
          </Card>
        </Grid>

        <Grid item xs={12} md={3}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>Stopped</Typography>
              <Typography variant="h4" color="error.main">
                {resources.filter(r => r.status === 'stopped').length}
              </Typography>
            </CardContent>
          </Card>
        </Grid>

        <Grid item xs={12} md={3}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>Pending</Typography>
              <Typography variant="h4" color="warning.main">
                {resources.filter(r => r.status === 'pending').length}
              </Typography>
            </CardContent>
          </Card>
        </Grid>

        {/* Resources Table */}
        <Grid item xs={12}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>
                Resources
              </Typography>
              <TableContainer component={Paper} sx={{ backgroundColor: 'transparent' }}>
                <Table>
                  <TableHead>
                    <TableRow>
                      <TableCell>Name</TableCell>
                      <TableCell>Type</TableCell>
                      <TableCell>Status</TableCell>
                      <TableCell>Region</TableCell>
                      <TableCell>Created</TableCell>
                      <TableCell>Tags</TableCell>
                      <TableCell>Actions</TableCell>
                    </TableRow>
                  </TableHead>
                  <TableBody>
                    {resources.map((resource) => (
                      <TableRow key={resource.id}>
                        <TableCell>
                          <Box sx={{ display: 'flex', alignItems: 'center' }}>
                            <Typography variant="body2" sx={{ mr: 1 }}>
                              {getResourceTypeIcon(resource.type)}
                            </Typography>
                            {resource.name}
                          </Box>
                        </TableCell>
                        <TableCell>
                          <Chip label={resource.type} size="small" />
                        </TableCell>
                        <TableCell>
                          <Chip 
                            label={resource.status} 
                            color={getStatusColor(resource.status)}
                            size="small"
                          />
                        </TableCell>
                        <TableCell>{resource.region}</TableCell>
                        <TableCell>
                          {new Date(resource.created_at).toLocaleDateString()}
                        </TableCell>
                        <TableCell>
                          <Box sx={{ display: 'flex', gap: 0.5, flexWrap: 'wrap' }}>
                            {resource.tags.slice(0, 2).map((tag, index) => (
                              <Chip key={index} label={tag} size="small" variant="outlined" />
                            ))}
                            {resource.tags.length > 2 && (
                              <Chip label={`+${resource.tags.length - 2}`} size="small" variant="outlined" />
                            )}
                          </Box>
                        </TableCell>
                        <TableCell>
                          <IconButton
                            size="small"
                            onClick={() => handleEditResource(resource)}
                          >
                            <EditIcon />
                          </IconButton>
                          <IconButton
                            size="small"
                            onClick={() => handleDeleteResource(resource.id)}
                            color="error"
                          >
                            <DeleteIcon />
                          </IconButton>
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

      {/* Create/Edit Dialog */}
      <Dialog open={openDialog} onClose={() => setOpenDialog(false)} maxWidth="sm" fullWidth>
        <DialogTitle>
          {dialogMode === 'create' ? 'Create Resource' : 'Edit Resource'}
        </DialogTitle>
        <DialogContent>
          <Box sx={{ display: 'flex', flexDirection: 'column', gap: 2, mt: 1 }}>
            <TextField
              label="Name"
              defaultValue={selectedResource?.name || ''}
              fullWidth
            />
            <TextField
              label="Type"
              select
              defaultValue={selectedResource?.type || ''}
              fullWidth
            >
              <MenuItem value="instance">Instance</MenuItem>
              <MenuItem value="network">Network</MenuItem>
              <MenuItem value="storage">Storage</MenuItem>
              <MenuItem value="loadbalancer">Load Balancer</MenuItem>
            </TextField>
            <TextField
              label="Region"
              defaultValue={selectedResource?.region || ''}
              fullWidth
            />
            <TextField
              label="Tags (comma-separated)"
              defaultValue={selectedResource?.tags?.join(', ') || ''}
              fullWidth
            />
          </Box>
        </DialogContent>
        <DialogActions>
          <Button onClick={() => setOpenDialog(false)}>Cancel</Button>
          <Button 
            onClick={() => handleSaveResource({})} 
            variant="contained"
          >
            {dialogMode === 'create' ? 'Create' : 'Save'}
          </Button>
        </DialogActions>
      </Dialog>
    </Box>
  );
};

export default CloudPods;
