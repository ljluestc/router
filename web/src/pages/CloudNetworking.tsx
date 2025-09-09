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
  Cloud,
  Network,
  Server,
  Shield,
  Zap,
  Globe,
  Database,
  Activity,
  Settings,
  Play,
  Pause,
  RefreshCw,
  AlertCircle,
  CheckCircle,
  XCircle,
} from 'lucide-react';

interface CloudProvider {
  id: string;
  name: string;
  type: 'cloudpods' | 'aviatrix';
  status: 'connected' | 'disconnected' | 'error';
  regions: number;
  instances: number;
  lastSync: string;
}

interface NetworkTopology {
  nodes: NetworkNode[];
  edges: NetworkEdge[];
}

interface NetworkNode {
  id: string;
  type: 'gateway' | 'instance' | 'vpc' | 'subnet' | 'loadbalancer';
  name: string;
  status: 'active' | 'inactive' | 'error';
  region: string;
  cloud: string;
  position: { x: number; y: number };
  metrics: {
    cpu: number;
    memory: number;
    network: number;
  };
}

interface NetworkEdge {
  id: string;
  source: string;
  target: string;
  type: 'peering' | 'attachment' | 'routing';
  status: 'active' | 'inactive' | 'error';
  bandwidth: number;
  latency: number;
}

const CloudNetworking: React.FC = () => {
  const [cloudProviders, setCloudProviders] = useState<CloudProvider[]>([
    {
      id: 'cloudpods-1',
      name: 'CloudPods Primary',
      type: 'cloudpods',
      status: 'connected',
      regions: 3,
      instances: 12,
      lastSync: '2 minutes ago',
    },
    {
      id: 'aviatrix-1',
      name: 'Aviatrix Controller',
      type: 'aviatrix',
      status: 'connected',
      regions: 5,
      instances: 8,
      lastSync: '1 minute ago',
    },
  ]);

  const [networkTopology, setNetworkTopology] = useState<NetworkTopology>({
    nodes: [
      {
        id: 'tgw-1',
        type: 'gateway',
        name: 'Transit Gateway US-West',
        status: 'active',
        region: 'us-west-1',
        cloud: 'aviatrix',
        position: { x: 200, y: 100 },
        metrics: { cpu: 45, memory: 60, network: 80 },
      },
      {
        id: 'tgw-2',
        type: 'gateway',
        name: 'Transit Gateway EU-West',
        status: 'active',
        region: 'eu-west-1',
        cloud: 'aviatrix',
        position: { x: 400, y: 100 },
        metrics: { cpu: 35, memory: 50, network: 70 },
      },
      {
        id: 'spoke-1',
        type: 'gateway',
        name: 'Spoke Gateway VPC-1',
        status: 'active',
        region: 'us-west-1',
        cloud: 'cloudpods',
        position: { x: 150, y: 250 },
        metrics: { cpu: 25, memory: 40, network: 60 },
      },
      {
        id: 'spoke-2',
        type: 'gateway',
        name: 'Spoke Gateway VPC-2',
        status: 'active',
        region: 'us-east-1',
        cloud: 'cloudpods',
        position: { x: 250, y: 250 },
        metrics: { cpu: 30, memory: 45, network: 65 },
      },
      {
        id: 'instance-1',
        type: 'instance',
        name: 'Web Server 1',
        status: 'active',
        region: 'us-west-1',
        cloud: 'cloudpods',
        position: { x: 100, y: 350 },
        metrics: { cpu: 20, memory: 30, network: 40 },
      },
      {
        id: 'instance-2',
        type: 'instance',
        name: 'Database Server',
        status: 'active',
        region: 'us-west-1',
        cloud: 'cloudpods',
        position: { x: 200, y: 350 },
        metrics: { cpu: 60, memory: 80, network: 50 },
      },
    ],
    edges: [
      {
        id: 'edge-1',
        source: 'tgw-1',
        target: 'tgw-2',
        type: 'peering',
        status: 'active',
        bandwidth: 1000,
        latency: 150,
      },
      {
        id: 'edge-2',
        source: 'tgw-1',
        target: 'spoke-1',
        type: 'attachment',
        status: 'active',
        bandwidth: 500,
        latency: 5,
      },
      {
        id: 'edge-3',
        source: 'tgw-1',
        target: 'spoke-2',
        type: 'attachment',
        status: 'active',
        bandwidth: 500,
        latency: 10,
      },
      {
        id: 'edge-4',
        source: 'spoke-1',
        target: 'instance-1',
        type: 'routing',
        status: 'active',
        bandwidth: 100,
        latency: 1,
      },
      {
        id: 'edge-5',
        source: 'spoke-1',
        target: 'instance-2',
        type: 'routing',
        status: 'active',
        bandwidth: 100,
        latency: 1,
      },
    ],
  });

  const [selectedNode, setSelectedNode] = useState<NetworkNode | null>(null);
  const [isSimulating, setIsSimulating] = useState(false);

  const getStatusIcon = (status: string) => {
    switch (status) {
      case 'active':
      case 'connected':
        return <CheckCircle className="h-4 w-4 text-green-500" />;
      case 'inactive':
      case 'disconnected':
        return <XCircle className="h-4 w-4 text-gray-500" />;
      case 'error':
        return <AlertCircle className="h-4 w-4 text-red-500" />;
      default:
        return <Activity className="h-4 w-4 text-blue-500" />;
    }
  };

  const getStatusColor = (status: string) => {
    switch (status) {
      case 'active':
      case 'connected':
        return 'bg-green-100 text-green-800';
      case 'inactive':
      case 'disconnected':
        return 'bg-gray-100 text-gray-800';
      case 'error':
        return 'bg-red-100 text-red-800';
      default:
        return 'bg-blue-100 text-blue-800';
    }
  };

  const getNodeIcon = (type: string) => {
    switch (type) {
      case 'gateway':
        return <Network className="h-5 w-5" />;
      case 'instance':
        return <Server className="h-5 w-5" />;
      case 'vpc':
        return <Cloud className="h-5 w-5" />;
      case 'subnet':
        return <Globe className="h-5 w-5" />;
      case 'loadbalancer':
        return <Zap className="h-5 w-5" />;
      default:
        return <Activity className="h-5 w-5" />;
    }
  };

  const startSimulation = () => {
    setIsSimulating(true);
    // Simulate network activity
    setTimeout(() => {
      setIsSimulating(false);
    }, 5000);
  };

  const refreshTopology = () => {
    // Simulate topology refresh
    console.log('Refreshing network topology...');
  };

  return (
    <div className="space-y-6">
      <div className="flex items-center justify-between">
        <div>
          <h1 className="text-3xl font-bold">Cloud Networking</h1>
          <p className="text-gray-600">
            Multi-cloud network management with CloudPods and Aviatrix integration
          </p>
        </div>
        <div className="flex space-x-2">
          <Button
            variant="outline"
            onClick={refreshTopology}
            disabled={isSimulating}
          >
            <RefreshCw className="h-4 w-4 mr-2" />
            Refresh
          </Button>
          <Button
            onClick={startSimulation}
            disabled={isSimulating}
          >
            {isSimulating ? (
              <>
                <Pause className="h-4 w-4 mr-2" />
                Simulating...
              </>
            ) : (
              <>
                <Play className="h-4 w-4 mr-2" />
                Start Simulation
              </>
            )}
          </Button>
        </div>
      </div>

      <Tabs defaultValue="overview" className="space-y-4">
        <TabsList>
          <TabsTrigger value="overview">Overview</TabsTrigger>
          <TabsTrigger value="topology">Network Topology</TabsTrigger>
          <TabsTrigger value="providers">Cloud Providers</TabsTrigger>
          <TabsTrigger value="monitoring">Monitoring</TabsTrigger>
        </TabsList>

        <TabsContent value="overview" className="space-y-4">
          <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-4 gap-4">
            <Card>
              <CardHeader className="flex flex-row items-center justify-between space-y-0 pb-2">
                <CardTitle className="text-sm font-medium">Total Gateways</CardTitle>
                <Network className="h-4 w-4 text-muted-foreground" />
              </CardHeader>
              <CardContent>
                <div className="text-2xl font-bold">4</div>
                <p className="text-xs text-muted-foreground">
                  +2 from last month
                </p>
              </CardContent>
            </Card>
            <Card>
              <CardHeader className="flex flex-row items-center justify-between space-y-0 pb-2">
                <CardTitle className="text-sm font-medium">Active Connections</CardTitle>
                <Activity className="h-4 w-4 text-muted-foreground" />
              </CardHeader>
              <CardContent>
                <div className="text-2xl font-bold">12</div>
                <p className="text-xs text-muted-foreground">
                  +5 from last hour
                </p>
              </CardContent>
            </Card>
            <Card>
              <CardHeader className="flex flex-row items-center justify-between space-y-0 pb-2">
                <CardTitle className="text-sm font-medium">Network Latency</CardTitle>
                <Zap className="h-4 w-4 text-muted-foreground" />
              </CardHeader>
              <CardContent>
                <div className="text-2xl font-bold">45ms</div>
                <p className="text-xs text-muted-foreground">
                  -10ms from last hour
                </p>
              </CardContent>
            </Card>
            <Card>
              <CardHeader className="flex flex-row items-center justify-between space-y-0 pb-2">
                <CardTitle className="text-sm font-medium">Throughput</CardTitle>
                <Globe className="h-4 w-4 text-muted-foreground" />
              </CardHeader>
              <CardContent>
                <div className="text-2xl font-bold">2.1 Gbps</div>
                <p className="text-xs text-muted-foreground">
                  +0.3 Gbps from last hour
                </p>
              </CardContent>
            </Card>
          </div>

          <div className="grid grid-cols-1 lg:grid-cols-2 gap-4">
            <Card>
              <CardHeader>
                <CardTitle>Network Health</CardTitle>
              </CardHeader>
              <CardContent className="space-y-4">
                <div className="flex items-center justify-between">
                  <span className="text-sm font-medium">Overall Health</span>
                  <Badge className={getStatusColor('active')}>Healthy</Badge>
                </div>
                <Progress value={85} className="w-full" />
                <div className="grid grid-cols-2 gap-4 text-sm">
                  <div>
                    <div className="flex items-center justify-between">
                      <span>Gateways</span>
                      <span className="font-medium">4/4</span>
                    </div>
                    <Progress value={100} className="w-full mt-1" />
                  </div>
                  <div>
                    <div className="flex items-center justify-between">
                      <span>Connections</span>
                      <span className="font-medium">12/12</span>
                    </div>
                    <Progress value={100} className="w-full mt-1" />
                  </div>
                </div>
              </CardContent>
            </Card>

            <Card>
              <CardHeader>
                <CardTitle>Recent Activity</CardTitle>
              </CardHeader>
              <CardContent className="space-y-3">
                <div className="flex items-center space-x-3">
                  <CheckCircle className="h-4 w-4 text-green-500" />
                  <div className="flex-1">
                    <p className="text-sm font-medium">Gateway TGW-1 connected</p>
                    <p className="text-xs text-gray-500">2 minutes ago</p>
                  </div>
                </div>
                <div className="flex items-center space-x-3">
                  <CheckCircle className="h-4 w-4 text-green-500" />
                  <div className="flex-1">
                    <p className="text-sm font-medium">Spoke gateway attached</p>
                    <p className="text-xs text-gray-500">5 minutes ago</p>
                  </div>
                </div>
                <div className="flex items-center space-x-3">
                  <AlertCircle className="h-4 w-4 text-yellow-500" />
                  <div className="flex-1">
                    <p className="text-sm font-medium">High latency detected</p>
                    <p className="text-xs text-gray-500">10 minutes ago</p>
                  </div>
                </div>
              </CardContent>
            </Card>
          </div>
        </TabsContent>

        <TabsContent value="topology" className="space-y-4">
          <Card>
            <CardHeader>
              <CardTitle>Network Topology</CardTitle>
            </CardHeader>
            <CardContent>
              <div className="relative h-96 bg-gray-50 rounded-lg border-2 border-dashed border-gray-300">
                <div className="absolute inset-0 p-4">
                  <div className="grid grid-cols-6 gap-4 h-full">
                    {networkTopology.nodes.map((node) => (
                      <div
                        key={node.id}
                        className={`relative p-3 rounded-lg border-2 cursor-pointer transition-all hover:shadow-md ${
                          selectedNode?.id === node.id
                            ? 'border-blue-500 bg-blue-50'
                            : 'border-gray-200 bg-white'
                        }`}
                        onClick={() => setSelectedNode(node)}
                        style={{
                          gridColumn: Math.floor(node.position.x / 100) + 1,
                          gridRow: Math.floor(node.position.y / 100) + 1,
                        }}
                      >
                        <div className="flex items-center space-x-2 mb-2">
                          {getNodeIcon(node.type)}
                          <span className="text-sm font-medium">{node.name}</span>
                          {getStatusIcon(node.status)}
                        </div>
                        <div className="space-y-1">
                          <div className="flex items-center justify-between text-xs">
                            <span>CPU</span>
                            <span>{node.metrics.cpu}%</span>
                          </div>
                          <Progress value={node.metrics.cpu} className="h-1" />
                          <div className="flex items-center justify-between text-xs">
                            <span>Memory</span>
                            <span>{node.metrics.memory}%</span>
                          </div>
                          <Progress value={node.metrics.memory} className="h-1" />
                        </div>
                      </div>
                    ))}
                  </div>
                </div>
              </div>
            </CardContent>
          </Card>

          {selectedNode && (
            <Card>
              <CardHeader>
                <CardTitle>Node Details: {selectedNode.name}</CardTitle>
              </CardHeader>
              <CardContent>
                <div className="grid grid-cols-2 gap-4">
                  <div>
                    <h4 className="font-medium mb-2">Basic Information</h4>
                    <div className="space-y-2 text-sm">
                      <div className="flex justify-between">
                        <span>Type:</span>
                        <span className="capitalize">{selectedNode.type}</span>
                      </div>
                      <div className="flex justify-between">
                        <span>Status:</span>
                        <Badge className={getStatusColor(selectedNode.status)}>
                          {selectedNode.status}
                        </Badge>
                      </div>
                      <div className="flex justify-between">
                        <span>Region:</span>
                        <span>{selectedNode.region}</span>
                      </div>
                      <div className="flex justify-between">
                        <span>Cloud:</span>
                        <span className="capitalize">{selectedNode.cloud}</span>
                      </div>
                    </div>
                  </div>
                  <div>
                    <h4 className="font-medium mb-2">Metrics</h4>
                    <div className="space-y-2">
                      <div>
                        <div className="flex justify-between text-sm mb-1">
                          <span>CPU Usage</span>
                          <span>{selectedNode.metrics.cpu}%</span>
                        </div>
                        <Progress value={selectedNode.metrics.cpu} />
                      </div>
                      <div>
                        <div className="flex justify-between text-sm mb-1">
                          <span>Memory Usage</span>
                          <span>{selectedNode.metrics.memory}%</span>
                        </div>
                        <Progress value={selectedNode.metrics.memory} />
                      </div>
                      <div>
                        <div className="flex justify-between text-sm mb-1">
                          <span>Network Usage</span>
                          <span>{selectedNode.metrics.network}%</span>
                        </div>
                        <Progress value={selectedNode.metrics.network} />
                      </div>
                    </div>
                  </div>
                </div>
              </CardContent>
            </Card>
          )}
        </TabsContent>

        <TabsContent value="providers" className="space-y-4">
          <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
            {cloudProviders.map((provider) => (
              <Card key={provider.id}>
                <CardHeader>
                  <div className="flex items-center justify-between">
                    <CardTitle className="flex items-center space-x-2">
                      {provider.type === 'cloudpods' ? (
                        <Cloud className="h-5 w-5" />
                      ) : (
                        <Shield className="h-5 w-5" />
                      )}
                      <span>{provider.name}</span>
                    </CardTitle>
                    <Badge className={getStatusColor(provider.status)}>
                      {provider.status}
                    </Badge>
                  </div>
                </CardHeader>
                <CardContent>
                  <div className="space-y-3">
                    <div className="flex justify-between text-sm">
                      <span>Regions:</span>
                      <span className="font-medium">{provider.regions}</span>
                    </div>
                    <div className="flex justify-between text-sm">
                      <span>Instances:</span>
                      <span className="font-medium">{provider.instances}</span>
                    </div>
                    <div className="flex justify-between text-sm">
                      <span>Last Sync:</span>
                      <span className="font-medium">{provider.lastSync}</span>
                    </div>
                    <div className="pt-2">
                      <Button variant="outline" size="sm" className="w-full">
                        <Settings className="h-4 w-4 mr-2" />
                        Configure
                      </Button>
                    </div>
                  </div>
                </CardContent>
              </Card>
            ))}
          </div>
        </TabsContent>

        <TabsContent value="monitoring" className="space-y-4">
          <Alert>
            <Activity className="h-4 w-4" />
            <AlertDescription>
              Real-time monitoring is {isSimulating ? 'active' : 'inactive'}. 
              {isSimulating ? ' Network simulation is running.' : ' Click "Start Simulation" to begin monitoring.'}
            </AlertDescription>
          </Alert>

          <div className="grid grid-cols-1 lg:grid-cols-2 gap-4">
            <Card>
              <CardHeader>
                <CardTitle>Network Performance</CardTitle>
              </CardHeader>
              <CardContent>
                <div className="space-y-4">
                  <div>
                    <div className="flex justify-between text-sm mb-1">
                      <span>Latency</span>
                      <span>45ms</span>
                    </div>
                    <Progress value={45} />
                  </div>
                  <div>
                    <div className="flex justify-between text-sm mb-1">
                      <span>Throughput</span>
                      <span>2.1 Gbps</span>
                    </div>
                    <Progress value={70} />
                  </div>
                  <div>
                    <div className="flex justify-between text-sm mb-1">
                      <span>Packet Loss</span>
                      <span>0.1%</span>
                    </div>
                    <Progress value={10} />
                  </div>
                </div>
              </CardContent>
            </Card>

            <Card>
              <CardHeader>
                <CardTitle>Resource Utilization</CardTitle>
              </CardHeader>
              <CardContent>
                <div className="space-y-4">
                  <div>
                    <div className="flex justify-between text-sm mb-1">
                      <span>CPU Usage</span>
                      <span>45%</span>
                    </div>
                    <Progress value={45} />
                  </div>
                  <div>
                    <div className="flex justify-between text-sm mb-1">
                      <span>Memory Usage</span>
                      <span>60%</span>
                    </div>
                    <Progress value={60} />
                  </div>
                  <div>
                    <div className="flex justify-between text-sm mb-1">
                      <span>Network I/O</span>
                      <span>80%</span>
                    </div>
                    <Progress value={80} />
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
