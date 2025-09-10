package analytics

import (
	"context"
	"encoding/json"
	"log"
	"sync"
	"time"

	"router-sim/internal/config"
)

// Engine represents the analytics engine
type Engine struct {
	config     config.AnalyticsConfig
	subscribers map[chan MetricUpdate]bool
	mutex      sync.RWMutex
	ctx        context.Context
	cancel     context.CancelFunc
}

// MetricUpdate represents a metric update
type MetricUpdate struct {
	Timestamp time.Time              `json:"timestamp"`
	Metrics   map[string]interface{} `json:"metrics"`
	Source    string                 `json:"source"`
}

// NewEngine creates a new analytics engine
func NewEngine(cfg config.AnalyticsConfig) (*Engine, error) {
	ctx, cancel := context.WithCancel(context.Background())
	
	engine := &Engine{
		config:      cfg,
		subscribers: make(map[chan MetricUpdate]bool),
		ctx:         ctx,
		cancel:      cancel,
	}

	// Start metric collection if enabled
	if cfg.Enabled {
		go engine.collectMetrics()
	}

	return engine, nil
}

// Subscribe subscribes to metric updates
func (e *Engine) Subscribe(updates chan MetricUpdate) {
	e.mutex.Lock()
	defer e.mutex.Unlock()
	e.subscribers[updates] = true
}

// Unsubscribe unsubscribes from metric updates
func (e *Engine) Unsubscribe(updates chan MetricUpdate) {
	e.mutex.Lock()
	defer e.mutex.Unlock()
	delete(e.subscribers, updates)
}

// Publish publishes a metric update to all subscribers
func (e *Engine) Publish(update MetricUpdate) {
	e.mutex.RLock()
	defer e.mutex.RUnlock()
	
	for ch := range e.subscribers {
		select {
		case ch <- update:
		default:
			// Channel is full, skip this update
		}
	}
}

// collectMetrics collects metrics periodically
func (e *Engine) collectMetrics() {
	ticker := time.NewTicker(5 * time.Second)
	defer ticker.Stop()

	for {
		select {
		case <-e.ctx.Done():
			return
		case <-ticker.C:
			// Generate mock metrics
			metrics := map[string]interface{}{
				"cpu_usage":    45.0 + (time.Now().Unix()%20),
				"memory_usage": 65.0 + (time.Now().Unix()%15),
				"network_rx":   1000000 + (time.Now().Unix()%500000),
				"network_tx":   2000000 + (time.Now().Unix()%1000000),
				"packet_loss":  0.001 + (time.Now().Unix()%10)/10000.0,
				"latency":      10.0 + (time.Now().Unix()%20),
			}

			update := MetricUpdate{
				Timestamp: time.Now(),
				Metrics:   metrics,
				Source:    "router-sim",
			}

			e.Publish(update)
		}
	}
}

// Query queries analytics data
func (e *Engine) Query(ctx context.Context, query string, timeRange string) ([]MetricUpdate, error) {
	// Mock query implementation
	results := []MetricUpdate{
		{
			Timestamp: time.Now().Add(-1 * time.Hour),
			Metrics: map[string]interface{}{
				"cpu_usage": 45.2,
				"memory_usage": 67.8,
			},
			Source: "router-sim",
		},
		{
			Timestamp: time.Now().Add(-30 * time.Minute),
			Metrics: map[string]interface{}{
				"cpu_usage": 52.1,
				"memory_usage": 71.2,
			},
			Source: "router-sim",
		},
		{
			Timestamp: time.Now(),
			Metrics: map[string]interface{}{
				"cpu_usage": 48.5,
				"memory_usage": 69.3,
			},
			Source: "router-sim",
		},
	}

	return results, nil
}

// Close closes the analytics engine
func (e *Engine) Close() error {
	e.cancel()
	return nil
}

// GetDashboardData returns dashboard data
func (e *Engine) GetDashboardData() map[string]interface{} {
	return map[string]interface{}{
		"system_status": "online",
		"protocols": map[string]interface{}{
			"bgp": map[string]interface{}{
				"status": "active",
				"routes": 1250,
				"neighbors": 8,
			},
			"ospf": map[string]interface{}{
				"status": "active",
				"routes": 890,
				"neighbors": 12,
			},
			"isis": map[string]interface{}{
				"status": "active",
				"routes": 2100,
				"neighbors": 6,
			},
		},
		"cloud_connections": map[string]interface{}{
			"aviatrix": map[string]interface{}{
				"status": "connected",
				"gateways": 4,
			},
			"cloudpods": map[string]interface{}{
				"status": "connected",
				"resources": 15,
			},
		},
		"recent_events": []map[string]interface{}{
			{
				"type": "info",
				"message": "BGP session established with neighbor 192.168.1.1",
				"time": "2 minutes ago",
			},
			{
				"type": "warning",
				"message": "High CPU usage detected on router-1",
				"time": "5 minutes ago",
			},
			{
				"type": "success",
				"message": "Aviatrix gateway connection restored",
				"time": "8 minutes ago",
			},
		},
	}
}

// GetMetrics returns current metrics
func (e *Engine) GetMetrics() map[string]interface{} {
	return map[string]interface{}{
		"cpu_usage":    45.2,
		"memory_usage": 67.8,
		"network_rx":   1024000,
		"network_tx":   2048000,
		"packet_loss":  0.001,
		"latency":      12.5,
		"timestamp":    time.Now().Unix(),
	}
}
