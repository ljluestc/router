import React, { useState, useEffect, useRef } from 'react';
import {
  Card,
  CardContent,
  CardHeader,
  CardTitle,
} from './components/ui/card';
import { Button } from './components/ui/button';
import { Badge } from './components/ui/badge';
import { Tabs, TabsContent, TabsList, TabsTrigger } from './components/ui/tabs';
import {
  Select,
  SelectContent,
  SelectItem,
  SelectTrigger,
  SelectValue,
} from './components/ui/select';
import {
  Dialog,
  DialogContent,
  DialogDescription,
  DialogHeader,
  DialogTitle,
  DialogTrigger,
} from './components/ui/dialog';
import { Input } from './components/ui/input';
import { Label } from './components/ui/label';
import { Textarea } from './components/ui/textarea';
import {
  Tooltip,
  TooltipContent,
  TooltipProvider,
  TooltipTrigger,
} from './components/ui/tooltip';
import {
  Network,
  Server,
  Cloud,
  LoadBalancer,
  Database,
  Router,
  Shield,
  Settings,
  Plus,
  Edit,
  Trash2,
  RefreshCw,
  Download,
  Upload,
  Eye,
  EyeOff,
  Zap,
  Globe,
  Lock,
  Unlock,
  Activity,
  AlertTriangle,
  CheckCircle,
  XCircle,
  Clock,
  Wifi,
  WifiOff,
} from 'lucide-react';

interface NetworkNode {
  id: string;
  type: 'vpc' | 'subnet' | 'instance' | 'loadbalancer' | 'natgateway' | 'router' | 'firewall';
  name: string;
  status: 'active' | 'inactive' | 'error' | 'pending';
  position: { x: number; y: number };
  size: { width: number; height: number };
  color: string;
  metadata: {
    ip?: string;
    cidr?: string;
    region?: string;
    az?: string;
    instanceType?: string;
    protocol?: string;
    port?: number;
    [key: string]: any;
  };
}

interface NetworkConnection {
  id: string;
  source: string;
  target: string;
  type: 'direct' | 'vpn' | 'peering' | 'transit';
  status: 'active' | 'inactive' | 'error';
  bandwidth?: number;
  latency?: number;
  color: string;
}

interface CloudProvider {
  id: string;
  name: string;
  type: 'aws' | 'azure' | 'gcp' | 'cloudpods' | 'aviatrix';
  region: string;
  status: 'connected' | 'disconnected' | 'error';
  nodes: NetworkNode[];
  connections: NetworkConnection[];
}

const NetworkTopology: React.FC = () => {
  const [providers, setProviders] = useState<CloudProvider[]>([]);
  const [selectedProvider, setSelectedProvider] = useState<string>('all');
  const [viewMode, setViewMode] = useState<'2d' | '3d'>('2d');
  const [showLabels, setShowLabels] = useState(true);
  const [showMetrics, setShowMetrics] = useState(true);
  const [isLoading, setIsLoading] = useState(true);
  const [selectedNode, setSelectedNode] = useState<NetworkNode | null>(null);
  const [isCreateDialogOpen, setIsCreateDialogOpen] = useState(false);
  const canvasRef = useRef<HTMLCanvasElement>(null);

  // Mock data for demonstration
  useEffect(() => {
    const mockProviders: CloudProvider[] = [
      {
        id: 'cloudpods-1',
        name: 'CloudPods Multi-Cloud',
        type: 'cloudpods',
        region: 'us-west-1',
        status: 'connected',
        nodes: [
          {
            id: 'vpc-1',
            type: 'vpc',
            name: 'Production VPC',
            status: 'active',
            position: { x: 100, y: 100 },
            size: { width: 200, height: 150 },
            color: '#3b82f6',
            metadata: {
              cidr: '10.0.0.0/16',
              region: 'us-west-1',
            },
          },
          {
            id: 'subnet-1',
            type: 'subnet',
            name: 'Public Subnet',
            status: 'active',
            position: { x: 120, y: 120 },
            size: { width: 80, height: 60 },
            color: '#10b981',
            metadata: {
              cidr: '10.0.1.0/24',
              az: 'us-west-1a',
            },
          },
          {
            id: 'subnet-2',
            type: 'subnet',
            name: 'Private Subnet',
            status: 'active',
            position: { x: 220, y: 120 },
            size: { width: 80, height: 60 },
            color: '#10b981',
            metadata: {
              cidr: '10.0.2.0/24',
              az: 'us-west-1b',
            },
          },
          {
            id: 'instance-1',
            type: 'instance',
            name: 'Web Server',
            status: 'active',
            position: { x: 130, y: 140 },
            size: { width: 60, height: 40 },
            color: '#f59e0b',
            metadata: {
              ip: '10.0.1.10',
              instanceType: 't3.medium',
            },
          },
          {
            id: 'lb-1',
            type: 'loadbalancer',
            name: 'Application LB',
            status: 'active',
            position: { x: 150, y: 80 },
            size: { width: 60, height: 40 },
            color: '#8b5cf6',
            metadata: {
              ip: '10.0.1.5',
              protocol: 'HTTP',
              port: 80,
            },
          },
          {
            id: 'nat-1',
            type: 'natgateway',
            name: 'NAT Gateway',
            status: 'active',
            position: { x: 250, y: 140 },
            size: { width: 60, height: 40 },
            color: '#ef4444',
            metadata: {
              ip: '10.0.2.1',
            },
          },
        ],
        connections: [
          {
            id: 'conn-1',
            source: 'vpc-1',
            target: 'subnet-1',
            type: 'direct',
            status: 'active',
            color: '#6b7280',
          },
          {
            id: 'conn-2',
            source: 'vpc-1',
            target: 'subnet-2',
            type: 'direct',
            status: 'active',
            color: '#6b7280',
          },
          {
            id: 'conn-3',
            source: 'subnet-1',
            target: 'instance-1',
            type: 'direct',
            status: 'active',
            color: '#6b7280',
          },
          {
            id: 'conn-4',
            source: 'subnet-1',
            target: 'lb-1',
            type: 'direct',
            status: 'active',
            color: '#6b7280',
          },
          {
            id: 'conn-5',
            source: 'subnet-2',
            target: 'nat-1',
            type: 'direct',
            status: 'active',
            color: '#6b7280',
          },
        ],
      },
      {
        id: 'aviatrix-1',
        name: 'Aviatrix Transit',
        type: 'aviatrix',
        region: 'us-east-1',
        status: 'connected',
        nodes: [
          {
            id: 'transit-1',
            type: 'router',
            name: 'Transit Gateway',
            status: 'active',
            position: { x: 400, y: 100 },
            size: { width: 120, height: 80 },
            color: '#f97316',
            metadata: {
              ip: '172.16.0.1',
              protocol: 'BGP',
            },
          },
          {
            id: 'firewall-1',
            type: 'firewall',
            name: 'Security Gateway',
            status: 'active',
            position: { x: 400, y: 200 },
            size: { width: 80, height: 60 },
            color: '#dc2626',
            metadata: {
              ip: '172.16.0.2',
            },
          },
        ],
        connections: [
          {
            id: 'conn-6',
            source: 'transit-1',
            target: 'firewall-1',
            type: 'direct',
            status: 'active',
            color: '#6b7280',
          },
        ],
      },
    ];

    setProviders(mockProviders);
    setIsLoading(false);
  }, []);

  const getStatusIcon = (status: string) => {
    switch (status) {
      case 'active':
        return <CheckCircle className="h-4 w-4 text-green-500" />;
      case 'inactive':
        return <XCircle className="h-4 w-4 text-gray-400" />;
      case 'error':
        return <AlertTriangle className="h-4 w-4 text-red-500" />;
      case 'pending':
        return <Clock className="h-4 w-4 text-yellow-500" />;
      default:
        return <XCircle className="h-4 w-4 text-gray-400" />;
    }
  };

  const getNodeIcon = (type: string) => {
    switch (type) {
      case 'vpc':
        return <Cloud className="h-5 w-5" />;
      case 'subnet':
        return <Network className="h-5 w-5" />;
      case 'instance':
        return <Server className="h-5 w-5" />;
      case 'loadbalancer':
        return <LoadBalancer className="h-5 w-5" />;
      case 'natgateway':
        return <Router className="h-5 w-5" />;
      case 'router':
        return <Router className="h-5 w-5" />;
      case 'firewall':
        return <Shield className="h-5 w-5" />;
      default:
        return <Server className="h-5 w-5" />;
    }
  };

  const getConnectionTypeIcon = (type: string) => {
    switch (type) {
      case 'direct':
        return <Wifi className="h-4 w-4" />;
      case 'vpn':
        return <Lock className="h-4 w-4" />;
      case 'peering':
        return <Globe className="h-4 w-4" />;
      case 'transit':
        return <Zap className="h-4 w-4" />;
      default:
        return <Wifi className="h-4 w-4" />;
    }
  };

  const renderNetworkCanvas = () => {
    const canvas = canvasRef.current;
    if (!canvas) return;

    const ctx = canvas.getContext('2d');
    if (!ctx) return;

    // Clear canvas
    ctx.clearRect(0, 0, canvas.width, canvas.height);

    // Set canvas size
    canvas.width = canvas.offsetWidth;
    canvas.height = canvas.offsetHeight;

    // Draw connections
    providers.forEach(provider => {
      if (selectedProvider !== 'all' && selectedProvider !== provider.id) return;

      provider.connections.forEach(connection => {
        const sourceNode = provider.nodes.find(n => n.id === connection.source);
        const targetNode = provider.nodes.find(n => n.id === connection.target);
        
        if (!sourceNode || !targetNode) return;

        ctx.strokeStyle = connection.color;
        ctx.lineWidth = 2;
        ctx.setLineDash(connection.status === 'active' ? [] : [5, 5]);
        
        ctx.beginPath();
        ctx.moveTo(
          sourceNode.position.x + sourceNode.size.width / 2,
          sourceNode.position.y + sourceNode.size.height / 2
        );
        ctx.lineTo(
          targetNode.position.x + targetNode.size.width / 2,
          targetNode.position.y + targetNode.size.height / 2
        );
        ctx.stroke();
      });
    });

    // Draw nodes
    providers.forEach(provider => {
      if (selectedProvider !== 'all' && selectedProvider !== provider.id) return;

      provider.nodes.forEach(node => {
        // Draw node background
        ctx.fillStyle = node.color + '20';
        ctx.fillRect(node.position.x, node.position.y, node.size.width, node.size.height);

        // Draw node border
        ctx.strokeStyle = node.color;
        ctx.lineWidth = 2;
        ctx.strokeRect(node.position.x, node.position.y, node.size.width, node.size.height);

        // Draw node text
        if (showLabels) {
          ctx.fillStyle = '#374151';
          ctx.font = '12px Inter, sans-serif';
          ctx.textAlign = 'center';
          ctx.fillText(
            node.name,
            node.position.x + node.size.width / 2,
            node.position.y + node.size.height / 2 + 4
          );
        }

        // Draw status indicator
        const statusColor = node.status === 'active' ? '#10b981' : 
                           node.status === 'error' ? '#ef4444' : '#6b7280';
        ctx.fillStyle = statusColor;
        ctx.beginPath();
        ctx.arc(node.position.x + node.size.width - 8, node.position.y + 8, 4, 0, 2 * Math.PI);
        ctx.fill();
      });
    });
  };

  useEffect(() => {
    renderNetworkCanvas();
  }, [providers, selectedProvider, showLabels]);

  const handleNodeClick = (node: NetworkNode) => {
    setSelectedNode(node);
  };

  const handleCanvasClick = (event: React.MouseEvent<HTMLCanvasElement>) => {
    const canvas = canvasRef.current;
    if (!canvas) return;

    const rect = canvas.getBoundingClientRect();
    const x = event.clientX - rect.left;
    const y = event.clientY - rect.top;

    // Find clicked node
    providers.forEach(provider => {
      provider.nodes.forEach(node => {
        if (x >= node.position.x && x <= node.position.x + node.size.width &&
            y >= node.position.y && y <= node.position.y + node.size.height) {
          handleNodeClick(node);
        }
      });
    });
  };

  if (isLoading) {
    return (
      <div className="flex items-center justify-center h-64">
        <RefreshCw className="h-8 w-8 animate-spin" />
        <span className="ml-2">Loading network topology...</span>
      </div>
    );
  }

  return (
    <div className="space-y-6">
      {/* Header */}
      <div className="flex items-center justify-between">
        <div>
          <h1 className="text-3xl font-bold text-gray-900">Network Topology</h1>
          <p className="text-gray-600 mt-1">
            Visualize and manage your multi-cloud network infrastructure
          </p>
        </div>
        <div className="flex items-center space-x-2">
          <Button
            variant="outline"
            size="sm"
            onClick={() => setIsCreateDialogOpen(true)}
          >
            <Plus className="h-4 w-4 mr-2" />
            Create Resource
          </Button>
          <Button variant="outline" size="sm">
            <Download className="h-4 w-4 mr-2" />
            Export
          </Button>
          <Button variant="outline" size="sm">
            <Upload className="h-4 w-4 mr-2" />
            Import
          </Button>
        </div>
      </div>

      {/* Controls */}
      <Card>
        <CardContent className="p-4">
          <div className="flex items-center justify-between">
            <div className="flex items-center space-x-4">
              <Select value={selectedProvider} onValueChange={setSelectedProvider}>
                <SelectTrigger className="w-48">
                  <SelectValue placeholder="Select provider" />
                </SelectTrigger>
                <SelectContent>
                  <SelectItem value="all">All Providers</SelectItem>
                  {providers.map(provider => (
                    <SelectItem key={provider.id} value={provider.id}>
                      {provider.name}
                    </SelectItem>
                  ))}
                </SelectContent>
              </Select>

              <Select value={viewMode} onValueChange={(value: '2d' | '3d') => setViewMode(value)}>
                <SelectTrigger className="w-32">
                  <SelectValue />
                </SelectTrigger>
                <SelectContent>
                  <SelectItem value="2d">2D View</SelectItem>
                  <SelectItem value="3d">3D View</SelectItem>
                </SelectContent>
              </Select>
            </div>

            <div className="flex items-center space-x-2">
              <TooltipProvider>
                <Tooltip>
                  <TooltipTrigger asChild>
                    <Button
                      variant="outline"
                      size="sm"
                      onClick={() => setShowLabels(!showLabels)}
                    >
                      {showLabels ? <EyeOff className="h-4 w-4" /> : <Eye className="h-4 w-4" />}
                    </Button>
                  </TooltipTrigger>
                  <TooltipContent>
                    {showLabels ? 'Hide Labels' : 'Show Labels'}
                  </TooltipContent>
                </Tooltip>
              </TooltipProvider>

              <TooltipProvider>
                <Tooltip>
                  <TooltipTrigger asChild>
                    <Button
                      variant="outline"
                      size="sm"
                      onClick={() => setShowMetrics(!showMetrics)}
                    >
                      <Activity className="h-4 w-4" />
                    </Button>
                  </TooltipTrigger>
                  <TooltipContent>
                    {showMetrics ? 'Hide Metrics' : 'Show Metrics'}
                  </TooltipContent>
                </Tooltip>
              </TooltipProvider>

              <Button variant="outline" size="sm" onClick={() => renderNetworkCanvas()}>
                <RefreshCw className="h-4 w-4" />
              </Button>
            </div>
          </div>
        </CardContent>
      </Card>

      <div className="grid grid-cols-1 lg:grid-cols-4 gap-6">
        {/* Network Canvas */}
        <div className="lg:col-span-3">
          <Card>
            <CardHeader>
              <CardTitle>Network Diagram</CardTitle>
            </CardHeader>
            <CardContent>
              <div className="relative">
                <canvas
                  ref={canvasRef}
                  className="w-full h-96 border border-gray-200 rounded-lg cursor-pointer"
                  onClick={handleCanvasClick}
                />
                {providers.length === 0 && (
                  <div className="absolute inset-0 flex items-center justify-center text-gray-500">
                    No network resources found
                  </div>
                )}
              </div>
            </CardContent>
          </Card>
        </div>

        {/* Resource Details */}
        <div className="space-y-4">
          <Card>
            <CardHeader>
              <CardTitle>Resources</CardTitle>
            </CardHeader>
            <CardContent>
              <div className="space-y-2">
                {providers.map(provider => (
                  <div key={provider.id} className="flex items-center justify-between p-2 border rounded">
                    <div className="flex items-center space-x-2">
                      {getNodeIcon(provider.type)}
                      <span className="text-sm font-medium">{provider.name}</span>
                    </div>
                    <Badge variant={provider.status === 'connected' ? 'default' : 'secondary'}>
                      {provider.status}
                    </Badge>
                  </div>
                ))}
              </div>
            </CardContent>
          </Card>

          {selectedNode && (
            <Card>
              <CardHeader>
                <CardTitle>Node Details</CardTitle>
              </CardHeader>
              <CardContent>
                <div className="space-y-3">
                  <div className="flex items-center space-x-2">
                    {getNodeIcon(selectedNode.type)}
                    <span className="font-medium">{selectedNode.name}</span>
                    {getStatusIcon(selectedNode.status)}
                  </div>
                  
                  <div className="space-y-2 text-sm">
                    <div className="flex justify-between">
                      <span className="text-gray-500">Type:</span>
                      <span className="capitalize">{selectedNode.type}</span>
                    </div>
                    {selectedNode.metadata.ip && (
                      <div className="flex justify-between">
                        <span className="text-gray-500">IP:</span>
                        <span>{selectedNode.metadata.ip}</span>
                      </div>
                    )}
                    {selectedNode.metadata.cidr && (
                      <div className="flex justify-between">
                        <span className="text-gray-500">CIDR:</span>
                        <span>{selectedNode.metadata.cidr}</span>
                      </div>
                    )}
                    {selectedNode.metadata.region && (
                      <div className="flex justify-between">
                        <span className="text-gray-500">Region:</span>
                        <span>{selectedNode.metadata.region}</span>
                      </div>
                    )}
                    {selectedNode.metadata.az && (
                      <div className="flex justify-between">
                        <span className="text-gray-500">AZ:</span>
                        <span>{selectedNode.metadata.az}</span>
                      </div>
                    )}
                  </div>

                  <div className="flex space-x-2">
                    <Button variant="outline" size="sm" className="flex-1">
                      <Edit className="h-4 w-4 mr-1" />
                      Edit
                    </Button>
                    <Button variant="outline" size="sm" className="flex-1">
                      <Settings className="h-4 w-4 mr-1" />
                      Config
                    </Button>
                  </div>
                </div>
              </CardContent>
            </Card>
          )}
        </div>
      </div>

      {/* Create Resource Dialog */}
      <Dialog open={isCreateDialogOpen} onOpenChange={setIsCreateDialogOpen}>
        <DialogContent>
          <DialogHeader>
            <DialogTitle>Create Network Resource</DialogTitle>
            <DialogDescription>
              Add a new network resource to your topology
            </DialogDescription>
          </DialogHeader>
          <div className="space-y-4">
            <div>
              <Label htmlFor="resource-type">Resource Type</Label>
              <Select>
                <SelectTrigger>
                  <SelectValue placeholder="Select resource type" />
                </SelectTrigger>
                <SelectContent>
                  <SelectItem value="vpc">VPC</SelectItem>
                  <SelectItem value="subnet">Subnet</SelectItem>
                  <SelectItem value="instance">Instance</SelectItem>
                  <SelectItem value="loadbalancer">Load Balancer</SelectItem>
                  <SelectItem value="natgateway">NAT Gateway</SelectItem>
                </SelectContent>
              </Select>
            </div>
            <div>
              <Label htmlFor="resource-name">Name</Label>
              <Input id="resource-name" placeholder="Enter resource name" />
            </div>
            <div>
              <Label htmlFor="resource-description">Description</Label>
              <Textarea id="resource-description" placeholder="Enter description" />
            </div>
            <div className="flex justify-end space-x-2">
              <Button variant="outline" onClick={() => setIsCreateDialogOpen(false)}>
                Cancel
              </Button>
              <Button onClick={() => setIsCreateDialogOpen(false)}>
                Create
              </Button>
            </div>
          </div>
        </DialogContent>
      </Dialog>
    </div>
  );
};

export default NetworkTopology;
