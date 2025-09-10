import React, { useState, useEffect } from 'react';
import {
  Card,
  CardContent,
  CardHeader,
  CardTitle,
} from '@/components/ui/card';
import { Button } from '@/components/ui/button';
import { Badge } from '@/components/ui/badge';
import { Tabs, TabsContent, TabsList, TabsTrigger } from '@/components/ui/tabs';
import { Progress } from '@/components/ui/progress';
import { Alert, AlertDescription } from '@/components/ui/alert';
import {
  Select,
  SelectContent,
  SelectItem,
  SelectTrigger,
  SelectValue,
} from '@/components/ui/select';
import {
  Table,
  TableBody,
  TableCell,
  TableHead,
  TableHeader,
  TableRow,
} from '@/components/ui/table';
import {
  Dialog,
  DialogContent,
  DialogDescription,
  DialogHeader,
  DialogTitle,
  DialogTrigger,
} from '@/components/ui/dialog';
import { Input } from '@/components/ui/input';
import { Label } from '@/components/ui/label';
import { Textarea } from '@/components/ui/textarea';
import {
  Cloud,
  Server,
  Network,
  Shield,
  Zap,
  BarChart3,
  Settings,
  Plus,
  Trash2,
  Edit,
  Play,
  Pause,
  RefreshCw,
  AlertTriangle,
  CheckCircle,
  XCircle,
  Globe,
  Database,
  Cpu,
  HardDrive,
  Wifi,
  Router,
  Layers,
  Activity,
  TrendingUp,
  TrendingDown,
  Minus,
  ArrowUpDown,
  ArrowRightLeft,
  ArrowDownUp,
  ArrowLeftRight,
  Circle,
  Square,
  Triangle,
  Hexagon,
  Octagon,
  Pentagon,
  Star,
  Heart,
  Zap as Lightning,
  Target,
  Compass,
  MapPin,
  Navigation,
  Route,
  GitBranch,
  GitCommit,
  GitMerge,
  GitPullRequest,
  GitCompare,
  GitBranchPlus,
  GitBranchMinus,
  GitBranchX,
  GitBranchCheck,
  GitBranchPlusIcon,
  GitBranchMinusIcon,
  GitBranchXIcon,
  GitBranchCheckIcon,
} from 'lucide-react';

interface CloudProvider {
  id: string;
  name: string;
  type: 'cloudpods' | 'aviatrix' | 'aws' | 'azure' | 'gcp';
  status: 'connected' | 'disconnected' | 'error';
  region: string;
  resources: number;
  cost: number;
  lastSync: string;
}

interface NetworkTopology {
  nodes: NetworkNode[];
  edges: NetworkEdge[];
}

interface NetworkNode {
  id: string;
  type: 'gateway' | 'router' | 'switch' | 'server' | 'cloud' | 'region';
  name: string;
  status: 'active' | 'inactive' | 'error';
  position: { x: number; y: number };
  properties: Record<string, any>;
  metrics: {
    cpu: number;
    memory: number;
    network: number;
    latency: number;
  };
}

interface NetworkEdge {
  id: string;
  source: string;
  target: string;
  type: 'bgp' | 'ospf' | 'isis' | 'static' | 'cloud';
  status: 'up' | 'down' | 'degraded';
  bandwidth: number;
  latency: number;
  utilization: number;
}

interface TrafficFlow {
  id: string;
  source: string;
  destination: string;
  protocol: string;
  bytes: number;
  packets: number;
  latency: number;
  jitter: number;
  loss: number;
  timestamp: string;
}

interface SecurityPolicy {
  id: string;
  name: string;
  type: 'firewall' | 'acl' | 'nat' | 'vpn';
  status: 'active' | 'inactive';
  rules: SecurityRule[];
  lastModified: string;
}

interface SecurityRule {
  id: string;
  action: 'allow' | 'deny';
  source: string;
  destination: string;
  protocol: string;
  port: string;
  description: string;
}

const CloudNetworking: React.FC = () => {
  const [cloudProviders, setCloudProviders] = useState<CloudProvider[]>([
    {
      id: '1',
      name: 'CloudPods Multi-Cloud',
      type: 'cloudpods',
      status: 'connected',
      region: 'us-west-1',
      resources: 45,
      cost: 1250.50,
      lastSync: '2 minutes ago',
    },
    {
      id: '2',
      name: 'Aviatrix Controller',
      type: 'aviatrix',
      status: 'connected',
      region: 'us-east-1',
      resources: 23,
      cost: 890.25,
      lastSync: '1 minute ago',
    },
    {
      id: '3',
      name: 'AWS Cloud',
      type: 'aws',
      status: 'connected',
      region: 'us-west-2',
      resources: 67,
      cost: 2100.75,
      lastSync: '3 minutes ago',
    },
    {
      id: '4',
      name: 'Azure Cloud',
      type: 'azure',
      status: 'error',
      region: 'eastus',
      resources: 0,
      cost: 0,
      lastSync: 'Never',
    },
  ]);

  const [networkTopology, setNetworkTopology] = useState<NetworkTopology>({
    nodes: [
      {
        id: '1',
        type: 'cloud',
        name: 'CloudPods',
        status: 'active',
        position: { x: 100, y: 100 },
        properties: { provider: 'cloudpods', region: 'us-west-1' },
        metrics: { cpu: 45, memory: 67, network: 23, latency: 12 },
      },
      {
        id: '2',
        type: 'gateway',
        name: 'Aviatrix Gateway',
        status: 'active',
        position: { x: 300, y: 100 },
        properties: { provider: 'aviatrix', region: 'us-east-1' },
        metrics: { cpu: 78, memory: 45, network: 89, latency: 8 },
      },
      {
        id: '3',
        type: 'router',
        name: 'BGP Router',
        status: 'active',
        position: { x: 500, y: 100 },
        properties: { protocol: 'bgp', asn: 65001 },
        metrics: { cpu: 34, memory: 56, network: 78, latency: 5 },
      },
      {
        id: '4',
        type: 'server',
        name: 'Web Server',
        status: 'active',
        position: { x: 700, y: 100 },
        properties: { service: 'web', port: 80 },
        metrics: { cpu: 67, memory: 78, network: 45, latency: 3 },
      },
    ],
    edges: [
      {
        id: '1',
        source: '1',
        target: '2',
        type: 'cloud',
        status: 'up',
        bandwidth: 1000,
        latency: 15,
        utilization: 45,
      },
      {
        id: '2',
        source: '2',
        target: '3',
        type: 'bgp',
        status: 'up',
        bandwidth: 10000,
        latency: 2,
        utilization: 23,
      },
      {
        id: '3',
        source: '3',
        target: '4',
        type: 'ospf',
        status: 'up',
        bandwidth: 1000,
        latency: 1,
        utilization: 67,
      },
    ],
  });

  const [trafficFlows, setTrafficFlows] = useState<TrafficFlow[]>([
    {
      id: '1',
      source: 'CloudPods',
      destination: 'Web Server',
      protocol: 'HTTP',
      bytes: 1024000,
      packets: 1500,
      latency: 25,
      jitter: 5,
      loss: 0.1,
      timestamp: '2024-01-15T10:30:00Z',
    },
    {
      id: '2',
      source: 'Aviatrix Gateway',
      destination: 'BGP Router',
      protocol: 'BGP',
      bytes: 512000,
      packets: 800,
      latency: 8,
      jitter: 2,
      loss: 0,
      timestamp: '2024-01-15T10:30:00Z',
    },
  ]);

  const [securityPolicies, setSecurityPolicies] = useState<SecurityPolicy[]>([
    {
      id: '1',
      name: 'Default Firewall',
      type: 'firewall',
      status: 'active',
      lastModified: '2024-01-15T09:00:00Z',
      rules: [
        {
          id: '1',
          action: 'allow',
          source: '0.0.0.0/0',
          destination: '10.0.0.0/8',
          protocol: 'TCP',
          port: '80,443',
          description: 'Allow HTTP/HTTPS traffic',
        },
        {
          id: '2',
          action: 'deny',
          source: '0.0.0.0/0',
          destination: '10.0.0.0/8',
          protocol: 'TCP',
          port: '22',
          description: 'Deny SSH access',
        },
      ],
    },
  ]);

  const [selectedProvider, setSelectedProvider] = useState<string>('all');
  const [selectedRegion, setSelectedRegion] = useState<string>('all');
  const [isLoading, setIsLoading] = useState(false);

  const getStatusIcon = (status: string) => {
    switch (status) {
      case 'connected':
      case 'active':
      case 'up':
        return <CheckCircle className="h-4 w-4 text-green-500" />;
      case 'disconnected':
      case 'inactive':
      case 'down':
        return <XCircle className="h-4 w-4 text-red-500" />;
      case 'error':
        return <AlertTriangle className="h-4 w-4 text-yellow-500" />;
      default:
        return <Circle className="h-4 w-4 text-gray-500" />;
    }
  };

  const getStatusBadge = (status: string) => {
    const variants = {
      connected: 'default',
      active: 'default',
      up: 'default',
      disconnected: 'secondary',
      inactive: 'secondary',
      down: 'destructive',
      error: 'destructive',
    } as const;

    return (
      <Badge variant={variants[status as keyof typeof variants] || 'secondary'}>
        {status}
      </Badge>
    );
  };

  const getProviderIcon = (type: string) => {
    switch (type) {
      case 'cloudpods':
        return <Cloud className="h-5 w-5" />;
      case 'aviatrix':
        return <Shield className="h-5 w-5" />;
      case 'aws':
        return <Server className="h-5 w-5" />;
      case 'azure':
        return <Database className="h-5 w-5" />;
      case 'gcp':
        return <Globe className="h-5 w-5" />;
      default:
        return <Network className="h-5 w-5" />;
    }
  };

  const getNodeIcon = (type: string) => {
    switch (type) {
      case 'cloud':
        return <Cloud className="h-6 w-6" />;
      case 'gateway':
        return <Shield className="h-6 w-6" />;
      case 'router':
        return <Router className="h-6 w-6" />;
      case 'switch':
        return <Layers className="h-6 w-6" />;
      case 'server':
        return <Server className="h-6 w-6" />;
      case 'region':
        return <Globe className="h-6 w-6" />;
      default:
        return <Circle className="h-6 w-6" />;
    }
  };

  const getEdgeIcon = (type: string) => {
    switch (type) {
      case 'bgp':
        return <GitBranch className="h-4 w-4" />;
      case 'ospf':
        return <Route className="h-4 w-4" />;
      case 'isis':
        return <Navigation className="h-4 w-4" />;
      case 'static':
        return <ArrowRightLeft className="h-4 w-4" />;
      case 'cloud':
        return <Cloud className="h-4 w-4" />;
      default:
        return <ArrowUpDown className="h-4 w-4" />;
    }
  };

  const handleRefresh = async () => {
    setIsLoading(true);
    // Simulate API call
    await new Promise(resolve => setTimeout(resolve, 1000));
    setIsLoading(false);
  };

  const handleProviderAction = (providerId: string, action: string) => {
    console.log(`Performing ${action} on provider ${providerId}`);
    // Implement provider actions
  };

  const handleNodeAction = (nodeId: string, action: string) => {
    console.log(`Performing ${action} on node ${nodeId}`);
    // Implement node actions
  };

  const handleEdgeAction = (edgeId: string, action: string) => {
    console.log(`Performing ${action} on edge ${edgeId}`);
    // Implement edge actions
  };

  const handleSecurityPolicyAction = (policyId: string, action: string) => {
    console.log(`Performing ${action} on security policy ${policyId}`);
    // Implement security policy actions
  };

  return (
    <div className="space-y-6">
      {/* Header */}
      <div className="flex items-center justify-between">
        <div>
          <h1 className="text-3xl font-bold">Cloud Networking</h1>
          <p className="text-muted-foreground">
            Multi-cloud network management and monitoring
          </p>
        </div>
        <div className="flex items-center space-x-2">
          <Button
            variant="outline"
            size="sm"
            onClick={handleRefresh}
            disabled={isLoading}
          >
            <RefreshCw className={`h-4 w-4 mr-2 ${isLoading ? 'animate-spin' : ''}`} />
            Refresh
          </Button>
          <Button size="sm">
            <Plus className="h-4 w-4 mr-2" />
            Add Provider
          </Button>
        </div>
      </div>

      {/* Stats Cards */}
      <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-4 gap-4">
        <Card>
          <CardContent className="p-6">
            <div className="flex items-center">
              <Cloud className="h-8 w-8 text-blue-500" />
              <div className="ml-4">
                <p className="text-sm font-medium text-muted-foreground">
                  Total Providers
                </p>
                <p className="text-2xl font-bold">{cloudProviders.length}</p>
              </div>
            </div>
          </CardContent>
        </Card>
        <Card>
          <CardContent className="p-6">
            <div className="flex items-center">
              <Server className="h-8 w-8 text-green-500" />
              <div className="ml-4">
                <p className="text-sm font-medium text-muted-foreground">
                  Active Resources
                </p>
                <p className="text-2xl font-bold">
                  {cloudProviders.reduce((sum, p) => sum + p.resources, 0)}
                </p>
              </div>
            </div>
          </CardContent>
        </Card>
        <Card>
          <CardContent className="p-6">
            <div className="flex items-center">
              <DollarSign className="h-8 w-8 text-yellow-500" />
              <div className="ml-4">
                <p className="text-sm font-medium text-muted-foreground">
                  Total Cost
                </p>
                <p className="text-2xl font-bold">
                  ${cloudProviders.reduce((sum, p) => sum + p.cost, 0).toFixed(2)}
                </p>
              </div>
            </div>
          </CardContent>
        </Card>
        <Card>
          <CardContent className="p-6">
            <div className="flex items-center">
              <Activity className="h-8 w-8 text-purple-500" />
              <div className="ml-4">
                <p className="text-sm font-medium text-muted-foreground">
                  Network Health
                </p>
                <p className="text-2xl font-bold">98.5%</p>
              </div>
            </div>
          </CardContent>
        </Card>
      </div>

      {/* Main Content */}
      <Tabs defaultValue="overview" className="space-y-4">
        <TabsList>
          <TabsTrigger value="overview">Overview</TabsTrigger>
          <TabsTrigger value="topology">Network Topology</TabsTrigger>
          <TabsTrigger value="traffic">Traffic Flows</TabsTrigger>
          <TabsTrigger value="security">Security</TabsTrigger>
          <TabsTrigger value="analytics">Analytics</TabsTrigger>
        </TabsList>

        {/* Overview Tab */}
        <TabsContent value="overview" className="space-y-4">
          <div className="grid grid-cols-1 lg:grid-cols-2 gap-6">
            {/* Cloud Providers */}
            <Card>
              <CardHeader>
                <CardTitle className="flex items-center">
                  <Cloud className="h-5 w-5 mr-2" />
                  Cloud Providers
                </CardTitle>
              </CardHeader>
              <CardContent>
                <div className="space-y-4">
                  {cloudProviders.map((provider) => (
                    <div
                      key={provider.id}
                      className="flex items-center justify-between p-4 border rounded-lg"
                    >
                      <div className="flex items-center space-x-3">
                        {getProviderIcon(provider.type)}
                        <div>
                          <p className="font-medium">{provider.name}</p>
                          <p className="text-sm text-muted-foreground">
                            {provider.region} • {provider.resources} resources
                          </p>
                        </div>
                      </div>
                      <div className="flex items-center space-x-2">
                        {getStatusIcon(provider.status)}
                        <div className="text-right">
                          <p className="text-sm font-medium">
                            ${provider.cost.toFixed(2)}
                          </p>
                          <p className="text-xs text-muted-foreground">
                            {provider.lastSync}
                          </p>
                        </div>
                      </div>
                    </div>
                  ))}
                </div>
              </CardContent>
            </Card>

            {/* Network Health */}
            <Card>
              <CardHeader>
                <CardTitle className="flex items-center">
                  <Activity className="h-5 w-5 mr-2" />
                  Network Health
                </CardTitle>
              </CardHeader>
              <CardContent>
                <div className="space-y-4">
                  <div className="flex items-center justify-between">
                    <span className="text-sm font-medium">Overall Health</span>
                    <span className="text-sm text-muted-foreground">98.5%</span>
                  </div>
                  <Progress value={98.5} className="h-2" />
                  
                  <div className="space-y-2">
                    <div className="flex items-center justify-between">
                      <span className="text-sm">BGP Sessions</span>
                      <Badge variant="default">12/12</Badge>
                    </div>
                    <div className="flex items-center justify-between">
                      <span className="text-sm">OSPF Neighbors</span>
                      <Badge variant="default">8/8</Badge>
                    </div>
                    <div className="flex items-center justify-between">
                      <span className="text-sm">ISIS Adjacencies</span>
                      <Badge variant="default">6/6</Badge>
                    </div>
                    <div className="flex items-center justify-between">
                      <span className="text-sm">Traffic Shaping</span>
                      <Badge variant="default">Active</Badge>
                    </div>
                  </div>
                </div>
              </CardContent>
            </Card>
          </div>
        </TabsContent>

        {/* Network Topology Tab */}
        <TabsContent value="topology" className="space-y-4">
          <Card>
            <CardHeader>
              <CardTitle className="flex items-center">
                <Network className="h-5 w-5 mr-2" />
                Network Topology
              </CardTitle>
            </CardHeader>
            <CardContent>
              <div className="space-y-4">
                {/* Topology Controls */}
                <div className="flex items-center space-x-4">
                  <Select value={selectedProvider} onValueChange={setSelectedProvider}>
                    <SelectTrigger className="w-48">
                      <SelectValue placeholder="Select Provider" />
                    </SelectTrigger>
                    <SelectContent>
                      <SelectItem value="all">All Providers</SelectItem>
                      <SelectItem value="cloudpods">CloudPods</SelectItem>
                      <SelectItem value="aviatrix">Aviatrix</SelectItem>
                      <SelectItem value="aws">AWS</SelectItem>
                      <SelectItem value="azure">Azure</SelectItem>
                    </SelectContent>
                  </Select>
                  
                  <Select value={selectedRegion} onValueChange={setSelectedRegion}>
                    <SelectTrigger className="w-48">
                      <SelectValue placeholder="Select Region" />
                    </SelectTrigger>
                    <SelectContent>
                      <SelectItem value="all">All Regions</SelectItem>
                      <SelectItem value="us-west-1">US West 1</SelectItem>
                      <SelectItem value="us-east-1">US East 1</SelectItem>
                      <SelectItem value="us-west-2">US West 2</SelectItem>
                      <SelectItem value="eastus">East US</SelectItem>
                    </SelectContent>
                  </Select>
                  
                  <Button variant="outline" size="sm">
                    <Settings className="h-4 w-4 mr-2" />
                    Settings
                  </Button>
                </div>

                {/* Topology Visualization */}
                <div className="border rounded-lg p-4 bg-gray-50 min-h-[400px]">
                  <div className="relative w-full h-96">
                    {networkTopology.nodes.map((node) => (
                      <div
                        key={node.id}
                        className="absolute flex items-center justify-center w-16 h-16 bg-white border-2 rounded-lg shadow-sm cursor-pointer hover:shadow-md transition-shadow"
                        style={{
                          left: `${node.position.x}px`,
                          top: `${node.position.y}px`,
                        }}
                      >
                        <div className="text-center">
                          {getNodeIcon(node.type)}
                          <p className="text-xs mt-1 font-medium">{node.name}</p>
                          <div className="flex items-center justify-center mt-1">
                            {getStatusIcon(node.status)}
                          </div>
                        </div>
                      </div>
                    ))}
                    
                    {/* Edges */}
                    <svg className="absolute inset-0 w-full h-full pointer-events-none">
                      {networkTopology.edges.map((edge) => {
                        const sourceNode = networkTopology.nodes.find(n => n.id === edge.source);
                        const targetNode = networkTopology.nodes.find(n => n.id === edge.target);
                        
                        if (!sourceNode || !targetNode) return null;
                        
                        const x1 = sourceNode.position.x + 32;
                        const y1 = sourceNode.position.y + 32;
                        const x2 = targetNode.position.x + 32;
                        const y2 = targetNode.position.y + 32;
                        
                        return (
                          <g key={edge.id}>
                            <line
                              x1={x1}
                              y1={y1}
                              x2={x2}
                              y2={y2}
                              stroke={edge.status === 'up' ? '#10b981' : edge.status === 'down' ? '#ef4444' : '#f59e0b'}
                              strokeWidth="2"
                              strokeDasharray={edge.status === 'degraded' ? '5,5' : '0'}
                            />
                            <circle
                              cx={(x1 + x2) / 2}
                              cy={(y1 + y2) / 2}
                              r="8"
                              fill="white"
                              stroke={edge.status === 'up' ? '#10b981' : edge.status === 'down' ? '#ef4444' : '#f59e0b'}
                              strokeWidth="2"
                            />
                            <text
                              x={(x1 + x2) / 2}
                              y={(y1 + y2) / 2 + 3}
                              textAnchor="middle"
                              className="text-xs font-medium fill-current"
                            >
                              {getEdgeIcon(edge.type)}
                            </text>
                          </g>
                        );
                      })}
                    </svg>
                  </div>
                </div>

                {/* Node Details */}
                <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-4 gap-4">
                  {networkTopology.nodes.map((node) => (
                    <Card key={node.id} className="p-4">
                      <div className="flex items-center justify-between mb-2">
                        <div className="flex items-center space-x-2">
                          {getNodeIcon(node.type)}
                          <span className="font-medium">{node.name}</span>
                        </div>
                        {getStatusIcon(node.status)}
                      </div>
                      <div className="space-y-2">
                        <div className="flex justify-between text-sm">
                          <span>CPU</span>
                          <span>{node.metrics.cpu}%</span>
                        </div>
                        <Progress value={node.metrics.cpu} className="h-1" />
                        <div className="flex justify-between text-sm">
                          <span>Memory</span>
                          <span>{node.metrics.memory}%</span>
                        </div>
                        <Progress value={node.metrics.memory} className="h-1" />
                        <div className="flex justify-between text-sm">
                          <span>Network</span>
                          <span>{node.metrics.network}%</span>
                        </div>
                        <Progress value={node.metrics.network} className="h-1" />
                        <div className="flex justify-between text-sm">
                          <span>Latency</span>
                          <span>{node.metrics.latency}ms</span>
                        </div>
                      </div>
                    </Card>
                  ))}
                </div>
              </div>
            </CardContent>
          </Card>
        </TabsContent>

        {/* Traffic Flows Tab */}
        <TabsContent value="traffic" className="space-y-4">
          <Card>
            <CardHeader>
              <CardTitle className="flex items-center">
                <Activity className="h-5 w-5 mr-2" />
                Traffic Flows
              </CardTitle>
            </CardHeader>
            <CardContent>
              <Table>
                <TableHeader>
                  <TableRow>
                    <TableHead>Source</TableHead>
                    <TableHead>Destination</TableHead>
                    <TableHead>Protocol</TableHead>
                    <TableHead>Bytes</TableHead>
                    <TableHead>Packets</TableHead>
                    <TableHead>Latency</TableHead>
                    <TableHead>Jitter</TableHead>
                    <TableHead>Loss</TableHead>
                    <TableHead>Actions</TableHead>
                  </TableRow>
                </TableHeader>
                <TableBody>
                  {trafficFlows.map((flow) => (
                    <TableRow key={flow.id}>
                      <TableCell className="font-medium">{flow.source}</TableCell>
                      <TableCell>{flow.destination}</TableCell>
                      <TableCell>
                        <Badge variant="outline">{flow.protocol}</Badge>
                      </TableCell>
                      <TableCell>{(flow.bytes / 1024 / 1024).toFixed(2)} MB</TableCell>
                      <TableCell>{flow.packets.toLocaleString()}</TableCell>
                      <TableCell>{flow.latency}ms</TableCell>
                      <TableCell>{flow.jitter}ms</TableCell>
                      <TableCell>
                        <span className={flow.loss > 0 ? 'text-red-500' : 'text-green-500'}>
                          {flow.loss}%
                        </span>
                      </TableCell>
                      <TableCell>
                        <div className="flex items-center space-x-2">
                          <Button variant="ghost" size="sm">
                            <Edit className="h-4 w-4" />
                          </Button>
                          <Button variant="ghost" size="sm">
                            <Trash2 className="h-4 w-4" />
                          </Button>
                        </div>
                      </TableCell>
                    </TableRow>
                  ))}
                </TableBody>
              </Table>
            </CardContent>
          </Card>
        </TabsContent>

        {/* Security Tab */}
        <TabsContent value="security" className="space-y-4">
          <Card>
            <CardHeader>
              <CardTitle className="flex items-center">
                <Shield className="h-5 w-5 mr-2" />
                Security Policies
              </CardTitle>
            </CardHeader>
            <CardContent>
              <div className="space-y-4">
                {securityPolicies.map((policy) => (
                  <Card key={policy.id} className="p-4">
                    <div className="flex items-center justify-between mb-4">
                      <div className="flex items-center space-x-2">
                        <Shield className="h-5 w-5" />
                        <div>
                          <h3 className="font-medium">{policy.name}</h3>
                          <p className="text-sm text-muted-foreground">
                            {policy.type} • {policy.rules.length} rules
                          </p>
                        </div>
                      </div>
                      <div className="flex items-center space-x-2">
                        {getStatusBadge(policy.status)}
                        <Button variant="ghost" size="sm">
                          <Edit className="h-4 w-4" />
                        </Button>
                        <Button variant="ghost" size="sm">
                          <Trash2 className="h-4 w-4" />
                        </Button>
                      </div>
                    </div>
                    
                    <div className="space-y-2">
                      {policy.rules.map((rule) => (
                        <div key={rule.id} className="flex items-center justify-between p-2 bg-gray-50 rounded">
                          <div className="flex items-center space-x-2">
                            <Badge variant={rule.action === 'allow' ? 'default' : 'destructive'}>
                              {rule.action}
                            </Badge>
                            <span className="text-sm">
                              {rule.source} → {rule.destination}
                            </span>
                            <span className="text-sm text-muted-foreground">
                              {rule.protocol}/{rule.port}
                            </span>
                          </div>
                          <span className="text-sm text-muted-foreground">
                            {rule.description}
                          </span>
                        </div>
                      ))}
                    </div>
                  </Card>
                ))}
              </div>
            </CardContent>
          </Card>
        </TabsContent>

        {/* Analytics Tab */}
        <TabsContent value="analytics" className="space-y-4">
          <div className="grid grid-cols-1 lg:grid-cols-2 gap-6">
            <Card>
              <CardHeader>
                <CardTitle className="flex items-center">
                  <BarChart3 className="h-5 w-5 mr-2" />
                  Network Performance
                </CardTitle>
              </CardHeader>
              <CardContent>
                <div className="space-y-4">
                  <div className="h-64 bg-gray-50 rounded flex items-center justify-center">
                    <p className="text-muted-foreground">Performance Chart</p>
                  </div>
                </div>
              </CardContent>
            </Card>
            
            <Card>
              <CardHeader>
                <CardTitle className="flex items-center">
                  <TrendingUp className="h-5 w-5 mr-2" />
                  Traffic Trends
                </CardTitle>
              </CardHeader>
              <CardContent>
                <div className="space-y-4">
                  <div className="h-64 bg-gray-50 rounded flex items-center justify-center">
                    <p className="text-muted-foreground">Traffic Chart</p>
                  </div>
                </div>
              </CardContent>
            </Card>
          </div>
        </TabsContent>
      </Tabs>
    </div>
  );
};

export default CloudNetworking;
