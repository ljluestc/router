import { jsx as _jsx, jsxs as _jsxs, Fragment as _Fragment } from "react/jsx-runtime";
import { useState, useEffect } from 'react';
import { Box, Paper, Typography, Grid, Card, CardContent, FormControl, InputLabel, Select, MenuItem, Button, Chip, Alert, CircularProgress, Tabs, Tab, } from '@mui/material';
import { LineChart, Line, AreaChart, Area, BarChart, Bar, XAxis, YAxis, CartesianGrid, Tooltip, Legend, ResponsiveContainer, PieChart, Pie, Cell, ScatterChart, Scatter, } from 'recharts';
import { TrendingUp, TrendingDown, Speed, Memory, NetworkCheck, Router, Cloud, Hub, } from '@mui/icons-material';
import { useAnalytics } from '../hooks/useAnalytics';
import { useWebSocket } from '../hooks/useWebSocket';
const Analytics = () => {
    const [tabValue, setTabValue] = useState(0);
    const [timeRange, setTimeRange] = useState('1h');
    const [selectedRouter, setSelectedRouter] = useState('all');
    const [selectedMetric, setSelectedMetric] = useState('bandwidth');
    const [isLoading, setIsLoading] = useState(false);
    const [error, setError] = useState(null);
    const [dashboardMetrics, setDashboardMetrics] = useState(null);
    const [bandwidthData, setBandwidthData] = useState([]);
    const [latencyData, setLatencyData] = useState([]);
    const [packetLossData, setPacketLossData] = useState([]);
    const [bgpData, setBgpData] = useState([]);
    const [ospfData, setOspfData] = useState([]);
    const [isisData, setIsisData] = useState([]);
    const [trafficShapingData, setTrafficShapingData] = useState([]);
    const [netemData, setNetemData] = useState([]);
    const { getMetrics, getDashboardData, getProtocolMetrics, getTrafficShapingMetrics, getNetemMetrics } = useAnalytics();
    const { data: realtimeData, isConnected } = useWebSocket('/ws');
    useEffect(() => {
        loadDashboardData();
    }, [timeRange, selectedRouter]);
    useEffect(() => {
        if (realtimeData && realtimeData.type === 'metrics_update') {
            updateRealtimeMetrics(realtimeData.data);
        }
    }, [realtimeData]);
    const loadDashboardData = async () => {
        setIsLoading(true);
        setError(null);
        try {
            const [dashboard, metrics, protocols, traffic, netem] = await Promise.all([
                getDashboardData(timeRange, selectedRouter),
                getMetrics('bandwidth', timeRange, selectedRouter),
                getProtocolMetrics(timeRange, selectedRouter),
                getTrafficShapingMetrics(timeRange, selectedRouter),
                getNetemMetrics(timeRange, selectedRouter),
            ]);
            setDashboardMetrics(dashboard);
            setBandwidthData(metrics.bandwidth || []);
            setLatencyData(metrics.latency || []);
            setPacketLossData(metrics.packetLoss || []);
            setBgpData(protocols.bgp || []);
            setOspfData(protocols.ospf || []);
            setIsisData(protocols.isis || []);
            setTrafficShapingData(traffic || []);
            setNetemData(netem || []);
        }
        catch (err) {
            setError('Failed to load analytics data');
            console.error('Analytics error:', err);
        }
        finally {
            setIsLoading(false);
        }
    };
    const updateRealtimeMetrics = (data) => {
        // Update real-time metrics based on WebSocket data
        if (data.bandwidth) {
            setBandwidthData(prev => [...prev.slice(-99), data.bandwidth]);
        }
        if (data.latency) {
            setLatencyData(prev => [...prev.slice(-99), data.latency]);
        }
        if (data.packetLoss) {
            setPacketLossData(prev => [...prev.slice(-99), data.packetLoss]);
        }
    };
    const getMetricIcon = (metric) => {
        switch (metric) {
            case 'bandwidth':
                return _jsx(Speed, {});
            case 'latency':
                return _jsx(NetworkCheck, {});
            case 'cpu':
                return _jsx(Memory, {});
            case 'routers':
                return _jsx(Router, {});
            case 'clouds':
                return _jsx(Cloud, {});
            case 'gateways':
                return _jsx(Hub, {});
            default:
                return _jsx(TrendingUp, {});
        }
    };
    const getMetricColor = (metric) => {
        switch (metric) {
            case 'bandwidth':
                return '#2196f3';
            case 'latency':
                return '#ff9800';
            case 'packetLoss':
                return '#f44336';
            case 'bgp':
                return '#4caf50';
            case 'ospf':
                return '#9c27b0';
            case 'isis':
                return '#ff5722';
            default:
                return '#9e9e9e';
        }
    };
    const formatValue = (value, metric) => {
        switch (metric) {
            case 'bandwidth':
                return `${(value / 1000000).toFixed(2)} Mbps`;
            case 'latency':
                return `${value.toFixed(2)} ms`;
            case 'packetLoss':
                return `${value.toFixed(3)}%`;
            case 'convergence':
                return `${value.toFixed(2)}s`;
            default:
                return value.toLocaleString();
        }
    };
    const renderMetricCard = (title, value, metric, trend) => (_jsx(Card, { children: _jsx(CardContent, { children: _jsxs(Box, { display: "flex", alignItems: "center", justifyContent: "space-between", children: [_jsxs(Box, { children: [_jsx(Typography, { color: "textSecondary", gutterBottom: true, variant: "body2", children: title }), _jsx(Typography, { variant: "h4", component: "div", children: formatValue(value, metric) }), trend !== undefined && (_jsxs(Box, { display: "flex", alignItems: "center", mt: 1, children: [trend > 0 ? _jsx(TrendingUp, { color: "success" }) : _jsx(TrendingDown, { color: "error" }), _jsxs(Typography, { variant: "body2", color: trend > 0 ? 'success.main' : 'error.main', children: [Math.abs(trend).toFixed(1), "%"] })] }))] }), _jsx(Box, { color: getMetricColor(metric), children: getMetricIcon(metric) })] }) }) }));
    const renderLineChart = (data, title, metric) => (_jsx(Card, { children: _jsxs(CardContent, { children: [_jsx(Typography, { variant: "h6", gutterBottom: true, children: title }), _jsx(ResponsiveContainer, { width: "100%", height: 300, children: _jsxs(LineChart, { data: data, children: [_jsx(CartesianGrid, { strokeDasharray: "3 3" }), _jsx(XAxis, { dataKey: "timestamp" }), _jsx(YAxis, {}), _jsx(Tooltip, { formatter: (value) => [formatValue(value, metric), title] }), _jsx(Legend, {}), _jsx(Line, { type: "monotone", dataKey: "value", stroke: getMetricColor(metric), strokeWidth: 2, dot: { r: 4 } })] }) })] }) }));
    const renderAreaChart = (data, title, metric) => (_jsx(Card, { children: _jsxs(CardContent, { children: [_jsx(Typography, { variant: "h6", gutterBottom: true, children: title }), _jsx(ResponsiveContainer, { width: "100%", height: 300, children: _jsxs(AreaChart, { data: data, children: [_jsx(CartesianGrid, { strokeDasharray: "3 3" }), _jsx(XAxis, { dataKey: "timestamp" }), _jsx(YAxis, {}), _jsx(Tooltip, { formatter: (value) => [formatValue(value, metric), title] }), _jsx(Legend, {}), _jsx(Area, { type: "monotone", dataKey: "value", stroke: getMetricColor(metric), fill: getMetricColor(metric), fillOpacity: 0.3 })] }) })] }) }));
    const renderProtocolComparison = () => {
        const protocolData = [
            { name: 'BGP', value: dashboardMetrics?.bgpConvergence || 0, color: getMetricColor('bgp') },
            { name: 'OSPF', value: dashboardMetrics?.ospfConvergence || 0, color: getMetricColor('ospf') },
            { name: 'IS-IS', value: dashboardMetrics?.isisConvergence || 0, color: getMetricColor('isis') },
        ];
        return (_jsx(Card, { children: _jsxs(CardContent, { children: [_jsx(Typography, { variant: "h6", gutterBottom: true, children: "Protocol Convergence Times" }), _jsx(ResponsiveContainer, { width: "100%", height: 300, children: _jsxs(PieChart, { children: [_jsx(Pie, { data: protocolData, cx: "50%", cy: "50%", labelLine: false, label: ({ name, value }) => `${name}: ${value.toFixed(2)}s`, outerRadius: 80, fill: "#8884d8", dataKey: "value", children: protocolData.map((entry, index) => (_jsx(Cell, { fill: entry.color }, `cell-${index}`))) }), _jsx(Tooltip, { formatter: (value) => [`${value}s`, 'Convergence Time'] })] }) })] }) }));
    };
    const renderTrafficShapingAnalysis = () => (_jsx(Card, { children: _jsxs(CardContent, { children: [_jsx(Typography, { variant: "h6", gutterBottom: true, children: "Traffic Shaping Effectiveness" }), _jsx(ResponsiveContainer, { width: "100%", height: 300, children: _jsxs(BarChart, { data: trafficShapingData, children: [_jsx(CartesianGrid, { strokeDasharray: "3 3" }), _jsx(XAxis, { dataKey: "algorithm" }), _jsx(YAxis, {}), _jsx(Tooltip, {}), _jsx(Legend, {}), _jsx(Bar, { dataKey: "packetsProcessed", fill: getMetricColor('bandwidth'), name: "Packets Processed" }), _jsx(Bar, { dataKey: "packetsDropped", fill: getMetricColor('packetLoss'), name: "Packets Dropped" })] }) })] }) }));
    const renderNetemImpact = () => (_jsx(Card, { children: _jsxs(CardContent, { children: [_jsx(Typography, { variant: "h6", gutterBottom: true, children: "Network Impairment Impact" }), _jsx(ResponsiveContainer, { width: "100%", height: 300, children: _jsxs(ScatterChart, { data: netemData, children: [_jsx(CartesianGrid, { strokeDasharray: "3 3" }), _jsx(XAxis, { dataKey: "latency", name: "Latency (ms)" }), _jsx(YAxis, { dataKey: "packetLoss", name: "Packet Loss (%)" }), _jsx(Tooltip, { cursor: { strokeDasharray: '3 3' } }), _jsx(Scatter, { dataKey: "throughput", fill: getMetricColor('bandwidth') })] }) })] }) }));
    if (isLoading) {
        return (_jsx(Box, { display: "flex", justifyContent: "center", alignItems: "center", height: "100vh", children: _jsx(CircularProgress, {}) }));
    }
    return (_jsxs(Box, { sx: { p: 3 }, children: [_jsx(Typography, { variant: "h4", gutterBottom: true, children: "Network Analytics" }), _jsx(Paper, { sx: { p: 2, mb: 3 }, children: _jsxs(Grid, { container: true, spacing: 2, alignItems: "center", children: [_jsx(Grid, { item: true, xs: 12, sm: 3, children: _jsxs(FormControl, { fullWidth: true, size: "small", children: [_jsx(InputLabel, { children: "Time Range" }), _jsxs(Select, { value: timeRange, onChange: (e) => setTimeRange(e.target.value), label: "Time Range", children: [_jsx(MenuItem, { value: "1h", children: "Last Hour" }), _jsx(MenuItem, { value: "6h", children: "Last 6 Hours" }), _jsx(MenuItem, { value: "24h", children: "Last 24 Hours" }), _jsx(MenuItem, { value: "7d", children: "Last 7 Days" }), _jsx(MenuItem, { value: "30d", children: "Last 30 Days" })] })] }) }), _jsx(Grid, { item: true, xs: 12, sm: 3, children: _jsxs(FormControl, { fullWidth: true, size: "small", children: [_jsx(InputLabel, { children: "Router" }), _jsxs(Select, { value: selectedRouter, onChange: (e) => setSelectedRouter(e.target.value), label: "Router", children: [_jsx(MenuItem, { value: "all", children: "All Routers" }), _jsx(MenuItem, { value: "router-1", children: "Router-1" }), _jsx(MenuItem, { value: "router-2", children: "Router-2" }), _jsx(MenuItem, { value: "aviatrix-gw-1", children: "Aviatrix GW-1" })] })] }) }), _jsx(Grid, { item: true, xs: 12, sm: 3, children: _jsxs(FormControl, { fullWidth: true, size: "small", children: [_jsx(InputLabel, { children: "Metric" }), _jsxs(Select, { value: selectedMetric, onChange: (e) => setSelectedMetric(e.target.value), label: "Metric", children: [_jsx(MenuItem, { value: "bandwidth", children: "Bandwidth" }), _jsx(MenuItem, { value: "latency", children: "Latency" }), _jsx(MenuItem, { value: "packetLoss", children: "Packet Loss" }), _jsx(MenuItem, { value: "convergence", children: "Convergence" })] })] }) }), _jsx(Grid, { item: true, xs: 12, sm: 3, children: _jsx(Button, { variant: "contained", onClick: loadDashboardData, fullWidth: true, disabled: isLoading, children: "Refresh" }) })] }) }), error && (_jsx(Alert, { severity: "error", sx: { mb: 3 }, onClose: () => setError(null), children: error })), _jsx(Paper, { sx: { mb: 3 }, children: _jsxs(Tabs, { value: tabValue, onChange: (_, value) => setTabValue(value), children: [_jsx(Tab, { label: "Overview" }), _jsx(Tab, { label: "Protocols" }), _jsx(Tab, { label: "Traffic Shaping" }), _jsx(Tab, { label: "Network Impairments" })] }) }), tabValue === 0 && (_jsxs(_Fragment, { children: [_jsxs(Grid, { container: true, spacing: 3, sx: { mb: 3 }, children: [_jsx(Grid, { item: true, xs: 12, sm: 6, md: 3, children: renderMetricCard('Total Routers', dashboardMetrics?.totalRouters || 0, 'routers') }), _jsx(Grid, { item: true, xs: 12, sm: 6, md: 3, children: renderMetricCard('Active Connections', dashboardMetrics?.activeConnections || 0, 'connections') }), _jsx(Grid, { item: true, xs: 12, sm: 6, md: 3, children: renderMetricCard('Total Bandwidth', dashboardMetrics?.totalBandwidth || 0, 'bandwidth') }), _jsx(Grid, { item: true, xs: 12, sm: 6, md: 3, children: renderMetricCard('Average Latency', dashboardMetrics?.averageLatency || 0, 'latency') })] }), _jsxs(Grid, { container: true, spacing: 3, children: [_jsx(Grid, { item: true, xs: 12, md: 6, children: renderLineChart(bandwidthData, 'Bandwidth Utilization', 'bandwidth') }), _jsx(Grid, { item: true, xs: 12, md: 6, children: renderAreaChart(latencyData, 'Latency Trends', 'latency') }), _jsx(Grid, { item: true, xs: 12, md: 6, children: renderLineChart(packetLossData, 'Packet Loss Rate', 'packetLoss') }), _jsx(Grid, { item: true, xs: 12, md: 6, children: renderProtocolComparison() })] })] })), tabValue === 1 && (_jsxs(Grid, { container: true, spacing: 3, children: [_jsx(Grid, { item: true, xs: 12, md: 6, children: renderLineChart(bgpData, 'BGP Convergence', 'bgp') }), _jsx(Grid, { item: true, xs: 12, md: 6, children: renderLineChart(ospfData, 'OSPF Convergence', 'ospf') }), _jsx(Grid, { item: true, xs: 12, md: 6, children: renderLineChart(isisData, 'IS-IS Convergence', 'isis') }), _jsx(Grid, { item: true, xs: 12, md: 6, children: renderProtocolComparison() })] })), tabValue === 2 && (_jsx(Grid, { container: true, spacing: 3, children: _jsx(Grid, { item: true, xs: 12, children: renderTrafficShapingAnalysis() }) })), tabValue === 3 && (_jsx(Grid, { container: true, spacing: 3, children: _jsx(Grid, { item: true, xs: 12, children: renderNetemImpact() }) })), _jsxs(Box, { sx: { mt: 3, display: 'flex', alignItems: 'center', gap: 1 }, children: [_jsx(Chip, { label: isConnected ? 'Connected' : 'Disconnected', color: isConnected ? 'success' : 'error', size: "small" }), _jsxs(Typography, { variant: "body2", color: "textSecondary", children: ["Real-time data ", isConnected ? 'enabled' : 'disabled'] })] })] }));
};
export default Analytics;
