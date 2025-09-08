package analytics

import (
	"context"
	"fmt"
	"log"
	"sync"
	"time"

	"router-sim/internal/config"
)

// Engine represents the analytics engine
type Engine struct {
	config     config.AnalyticsConfig
	clickhouse *ClickHouseClient
	metrics    *MetricsCollector
	mu         sync.RWMutex
	running    bool
}

// ClickHouseClient represents a ClickHouse client
type ClickHouseClient struct {
	config     config.ClickHouseConfig
	connection *Connection
	pool       *ConnectionPool
}

// Connection represents a ClickHouse connection
type Connection struct {
	host     string
	port     int
	database string
	username string
	password string
}

// ConnectionPool represents a connection pool
type ConnectionPool struct {
	connections chan *Connection
	maxSize     int
	mu          sync.Mutex
}

// MetricsCollector collects and aggregates metrics
type MetricsCollector struct {
	packetMetrics    []PacketMetrics
	routeMetrics     []RouteMetrics
	systemMetrics    []SystemMetrics
	trafficFlows     []TrafficFlow
	mu               sync.RWMutex
	batchSize        int
	flushInterval    time.Duration
	lastFlush        time.Time
}

// PacketMetrics represents packet-level metrics
type PacketMetrics struct {
	Timestamp       time.Time `json:"timestamp"`
	InterfaceName   string    `json:"interface_name"`
	SourceIP        string    `json:"source_ip"`
	DestinationIP   string    `json:"destination_ip"`
	SourcePort      uint16    `json:"source_port"`
	DestinationPort uint16    `json:"destination_port"`
	Protocol        uint8     `json:"protocol"`
	PacketSize      uint16    `json:"packet_size"`
	DSCP            uint8     `json:"dscp"`
	TTL             uint8     `json:"ttl"`
	IsFragmented    bool      `json:"is_fragmented"`
	IsRouted        bool      `json:"is_routed"`
	IsDropped       bool      `json:"is_dropped"`
	ProcessingTime  time.Duration `json:"processing_time_us"`
	NextHop         string    `json:"next_hop"`
	RouteSource     string    `json:"route_source"`
}

// RouteMetrics represents route-level metrics
type RouteMetrics struct {
	Timestamp     time.Time `json:"timestamp"`
	Network       string    `json:"network"`
	PrefixLength  uint8     `json:"prefix_length"`
	NextHop       string    `json:"next_hop"`
	Metric        uint32    `json:"metric"`
	Protocol      string    `json:"protocol"`
	ASPathLength  uint32    `json:"as_path_length"`
	Communities   string    `json:"communities"`
	IsActive      bool      `json:"is_active"`
	Age           time.Duration `json:"age"`
	PacketCount   uint32    `json:"packet_count"`
	ByteCount     uint64    `json:"byte_count"`
}

// SystemMetrics represents system-level metrics
type SystemMetrics struct {
	Timestamp              time.Time `json:"timestamp"`
	CPUUsage               float64   `json:"cpu_usage"`
	MemoryUsage            float64   `json:"memory_usage"`
	DiskUsage              float64   `json:"disk_usage"`
	TotalPacketsProcessed  uint64    `json:"total_packets_processed"`
	TotalBytesProcessed    uint64    `json:"total_bytes_processed"`
	PacketsDropped         uint64    `json:"packets_dropped"`
	RoutingTableSize       uint64    `json:"routing_table_size"`
	ActiveNeighbors        uint32    `json:"active_neighbors"`
	ActiveInterfaces       uint32    `json:"active_interfaces"`
	PacketsPerSecond       float64   `json:"packets_per_second"`
	BytesPerSecond         float64   `json:"bytes_per_second"`
	AverageLatency         float64   `json:"average_latency"`
	PacketLossRate         float64   `json:"packet_loss_rate"`
}

// TrafficFlow represents traffic flow metrics
type TrafficFlow struct {
	Timestamp      time.Time `json:"timestamp"`
	FlowID         string    `json:"flow_id"`
	SourceIP       string    `json:"source_ip"`
	DestinationIP  string    `json:"destination_ip"`
	SourcePort     uint16    `json:"source_port"`
	DestinationPort uint16   `json:"destination_port"`
	Protocol       uint8     `json:"protocol"`
	PacketCount    uint64    `json:"packet_count"`
	ByteCount      uint64    `json:"byte_count"`
	Duration       time.Duration `json:"duration"`
	Application    string    `json:"application"`
	DSCP           uint8     `json:"dscp"`
	IsEncrypted    bool      `json:"is_encrypted"`
	TrafficClass   string    `json:"traffic_class"`
	Throughput     float64   `json:"throughput"`
	Jitter         float64   `json:"jitter"`
	Latency        float64   `json:"latency"`
}

// NewEngine creates a new analytics engine
func NewEngine(config config.AnalyticsConfig) (*Engine, error) {
	clickhouse, err := NewClickHouseClient(config.ClickHouse)
	if err != nil {
		return nil, fmt.Errorf("failed to create ClickHouse client: %w", err)
	}

	metrics := &MetricsCollector{
		packetMetrics: make([]PacketMetrics, 0, config.ClickHouse.BatchSize),
		routeMetrics:  make([]RouteMetrics, 0, config.ClickHouse.BatchSize),
		systemMetrics: make([]SystemMetrics, 0, config.ClickHouse.BatchSize),
		trafficFlows:  make([]TrafficFlow, 0, config.ClickHouse.BatchSize),
		batchSize:     config.ClickHouse.BatchSize,
		flushInterval: config.ClickHouse.FlushInterval,
		lastFlush:     time.Now(),
	}

	return &Engine{
		config:     config,
		clickhouse: clickhouse,
		metrics:    metrics,
	}, nil
}

// Start starts the analytics engine
func (e *Engine) Start(ctx context.Context) error {
	e.mu.Lock()
	defer e.mu.Unlock()

	if e.running {
		return fmt.Errorf("analytics engine is already running")
	}

	// Start background processing
	go e.processMetrics(ctx)

	e.running = true
	log.Println("Analytics engine started")
	return nil
}

// Stop stops the analytics engine
func (e *Engine) Stop() error {
	e.mu.Lock()
	defer e.mu.Unlock()

	if !e.running {
		return fmt.Errorf("analytics engine is not running")
	}

	e.running = false
	log.Println("Analytics engine stopped")
	return nil
}

// InsertPacketMetrics inserts packet metrics
func (e *Engine) InsertPacketMetrics(metrics PacketMetrics) error {
	e.mu.RLock()
	defer e.mu.RUnlock()

	if !e.running {
		return fmt.Errorf("analytics engine is not running")
	}

	e.metrics.mu.Lock()
	e.metrics.packetMetrics = append(e.metrics.packetMetrics, metrics)
	e.metrics.mu.Unlock()

	return nil
}

// InsertRouteMetrics inserts route metrics
func (e *Engine) InsertRouteMetrics(metrics RouteMetrics) error {
	e.mu.RLock()
	defer e.mu.RUnlock()

	if !e.running {
		return fmt.Errorf("analytics engine is not running")
	}

	e.metrics.mu.Lock()
	e.metrics.routeMetrics = append(e.metrics.routeMetrics, metrics)
	e.metrics.mu.Unlock()

	return nil
}

// InsertSystemMetrics inserts system metrics
func (e *Engine) InsertSystemMetrics(metrics SystemMetrics) error {
	e.mu.RLock()
	defer e.mu.RUnlock()

	if !e.running {
		return fmt.Errorf("analytics engine is not running")
	}

	e.metrics.mu.Lock()
	e.metrics.systemMetrics = append(e.metrics.systemMetrics, metrics)
	e.metrics.mu.Unlock()

	return nil
}

// InsertTrafficFlow inserts traffic flow metrics
func (e *Engine) InsertTrafficFlow(flow TrafficFlow) error {
	e.mu.RLock()
	defer e.mu.RUnlock()

	if !e.running {
		return fmt.Errorf("analytics engine is not running")
	}

	e.metrics.mu.Lock()
	e.metrics.trafficFlows = append(e.metrics.trafficFlows, flow)
	e.metrics.mu.Unlock()

	return nil
}

// GetStats returns analytics engine statistics
func (e *Engine) GetStats() map[string]interface{} {
	e.mu.RLock()
	defer e.mu.RUnlock()

	e.metrics.mu.RLock()
	defer e.metrics.mu.RUnlock()

	return map[string]interface{}{
		"running":           e.running,
		"packet_metrics":    len(e.metrics.packetMetrics),
		"route_metrics":     len(e.metrics.routeMetrics),
		"system_metrics":    len(e.metrics.systemMetrics),
		"traffic_flows":     len(e.metrics.trafficFlows),
		"total_queued":      len(e.metrics.packetMetrics) + len(e.metrics.routeMetrics) + len(e.metrics.systemMetrics) + len(e.metrics.trafficFlows),
		"last_flush":        e.metrics.lastFlush,
		"batch_size":        e.metrics.batchSize,
		"flush_interval":    e.metrics.flushInterval,
	}
}

// processMetrics processes metrics in the background
func (e *Engine) processMetrics(ctx context.Context) {
	ticker := time.NewTicker(e.metrics.flushInterval)
	defer ticker.Stop()

	for {
		select {
		case <-ctx.Done():
			// Flush remaining metrics before shutdown
			e.flushMetrics()
			return
		case <-ticker.C:
			e.flushMetrics()
		}
	}
}

// flushMetrics flushes metrics to ClickHouse
func (e *Engine) flushMetrics() {
	e.metrics.mu.Lock()
	defer e.metrics.mu.Unlock()

	now := time.Now()
	shouldFlush := false

	// Check if we should flush based on batch size or time
	if len(e.metrics.packetMetrics) >= e.metrics.batchSize ||
		len(e.metrics.routeMetrics) >= e.metrics.batchSize ||
		len(e.metrics.systemMetrics) >= e.metrics.batchSize ||
		len(e.metrics.trafficFlows) >= e.metrics.batchSize ||
		now.Sub(e.metrics.lastFlush) >= e.metrics.flushInterval {
		shouldFlush = true
	}

	if !shouldFlush {
		return
	}

	// Flush packet metrics
	if len(e.metrics.packetMetrics) > 0 {
		if err := e.clickhouse.InsertPacketMetrics(e.metrics.packetMetrics); err != nil {
			log.Printf("Failed to insert packet metrics: %v", err)
		} else {
			e.metrics.packetMetrics = e.metrics.packetMetrics[:0]
		}
	}

	// Flush route metrics
	if len(e.metrics.routeMetrics) > 0 {
		if err := e.clickhouse.InsertRouteMetrics(e.metrics.routeMetrics); err != nil {
			log.Printf("Failed to insert route metrics: %v", err)
		} else {
			e.metrics.routeMetrics = e.metrics.routeMetrics[:0]
		}
	}

	// Flush system metrics
	if len(e.metrics.systemMetrics) > 0 {
		if err := e.clickhouse.InsertSystemMetrics(e.metrics.systemMetrics); err != nil {
			log.Printf("Failed to insert system metrics: %v", err)
		} else {
			e.metrics.systemMetrics = e.metrics.systemMetrics[:0]
		}
	}

	// Flush traffic flows
	if len(e.metrics.trafficFlows) > 0 {
		if err := e.clickhouse.InsertTrafficFlows(e.metrics.trafficFlows); err != nil {
			log.Printf("Failed to insert traffic flows: %v", err)
		} else {
			e.metrics.trafficFlows = e.metrics.trafficFlows[:0]
		}
	}

	e.metrics.lastFlush = now
}

// NewClickHouseClient creates a new ClickHouse client
func NewClickHouseClient(config config.ClickHouseConfig) (*ClickHouseClient, error) {
	connection := &Connection{
		host:     config.Host,
		port:     config.Port,
		database: config.Database,
		username: config.Username,
		password: config.Password,
	}

	pool := &ConnectionPool{
		connections: make(chan *Connection, config.MaxConnections),
		maxSize:     config.MaxConnections,
	}

	// Fill the pool
	for i := 0; i < config.MaxConnections; i++ {
		pool.connections <- connection
	}

	return &ClickHouseClient{
		config:     config,
		connection: connection,
		pool:       pool,
	}, nil
}

// InsertPacketMetrics inserts packet metrics into ClickHouse
func (c *ClickHouseClient) InsertPacketMetrics(metrics []PacketMetrics) error {
	// This would contain the actual ClickHouse insertion logic
	// For now, we'll just log the operation
	log.Printf("Inserting %d packet metrics into ClickHouse", len(metrics))
	return nil
}

// InsertRouteMetrics inserts route metrics into ClickHouse
func (c *ClickHouseClient) InsertRouteMetrics(metrics []RouteMetrics) error {
	log.Printf("Inserting %d route metrics into ClickHouse", len(metrics))
	return nil
}

// InsertSystemMetrics inserts system metrics into ClickHouse
func (c *ClickHouseClient) InsertSystemMetrics(metrics []SystemMetrics) error {
	log.Printf("Inserting %d system metrics into ClickHouse", len(metrics))
	return nil
}

// InsertTrafficFlows inserts traffic flows into ClickHouse
func (c *ClickHouseClient) InsertTrafficFlows(flows []TrafficFlow) error {
	log.Printf("Inserting %d traffic flows into ClickHouse", len(flows))
	return nil
}
