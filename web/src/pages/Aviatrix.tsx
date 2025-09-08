import React, { useState } from 'react';
import {
  Box,
  Typography,
  Card,
  CardContent,
  Grid,
  Button,
  Table,
  TableBody,
  TableCell,
  TableContainer,
  TableHead,
  TableRow,
  Paper,
  Chip,
  IconButton,
  Dialog,
  DialogTitle,
  DialogContent,
  DialogActions,
  TextField,
  FormControl,
  InputLabel,
  Select,
  MenuItem,
} from '@mui/material';
import {
  Add as AddIcon,
  Edit as EditIcon,
  Delete as DeleteIcon,
  Refresh as RefreshIcon,
} from '@mui/icons-material';
import { useQuery, useMutation, useQueryClient } from '@tanstack/react-query';

const Aviatrix: React.FC = () => {
  const [open, setOpen] = useState(false);
  const [editingItem, setEditingItem] = useState<any>(null);
  const [activeTab, setActiveTab] = useState('transit-gateways');
  const queryClient = useQueryClient();

  const { data: transitGateways, isLoading: transitGatewaysLoading } = useQuery({
    queryKey: ['aviatrix-transit-gateways'],
    queryFn: async () => {
      const response = await fetch('/api/v1/aviatrix/transit-gateways');
      return response.json();
    },
  });

  const { data: spokeGateways, isLoading: spokeGatewaysLoading } = useQuery({
    queryKey: ['aviatrix-spoke-gateways'],
    queryFn: async () => {
      const response = await fetch('/api/v1/aviatrix/spoke-gateways');
      return response.json();
    },
  });

  const { data: vpcConnections, isLoading: vpcConnectionsLoading } = useQuery({
    queryKey: ['aviatrix-vpc-connections'],
    queryFn: async () => {
      const response = await fetch('/api/v1/aviatrix/vpc-connections');
      return response.json();
    },
  });

  const { data: site2CloudConnections, isLoading: site2CloudConnectionsLoading } = useQuery({
    queryKey: ['aviatrix-site2cloud'],
    queryFn: async () => {
      const response = await fetch('/api/v1/aviatrix/site2cloud');
      return response.json();
    },
  });

  const createTransitGateway = useMutation({
    mutationFn: async (data: any) => {
      const response = await fetch('/api/v1/aviatrix/transit-gateways', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(data),
      });
      return response.json();
    },
    onSuccess: () => {
      queryClient.invalidateQueries({ queryKey: ['aviatrix-transit-gateways'] });
      setOpen(false);
    },
  });

  const deleteTransitGateway = useMutation({
    mutationFn: async (name: string) => {
      const response = await fetch(`/api/v1/aviatrix/transit-gateways/${name}`, {
        method: 'DELETE',
      });
      return response.json();
    },
    onSuccess: () => {
      queryClient.invalidateQueries({ queryKey: ['aviatrix-transit-gateways'] });
    },
  });

  const handleCreate = () => {
    setEditingItem(null);
    setOpen(true);
  };

  const handleEdit = (item: any) => {
    setEditingItem(item);
    setOpen(true);
  };

  const handleDelete = (name: string) => {
    if (window.confirm('Are you sure you want to delete this item?')) {
      deleteTransitGateway.mutate(name);
    }
  };

  const getStatusColor = (status: string) => {
    switch (status?.toLowerCase()) {
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

  const renderTransitGateways = () => (
    <Card>
      <CardContent>
        <Box sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', mb: 2 }}>
          <Typography variant="h6">Transit Gateways</Typography>
          <Box>
            <IconButton onClick={() => queryClient.invalidateQueries({ queryKey: ['aviatrix-transit-gateways'] })}>
              <RefreshIcon />
            </IconButton>
            <Button variant="contained" startIcon={<AddIcon />} onClick={handleCreate}>
              Create
            </Button>
          </Box>
        </Box>
        <TableContainer component={Paper}>
          <Table>
            <TableHead>
              <TableRow>
                <TableCell>Name</TableCell>
                <TableCell>Cloud Type</TableCell>
                <TableCell>VPC ID</TableCell>
                <TableCell>Region</TableCell>
                <TableCell>Status</TableCell>
                <TableCell>Actions</TableCell>
              </TableRow>
            </TableHead>
            <TableBody>
              {transitGateways?.data?.map((gateway: any) => (
                <TableRow key={gateway.gw_name}>
                  <TableCell>{gateway.gw_name}</TableCell>
                  <TableCell>{gateway.cloud_type}</TableCell>
                  <TableCell>{gateway.vpc_id}</TableCell>
                  <TableCell>{gateway.vpc_reg}</TableCell>
                  <TableCell>
                    <Chip
                      label={gateway.status}
                      color={getStatusColor(gateway.status)}
                      size="small"
                    />
                  </TableCell>
                  <TableCell>
                    <IconButton onClick={() => handleEdit(gateway)}>
                      <EditIcon />
                    </IconButton>
                    <IconButton onClick={() => handleDelete(gateway.gw_name)}>
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
  );

  const renderSpokeGateways = () => (
    <Card>
      <CardContent>
        <Box sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', mb: 2 }}>
          <Typography variant="h6">Spoke Gateways</Typography>
          <Box>
            <IconButton onClick={() => queryClient.invalidateQueries({ queryKey: ['aviatrix-spoke-gateways'] })}>
              <RefreshIcon />
            </IconButton>
            <Button variant="contained" startIcon={<AddIcon />} onClick={handleCreate}>
              Create
            </Button>
          </Box>
        </Box>
        <TableContainer component={Paper}>
          <Table>
            <TableHead>
              <TableRow>
                <TableCell>Name</TableCell>
                <TableCell>Cloud Type</TableCell>
                <TableCell>VPC ID</TableCell>
                <TableCell>Region</TableCell>
                <TableCell>Status</TableCell>
                <TableCell>Actions</TableCell>
              </TableRow>
            </TableHead>
            <TableBody>
              {spokeGateways?.data?.map((gateway: any) => (
                <TableRow key={gateway.gw_name}>
                  <TableCell>{gateway.gw_name}</TableCell>
                  <TableCell>{gateway.cloud_type}</TableCell>
                  <TableCell>{gateway.vpc_id}</TableCell>
                  <TableCell>{gateway.vpc_reg}</TableCell>
                  <TableCell>
                    <Chip
                      label={gateway.status}
                      color={getStatusColor(gateway.status)}
                      size="small"
                    />
                  </TableCell>
                  <TableCell>
                    <IconButton onClick={() => handleEdit(gateway)}>
                      <EditIcon />
                    </IconButton>
                    <IconButton onClick={() => handleDelete(gateway.gw_name)}>
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
  );

  const renderVPCConnections = () => (
    <Card>
      <CardContent>
        <Box sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', mb: 2 }}>
          <Typography variant="h6">VPC Connections</Typography>
          <Box>
            <IconButton onClick={() => queryClient.invalidateQueries({ queryKey: ['aviatrix-vpc-connections'] })}>
              <RefreshIcon />
            </IconButton>
            <Button variant="contained" startIcon={<AddIcon />} onClick={handleCreate}>
              Create
            </Button>
          </Box>
        </Box>
        <TableContainer component={Paper}>
          <Table>
            <TableHead>
              <TableRow>
                <TableCell>Connection Name</TableCell>
                <TableCell>VPC ID</TableCell>
                <TableCell>Transit Gateway</TableCell>
                <TableCell>Spoke Gateway</TableCell>
                <TableCell>Status</TableCell>
                <TableCell>Actions</TableCell>
              </TableRow>
            </TableHead>
            <TableBody>
              {vpcConnections?.data?.map((connection: any) => (
                <TableRow key={connection.connection_name}>
                  <TableCell>{connection.connection_name}</TableCell>
                  <TableCell>{connection.vpc_id}</TableCell>
                  <TableCell>{connection.transit_gateway}</TableCell>
                  <TableCell>{connection.spoke_gateway}</TableCell>
                  <TableCell>
                    <Chip
                      label={connection.status}
                      color={getStatusColor(connection.status)}
                      size="small"
                    />
                  </TableCell>
                  <TableCell>
                    <IconButton onClick={() => handleEdit(connection)}>
                      <EditIcon />
                    </IconButton>
                    <IconButton onClick={() => handleDelete(connection.connection_name)}>
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
  );

  const renderSite2CloudConnections = () => (
    <Card>
      <CardContent>
        <Box sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', mb: 2 }}>
          <Typography variant="h6">Site-to-Cloud Connections</Typography>
          <Box>
            <IconButton onClick={() => queryClient.invalidateQueries({ queryKey: ['aviatrix-site2cloud'] })}>
              <RefreshIcon />
            </IconButton>
            <Button variant="contained" startIcon={<AddIcon />} onClick={handleCreate}>
              Create
            </Button>
          </Box>
        </Box>
        <TableContainer component={Paper}>
          <Table>
            <TableHead>
              <TableRow>
                <TableCell>Connection Name</TableCell>
                <TableCell>VPC ID</TableCell>
                <TableCell>Remote Gateway IP</TableCell>
                <TableCell>Connection Type</TableCell>
                <TableCell>Status</TableCell>
                <TableCell>Actions</TableCell>
              </TableRow>
            </TableHead>
            <TableBody>
              {site2CloudConnections?.data?.map((connection: any) => (
                <TableRow key={connection.connection_name}>
                  <TableCell>{connection.connection_name}</TableCell>
                  <TableCell>{connection.vpc_id}</TableCell>
                  <TableCell>{connection.remote_gateway_ip}</TableCell>
                  <TableCell>{connection.connection_type}</TableCell>
                  <TableCell>
                    <Chip
                      label={connection.status}
                      color={getStatusColor(connection.status)}
                      size="small"
                    />
                  </TableCell>
                  <TableCell>
                    <IconButton onClick={() => handleEdit(connection)}>
                      <EditIcon />
                    </IconButton>
                    <IconButton onClick={() => handleDelete(connection.connection_name)}>
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
  );

  return (
    <Box>
      <Typography variant="h4" component="h1" gutterBottom>
        Aviatrix Integration
      </Typography>

      <Grid container spacing={3}>
        <Grid item xs={12}>
          <Card>
            <CardContent>
              <Box sx={{ borderBottom: 1, borderColor: 'divider', mb: 2 }}>
                <Button
                  variant={activeTab === 'transit-gateways' ? 'contained' : 'text'}
                  onClick={() => setActiveTab('transit-gateways')}
                  sx={{ mr: 1 }}
                >
                  Transit Gateways
                </Button>
                <Button
                  variant={activeTab === 'spoke-gateways' ? 'contained' : 'text'}
                  onClick={() => setActiveTab('spoke-gateways')}
                  sx={{ mr: 1 }}
                >
                  Spoke Gateways
                </Button>
                <Button
                  variant={activeTab === 'vpc-connections' ? 'contained' : 'text'}
                  onClick={() => setActiveTab('vpc-connections')}
                  sx={{ mr: 1 }}
                >
                  VPC Connections
                </Button>
                <Button
                  variant={activeTab === 'site2cloud' ? 'contained' : 'text'}
                  onClick={() => setActiveTab('site2cloud')}
                >
                  Site-to-Cloud
                </Button>
              </Box>

              {activeTab === 'transit-gateways' && renderTransitGateways()}
              {activeTab === 'spoke-gateways' && renderSpokeGateways()}
              {activeTab === 'vpc-connections' && renderVPCConnections()}
              {activeTab === 'site2cloud' && renderSite2CloudConnections()}
            </CardContent>
          </Card>
        </Grid>
      </Grid>

      {/* Create/Edit Dialog */}
      <Dialog open={open} onClose={() => setOpen(false)} maxWidth="md" fullWidth>
        <DialogTitle>
          {editingItem ? 'Edit' : 'Create'} {activeTab.replace('-', ' ').replace(/\b\w/g, l => l.toUpperCase())}
        </DialogTitle>
        <DialogContent>
          <Grid container spacing={2} sx={{ mt: 1 }}>
            <Grid item xs={12} sm={6}>
              <TextField
                fullWidth
                label="Name"
                defaultValue={editingItem?.gw_name || editingItem?.connection_name || ''}
                required
              />
            </Grid>
            <Grid item xs={12} sm={6}>
              <FormControl fullWidth>
                <InputLabel>Cloud Type</InputLabel>
                <Select defaultValue={editingItem?.cloud_type || 1}>
                  <MenuItem value={1}>AWS</MenuItem>
                  <MenuItem value={4}>Azure</MenuItem>
                  <MenuItem value={8}>GCP</MenuItem>
                </Select>
              </FormControl>
            </Grid>
            <Grid item xs={12} sm={6}>
              <TextField
                fullWidth
                label="VPC ID"
                defaultValue={editingItem?.vpc_id || ''}
                required
              />
            </Grid>
            <Grid item xs={12} sm={6}>
              <TextField
                fullWidth
                label="Region"
                defaultValue={editingItem?.vpc_reg || editingItem?.region || ''}
                required
              />
            </Grid>
            <Grid item xs={12} sm={6}>
              <TextField
                fullWidth
                label="Subnet"
                defaultValue={editingItem?.subnet || ''}
              />
            </Grid>
            <Grid item xs={12} sm={6}>
              <FormControl fullWidth>
                <InputLabel>Gateway Size</InputLabel>
                <Select defaultValue={editingItem?.gw_size || 't2.micro'}>
                  <MenuItem value="t2.micro">t2.micro</MenuItem>
                  <MenuItem value="t2.small">t2.small</MenuItem>
                  <MenuItem value="t2.medium">t2.medium</MenuItem>
                  <MenuItem value="t2.large">t2.large</MenuItem>
                </Select>
              </FormControl>
            </Grid>
          </Grid>
        </DialogContent>
        <DialogActions>
          <Button onClick={() => setOpen(false)}>Cancel</Button>
          <Button
            variant="contained"
            onClick={() => {
              // Handle create/edit logic here
              setOpen(false);
            }}
          >
            {editingItem ? 'Update' : 'Create'}
          </Button>
        </DialogActions>
      </Dialog>
    </Box>
  );
};

export default Aviatrix;
