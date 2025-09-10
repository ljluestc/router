use std::collections::HashMap;
use std::sync::Arc;
use tokio::sync::RwLock;
use serde::{Deserialize, Serialize};
use chrono::{DateTime, Utc};

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct NetworkMetrics {
    pub timestamp: DateTime<Utc>,
    pub interface: String,
    pub bytes_in: u64,
    pub bytes_out: u64,
    pub packets_in: u64,
    pub packets_out: u64,
    pub errors_in: u64,
    pub errors_out: u64,
    pub drops_in: u64,
    pub drops_out: u64,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct RoutingMetrics {
    pub timestamp: DateTime<Utc>,
    pub protocol: String,
    pub routes_count: u32,
    pub neighbors_count: u32,
    pub convergence_time_ms: u32,
    pub updates_sent: u64,
    pub updates_received: u64,
    pub withdrawals_sent: u64,
    pub withdrawals_received: u64,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct TrafficShapingMetrics {
    pub timestamp: DateTime<Utc>,
    pub algorithm: String,
    pub packets_processed: u64,
    pub packets_dropped: u64,
    pub bytes_processed: u64,
    pub bytes_dropped: u64,
    pub utilization_percentage: f64,
    pub queue_depth: u32,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ImpairmentMetrics {
    pub timestamp: DateTime<Utc>,
    pub interface: String,
    pub impairment_type: String,
    pub packets_affected: u64,
    pub loss_percentage: f64,
    pub delay_ms: f64,
    pub jitter_ms: f64,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct CloudResourceMetrics {
    pub timestamp: DateTime<Utc>,
    pub provider: String,
    pub resource_type: String,
    pub resource_id: String,
    pub region: String,
    pub status: String,
    pub cpu_usage: f64,
    pub memory_usage: f64,
    pub network_usage: f64,
    pub cost_per_hour: f64,
}

pub struct ClickHouseClient {
    client: clickhouse_rs::Pool,
    config: ClickHouseConfig,
}

#[derive(Debug, Clone)]
pub struct ClickHouseConfig {
    pub host: String,
    pub port: u16,
    pub database: String,
    pub username: String,
    pub password: String,
    pub max_connections: u32,
}

impl ClickHouseClient {
    pub async fn new(config: ClickHouseConfig) -> Result<Self, Box<dyn std::error::Error>> {
        let ddl = format!(
            "CREATE DATABASE IF NOT EXISTS {}",
            config.database
        );
        
        let pool = clickhouse_rs::Pool::new(format!(
            "tcp://{}:{}?database={}&username={}&password={}&pool_max={}",
            config.host,
            config.port,
            config.database,
            config.username,
            config.password,
            config.max_connections
        ));
        
        let mut client = ClickHouseClient {
            client: pool,
            config: config.clone(),
        };
        
        // Create database if it doesn't exist
        client.execute_ddl(&ddl).await?;
        
        // Create tables
        client.create_tables().await?;
        
        Ok(client)
    }
    
    async fn create_tables(&self) -> Result<(), Box<dyn std::error::Error>> {
        let tables = vec![
            // Network metrics table
            r#"
            CREATE TABLE IF NOT EXISTS network_metrics (
                timestamp DateTime64(3),
                interface String,
                bytes_in UInt64,
                bytes_out UInt64,
                packets_in UInt64,
                packets_out UInt64,
                errors_in UInt64,
                errors_out UInt64,
                drops_in UInt64,
                drops_out UInt64
            ) ENGINE = MergeTree()
            ORDER BY (timestamp, interface)
            TTL timestamp + INTERVAL 30 DAY
            "#,
            
            // Routing metrics table
            r#"
            CREATE TABLE IF NOT EXISTS routing_metrics (
                timestamp DateTime64(3),
                protocol String,
                routes_count UInt32,
                neighbors_count UInt32,
                convergence_time_ms UInt32,
                updates_sent UInt64,
                updates_received UInt64,
                withdrawals_sent UInt64,
                withdrawals_received UInt64
            ) ENGINE = MergeTree()
            ORDER BY (timestamp, protocol)
            TTL timestamp + INTERVAL 30 DAY
            "#,
            
            // Traffic shaping metrics table
            r#"
            CREATE TABLE IF NOT EXISTS traffic_shaping_metrics (
                timestamp DateTime64(3),
                algorithm String,
                packets_processed UInt64,
                packets_dropped UInt64,
                bytes_processed UInt64,
                bytes_dropped UInt64,
                utilization_percentage Float64,
                queue_depth UInt32
            ) ENGINE = MergeTree()
            ORDER BY (timestamp, algorithm)
            TTL timestamp + INTERVAL 30 DAY
            "#,
            
            // Impairment metrics table
            r#"
            CREATE TABLE IF NOT EXISTS impairment_metrics (
                timestamp DateTime64(3),
                interface String,
                impairment_type String,
                packets_affected UInt64,
                loss_percentage Float64,
                delay_ms Float64,
                jitter_ms Float64
            ) ENGINE = MergeTree()
            ORDER BY (timestamp, interface, impairment_type)
            TTL timestamp + INTERVAL 30 DAY
            "#,
            
            // Cloud resource metrics table
            r#"
            CREATE TABLE IF NOT EXISTS cloud_resource_metrics (
                timestamp DateTime64(3),
                provider String,
                resource_type String,
                resource_id String,
                region String,
                status String,
                cpu_usage Float64,
                memory_usage Float64,
                network_usage Float64,
                cost_per_hour Float64
            ) ENGINE = MergeTree()
            ORDER BY (timestamp, provider, resource_type, resource_id)
            TTL timestamp + INTERVAL 30 DAY
            "#,
        ];
        
        for table_ddl in tables {
            self.execute_ddl(table_ddl).await?;
        }
        
        Ok(())
    }
    
    async fn execute_ddl(&self, ddl: &str) -> Result<(), Box<dyn std::error::Error>> {
        let mut session = self.client.get_handle().await?;
        session.execute(ddl).await?;
        Ok(())
    }
    
    pub async fn insert_network_metrics(&self, metrics: &[NetworkMetrics]) -> Result<(), Box<dyn std::error::Error>> {
        let mut session = self.client.get_handle().await?;
        
        let query = r#"
            INSERT INTO network_metrics (
                timestamp, interface, bytes_in, bytes_out, packets_in, packets_out,
                errors_in, errors_out, drops_in, drops_out
            ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        "#;
        
        let mut insert = session.insert(query)?;
        
        for metric in metrics {
            insert.write(&metric.timestamp).await?;
            insert.write(&metric.interface).await?;
            insert.write(&metric.bytes_in).await?;
            insert.write(&metric.bytes_out).await?;
            insert.write(&metric.packets_in).await?;
            insert.write(&metric.packets_out).await?;
            insert.write(&metric.errors_in).await?;
            insert.write(&metric.errors_out).await?;
            insert.write(&metric.drops_in).await?;
            insert.write(&metric.drops_out).await?;
        }
        
        insert.end().await?;
        Ok(())
    }
    
    pub async fn insert_routing_metrics(&self, metrics: &[RoutingMetrics]) -> Result<(), Box<dyn std::error::Error>> {
        let mut session = self.client.get_handle().await?;
        
        let query = r#"
            INSERT INTO routing_metrics (
                timestamp, protocol, routes_count, neighbors_count, convergence_time_ms,
                updates_sent, updates_received, withdrawals_sent, withdrawals_received
            ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
        "#;
        
        let mut insert = session.insert(query)?;
        
        for metric in metrics {
            insert.write(&metric.timestamp).await?;
            insert.write(&metric.protocol).await?;
            insert.write(&metric.routes_count).await?;
            insert.write(&metric.neighbors_count).await?;
            insert.write(&metric.convergence_time_ms).await?;
            insert.write(&metric.updates_sent).await?;
            insert.write(&metric.updates_received).await?;
            insert.write(&metric.withdrawals_sent).await?;
            insert.write(&metric.withdrawals_received).await?;
        }
        
        insert.end().await?;
        Ok(())
    }
    
    pub async fn insert_traffic_shaping_metrics(&self, metrics: &[TrafficShapingMetrics]) -> Result<(), Box<dyn std::error::Error>> {
        let mut session = self.client.get_handle().await?;
        
        let query = r#"
            INSERT INTO traffic_shaping_metrics (
                timestamp, algorithm, packets_processed, packets_dropped,
                bytes_processed, bytes_dropped, utilization_percentage, queue_depth
            ) VALUES (?, ?, ?, ?, ?, ?, ?, ?)
        "#;
        
        let mut insert = session.insert(query)?;
        
        for metric in metrics {
            insert.write(&metric.timestamp).await?;
            insert.write(&metric.algorithm).await?;
            insert.write(&metric.packets_processed).await?;
            insert.write(&metric.packets_dropped).await?;
            insert.write(&metric.bytes_processed).await?;
            insert.write(&metric.bytes_dropped).await?;
            insert.write(&metric.utilization_percentage).await?;
            insert.write(&metric.queue_depth).await?;
        }
        
        insert.end().await?;
        Ok(())
    }
    
    pub async fn insert_impairment_metrics(&self, metrics: &[ImpairmentMetrics]) -> Result<(), Box<dyn std::error::Error>> {
        let mut session = self.client.get_handle().await?;
        
        let query = r#"
            INSERT INTO impairment_metrics (
                timestamp, interface, impairment_type, packets_affected,
                loss_percentage, delay_ms, jitter_ms
            ) VALUES (?, ?, ?, ?, ?, ?, ?)
        "#;
        
        let mut insert = session.insert(query)?;
        
        for metric in metrics {
            insert.write(&metric.timestamp).await?;
            insert.write(&metric.interface).await?;
            insert.write(&metric.impairment_type).await?;
            insert.write(&metric.packets_affected).await?;
            insert.write(&metric.loss_percentage).await?;
            insert.write(&metric.delay_ms).await?;
            insert.write(&metric.jitter_ms).await?;
        }
        
        insert.end().await?;
        Ok(())
    }
    
    pub async fn insert_cloud_resource_metrics(&self, metrics: &[CloudResourceMetrics]) -> Result<(), Box<dyn std::error::Error>> {
        let mut session = self.client.get_handle().await?;
        
        let query = r#"
            INSERT INTO cloud_resource_metrics (
                timestamp, provider, resource_type, resource_id, region, status,
                cpu_usage, memory_usage, network_usage, cost_per_hour
            ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        "#;
        
        let mut insert = session.insert(query)?;
        
        for metric in metrics {
            insert.write(&metric.timestamp).await?;
            insert.write(&metric.provider).await?;
            insert.write(&metric.resource_type).await?;
            insert.write(&metric.resource_id).await?;
            insert.write(&metric.region).await?;
            insert.write(&metric.status).await?;
            insert.write(&metric.cpu_usage).await?;
            insert.write(&metric.memory_usage).await?;
            insert.write(&metric.network_usage).await?;
            insert.write(&metric.cost_per_hour).await?;
        }
        
        insert.end().await?;
        Ok(())
    }
    
    pub async fn query_network_metrics(
        &self,
        start_time: DateTime<Utc>,
        end_time: DateTime<Utc>,
        interface: Option<&str>,
    ) -> Result<Vec<NetworkMetrics>, Box<dyn std::error::Error>> {
        let mut session = self.client.get_handle().await?;
        
        let mut query = r#"
            SELECT timestamp, interface, bytes_in, bytes_out, packets_in, packets_out,
                   errors_in, errors_out, drops_in, drops_out
            FROM network_metrics
            WHERE timestamp >= ? AND timestamp <= ?
        "#.to_string();
        
        let mut params = vec![start_time, end_time];
        
        if let Some(iface) = interface {
            query.push_str(" AND interface = ?");
            params.push(iface.into());
        }
        
        query.push_str(" ORDER BY timestamp DESC");
        
        let mut cursor = session.query(&query, &params).await?;
        let mut results = Vec::new();
        
        while let Some(row) = cursor.next().await? {
            let metric = NetworkMetrics {
                timestamp: row.get("timestamp")?,
                interface: row.get("interface")?,
                bytes_in: row.get("bytes_in")?,
                bytes_out: row.get("bytes_out")?,
                packets_in: row.get("packets_in")?,
                packets_out: row.get("packets_out")?,
                errors_in: row.get("errors_in")?,
                errors_out: row.get("errors_out")?,
                drops_in: row.get("drops_in")?,
                drops_out: row.get("drops_out")?,
            };
            results.push(metric);
        }
        
        Ok(results)
    }
    
    pub async fn query_routing_metrics(
        &self,
        start_time: DateTime<Utc>,
        end_time: DateTime<Utc>,
        protocol: Option<&str>,
    ) -> Result<Vec<RoutingMetrics>, Box<dyn std::error::Error>> {
        let mut session = self.client.get_handle().await?;
        
        let mut query = r#"
            SELECT timestamp, protocol, routes_count, neighbors_count, convergence_time_ms,
                   updates_sent, updates_received, withdrawals_sent, withdrawals_received
            FROM routing_metrics
            WHERE timestamp >= ? AND timestamp <= ?
        "#.to_string();
        
        let mut params = vec![start_time, end_time];
        
        if let Some(proto) = protocol {
            query.push_str(" AND protocol = ?");
            params.push(proto.into());
        }
        
        query.push_str(" ORDER BY timestamp DESC");
        
        let mut cursor = session.query(&query, &params).await?;
        let mut results = Vec::new();
        
        while let Some(row) = cursor.next().await? {
            let metric = RoutingMetrics {
                timestamp: row.get("timestamp")?,
                protocol: row.get("protocol")?,
                routes_count: row.get("routes_count")?,
                neighbors_count: row.get("neighbors_count")?,
                convergence_time_ms: row.get("convergence_time_ms")?,
                updates_sent: row.get("updates_sent")?,
                updates_received: row.get("updates_received")?,
                withdrawals_sent: row.get("withdrawals_sent")?,
                withdrawals_received: row.get("withdrawals_received")?,
            };
            results.push(metric);
        }
        
        Ok(results)
    }
    
    pub async fn get_aggregated_metrics(
        &self,
        start_time: DateTime<Utc>,
        end_time: DateTime<Utc>,
    ) -> Result<HashMap<String, f64>, Box<dyn std::error::Error>> {
        let mut session = self.client.get_handle().await?;
        
        let query = r#"
            SELECT 
                'total_bytes' as metric_name,
                sum(bytes_in + bytes_out) as value
            FROM network_metrics
            WHERE timestamp >= ? AND timestamp <= ?
            UNION ALL
            SELECT 
                'total_packets' as metric_name,
                sum(packets_in + packets_out) as value
            FROM network_metrics
            WHERE timestamp >= ? AND timestamp <= ?
            UNION ALL
            SELECT 
                'total_errors' as metric_name,
                sum(errors_in + errors_out) as value
            FROM network_metrics
            WHERE timestamp >= ? AND timestamp <= ?
            UNION ALL
            SELECT 
                'total_drops' as metric_name,
                sum(drops_in + drops_out) as value
            FROM network_metrics
            WHERE timestamp >= ? AND timestamp <= ?
        "#;
        
        let params = vec![start_time, end_time, start_time, end_time, start_time, end_time, start_time, end_time];
        let mut cursor = session.query(query, &params).await?;
        let mut results = HashMap::new();
        
        while let Some(row) = cursor.next().await? {
            let metric_name: String = row.get("metric_name")?;
            let value: f64 = row.get("value")?;
            results.insert(metric_name, value);
        }
        
        Ok(results)
    }
}

pub struct AnalyticsEngine {
    clickhouse: Arc<ClickHouseClient>,
    metrics_buffer: Arc<RwLock<Vec<NetworkMetrics>>>,
    routing_buffer: Arc<RwLock<Vec<RoutingMetrics>>>,
    traffic_shaping_buffer: Arc<RwLock<Vec<TrafficShapingMetrics>>>,
    impairment_buffer: Arc<RwLock<Vec<ImpairmentMetrics>>>,
    cloud_resource_buffer: Arc<RwLock<Vec<CloudResourceMetrics>>>,
}

impl AnalyticsEngine {
    pub async fn new(config: ClickHouseConfig) -> Result<Self, Box<dyn std::error::Error>> {
        let clickhouse = Arc::new(ClickHouseClient::new(config).await?);
        
        Ok(AnalyticsEngine {
            clickhouse,
            metrics_buffer: Arc::new(RwLock::new(Vec::new())),
            routing_buffer: Arc::new(RwLock::new(Vec::new())),
            traffic_shaping_buffer: Arc::new(RwLock::new(Vec::new())),
            impairment_buffer: Arc::new(RwLock::new(Vec::new())),
            cloud_resource_buffer: Arc::new(RwLock::new(Vec::new())),
        })
    }
    
    pub async fn record_network_metrics(&self, metrics: NetworkMetrics) -> Result<(), Box<dyn std::error::Error>> {
        let mut buffer = self.metrics_buffer.write().await;
        buffer.push(metrics);
        
        // Flush buffer if it's getting large
        if buffer.len() >= 1000 {
            let metrics_to_flush = buffer.clone();
            buffer.clear();
            drop(buffer); // Release the lock before async operation
            
            self.clickhouse.insert_network_metrics(&metrics_to_flush).await?;
        }
        
        Ok(())
    }
    
    pub async fn record_routing_metrics(&self, metrics: RoutingMetrics) -> Result<(), Box<dyn std::error::Error>> {
        let mut buffer = self.routing_buffer.write().await;
        buffer.push(metrics);
        
        if buffer.len() >= 1000 {
            let metrics_to_flush = buffer.clone();
            buffer.clear();
            drop(buffer);
            
            self.clickhouse.insert_routing_metrics(&metrics_to_flush).await?;
        }
        
        Ok(())
    }
    
    pub async fn record_traffic_shaping_metrics(&self, metrics: TrafficShapingMetrics) -> Result<(), Box<dyn std::error::Error>> {
        let mut buffer = self.traffic_shaping_buffer.write().await;
        buffer.push(metrics);
        
        if buffer.len() >= 1000 {
            let metrics_to_flush = buffer.clone();
            buffer.clear();
            drop(buffer);
            
            self.clickhouse.insert_traffic_shaping_metrics(&metrics_to_flush).await?;
        }
        
        Ok(())
    }
    
    pub async fn record_impairment_metrics(&self, metrics: ImpairmentMetrics) -> Result<(), Box<dyn std::error::Error>> {
        let mut buffer = self.impairment_buffer.write().await;
        buffer.push(metrics);
        
        if buffer.len() >= 1000 {
            let metrics_to_flush = buffer.clone();
            buffer.clear();
            drop(buffer);
            
            self.clickhouse.insert_impairment_metrics(&metrics_to_flush).await?;
        }
        
        Ok(())
    }
    
    pub async fn record_cloud_resource_metrics(&self, metrics: CloudResourceMetrics) -> Result<(), Box<dyn std::error::Error>> {
        let mut buffer = self.cloud_resource_buffer.write().await;
        buffer.push(metrics);
        
        if buffer.len() >= 1000 {
            let metrics_to_flush = buffer.clone();
            buffer.clear();
            drop(buffer);
            
            self.clickhouse.insert_cloud_resource_metrics(&metrics_to_flush).await?;
        }
        
        Ok(())
    }
    
    pub async fn flush_all_buffers(&self) -> Result<(), Box<dyn std::error::Error>> {
        // Flush network metrics
        {
            let mut buffer = self.metrics_buffer.write().await;
            if !buffer.is_empty() {
                let metrics_to_flush = buffer.clone();
                buffer.clear();
                drop(buffer);
                self.clickhouse.insert_network_metrics(&metrics_to_flush).await?;
            }
        }
        
        // Flush routing metrics
        {
            let mut buffer = self.routing_buffer.write().await;
            if !buffer.is_empty() {
                let metrics_to_flush = buffer.clone();
                buffer.clear();
                drop(buffer);
                self.clickhouse.insert_routing_metrics(&metrics_to_flush).await?;
            }
        }
        
        // Flush traffic shaping metrics
        {
            let mut buffer = self.traffic_shaping_buffer.write().await;
            if !buffer.is_empty() {
                let metrics_to_flush = buffer.clone();
                buffer.clear();
                drop(buffer);
                self.clickhouse.insert_traffic_shaping_metrics(&metrics_to_flush).await?;
            }
        }
        
        // Flush impairment metrics
        {
            let mut buffer = self.impairment_buffer.write().await;
            if !buffer.is_empty() {
                let metrics_to_flush = buffer.clone();
                buffer.clear();
                drop(buffer);
                self.clickhouse.insert_impairment_metrics(&metrics_to_flush).await?;
            }
        }
        
        // Flush cloud resource metrics
        {
            let mut buffer = self.cloud_resource_buffer.write().await;
            if !buffer.is_empty() {
                let metrics_to_flush = buffer.clone();
                buffer.clear();
                drop(buffer);
                self.clickhouse.insert_cloud_resource_metrics(&metrics_to_flush).await?;
            }
        }
        
        Ok(())
    }
    
    pub async fn get_network_metrics(
        &self,
        start_time: DateTime<Utc>,
        end_time: DateTime<Utc>,
        interface: Option<&str>,
    ) -> Result<Vec<NetworkMetrics>, Box<dyn std::error::Error>> {
        self.clickhouse.query_network_metrics(start_time, end_time, interface).await
    }
    
    pub async fn get_routing_metrics(
        &self,
        start_time: DateTime<Utc>,
        end_time: DateTime<Utc>,
        protocol: Option<&str>,
    ) -> Result<Vec<RoutingMetrics>, Box<dyn std::error::Error>> {
        self.clickhouse.query_routing_metrics(start_time, end_time, protocol).await
    }
    
    pub async fn get_aggregated_metrics(
        &self,
        start_time: DateTime<Utc>,
        end_time: DateTime<Utc>,
    ) -> Result<HashMap<String, f64>, Box<dyn std::error::Error>> {
        self.clickhouse.get_aggregated_metrics(start_time, end_time).await
    }
}
