//! ClickHouse Analytics Engine
//! 
//! High-performance analytics engine using ClickHouse for router simulation data

use std::collections::HashMap;
use std::sync::Arc;
use std::time::{SystemTime, UNIX_EPOCH};
use serde::{Deserialize, Serialize};
use tokio::sync::RwLock;

/// ClickHouse analytics engine
pub struct ClickHouseAnalytics {
    client: clickhouse::Client,
    metrics_cache: Arc<RwLock<HashMap<String, MetricValue>>>,
    batch_size: usize,
    flush_interval_ms: u64,
}

/// Metric types for analytics
#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum MetricType {
    Counter,
    Gauge,
    Histogram,
    Summary,
}

/// Metric value with timestamp
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct MetricValue {
    pub name: String,
    pub value: f64,
    pub timestamp: u64,
    pub labels: HashMap<String, String>,
    pub metric_type: MetricType,
}

/// Packet analytics data
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct PacketAnalytics {
    pub packet_id: u64,
    pub timestamp: u64,
    pub src_ip: u32,
    pub dst_ip: u32,
    pub src_port: u16,
    pub dst_port: u16,
    pub protocol: u8,
    pub size: u32,
    pub ttl: u8,
    pub priority: u32,
    pub processing_time_ns: u64,
    pub route_hop_count: u8,
    pub interface: String,
    pub success: bool,
}

/// Routing analytics data
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct RoutingAnalytics {
    pub timestamp: u64,
    pub protocol: String,
    pub route_count: u32,
    pub neighbor_count: u32,
    pub convergence_time_ms: u64,
    pub update_count: u32,
    pub withdraw_count: u32,
    pub error_count: u32,
}

/// Traffic shaping analytics
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct TrafficShapingAnalytics {
    pub timestamp: u64,
    pub interface: String,
    pub algorithm: String,
    pub rate_bps: u64,
    pub burst_size: u64,
    pub tokens_available: u64,
    pub packets_queued: u32,
    pub packets_dropped: u32,
    pub packets_shaped: u32,
    pub average_delay_ms: f64,
}

/// Network impairment analytics
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ImpairmentAnalytics {
    pub timestamp: u64,
    pub interface: String,
    pub impairment_type: String,
    pub packets_affected: u32,
    pub packets_dropped: u32,
    pub packets_delayed: u32,
    pub packets_corrupted: u32,
    pub average_delay_ms: f64,
    pub loss_rate: f64,
}

/// Performance analytics
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct PerformanceAnalytics {
    pub timestamp: u64,
    pub component: String,
    pub cpu_usage: f64,
    pub memory_usage: f64,
    pub disk_usage: f64,
    pub network_usage: f64,
    pub packets_per_second: f64,
    pub bytes_per_second: f64,
    pub average_latency_ns: u64,
    pub max_latency_ns: u64,
    pub min_latency_ns: u64,
}

impl ClickHouseAnalytics {
    /// Create a new ClickHouse analytics engine
    pub fn new(host: &str, port: u16, database: &str, username: &str, password: &str) -> Result<Self, String> {
        let client = clickhouse::Client::default()
            .with_url(format!("http://{}:{}", host, port))
            .with_database(database)
            .with_user(username)
            .with_password(password)
            .map_err(|e| format!("Failed to create ClickHouse client: {}", e))?;

        Ok(Self {
            client,
            metrics_cache: Arc::new(RwLock::new(HashMap::new())),
            batch_size: 1000,
            flush_interval_ms: 5000,
        })
    }

    /// Initialize ClickHouse tables
    pub async fn initialize_tables(&self) -> Result<(), String> {
        // Create packet analytics table
        let packet_table_sql = r#"
            CREATE TABLE IF NOT EXISTS packet_analytics (
                packet_id UInt64,
                timestamp UInt64,
                src_ip UInt32,
                dst_ip UInt32,
                src_port UInt16,
                dst_port UInt16,
                protocol UInt8,
                size UInt32,
                ttl UInt8,
                priority UInt32,
                processing_time_ns UInt64,
                route_hop_count UInt8,
                interface String,
                success UInt8,
                date Date MATERIALIZED toDate(timestamp / 1000)
            ) ENGINE = MergeTree()
            PARTITION BY date
            ORDER BY (timestamp, packet_id)
        "#;

        self.client
            .query(packet_table_sql)
            .execute()
            .await
            .map_err(|e| format!("Failed to create packet analytics table: {}", e))?;

        // Create routing analytics table
        let routing_table_sql = r#"
            CREATE TABLE IF NOT EXISTS routing_analytics (
                timestamp UInt64,
                protocol String,
                route_count UInt32,
                neighbor_count UInt32,
                convergence_time_ms UInt64,
                update_count UInt32,
                withdraw_count UInt32,
                error_count UInt32,
                date Date MATERIALIZED toDate(timestamp / 1000)
            ) ENGINE = MergeTree()
            PARTITION BY date
            ORDER BY (timestamp, protocol)
        "#;

        self.client
            .query(routing_table_sql)
            .execute()
            .await
            .map_err(|e| format!("Failed to create routing analytics table: {}", e))?;

        // Create traffic shaping analytics table
        let traffic_table_sql = r#"
            CREATE TABLE IF NOT EXISTS traffic_shaping_analytics (
                timestamp UInt64,
                interface String,
                algorithm String,
                rate_bps UInt64,
                burst_size UInt64,
                tokens_available UInt64,
                packets_queued UInt32,
                packets_dropped UInt32,
                packets_shaped UInt32,
                average_delay_ms Float64,
                date Date MATERIALIZED toDate(timestamp / 1000)
            ) ENGINE = MergeTree()
            PARTITION BY date
            ORDER BY (timestamp, interface)
        "#;

        self.client
            .query(traffic_table_sql)
            .execute()
            .await
            .map_err(|e| format!("Failed to create traffic shaping analytics table: {}", e))?;

        // Create impairment analytics table
        let impairment_table_sql = r#"
            CREATE TABLE IF NOT EXISTS impairment_analytics (
                timestamp UInt64,
                interface String,
                impairment_type String,
                packets_affected UInt32,
                packets_dropped UInt32,
                packets_delayed UInt32,
                packets_corrupted UInt32,
                average_delay_ms Float64,
                loss_rate Float64,
                date Date MATERIALIZED toDate(timestamp / 1000)
            ) ENGINE = MergeTree()
            PARTITION BY date
            ORDER BY (timestamp, interface)
        "#;

        self.client
            .query(impairment_table_sql)
            .execute()
            .await
            .map_err(|e| format!("Failed to create impairment analytics table: {}", e))?;

        // Create performance analytics table
        let performance_table_sql = r#"
            CREATE TABLE IF NOT EXISTS performance_analytics (
                timestamp UInt64,
                component String,
                cpu_usage Float64,
                memory_usage Float64,
                disk_usage Float64,
                network_usage Float64,
                packets_per_second Float64,
                bytes_per_second Float64,
                average_latency_ns UInt64,
                max_latency_ns UInt64,
                min_latency_ns UInt64,
                date Date MATERIALIZED toDate(timestamp / 1000)
            ) ENGINE = MergeTree()
            PARTITION BY date
            ORDER BY (timestamp, component)
        "#;

        self.client
            .query(performance_table_sql)
            .execute()
            .await
            .map_err(|e| format!("Failed to create performance analytics table: {}", e))?;

        Ok(())
    }

    /// Record packet analytics
    pub async fn record_packet_analytics(&self, analytics: PacketAnalytics) -> Result<(), String> {
        let insert_sql = r#"
            INSERT INTO packet_analytics (
                packet_id, timestamp, src_ip, dst_ip, src_port, dst_port,
                protocol, size, ttl, priority, processing_time_ns,
                route_hop_count, interface, success
            ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        "#;

        self.client
            .query(insert_sql)
            .bind(analytics.packet_id)
            .bind(analytics.timestamp)
            .bind(analytics.src_ip)
            .bind(analytics.dst_ip)
            .bind(analytics.src_port)
            .bind(analytics.dst_port)
            .bind(analytics.protocol)
            .bind(analytics.size)
            .bind(analytics.ttl)
            .bind(analytics.priority)
            .bind(analytics.processing_time_ns)
            .bind(analytics.route_hop_count)
            .bind(analytics.interface)
            .bind(if analytics.success { 1u8 } else { 0u8 })
            .execute()
            .await
            .map_err(|e| format!("Failed to insert packet analytics: {}", e))?;

        Ok(())
    }

    /// Record routing analytics
    pub async fn record_routing_analytics(&self, analytics: RoutingAnalytics) -> Result<(), String> {
        let insert_sql = r#"
            INSERT INTO routing_analytics (
                timestamp, protocol, route_count, neighbor_count,
                convergence_time_ms, update_count, withdraw_count, error_count
            ) VALUES (?, ?, ?, ?, ?, ?, ?, ?)
        "#;

        self.client
            .query(insert_sql)
            .bind(analytics.timestamp)
            .bind(analytics.protocol)
            .bind(analytics.route_count)
            .bind(analytics.neighbor_count)
            .bind(analytics.convergence_time_ms)
            .bind(analytics.update_count)
            .bind(analytics.withdraw_count)
            .bind(analytics.error_count)
            .execute()
            .await
            .map_err(|e| format!("Failed to insert routing analytics: {}", e))?;

        Ok(())
    }

    /// Record traffic shaping analytics
    pub async fn record_traffic_shaping_analytics(&self, analytics: TrafficShapingAnalytics) -> Result<(), String> {
        let insert_sql = r#"
            INSERT INTO traffic_shaping_analytics (
                timestamp, interface, algorithm, rate_bps, burst_size,
                tokens_available, packets_queued, packets_dropped,
                packets_shaped, average_delay_ms
            ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        "#;

        self.client
            .query(insert_sql)
            .bind(analytics.timestamp)
            .bind(analytics.interface)
            .bind(analytics.algorithm)
            .bind(analytics.rate_bps)
            .bind(analytics.burst_size)
            .bind(analytics.tokens_available)
            .bind(analytics.packets_queued)
            .bind(analytics.packets_dropped)
            .bind(analytics.packets_shaped)
            .bind(analytics.average_delay_ms)
            .execute()
            .await
            .map_err(|e| format!("Failed to insert traffic shaping analytics: {}", e))?;

        Ok(())
    }

    /// Record impairment analytics
    pub async fn record_impairment_analytics(&self, analytics: ImpairmentAnalytics) -> Result<(), String> {
        let insert_sql = r#"
            INSERT INTO impairment_analytics (
                timestamp, interface, impairment_type, packets_affected,
                packets_dropped, packets_delayed, packets_corrupted,
                average_delay_ms, loss_rate
            ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
        "#;

        self.client
            .query(insert_sql)
            .bind(analytics.timestamp)
            .bind(analytics.interface)
            .bind(analytics.impairment_type)
            .bind(analytics.packets_affected)
            .bind(analytics.packets_dropped)
            .bind(analytics.packets_delayed)
            .bind(analytics.packets_corrupted)
            .bind(analytics.average_delay_ms)
            .bind(analytics.loss_rate)
            .execute()
            .await
            .map_err(|e| format!("Failed to insert impairment analytics: {}", e))?;

        Ok(())
    }

    /// Record performance analytics
    pub async fn record_performance_analytics(&self, analytics: PerformanceAnalytics) -> Result<(), String> {
        let insert_sql = r#"
            INSERT INTO performance_analytics (
                timestamp, component, cpu_usage, memory_usage, disk_usage,
                network_usage, packets_per_second, bytes_per_second,
                average_latency_ns, max_latency_ns, min_latency_ns
            ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        "#;

        self.client
            .query(insert_sql)
            .bind(analytics.timestamp)
            .bind(analytics.component)
            .bind(analytics.cpu_usage)
            .bind(analytics.memory_usage)
            .bind(analytics.disk_usage)
            .bind(analytics.network_usage)
            .bind(analytics.packets_per_second)
            .bind(analytics.bytes_per_second)
            .bind(analytics.average_latency_ns)
            .bind(analytics.max_latency_ns)
            .bind(analytics.min_latency_ns)
            .execute()
            .await
            .map_err(|e| format!("Failed to insert performance analytics: {}", e))?;

        Ok(())
    }

    /// Get current timestamp in milliseconds
    pub fn current_timestamp_ms() -> u64 {
        SystemTime::now()
            .duration_since(UNIX_EPOCH)
            .unwrap_or_default()
            .as_millis() as u64
    }

    /// Query packet analytics with filters
    pub async fn query_packet_analytics(
        &self,
        start_time: u64,
        end_time: u64,
        interface: Option<&str>,
        protocol: Option<u8>,
    ) -> Result<Vec<PacketAnalytics>, String> {
        let mut query = "SELECT * FROM packet_analytics WHERE timestamp >= ? AND timestamp <= ?".to_string();
        let mut params: Vec<Box<dyn clickhouse::ToSql>> = vec![
            Box::new(start_time),
            Box::new(end_time),
        ];

        if let Some(iface) = interface {
            query.push_str(" AND interface = ?");
            params.push(Box::new(iface.to_string()));
        }

        if let Some(proto) = protocol {
            query.push_str(" AND protocol = ?");
            params.push(Box::new(proto));
        }

        query.push_str(" ORDER BY timestamp DESC LIMIT 1000");

        let mut rows = self.client
            .query(&query)
            .bind(start_time)
            .bind(end_time)
            .fetch_all::<PacketAnalytics>()
            .await
            .map_err(|e| format!("Failed to query packet analytics: {}", e))?;

        Ok(rows)
    }

    /// Get performance summary
    pub async fn get_performance_summary(&self, component: &str, hours: u32) -> Result<PerformanceSummary, String> {
        let end_time = Self::current_timestamp_ms();
        let start_time = end_time - (hours as u64 * 3600 * 1000);

        let query = r#"
            SELECT 
                AVG(cpu_usage) as avg_cpu,
                AVG(memory_usage) as avg_memory,
                AVG(disk_usage) as avg_disk,
                AVG(network_usage) as avg_network,
                AVG(packets_per_second) as avg_pps,
                AVG(bytes_per_second) as avg_bps,
                AVG(average_latency_ns) as avg_latency,
                MAX(max_latency_ns) as max_latency,
                MIN(min_latency_ns) as min_latency
            FROM performance_analytics 
            WHERE component = ? AND timestamp >= ? AND timestamp <= ?
        "#;

        let row = self.client
            .query(query)
            .bind(component)
            .bind(start_time)
            .bind(end_time)
            .fetch_one::<PerformanceSummaryRow>()
            .await
            .map_err(|e| format!("Failed to query performance summary: {}", e))?;

        Ok(PerformanceSummary {
            component: component.to_string(),
            avg_cpu: row.avg_cpu,
            avg_memory: row.avg_memory,
            avg_disk: row.avg_disk,
            avg_network: row.avg_network,
            avg_packets_per_second: row.avg_pps,
            avg_bytes_per_second: row.avg_bps,
            avg_latency_ns: row.avg_latency as u64,
            max_latency_ns: row.max_latency,
            min_latency_ns: row.min_latency,
        })
    }
}

/// Performance summary row for ClickHouse queries
#[derive(Debug, Deserialize)]
struct PerformanceSummaryRow {
    avg_cpu: f64,
    avg_memory: f64,
    avg_disk: f64,
    avg_network: f64,
    avg_pps: f64,
    avg_bps: f64,
    avg_latency: f64,
    max_latency: u64,
    min_latency: u64,
}

/// Performance summary
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct PerformanceSummary {
    pub component: String,
    pub avg_cpu: f64,
    pub avg_memory: f64,
    pub avg_disk: f64,
    pub avg_network: f64,
    pub avg_packets_per_second: f64,
    pub avg_bytes_per_second: f64,
    pub avg_latency_ns: u64,
    pub max_latency_ns: u64,
    pub min_latency_ns: u64,
}

#[cfg(test)]
mod tests {
    use super::*;

    #[tokio::test]
    async fn test_clickhouse_analytics_creation() {
        // This would require a running ClickHouse instance
        // For now, just test the structure
        let analytics = ClickHouseAnalytics::new("localhost", 9000, "test", "default", "");
        assert!(analytics.is_ok());
    }

    #[test]
    fn test_current_timestamp() {
        let timestamp = ClickHouseAnalytics::current_timestamp_ms();
        assert!(timestamp > 0);
    }
}
