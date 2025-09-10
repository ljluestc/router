import { jsx as _jsx, jsxs as _jsxs } from "react/jsx-runtime";
import { useState, useEffect, useCallback } from 'react';
import { Box, Paper, Typography, Button, IconButton, Switch, FormControlLabel, Slider, Select, MenuItem, FormControl, InputLabel, Chip, Alert, CircularProgress, } from '@mui/material';
import { PlayArrow, Stop, Refresh, Cloud, Router, Hub, DeviceHub, } from '@mui/icons-material';
import ReactFlow, { Controls, Background, useNodesState, useEdgesState, addEdge, MarkerType, } from 'reactflow';
import 'reactflow/dist/style.css';
import { useWebSocket } from '../hooks/useWebSocket';
import { useAnalytics } from '../hooks/useAnalytics';
const NetworkTopology = () => {
    const [nodes, setNodes, onNodesChange] = useNodesState([]);
    const [edges, setEdges, onEdgesChange] = useEdgesState([]);
    const [isSimulating, setIsSimulating] = useState(false);
    const [simulationSpeed, setSimulationSpeed] = useState(1);
    const [showMetrics, setShowMetrics] = useState(true);
    const [selectedProtocol, setSelectedProtocol] = useState('all');
    const [selectedCloud, setSelectedCloud] = useState('all');
    const [isLoading, setIsLoading] = useState(false);
    const [error, setError] = useState(null);
    const { data: realtimeData, isConnected } = useWebSocket('/ws');
    const { getTopologyData, getMetrics } = useAnalytics();
    // Initialize topology with sample data
    useEffect(() => {
        initializeTopology();
    }, []);
    // Update topology based on real-time data
    useEffect(() => {
        if (realtimeData && realtimeData.type === 'topology_update') {
            updateTopology(realtimeData.data);
        }
    }, [realtimeData]);
    const initializeTopology = async () => {
        setIsLoading(true);
        try {
            const topologyData = await getTopologyData();
            if (topologyData) {
                setNodes(topologyData.nodes);
                setEdges(topologyData.edges);
            }
            else {
                // Fallback to sample data
                setNodes(getSampleNodes());
                setEdges(getSampleEdges());
            }
        }
        catch (err) {
            setError('Failed to load topology data');
            // Use sample data as fallback
            setNodes(getSampleNodes());
            setEdges(getSampleEdges());
        }
        finally {
            setIsLoading(false);
        }
    };
    const getSampleNodes = () => [
        {
            id: 'cloudpods-1',
            type: 'cloud',
            position: { x: 100, y: 100 },
            data: {
                label: 'CloudPods DC1',
                type: 'cloud',
                status: 'active',
                metrics: { cpu: 45, memory: 67, bandwidth: 85, latency: 12 },
                properties: { region: 'us-west-1', provider: 'CloudPods' },
            },
        },
        {
            id: 'aviatrix-gw-1',
            type: 'gateway',
            position: { x: 300, y: 100 },
            data: {
                label: 'Aviatrix GW1',
                type: 'gateway',
                status: 'active',
                metrics: { cpu: 23, memory: 45, bandwidth: 92, latency: 8 },
                properties: { asn: 65001, region: 'us-west-1' },
            },
        },
        {
            id: 'router-1',
            type: 'router',
            position: { x: 500, y: 100 },
            data: {
                label: 'Router-1',
                type: 'router',
                status: 'active',
                metrics: { cpu: 34, memory: 56, bandwidth: 78, latency: 5 },
                properties: { protocols: ['BGP', 'OSPF'], asn: 65002 },
            },
        },
        {
            id: 'router-2',
            type: 'router',
            position: { x: 700, y: 100 },
            data: {
                label: 'Router-2',
                type: 'router',
                status: 'active',
                metrics: { cpu: 28, memory: 43, bandwidth: 65, latency: 3 },
                properties: { protocols: ['BGP', 'ISIS'], asn: 65003 },
            },
        },
        {
            id: 'endpoint-1',
            type: 'endpoint',
            position: { x: 900, y: 100 },
            data: {
                label: 'Endpoint-1',
                type: 'endpoint',
                status: 'active',
                metrics: { cpu: 12, memory: 23, bandwidth: 45, latency: 1 },
                properties: { ip: '192.168.1.100' },
            },
        },
    ];
    const getSampleEdges = () => [
        {
            id: 'e1-2',
            source: 'cloudpods-1',
            target: 'aviatrix-gw-1',
            type: 'smoothstep',
            data: {
                bandwidth: 1000,
                latency: 15,
                utilization: 75,
                protocol: 'BGP',
                status: 'up',
            },
            markerEnd: { type: MarkerType.ArrowClosed },
        },
        {
            id: 'e2-3',
            source: 'aviatrix-gw-1',
            target: 'router-1',
            type: 'smoothstep',
            data: {
                bandwidth: 1000,
                latency: 8,
                utilization: 60,
                protocol: 'BGP',
                status: 'up',
            },
            markerEnd: { type: MarkerType.ArrowClosed },
        },
        {
            id: 'e3-4',
            source: 'router-1',
            target: 'router-2',
            type: 'smoothstep',
            data: {
                bandwidth: 1000,
                latency: 5,
                utilization: 45,
                protocol: 'OSPF',
                status: 'up',
            },
            markerEnd: { type: MarkerType.ArrowClosed },
        },
        {
            id: 'e4-5',
            source: 'router-2',
            target: 'endpoint-1',
            type: 'smoothstep',
            data: {
                bandwidth: 100,
                latency: 2,
                utilization: 30,
                protocol: 'ISIS',
                status: 'up',
            },
            markerEnd: { type: MarkerType.ArrowClosed },
        },
    ];
    const updateTopology = (data) => {
        if (data.nodes) {
            setNodes(data.nodes);
        }
        if (data.edges) {
            setEdges(data.edges);
        }
    };
    const handleStartSimulation = () => {
        setIsSimulating(true);
        // Start simulation logic here
    };
    const handleStopSimulation = () => {
        setIsSimulating(false);
        // Stop simulation logic here
    };
    const handleRefresh = () => {
        initializeTopology();
    };
    const onConnect = useCallback((params) => setEdges((eds) => addEdge(params, eds)), [setEdges]);
    const getNodeIcon = (type) => {
        switch (type) {
            case 'cloud':
                return _jsx(Cloud, {});
            case 'gateway':
                return _jsx(Hub, {});
            case 'router':
                return _jsx(Router, {});
            case 'endpoint':
                return _jsx(DeviceHub, {});
            default:
                return _jsx(Router, {});
        }
    };
    const getStatusColor = (status) => {
        switch (status) {
            case 'active':
                return '#4caf50';
            case 'inactive':
                return '#f44336';
            case 'error':
                return '#ff9800';
            default:
                return '#9e9e9e';
        }
    };
    const getEdgeColor = (status) => {
        switch (status) {
            case 'up':
                return '#4caf50';
            case 'down':
                return '#f44336';
            case 'degraded':
                return '#ff9800';
            default:
                return '#9e9e9e';
        }
    };
    const nodeTypes = {
        custom: ({ data }) => (_jsxs(Box, { sx: {
                padding: 2,
                border: `2px solid ${getStatusColor(data.status)}`,
                borderRadius: 2,
                backgroundColor: 'white',
                minWidth: 150,
                textAlign: 'center',
            }, children: [_jsxs(Box, { sx: { display: 'flex', alignItems: 'center', justifyContent: 'center', mb: 1 }, children: [getNodeIcon(data.type), _jsx(Typography, { variant: "subtitle2", sx: { ml: 1 }, children: data.label })] }), showMetrics && data.metrics && (_jsxs(Box, { sx: { mt: 1 }, children: [_jsxs(Typography, { variant: "caption", display: "block", children: ["CPU: ", data.metrics.cpu, "%"] }), _jsxs(Typography, { variant: "caption", display: "block", children: ["Memory: ", data.metrics.memory, "%"] }), _jsxs(Typography, { variant: "caption", display: "block", children: ["Bandwidth: ", data.metrics.bandwidth, "%"] }), _jsxs(Typography, { variant: "caption", display: "block", children: ["Latency: ", data.metrics.latency, "ms"] })] })), data.properties && (_jsx(Box, { sx: { mt: 1 }, children: Object.entries(data.properties).map(([key, value]) => (_jsx(Chip, { label: `${key}: ${value}`, size: "small", sx: { mr: 0.5, mb: 0.5 } }, key))) }))] })),
    };
    const edgeTypes = {
        custom: ({ data }) => (_jsx(Box, { sx: {
                position: 'relative',
                '&::before': {
                    content: '""',
                    position: 'absolute',
                    top: '50%',
                    left: '50%',
                    transform: 'translate(-50%, -50%)',
                    width: '100%',
                    height: '2px',
                    backgroundColor: getEdgeColor(data.status),
                    opacity: 0.7,
                },
            }, children: _jsxs(Box, { sx: {
                    position: 'absolute',
                    top: '50%',
                    left: '50%',
                    transform: 'translate(-50%, -50%)',
                    backgroundColor: 'white',
                    padding: '2px 8px',
                    borderRadius: 1,
                    fontSize: '0.75rem',
                    border: `1px solid ${getEdgeColor(data.status)}`,
                }, children: [data.protocol, " (", data.utilization, "%)"] }) })),
    };
    if (isLoading) {
        return (_jsx(Box, { display: "flex", justifyContent: "center", alignItems: "center", height: "100vh", children: _jsx(CircularProgress, {}) }));
    }
    return (_jsxs(Box, { sx: { height: '100vh', display: 'flex', flexDirection: 'column' }, children: [_jsxs(Paper, { sx: { p: 2, mb: 2 }, children: [_jsxs(Box, { display: "flex", justifyContent: "space-between", alignItems: "center", children: [_jsx(Typography, { variant: "h5", children: "Network Topology" }), _jsxs(Box, { display: "flex", gap: 2, alignItems: "center", children: [_jsx(FormControlLabel, { control: _jsx(Switch, { checked: showMetrics, onChange: (e) => setShowMetrics(e.target.checked) }), label: "Show Metrics" }), _jsxs(FormControl, { size: "small", sx: { minWidth: 120 }, children: [_jsx(InputLabel, { children: "Protocol" }), _jsxs(Select, { value: selectedProtocol, onChange: (e) => setSelectedProtocol(e.target.value), label: "Protocol", children: [_jsx(MenuItem, { value: "all", children: "All Protocols" }), _jsx(MenuItem, { value: "bgp", children: "BGP" }), _jsx(MenuItem, { value: "ospf", children: "OSPF" }), _jsx(MenuItem, { value: "isis", children: "IS-IS" })] })] }), _jsxs(FormControl, { size: "small", sx: { minWidth: 120 }, children: [_jsx(InputLabel, { children: "Cloud" }), _jsxs(Select, { value: selectedCloud, onChange: (e) => setSelectedCloud(e.target.value), label: "Cloud", children: [_jsx(MenuItem, { value: "all", children: "All Clouds" }), _jsx(MenuItem, { value: "cloudpods", children: "CloudPods" }), _jsx(MenuItem, { value: "aviatrix", children: "Aviatrix" })] })] }), _jsxs(Button, { variant: "contained", startIcon: isSimulating ? _jsx(Stop, {}) : _jsx(PlayArrow, {}), onClick: isSimulating ? handleStopSimulation : handleStartSimulation, color: isSimulating ? 'error' : 'primary', children: [isSimulating ? 'Stop' : 'Start', " Simulation"] }), _jsx(IconButton, { onClick: handleRefresh, children: _jsx(Refresh, {}) })] })] }), isSimulating && (_jsxs(Box, { sx: { mt: 2 }, children: [_jsxs(Typography, { variant: "body2", gutterBottom: true, children: ["Simulation Speed: ", simulationSpeed, "x"] }), _jsx(Slider, { value: simulationSpeed, onChange: (_, value) => setSimulationSpeed(value), min: 0.1, max: 10, step: 0.1, sx: { maxWidth: 200 } })] }))] }), error && (_jsx(Alert, { severity: "error", sx: { mb: 2 }, onClose: () => setError(null), children: error })), _jsx(Box, { sx: { flex: 1, position: 'relative' }, children: _jsxs(ReactFlow, { nodes: nodes, edges: edges, onNodesChange: onNodesChange, onEdgesChange: onEdgesChange, onConnect: onConnect, nodeTypes: nodeTypes, edgeTypes: edgeTypes, fitView: true, attributionPosition: "bottom-left", children: [_jsx(Controls, {}), _jsx(Background, {})] }) }), _jsx(Paper, { sx: { p: 1, mt: 2 }, children: _jsxs(Box, { display: "flex", justifyContent: "space-between", alignItems: "center", children: [_jsxs(Typography, { variant: "body2", children: ["Nodes: ", nodes.length, " | Edges: ", edges.length, " | Status: ", isConnected ? 'Connected' : 'Disconnected'] }), _jsxs(Box, { display: "flex", gap: 1, children: [_jsx(Chip, { icon: _jsx(Cloud, {}), label: "CloudPods", color: "primary", size: "small" }), _jsx(Chip, { icon: _jsx(Hub, {}), label: "Aviatrix", color: "secondary", size: "small" }), _jsx(Chip, { icon: _jsx(Router, {}), label: "Routers", color: "default", size: "small" })] })] }) })] }));
};
export default NetworkTopology;
