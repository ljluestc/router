use crate::packet_engine::PacketStats;
use std::sync::Arc;
use std::sync::atomic::{AtomicU64, Ordering};
use std::collections::HashMap;
use tokio::sync::RwLock;
use tokio::time::{Duration, Instant};
use serde::{Serialize, Deserialize};

/// Analytics engine for collecting and processing network metrics
pub struct AnalyticsEngine {
    packet_stats: Arc<PacketStats>,
    flow_table: Arc<RwLock<HashMap<String, TrafficFlow>>>,
    route_metrics: Arc<RwLock<Vec<RouteMetrics>>>,
    system_metrics: Arc<RwLock<SystemMetrics>>,
    start_time: Instant,
}

/// Traffic flow tracking
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct TrafficFlow {
    pub flow_id: String,
    pub source_ip: String,
    pub destination_ip: String,
    pub source_port: u16,
    pub destination_port: u16,
    pub protocol: u8,
    pub packet_count: u64,
    pub byte_count: u64,
    pub first_seen: Instant,
    pub last_seen: Instant,
    pub application: String,
    pub dscp: u8,
    pub is_encrypted: bool,
    pub traffic_class: String,
}

/// Route metrics
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct RouteMetrics {
    pub timestamp: Instant,
    pub network: String,
    pub prefix_length: u8,
    pub next_hop: String,
    pub metric: u32,
    pub protocol: String,
    pub as_path_length: u32,
    pub communities: String,
    pub is_active: bool,
    pub age: Duration,
    pub packet_count: u32,
    pub byte_count: u64,
}

/// System metrics
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct SystemMetrics {
    pub timestamp: Instant,
    pub cpu_usage: f64,
    pub memory_usage: f64,
    pub disk_usage: f64,
    pub total_packets_processed: u64,
    pub total_bytes_processed: u64,
    pub packets_dropped: u64,
    pub routing_table_size: u64,
    pub active_neighbors: u32,
    pub active_interfaces: u32,
    pub packets_per_second: f64,
    pub bytes_per_second: f64,
    pub average_latency: f64,
    pub packet_loss_rate: f64,
}

/// Analytics statistics
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct AnalyticsStats {
    pub total_flows: u64,
    pub active_flows: u64,
    pub total_routes: u64,
    pub active_routes: u64,
    pub uptime_seconds: u64,
    pub packets_per_second: f64,
    pub bytes_per_second: f64,
    pub average_flow_duration: f64,
    pub top_applications: Vec<(String, u64)>,
    pub top_protocols: Vec<(u8, u64)>,
    pub top_sources: Vec<(String, u64)>,
    pub top_destinations: Vec<(String, u64)>,
}

impl AnalyticsEngine {
    /// Create a new analytics engine
    pub fn new(packet_stats: Arc<PacketStats>) -> Self {
        Self {
            packet_stats,
            flow_table: Arc::new(RwLock::new(HashMap::new())),
            route_metrics: Arc::new(RwLock::new(Vec::new())),
            system_metrics: Arc::new(RwLock::new(SystemMetrics::default())),
            start_time: Instant::now(),
        }
    }

    /// Process a packet and update analytics
    pub async fn process_packet(
        &self,
        source_ip: &str,
        destination_ip: &str,
        source_port: u16,
        destination_port: u16,
        protocol: u8,
        packet_size: u16,
        dscp: u8,
        is_encrypted: bool,
    ) -> Result<(), Box<dyn std::error::Error>> {
        let flow_id = self.generate_flow_id(source_ip, destination_ip, source_port, destination_port, protocol);
        let now = Instant::now();

        // Update flow table
        let mut flows = self.flow_table.write().await;
        if let Some(flow) = flows.get_mut(&flow_id) {
            flow.packet_count += 1;
            flow.byte_count += packet_size as u64;
            flow.last_seen = now;
        } else {
            let flow = TrafficFlow {
                flow_id: flow_id.clone(),
                source_ip: source_ip.to_string(),
                destination_ip: destination_ip.to_string(),
                source_port,
                destination_port,
                protocol,
                packet_count: 1,
                byte_count: packet_size as u64,
                first_seen: now,
                last_seen: now,
                application: self.detect_application(protocol, destination_port),
                dscp,
                is_encrypted,
                traffic_class: self.classify_traffic(dscp, protocol),
            };
            flows.insert(flow_id, flow);
        }

        Ok(())
    }

    /// Add route metrics
    pub async fn add_route_metrics(&self, metrics: RouteMetrics) {
        let mut routes = self.route_metrics.write().await;
        routes.push(metrics);
        
        // Keep only last 1000 routes to prevent memory growth
        if routes.len() > 1000 {
            routes.drain(0..routes.len() - 1000);
        }
    }

    /// Update system metrics
    pub async fn update_system_metrics(&self, metrics: SystemMetrics) {
        let mut system = self.system_metrics.write().await;
        *system = metrics;
    }

    /// Get analytics statistics
    pub async fn get_stats(&self) -> AnalyticsStats {
        let flows = self.flow_table.read().await;
        let routes = self.route_metrics.read().await;
        let system = self.system_metrics.read().await;

        let total_flows = flows.len() as u64;
        let active_flows = flows.values()
            .filter(|flow| self.start_time.elapsed() - flow.last_seen < Duration::from_secs(300))
            .count() as u64;

        let total_routes = routes.len() as u64;
        let active_routes = routes.iter()
            .filter(|route| route.is_active)
            .count() as u64;

        let uptime_seconds = self.start_time.elapsed().as_secs();
        let packets_per_second = self.packet_stats.calculate_packets_per_second();
        let bytes_per_second = self.packet_stats.calculate_bytes_per_second();

        let average_flow_duration = if !flows.is_empty() {
            flows.values()
                .map(|flow| flow.last_seen.duration_since(flow.first_seen).as_secs_f64())
                .sum::<f64>() / flows.len() as f64
        } else {
            0.0
        };

        // Calculate top applications
        let mut app_counts: HashMap<String, u64> = HashMap::new();
        for flow in flows.values() {
            *app_counts.entry(flow.application.clone()).or_insert(0) += flow.packet_count;
        }
        let mut top_applications: Vec<(String, u64)> = app_counts.into_iter().collect();
        top_applications.sort_by(|a, b| b.1.cmp(&a.1));
        top_applications.truncate(10);

        // Calculate top protocols
        let mut protocol_counts: HashMap<u8, u64> = HashMap::new();
        for flow in flows.values() {
            *protocol_counts.entry(flow.protocol).or_insert(0) += flow.packet_count;
        }
        let mut top_protocols: Vec<(u8, u64)> = protocol_counts.into_iter().collect();
        top_protocols.sort_by(|a, b| b.1.cmp(&a.1));
        top_protocols.truncate(10);

        // Calculate top sources
        let mut source_counts: HashMap<String, u64> = HashMap::new();
        for flow in flows.values() {
            *source_counts.entry(flow.source_ip.clone()).or_insert(0) += flow.packet_count;
        }
        let mut top_sources: Vec<(String, u64)> = source_counts.into_iter().collect();
        top_sources.sort_by(|a, b| b.1.cmp(&a.1));
        top_sources.truncate(10);

        // Calculate top destinations
        let mut dest_counts: HashMap<String, u64> = HashMap::new();
        for flow in flows.values() {
            *dest_counts.entry(flow.destination_ip.clone()).or_insert(0) += flow.packet_count;
        }
        let mut top_destinations: Vec<(String, u64)> = dest_counts.into_iter().collect();
        top_destinations.sort_by(|a, b| b.1.cmp(&a.1));
        top_destinations.truncate(10);

        AnalyticsStats {
            total_flows,
            active_flows,
            total_routes,
            active_routes,
            uptime_seconds,
            packets_per_second,
            bytes_per_second,
            average_flow_duration,
            top_applications,
            top_protocols,
            top_sources,
            top_destinations,
        }
    }

    /// Get traffic flows
    pub async fn get_flows(&self) -> Vec<TrafficFlow> {
        let flows = self.flow_table.read().await;
        flows.values().cloned().collect()
    }

    /// Get route metrics
    pub async fn get_route_metrics(&self) -> Vec<RouteMetrics> {
        let routes = self.route_metrics.read().await;
        routes.clone()
    }

    /// Get system metrics
    pub async fn get_system_metrics(&self) -> SystemMetrics {
        let system = self.system_metrics.read().await;
        system.clone()
    }

    /// Clean up old flows
    pub async fn cleanup_old_flows(&self, max_age: Duration) {
        let mut flows = self.flow_table.write().await;
        let now = Instant::now();
        flows.retain(|_, flow| now.duration_since(flow.last_seen) < max_age);
    }

    /// Generate flow ID
    fn generate_flow_id(
        &self,
        source_ip: &str,
        destination_ip: &str,
        source_port: u16,
        destination_port: u16,
        protocol: u8,
    ) -> String {
        format!("{}:{}:{}:{}:{}", source_ip, destination_ip, source_port, destination_port, protocol)
    }

    /// Detect application based on protocol and port
    fn detect_application(&self, protocol: u8, port: u16) -> String {
        match protocol {
            6 => { // TCP
                match port {
                    80 => "HTTP".to_string(),
                    443 => "HTTPS".to_string(),
                    22 => "SSH".to_string(),
                    23 => "Telnet".to_string(),
                    25 => "SMTP".to_string(),
                    53 => "DNS".to_string(),
                    110 => "POP3".to_string(),
                    143 => "IMAP".to_string(),
                    993 => "IMAPS".to_string(),
                    995 => "POP3S".to_string(),
                    3389 => "RDP".to_string(),
                    5900 => "VNC".to_string(),
                    _ => "TCP".to_string(),
                }
            }
            17 => { // UDP
                match port {
                    53 => "DNS".to_string(),
                    67 => "DHCP".to_string(),
                    68 => "DHCP".to_string(),
                    123 => "NTP".to_string(),
                    161 => "SNMP".to_string(),
                    162 => "SNMP".to_string(),
                    500 => "IKE".to_string(),
                    4500 => "IPsec".to_string(),
                    _ => "UDP".to_string(),
                }
            }
            1 => "ICMP".to_string(),
            2 => "IGMP".to_string(),
            41 => "IPv6".to_string(),
            47 => "GRE".to_string(),
            50 => "ESP".to_string(),
            51 => "AH".to_string(),
            89 => "OSPF".to_string(),
            _ => "Unknown".to_string(),
        }
    }

    /// Classify traffic based on DSCP and protocol
    fn classify_traffic(&self, dscp: u8, protocol: u8) -> String {
        match dscp {
            0 => "Best Effort".to_string(),
            10 => "AF11".to_string(),
            12 => "AF12".to_string(),
            14 => "AF13".to_string(),
            18 => "AF21".to_string(),
            20 => "AF22".to_string(),
            22 => "AF23".to_string(),
            26 => "AF31".to_string(),
            28 => "AF32".to_string(),
            30 => "AF33".to_string(),
            34 => "AF41".to_string(),
            36 => "AF42".to_string(),
            38 => "AF43".to_string(),
            46 => "EF".to_string(),
            48 => "CS6".to_string(),
            56 => "CS7".to_string(),
            _ => "Unknown".to_string(),
        }
    }
}

impl Default for SystemMetrics {
    fn default() -> Self {
        Self {
            timestamp: Instant::now(),
            cpu_usage: 0.0,
            memory_usage: 0.0,
            disk_usage: 0.0,
            total_packets_processed: 0,
            total_bytes_processed: 0,
            packets_dropped: 0,
            routing_table_size: 0,
            active_neighbors: 0,
            active_interfaces: 0,
            packets_per_second: 0.0,
            bytes_per_second: 0.0,
            average_latency: 0.0,
            packet_loss_rate: 0.0,
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::packet_engine::PacketStats;

    #[tokio::test]
    async fn test_analytics_engine_creation() {
        let packet_stats = Arc::new(PacketStats::new());
        let engine = AnalyticsEngine::new(packet_stats);
        assert!(engine.start_time.elapsed() < Duration::from_secs(1));
    }

    #[tokio::test]
    async fn test_flow_processing() {
        let packet_stats = Arc::new(PacketStats::new());
        let engine = AnalyticsEngine::new(packet_stats);
        
        engine.process_packet(
            "192.168.1.1",
            "192.168.1.2",
            80,
            8080,
            6,
            1500,
            0,
            false,
        ).await.unwrap();

        let flows = engine.get_flows().await;
        assert_eq!(flows.len(), 1);
        assert_eq!(flows[0].source_ip, "192.168.1.1");
        assert_eq!(flows[0].destination_ip, "192.168.1.2");
        assert_eq!(flows[0].packet_count, 1);
    }

    #[tokio::test]
    async fn test_application_detection() {
        let packet_stats = Arc::new(PacketStats::new());
        let engine = AnalyticsEngine::new(packet_stats);
        
        engine.process_packet(
            "192.168.1.1",
            "192.168.1.2",
            80,
            8080,
            6,
            1500,
            0,
            false,
        ).await.unwrap();

        let flows = engine.get_flows().await;
        assert_eq!(flows[0].application, "HTTP");
    }
}
