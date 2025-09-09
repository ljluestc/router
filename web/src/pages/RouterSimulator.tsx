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
  Router, 
  Network, 
  Activity, 
  Settings, 
  Play, 
  Pause, 
  RefreshCw,
  Zap,
  Shield,
  Globe,
  Database,
  BarChart3,
  Terminal,
  FileText
} from 'lucide-react';

interface RouterInterface {
  name: string;
  ip: string;
  status: 'up' | 'down' | 'unknown';
  mtu: number;
  rx_bytes: number;
  tx_bytes: number;
  rx_packets: number;
  tx_packets: number;
  errors: number;
}

interface RoutingProtocol {
  name: string;
  type: 'BGP' | 'OSPF' | 'ISIS';
  status: 'running' | 'stopped' | 'error';
  neighbors: number;
  routes: number;
  uptime: number;
  last_update: string;
}

interface Route {
  destination: string;
  next_hop: string;
  interface: string;
  protocol: string;
  metric: number;
  age: number;
}

interface TrafficShaping {
  interface: string;
  algorithm: string;
  rate_bps: number;
  burst_size: number;
  tokens_available: number;
  packets_queued: number;
  packets_dropped: number;
  enabled: boolean;
}

interface NetworkImpairment {
  interface: string;
  type: 'delay' | 'loss' | 'duplicate' | 'corrupt' | 'reorder';
  value: number;
  enabled: boolean;
}

const RouterSimulator: React.FC = () => {
  const [interfaces, setInterfaces] = useState<RouterInterface[]>([]);
  const [protocols, setProtocols] = useState<RoutingProtocol[]>([]);
  const [routes, setRoutes] = useState<Route[]>([]);
  const [trafficShaping, setTrafficShaping] = useState<TrafficShaping[]>([]);
  const [impairments, setImpairments] = useState<NetworkImpairment[]>([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);
  const [simulationRunning, setSimulationRunning] = useState(false);

  useEffect(() => {
    loadRouterData();
    
    const interval = setInterval(() => {
      if (simulationRunning) {
        loadRouterData();
      }
    }, 2000);

    return () => clearInterval(interval);
  }, [simulationRunning]);

  const loadRouterData = async () => {
    try {
      const [interfacesRes, protocolsRes, routesRes, trafficRes, impairmentsRes] = await Promise.all([
        fetch('/api/router/interfaces'),
        fetch('/api/router/protocols'),
        fetch('/api/router/routes'),
        fetch('/api/router/traffic-shaping'),
        fetch('/api/router/impairments')
      ]);

      if (interfacesRes.ok) {
        const interfacesData = await interfacesRes.json();
        setInterfaces(interfacesData);
      }

      if (protocolsRes.ok) {
        const protocolsData = await protocolsRes.json();
        setProtocols(protocolsData);
      }

      if (routesRes.ok) {
        const routesData = await routesRes.json();
        setRoutes(routesData);
      }

      if (trafficRes.ok) {
        const trafficData = await trafficRes.json();
        setTrafficShaping(trafficData);
      }

      if (impairmentsRes.ok) {
        const impairmentsData = await impairmentsRes.json();
        setImpairments(impairmentsData);
      }
    } catch (err) {
      setError(err instanceof Error ? err.message : 'Failed to load router data');
    } finally {
      setLoading(false);
    }
  };

  const startSimulation = async () => {
    try {
      const response = await fetch('/api/router/simulation/start', {
        method: 'POST'
      });
      if (!response.ok) throw new Error('Failed to start simulation');
      setSimulationRunning(true);
    } catch (err) {
      setError(err instanceof Error ? err.message : 'Failed to start simulation');
    }
  };

  const stopSimulation = async () => {
    try {
      const response = await fetch('/api/router/simulation/stop', {
        method: 'POST'
      });
      if (!response.ok) throw new Error('Failed to stop simulation');
      setSimulationRunning(false);
    } catch (err) {
      setError(err instanceof Error ? err.message : 'Failed to stop simulation');
    }
  };

  const getStatusColor = (status: string) => {
    switch (status) {
      case 'up':
      case 'running':
        return 'bg-green-500';
      case 'down':
      case 'stopped':
        return 'bg-red-500';
      case 'error':
        return 'bg-red-500';
      case 'unknown':
        return 'bg-gray-500';
      default:
        return 'bg-gray-500';
    }
  };

  const getProtocolIcon = (type: string) => {
    switch (type) {
      case 'BGP': return <Globe className="h-4 w-4" />;
      case 'OSPF': return <Network className="h-4 w-4" />;
      case 'ISIS': return <Shield className="h-4 w-4" />;
      default: return <Router className="h-4 w-4" />;
    }
  };

  if (loading) {
    return (
      <div className="flex items-center justify-center h-64">
        <RefreshCw className="h-8 w-8 animate-spin" />
        <span className="ml-2">Loading router data...</span>
      </div>
    );
  }

  return (
    <div className="space-y-6">
      <div className="flex items-center justify-between">
        <div>
          <h1 className="text-3xl font-bold">Router Simulator</h1>
          <p className="text-muted-foreground">
            Multi-protocol router with FRR integration, traffic shaping, and network impairments
          </p>
        </div>
        <div className="flex space-x-2">
          <Button 
            onClick={simulationRunning ? stopSimulation : startSimulation}
            variant={simulationRunning ? "destructive" : "default"}
          >
            {simulationRunning ? (
              <>
                <Pause className="h-4 w-4 mr-2" />
                Stop Simulation
              </>
            ) : (
              <>
                <Play className="h-4 w-4 mr-2" />
                Start Simulation
              </>
            )}
          </Button>
          <Button onClick={loadRouterData} variant="outline" size="sm">
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

      {/* Status Overview */}
      <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-4 gap-4">
        <Card>
          <CardHeader className="flex flex-row items-center justify-between space-y-0 pb-2">
            <CardTitle className="text-sm font-medium">Interfaces</CardTitle>
            <Network className="h-4 w-4 text-muted-foreground" />
          </CardHeader>
          <CardContent>
            <div className="text-2xl font-bold">{interfaces.length}</div>
            <p className="text-xs text-muted-foreground">
              {interfaces.filter(i => i.status === 'up').length} up
            </p>
          </CardContent>
        </Card>

        <Card>
          <CardHeader className="flex flex-row items-center justify-between space-y-0 pb-2">
            <CardTitle className="text-sm font-medium">Protocols</CardTitle>
            <Router className="h-4 w-4 text-muted-foreground" />
          </CardHeader>
          <CardContent>
            <div className="text-2xl font-bold">{protocols.length}</div>
            <p className="text-xs text-muted-foreground">
              {protocols.filter(p => p.status === 'running').length} running
            </p>
          </CardContent>
        </Card>

        <Card>
          <CardHeader className="flex flex-row items-center justify-between space-y-0 pb-2">
            <CardTitle className="text-sm font-medium">Routes</CardTitle>
            <Globe className="h-4 w-4 text-muted-foreground" />
          </CardHeader>
          <CardContent>
            <div className="text-2xl font-bold">{routes.length}</div>
            <p className="text-xs text-muted-foreground">
              Active routing entries
            </p>
          </CardContent>
        </Card>

        <Card>
          <CardHeader className="flex flex-row items-center justify-between space-y-0 pb-2">
            <CardTitle className="text-sm font-medium">Simulation</CardTitle>
            <Activity className="h-4 w-4 text-muted-foreground" />
          </CardHeader>
          <CardContent>
            <div className="text-2xl font-bold">
              {simulationRunning ? 'Running' : 'Stopped'}
            </div>
            <p className="text-xs text-muted-foreground">
              {simulationRunning ? 'Active' : 'Inactive'}
            </p>
          </CardContent>
        </Card>
      </div>

      <Tabs defaultValue="interfaces" className="space-y-4">
        <TabsList>
          <TabsTrigger value="interfaces">Interfaces</TabsTrigger>
          <TabsTrigger value="protocols">Protocols</TabsTrigger>
          <TabsTrigger value="routes">Routes</TabsTrigger>
          <TabsTrigger value="traffic">Traffic Shaping</TabsTrigger>
          <TabsTrigger value="impairments">Impairments</TabsTrigger>
          <TabsTrigger value="terminal">Terminal</TabsTrigger>
        </TabsList>

        <TabsContent value="interfaces" className="space-y-4">
          <Card>
            <CardHeader>
              <CardTitle>Network Interfaces</CardTitle>
            </CardHeader>
            <CardContent>
              <div className="space-y-4">
                {interfaces.map((iface) => (
                  <div key={iface.name} className="flex items-center justify-between p-4 border rounded-lg">
                    <div className="flex items-center space-x-4">
                      <div className={`w-3 h-3 rounded-full ${getStatusColor(iface.status)}`} />
                      <div>
                        <div className="font-medium">{iface.name}</div>
                        <div className="text-sm text-muted-foreground">{iface.ip}</div>
                      </div>
                    </div>
                    <div className="flex items-center space-x-6">
                      <div className="text-right">
                        <div className="text-sm font-medium">MTU: {iface.mtu}</div>
                        <div className="text-xs text-muted-foreground">
                          RX: {(iface.rx_bytes / 1024 / 1024).toFixed(1)} MB
                        </div>
                        <div className="text-xs text-muted-foreground">
                          TX: {(iface.tx_bytes / 1024 / 1024).toFixed(1)} MB
                        </div>
                      </div>
                      <div className="text-right">
                        <div className="text-sm font-medium">
                          {iface.rx_packets.toLocaleString()} packets
                        </div>
                        <div className="text-xs text-muted-foreground">
                          {iface.tx_packets.toLocaleString()} sent
                        </div>
                        {iface.errors > 0 && (
                          <div className="text-xs text-red-500">
                            {iface.errors} errors
                          </div>
                        )}
                      </div>
                    </div>
                  </div>
                ))}
              </div>
            </CardContent>
          </Card>
        </TabsContent>

        <TabsContent value="protocols" className="space-y-4">
          <Card>
            <CardHeader>
              <CardTitle>Routing Protocols</CardTitle>
            </CardHeader>
            <CardContent>
              <div className="space-y-4">
                {protocols.map((protocol) => (
                  <div key={protocol.name} className="flex items-center justify-between p-4 border rounded-lg">
                    <div className="flex items-center space-x-4">
                      {getProtocolIcon(protocol.type)}
                      <div>
                        <div className="font-medium">{protocol.name}</div>
                        <div className="text-sm text-muted-foreground">{protocol.type}</div>
                      </div>
                    </div>
                    <div className="flex items-center space-x-4">
                      <Badge className={`${getStatusColor(protocol.status)} text-white`}>
                        {protocol.status}
                      </Badge>
                      <div className="text-right">
                        <div className="text-sm font-medium">
                          {protocol.neighbors} neighbors
                        </div>
                        <div className="text-xs text-muted-foreground">
                          {protocol.routes} routes
                        </div>
                        <div className="text-xs text-muted-foreground">
                          Uptime: {Math.floor(protocol.uptime / 3600)}h
                        </div>
                      </div>
                    </div>
                  </div>
                ))}
              </div>
            </CardContent>
          </Card>
        </TabsContent>

        <TabsContent value="routes" className="space-y-4">
          <Card>
            <CardHeader>
              <CardTitle>Routing Table</CardTitle>
            </CardHeader>
            <CardContent>
              <div className="overflow-x-auto">
                <table className="w-full">
                  <thead>
                    <tr className="border-b">
                      <th className="text-left p-2">Destination</th>
                      <th className="text-left p-2">Next Hop</th>
                      <th className="text-left p-2">Interface</th>
                      <th className="text-left p-2">Protocol</th>
                      <th className="text-left p-2">Metric</th>
                      <th className="text-left p-2">Age</th>
                    </tr>
                  </thead>
                  <tbody>
                    {routes.map((route, index) => (
                      <tr key={index} className="border-b">
                        <td className="p-2 font-mono">{route.destination}</td>
                        <td className="p-2 font-mono">{route.next_hop}</td>
                        <td className="p-2">{route.interface}</td>
                        <td className="p-2">
                          <Badge variant="outline">{route.protocol}</Badge>
                        </td>
                        <td className="p-2">{route.metric}</td>
                        <td className="p-2">{route.age}s</td>
                      </tr>
                    ))}
                  </tbody>
                </table>
              </div>
            </CardContent>
          </Card>
        </TabsContent>

        <TabsContent value="traffic" className="space-y-4">
          <Card>
            <CardHeader>
              <CardTitle>Traffic Shaping</CardTitle>
            </CardHeader>
            <CardContent>
              <div className="space-y-4">
                {trafficShaping.map((shaping) => (
                  <div key={shaping.interface} className="p-4 border rounded-lg">
                    <div className="flex items-center justify-between mb-4">
                      <div className="flex items-center space-x-2">
                        <Zap className="h-4 w-4" />
                        <span className="font-medium">{shaping.interface}</span>
                        <Badge variant={shaping.enabled ? "default" : "secondary"}>
                          {shaping.enabled ? "Enabled" : "Disabled"}
                        </Badge>
                      </div>
                      <div className="text-sm text-muted-foreground">
                        {shaping.algorithm}
                      </div>
                    </div>
                    <div className="grid grid-cols-2 md:grid-cols-4 gap-4">
                      <div>
                        <div className="text-sm text-muted-foreground">Rate</div>
                        <div className="font-medium">
                          {(shaping.rate_bps / 1000000).toFixed(1)} Mbps
                        </div>
                      </div>
                      <div>
                        <div className="text-sm text-muted-foreground">Burst Size</div>
                        <div className="font-medium">
                          {(shaping.burst_size / 1024).toFixed(1)} KB
                        </div>
                      </div>
                      <div>
                        <div className="text-sm text-muted-foreground">Tokens</div>
                        <div className="font-medium">
                          {shaping.tokens_available}
                        </div>
                      </div>
                      <div>
                        <div className="text-sm text-muted-foreground">Queued</div>
                        <div className="font-medium">
                          {shaping.packets_queued}
                        </div>
                      </div>
                    </div>
                    <div className="mt-4">
                      <div className="flex justify-between text-sm text-muted-foreground mb-2">
                        <span>Packets Dropped</span>
                        <span>{shaping.packets_dropped}</span>
                      </div>
                      <Progress 
                        value={(shaping.packets_dropped / (shaping.packets_queued + shaping.packets_dropped)) * 100} 
                        className="h-2"
                      />
                    </div>
                  </div>
                ))}
              </div>
            </CardContent>
          </Card>
        </TabsContent>

        <TabsContent value="impairments" className="space-y-4">
          <Card>
            <CardHeader>
              <CardTitle>Network Impairments</CardTitle>
            </CardHeader>
            <CardContent>
              <div className="space-y-4">
                {impairments.map((impairment) => (
                  <div key={`${impairment.interface}-${impairment.type}`} className="p-4 border rounded-lg">
                    <div className="flex items-center justify-between mb-4">
                      <div className="flex items-center space-x-2">
                        <Settings className="h-4 w-4" />
                        <span className="font-medium">{impairment.interface}</span>
                        <Badge variant="outline">{impairment.type}</Badge>
                        <Badge variant={impairment.enabled ? "default" : "secondary"}>
                          {impairment.enabled ? "Enabled" : "Disabled"}
                        </Badge>
                      </div>
                      <div className="text-sm text-muted-foreground">
                        Value: {impairment.value}
                      </div>
                    </div>
                  </div>
                ))}
              </div>
            </CardContent>
          </Card>
        </TabsContent>

        <TabsContent value="terminal" className="space-y-4">
          <Card>
            <CardHeader>
              <CardTitle>Router Terminal</CardTitle>
            </CardHeader>
            <CardContent>
              <div className="h-96 bg-black text-green-400 font-mono p-4 rounded-lg overflow-auto">
                <div className="space-y-2">
                  <div>Router Simulator Terminal</div>
                  <div>Type 'help' for available commands</div>
                  <div className="flex items-center">
                    <span className="text-yellow-400">router#</span>
                    <span className="ml-2">show interfaces</span>
                  </div>
                  <div className="text-gray-400">
                    {interfaces.map(iface => (
                      <div key={iface.name}>
                        {iface.name}: {iface.status} - {iface.ip}
                      </div>
                    ))}
                  </div>
                  <div className="flex items-center">
                    <span className="text-yellow-400">router#</span>
                    <span className="ml-2">show ip route</span>
                  </div>
                  <div className="text-gray-400">
                    {routes.slice(0, 5).map((route, i) => (
                      <div key={i}>
                        {route.destination} via {route.next_hop} {route.interface}
                      </div>
                    ))}
                  </div>
                  <div className="flex items-center">
                    <span className="text-yellow-400">router#</span>
                    <span className="ml-2 animate-pulse">_</span>
                  </div>
                </div>
              </div>
            </CardContent>
          </Card>
        </TabsContent>
      </Tabs>
    </div>
  );
};

export default RouterSimulator;
