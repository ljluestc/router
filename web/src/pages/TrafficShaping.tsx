import { jsx as _jsx, jsxs as _jsxs } from "react/jsx-runtime";
import { useState, useEffect } from 'react';
import { Box, Grid, Card, CardContent, Typography, Button, Table, TableBody, TableCell, TableContainer, TableHead, TableRow, Paper, Chip, IconButton, Dialog, DialogTitle, DialogContent, DialogActions, TextField, MenuItem, Tabs, Tab, List, ListItem, ListItemText, LinearProgress, FormControl, InputLabel, Select, } from '@mui/material';
import { Speed as SpeedIcon, NetworkCheck as NetworkIcon, Add as AddIcon, Edit as EditIcon, Delete as DeleteIcon, Refresh as RefreshIcon, PlayArrow as PlayIcon, Pause as PauseIcon, Timeline as TimelineIcon, Storage as StorageIcon, } from '@mui/icons-material';
import { XAxis, YAxis, CartesianGrid, Tooltip, ResponsiveContainer, BarChart, Bar, PieChart, Pie, Cell, AreaChart, Area, } from 'recharts';
const TrafficShaping = () => {
    const [rules, setRules] = useState([
        {
            id: '1',
            name: 'High Priority Traffic',
            type: 'priority',
            interface: 'eth0',
            priority: 1,
            bandwidth: 1000,
            burst: 2000,
            status: 'active',
            packets: 12500,
            bytes: 15600000,
            dropped: 25,
            utilization: 85,
        },
        {
            id: '2',
            name: 'BGP Traffic',
            type: 'token-bucket',
            interface: 'eth1',
            priority: 2,
            bandwidth: 500,
            burst: 1000,
            status: 'active',
            packets: 8500,
            bytes: 10200000,
            dropped: 12,
            utilization: 70,
        },
        {
            id: '3',
            name: 'OSPF Traffic',
            type: 'wfq',
            interface: 'eth2',
            priority: 3,
            bandwidth: 300,
            burst: 600,
            status: 'active',
            packets: 6200,
            bytes: 7440000,
            dropped: 8,
            utilization: 60,
        },
        {
            id: '4',
            name: 'ISIS Traffic',
            type: 'rate-limit',
            interface: 'eth3',
            priority: 4,
            bandwidth: 200,
            burst: 400,
            status: 'active',
            packets: 4800,
            bytes: 5760000,
            dropped: 5,
            utilization: 45,
        },
    ]);
    const [tokenBuckets, setTokenBuckets] = useState([
        { rate: 1000, burst: 2000, tokens: 1500, lastUpdate: Date.now() },
        { rate: 500, burst: 1000, tokens: 750, lastUpdate: Date.now() },
        { rate: 300, burst: 600, tokens: 450, lastUpdate: Date.now() },
        { rate: 200, burst: 400, tokens: 300, lastUpdate: Date.now() },
    ]);
    const [wfqConfigs, setWfqConfigs] = useState([
        { queues: 4, weights: [40, 30, 20, 10], currentQueue: 0, packetsPerQueue: [120, 90, 60, 30] },
        { queues: 3, weights: [50, 30, 20], currentQueue: 1, packetsPerQueue: [150, 90, 60] },
        { queues: 2, weights: [70, 30], currentQueue: 0, packetsPerQueue: [210, 90] },
    ]);
    const [currentTab, setCurrentTab] = useState(0);
    const [openDialog, setOpenDialog] = useState(false);
    const [selectedRule, setSelectedRule] = useState(null);
    const [simulationRunning, setSimulationRunning] = useState(false);
    const [loading, setLoading] = useState(false);
    const [trafficData, setTrafficData] = useState([
        { time: '00:00', ingress: 120, egress: 95, shaped: 80, dropped: 5 },
        { time: '01:00', ingress: 135, egress: 110, shaped: 90, dropped: 8 },
        { time: '02:00', ingress: 98, egress: 78, shaped: 70, dropped: 3 },
        { time: '03:00', ingress: 145, egress: 120, shaped: 100, dropped: 12 },
        { time: '04:00', ingress: 167, egress: 140, shaped: 115, dropped: 15 },
        { time: '05:00', ingress: 189, egress: 155, shaped: 130, dropped: 18 },
    ]);
    const [utilizationData, setUtilizationData] = useState([
        { rule: 'High Priority', utilization: 85, dropped: 25 },
        { rule: 'BGP Traffic', utilization: 70, dropped: 12 },
        { rule: 'OSPF Traffic', utilization: 60, dropped: 8 },
        { rule: 'ISIS Traffic', utilization: 45, dropped: 5 },
    ]);
    const [queueData, setQueueData] = useState([
        { queue: 'Queue 1', packets: 120, weight: 40, utilization: 80 },
        { queue: 'Queue 2', packets: 90, weight: 30, utilization: 60 },
        { queue: 'Queue 3', packets: 60, weight: 20, utilization: 40 },
        { queue: 'Queue 4', packets: 30, weight: 10, utilization: 20 },
    ]);
    const getTypeIcon = (type) => {
        switch (type) {
            case 'token-bucket':
                return _jsx(StorageIcon, { color: "primary" });
            case 'wfq':
                return _jsx(TimelineIcon, { color: "secondary" });
            case 'priority':
                return _jsx(SpeedIcon, { color: "success" });
            case 'rate-limit':
                return _jsx(NetworkIcon, { color: "warning" });
            default:
                return _jsx(SpeedIcon, {});
        }
    };
    const getStatusColor = (status) => {
        switch (status.toLowerCase()) {
            case 'active':
            case 'up':
                return 'success';
            case 'inactive':
            case 'down':
                return 'error';
            case 'pending':
                return 'warning';
            default:
                return 'default';
        }
    };
    const getUtilizationColor = (utilization) => {
        if (utilization > 90)
            return '#f44336';
        if (utilization > 70)
            return '#ff9800';
        if (utilization > 50)
            return '#ffeb3b';
        return '#4caf50';
    };
    const handleRefresh = async () => {
        setLoading(true);
        try {
            // Simulate API call
            await new Promise(resolve => setTimeout(resolve, 1000));
            // Update data here
        }
        catch (error) {
            console.error('Failed to refresh traffic shaping data:', error);
        }
        finally {
            setLoading(false);
        }
    };
    const handleEditRule = (rule) => {
        setSelectedRule(rule);
        setOpenDialog(true);
    };
    const handleDeleteRule = async (ruleId) => {
        if (window.confirm('Are you sure you want to delete this rule?')) {
            try {
                // Simulate API call
                await new Promise(resolve => setTimeout(resolve, 500));
                setRules(prev => prev.filter(r => r.id !== ruleId));
            }
            catch (error) {
                console.error('Failed to delete rule:', error);
            }
        }
    };
    const handleStartSimulation = () => {
        setSimulationRunning(true);
        // Start simulation logic here
    };
    const handleStopSimulation = () => {
        setSimulationRunning(false);
        // Stop simulation logic here
    };
    useEffect(() => {
        if (simulationRunning) {
            const interval = setInterval(() => {
                // Simulate real-time updates
                setRules(prev => prev.map(rule => ({
                    ...rule,
                    packets: rule.packets + Math.floor(Math.random() * 100),
                    bytes: rule.bytes + Math.floor(Math.random() * 10000),
                    dropped: rule.dropped + Math.floor(Math.random() * 2),
                    utilization: Math.max(0, Math.min(100, rule.utilization + (Math.random() - 0.5) * 10))
                })));
            }, 2000);
            return () => clearInterval(interval);
        }
    }, [simulationRunning]);
    return (_jsxs(Box, { children: [_jsxs(Box, { sx: { display: 'flex', justifyContent: 'space-between', alignItems: 'center', mb: 3 }, children: [_jsx(Typography, { variant: "h4", component: "h1", children: "Traffic Shaping" }), _jsxs(Box, { children: [_jsx(Button, { variant: "outlined", startIcon: _jsx(RefreshIcon, {}), onClick: handleRefresh, disabled: loading, sx: { mr: 1 }, children: "Refresh" }), !simulationRunning ? (_jsx(Button, { variant: "contained", startIcon: _jsx(PlayIcon, {}), onClick: handleStartSimulation, children: "Start Simulation" })) : (_jsx(Button, { variant: "contained", color: "error", startIcon: _jsx(PauseIcon, {}), onClick: handleStopSimulation, children: "Stop Simulation" }))] })] }), loading && _jsx(LinearProgress, { sx: { mb: 2 } }), _jsx(Paper, { sx: { mb: 3 }, children: _jsxs(Tabs, { value: currentTab, onChange: (e, v) => setCurrentTab(v), children: [_jsx(Tab, { label: "Rules" }), _jsx(Tab, { label: "Token Buckets" }), _jsx(Tab, { label: "WFQ Queues" }), _jsx(Tab, { label: "Analytics" })] }) }), currentTab === 0 && (_jsx(Grid, { container: true, spacing: 3, children: _jsx(Grid, { item: true, xs: 12, children: _jsx(Card, { children: _jsxs(CardContent, { children: [_jsxs(Box, { sx: { display: 'flex', justifyContent: 'space-between', alignItems: 'center', mb: 2 }, children: [_jsx(Typography, { variant: "h6", children: "Traffic Shaping Rules" }), _jsx(Button, { variant: "contained", startIcon: _jsx(AddIcon, {}), onClick: () => setOpenDialog(true), children: "Add Rule" })] }), _jsx(TableContainer, { component: Paper, sx: { backgroundColor: 'transparent' }, children: _jsxs(Table, { children: [_jsx(TableHead, { children: _jsxs(TableRow, { children: [_jsx(TableCell, { children: "Name" }), _jsx(TableCell, { children: "Type" }), _jsx(TableCell, { children: "Interface" }), _jsx(TableCell, { children: "Priority" }), _jsx(TableCell, { children: "Bandwidth" }), _jsx(TableCell, { children: "Status" }), _jsx(TableCell, { children: "Packets" }), _jsx(TableCell, { children: "Bytes" }), _jsx(TableCell, { children: "Dropped" }), _jsx(TableCell, { children: "Utilization" }), _jsx(TableCell, { children: "Actions" })] }) }), _jsx(TableBody, { children: rules.map((rule) => (_jsxs(TableRow, { children: [_jsx(TableCell, { children: _jsxs(Box, { sx: { display: 'flex', alignItems: 'center' }, children: [getTypeIcon(rule.type), _jsx(Typography, { variant: "body2", sx: { ml: 1 }, children: rule.name })] }) }), _jsx(TableCell, { children: _jsx(Chip, { label: rule.type.toUpperCase(), size: "small" }) }), _jsx(TableCell, { children: _jsx(Typography, { variant: "body2", fontFamily: "monospace", children: rule.interface }) }), _jsx(TableCell, { children: rule.priority }), _jsxs(TableCell, { children: [rule.bandwidth, " Mbps"] }), _jsx(TableCell, { children: _jsx(Chip, { label: rule.status, color: getStatusColor(rule.status), size: "small" }) }), _jsx(TableCell, { children: rule.packets.toLocaleString() }), _jsxs(TableCell, { children: [(rule.bytes / 1024 / 1024).toFixed(2), " MB"] }), _jsx(TableCell, { children: _jsx(Chip, { label: rule.dropped, color: rule.dropped > 10 ? 'error' : 'default', size: "small" }) }), _jsx(TableCell, { children: _jsxs(Box, { sx: { display: 'flex', alignItems: 'center', gap: 1 }, children: [_jsx(Box, { sx: { width: 60, height: 4, backgroundColor: '#e0e0e0', borderRadius: 2 }, children: _jsx(Box, { sx: {
                                                                                width: `${rule.utilization}%`,
                                                                                height: '100%',
                                                                                backgroundColor: getUtilizationColor(rule.utilization),
                                                                                borderRadius: 2,
                                                                            } }) }), _jsxs(Typography, { variant: "body2", children: [rule.utilization.toFixed(1), "%"] })] }) }), _jsxs(TableCell, { children: [_jsx(IconButton, { size: "small", onClick: () => handleEditRule(rule), children: _jsx(EditIcon, {}) }), _jsx(IconButton, { size: "small", onClick: () => handleDeleteRule(rule.id), color: "error", children: _jsx(DeleteIcon, {}) })] })] }, rule.id))) })] }) })] }) }) }) })), currentTab === 1 && (_jsx(Grid, { container: true, spacing: 3, children: tokenBuckets.map((bucket, index) => (_jsx(Grid, { item: true, xs: 12, md: 6, children: _jsx(Card, { children: _jsxs(CardContent, { children: [_jsxs(Typography, { variant: "h6", gutterBottom: true, children: ["Token Bucket ", index + 1] }), _jsxs(List, { dense: true, children: [_jsx(ListItem, { children: _jsx(ListItemText, { primary: "Rate", secondary: `${bucket.rate} tokens/sec` }) }), _jsx(ListItem, { children: _jsx(ListItemText, { primary: "Burst Size", secondary: `${bucket.burst} tokens` }) }), _jsx(ListItem, { children: _jsx(ListItemText, { primary: "Current Tokens", secondary: `${bucket.tokens} tokens` }) }), _jsx(ListItem, { children: _jsx(ListItemText, { primary: "Utilization", secondary: `${((bucket.tokens / bucket.burst) * 100).toFixed(1)}%` }) })] }), _jsxs(Box, { sx: { mt: 2 }, children: [_jsx(Typography, { variant: "body2", gutterBottom: true, children: "Token Level" }), _jsx(Box, { sx: { width: '100%', height: 20, backgroundColor: '#e0e0e0', borderRadius: 2, overflow: 'hidden' }, children: _jsx(Box, { sx: {
                                                    width: `${(bucket.tokens / bucket.burst) * 100}%`,
                                                    height: '100%',
                                                    backgroundColor: getUtilizationColor((bucket.tokens / bucket.burst) * 100),
                                                } }) })] })] }) }) }, index))) })), currentTab === 2 && (_jsx(Grid, { container: true, spacing: 3, children: wfqConfigs.map((config, index) => (_jsx(Grid, { item: true, xs: 12, md: 6, children: _jsx(Card, { children: _jsxs(CardContent, { children: [_jsxs(Typography, { variant: "h6", gutterBottom: true, children: ["WFQ Configuration ", index + 1] }), _jsxs(List, { dense: true, children: [_jsx(ListItem, { children: _jsx(ListItemText, { primary: "Number of Queues", secondary: config.queues.toString() }) }), _jsx(ListItem, { children: _jsx(ListItemText, { primary: "Current Queue", secondary: `Queue ${config.currentQueue + 1}` }) })] }), _jsx(Typography, { variant: "h6", gutterBottom: true, sx: { mt: 2 }, children: "Queue Details" }), config.weights.map((weight, queueIndex) => (_jsxs(Box, { sx: { mb: 2 }, children: [_jsxs(Box, { sx: { display: 'flex', justifyContent: 'space-between', alignItems: 'center', mb: 1 }, children: [_jsxs(Typography, { variant: "body2", children: ["Queue ", queueIndex + 1] }), _jsxs(Typography, { variant: "body2", children: [config.packetsPerQueue[queueIndex], " packets"] })] }), _jsxs(Box, { sx: { display: 'flex', gap: 1, mb: 1 }, children: [_jsx(Box, { sx: { flex: 1, height: 8, backgroundColor: '#e0e0e0', borderRadius: 1 }, children: _jsx(Box, { sx: {
                                                            width: `${(config.packetsPerQueue[queueIndex] / Math.max(...config.packetsPerQueue)) * 100}%`,
                                                            height: '100%',
                                                            backgroundColor: getUtilizationColor((config.packetsPerQueue[queueIndex] / Math.max(...config.packetsPerQueue)) * 100),
                                                            borderRadius: 1,
                                                        } }) }), _jsxs(Typography, { variant: "body2", sx: { minWidth: 40 }, children: [weight, "%"] })] })] }, queueIndex)))] }) }) }, index))) })), currentTab === 3 && (_jsxs(Grid, { container: true, spacing: 3, children: [_jsx(Grid, { item: true, xs: 12, md: 8, children: _jsx(Card, { children: _jsxs(CardContent, { children: [_jsx(Typography, { variant: "h6", gutterBottom: true, children: "Traffic Shaping Analytics" }), _jsx(ResponsiveContainer, { width: "100%", height: 400, children: _jsxs(AreaChart, { data: trafficData, children: [_jsx(CartesianGrid, { strokeDasharray: "3 3" }), _jsx(XAxis, { dataKey: "time" }), _jsx(YAxis, {}), _jsx(Tooltip, {}), _jsx(Area, { type: "monotone", dataKey: "ingress", stackId: "1", stroke: "#1976d2", fill: "#1976d2", fillOpacity: 0.3, name: "Ingress" }), _jsx(Area, { type: "monotone", dataKey: "egress", stackId: "1", stroke: "#dc004e", fill: "#dc004e", fillOpacity: 0.3, name: "Egress" }), _jsx(Area, { type: "monotone", dataKey: "shaped", stackId: "1", stroke: "#2e7d32", fill: "#2e7d32", fillOpacity: 0.3, name: "Shaped" }), _jsx(Area, { type: "monotone", dataKey: "dropped", stackId: "1", stroke: "#f44336", fill: "#f44336", fillOpacity: 0.3, name: "Dropped" })] }) })] }) }) }), _jsx(Grid, { item: true, xs: 12, md: 4, children: _jsx(Card, { children: _jsxs(CardContent, { children: [_jsx(Typography, { variant: "h6", gutterBottom: true, children: "Rule Utilization" }), _jsx(ResponsiveContainer, { width: "100%", height: 300, children: _jsxs(BarChart, { data: utilizationData, children: [_jsx(CartesianGrid, { strokeDasharray: "3 3" }), _jsx(XAxis, { dataKey: "rule" }), _jsx(YAxis, {}), _jsx(Tooltip, {}), _jsx(Bar, { dataKey: "utilization", fill: "#1976d2", name: "Utilization %" }), _jsx(Bar, { dataKey: "dropped", fill: "#f44336", name: "Dropped Packets" })] }) })] }) }) }), _jsx(Grid, { item: true, xs: 12, md: 6, children: _jsx(Card, { children: _jsxs(CardContent, { children: [_jsx(Typography, { variant: "h6", gutterBottom: true, children: "Queue Distribution" }), _jsx(ResponsiveContainer, { width: "100%", height: 300, children: _jsxs(PieChart, { children: [_jsx(Pie, { data: queueData, cx: "50%", cy: "50%", innerRadius: 60, outerRadius: 100, paddingAngle: 5, dataKey: "packets", children: queueData.map((entry, index) => (_jsx(Cell, { fill: getUtilizationColor(entry.utilization) }, `cell-${index}`))) }), _jsx(Tooltip, {})] }) })] }) }) }), _jsx(Grid, { item: true, xs: 12, md: 6, children: _jsx(Card, { children: _jsxs(CardContent, { children: [_jsx(Typography, { variant: "h6", gutterBottom: true, children: "Queue Statistics" }), _jsx(List, { children: queueData.map((queue, index) => (_jsx(ListItem, { children: _jsx(ListItemText, { primary: queue.queue, secondary: `${queue.packets} packets, ${queue.weight}% weight, ${queue.utilization}% utilization` }) }, index))) })] }) }) })] })), _jsxs(Dialog, { open: openDialog, onClose: () => setOpenDialog(false), maxWidth: "sm", fullWidth: true, children: [_jsx(DialogTitle, { children: selectedRule ? 'Edit Rule' : 'Add Rule' }), _jsx(DialogContent, { children: _jsxs(Box, { sx: { display: 'flex', flexDirection: 'column', gap: 2, mt: 1 }, children: [_jsx(TextField, { label: "Rule Name", defaultValue: selectedRule?.name || '', fullWidth: true }), _jsxs(FormControl, { fullWidth: true, children: [_jsx(InputLabel, { children: "Type" }), _jsxs(Select, { defaultValue: selectedRule?.type || '', label: "Type", children: [_jsx(MenuItem, { value: "token-bucket", children: "Token Bucket" }), _jsx(MenuItem, { value: "wfq", children: "Weighted Fair Queuing" }), _jsx(MenuItem, { value: "priority", children: "Priority" }), _jsx(MenuItem, { value: "rate-limit", children: "Rate Limit" })] })] }), _jsx(TextField, { label: "Interface", defaultValue: selectedRule?.interface || '', fullWidth: true }), _jsx(TextField, { label: "Priority", type: "number", defaultValue: selectedRule?.priority || '', fullWidth: true }), _jsx(TextField, { label: "Bandwidth (Mbps)", type: "number", defaultValue: selectedRule?.bandwidth || '', fullWidth: true }), _jsx(TextField, { label: "Burst Size", type: "number", defaultValue: selectedRule?.burst || '', fullWidth: true })] }) }), _jsxs(DialogActions, { children: [_jsx(Button, { onClick: () => setOpenDialog(false), children: "Cancel" }), _jsx(Button, { variant: "contained", children: "Save" })] })] })] }));
};
export default TrafficShaping;
