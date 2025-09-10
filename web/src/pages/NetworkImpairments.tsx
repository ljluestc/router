import { jsx as _jsx, jsxs as _jsxs } from "react/jsx-runtime";
import { useState, useEffect } from 'react';
import { Box, Grid, Card, CardContent, Typography, Button, Table, TableBody, TableCell, TableContainer, TableHead, TableRow, Paper, Chip, IconButton, Dialog, DialogTitle, DialogContent, DialogActions, TextField, MenuItem, Switch, FormControlLabel, Tabs, Tab, List, ListItem, ListItemText, ListItemIcon, LinearProgress, Slider, FormControl, InputLabel, Select, } from '@mui/material';
import { NetworkCheck as NetworkIcon, Security as SecurityIcon, Add as AddIcon, Edit as EditIcon, Delete as DeleteIcon, Refresh as RefreshIcon, PlayArrow as PlayIcon, Pause as PauseIcon, Error as ErrorIcon, Timeline as TimelineIcon, Storage as StorageIcon, Speed as SpeedIcon, } from '@mui/icons-material';
import { XAxis, YAxis, CartesianGrid, Tooltip, ResponsiveContainer, PieChart, Pie, Cell, AreaChart, Area, } from 'recharts';
const NetworkImpairments = () => {
    const [impairments, setImpairments] = useState([
        {
            id: '1',
            name: 'High Latency Simulation',
            type: 'delay',
            interface: 'eth0',
            enabled: true,
            parameters: { delay: 100, jitter: 20, distribution: 'normal' },
            status: 'active',
            packets: 12500,
            affected: 12500,
            rate: 100,
        },
        {
            id: '2',
            name: 'Packet Loss Simulation',
            type: 'loss',
            interface: 'eth1',
            enabled: true,
            parameters: { loss: 5, correlation: 0.1, distribution: 'bernoulli' },
            status: 'active',
            packets: 8500,
            affected: 425,
            rate: 5,
        },
        {
            id: '3',
            name: 'Bandwidth Limitation',
            type: 'bandwidth',
            interface: 'eth2',
            enabled: true,
            parameters: { bandwidth: 100, buffer: 1000, queue: 10 },
            status: 'active',
            packets: 6200,
            affected: 6200,
            rate: 100,
        },
        {
            id: '4',
            name: 'Packet Duplication',
            type: 'duplicate',
            interface: 'eth3',
            enabled: false,
            parameters: { duplicate: 2, correlation: 0.05 },
            status: 'inactive',
            packets: 4800,
            affected: 0,
            rate: 0,
        },
        {
            id: '5',
            name: 'Packet Reordering',
            type: 'reorder',
            interface: 'eth4',
            enabled: false,
            parameters: { reorder: 10, gap: 5, correlation: 0.1 },
            status: 'inactive',
            packets: 3200,
            affected: 0,
            rate: 0,
        },
        {
            id: '6',
            name: 'Packet Corruption',
            type: 'corrupt',
            interface: 'eth5',
            enabled: false,
            parameters: { corrupt: 1, correlation: 0.02, pattern: 'random' },
            status: 'inactive',
            packets: 2100,
            affected: 0,
            rate: 0,
        },
    ]);
    const [currentTab, setCurrentTab] = useState(0);
    const [openDialog, setOpenDialog] = useState(false);
    const [selectedImpairment, setSelectedImpairment] = useState(null);
    const [simulationRunning, setSimulationRunning] = useState(false);
    const [loading, setLoading] = useState(false);
    const [impairmentData, setImpairmentData] = useState([
        { time: '00:00', delay: 15, loss: 2, duplicate: 0, reorder: 0, corrupt: 0 },
        { time: '01:00', delay: 18, loss: 3, duplicate: 1, reorder: 0, corrupt: 0 },
        { time: '02:00', delay: 12, loss: 1, duplicate: 0, reorder: 0, corrupt: 0 },
        { time: '03:00', delay: 22, loss: 4, duplicate: 2, reorder: 1, corrupt: 0 },
        { time: '04:00', delay: 25, loss: 5, duplicate: 1, reorder: 0, corrupt: 1 },
        { time: '05:00', delay: 28, loss: 6, duplicate: 3, reorder: 2, corrupt: 1 },
    ]);
    const [typeDistribution, setTypeDistribution] = useState([
        { name: 'Delay', value: 35, color: '#1976d2' },
        { name: 'Loss', value: 25, color: '#dc004e' },
        { name: 'Bandwidth', value: 20, color: '#2e7d32' },
        { name: 'Duplicate', value: 10, color: '#ff9800' },
        { name: 'Reorder', value: 5, color: '#9c27b0' },
        { name: 'Corrupt', value: 5, color: '#f44336' },
    ]);
    const getTypeIcon = (type) => {
        switch (type) {
            case 'delay':
                return _jsx(TimelineIcon, { color: "primary" });
            case 'loss':
                return _jsx(ErrorIcon, { color: "error" });
            case 'duplicate':
                return _jsx(StorageIcon, { color: "warning" });
            case 'reorder':
                return _jsx(NetworkIcon, { color: "secondary" });
            case 'corrupt':
                return _jsx(SecurityIcon, { color: "error" });
            case 'bandwidth':
                return _jsx(SpeedIcon, { color: "success" });
            default:
                return _jsx(NetworkIcon, {});
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
    const getTypeColor = (type) => {
        switch (type) {
            case 'delay':
                return '#1976d2';
            case 'loss':
                return '#dc004e';
            case 'duplicate':
                return '#ff9800';
            case 'reorder':
                return '#9c27b0';
            case 'corrupt':
                return '#f44336';
            case 'bandwidth':
                return '#2e7d32';
            default:
                return '#666';
        }
    };
    const handleRefresh = async () => {
        setLoading(true);
        try {
            // Simulate API call
            await new Promise(resolve => setTimeout(resolve, 1000));
            // Update data here
        }
        catch (error) {
            console.error('Failed to refresh network impairments data:', error);
        }
        finally {
            setLoading(false);
        }
    };
    const handleEditImpairment = (impairment) => {
        setSelectedImpairment(impairment);
        setOpenDialog(true);
    };
    const handleDeleteImpairment = async (impairmentId) => {
        if (window.confirm('Are you sure you want to delete this impairment?')) {
            try {
                // Simulate API call
                await new Promise(resolve => setTimeout(resolve, 500));
                setImpairments(prev => prev.filter(i => i.id !== impairmentId));
            }
            catch (error) {
                console.error('Failed to delete impairment:', error);
            }
        }
    };
    const handleToggleImpairment = async (impairmentId) => {
        try {
            // Simulate API call
            await new Promise(resolve => setTimeout(resolve, 500));
            setImpairments(prev => prev.map(i => i.id === impairmentId
                ? {
                    ...i,
                    enabled: !i.enabled,
                    status: !i.enabled ? 'active' : 'inactive',
                    rate: !i.enabled ? (i.type === 'delay' ? 100 : i.type === 'loss' ? 5 : i.type === 'bandwidth' ? 100 : 0) : 0
                }
                : i));
        }
        catch (error) {
            console.error('Failed to toggle impairment:', error);
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
                setImpairments(prev => prev.map(impairment => ({
                    ...impairment,
                    packets: impairment.packets + Math.floor(Math.random() * 100),
                    affected: impairment.enabled ? impairment.affected + Math.floor(Math.random() * 10) : impairment.affected
                })));
            }, 2000);
            return () => clearInterval(interval);
        }
    }, [simulationRunning]);
    return (_jsxs(Box, { children: [_jsxs(Box, { sx: { display: 'flex', justifyContent: 'space-between', alignItems: 'center', mb: 3 }, children: [_jsx(Typography, { variant: "h4", component: "h1", children: "Network Impairments" }), _jsxs(Box, { children: [_jsx(Button, { variant: "outlined", startIcon: _jsx(RefreshIcon, {}), onClick: handleRefresh, disabled: loading, sx: { mr: 1 }, children: "Refresh" }), !simulationRunning ? (_jsx(Button, { variant: "contained", startIcon: _jsx(PlayIcon, {}), onClick: handleStartSimulation, children: "Start Simulation" })) : (_jsx(Button, { variant: "contained", color: "error", startIcon: _jsx(PauseIcon, {}), onClick: handleStopSimulation, children: "Stop Simulation" }))] })] }), loading && _jsx(LinearProgress, { sx: { mb: 2 } }), _jsx(Paper, { sx: { mb: 3 }, children: _jsxs(Tabs, { value: currentTab, onChange: (e, v) => setCurrentTab(v), children: [_jsx(Tab, { label: "Impairments" }), _jsx(Tab, { label: "Analytics" }), _jsx(Tab, { label: "Configuration" })] }) }), currentTab === 0 && (_jsx(Grid, { container: true, spacing: 3, children: _jsx(Grid, { item: true, xs: 12, children: _jsx(Card, { children: _jsxs(CardContent, { children: [_jsxs(Box, { sx: { display: 'flex', justifyContent: 'space-between', alignItems: 'center', mb: 2 }, children: [_jsx(Typography, { variant: "h6", children: "Network Impairments" }), _jsx(Button, { variant: "contained", startIcon: _jsx(AddIcon, {}), onClick: () => setOpenDialog(true), children: "Add Impairment" })] }), _jsx(TableContainer, { component: Paper, sx: { backgroundColor: 'transparent' }, children: _jsxs(Table, { children: [_jsx(TableHead, { children: _jsxs(TableRow, { children: [_jsx(TableCell, { children: "Name" }), _jsx(TableCell, { children: "Type" }), _jsx(TableCell, { children: "Interface" }), _jsx(TableCell, { children: "Enabled" }), _jsx(TableCell, { children: "Status" }), _jsx(TableCell, { children: "Packets" }), _jsx(TableCell, { children: "Affected" }), _jsx(TableCell, { children: "Rate" }), _jsx(TableCell, { children: "Parameters" }), _jsx(TableCell, { children: "Actions" })] }) }), _jsx(TableBody, { children: impairments.map((impairment) => (_jsxs(TableRow, { children: [_jsx(TableCell, { children: _jsxs(Box, { sx: { display: 'flex', alignItems: 'center' }, children: [getTypeIcon(impairment.type), _jsx(Typography, { variant: "body2", sx: { ml: 1 }, children: impairment.name })] }) }), _jsx(TableCell, { children: _jsx(Chip, { label: impairment.type.toUpperCase(), size: "small", sx: { backgroundColor: getTypeColor(impairment.type), color: 'white' } }) }), _jsx(TableCell, { children: _jsx(Typography, { variant: "body2", fontFamily: "monospace", children: impairment.interface }) }), _jsx(TableCell, { children: _jsx(Switch, { checked: impairment.enabled, onChange: () => handleToggleImpairment(impairment.id) }) }), _jsx(TableCell, { children: _jsx(Chip, { label: impairment.status, color: getStatusColor(impairment.status), size: "small" }) }), _jsx(TableCell, { children: impairment.packets.toLocaleString() }), _jsx(TableCell, { children: _jsx(Chip, { label: impairment.affected.toLocaleString(), color: impairment.affected > 0 ? 'error' : 'default', size: "small" }) }), _jsx(TableCell, { children: _jsxs(Typography, { variant: "body2", children: [impairment.rate, "%"] }) }), _jsx(TableCell, { children: _jsx(Typography, { variant: "body2", fontFamily: "monospace", children: Object.entries(impairment.parameters).map(([key, value]) => `${key}: ${value}`).join(', ') }) }), _jsxs(TableCell, { children: [_jsx(IconButton, { size: "small", onClick: () => handleEditImpairment(impairment), children: _jsx(EditIcon, {}) }), _jsx(IconButton, { size: "small", onClick: () => handleDeleteImpairment(impairment.id), color: "error", children: _jsx(DeleteIcon, {}) })] })] }, impairment.id))) })] }) })] }) }) }) })), currentTab === 1 && (_jsxs(Grid, { container: true, spacing: 3, children: [_jsx(Grid, { item: true, xs: 12, md: 8, children: _jsx(Card, { children: _jsxs(CardContent, { children: [_jsx(Typography, { variant: "h6", gutterBottom: true, children: "Impairment Analytics" }), _jsx(ResponsiveContainer, { width: "100%", height: 400, children: _jsxs(AreaChart, { data: impairmentData, children: [_jsx(CartesianGrid, { strokeDasharray: "3 3" }), _jsx(XAxis, { dataKey: "time" }), _jsx(YAxis, {}), _jsx(Tooltip, {}), _jsx(Area, { type: "monotone", dataKey: "delay", stackId: "1", stroke: "#1976d2", fill: "#1976d2", fillOpacity: 0.3, name: "Delay (ms)" }), _jsx(Area, { type: "monotone", dataKey: "loss", stackId: "1", stroke: "#dc004e", fill: "#dc004e", fillOpacity: 0.3, name: "Loss %" }), _jsx(Area, { type: "monotone", dataKey: "duplicate", stackId: "1", stroke: "#ff9800", fill: "#ff9800", fillOpacity: 0.3, name: "Duplicate %" }), _jsx(Area, { type: "monotone", dataKey: "reorder", stackId: "1", stroke: "#9c27b0", fill: "#9c27b0", fillOpacity: 0.3, name: "Reorder %" }), _jsx(Area, { type: "monotone", dataKey: "corrupt", stackId: "1", stroke: "#f44336", fill: "#f44336", fillOpacity: 0.3, name: "Corrupt %" })] }) })] }) }) }), _jsx(Grid, { item: true, xs: 12, md: 4, children: _jsx(Card, { children: _jsxs(CardContent, { children: [_jsx(Typography, { variant: "h6", gutterBottom: true, children: "Impairment Types" }), _jsx(ResponsiveContainer, { width: "100%", height: 300, children: _jsxs(PieChart, { children: [_jsx(Pie, { data: typeDistribution, cx: "50%", cy: "50%", innerRadius: 60, outerRadius: 100, paddingAngle: 5, dataKey: "value", children: typeDistribution.map((entry, index) => (_jsx(Cell, { fill: entry.color }, `cell-${index}`))) }), _jsx(Tooltip, {})] }) })] }) }) }), _jsx(Grid, { item: true, xs: 12, children: _jsx(Card, { children: _jsxs(CardContent, { children: [_jsx(Typography, { variant: "h6", gutterBottom: true, children: "Impairment Statistics" }), _jsxs(Grid, { container: true, spacing: 2, children: [_jsx(Grid, { item: true, xs: 12, md: 3, children: _jsxs(Box, { sx: { textAlign: 'center' }, children: [_jsx(Typography, { variant: "h4", color: "primary", children: impairments.filter(i => i.enabled).length }), _jsx(Typography, { variant: "body2", color: "text.secondary", children: "Active Impairments" })] }) }), _jsx(Grid, { item: true, xs: 12, md: 3, children: _jsxs(Box, { sx: { textAlign: 'center' }, children: [_jsx(Typography, { variant: "h4", color: "error", children: impairments.reduce((sum, i) => sum + i.affected, 0).toLocaleString() }), _jsx(Typography, { variant: "body2", color: "text.secondary", children: "Affected Packets" })] }) }), _jsx(Grid, { item: true, xs: 12, md: 3, children: _jsxs(Box, { sx: { textAlign: 'center' }, children: [_jsxs(Typography, { variant: "h4", color: "warning", children: [(impairments.reduce((sum, i) => sum + i.affected, 0) / impairments.reduce((sum, i) => sum + i.packets, 0) * 100).toFixed(1), "%"] }), _jsx(Typography, { variant: "body2", color: "text.secondary", children: "Impact Rate" })] }) }), _jsx(Grid, { item: true, xs: 12, md: 3, children: _jsxs(Box, { sx: { textAlign: 'center' }, children: [_jsx(Typography, { variant: "h4", color: "success", children: impairments.filter(i => i.type === 'delay').length }), _jsx(Typography, { variant: "body2", color: "text.secondary", children: "Delay Rules" })] }) })] })] }) }) })] })), currentTab === 2 && (_jsxs(Grid, { container: true, spacing: 3, children: [_jsx(Grid, { item: true, xs: 12, md: 6, children: _jsx(Card, { children: _jsxs(CardContent, { children: [_jsx(Typography, { variant: "h6", gutterBottom: true, children: "Impairment Configuration" }), _jsx(List, { children: impairments.map((impairment) => (_jsxs(ListItem, { children: [_jsx(ListItemIcon, { children: getTypeIcon(impairment.type) }), _jsx(ListItemText, { primary: impairment.name, secondary: _jsxs(Box, { children: [_jsxs(Typography, { variant: "body2", children: ["Interface: ", impairment.interface, " | Status: ", impairment.status, " | Rate: ", impairment.rate, "%"] }), _jsxs(Typography, { variant: "body2", fontFamily: "monospace", children: ["Parameters: ", Object.entries(impairment.parameters).map(([key, value]) => `${key}: ${value}`).join(', ')] })] }) })] }, impairment.id))) })] }) }) }), _jsx(Grid, { item: true, xs: 12, md: 6, children: _jsx(Card, { children: _jsxs(CardContent, { children: [_jsx(Typography, { variant: "h6", gutterBottom: true, children: "Global Settings" }), _jsxs(Box, { sx: { display: 'flex', flexDirection: 'column', gap: 2 }, children: [_jsx(FormControlLabel, { control: _jsx(Switch, { defaultChecked: true }), label: "Enable All Impairments" }), _jsx(FormControlLabel, { control: _jsx(Switch, { defaultChecked: true }), label: "Auto-apply Changes" }), _jsx(FormControlLabel, { control: _jsx(Switch, {}), label: "Log Impairment Events" }), _jsxs(Box, { children: [_jsx(Typography, { gutterBottom: true, children: "Simulation Speed" }), _jsx(Slider, { defaultValue: 1, min: 0.1, max: 5, step: 0.1, marks: [
                                                            { value: 0.1, label: 'Slow' },
                                                            { value: 1, label: 'Normal' },
                                                            { value: 5, label: 'Fast' },
                                                        ] })] })] })] }) }) })] })), _jsxs(Dialog, { open: openDialog, onClose: () => setOpenDialog(false), maxWidth: "sm", fullWidth: true, children: [_jsx(DialogTitle, { children: selectedImpairment ? 'Edit Impairment' : 'Add Impairment' }), _jsx(DialogContent, { children: _jsxs(Box, { sx: { display: 'flex', flexDirection: 'column', gap: 2, mt: 1 }, children: [_jsx(TextField, { label: "Impairment Name", defaultValue: selectedImpairment?.name || '', fullWidth: true }), _jsxs(FormControl, { fullWidth: true, children: [_jsx(InputLabel, { children: "Type" }), _jsxs(Select, { defaultValue: selectedImpairment?.type || '', label: "Type", children: [_jsx(MenuItem, { value: "delay", children: "Delay" }), _jsx(MenuItem, { value: "loss", children: "Loss" }), _jsx(MenuItem, { value: "duplicate", children: "Duplicate" }), _jsx(MenuItem, { value: "reorder", children: "Reorder" }), _jsx(MenuItem, { value: "corrupt", children: "Corrupt" }), _jsx(MenuItem, { value: "bandwidth", children: "Bandwidth" })] })] }), _jsx(TextField, { label: "Interface", defaultValue: selectedImpairment?.interface || '', fullWidth: true }), _jsx(TextField, { label: "Rate (%)", type: "number", defaultValue: selectedImpairment?.rate || '', fullWidth: true }), _jsx(FormControlLabel, { control: _jsx(Switch, { defaultChecked: selectedImpairment?.enabled || false }), label: "Enabled" })] }) }), _jsxs(DialogActions, { children: [_jsx(Button, { onClick: () => setOpenDialog(false), children: "Cancel" }), _jsx(Button, { variant: "contained", children: "Save" })] })] })] }));
};
export default NetworkImpairments;
