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
  Security as SecurityIcon,
  Cloud as CloudIcon,
} from '@mui/icons-material';

interface AviatrixGateway {
  id: string;
  name: string;
  type: string;
  status: string;
  region: string;
  vpc_id: string;
  public_ip: string;
  private_ip: string;
  created_at: string;
}

interface AviatrixConnection {
  id: string;
  name: string;
  source_gateway: string;
  dest_gateway: string;
  status: string;
  bandwidth: string;
  latency: string;
  created_at: string;
}

const Aviatrix: React.FC = () => {
  const [gateways, setGateways] = useState<AviatrixGateway[]>([]);
  const [connections, setConnections] = useState<AviatrixConnection[]>([]);
  const [loading, setLoading] = useState(true);
  const [activeTab, setActiveTab] = useState<'gateways' | 'connections'>('gateways');
  const [openDialog, setOpenDialog] = useState(false);
  const [dialogMode, setDialogMode] = useState<'create' | 'edit'>('create');
  const [selectedItem, setSelectedItem] = useState<any>(null);

  useEffect(() => {
    fetchData();
  }, []);

  const fetchData = async () => {
    try {
      setLoading(true);
      const [gatewaysRes, connectionsRes] = await Promise.all([
        fetch('/api/v1/aviatrix/gateways'),
        fetch('/api/v1/aviatrix/connections')
      ]);
      
      const gatewaysData = await gatewaysRes.json();
      const connectionsData = await connectionsRes.json();
      
      setGateways(gatewaysData.gateways || []);
      setConnections(connectionsData.connections || []);
    } catch (error) {
      console.error('Failed to fetch Aviatrix data:', error);
    } finally {
      setLoading(false);
    }
  };

  const handleCreate = () => {
    setSelectedItem(null);
    setDialogMode('create');
    setOpenDialog(true);
  };

  const handleEdit = (item: any) => {
    setSelectedItem(item);
    setDialogMode('edit');
    setOpenDialog(true);
  };

  const handleDelete = async (id: string) => {
    if (window.confirm('Are you sure you want to delete this item?')) {
      try {
        const endpoint = activeTab === 'gateways' ? 'gateways' : 'connections';
        await fetch(`/api/v1/aviatrix/${endpoint}/${id}`, {
          method: 'DELETE',
        });
        fetchData();
      } catch (error) {
        console.error('Failed to delete item:', error);
      }
    }
  };

  const getStatusColor = (status: string) => {
    switch (status.toLowerCase()) {
      case 'up':
      case 'running':
      case 'active':
        return 'success';
      case 'down':
      case 'stopped':
        return 'error';
      case 'pending':
      case 'starting':
        return 'warning';
      default:
        return 'default';
    }
  };

  const getGatewayTypeIcon = (type: string) => {
    switch (type.toLowerCase()) {
      case 'transit':
        return '🔄';
      case 'spoke':
        return '🔗';
      case 'egress':
        return '🚪';
      default:
        return '🏗️';
    }
  };

  return (
    <Box>
      <Box sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', mb: 3 }}>
        <Typography variant="h4" component="h1">
          Aviatrix Platform
        </Typography>
        <Box>
          <Button
            variant="outlined"
            startIcon={<RefreshIcon />}
            onClick={fetchData}
            sx={{ mr: 1 }}
          >
            Refresh
          </Button>
          <Button
            variant="contained"
            startIcon={<AddIcon />}
            onClick={handleCreate}
          >
            Create {activeTab === 'gateways' ? 'Gateway' : 'Connection'}
          </Button>
        </Box>
      </Box>

      {/* Tab Navigation */}
      <Box sx={{ borderBottom: 1, borderColor: 'divider', mb: 3 }}>
        <Button
          onClick={() => setActiveTab('gateways')}
          variant={activeTab === 'gateways' ? 'contained' : 'text'}
          sx={{ mr: 2 }}
        >
          Gateways ({gateways.length})
        </Button>
        <Button
          onClick={() => setActiveTab('connections')}
          variant={activeTab === 'connections' ? 'contained' : 'text'}
        >
          Connections ({connections.length})
        </Button>
      </Box>

      <Grid container spacing={3}>
        {/* Summary Cards */}
        <Grid item xs={12} md={3}>
          <Card>
            <CardContent>
              <Box sx={{ display: 'flex', alignItems: 'center', mb: 2 }}>
                <SecurityIcon color="primary" sx={{ mr: 1 }} />
                <Typography variant="h6">Total Gateways</Typography>
              </Box>
              <Typography variant="h4">{gateways.length}</Typography>
            </CardContent>
          </Card>
        </Grid>

        <Grid item xs={12} md={3}>
          <Card>
            <CardContent>
              <Box sx={{ display: 'flex', alignItems: 'center', mb: 2 }}>
                <CloudIcon color="primary" sx={{ mr: 1 }} />
                <Typography variant="h6">Total Connections</Typography>
              </Box>
              <Typography variant="h4">{connections.length}</Typography>
            </CardContent>
          </Card>
        </Grid>

        <Grid item xs={12} md={3}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>Active Gateways</Typography>
              <Typography variant="h4" color="success.main">
                {gateways.filter(g => g.status === 'up').length}
              </Typography>
            </CardContent>
          </Card>
        </Grid>

        <Grid item xs={12} md={3}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>Active Connections</Typography>
              <Typography variant="h4" color="success.main">
                {connections.filter(c => c.status === 'up').length}
              </Typography>
            </CardContent>
          </Card>
        </Grid>

        {/* Gateways Table */}
        {activeTab === 'gateways' && (
          <Grid item xs={12}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  Gateways
                </Typography>
                <TableContainer component={Paper} sx={{ backgroundColor: 'transparent' }}>
                  <Table>
                    <TableHead>
                      <TableRow>
                        <TableCell>Name</TableCell>
                        <TableCell>Type</TableCell>
                        <TableCell>Status</TableCell>
                        <TableCell>Region</TableCell>
                        <TableCell>Public IP</TableCell>
                        <TableCell>Private IP</TableCell>
                        <TableCell>Created</TableCell>
                        <TableCell>Actions</TableCell>
                      </TableRow>
                    </TableHead>
                    <TableBody>
                      {gateways.map((gateway) => (
                        <TableRow key={gateway.id}>
                          <TableCell>
                            <Box sx={{ display: 'flex', alignItems: 'center' }}>
                              <Typography variant="body2" sx={{ mr: 1 }}>
                                {getGatewayTypeIcon(gateway.type)}
                              </Typography>
                              {gateway.name}
                            </Box>
                          </TableCell>
                          <TableCell>
                            <Chip label={gateway.type} size="small" />
                          </TableCell>
                          <TableCell>
                            <Chip 
                              label={gateway.status} 
                              color={getStatusColor(gateway.status)}
                              size="small"
                            />
                          </TableCell>
                          <TableCell>{gateway.region}</TableCell>
                          <TableCell>
                            <Typography variant="body2" fontFamily="monospace">
                              {gateway.public_ip}
                            </Typography>
                          </TableCell>
                          <TableCell>
                            <Typography variant="body2" fontFamily="monospace">
                              {gateway.private_ip}
                            </Typography>
                          </TableCell>
                          <TableCell>
                            {new Date(gateway.created_at).toLocaleDateString()}
                          </TableCell>
                          <TableCell>
                            <IconButton
                              size="small"
                              onClick={() => handleEdit(gateway)}
                            >
                              <EditIcon />
                            </IconButton>
                            <IconButton
                              size="small"
                              onClick={() => handleDelete(gateway.id)}
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
        )}

        {/* Connections Table */}
        {activeTab === 'connections' && (
          <Grid item xs={12}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  Connections
                </Typography>
                <TableContainer component={Paper} sx={{ backgroundColor: 'transparent' }}>
                  <Table>
                    <TableHead>
                      <TableRow>
                        <TableCell>Name</TableCell>
                        <TableCell>Source Gateway</TableCell>
                        <TableCell>Destination Gateway</TableCell>
                        <TableCell>Status</TableCell>
                        <TableCell>Bandwidth</TableCell>
                        <TableCell>Latency</TableCell>
                        <TableCell>Created</TableCell>
                        <TableCell>Actions</TableCell>
                      </TableRow>
                    </TableHead>
                    <TableBody>
                      {connections.map((connection) => (
                        <TableRow key={connection.id}>
                          <TableCell>{connection.name}</TableCell>
                          <TableCell>{connection.source_gateway}</TableCell>
                          <TableCell>{connection.dest_gateway}</TableCell>
                          <TableCell>
                            <Chip 
                              label={connection.status} 
                              color={getStatusColor(connection.status)}
                              size="small"
                            />
                          </TableCell>
                          <TableCell>{connection.bandwidth}</TableCell>
                          <TableCell>{connection.latency}</TableCell>
                          <TableCell>
                            {new Date(connection.created_at).toLocaleDateString()}
                          </TableCell>
                          <TableCell>
                            <IconButton
                              size="small"
                              onClick={() => handleEdit(connection)}
                            >
                              <EditIcon />
                            </IconButton>
                            <IconButton
                              size="small"
                              onClick={() => handleDelete(connection.id)}
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
        )}
      </Grid>

      {/* Create/Edit Dialog */}
      <Dialog open={openDialog} onClose={() => setOpenDialog(false)} maxWidth="sm" fullWidth>
        <DialogTitle>
          {dialogMode === 'create' ? `Create ${activeTab === 'gateways' ? 'Gateway' : 'Connection'}` : `Edit ${activeTab === 'gateways' ? 'Gateway' : 'Connection'}`}
        </DialogTitle>
        <DialogContent>
          <Box sx={{ display: 'flex', flexDirection: 'column', gap: 2, mt: 1 }}>
            <TextField
              label="Name"
              defaultValue={selectedItem?.name || ''}
              fullWidth
            />
            {activeTab === 'gateways' ? (
              <>
                <TextField
                  label="Type"
                  select
                  defaultValue={selectedItem?.type || ''}
                  fullWidth
                >
                  <MenuItem value="transit">Transit</MenuItem>
                  <MenuItem value="spoke">Spoke</MenuItem>
                  <MenuItem value="egress">Egress</MenuItem>
                </TextField>
                <TextField
                  label="Region"
                  defaultValue={selectedItem?.region || ''}
                  fullWidth
                />
                <TextField
                  label="VPC ID"
                  defaultValue={selectedItem?.vpc_id || ''}
                  fullWidth
                />
              </>
            ) : (
              <>
                <TextField
                  label="Source Gateway"
                  defaultValue={selectedItem?.source_gateway || ''}
                  fullWidth
                />
                <TextField
                  label="Destination Gateway"
                  defaultValue={selectedItem?.dest_gateway || ''}
                  fullWidth
                />
                <TextField
                  label="Bandwidth"
                  defaultValue={selectedItem?.bandwidth || ''}
                  fullWidth
                />
              </>
            )}
          </Box>
        </DialogContent>
        <DialogActions>
          <Button onClick={() => setOpenDialog(false)}>Cancel</Button>
          <Button 
            onClick={() => setOpenDialog(false)} 
            variant="contained"
          >
            {dialogMode === 'create' ? 'Create' : 'Save'}
          </Button>
        </DialogActions>
      </Dialog>
    </Box>
  );
};

export default Aviatrix;
