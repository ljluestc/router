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
  Stepper,
  Step,
  StepLabel,
  StepContent,
} from '@mui/material';
import {
  Add as AddIcon,
  Edit as EditIcon,
  Delete as DeleteIcon,
  PlayArrow as PlayIcon,
  Stop as StopIcon,
  Pause as PauseIcon,
  Refresh as RefreshIcon,
  BugReport as BugReportIcon,
  CheckCircle as CheckCircleIcon,
  Error as ErrorIcon,
  Warning as WarningIcon,
  Download as DownloadIcon,
  Upload as UploadIcon,
  Visibility as ViewIcon,
} from '@mui/icons-material';
import { useQuery, useMutation, useQueryClient } from '@tanstack/react-query';

interface TestCase {
  id: string;
  name: string;
  description: string;
  type: 'unit' | 'integration' | 'performance' | 'stress' | 'regression' | 'smoke';
  status: 'pending' | 'running' | 'passed' | 'failed' | 'skipped' | 'error';
  priority: 'low' | 'medium' | 'high' | 'critical';
  tags: string[];
  steps: TestStep[];
  expected_result: string;
  actual_result: string;
  duration: number;
  created_at: string;
  updated_at: string;
}

interface TestStep {
  id: string;
  description: string;
  action: string;
  expected: string;
  actual: string;
  status: 'pending' | 'passed' | 'failed' | 'skipped';
}

interface TestSuite {
  id: string;
  name: string;
  description: string;
  test_cases: string[];
  status: 'pending' | 'running' | 'completed' | 'failed';
  created_at: string;
  updated_at: string;
}

interface TestResult {
  id: string;
  test_case_id: string;
  status: 'passed' | 'failed' | 'skipped' | 'error';
  duration: number;
  error_message: string;
  logs: string[];
  created_at: string;
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
      id={`testing-tabpanel-${index}`}
      aria-labelledby={`testing-tab-${index}`}
      {...other}
    >
      {value === index && <Box sx={{ p: 3 }}>{children}</Box>}
    </div>
  );
}

const Testing: React.FC = () => {
  const [selectedTestCase, setSelectedTestCase] = useState<TestCase | null>(null);
  const [testDialogOpen, setTestDialogOpen] = useState(false);
  const [editingTestCase, setEditingTestCase] = useState<TestCase | null>(null);
  const [snackbarOpen, setSnackbarOpen] = useState(false);
  const [snackbarMessage, setSnackbarMessage] = useState('');
  const [snackbarSeverity, setSnackbarSeverity] = useState<'success' | 'error'>('success');
  const [tabValue, setTabValue] = useState(0);
  const [activeStep, setActiveStep] = useState(0);

  const queryClient = useQueryClient();

  const { data: testCases, isLoading } = useQuery({
    queryKey: ['test-cases'],
    queryFn: async () => {
      const response = await fetch('/api/v1/testing/test-cases');
      return response.json();
    },
  });

  const { data: testSuites } = useQuery({
    queryKey: ['test-suites'],
    queryFn: async () => {
      const response = await fetch('/api/v1/testing/test-suites');
      return response.json();
    },
  });

  const { data: testResults } = useQuery({
    queryKey: ['test-results'],
    queryFn: async () => {
      const response = await fetch('/api/v1/testing/test-results');
      return response.json();
    },
  });

  const { data: testStats } = useQuery({
    queryKey: ['test-stats'],
    queryFn: async () => {
      const response = await fetch('/api/v1/testing/stats');
      return response.json();
    },
  });

  const createTestCaseMutation = useMutation({
    mutationFn: async (testData: Partial<TestCase>) => {
      const response = await fetch('/api/v1/testing/test-cases', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify(testData),
      });
      return response.json();
    },
    onSuccess: () => {
      queryClient.invalidateQueries({ queryKey: ['test-cases'] });
      setSnackbarMessage('Test case created successfully');
      setSnackbarOpen(true);
      setTestDialogOpen(false);
    },
    onError: () => {
      setSnackbarMessage('Failed to create test case');
      setSnackbarSeverity('error');
      setSnackbarOpen(true);
    },
  });

  const updateTestCaseMutation = useMutation({
    mutationFn: async ({ id, ...testData }: Partial<TestCase> & { id: string }) => {
      const response = await fetch(`/api/v1/testing/test-cases/${id}`, {
        method: 'PUT',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify(testData),
      });
      return response.json();
    },
    onSuccess: () => {
      queryClient.invalidateQueries({ queryKey: ['test-cases'] });
      setSnackbarMessage('Test case updated successfully');
      setSnackbarOpen(true);
      setTestDialogOpen(false);
    },
    onError: () => {
      setSnackbarMessage('Failed to update test case');
      setSnackbarSeverity('error');
      setSnackbarOpen(true);
    },
  });

  const deleteTestCaseMutation = useMutation({
    mutationFn: async (testId: string) => {
      const response = await fetch(`/api/v1/testing/test-cases/${testId}`, {
        method: 'DELETE',
      });
      return response.json();
    },
    onSuccess: () => {
      queryClient.invalidateQueries({ queryKey: ['test-cases'] });
      setSnackbarMessage('Test case deleted successfully');
      setSnackbarOpen(true);
    },
    onError: () => {
      setSnackbarMessage('Failed to delete test case');
      setSnackbarSeverity('error');
      setSnackbarOpen(true);
    },
  });

  const runTestCaseMutation = useMutation({
    mutationFn: async (testId: string) => {
      const response = await fetch(`/api/v1/testing/test-cases/${testId}/run`, {
        method: 'POST',
      });
      return response.json();
    },
    onSuccess: () => {
      queryClient.invalidateQueries({ queryKey: ['test-cases'] });
      queryClient.invalidateQueries({ queryKey: ['test-results'] });
      setSnackbarMessage('Test case started successfully');
      setSnackbarOpen(true);
    },
    onError: () => {
      setSnackbarMessage('Failed to start test case');
      setSnackbarSeverity('error');
      setSnackbarOpen(true);
    },
  });

  const runTestSuiteMutation = useMutation({
    mutationFn: async (suiteId: string) => {
      const response = await fetch(`/api/v1/testing/test-suites/${suiteId}/run`, {
        method: 'POST',
      });
      return response.json();
    },
    onSuccess: () => {
      queryClient.invalidateQueries({ queryKey: ['test-suites'] });
      queryClient.invalidateQueries({ queryKey: ['test-results'] });
      setSnackbarMessage('Test suite started successfully');
      setSnackbarOpen(true);
    },
    onError: () => {
      setSnackbarMessage('Failed to start test suite');
      setSnackbarSeverity('error');
      setSnackbarOpen(true);
    },
  });

  const handleCreateTestCase = () => {
    setEditingTestCase(null);
    setTestDialogOpen(true);
  };

  const handleEditTestCase = (testCase: TestCase) => {
    setEditingTestCase(testCase);
    setTestDialogOpen(true);
  };

  const handleDeleteTestCase = (testId: string) => {
    if (window.confirm('Are you sure you want to delete this test case?')) {
      deleteTestCaseMutation.mutate(testId);
    }
  };

  const handleRunTestCase = (testId: string) => {
    runTestCaseMutation.mutate(testId);
  };

  const handleRunTestSuite = (suiteId: string) => {
    runTestSuiteMutation.mutate(suiteId);
  };

  const handleSaveTestCase = (testData: Partial<TestCase>) => {
    if (editingTestCase) {
      updateTestCaseMutation.mutate({ ...testData, id: editingTestCase.id });
    } else {
      createTestCaseMutation.mutate(testData);
    }
  };

  const handleTabChange = (event: React.SyntheticEvent, newValue: number) => {
    setTabValue(newValue);
  };

  const getStatusColor = (status: string) => {
    switch (status) {
      case 'passed':
        return 'success';
      case 'failed':
        return 'error';
      case 'running':
        return 'info';
      case 'pending':
        return 'default';
      case 'skipped':
        return 'warning';
      case 'error':
        return 'error';
      default:
        return 'default';
    }
  };

  const getStatusIcon = (status: string) => {
    switch (status) {
      case 'passed':
        return <CheckCircleIcon />;
      case 'failed':
        return <ErrorIcon />;
      case 'running':
        return <PlayIcon />;
      case 'pending':
        return <StopIcon />;
      case 'skipped':
        return <WarningIcon />;
      case 'error':
        return <ErrorIcon />;
      default:
        return <StopIcon />;
    }
  };

  const getTestTypeLabel = (type: string) => {
    switch (type) {
      case 'unit':
        return 'Unit Test';
      case 'integration':
        return 'Integration Test';
      case 'performance':
        return 'Performance Test';
      case 'stress':
        return 'Stress Test';
      case 'regression':
        return 'Regression Test';
      case 'smoke':
        return 'Smoke Test';
      default:
        return type;
    }
  };

  const getPriorityColor = (priority: string) => {
    switch (priority) {
      case 'critical':
        return 'error';
      case 'high':
        return 'warning';
      case 'medium':
        return 'info';
      case 'low':
        return 'default';
      default:
        return 'default';
    }
  };

  if (isLoading) {
    return (
      <Box display="flex" justifyContent="center" alignItems="center" minHeight="400px">
        <Typography>Loading test cases...</Typography>
      </Box>
    );
  }

  return (
    <Box>
      <Box display="flex" justifyContent="space-between" alignItems="center" mb={3}>
        <Typography variant="h4" component="h1">
          Testing
        </Typography>
        <Box display="flex" gap={2}>
          <Button
            variant="outlined"
            startIcon={<RefreshIcon />}
            onClick={() => queryClient.invalidateQueries({ queryKey: ['test-cases'] })}
          >
            Refresh
          </Button>
          <Button
            variant="contained"
            startIcon={<AddIcon />}
            onClick={handleCreateTestCase}
          >
            Add Test Case
          </Button>
        </Box>
      </Box>

      <Box sx={{ borderBottom: 1, borderColor: 'divider' }}>
        <Tabs value={tabValue} onChange={handleTabChange} aria-label="testing tabs">
          <Tab label="Test Cases" />
          <Tab label="Test Suites" />
          <Tab label="Test Results" />
          <Tab label="Statistics" />
        </Tabs>
      </Box>

      {/* Test Cases Tab */}
      <TabPanel value={tabValue} index={0}>
        <Grid container spacing={3}>
          {/* Test Statistics */}
          <Grid item xs={12} md={6}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  Test Statistics
                </Typography>
                {testStats?.data && (
                  <Grid container spacing={2}>
                    <Grid item xs={6}>
                      <Typography variant="body2" color="text.secondary">
                        Total Tests
                      </Typography>
                      <Typography variant="h6">
                        {testStats.data.total_tests}
                      </Typography>
                    </Grid>
                    <Grid item xs={6}>
                      <Typography variant="body2" color="text.secondary">
                        Passed
                      </Typography>
                      <Typography variant="h6" color="success.main">
                        {testStats.data.passed_tests}
                      </Typography>
                    </Grid>
                    <Grid item xs={6}>
                      <Typography variant="body2" color="text.secondary">
                        Failed
                      </Typography>
                      <Typography variant="h6" color="error.main">
                        {testStats.data.failed_tests}
                      </Typography>
                    </Grid>
                    <Grid item xs={6}>
                      <Typography variant="body2" color="text.secondary">
                        Success Rate
                      </Typography>
                      <Typography variant="h6">
                        {(testStats.data.success_rate * 100).toFixed(1)}%
                      </Typography>
                    </Grid>
                  </Grid>
                )}
              </CardContent>
            </Card>
          </Grid>

          {/* Test Performance */}
          <Grid item xs={12} md={6}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  Test Performance
                </Typography>
                {testStats?.data && (
                  <Grid container spacing={2}>
                    <Grid item xs={12}>
                      <Typography variant="body2" color="text.secondary">
                        Average Duration
                      </Typography>
                      <Typography variant="h6">
                        {testStats.data.average_duration?.toFixed(2)}s
                      </Typography>
                      <LinearProgress
                        variant="determinate"
                        value={Math.min((testStats.data.average_duration / 60) * 100, 100)}
                        sx={{ mt: 1 }}
                      />
                    </Grid>
                    <Grid item xs={12}>
                      <Typography variant="body2" color="text.secondary">
                        Total Duration
                      </Typography>
                      <Typography variant="h6">
                        {testStats.data.total_duration?.toFixed(2)}s
                      </Typography>
                    </Grid>
                    <Grid item xs={12}>
                      <Typography variant="body2" color="text.secondary">
                        Tests per Hour
                      </Typography>
                      <Typography variant="h6">
                        {testStats.data.tests_per_hour?.toFixed(0)}
                      </Typography>
                    </Grid>
                  </Grid>
                )}
              </CardContent>
            </Card>
          </Grid>

          {/* Test Cases List */}
          <Grid item xs={12}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  Test Cases
                </Typography>
                {testCases?.data && testCases.data.length > 0 ? (
                  <List>
                    {testCases.data.map((testCase: TestCase) => (
                      <ListItem key={testCase.id} divider>
                        <ListItemText
                          primary={testCase.name}
                          secondary={
                            <Box>
                              <Typography variant="body2" color="text.secondary">
                                {testCase.description}
                              </Typography>
                              <Box display="flex" gap={1} mt={1}>
                                <Chip
                                  icon={getStatusIcon(testCase.status)}
                                  label={testCase.status}
                                  color={getStatusColor(testCase.status) as any}
                                  size="small"
                                />
                                <Chip
                                  label={getTestTypeLabel(testCase.type)}
                                  size="small"
                                  variant="outlined"
                                />
                                <Chip
                                  label={testCase.priority}
                                  color={getPriorityColor(testCase.priority) as any}
                                  size="small"
                                />
                                <Chip
                                  label={`${testCase.duration}s`}
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
                              onClick={() => handleRunTestCase(testCase.id)}
                              color="primary"
                              disabled={testCase.status === 'running'}
                            >
                              <PlayIcon />
                            </IconButton>
                            <IconButton
                              onClick={() => handleEditTestCase(testCase)}
                              color="primary"
                            >
                              <EditIcon />
                            </IconButton>
                            <IconButton
                              onClick={() => handleDeleteTestCase(testCase.id)}
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
                      No test cases available. Create your first test case to get started.
                    </Typography>
                  </Box>
                )}
              </CardContent>
            </Card>
          </Grid>
        </Grid>
      </TabPanel>

      {/* Test Suites Tab */}
      <TabPanel value={tabValue} index={1}>
        <Grid container spacing={3}>
          <Grid item xs={12}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  Test Suites
                </Typography>
                {testSuites?.data && testSuites.data.length > 0 ? (
                  <List>
                    {testSuites.data.map((suite: TestSuite) => (
                      <ListItem key={suite.id} divider>
                        <ListItemText
                          primary={suite.name}
                          secondary={
                            <Box>
                              <Typography variant="body2" color="text.secondary">
                                {suite.description}
                              </Typography>
                              <Box display="flex" gap={1} mt={1}>
                                <Chip
                                  label={suite.status}
                                  color={getStatusColor(suite.status) as any}
                                  size="small"
                                />
                                <Chip
                                  label={`${suite.test_cases.length} tests`}
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
                              onClick={() => handleRunTestSuite(suite.id)}
                              color="primary"
                              disabled={suite.status === 'running'}
                            >
                              <PlayIcon />
                            </IconButton>
                            <IconButton color="primary">
                              <EditIcon />
                            </IconButton>
                            <IconButton color="error">
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
                      No test suites available.
                    </Typography>
                  </Box>
                )}
              </CardContent>
            </Card>
          </Grid>
        </Grid>
      </TabPanel>

      {/* Test Results Tab */}
      <TabPanel value={tabValue} index={2}>
        <Grid container spacing={3}>
          <Grid item xs={12}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  Test Results
                </Typography>
                {testResults?.data && testResults.data.length > 0 ? (
                  <List>
                    {testResults.data.map((result: TestResult) => (
                      <ListItem key={result.id} divider>
                        <ListItemText
                          primary={`Test Case: ${result.test_case_id}`}
                          secondary={
                            <Box>
                              <Typography variant="body2" color="text.secondary">
                                Status: {result.status} | Duration: {result.duration}s
                              </Typography>
                              {result.error_message && (
                                <Typography variant="body2" color="error">
                                  Error: {result.error_message}
                                </Typography>
                              )}
                            </Box>
                          }
                        />
                        <ListItemSecondaryAction>
                          <Chip
                            icon={getStatusIcon(result.status)}
                            label={result.status}
                            color={getStatusColor(result.status) as any}
                            size="small"
                          />
                        </ListItemSecondaryAction>
                      </ListItem>
                    ))}
                  </List>
                ) : (
                  <Box textAlign="center" py={4}>
                    <Typography variant="body1" color="text.secondary">
                      No test results available.
                    </Typography>
                  </Box>
                )}
              </CardContent>
            </Card>
          </Grid>
        </Grid>
      </TabPanel>

      {/* Statistics Tab */}
      <TabPanel value={tabValue} index={3}>
        <Grid container spacing={3}>
          <Grid item xs={12}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  Detailed Statistics
                </Typography>
                {testStats?.data && (
                  <Grid container spacing={2}>
                    <Grid item xs={12} md={3}>
                      <Paper sx={{ p: 2, textAlign: 'center' }}>
                        <Typography variant="h4" color="primary">
                          {testStats.data.total_tests}
                        </Typography>
                        <Typography variant="body2" color="text.secondary">
                          Total Tests
                        </Typography>
                      </Paper>
                    </Grid>
                    <Grid item xs={12} md={3}>
                      <Paper sx={{ p: 2, textAlign: 'center' }}>
                        <Typography variant="h4" color="success.main">
                          {testStats.data.passed_tests}
                        </Typography>
                        <Typography variant="body2" color="text.secondary">
                          Passed
                        </Typography>
                      </Paper>
                    </Grid>
                    <Grid item xs={12} md={3}>
                      <Paper sx={{ p: 2, textAlign: 'center' }}>
                        <Typography variant="h4" color="error.main">
                          {testStats.data.failed_tests}
                        </Typography>
                        <Typography variant="body2" color="text.secondary">
                          Failed
                        </Typography>
                      </Paper>
                    </Grid>
                    <Grid item xs={12} md={3}>
                      <Paper sx={{ p: 2, textAlign: 'center' }}>
                        <Typography variant="h4" color="warning.main">
                          {testStats.data.skipped_tests}
                        </Typography>
                        <Typography variant="body2" color="text.secondary">
                          Skipped
                        </Typography>
                      </Paper>
                    </Grid>
                  </Grid>
                )}
              </CardContent>
            </Card>
          </Grid>
        </Grid>
      </TabPanel>

      {/* Test Case Dialog */}
      <Dialog
        open={testDialogOpen}
        onClose={() => setTestDialogOpen(false)}
        maxWidth="md"
        fullWidth
      >
        <DialogTitle>
          {editingTestCase ? 'Edit Test Case' : 'Create Test Case'}
        </DialogTitle>
        <DialogContent>
          <TestCaseForm
            testCase={editingTestCase}
            onSave={handleSaveTestCase}
            onCancel={() => setTestDialogOpen(false)}
          />
        </DialogContent>
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

interface TestCaseFormProps {
  testCase: TestCase | null;
  onSave: (data: Partial<TestCase>) => void;
  onCancel: () => void;
}

const TestCaseForm: React.FC<TestCaseFormProps> = ({ testCase, onSave, onCancel }) => {
  const [formData, setFormData] = useState({
    name: testCase?.name || '',
    description: testCase?.description || '',
    type: testCase?.type || 'unit',
    priority: testCase?.priority || 'medium',
    tags: testCase?.tags || [],
    expected_result: testCase?.expected_result || '',
    actual_result: testCase?.actual_result || '',
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
            label="Test Case Name"
            value={formData.name}
            onChange={(e) => setFormData({ ...formData, name: e.target.value })}
            required
          />
        </Grid>
        <Grid item xs={12} md={6}>
          <TextField
            fullWidth
            label="Description"
            value={formData.description}
            onChange={(e) => setFormData({ ...formData, description: e.target.value })}
          />
        </Grid>
        <Grid item xs={12} md={6}>
          <FormControl fullWidth>
            <InputLabel>Test Type</InputLabel>
            <Select
              value={formData.type}
              onChange={(e) => setFormData({ ...formData, type: e.target.value as any })}
            >
              <MenuItem value="unit">Unit Test</MenuItem>
              <MenuItem value="integration">Integration Test</MenuItem>
              <MenuItem value="performance">Performance Test</MenuItem>
              <MenuItem value="stress">Stress Test</MenuItem>
              <MenuItem value="regression">Regression Test</MenuItem>
              <MenuItem value="smoke">Smoke Test</MenuItem>
            </Select>
          </FormControl>
        </Grid>
        <Grid item xs={12} md={6}>
          <FormControl fullWidth>
            <InputLabel>Priority</InputLabel>
            <Select
              value={formData.priority}
              onChange={(e) => setFormData({ ...formData, priority: e.target.value as any })}
            >
              <MenuItem value="low">Low</MenuItem>
              <MenuItem value="medium">Medium</MenuItem>
              <MenuItem value="high">High</MenuItem>
              <MenuItem value="critical">Critical</MenuItem>
            </Select>
          </FormControl>
        </Grid>
        <Grid item xs={12}>
          <TextField
            fullWidth
            label="Expected Result"
            multiline
            rows={3}
            value={formData.expected_result}
            onChange={(e) => setFormData({ ...formData, expected_result: e.target.value })}
          />
        </Grid>
        <Grid item xs={12}>
          <TextField
            fullWidth
            label="Actual Result"
            multiline
            rows={3}
            value={formData.actual_result}
            onChange={(e) => setFormData({ ...formData, actual_result: e.target.value })}
          />
        </Grid>
      </Grid>
      <DialogActions>
        <Button onClick={onCancel}>Cancel</Button>
        <Button type="submit" variant="contained">
          {testCase ? 'Update' : 'Create'}
        </Button>
      </DialogActions>
    </Box>
  );
};

export default Testing;
