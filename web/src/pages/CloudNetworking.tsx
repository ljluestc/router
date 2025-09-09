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
  Activity, 
  Shield, 
  Zap,
  Globe,
  Database,
  BarChart3,
  Settings,
  Play,
  Pause,
  RefreshCw
} from 'lucide-react';

interface CloudResource {
  id: string;
  name: string;
  type: 'vm' | 'network' | 'gateway' | 'vpc';
  status: 'running' | 'stopped' | 'error' | 'pending';
  region: string;
  provider: 'cloudpods' | 'aviatrix' | 'aws' | 'azure' | 'gcp';
  cpu: number;
  memory: number;
  network: {
    bytesIn: number;
    bytesOut: number;
    packetsIn: number;
    packetsOut: number;
  };
  lastUpdated: string;
}

interface NetworkTopology {
  nodes: Array<{
    id: string;
    label: string;
    type: 'router' | 'gateway' | 'vm' | 'network';
    x: number;
    y: number;
    status: string;
    provider: string;
  }>;
  edges: Array<{
    source: string;
    target: string;
    bandwidth: number;
    latency: number;
    protocol: string;
  }>;
}

interface Metrics {
  timestamp: string;
  cpu: number;
  memory: number;
  network: number;
  packets: number;
  latency: number;
}

const CloudNetworking: React.FC = () => {
  const [resources, setResources] = useState<CloudResource[]>([]);
  const [topology, setTopology] = useState<NetworkTopology>({ nodes: [], edges: [] });
  const [metrics, setMetrics] = useState<Metrics[]>([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);
  const [selectedProvider, setSelectedProvider] = useState<string>('all');

  useEffect(() => {
    loadCloudResources();
    loadNetworkTopology();
    loadMetrics();
    
    const interval = setInterval(() => {
      loadMetrics();
    }, 5000);

    return () => clearInterval(interval);
  }, []);

  const loadCloudResources = async () => {
    try {
      const response = await fetch('/api/cloud/resources');
      if (!response.ok) throw new Error('Failed to load resources');
      const data = await response.json();
      setResources(data);
    } catch (err) {
      setError(err instanceof Error ? err.message : 'Failed to load resources');
    }
  };

  const loadNetworkTopology = async () => {
    try {
      const response = await fetch('/api/network/topology');
      if (!response.ok) throw new Error('Failed to load topology');
      const data = await response.json();
      setTopology(data);
    } catch (err) {
      setError(err instanceof Error ? err.message : 'Failed to load topology');
    }
  };

  const loadMetrics = async () => {
    try {
      const response = await fetch('/api/metrics/cloud');
      if (!response.ok) throw new Error('Failed to load metrics');
      const data = await response.json();
      setMetrics(data);
    } catch (err) {
      console.error('Failed to load metrics:', err);
    } finally {
      setLoading(false);
    }
  };

  const getStatusColor = (status: string) => {
    switch (status) {
      case 'running': return 'bg-green-500';
      case 'stopped': return 'bg-gray-500';
      case 'error': return 'bg-red-500';
      case 'pending': return 'bg-yellow-500';
      default: return 'bg-gray-500';
    }
  };

  const getProviderIcon = (provider: string) => {
    switch (provider) {
      case 'cloudpods': return <Cloud className="h-4 w-4" />;
      case 'aviatrix': return <Shield className="h-4 w-4" />;
      case 'aws': return <Globe className="h-4 w-4" />;
      case 'azure': return <Server className="h-4 w-4" />;
      case 'gcp': return <Database className="h-4 w-4" />;
      default: return <Network className="h-4 w-4" />;
    }
  };

  const filteredResources = selectedProvider === 'all' 
    ? resources 
    : resources.filter(r => r.provider === selectedProvider);

  const currentMetrics = metrics[metrics.length - 1] || {
    timestamp: new Date().toISOString(),
    cpu: 0,
    memory: 0,
    network: 0,
    packets: 0,
    latency: 0
  };

  if (loading) {
    return (
      <div className="flex items-center justify-center h-64">
        <RefreshCw className="h-8 w-8 animate-spin" />
        <span className="ml-2">Loading cloud networking data...</span>
      </div>
    );
  }

  return (
    <div className="space-y-6">
      <div className="flex items-center justify-between">
        <div>
          <h1 className="text-3xl font-bold">Cloud Networking</h1>
          <p className="text-muted-foreground">
            Multi-cloud networking platform with CloudPods and Aviatrix integration
          </p>
        </div>
        <div className="flex space-x-2">
          <Button onClick={loadCloudResources} variant="outline" size="sm">
            <RefreshCw className="h-4 w-4 mr-2" />
            Refresh
          </Button>
        </div>
      </div>

      {error && (
        <Alert variant="destructive">
          <AlertDescription>{error}</AlertDescription>
        </Alert>
      )}

      {/* Overview Cards */}
      <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-4 gap-4">
        <Card>
          <CardHeader className="flex flex-row items-center justify-between space-y-0 pb-2">
            <CardTitle className="text-sm font-medium">Total Resources</CardTitle>
            <Server className="h-4 w-4 text-muted-foreground" />
          </CardHeader>
          <CardContent>
            <div className="text-2xl font-bold">{resources.length}</div>
            <p className="text-xs text-muted-foreground">
              {resources.filter(r => r.status === 'running').length} running
            </p>
          </CardContent>
        </Card>

        <Card>
          <CardHeader className="flex flex-row items-center justify-between space-y-0 pb-2">
            <CardTitle className="text-sm font-medium">CPU Usage</CardTitle>
            <Activity className="h-4 w-4 text-muted-foreground" />
          </CardHeader>
          <CardContent>
            <div className="text-2xl font-bold">{currentMetrics.cpu.toFixed(1)}%</div>
            <Progress value={currentMetrics.cpu} className="mt-2" />
          </CardContent>
        </Card>

        <Card>
          <CardHeader className="flex flex-row items-center justify-between space-y-0 pb-2">
            <CardTitle className="text-sm font-medium">Memory Usage</CardTitle>
            <Database className="h-4 w-4 text-muted-foreground" />
          </CardHeader>
          <CardContent>
            <div className="text-2xl font-bold">{currentMetrics.memory.toFixed(1)}%</div>
            <Progress value={currentMetrics.memory} className="mt-2" />
          </CardContent>
        </Card>

        <Card>
          <CardHeader className="flex flex-row items-center justify-between space-y-0 pb-2">
            <CardTitle className="text-sm font-medium">Network Throughput</CardTitle>
            <Network className="h-4 w-4 text-muted-foreground" />
          </CardHeader>
          <CardContent>
            <div className="text-2xl font-bold">
              {(currentMetrics.network / 1024 / 1024).toFixed(1)} MB/s
            </div>
            <p className="text-xs text-muted-foreground">
              {currentMetrics.packets.toFixed(0)} packets/s
            </p>
          </CardContent>
        </Card>
      </div>

      <Tabs defaultValue="resources" className="space-y-4">
        <TabsList>
          <TabsTrigger value="resources">Resources</TabsTrigger>
          <TabsTrigger value="topology">Network Topology</TabsTrigger>
          <TabsTrigger value="metrics">Metrics</TabsTrigger>
          <TabsTrigger value="settings">Settings</TabsTrigger>
        </TabsList>

        <TabsContent value="resources" className="space-y-4">
          <div className="flex items-center space-x-2">
            <label className="text-sm font-medium">Filter by Provider:</label>
            <select
              value={selectedProvider}
              onChange={(e) => setSelectedProvider(e.target.value)}
              className="px-3 py-1 border rounded-md"
            >
              <option value="all">All Providers</option>
              <option value="cloudpods">CloudPods</option>
              <option value="aviatrix">Aviatrix</option>
              <option value="aws">AWS</option>
              <option value="azure">Azure</option>
              <option value="gcp">GCP</option>
            </select>
          </div>

          <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4">
            {filteredResources.map((resource) => (
              <Card key={resource.id} className="hover:shadow-lg transition-shadow">
                <CardHeader className="pb-3">
                  <div className="flex items-center justify-between">
                    <CardTitle className="text-lg">{resource.name}</CardTitle>
                    <div className="flex items-center space-x-2">
                      {getProviderIcon(resource.provider)}
                      <Badge 
                        className={`${getStatusColor(resource.status)} text-white`}
                      >
                        {resource.status}
                      </Badge>
                    </div>
                  </div>
                </CardHeader>
                <CardContent className="space-y-3">
                  <div className="flex justify-between text-sm">
                    <span className="text-muted-foreground">Type:</span>
                    <span className="font-medium">{resource.type.toUpperCase()}</span>
                  </div>
                  <div className="flex justify-between text-sm">
                    <span className="text-muted-foreground">Region:</span>
                    <span className="font-medium">{resource.region}</span>
                  </div>
                  <div className="flex justify-between text-sm">
                    <span className="text-muted-foreground">CPU:</span>
                    <span className="font-medium">{resource.cpu}%</span>
                  </div>
                  <div className="flex justify-between text-sm">
                    <span className="text-muted-foreground">Memory:</span>
                    <span className="font-medium">{resource.memory}%</span>
                  </div>
                  <div className="space-y-1">
                    <div className="flex justify-between text-xs text-muted-foreground">
                      <span>Network In:</span>
                      <span>{(resource.network.bytesIn / 1024 / 1024).toFixed(1)} MB</span>
                    </div>
                    <div className="flex justify-between text-xs text-muted-foreground">
                      <span>Network Out:</span>
                      <span>{(resource.network.bytesOut / 1024 / 1024).toFixed(1)} MB</span>
                    </div>
                  </div>
                  <div className="pt-2 border-t">
                    <div className="flex justify-between text-xs text-muted-foreground">
                      <span>Last Updated:</span>
                      <span>{new Date(resource.lastUpdated).toLocaleTimeString()}</span>
                    </div>
                  </div>
                </CardContent>
              </Card>
            ))}
          </div>
        </TabsContent>

        <TabsContent value="topology" className="space-y-4">
          <Card>
            <CardHeader>
              <CardTitle>Network Topology</CardTitle>
            </CardHeader>
            <CardContent>
              <div className="h-96 border rounded-lg flex items-center justify-center">
                <div className="text-center">
                  <Network className="h-12 w-12 mx-auto text-muted-foreground mb-4" />
                  <p className="text-muted-foreground">
                    Network topology visualization will be rendered here
                  </p>
                  <p className="text-sm text-muted-foreground mt-2">
                    {topology.nodes.length} nodes, {topology.edges.length} connections
                  </p>
                </div>
              </div>
            </CardContent>
          </Card>
        </TabsContent>

        <TabsContent value="metrics" className="space-y-4">
          <Card>
            <CardHeader>
              <CardTitle>Real-time Metrics</CardTitle>
            </CardHeader>
            <CardContent>
              <div className="h-96 border rounded-lg flex items-center justify-center">
                <div className="text-center">
                  <BarChart3 className="h-12 w-12 mx-auto text-muted-foreground mb-4" />
                  <p className="text-muted-foreground">
                    Real-time metrics charts will be rendered here
                  </p>
                  <p className="text-sm text-muted-foreground mt-2">
                    {metrics.length} data points collected
                  </p>
                </div>
              </div>
            </CardContent>
          </Card>
        </TabsContent>

        <TabsContent value="settings" className="space-y-4">
          <Card>
            <CardHeader>
              <CardTitle>Cloud Provider Settings</CardTitle>
            </CardHeader>
            <CardContent className="space-y-4">
              <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
                <div className="space-y-2">
                  <label className="text-sm font-medium">CloudPods Configuration</label>
                  <div className="space-y-2">
                    <input
                      type="text"
                      placeholder="API Endpoint"
                      className="w-full px-3 py-2 border rounded-md"
                    />
                    <input
                      type="text"
                      placeholder="Username"
                      className="w-full px-3 py-2 border rounded-md"
                    />
                    <input
                      type="password"
                      placeholder="Password"
                      className="w-full px-3 py-2 border rounded-md"
                    />
                  </div>
                </div>
                <div className="space-y-2">
                  <label className="text-sm font-medium">Aviatrix Configuration</label>
                  <div className="space-y-2">
                    <input
                      type="text"
                      placeholder="Controller URL"
                      className="w-full px-3 py-2 border rounded-md"
                    />
                    <input
                      type="text"
                      placeholder="CoPilot URL"
                      className="w-full px-3 py-2 border rounded-md"
                    />
                    <input
                      type="text"
                      placeholder="Username"
                      className="w-full px-3 py-2 border rounded-md"
                    />
                    <input
                      type="password"
                      placeholder="Password"
                      className="w-full px-3 py-2 border rounded-md"
                    />
                  </div>
                </div>
              </div>
              <div className="flex justify-end space-x-2">
                <Button variant="outline">Cancel</Button>
                <Button>Save Configuration</Button>
              </div>
            </CardContent>
          </Card>
        </TabsContent>
      </Tabs>
    </div>
  );
};

export default CloudNetworking;
