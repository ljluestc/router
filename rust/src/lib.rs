use std::collections::HashMap;
use std::sync::{Arc, Mutex};
use std::time::{SystemTime, UNIX_EPOCH};

pub mod packet_engine;
pub mod routing_table;

// Re-export main components
pub use packet_engine::PacketEngine;
pub use routing_table::RoutingTable;

/// Router Analytics Engine
/// High-performance analytics engine for router simulation
pub struct RouterAnalytics {
    packet_engine: Arc<Mutex<PacketEngine>>,
    routing_table: Arc<Mutex<RoutingTable>>,
    metrics: Arc<Mutex<HashMap<String, f64>>>,
}

impl RouterAnalytics {
    pub fn new() -> Self {
        Self {
            packet_engine: Arc::new(Mutex::new(PacketEngine::new())),
            routing_table: Arc::new(Mutex::new(RoutingTable::new())),
            metrics: Arc::new(Mutex::new(HashMap::new())),
        }
    }

    /// Process a packet through the analytics engine
    pub fn process_packet(&self, packet: &Packet) -> Result<(), String> {
        let mut engine = self.packet_engine.lock().map_err(|e| e.to_string())?;
        engine.process_packet(packet)?;
        Ok(())
    }

    /// Add a route to the routing table
    pub fn add_route(&self, route: &Route) -> Result<(), String> {
        let mut table = self.routing_table.lock().map_err(|e| e.to_string())?;
        table.add_route(route)?;
        Ok(())
    }

    /// Remove a route from the routing table
    pub fn remove_route(&self, destination: &str) -> Result<(), String> {
        let mut table = self.routing_table.lock().map_err(|e| e.to_string())?;
        table.remove_route(destination)?;
        Ok(())
    }

    /// Get routing statistics
    pub fn get_routing_stats(&self) -> Result<RoutingStats, String> {
        let table = self.routing_table.lock().map_err(|e| e.to_string())?;
        Ok(table.get_stats())
    }

    /// Get packet processing statistics
    pub fn get_packet_stats(&self) -> Result<PacketStats, String> {
        let engine = self.packet_engine.lock().map_err(|e| e.to_string())?;
        Ok(engine.get_stats())
    }

    /// Update a metric
    pub fn update_metric(&self, name: &str, value: f64) -> Result<(), String> {
        let mut metrics = self.metrics.lock().map_err(|e| e.to_string())?;
        metrics.insert(name.to_string(), value);
        Ok(())
    }

    /// Get all metrics
    pub fn get_metrics(&self) -> Result<HashMap<String, f64>, String> {
        let metrics = self.metrics.lock().map_err(|e| e.to_string())?;
        Ok(metrics.clone())
    }

    /// Reset all statistics
    pub fn reset(&self) -> Result<(), String> {
        let mut engine = self.packet_engine.lock().map_err(|e| e.to_string())?;
        engine.reset();
        
        let mut table = self.routing_table.lock().map_err(|e| e.to_string())?;
        table.reset();
        
        let mut metrics = self.metrics.lock().map_err(|e| e.to_string())?;
        metrics.clear();
        
        Ok(())
    }
}

/// Packet representation
#[derive(Debug, Clone)]
pub struct Packet {
    pub id: u64,
    pub size: u32,
    pub priority: u32,
    pub source_ip: String,
    pub dest_ip: String,
    pub source_port: u16,
    pub dest_port: u16,
    pub protocol: u8,
    pub timestamp: u64,
}

impl Packet {
    pub fn new(
        id: u64,
        size: u32,
        priority: u32,
        source_ip: String,
        dest_ip: String,
        source_port: u16,
        dest_port: u16,
        protocol: u8,
    ) -> Self {
        let timestamp = SystemTime::now()
            .duration_since(UNIX_EPOCH)
            .unwrap()
            .as_millis() as u64;

        Self {
            id,
            size,
            priority,
            source_ip,
            dest_ip,
            source_port,
            dest_port,
            protocol,
            timestamp,
        }
    }
}

/// Route representation
#[derive(Debug, Clone)]
pub struct Route {
    pub destination: String,
    pub gateway: String,
    pub interface: String,
    pub prefix_length: u8,
    pub metric: u32,
    pub protocol: String,
    pub is_active: bool,
}

impl Route {
    pub fn new(
        destination: String,
        gateway: String,
        interface: String,
        prefix_length: u8,
        metric: u32,
        protocol: String,
    ) -> Self {
        Self {
            destination,
            gateway,
            interface,
            prefix_length,
            metric,
            protocol,
            is_active: true,
        }
    }
}

/// Routing statistics
#[derive(Debug, Clone)]
pub struct RoutingStats {
    pub total_routes: usize,
    pub active_routes: usize,
    pub bgp_routes: usize,
    pub ospf_routes: usize,
    pub isis_routes: usize,
    pub static_routes: usize,
    pub last_update: u64,
}

/// Packet processing statistics
#[derive(Debug, Clone)]
pub struct PacketStats {
    pub total_packets: u64,
    pub total_bytes: u64,
    pub packets_per_second: f64,
    pub bytes_per_second: f64,
    pub dropped_packets: u64,
    pub dropped_bytes: u64,
    pub average_packet_size: f64,
    pub last_packet_time: u64,
}

/// Network interface statistics
#[derive(Debug, Clone)]
pub struct InterfaceStats {
    pub name: String,
    pub rx_packets: u64,
    pub tx_packets: u64,
    pub rx_bytes: u64,
    pub tx_bytes: u64,
    pub rx_errors: u64,
    pub tx_errors: u64,
    pub is_up: bool,
}

/// Traffic shaping statistics
#[derive(Debug, Clone)]
pub struct TrafficShapingStats {
    pub total_packets_processed: u64,
    pub total_bytes_processed: u64,
    pub packets_dropped: u64,
    pub bytes_dropped: u64,
    pub utilization_percentage: f64,
    pub queue_lengths: Vec<u32>,
    pub queue_utilizations: Vec<f64>,
}

/// BGP neighbor statistics
#[derive(Debug, Clone)]
pub struct BGPNeighborStats {
    pub ip_address: String,
    pub as_number: u32,
    pub state: String,
    pub hold_time: u32,
    pub keepalive: u32,
    pub routes_received: u64,
    pub routes_advertised: u64,
    pub last_update: u64,
}

/// OSPF area statistics
#[derive(Debug, Clone)]
pub struct OSPFAreaStats {
    pub area_id: String,
    pub area_type: String,
    pub interfaces: Vec<String>,
    pub networks: Vec<String>,
    pub lsas_count: u64,
    pub last_update: u64,
}

/// IS-IS level statistics
#[derive(Debug, Clone)]
pub struct ISISLevelStats {
    pub level: u8,
    pub system_id: String,
    pub interfaces: Vec<String>,
    pub networks: Vec<String>,
    pub lsps_count: u64,
    pub last_update: u64,
}

/// Router performance metrics
#[derive(Debug, Clone)]
pub struct PerformanceMetrics {
    pub cpu_usage: f64,
    pub memory_usage: f64,
    pub disk_usage: f64,
    pub network_usage: f64,
    pub packet_processing_rate: f64,
    pub route_update_rate: f64,
    pub timestamp: u64,
}

/// Network topology representation
#[derive(Debug, Clone)]
pub struct NetworkTopology {
    pub nodes: Vec<NetworkNode>,
    pub links: Vec<NetworkLink>,
    pub last_update: u64,
}

#[derive(Debug, Clone)]
pub struct NetworkNode {
    pub id: String,
    pub name: String,
    pub node_type: String,
    pub ip_address: String,
    pub region: String,
    pub status: String,
    pub properties: HashMap<String, String>,
}

#[derive(Debug, Clone)]
pub struct NetworkLink {
    pub id: String,
    pub source_node: String,
    pub dest_node: String,
    pub bandwidth: u64,
    pub latency: u32,
    pub status: String,
    pub properties: HashMap<String, String>,
}

/// Analytics query interface
pub trait AnalyticsQuery {
    fn query_packets(&self, filter: &PacketFilter) -> Result<Vec<Packet>, String>;
    fn query_routes(&self, filter: &RouteFilter) -> Result<Vec<Route>, String>;
    fn query_metrics(&self, filter: &MetricFilter) -> Result<Vec<MetricData>, String>;
}

/// Packet filter for queries
#[derive(Debug, Clone)]
pub struct PacketFilter {
    pub source_ip: Option<String>,
    pub dest_ip: Option<String>,
    pub protocol: Option<u8>,
    pub port_range: Option<(u16, u16)>,
    pub time_range: Option<(u64, u64)>,
    pub size_range: Option<(u32, u32)>,
}

/// Route filter for queries
#[derive(Debug, Clone)]
pub struct RouteFilter {
    pub destination: Option<String>,
    pub protocol: Option<String>,
    pub interface: Option<String>,
    pub metric_range: Option<(u32, u32)>,
    pub is_active: Option<bool>,
}

/// Metric filter for queries
#[derive(Debug, Clone)]
pub struct MetricFilter {
    pub name: Option<String>,
    pub time_range: Option<(u64, u64)>,
    pub value_range: Option<(f64, f64)>,
}

/// Metric data point
#[derive(Debug, Clone)]
pub struct MetricData {
    pub name: String,
    pub value: f64,
    pub timestamp: u64,
    pub tags: HashMap<String, String>,
}

impl Default for RouterAnalytics {
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_router_analytics_creation() {
        let analytics = RouterAnalytics::new();
        assert!(analytics.get_metrics().is_ok());
    }

    #[test]
    fn test_packet_processing() {
        let analytics = RouterAnalytics::new();
        let packet = Packet::new(
            1,
            1500,
            1,
            "192.168.1.1".to_string(),
            "192.168.1.2".to_string(),
            80,
            8080,
            6,
        );
        
        assert!(analytics.process_packet(&packet).is_ok());
    }

    #[test]
    fn test_route_management() {
        let analytics = RouterAnalytics::new();
        let route = Route::new(
            "192.168.1.0/24".to_string(),
            "192.168.1.1".to_string(),
            "eth0".to_string(),
            24,
            1,
            "static".to_string(),
        );
        
        assert!(analytics.add_route(&route).is_ok());
        assert!(analytics.remove_route("192.168.1.0/24").is_ok());
    }

    #[test]
    fn test_metric_updates() {
        let analytics = RouterAnalytics::new();
        assert!(analytics.update_metric("cpu_usage", 75.5).is_ok());
        assert!(analytics.update_metric("memory_usage", 60.0).is_ok());
        
        let metrics = analytics.get_metrics().unwrap();
        assert_eq!(metrics.get("cpu_usage"), Some(&75.5));
        assert_eq!(metrics.get("memory_usage"), Some(&60.0));
    }
}