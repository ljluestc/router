pub mod packet_engine;
pub mod routing_table;
pub mod analytics;
pub mod cloudpods;
pub mod metrics;

use std::time::Duration;
use serde::{Deserialize, Serialize};

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct AnalyticsConfig {
    pub packet_engine: PacketEngineConfig,
    pub analytics: AnalyticsEngineConfig,
    pub cloudpods: CloudPodsConfig,
    pub metrics: MetricsConfig,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct PacketEngineConfig {
    pub buffer_size: usize,
    pub max_packet_size: usize,
    pub enable_offload: bool,
    pub worker_threads: usize,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct AnalyticsEngineConfig {
    pub clickhouse_url: String,
    pub batch_size: usize,
    pub flush_interval: Duration,
    pub enable_compression: bool,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct CloudPodsConfig {
    pub endpoint: String,
    pub username: String,
    pub password: String,
    pub timeout: Duration,
    pub retry_attempts: u32,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct MetricsConfig {
    pub prometheus_port: u16,
    pub collect_interval: Duration,
    pub enable_histograms: bool,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ParsedPacket {
    pub timestamp: u64,
    pub src_ip: String,
    pub dst_ip: String,
    pub protocol: String,
    pub src_port: u16,
    pub dst_port: u16,
    pub bytes: u32,
    pub routing_update: Option<RoutingUpdate>,
    pub flow_id: String,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct RoutingUpdate {
    pub protocol: String,
    pub prefix: String,
    pub next_hop: String,
    pub metric: u32,
    pub as_path: Vec<u32>,
    pub community: Vec<String>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct RoutingStats {
    pub total_routes: u32,
    pub bgp_routes: u32,
    pub ospf_routes: u32,
    pub isis_routes: u32,
    pub convergence_time: Duration,
    pub last_update: u64,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct TrafficStats {
    pub total_packets: u64,
    pub total_bytes: u64,
    pub packets_per_second: f64,
    pub bytes_per_second: f64,
    pub top_protocols: Vec<(String, u64)>,
    pub top_flows: Vec<(String, u64)>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct CloudPodsStats {
    pub vpc_count: u32,
    pub nat_gateway_count: u32,
    pub load_balancer_count: u32,
    pub service_mesh_count: u32,
    pub total_traffic: u64,
    pub active_connections: u32,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct PerformanceMetrics {
    pub cpu_usage: f64,
    pub memory_usage: f64,
    pub packet_processing_rate: f64,
    pub average_latency: f64,
    pub error_rate: f64,
}