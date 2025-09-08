import React, { useState, useEffect } from 'react';
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card';
import { Badge } from '@/components/ui/badge';
import { Button } from '@/components/ui/button';
import { Tabs, TabsContent, TabsList, TabsTrigger } from '@/components/ui/tabs';
import { 
  Activity, 
  Network, 
  Server, 
  Cpu, 
  HardDrive, 
  Wifi, 
  AlertTriangle,
  CheckCircle,
  XCircle,
  RefreshCw
} from 'lucide-react';

interface SystemStats {
  cpu_usage: number;
  memory_usage: number;
  disk_usage: number;
  network_in: number;
  network_out: number;
  uptime: string;
}

interface RouterStats {
  total_routes: number;
  bgp_routes: number;
  ospf_routes: number;
  isis_routes: number;
  static_routes: number;
  interfaces_up: number;
  interfaces_total: number;
  packets_processed: number;
  packets_dropped: number;
  bytes_processed: number;
}

interface ProtocolStatus {
  bgp: {
    enabled: boolean;
    neighbors: number;
    routes: number;
    state: 'up' | 'down' | 'warning';
  };
  ospf: {
    enabled: boolean;
    areas: number;
    routes: number;
    state: 'up' | 'down' | 'warning';
  };
  isis: {
    enabled: boolean;
    interfaces: number;
    routes: number;
    state: 'up' | 'down' | 'warning';
  };
}

interface TrafficStats {
  total_throughput: number;
  peak_throughput: number;
  avg_latency: number;
  packet_loss_rate: number;
  jitter: number;
}

const Dashboard: React.FC = () => {
  const [systemStats, setSystemStats] = useState<SystemStats>({
    cpu_usage: 0,
    memory_usage: 0,
    disk_usage: 0,
    network_in: 0,
    network_out: 0,
    uptime: '0d 0h 0m'
  });

  const [routerStats, setRouterStats] = useState<RouterStats>({
    total_routes: 0,
    bgp_routes: 0,
    ospf_routes: 0,
    isis_routes: 0,
    static_routes: 0,
    interfaces_up: 0,
    interfaces_total: 0,
    packets_processed: 0,
    packets_dropped: 0,
    bytes_processed: 0
  });

  const [protocolStatus, setProtocolStatus] = useState<ProtocolStatus>({
    bgp: { enabled: false, neighbors: 0, routes: 0, state: 'down' },
    ospf: { enabled: false, areas: 0, routes: 0, state: 'down' },
    isis: { enabled: false, interfaces: 0, routes: 0, state: 'down' }
  });

  const [trafficStats, setTrafficStats] = useState<TrafficStats>({
    total_throughput: 0,
    peak_throughput: 0,
    avg_latency: 0,
    packet_loss_rate: 0,
    jitter: 0
  });

  const [isRefreshing, setIsRefreshing] = useState(false);

  useEffect(() => {
    fetchDashboardData();
    const interval = setInterval(fetchDashboardData, 5000); // Refresh every 5 seconds
    return () => clearInterval(interval);
  }, []);

  const fetchDashboardData = async () => {
    try {
      setIsRefreshing(true);
      
      // Fetch system statistics
      const systemResponse = await fetch('/api/system/stats');
      if (systemResponse.ok) {
        const systemData = await systemResponse.json();
        setSystemStats(systemData);
      }

      // Fetch router statistics
      const routerResponse = await fetch('/api/router/stats');
      if (routerResponse.ok) {
        const routerData = await routerResponse.json();
        setRouterStats(routerData);
      }

      // Fetch protocol status
      const protocolResponse = await fetch('/api/protocols/status');
      if (protocolResponse.ok) {
        const protocolData = await protocolResponse.json();
        setProtocolStatus(protocolData);
      }

      // Fetch traffic statistics
      const trafficResponse = await fetch('/api/traffic/stats');
      if (trafficResponse.ok) {
        const trafficData = await trafficResponse.json();
        setTrafficStats(trafficData);
      }
    } catch (error) {
      console.error('Error fetching dashboard data:', error);
    } finally {
      setIsRefreshing(false);
    }
  };

  const getStatusIcon = (state: string) => {
    switch (state) {
      case 'up':
        return <CheckCircle className="h-4 w-4 text-green-500" />;
      case 'warning':
        return <AlertTriangle className="h-4 w-4 text-yellow-500" />;
      case 'down':
        return <XCircle className="h-4 w-4 text-red-500" />;
      default:
        return <XCircle className="h-4 w-4 text-gray-500" />;
    }
  };

  const getStatusColor = (state: string) => {
    switch (state) {
      case 'up':
        return 'bg-green-100 text-green-800';
      case 'warning':
        return 'bg-yellow-100 text-yellow-800';
      case 'down':
        return 'bg-red-100 text-red-800';
      default:
        return 'bg-gray-100 text-gray-800';
    }
  };

  const formatBytes = (bytes: number) => {
    if (bytes === 0) return '0 B';
    const k = 1024;
    const sizes = ['B', 'KB', 'MB', 'GB', 'TB'];
    const i = Math.floor(Math.log(bytes) / Math.log(k));
    return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + ' ' + sizes[i];
  };

  const formatRate = (rate: number) => {
    return formatBytes(rate) + '/s';
  };

  return (
    <div className="space-y-6">
      {/* Header */}
      <div className="flex items-center justify-between">
        <div>
          <h1 className="text-3xl font-bold">Router Dashboard</h1>
          <p className="text-gray-600">Real-time monitoring and control</p>
        </div>
        <Button 
          onClick={fetchDashboardData} 
          disabled={isRefreshing}
          className="flex items-center gap-2"
        >
          <RefreshCw className={`h-4 w-4 ${isRefreshing ? 'animate-spin' : ''}`} />
          Refresh
        </Button>
      </div>

      {/* System Overview */}
      <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-4 gap-4">
        <Card>
          <CardHeader className="flex flex-row items-center justify-between space-y-0 pb-2">
            <CardTitle className="text-sm font-medium">CPU Usage</CardTitle>
            <Cpu className="h-4 w-4 text-muted-foreground" />
          </CardHeader>
          <CardContent>
            <div className="text-2xl font-bold">{systemStats.cpu_usage.toFixed(1)}%</div>
            <div className="w-full bg-gray-200 rounded-full h-2 mt-2">
              <div 
                className="bg-blue-600 h-2 rounded-full" 
                style={{ width: `${systemStats.cpu_usage}%` }}
              ></div>
            </div>
          </CardContent>
        </Card>

        <Card>
          <CardHeader className="flex flex-row items-center justify-between space-y-0 pb-2">
            <CardTitle className="text-sm font-medium">Memory Usage</CardTitle>
            <HardDrive className="h-4 w-4 text-muted-foreground" />
          </CardHeader>
          <CardContent>
            <div className="text-2xl font-bold">{systemStats.memory_usage.toFixed(1)}%</div>
            <div className="w-full bg-gray-200 rounded-full h-2 mt-2">
              <div 
                className="bg-green-600 h-2 rounded-full" 
                style={{ width: `${systemStats.memory_usage}%` }}
              ></div>
            </div>
          </CardContent>
        </Card>

        <Card>
          <CardHeader className="flex flex-row items-center justify-between space-y-0 pb-2">
            <CardTitle className="text-sm font-medium">Network In</CardTitle>
            <Wifi className="h-4 w-4 text-muted-foreground" />
          </CardHeader>
          <CardContent>
            <div className="text-2xl font-bold">{formatRate(systemStats.network_in)}</div>
            <p className="text-xs text-muted-foreground">Current rate</p>
          </CardContent>
        </Card>

        <Card>
          <CardHeader className="flex flex-row items-center justify-between space-y-0 pb-2">
            <CardTitle className="text-sm font-medium">Network Out</CardTitle>
            <Wifi className="h-4 w-4 text-muted-foreground" />
          </CardHeader>
          <CardContent>
            <div className="text-2xl font-bold">{formatRate(systemStats.network_out)}</div>
            <p className="text-xs text-muted-foreground">Current rate</p>
          </CardContent>
        </Card>
      </div>

      {/* Main Content Tabs */}
      <Tabs defaultValue="overview" className="space-y-4">
        <TabsList>
          <TabsTrigger value="overview">Overview</TabsTrigger>
          <TabsTrigger value="routing">Routing</TabsTrigger>
          <TabsTrigger value="traffic">Traffic</TabsTrigger>
          <TabsTrigger value="protocols">Protocols</TabsTrigger>
        </TabsList>

        <TabsContent value="overview" className="space-y-4">
          <div className="grid grid-cols-1 lg:grid-cols-2 gap-4">
            {/* Router Statistics */}
            <Card>
              <CardHeader>
                <CardTitle className="flex items-center gap-2">
                  <Server className="h-5 w-5" />
                  Router Statistics
                </CardTitle>
              </CardHeader>
              <CardContent className="space-y-4">
                <div className="grid grid-cols-2 gap-4">
                  <div>
                    <p className="text-sm text-gray-600">Total Routes</p>
                    <p className="text-2xl font-bold">{routerStats.total_routes.toLocaleString()}</p>
                  </div>
                  <div>
                    <p className="text-sm text-gray-600">Interfaces Up</p>
                    <p className="text-2xl font-bold">
                      {routerStats.interfaces_up}/{routerStats.interfaces_total}
                    </p>
                  </div>
                </div>
                <div className="grid grid-cols-2 gap-4">
                  <div>
                    <p className="text-sm text-gray-600">Packets Processed</p>
                    <p className="text-lg font-semibold">{routerStats.packets_processed.toLocaleString()}</p>
                  </div>
                  <div>
                    <p className="text-sm text-gray-600">Packets Dropped</p>
                    <p className="text-lg font-semibold text-red-600">
                      {routerStats.packets_dropped.toLocaleString()}
                    </p>
                  </div>
                </div>
                <div>
                  <p className="text-sm text-gray-600">Bytes Processed</p>
                  <p className="text-lg font-semibold">{formatBytes(routerStats.bytes_processed)}</p>
                </div>
              </CardContent>
            </Card>

            {/* Protocol Status */}
            <Card>
              <CardHeader>
                <CardTitle className="flex items-center gap-2">
                  <Network className="h-5 w-5" />
                  Protocol Status
                </CardTitle>
              </CardHeader>
              <CardContent className="space-y-4">
                {Object.entries(protocolStatus).map(([protocol, status]) => (
                  <div key={protocol} className="flex items-center justify-between">
                    <div className="flex items-center gap-2">
                      {getStatusIcon(status.state)}
                      <span className="font-medium capitalize">{protocol.toUpperCase()}</span>
                    </div>
                    <div className="flex items-center gap-2">
                      <Badge className={getStatusColor(status.state)}>
                        {status.state}
                      </Badge>
                      <span className="text-sm text-gray-600">
                        {protocol === 'bgp' ? status.neighbors : 
                         protocol === 'ospf' ? status.areas : status.interfaces} active
                      </span>
                    </div>
                  </div>
                ))}
              </CardContent>
            </Card>
          </div>
        </TabsContent>

        <TabsContent value="routing" className="space-y-4">
          <div className="grid grid-cols-1 lg:grid-cols-2 gap-4">
            {/* Route Distribution */}
            <Card>
              <CardHeader>
                <CardTitle>Route Distribution</CardTitle>
              </CardHeader>
              <CardContent>
                <div className="space-y-3">
                  <div className="flex justify-between items-center">
                    <span>BGP Routes</span>
                    <span className="font-semibold">{routerStats.bgp_routes.toLocaleString()}</span>
                  </div>
                  <div className="flex justify-between items-center">
                    <span>OSPF Routes</span>
                    <span className="font-semibold">{routerStats.ospf_routes.toLocaleString()}</span>
                  </div>
                  <div className="flex justify-between items-center">
                    <span>ISIS Routes</span>
                    <span className="font-semibold">{routerStats.isis_routes.toLocaleString()}</span>
                  </div>
                  <div className="flex justify-between items-center">
                    <span>Static Routes</span>
                    <span className="font-semibold">{routerStats.static_routes.toLocaleString()}</span>
                  </div>
                </div>
              </CardContent>
            </Card>

            {/* Interface Status */}
            <Card>
              <CardHeader>
                <CardTitle>Interface Status</CardTitle>
              </CardHeader>
              <CardContent>
                <div className="space-y-3">
                  <div className="flex justify-between items-center">
                    <span>Total Interfaces</span>
                    <span className="font-semibold">{routerStats.interfaces_total}</span>
                  </div>
                  <div className="flex justify-between items-center">
                    <span>Up Interfaces</span>
                    <span className="font-semibold text-green-600">{routerStats.interfaces_up}</span>
                  </div>
                  <div className="flex justify-between items-center">
                    <span>Down Interfaces</span>
                    <span className="font-semibold text-red-600">
                      {routerStats.interfaces_total - routerStats.interfaces_up}
                    </span>
                  </div>
                </div>
              </CardContent>
            </Card>
          </div>
        </TabsContent>

        <TabsContent value="traffic" className="space-y-4">
          <div className="grid grid-cols-1 lg:grid-cols-2 gap-4">
            {/* Traffic Statistics */}
            <Card>
              <CardHeader>
                <CardTitle>Traffic Statistics</CardTitle>
              </CardHeader>
              <CardContent className="space-y-4">
                <div>
                  <p className="text-sm text-gray-600">Total Throughput</p>
                  <p className="text-2xl font-bold">{formatRate(trafficStats.total_throughput)}</p>
                </div>
                <div>
                  <p className="text-sm text-gray-600">Peak Throughput</p>
                  <p className="text-lg font-semibold">{formatRate(trafficStats.peak_throughput)}</p>
                </div>
                <div>
                  <p className="text-sm text-gray-600">Average Latency</p>
                  <p className="text-lg font-semibold">{trafficStats.avg_latency.toFixed(2)} ms</p>
                </div>
                <div>
                  <p className="text-sm text-gray-600">Packet Loss Rate</p>
                  <p className="text-lg font-semibold text-red-600">
                    {(trafficStats.packet_loss_rate * 100).toFixed(3)}%
                  </p>
                </div>
                <div>
                  <p className="text-sm text-gray-600">Jitter</p>
                  <p className="text-lg font-semibold">{trafficStats.jitter.toFixed(2)} ms</p>
                </div>
              </CardContent>
            </Card>

            {/* Traffic Chart Placeholder */}
            <Card>
              <CardHeader>
                <CardTitle>Traffic Over Time</CardTitle>
              </CardHeader>
              <CardContent>
                <div className="h-64 flex items-center justify-center bg-gray-50 rounded-lg">
                  <p className="text-gray-500">Traffic chart will be implemented here</p>
                </div>
              </CardContent>
            </Card>
          </div>
        </TabsContent>

        <TabsContent value="protocols" className="space-y-4">
          <div className="grid grid-cols-1 lg:grid-cols-3 gap-4">
            {/* BGP Status */}
            <Card>
              <CardHeader>
                <CardTitle className="flex items-center gap-2">
                  {getStatusIcon(protocolStatus.bgp.state)}
                  BGP Status
                </CardTitle>
              </CardHeader>
              <CardContent className="space-y-3">
                <div className="flex justify-between">
                  <span>Neighbors</span>
                  <span className="font-semibold">{protocolStatus.bgp.neighbors}</span>
                </div>
                <div className="flex justify-between">
                  <span>Routes</span>
                  <span className="font-semibold">{protocolStatus.bgp.routes.toLocaleString()}</span>
                </div>
                <div className="flex justify-between">
                  <span>Status</span>
                  <Badge className={getStatusColor(protocolStatus.bgp.state)}>
                    {protocolStatus.bgp.state}
                  </Badge>
                </div>
              </CardContent>
            </Card>

            {/* OSPF Status */}
            <Card>
              <CardHeader>
                <CardTitle className="flex items-center gap-2">
                  {getStatusIcon(protocolStatus.ospf.state)}
                  OSPF Status
                </CardTitle>
              </CardHeader>
              <CardContent className="space-y-3">
                <div className="flex justify-between">
                  <span>Areas</span>
                  <span className="font-semibold">{protocolStatus.ospf.areas}</span>
                </div>
                <div className="flex justify-between">
                  <span>Routes</span>
                  <span className="font-semibold">{protocolStatus.ospf.routes.toLocaleString()}</span>
                </div>
                <div className="flex justify-between">
                  <span>Status</span>
                  <Badge className={getStatusColor(protocolStatus.ospf.state)}>
                    {protocolStatus.ospf.state}
                  </Badge>
                </div>
              </CardContent>
            </Card>

            {/* ISIS Status */}
            <Card>
              <CardHeader>
                <CardTitle className="flex items-center gap-2">
                  {getStatusIcon(protocolStatus.isis.state)}
                  ISIS Status
                </CardTitle>
              </CardHeader>
              <CardContent className="space-y-3">
                <div className="flex justify-between">
                  <span>Interfaces</span>
                  <span className="font-semibold">{protocolStatus.isis.interfaces}</span>
                </div>
                <div className="flex justify-between">
                  <span>Routes</span>
                  <span className="font-semibold">{protocolStatus.isis.routes.toLocaleString()}</span>
                </div>
                <div className="flex justify-between">
                  <span>Status</span>
                  <Badge className={getStatusColor(protocolStatus.isis.state)}>
                    {protocolStatus.isis.state}
                  </Badge>
                </div>
              </CardContent>
            </Card>
          </div>
        </TabsContent>
      </Tabs>
    </div>
  );
};

export default Dashboard;