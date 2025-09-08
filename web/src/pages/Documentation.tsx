import React, { useState } from 'react';
import {
  Box,
  Typography,
  Card,
  CardContent,
  Grid,
  Button,
  List,
  ListItem,
  ListItemText,
  ListItemSecondaryAction,
  IconButton,
  Chip,
  Dialog,
  DialogTitle,
  DialogContent,
  DialogActions,
  TextField,
  FormControl,
  InputLabel,
  Select,
  MenuItem,
  Switch,
  FormControlLabel,
  Slider,
  Alert,
  Snackbar,
  Paper,
  Divider,
  LinearProgress,
  Tabs,
  Tab,
  Accordion,
  AccordionSummary,
  AccordionDetails,
  Link,
} from '@mui/material';
import {
  Add as AddIcon,
  Edit as EditIcon,
  Delete as DeleteIcon,
  Refresh as RefreshIcon,
  Download as DownloadIcon,
  Upload as UploadIcon,
  Visibility as ViewIcon,
  Code as CodeIcon,
  Description as DescriptionIcon,
  Book as BookIcon,
  Article as ArticleIcon,
  ExpandMore as ExpandMoreIcon,
} from '@mui/icons-material';
import { useQuery, useMutation, useQueryClient } from '@tanstack/react-query';

interface DocumentationItem {
  id: string;
  title: string;
  content: string;
  type: 'api' | 'user_guide' | 'developer_guide' | 'tutorial' | 'reference' | 'changelog';
  category: string;
  tags: string[];
  status: 'draft' | 'published' | 'archived';
  author: string;
  created_at: string;
  updated_at: string;
  version: string;
}

interface DocumentationCategory {
  id: string;
  name: string;
  description: string;
  items: string[];
  created_at: string;
  updated_at: string;
}

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
      id={`documentation-tabpanel-${index}`}
      aria-labelledby={`documentation-tab-${index}`}
      {...other}
    >
      {value === index && <Box sx={{ p: 3 }}>{children}</Box>}
    </div>
  );
}

const Documentation: React.FC = () => {
  const [selectedItem, setSelectedItem] = useState<DocumentationItem | null>(null);
  const [itemDialogOpen, setItemDialogOpen] = useState(false);
  const [editingItem, setEditingItem] = useState<DocumentationItem | null>(null);
  const [snackbarOpen, setSnackbarOpen] = useState(false);
  const [snackbarMessage, setSnackbarMessage] = useState('');
  const [snackbarSeverity, setSnackbarSeverity] = useState<'success' | 'error'>('success');
  const [tabValue, setTabValue] = useState(0);

  const queryClient = useQueryClient();

  const { data: items, isLoading } = useQuery({
    queryKey: ['documentation-items'],
    queryFn: async () => {
      const response = await fetch('/api/v1/documentation/items');
      return response.json();
    },
  });

  const { data: categories } = useQuery({
    queryKey: ['documentation-categories'],
    queryFn: async () => {
      const response = await fetch('/api/v1/documentation/categories');
      return response.json();
    },
  });

  const { data: stats } = useQuery({
    queryKey: ['documentation-stats'],
    queryFn: async () => {
      const response = await fetch('/api/v1/documentation/stats');
      return response.json();
    },
  });

  const createItemMutation = useMutation({
    mutationFn: async (itemData: Partial<DocumentationItem>) => {
      const response = await fetch('/api/v1/documentation/items', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify(itemData),
      });
      return response.json();
    },
    onSuccess: () => {
      queryClient.invalidateQueries({ queryKey: ['documentation-items'] });
      setSnackbarMessage('Documentation item created successfully');
      setSnackbarOpen(true);
      setItemDialogOpen(false);
    },
    onError: () => {
      setSnackbarMessage('Failed to create documentation item');
      setSnackbarSeverity('error');
      setSnackbarOpen(true);
    },
  });

  const updateItemMutation = useMutation({
    mutationFn: async ({ id, ...itemData }: Partial<DocumentationItem> & { id: string }) => {
      const response = await fetch(`/api/v1/documentation/items/${id}`, {
        method: 'PUT',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify(itemData),
      });
      return response.json();
    },
    onSuccess: () => {
      queryClient.invalidateQueries({ queryKey: ['documentation-items'] });
      setSnackbarMessage('Documentation item updated successfully');
      setSnackbarOpen(true);
      setItemDialogOpen(false);
    },
    onError: () => {
      setSnackbarMessage('Failed to update documentation item');
      setSnackbarSeverity('error');
      setSnackbarOpen(true);
    },
  });

  const deleteItemMutation = useMutation({
    mutationFn: async (itemId: string) => {
      const response = await fetch(`/api/v1/documentation/items/${itemId}`, {
        method: 'DELETE',
      });
      return response.json();
    },
    onSuccess: () => {
      queryClient.invalidateQueries({ queryKey: ['documentation-items'] });
      setSnackbarMessage('Documentation item deleted successfully');
      setSnackbarOpen(true);
    },
    onError: () => {
      setSnackbarMessage('Failed to delete documentation item');
      setSnackbarSeverity('error');
      setSnackbarOpen(true);
    },
  });

  const handleCreateItem = () => {
    setEditingItem(null);
    setItemDialogOpen(true);
  };

  const handleEditItem = (item: DocumentationItem) => {
    setEditingItem(item);
    setItemDialogOpen(true);
  };

  const handleDeleteItem = (itemId: string) => {
    if (window.confirm('Are you sure you want to delete this documentation item?')) {
      deleteItemMutation.mutate(itemId);
    }
  };

  const handleSaveItem = (itemData: Partial<DocumentationItem>) => {
    if (editingItem) {
      updateItemMutation.mutate({ ...itemData, id: editingItem.id });
    } else {
      createItemMutation.mutate(itemData);
    }
  };

  const handleTabChange = (event: React.SyntheticEvent, newValue: number) => {
    setTabValue(newValue);
  };

  const getTypeIcon = (type: string) => {
    switch (type) {
      case 'api':
        return <CodeIcon />;
      case 'user_guide':
        return <BookIcon />;
      case 'developer_guide':
        return <CodeIcon />;
      case 'tutorial':
        return <ArticleIcon />;
      case 'reference':
        return <DescriptionIcon />;
      case 'changelog':
        return <DescriptionIcon />;
      default:
        return <DescriptionIcon />;
    }
  };

  const getTypeLabel = (type: string) => {
    switch (type) {
      case 'api':
        return 'API Documentation';
      case 'user_guide':
        return 'User Guide';
      case 'developer_guide':
        return 'Developer Guide';
      case 'tutorial':
        return 'Tutorial';
      case 'reference':
        return 'Reference';
      case 'changelog':
        return 'Changelog';
      default:
        return type;
    }
  };

  const getStatusColor = (status: string) => {
    switch (status) {
      case 'published':
        return 'success';
      case 'draft':
        return 'warning';
      case 'archived':
        return 'default';
      default:
        return 'default';
    }
  };

  if (isLoading) {
    return (
      <Box display="flex" justifyContent="center" alignItems="center" minHeight="400px">
        <Typography>Loading documentation...</Typography>
      </Box>
    );
  }

  return (
    <Box>
      <Box display="flex" justifyContent="space-between" alignItems="center" mb={3}>
        <Typography variant="h4" component="h1">
          Documentation
        </Typography>
        <Box display="flex" gap={2}>
          <Button
            variant="outlined"
            startIcon={<RefreshIcon />}
            onClick={() => queryClient.invalidateQueries({ queryKey: ['documentation-items'] })}
          >
            Refresh
          </Button>
          <Button
            variant="contained"
            startIcon={<AddIcon />}
            onClick={handleCreateItem}
          >
            Add Item
          </Button>
        </Box>
      </Box>

      <Box sx={{ borderBottom: 1, borderColor: 'divider' }}>
        <Tabs value={tabValue} onChange={handleTabChange} aria-label="documentation tabs">
          <Tab label="All Items" />
          <Tab label="Categories" />
          <Tab label="API Reference" />
          <Tab label="User Guides" />
          <Tab label="Developer Guides" />
        </Tabs>
      </Box>

      {/* All Items Tab */}
      <TabPanel value={tabValue} index={0}>
        <Grid container spacing={3}>
          {/* Documentation Statistics */}
          <Grid item xs={12} md={6}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  Documentation Statistics
                </Typography>
                {stats?.data && (
                  <Grid container spacing={2}>
                    <Grid item xs={6}>
                      <Typography variant="body2" color="text.secondary">
                        Total Items
                      </Typography>
                      <Typography variant="h6">
                        {stats.data.total_items}
                      </Typography>
                    </Grid>
                    <Grid item xs={6}>
                      <Typography variant="body2" color="text.secondary">
                        Published
                      </Typography>
                      <Typography variant="h6" color="success.main">
                        {stats.data.published_items}
                      </Typography>
                    </Grid>
                    <Grid item xs={6}>
                      <Typography variant="body2" color="text.secondary">
                        Drafts
                      </Typography>
                      <Typography variant="h6" color="warning.main">
                        {stats.data.draft_items}
                      </Typography>
                    </Grid>
                    <Grid item xs={6}>
                      <Typography variant="body2" color="text.secondary">
                        Categories
                      </Typography>
                      <Typography variant="h6">
                        {stats.data.total_categories}
                      </Typography>
                    </Grid>
                  </Grid>
                )}
              </CardContent>
            </Card>
          </Grid>

          {/* Recent Updates */}
          <Grid item xs={12} md={6}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  Recent Updates
                </Typography>
                {items?.data && (
                  <List dense>
                    {items.data.slice(0, 5).map((item: DocumentationItem) => (
                      <ListItem key={item.id}>
                        <ListItemText
                          primary={item.title}
                          secondary={`Updated: ${new Date(item.updated_at).toLocaleDateString()}`}
                        />
                        <Chip
                          label={item.status}
                          color={getStatusColor(item.status) as any}
                          size="small"
                        />
                      </ListItem>
                    ))}
                  </List>
                )}
              </CardContent>
            </Card>
          </Grid>

          {/* Documentation Items */}
          <Grid item xs={12}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  Documentation Items
                </Typography>
                {items?.data && items.data.length > 0 ? (
                  <List>
                    {items.data.map((item: DocumentationItem) => (
                      <ListItem key={item.id} divider>
                        <ListItemText
                          primary={item.title}
                          secondary={
                            <Box>
                              <Typography variant="body2" color="text.secondary">
                                {item.content.substring(0, 100)}...
                              </Typography>
                              <Box display="flex" gap={1} mt={1}>
                                <Chip
                                  icon={getTypeIcon(item.type)}
                                  label={getTypeLabel(item.type)}
                                  size="small"
                                  variant="outlined"
                                />
                                <Chip
                                  label={item.status}
                                  color={getStatusColor(item.status) as any}
                                  size="small"
                                />
                                <Chip
                                  label={`v${item.version}`}
                                  size="small"
                                  variant="outlined"
                                />
                                <Chip
                                  label={item.category}
                                  size="small"
                                  variant="outlined"
                                />
                              </Box>
                            </Box>
                          }
                        />
                        <ListItemSecondaryAction>
                          <Box display="flex" gap={1}>
                            <IconButton
                              onClick={() => setSelectedItem(item)}
                              color="primary"
                            >
                              <ViewIcon />
                            </IconButton>
                            <IconButton
                              onClick={() => handleEditItem(item)}
                              color="primary"
                            >
                              <EditIcon />
                            </IconButton>
                            <IconButton
                              onClick={() => handleDeleteItem(item.id)}
                              color="error"
                            >
                              <DeleteIcon />
                            </IconButton>
                          </Box>
                        </ListItemSecondaryAction>
                      </ListItem>
                    ))}
                  </List>
                ) : (
                  <Box textAlign="center" py={4}>
                    <Typography variant="body1" color="text.secondary">
                      No documentation items available. Create your first item to get started.
                    </Typography>
                  </Box>
                )}
              </CardContent>
            </Card>
          </Grid>
        </Grid>
      </TabPanel>

      {/* Categories Tab */}
      <TabPanel value={tabValue} index={1}>
        <Grid container spacing={3}>
          <Grid item xs={12}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  Documentation Categories
                </Typography>
                {categories?.data && categories.data.length > 0 ? (
                  <Grid container spacing={2}>
                    {categories.data.map((category: DocumentationCategory) => (
                      <Grid item xs={12} md={6} key={category.id}>
                        <Accordion>
                          <AccordionSummary expandIcon={<ExpandMoreIcon />}>
                            <Typography variant="h6">{category.name}</Typography>
                          </AccordionSummary>
                          <AccordionDetails>
                            <Typography variant="body2" color="text.secondary" paragraph>
                              {category.description}
                            </Typography>
                            <Typography variant="body2">
                              {category.items.length} items
                            </Typography>
                          </AccordionDetails>
                        </Accordion>
                      </Grid>
                    ))}
                  </Grid>
                ) : (
                  <Box textAlign="center" py={4}>
                    <Typography variant="body1" color="text.secondary">
                      No categories available.
                    </Typography>
                  </Box>
                )}
              </CardContent>
            </Card>
          </Grid>
        </Grid>
      </TabPanel>

      {/* API Reference Tab */}
      <TabPanel value={tabValue} index={2}>
        <Grid container spacing={3}>
          <Grid item xs={12}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  API Reference
                </Typography>
                <Typography variant="body2" color="text.secondary" paragraph>
                  Complete API documentation for the Multi-Protocol Router Simulator.
                </Typography>
                <Box>
                  <Accordion>
                    <AccordionSummary expandIcon={<ExpandMoreIcon />}>
                      <Typography variant="h6">REST API</Typography>
                    </AccordionSummary>
                    <AccordionDetails>
                      <Typography variant="body2" paragraph>
                        The REST API provides programmatic access to all router simulator features.
                      </Typography>
                      <Typography variant="body2" paragraph>
                        Base URL: <code>http://localhost:8080/api/v1</code>
                      </Typography>
                      <Typography variant="body2" paragraph>
                        Authentication: Bearer token or API key
                      </Typography>
                    </AccordionDetails>
                  </Accordion>
                  <Accordion>
                    <AccordionSummary expandIcon={<ExpandMoreIcon />}>
                      <Typography variant="h6">C++ API</Typography>
                    </AccordionSummary>
                    <AccordionDetails>
                      <Typography variant="body2" paragraph>
                        C++ API for direct integration with the router core.
                      </Typography>
                      <Typography variant="body2" paragraph>
                        Include: <code>#include "router_core.h"</code>
                      </Typography>
                    </AccordionDetails>
                  </Accordion>
                  <Accordion>
                    <AccordionSummary expandIcon={<ExpandMoreIcon />}>
                      <Typography variant="h6">Rust API</Typography>
                    </AccordionSummary>
                    <AccordionDetails>
                      <Typography variant="body2" paragraph>
                        Rust API for high-performance analytics and packet processing.
                      </Typography>
                      <Typography variant="body2" paragraph>
                        Add to Cargo.toml: <code>router-sim = "0.1.0"</code>
                      </Typography>
                    </AccordionDetails>
                  </Accordion>
                </Box>
              </CardContent>
            </Card>
          </Grid>
        </Grid>
      </TabPanel>

      {/* User Guides Tab */}
      <TabPanel value={tabValue} index={3}>
        <Grid container spacing={3}>
          <Grid item xs={12}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  User Guides
                </Typography>
                <Typography variant="body2" color="text.secondary" paragraph>
                  Step-by-step guides for using the Multi-Protocol Router Simulator.
                </Typography>
                <Box>
                  <Accordion>
                    <AccordionSummary expandIcon={<ExpandMoreIcon />}>
                      <Typography variant="h6">Getting Started</Typography>
                    </AccordionSummary>
                    <AccordionDetails>
                      <Typography variant="body2" paragraph>
                        Learn how to set up and run your first router simulation.
                      </Typography>
                      <Button variant="outlined" startIcon={<ViewIcon />}>
                        View Guide
                      </Button>
                    </AccordionDetails>
                  </Accordion>
                  <Accordion>
                    <AccordionSummary expandIcon={<ExpandMoreIcon />}>
                      <Typography variant="h6">Configuration</Typography>
                    </AccordionSummary>
                    <AccordionDetails>
                      <Typography variant="body2" paragraph>
                        Configure routing protocols, traffic shaping, and network impairments.
                      </Typography>
                      <Button variant="outlined" startIcon={<ViewIcon />}>
                        View Guide
                      </Button>
                    </AccordionDetails>
                  </Accordion>
                  <Accordion>
                    <AccordionSummary expandIcon={<ExpandMoreIcon />}>
                      <Typography variant="h6">Scenarios</Typography>
                    </AccordionSummary>
                    <AccordionDetails>
                      <Typography variant="body2" paragraph>
                        Create and run network simulation scenarios.
                      </Typography>
                      <Button variant="outlined" startIcon={<ViewIcon />}>
                        View Guide
                      </Button>
                    </AccordionDetails>
                  </Accordion>
                </Box>
              </CardContent>
            </Card>
          </Grid>
        </Grid>
      </TabPanel>

      {/* Developer Guides Tab */}
      <TabPanel value={tabValue} index={4}>
        <Grid container spacing={3}>
          <Grid item xs={12}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  Developer Guides
                </Typography>
                <Typography variant="body2" color="text.secondary" paragraph>
                  Technical documentation for developers and contributors.
                </Typography>
                <Box>
                  <Accordion>
                    <AccordionSummary expandIcon={<ExpandMoreIcon />}>
                      <Typography variant="h6">Architecture</Typography>
                    </AccordionSummary>
                    <AccordionDetails>
                      <Typography variant="body2" paragraph>
                        Overview of the system architecture and component interactions.
                      </Typography>
                      <Button variant="outlined" startIcon={<ViewIcon />}>
                        View Guide
                      </Button>
                    </AccordionDetails>
                  </Accordion>
                  <Accordion>
                    <AccordionSummary expandIcon={<ExpandMoreIcon />}>
                      <Typography variant="h6">Contributing</Typography>
                    </AccordionSummary>
                    <AccordionDetails>
                      <Typography variant="body2" paragraph>
                        How to contribute to the project and submit pull requests.
                      </Typography>
                      <Button variant="outlined" startIcon={<ViewIcon />}>
                        View Guide
                      </Button>
                    </AccordionDetails>
                  </Accordion>
                  <Accordion>
                    <AccordionSummary expandIcon={<ExpandMoreIcon />}>
                      <Typography variant="h6">Testing</Typography>
                    </AccordionSummary>
                    <AccordionDetails>
                      <Typography variant="body2" paragraph>
                        Running tests and adding new test cases.
                      </Typography>
                      <Button variant="outlined" startIcon={<ViewIcon />}>
                        View Guide
                      </Button>
                    </AccordionDetails>
                  </Accordion>
                </Box>
              </CardContent>
            </Card>
          </Grid>
        </Grid>
      </TabPanel>

      {/* Item Dialog */}
      <Dialog
        open={itemDialogOpen}
        onClose={() => setItemDialogOpen(false)}
        maxWidth="md"
        fullWidth
      >
        <DialogTitle>
          {editingItem ? 'Edit Documentation Item' : 'Create Documentation Item'}
        </DialogTitle>
        <DialogContent>
          <DocumentationItemForm
            item={editingItem}
            onSave={handleSaveItem}
            onCancel={() => setItemDialogOpen(false)}
            categories={categories?.data || []}
          />
        </DialogContent>
      </Dialog>

      {/* View Item Dialog */}
      <Dialog
        open={!!selectedItem}
        onClose={() => setSelectedItem(null)}
        maxWidth="md"
        fullWidth
      >
        <DialogTitle>
          {selectedItem?.title}
        </DialogTitle>
        <DialogContent>
          {selectedItem && (
            <Box>
              <Typography variant="body2" color="text.secondary" paragraph>
                Type: {getTypeLabel(selectedItem.type)} | Status: {selectedItem.status} | Version: {selectedItem.version}
              </Typography>
              <Divider sx={{ my: 2 }} />
              <Typography variant="body1" paragraph>
                {selectedItem.content}
              </Typography>
            </Box>
          )}
        </DialogContent>
        <DialogActions>
          <Button onClick={() => setSelectedItem(null)}>Close</Button>
          <Button
            variant="contained"
            onClick={() => selectedItem && handleEditItem(selectedItem)}
          >
            Edit
          </Button>
        </DialogActions>
      </Dialog>

      {/* Snackbar for notifications */}
      <Snackbar
        open={snackbarOpen}
        autoHideDuration={6000}
        onClose={() => setSnackbarOpen(false)}
      >
        <Alert
          onClose={() => setSnackbarOpen(false)}
          severity={snackbarSeverity}
          sx={{ width: '100%' }}
        >
          {snackbarMessage}
        </Alert>
      </Snackbar>
    </Box>
  );
};

interface DocumentationItemFormProps {
  item: DocumentationItem | null;
  onSave: (data: Partial<DocumentationItem>) => void;
  onCancel: () => void;
  categories: DocumentationCategory[];
}

const DocumentationItemForm: React.FC<DocumentationItemFormProps> = ({ item, onSave, onCancel, categories }) => {
  const [formData, setFormData] = useState({
    title: item?.title || '',
    content: item?.content || '',
    type: item?.type || 'user_guide',
    category: item?.category || '',
    tags: item?.tags || [],
    status: item?.status || 'draft',
    version: item?.version || '1.0.0',
  });

  const handleSubmit = (e: React.FormEvent) => {
    e.preventDefault();
    onSave(formData);
  };

  return (
    <Box component="form" onSubmit={handleSubmit}>
      <Grid container spacing={2} sx={{ mt: 1 }}>
        <Grid item xs={12} md={6}>
          <TextField
            fullWidth
            label="Title"
            value={formData.title}
            onChange={(e) => setFormData({ ...formData, title: e.target.value })}
            required
          />
        </Grid>
        <Grid item xs={12} md={6}>
          <FormControl fullWidth>
            <InputLabel>Type</InputLabel>
            <Select
              value={formData.type}
              onChange={(e) => setFormData({ ...formData, type: e.target.value as any })}
            >
              <MenuItem value="api">API Documentation</MenuItem>
              <MenuItem value="user_guide">User Guide</MenuItem>
              <MenuItem value="developer_guide">Developer Guide</MenuItem>
              <MenuItem value="tutorial">Tutorial</MenuItem>
              <MenuItem value="reference">Reference</MenuItem>
              <MenuItem value="changelog">Changelog</MenuItem>
            </Select>
          </FormControl>
        </Grid>
        <Grid item xs={12} md={6}>
          <FormControl fullWidth>
            <InputLabel>Category</InputLabel>
            <Select
              value={formData.category}
              onChange={(e) => setFormData({ ...formData, category: e.target.value })}
            >
              {categories.map((category) => (
                <MenuItem key={category.id} value={category.name}>
                  {category.name}
                </MenuItem>
              ))}
            </Select>
          </FormControl>
        </Grid>
        <Grid item xs={12} md={6}>
          <FormControl fullWidth>
            <InputLabel>Status</InputLabel>
            <Select
              value={formData.status}
              onChange={(e) => setFormData({ ...formData, status: e.target.value as any })}
            >
              <MenuItem value="draft">Draft</MenuItem>
              <MenuItem value="published">Published</MenuItem>
              <MenuItem value="archived">Archived</MenuItem>
            </Select>
          </FormControl>
        </Grid>
        <Grid item xs={12} md={6}>
          <TextField
            fullWidth
            label="Version"
            value={formData.version}
            onChange={(e) => setFormData({ ...formData, version: e.target.value })}
          />
        </Grid>
        <Grid item xs={12} md={6}>
          <TextField
            fullWidth
            label="Tags (comma-separated)"
            value={formData.tags.join(', ')}
            onChange={(e) => setFormData({ ...formData, tags: e.target.value.split(',').map(t => t.trim()) })}
          />
        </Grid>
        <Grid item xs={12}>
          <TextField
            fullWidth
            label="Content"
            multiline
            rows={10}
            value={formData.content}
            onChange={(e) => setFormData({ ...formData, content: e.target.value })}
            required
          />
        </Grid>
      </Grid>
      <DialogActions>
        <Button onClick={onCancel}>Cancel</Button>
        <Button type="submit" variant="contained">
          {item ? 'Update' : 'Create'}
        </Button>
      </DialogActions>
    </Box>
  );
};

export default Documentation;
