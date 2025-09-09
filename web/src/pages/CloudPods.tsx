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
  AlertTriangle,
  Database,
  Layers
} from 'lucide-react'

interface CloudPodsVPC {
  id: string
  name: string
  cidr: string
  region: string
  status: 'active' | 'inactive' | 'pending' | 'error'
  createdAt: string
  subnets: CloudPodsSubnet[]
  gateways: CloudPodsGateway[]
}

interface CloudPodsSubnet {
  id: string
  name: string
  cidr: string
  vpcId: string
  zone: string
  status: 'active' | 'inactive' | 'pending' | 'error'
  createdAt: string
}

interface CloudPodsGateway {
  id: string
  name: string
  type: 'internet' | 'nat' | 'vpn' | 'peering'
  vpcId: string
  subnetId: string
  ipAddress: string
  status: 'active' | 'inactive' | 'pending' | 'error'
  createdAt: string
}

interface CloudPodsRoute {
  id: string
  destination: string
  nextHop: string
  vpcId: string
  type: 'local' | 'static' | 'dynamic'
  status: 'active' | 'inactive'
  metric: number
}

interface CloudPodsSecurityGroup {
  id: string
  name: string
  description: string
  vpcId: string
  rules: CloudPodsSecurityRule[]
  status: 'active' | 'inactive'
  createdAt: string
}

interface CloudPodsSecurityRule {
  id: string
  direction: 'ingress' | 'egress'
  protocol: 'tcp' | 'udp' | 'icmp' | 'all'
  portRange: string
  source: string
  destination: string
  action: 'allow' | 'deny'
  priority: number
  description: string
}

export default function CloudPods() {
  const [vpcs, setVpcs] = useState<CloudPodsVPC[]>([])
  const [subnets, setSubnets] = useState<CloudPodsSubnet[]>([])
  const [gateways, setGateways] = useState<CloudPodsGateway[]>([])
  const [routes, setRoutes] = useState<CloudPodsRoute[]>([])
  const [securityGroups, setSecurityGroups] = useState<CloudPodsSecurityGroup[]>([])
  const [loading, setLoading] = useState(true)
  const [error, setError] = useState<string | null>(null)
  const [selectedVPC, setSelectedVPC] = useState<string | null>(null)

  // Mock data for demonstration
  useEffect(() => {
    const mockVPCs: CloudPodsVPC[] = [
      {
        id: 'vpc-1',
        name: 'production-vpc',
        cidr: '10.0.0.0/16',
        region: 'us-east-1',
        status: 'active',
        createdAt: '2024-01-15T10:30:00Z',
        subnets: [
          {
            id: 'subnet-1',
            name: 'public-subnet-1a',
            cidr: '10.0.1.0/24',
            vpcId: 'vpc-1',
            zone: 'us-east-1a',
            status: 'active',
            createdAt: '2024-01-15T10:35:00Z'
          },
          {
            id: 'subnet-2',
            name: 'private-subnet-1a',
            cidr: '10.0.2.0/24',
            vpcId: 'vpc-1',
            zone: 'us-east-1a',
            status: 'active',
            createdAt: '2024-01-15T10:40:00Z'
          }
        ],
        gateways: [
          {
            id: 'igw-1',
            name: 'internet-gateway',
            type: 'internet',
            vpcId: 'vpc-1',
            subnetId: 'subnet-1',
            ipAddress: '54.123.45.67',
            status: 'active',
            createdAt: '2024-01-15T10:45:00Z'
          }
        ]
      },
      {
        id: 'vpc-2',
        name: 'staging-vpc',
        cidr: '172.16.0.0/16',
        region: 'us-west-2',
        status: 'active',
        createdAt: '2024-01-16T14:20:00Z',
        subnets: [
          {
            id: 'subnet-3',
            name: 'staging-subnet-2a',
            cidr: '172.16.1.0/24',
            vpcId: 'vpc-2',
            zone: 'us-west-2a',
            status: 'active',
            createdAt: '2024-01-16T14:25:00Z'
          }
        ],
        gateways: [
          {
            id: 'ngw-1',
            name: 'nat-gateway',
            type: 'nat',
            vpcId: 'vpc-2',
            subnetId: 'subnet-3',
            ipAddress: '172.16.1.100',
            status: 'active',
            createdAt: '2024-01-16T14:30:00Z'
          }
        ]
      }
    ]

    const mockRoutes: CloudPodsRoute[] = [
      {
        id: 'route-1',
        destination: '0.0.0.0/0',
        nextHop: 'igw-1',
        vpcId: 'vpc-1',
        type: 'static',
        status: 'active',
        metric: 0
      },
      {
        id: 'route-2',
        destination: '10.0.0.0/16',
        nextHop: 'local',
        vpcId: 'vpc-1',
        type: 'local',
        status: 'active',
        metric: 0
      }
    ]

    const mockSecurityGroups: CloudPodsSecurityGroup[] = [
      {
        id: 'sg-1',
        name: 'web-servers',
        description: 'Security group for web servers',
        vpcId: 'vpc-1',
        rules: [
          {
            id: 'rule-1',
            direction: 'ingress',
            protocol: 'tcp',
            portRange: '80,443',
            source: '0.0.0.0/0',
            destination: '0.0.0.0/0',
            action: 'allow',
            priority: 100,
            description: 'Allow HTTP/HTTPS traffic'
          },
          {
            id: 'rule-2',
            direction: 'ingress',
            protocol: 'tcp',
            portRange: '22',
            source: '10.0.0.0/8',
            destination: '0.0.0.0/0',
            action: 'allow',
            priority: 200,
            description: 'Allow SSH from internal networks'
          }
        ],
        status: 'active',
        createdAt: '2024-01-15T11:00:00Z'
      }
    ]

    setTimeout(() => {
      setVpcs(mockVPCs)
      setSubnets(mockVPCs.flatMap(vpc => vpc.subnets))
      setGateways(mockVPCs.flatMap(vpc => vpc.gateways))
      setRoutes(mockRoutes)
      setSecurityGroups(mockSecurityGroups)
      setLoading(false)
    }, 1000)
  }, [])

  const getStatusIcon = (status: string) => {
    switch (status) {
      case 'active':
        return <CheckCircle className="h-4 w-4 text-green-500" />
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
      active: 'default',
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

  const getGatewayIcon = (type: string) => {
    switch (type) {
      case 'internet':
        return <Cloud className="h-4 w-4 text-blue-500" />
      case 'nat':
        return <Server className="h-4 w-4 text-green-500" />
      case 'vpn':
        return <Shield className="h-4 w-4 text-purple-500" />
      case 'peering':
        return <Network className="h-4 w-4 text-orange-500" />
      default:
        return <Server className="h-4 w-4 text-gray-500" />
    }
  }

  if (loading) {
    return (
      <div className="flex items-center justify-center h-64">
        <RefreshCw className="h-8 w-8 animate-spin" />
        <span className="ml-2">Loading CloudPods data...</span>
      </div>
    )
  }

  return (
    <div className="space-y-6">
      <div className="flex items-center justify-between">
        <div>
          <h1 className="text-3xl font-bold">CloudPods Integration</h1>
          <p className="text-muted-foreground">
            Manage CloudPods VPCs, subnets, gateways, and security groups
          </p>
        </div>
        <div className="flex space-x-2">
          <Button onClick={() => setLoading(true)}>
            <RefreshCw className="h-4 w-4 mr-2" />
            Refresh
          </Button>
          <Button>
            <Plus className="h-4 w-4 mr-2" />
            Create VPC
          </Button>
        </div>
      </div>

      {error && (
        <Alert variant="destructive">
          <AlertTriangle className="h-4 w-4" />
          <AlertDescription>{error}</AlertDescription>
        </Alert>
      )}

      <Tabs defaultValue="vpcs" className="space-y-4">
        <TabsList>
          <TabsTrigger value="vpcs">VPCs</TabsTrigger>
          <TabsTrigger value="subnets">Subnets</TabsTrigger>
          <TabsTrigger value="gateways">Gateways</TabsTrigger>
          <TabsTrigger value="routes">Routes</TabsTrigger>
          <TabsTrigger value="security">Security Groups</TabsTrigger>
          <TabsTrigger value="monitoring">Monitoring</TabsTrigger>
        </TabsList>

        <TabsContent value="vpcs" className="space-y-4">
          <div className="grid gap-4 md:grid-cols-2 lg:grid-cols-3">
            {vpcs.map((vpc) => (
              <Card key={vpc.id} className="hover:shadow-lg transition-shadow">
                <CardHeader className="pb-3">
                  <div className="flex items-center justify-between">
                    <CardTitle className="text-lg">{vpc.name}</CardTitle>
                    <div className="flex items-center space-x-2">
                      {getStatusIcon(vpc.status)}
                      {getStatusBadge(vpc.status)}
                    </div>
                  </div>
                  <div className="flex items-center space-x-2 text-sm text-muted-foreground">
                    <Database className="h-4 w-4" />
                    <span>{vpc.region}</span>
                    <span>•</span>
                    <span className="font-mono text-xs">{vpc.cidr}</span>
                  </div>
                </CardHeader>
                <CardContent className="space-y-3">
                  <div className="grid grid-cols-2 gap-4 text-sm">
                    <div>
                      <Label className="text-xs text-muted-foreground">Subnets</Label>
                      <p className="font-medium">{vpc.subnets.length}</p>
                    </div>
                    <div>
                      <Label className="text-xs text-muted-foreground">Gateways</Label>
                      <p className="font-medium">{vpc.gateways.length}</p>
                    </div>
                    <div>
                      <Label className="text-xs text-muted-foreground">Created</Label>
                      <p className="text-xs">{new Date(vpc.createdAt).toLocaleDateString()}</p>
                    </div>
                    <div>
                      <Label className="text-xs text-muted-foreground">Status</Label>
                      <p className="text-xs capitalize">{vpc.status}</p>
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

        <TabsContent value="subnets" className="space-y-4">
          <Card>
            <CardHeader>
              <CardTitle>Subnets</CardTitle>
            </CardHeader>
            <CardContent>
              <div className="space-y-4">
                {subnets.map((subnet) => (
                  <div key={subnet.id} className="flex items-center justify-between p-4 border rounded-lg">
                    <div className="space-y-1">
                      <div className="flex items-center space-x-2">
                        <h3 className="font-medium">{subnet.name}</h3>
                        {getStatusIcon(subnet.status)}
                        {getStatusBadge(subnet.status)}
                      </div>
                      <p className="text-sm text-muted-foreground">
                        <span className="font-mono">{subnet.cidr}</span>
                        <span className="mx-2">•</span>
                        <span>Zone: {subnet.zone}</span>
                        <span className="mx-2">•</span>
                        <span>VPC: {subnet.vpcId}</span>
                      </p>
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

        <TabsContent value="gateways" className="space-y-4">
          <Card>
            <CardHeader>
              <CardTitle>Gateways</CardTitle>
            </CardHeader>
            <CardContent>
              <div className="space-y-4">
                {gateways.map((gateway) => (
                  <div key={gateway.id} className="flex items-center justify-between p-4 border rounded-lg">
                    <div className="space-y-1">
                      <div className="flex items-center space-x-2">
                        {getGatewayIcon(gateway.type)}
                        <h3 className="font-medium">{gateway.name}</h3>
                        {getStatusIcon(gateway.status)}
                        {getStatusBadge(gateway.status)}
                      </div>
                      <p className="text-sm text-muted-foreground">
                        <span className="capitalize">{gateway.type} Gateway</span>
                        <span className="mx-2">•</span>
                        <span className="font-mono">{gateway.ipAddress}</span>
                        <span className="mx-2">•</span>
                        <span>VPC: {gateway.vpcId}</span>
                      </p>
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
                        <span>Type: {route.type}</span>
                        <span>Metric: {route.metric}</span>
                        <span>VPC: {route.vpcId}</span>
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

        <TabsContent value="security" className="space-y-4">
          <Card>
            <CardHeader>
              <CardTitle>Security Groups</CardTitle>
            </CardHeader>
            <CardContent>
              <div className="space-y-4">
                {securityGroups.map((sg) => (
                  <div key={sg.id} className="border rounded-lg p-4">
                    <div className="flex items-center justify-between mb-3">
                      <div className="flex items-center space-x-2">
                        <Shield className="h-4 w-4" />
                        <h3 className="font-medium">{sg.name}</h3>
                        {getStatusIcon(sg.status)}
                        {getStatusBadge(sg.status)}
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
                    <p className="text-sm text-muted-foreground mb-3">{sg.description}</p>
                    <div className="space-y-2">
                      <h4 className="text-sm font-medium">Rules ({sg.rules.length})</h4>
                      {sg.rules.map((rule) => (
                        <div key={rule.id} className="flex items-center justify-between p-2 bg-muted rounded text-sm">
                          <div className="flex items-center space-x-4">
                            <Badge variant={rule.direction === 'ingress' ? 'default' : 'secondary'}>
                              {rule.direction}
                            </Badge>
                            <span className="font-mono">{rule.protocol}</span>
                            <span>{rule.portRange}</span>
                            <span className="text-muted-foreground">
                              {rule.source} → {rule.destination}
                            </span>
                            <Badge variant={rule.action === 'allow' ? 'default' : 'destructive'}>
                              {rule.action}
                            </Badge>
                          </div>
                          <span className="text-xs text-muted-foreground">
                            Priority: {rule.priority}
                          </span>
                        </div>
                      ))}
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
                <CardTitle className="text-sm font-medium">Total VPCs</CardTitle>
                <Database className="h-4 w-4 text-muted-foreground" />
              </CardHeader>
              <CardContent>
                <div className="text-2xl font-bold">{vpcs.length}</div>
                <p className="text-xs text-muted-foreground">
                  {vpcs.filter(v => v.status === 'active').length} active
                </p>
              </CardContent>
            </Card>

            <Card>
              <CardHeader className="flex flex-row items-center justify-between space-y-0 pb-2">
                <CardTitle className="text-sm font-medium">Total Subnets</CardTitle>
                <Layers className="h-4 w-4 text-muted-foreground" />
              </CardHeader>
              <CardContent>
                <div className="text-2xl font-bold">{subnets.length}</div>
                <p className="text-xs text-muted-foreground">
                  {subnets.filter(s => s.status === 'active').length} active
                </p>
              </CardContent>
            </Card>

            <Card>
              <CardHeader className="flex flex-row items-center justify-between space-y-0 pb-2">
                <CardTitle className="text-sm font-medium">Total Gateways</CardTitle>
                <Server className="h-4 w-4 text-muted-foreground" />
              </CardHeader>
              <CardContent>
                <div className="text-2xl font-bold">{gateways.length}</div>
                <p className="text-xs text-muted-foreground">
                  {gateways.filter(g => g.status === 'active').length} active
                </p>
              </CardContent>
            </Card>

            <Card>
              <CardHeader className="flex flex-row items-center justify-between space-y-0 pb-2">
                <CardTitle className="text-sm font-medium">Security Groups</CardTitle>
                <Shield className="h-4 w-4 text-muted-foreground" />
              </CardHeader>
              <CardContent>
                <div className="text-2xl font-bold">{securityGroups.length}</div>
                <p className="text-xs text-muted-foreground">
                  {securityGroups.filter(sg => sg.status === 'active').length} active
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
                <p className="text-sm">Integration with CloudPods metrics and analytics</p>
              </div>
            </CardContent>
          </Card>
        </TabsContent>
      </Tabs>
    </div>
  )
}