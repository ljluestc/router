import React, { useState, useEffect, useRef } from 'react';
import {
  Box,
  Card,
  CardContent,
  Typography,
  IconButton,
  Tooltip,
  Chip,
} from '@mui/material';
import {
  ZoomIn as ZoomInIcon,
  ZoomOut as ZoomOutIcon,
  Refresh as RefreshIcon,
  Fullscreen as FullscreenIcon,
} from '@mui/icons-material';
import * as d3 from 'd3';

interface NetworkNode {
  id: string;
  name: string;
  type: 'router' | 'switch' | 'host' | 'gateway';
  ip_address: string;
  status: 'up' | 'down';
  x?: number;
  y?: number;
}

interface NetworkLink {
  source: string;
  target: string;
  bandwidth: string;
  latency: string;
  utilization: number;
  status: 'up' | 'down';
}

interface TopologyData {
  nodes: NetworkNode[];
  links: NetworkLink[];
}

const Topology: React.FC = () => {
  const svgRef = useRef<SVGSVGElement>(null);
  const [topologyData, setTopologyData] = useState<TopologyData | null>(null);
  const [loading, setLoading] = useState(true);
  const [zoomLevel, setZoomLevel] = useState(1);

  useEffect(() => {
    fetchTopologyData();
    const interval = setInterval(fetchTopologyData, 10000); // Update every 10 seconds
    return () => clearInterval(interval);
  }, []);

  useEffect(() => {
    if (topologyData && svgRef.current) {
      renderTopology();
    }
  }, [topologyData]);

  const fetchTopologyData = async () => {
    try {
      const response = await fetch('/api/v1/monitoring/topology');
      const data = await response.json();
      setTopologyData(data.topology);
      setLoading(false);
    } catch (error) {
      console.error('Failed to fetch topology data:', error);
      setLoading(false);
    }
  };

  const renderTopology = () => {
    if (!topologyData || !svgRef.current) return;

    const svg = d3.select(svgRef.current);
    svg.selectAll('*').remove();

    const width = 800;
    const height = 600;
    const margin = { top: 20, right: 20, bottom: 20, left: 20 };

    svg.attr('width', width).attr('height', height);

    const g = svg.append('g')
      .attr('transform', `translate(${margin.left},${margin.top})`);

    // Create force simulation
    const simulation = d3.forceSimulation(topologyData.nodes)
      .force('link', d3.forceLink(topologyData.links).id((d: any) => d.id).distance(100))
      .force('charge', d3.forceManyBody().strength(-300))
      .force('center', d3.forceCenter(width / 2, height / 2));

    // Create links
    const link = g.append('g')
      .selectAll('line')
      .data(topologyData.links)
      .enter().append('line')
      .attr('stroke', (d: any) => d.status === 'up' ? '#4caf50' : '#f44336')
      .attr('stroke-width', (d: any) => Math.max(1, d.utilization / 10))
      .attr('stroke-opacity', 0.6);

    // Create nodes
    const node = g.append('g')
      .selectAll('circle')
      .data(topologyData.nodes)
      .enter().append('circle')
      .attr('r', (d: any) => {
        switch (d.type) {
          case 'router': return 15;
          case 'gateway': return 12;
          case 'switch': return 10;
          case 'host': return 8;
          default: return 10;
        }
      })
      .attr('fill', (d: any) => {
        switch (d.type) {
          case 'router': return d.status === 'up' ? '#1976d2' : '#f44336';
          case 'gateway': return d.status === 'up' ? '#4caf50' : '#f44336';
          case 'switch': return d.status === 'up' ? '#ff9800' : '#f44336';
          case 'host': return d.status === 'up' ? '#9c27b0' : '#f44336';
          default: return '#666';
        }
      })
      .attr('stroke', '#fff')
      .attr('stroke-width', 2)
      .call(d3.drag()
        .on('start', dragstarted)
        .on('drag', dragged)
        .on('end', dragended));

    // Add labels
    const labels = g.append('g')
      .selectAll('text')
      .data(topologyData.nodes)
      .enter().append('text')
      .text((d: any) => d.name)
      .attr('text-anchor', 'middle')
      .attr('dy', 25)
      .attr('fill', '#fff')
      .attr('font-size', '12px');

    // Add tooltips
    node.append('title')
      .text((d: any) => `${d.name}\n${d.ip_address}\nType: ${d.type}\nStatus: ${d.status}`);

    // Update positions on simulation tick
    simulation.on('tick', () => {
      link
        .attr('x1', (d: any) => d.source.x)
        .attr('y1', (d: any) => d.source.y)
        .attr('x2', (d: any) => d.target.x)
        .attr('y2', (d: any) => d.target.y);

      node
        .attr('cx', (d: any) => d.x)
        .attr('cy', (d: any) => d.y);

      labels
        .attr('x', (d: any) => d.x)
        .attr('y', (d: any) => d.y);
    });

    function dragstarted(event: any, d: any) {
      if (!event.active) simulation.alphaTarget(0.3).restart();
      d.fx = d.x;
      d.fy = d.y;
    }

    function dragged(event: any, d: any) {
      d.fx = event.x;
      d.fy = event.y;
    }

    function dragended(event: any, d: any) {
      if (!event.active) simulation.alphaTarget(0);
      d.fx = null;
      d.fy = null;
    }
  };

  const handleZoomIn = () => {
    setZoomLevel(prev => Math.min(prev * 1.2, 3));
  };

  const handleZoomOut = () => {
    setZoomLevel(prev => Math.max(prev / 1.2, 0.5));
  };

  const handleRefresh = () => {
    fetchTopologyData();
  };

  const getNodeTypeColor = (type: string) => {
    switch (type) {
      case 'router': return '#1976d2';
      case 'gateway': return '#4caf50';
      case 'switch': return '#ff9800';
      case 'host': return '#9c27b0';
      default: return '#666';
    }
  };

  return (
    <Box>
      <Box sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', mb: 3 }}>
        <Typography variant="h4" component="h1">
          Network Topology
        </Typography>
        <Box>
          <Tooltip title="Zoom In">
            <IconButton onClick={handleZoomIn}>
              <ZoomInIcon />
            </IconButton>
          </Tooltip>
          <Tooltip title="Zoom Out">
            <IconButton onClick={handleZoomOut}>
              <ZoomOutIcon />
            </IconButton>
          </Tooltip>
          <Tooltip title="Refresh">
            <IconButton onClick={handleRefresh}>
              <RefreshIcon />
            </IconButton>
          </Tooltip>
          <Tooltip title="Fullscreen">
            <IconButton>
              <FullscreenIcon />
            </IconButton>
          </Tooltip>
        </Box>
      </Box>

      <Grid container spacing={3}>
        <Grid item xs={12} md={9}>
          <Card>
            <CardContent>
              <Box sx={{ display: 'flex', justifyContent: 'center' }}>
                <svg
                  ref={svgRef}
                  style={{
                    transform: `scale(${zoomLevel})`,
                    transformOrigin: 'center',
                  }}
                />
              </Box>
            </CardContent>
          </Card>
        </Grid>

        <Grid item xs={12} md={3}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>
                Legend
              </Typography>
              <Box sx={{ display: 'flex', flexDirection: 'column', gap: 1 }}>
                <Box sx={{ display: 'flex', alignItems: 'center', gap: 1 }}>
                  <Box
                    sx={{
                      width: 12,
                      height: 12,
                      borderRadius: '50%',
                      backgroundColor: '#1976d2',
                    }}
                  />
                  <Typography variant="body2">Router</Typography>
                </Box>
                <Box sx={{ display: 'flex', alignItems: 'center', gap: 1 }}>
                  <Box
                    sx={{
                      width: 12,
                      height: 12,
                      borderRadius: '50%',
                      backgroundColor: '#4caf50',
                    }}
                  />
                  <Typography variant="body2">Gateway</Typography>
                </Box>
                <Box sx={{ display: 'flex', alignItems: 'center', gap: 1 }}>
                  <Box
                    sx={{
                      width: 12,
                      height: 12,
                      borderRadius: '50%',
                      backgroundColor: '#ff9800',
                    }}
                  />
                  <Typography variant="body2">Switch</Typography>
                </Box>
                <Box sx={{ display: 'flex', alignItems: 'center', gap: 1 }}>
                  <Box
                    sx={{
                      width: 12,
                      height: 12,
                      borderRadius: '50%',
                      backgroundColor: '#9c27b0',
                    }}
                  />
                  <Typography variant="body2">Host</Typography>
                </Box>
              </Box>

              <Typography variant="h6" gutterBottom sx={{ mt: 3 }}>
                Link Status
              </Typography>
              <Box sx={{ display: 'flex', flexDirection: 'column', gap: 1 }}>
                <Box sx={{ display: 'flex', alignItems: 'center', gap: 1 }}>
                  <Box
                    sx={{
                      width: 20,
                      height: 2,
                      backgroundColor: '#4caf50',
                    }}
                  />
                  <Typography variant="body2">Up</Typography>
                </Box>
                <Box sx={{ display: 'flex', alignItems: 'center', gap: 1 }}>
                  <Box
                    sx={{
                      width: 20,
                      height: 2,
                      backgroundColor: '#f44336',
                    }}
                  />
                  <Typography variant="body2">Down</Typography>
                </Box>
              </Box>

              <Typography variant="h6" gutterBottom sx={{ mt: 3 }}>
                Statistics
              </Typography>
              <Typography variant="body2">
                Nodes: {topologyData?.nodes.length || 0}
              </Typography>
              <Typography variant="body2">
                Links: {topologyData?.links.length || 0}
              </Typography>
            </CardContent>
          </Card>
        </Grid>
      </Grid>
    </Box>
  );
};

export default Topology;
