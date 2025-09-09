import React, { useState, useEffect } from 'react'
import { Card, CardContent, CardHeader, CardTitle } from './components/ui/card'
import { Button } from './components/ui/button'
import { Input } from './components/ui/input'
import { Label } from './components/ui/label'
import { Tabs, TabsContent, TabsList, TabsTrigger } from './components/ui/tabs'
import { Badge } from './components/ui/badge'
import { Alert, AlertDescription } from './components/ui/alert'
import { 
  Cloud, 
  Server, 
  Network, 
  Shield, 
  Activity, 
  Settings,
  Plus,
  Trash2,
  Edit,
  RefreshCw,
  CheckCircle,
  XCircle,
  AlertTriangle
} from 'lucide-react'

interface AviatrixGateway {
  id: string
  name: string
  type: 'transit' | 'spoke' | 'vpn'
  cloudType: 'aws' | 'azure' | 'gcp' | 'oci'
  region: string
  vpcId: string
  publicIp: string
  privateIp: string
  status: 'running' | 'stopped' | 'pending' | 'error'
  createdAt: string
}

interface AviatrixConnection {
  id: string
  name: string
  sourceGateway: string
  destinationGateway: string
  type: 'transit-peering' | 'spoke-attachment' | 'vpn-tunnel'
  status: 'active' | 'inactive' | 'pending' | 'error'
  bandwidth: string
  latency: number
}

interface AviatrixRoute {
  id: string
  destination: string
  nextHop: string
  gatewayId: string
  type: 'bgp' | 'static' | 'connected'
  status: 'active' | 'inactive'
  metric: number
}

export default function Aviatrix() {
  const [gateways, setGateways] = useState<AviatrixGateway[]>([])
  const [connections, setConnections] = useState<AviatrixConnection[]>([])
  const [routes, setRoutes] = useState<AviatrixRoute[]>([])
  const [loading, setLoading] = useState(true)
  const [error, setError] = useState<string | null>(null)
  const [selectedGateway, setSelectedGateway] = useState<string | null>(null)

  // Mock data for demonstration
  useEffect(() => {
    const mockGateways: AviatrixGateway[] = [
      {
        id: 'gw-1',
        name: 'transit-gateway-us-east-1',
        type: 'transit',
        cloudType: 'aws',
        region: 'us-east-1',
        vpcId: 'vpc-12345678',
        publicIp: '54.123.45.67',
        privateIp: '10.0.1.100',
        status: 'running',
        createdAt: '2024-01-15T10:30:00Z'
      },
      {
        id: 'gw-2',
        name: 'spoke-gateway-us-west-2',
        type: 'spoke',
        cloudType: 'aws',
        region: 'us-west-2',
        vpcId: 'vpc-87654321',
        publicIp: '54.234.56.78',
        privateIp: '10.1.1.100',
        status: 'running',
        createdAt: '2024-01-16T14:20:00Z'
      },
      {
        id: 'gw-3',
        name: 'vpn-gateway-azure',
        type: 'vpn',
        cloudType: 'azure',
        region: 'eastus',
        vpcId: 'vnet-azure-123',
        publicIp: '20.123.45.67',
        privateIp: '10.2.1.100',
        status: 'pending',
        createdAt: '2024-01-17T09:15:00Z'
      }
    ]

    const mockConnections: AviatrixConnection[] = [
      {
        id: 'conn-1',
        name: 'transit-peering-us-east-us-west',
        sourceGateway: 'gw-1',
        destinationGateway: 'gw-2',
        type: 'transit-peering',
        status: 'active',
        bandwidth: '1 Gbps',
        latency: 45
      },
      {
        id: 'conn-2',
        name: 'spoke-attachment-azure',
        sourceGateway: 'gw-2',
        destinationGateway: 'gw-3',
        type: 'spoke-attachment',
        status: 'pending',
        bandwidth: '500 Mbps',
        latency: 120
      }
    ]

    const mockRoutes: AviatrixRoute[] = [
      {
        id: 'route-1',
        destination: '10.0.0.0/8',
        nextHop: '10.0.1.100',
        gatewayId: 'gw-1',
        type: 'bgp',
        status: 'active',
        metric: 100
      },
      {
        id: 'route-2',
        destination: '172.16.0.0/12',
        nextHop: '10.1.1.100',
        gatewayId: 'gw-2',
        type: 'bgp',
        status: 'active',
        metric: 200
      }
    ]

    setTimeout(() => {
      setGateways(mockGateways)
      setConnections(mockConnections)
      setRoutes(mockRoutes)
      setLoading(false)
    }, 1000)
  }, [])

  const getStatusIcon = (status: string) => {
    switch (status) {
      case 'running':
      case 'active':
        return <CheckCircle className="h-4 w-4 text-green-500" />
      case 'stopped':
      case 'inactive':
        return <XCircle className="h-4 w-4 text-red-500" />
      case 'pending':
        return <RefreshCw className="h-4 w-4 text-yellow-500 animate-spin" />
      case 'error':
        return <AlertTriangle className="h-4 w-4 text-red-500" />
      default:
        return <Activity className="h-4 w-4 text-gray-500" />
    }
  }

  const getStatusBadge = (status: string) => {
    const variants = {
      running: 'default',
      active: 'default',
      stopped: 'destructive',
      inactive: 'destructive',
      pending: 'secondary',
      error: 'destructive'
    } as const

    return (
      <Badge variant={variants[status as keyof typeof variants] || 'secondary'}>
        {status}
      </Badge>
    )
  }

  const getCloudIcon = (cloudType: string) => {
    switch (cloudType) {
      case 'aws':
        return <Cloud className="h-4 w-4 text-orange-500" />
      case 'azure':
        return <Cloud className="h-4 w-4 text-blue-500" />
      case 'gcp':
        return <Cloud className="h-4 w-4 text-green-500" />
      case 'oci':
        return <Cloud className="h-4 w-4 text-red-500" />
      default:
        return <Cloud className="h-4 w-4 text-gray-500" />
    }
  }

  if (loading) {
    return (
      <div className="flex items-center justify-center h-64">
        <RefreshCw className="h-8 w-8 animate-spin" />
        <span className="ml-2">Loading Aviatrix data...</span>
      </div>
    )
  }

  return (
    <div className="space-y-6">
      <div className="flex items-center justify-between">
        <div>
          <h1 className="text-3xl font-bold">Aviatrix Integration</h1>
          <p className="text-muted-foreground">
            Manage Aviatrix gateways, connections, and routing
          </p>
        </div>
        <div className="flex space-x-2">
          <Button onClick={() => setLoading(true)}>
            <RefreshCw className="h-4 w-4 mr-2" />
            Refresh
          </Button>
          <Button>
            <Plus className="h-4 w-4 mr-2" />
            Add Gateway
          </Button>
        </div>
      </div>

      {error && (
        <Alert variant="destructive">
          <AlertTriangle className="h-4 w-4" />
          <AlertDescription>{error}</AlertDescription>
        </Alert>
      )}

      <Tabs defaultValue="gateways" className="space-y-4">
        <TabsList>
          <TabsTrigger value="gateways">Gateways</TabsTrigger>
          <TabsTrigger value="connections">Connections</TabsTrigger>
          <TabsTrigger value="routes">Routes</TabsTrigger>
          <TabsTrigger value="monitoring">Monitoring</TabsTrigger>
        </TabsList>

        <TabsContent value="gateways" className="space-y-4">
          <div className="grid gap-4 md:grid-cols-2 lg:grid-cols-3">
            {gateways.map((gateway) => (
              <Card key={gateway.id} className="hover:shadow-lg transition-shadow">
                <CardHeader className="pb-3">
                  <div className="flex items-center justify-between">
                    <CardTitle className="text-lg">{gateway.name}</CardTitle>
                    <div className="flex items-center space-x-2">
                      {getStatusIcon(gateway.status)}
                      {getStatusBadge(gateway.status)}
                    </div>
                  </div>
                  <div className="flex items-center space-x-2 text-sm text-muted-foreground">
                    {getCloudIcon(gateway.cloudType)}
                    <span>{gateway.cloudType.toUpperCase()}</span>
                    <span>•</span>
                    <span>{gateway.region}</span>
                  </div>
                </CardHeader>
                <CardContent className="space-y-3">
                  <div className="grid grid-cols-2 gap-4 text-sm">
                    <div>
                      <Label className="text-xs text-muted-foreground">Type</Label>
                      <p className="font-medium capitalize">{gateway.type}</p>
                    </div>
                    <div>
                      <Label className="text-xs text-muted-foreground">VPC ID</Label>
                      <p className="font-mono text-xs">{gateway.vpcId}</p>
                    </div>
                    <div>
                      <Label className="text-xs text-muted-foreground">Public IP</Label>
                      <p className="font-mono text-xs">{gateway.publicIp}</p>
                    </div>
                    <div>
                      <Label className="text-xs text-muted-foreground">Private IP</Label>
                      <p className="font-mono text-xs">{gateway.privateIp}</p>
                    </div>
                  </div>
                  <div className="flex space-x-2 pt-2">
                    <Button size="sm" variant="outline">
                      <Edit className="h-3 w-3 mr-1" />
                      Edit
                    </Button>
                    <Button size="sm" variant="outline">
                      <Settings className="h-3 w-3 mr-1" />
                      Config
                    </Button>
                    <Button size="sm" variant="destructive">
                      <Trash2 className="h-3 w-3 mr-1" />
                      Delete
                    </Button>
                  </div>
                </CardContent>
              </Card>
            ))}
          </div>
        </TabsContent>

        <TabsContent value="connections" className="space-y-4">
          <Card>
            <CardHeader>
              <CardTitle>Connections</CardTitle>
            </CardHeader>
            <CardContent>
              <div className="space-y-4">
                {connections.map((connection) => (
                  <div key={connection.id} className="flex items-center justify-between p-4 border rounded-lg">
                    <div className="space-y-1">
                      <div className="flex items-center space-x-2">
                        <h3 className="font-medium">{connection.name}</h3>
                        {getStatusIcon(connection.status)}
                        {getStatusBadge(connection.status)}
                      </div>
                      <p className="text-sm text-muted-foreground">
                        {connection.sourceGateway} → {connection.destinationGateway}
                      </p>
                      <div className="flex items-center space-x-4 text-xs text-muted-foreground">
                        <span>Type: {connection.type}</span>
                        <span>Bandwidth: {connection.bandwidth}</span>
                        <span>Latency: {connection.latency}ms</span>
                      </div>
                    </div>
                    <div className="flex space-x-2">
                      <Button size="sm" variant="outline">
                        <Edit className="h-3 w-3 mr-1" />
                        Edit
                      </Button>
                      <Button size="sm" variant="destructive">
                        <Trash2 className="h-3 w-3 mr-1" />
                        Delete
                      </Button>
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
              <div className="space-y-2">
                {routes.map((route) => (
                  <div key={route.id} className="flex items-center justify-between p-3 border rounded">
                    <div className="space-y-1">
                      <div className="flex items-center space-x-2">
                        <span className="font-mono text-sm">{route.destination}</span>
                        <span className="text-muted-foreground">via</span>
                        <span className="font-mono text-sm">{route.nextHop}</span>
                        {getStatusIcon(route.status)}
                        {getStatusBadge(route.status)}
                      </div>
                      <div className="flex items-center space-x-4 text-xs text-muted-foreground">
                        <span>Type: {route.type.toUpperCase()}</span>
                        <span>Metric: {route.metric}</span>
                        <span>Gateway: {route.gatewayId}</span>
                      </div>
                    </div>
                    <div className="flex space-x-2">
                      <Button size="sm" variant="outline">
                        <Edit className="h-3 w-3 mr-1" />
                        Edit
                      </Button>
                      <Button size="sm" variant="destructive">
                        <Trash2 className="h-3 w-3 mr-1" />
                        Delete
                      </Button>
                    </div>
                  </div>
                ))}
              </div>
            </CardContent>
          </Card>
        </TabsContent>

        <TabsContent value="monitoring" className="space-y-4">
          <div className="grid gap-4 md:grid-cols-2 lg:grid-cols-4">
            <Card>
              <CardHeader className="flex flex-row items-center justify-between space-y-0 pb-2">
                <CardTitle className="text-sm font-medium">Total Gateways</CardTitle>
                <Server className="h-4 w-4 text-muted-foreground" />
              </CardHeader>
              <CardContent>
                <div className="text-2xl font-bold">{gateways.length}</div>
                <p className="text-xs text-muted-foreground">
                  {gateways.filter(g => g.status === 'running').length} running
                </p>
              </CardContent>
            </Card>

            <Card>
              <CardHeader className="flex flex-row items-center justify-between space-y-0 pb-2">
                <CardTitle className="text-sm font-medium">Active Connections</CardTitle>
                <Network className="h-4 w-4 text-muted-foreground" />
              </CardHeader>
              <CardContent>
                <div className="text-2xl font-bold">
                  {connections.filter(c => c.status === 'active').length}
                </div>
                <p className="text-xs text-muted-foreground">
                  {connections.length} total connections
                </p>
              </CardContent>
            </Card>

            <Card>
              <CardHeader className="flex flex-row items-center justify-between space-y-0 pb-2">
                <CardTitle className="text-sm font-medium">Active Routes</CardTitle>
                <Shield className="h-4 w-4 text-muted-foreground" />
              </CardHeader>
              <CardContent>
                <div className="text-2xl font-bold">
                  {routes.filter(r => r.status === 'active').length}
                </div>
                <p className="text-xs text-muted-foreground">
                  {routes.length} total routes
                </p>
              </CardContent>
            </Card>

            <Card>
              <CardHeader className="flex flex-row items-center justify-between space-y-0 pb-2">
                <CardTitle className="text-sm font-medium">Cloud Providers</CardTitle>
                <Cloud className="h-4 w-4 text-muted-foreground" />
              </CardHeader>
              <CardContent>
                <div className="text-2xl font-bold">
                  {new Set(gateways.map(g => g.cloudType)).size}
                </div>
                <p className="text-xs text-muted-foreground">
                  Multi-cloud deployment
                </p>
              </CardContent>
            </Card>
          </div>

          <Card>
            <CardHeader>
              <CardTitle>Real-time Monitoring</CardTitle>
            </CardHeader>
            <CardContent>
              <div className="text-center py-8 text-muted-foreground">
                <Activity className="h-12 w-12 mx-auto mb-4 opacity-50" />
                <p>Real-time monitoring charts will be displayed here</p>
                <p className="text-sm">Integration with Aviatrix CoPilot metrics</p>
              </div>
            </CardContent>
          </Card>
        </TabsContent>
      </Tabs>
    </div>
  )
}
