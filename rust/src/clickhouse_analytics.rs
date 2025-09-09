use std::collections::HashMap;
use std::sync::Arc;
use tokio::sync::RwLock;
use serde::{Deserialize, Serialize};
use chrono::{DateTime, Utc};

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct NetworkMetrics {
    pub timestamp: DateTime<Utc>,
    pub router_id: String,
    pub interface: String,
    pub protocol: String,
    pub packets_sent: u64,
    pub packets_received: u64,
    pub bytes_sent: u64,
    pub bytes_received: u64,
    pub latency_ms: f64,
    pub packet_loss_percent: f64,
    pub throughput_bps: u64,
    pub jitter_ms: f64,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct BGPConvergenceMetrics {
    pub timestamp: DateTime<Utc>,
    pub router_id: String,
    pub neighbor: String,
    pub convergence_time_ms: u64,
    pub routes_advertised: u64,
    pub routes_received: u64,
    pub routes_withdrawn: u64,
    pub session_state: String,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct TrafficShapingMetrics {
    pub timestamp: DateTime<Utc>,
    pub router_id: String,
    pub interface: String,
    pub algorithm: String,
    pub packets_processed: u64,
    pub packets_dropped: u64,
    pub bytes_processed: u64,
    pub bytes_dropped: u64,
    pub utilization_percent: f64,
    pub queue_depth: u32,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct NetworkImpairmentMetrics {
    pub timestamp: DateTime<Utc>,
    pub router_id: String,
    pub interface: String,
    pub impairment_type: String,
    pub packets_affected: u64,
    pub packets_delayed: u64,
    pub packets_dropped: u64,
    pub packets_duplicated: u64,
    pub packets_corrupted: u64,
    pub average_delay_ms: f64,
}

pub struct ClickHouseClient {
    client: reqwest::Client,
    base_url: String,
    database: String,
    username: String,
    password: String,
}

impl ClickHouseClient {
    pub fn new(base_url: String, database: String, username: String, password: String) -> Self {
        Self {
            client: reqwest::Client::new(),
            base_url,
            database,
            username,
            password,
        }
    }

    pub async fn insert_network_metrics(&self, metrics: &[NetworkMetrics]) -> Result<(), Box<dyn std::error::Error>> {
        let url = format!("{}/?database={}", self.base_url, self.database);
        
        let mut rows = Vec::new();
        for metric in metrics {
            rows.push(format!(
                "('{}', '{}', '{}', '{}', {}, {}, {}, {}, {}, {}, {}, {})",
                metric.timestamp.format("%Y-%m-%d %H:%M:%S%.3f"),
                metric.router_id,
                metric.interface,
                metric.protocol,
                metric.packets_sent,
                metric.packets_received,
                metric.bytes_sent,
                metric.bytes_received,
                metric.latency_ms,
                metric.packet_loss_percent,
                metric.throughput_bps,
                metric.jitter_ms
            ));
        }

        let query = format!(
            "INSERT INTO network_metrics (timestamp, router_id, interface, protocol, packets_sent, packets_received, bytes_sent, bytes_received, latency_ms, packet_loss_percent, throughput_bps, jitter_ms) VALUES {}",
            rows.join(", ")
        );

        let response = self.client
            .post(&url)
            .basic_auth(&self.username, Some(&self.password))
            .body(query)
            .send()
            .await?;

        if !response.status().is_success() {
            return Err(format!("ClickHouse insert failed: {}", response.status()).into());
        }

        Ok(())
    }

    pub async fn insert_bgp_metrics(&self, metrics: &[BGPConvergenceMetrics]) -> Result<(), Box<dyn std::error::Error>> {
        let url = format!("{}/?database={}", self.base_url, self.database);
        
        let mut rows = Vec::new();
        for metric in metrics {
            rows.push(format!(
                "('{}', '{}', '{}', {}, {}, {}, {}, '{}')",
                metric.timestamp.format("%Y-%m-%d %H:%M:%S%.3f"),
                metric.router_id,
                metric.neighbor,
                metric.convergence_time_ms,
                metric.routes_advertised,
                metric.routes_received,
                metric.routes_withdrawn,
                metric.session_state
            ));
        }

        let query = format!(
            "INSERT INTO bgp_convergence_metrics (timestamp, router_id, neighbor, convergence_time_ms, routes_advertised, routes_received, routes_withdrawn, session_state) VALUES {}",
            rows.join(", ")
        );

        let response = self.client
            .post(&url)
            .basic_auth(&self.username, Some(&self.password))
            .body(query)
            .send()
            .await?;

        if !response.status().is_success() {
            return Err(format!("ClickHouse insert failed: {}", response.status()).into());
        }

        Ok(())
    }

    pub async fn insert_traffic_shaping_metrics(&self, metrics: &[TrafficShapingMetrics]) -> Result<(), Box<dyn std::error::Error>> {
        let url = format!("{}/?database={}", self.base_url, self.database);
        
        let mut rows = Vec::new();
        for metric in metrics {
            rows.push(format!(
                "('{}', '{}', '{}', '{}', {}, {}, {}, {}, {}, {})",
                metric.timestamp.format("%Y-%m-%d %H:%M:%S%.3f"),
                metric.router_id,
                metric.interface,
                metric.algorithm,
                metric.packets_processed,
                metric.packets_dropped,
                metric.bytes_processed,
                metric.bytes_dropped,
                metric.utilization_percent,
                metric.queue_depth
            ));
        }

        let query = format!(
            "INSERT INTO traffic_shaping_metrics (timestamp, router_id, interface, algorithm, packets_processed, packets_dropped, bytes_processed, bytes_dropped, utilization_percent, queue_depth) VALUES {}",
            rows.join(", ")
        );

        let response = self.client
            .post(&url)
            .basic_auth(&self.username, Some(&self.password))
            .body(query)
            .send()
            .await?;

        if !response.status().is_success() {
            return Err(format!("ClickHouse insert failed: {}", response.status()).into());
        }

        Ok(())
    }

    pub async fn insert_impairment_metrics(&self, metrics: &[NetworkImpairmentMetrics]) -> Result<(), Box<dyn std::error::Error>> {
        let url = format!("{}/?database={}", self.base_url, self.database);
        
        let mut rows = Vec::new();
        for metric in metrics {
            rows.push(format!(
                "('{}', '{}', '{}', '{}', {}, {}, {}, {}, {}, {})",
                metric.timestamp.format("%Y-%m-%d %H:%M:%S%.3f"),
                metric.router_id,
                metric.interface,
                metric.impairment_type,
                metric.packets_affected,
                metric.packets_delayed,
                metric.packets_dropped,
                metric.packets_duplicated,
                metric.packets_corrupted,
                metric.average_delay_ms
            ));
        }

        let query = format!(
            "INSERT INTO network_impairment_metrics (timestamp, router_id, interface, impairment_type, packets_affected, packets_delayed, packets_dropped, packets_duplicated, packets_corrupted, average_delay_ms) VALUES {}",
            rows.join(", ")
        );

        let response = self.client
            .post(&url)
            .basic_auth(&self.username, Some(&self.password))
            .body(query)
            .send()
            .await?;

        if !response.status().is_success() {
            return Err(format!("ClickHouse insert failed: {}", response.status()).into());
        }

        Ok(())
    }

    pub async fn query_network_metrics(&self, 
        router_id: Option<&str>, 
        interface: Option<&str>,
        start_time: Option<DateTime<Utc>>,
        end_time: Option<DateTime<Utc>>
    ) -> Result<Vec<NetworkMetrics>, Box<dyn std::error::Error>> {
        let mut conditions = Vec::new();
        
        if let Some(router_id) = router_id {
            conditions.push(format!("router_id = '{}'", router_id));
        }
        
        if let Some(interface) = interface {
            conditions.push(format!("interface = '{}'", interface));
        }
        
        if let Some(start_time) = start_time {
            conditions.push(format!("timestamp >= '{}'", start_time.format("%Y-%m-%d %H:%M:%S%.3f")));
        }
        
        if let Some(end_time) = end_time {
            conditions.push(format!("timestamp <= '{}'", end_time.format("%Y-%m-%d %H:%M:%S%.3f")));
        }

        let where_clause = if conditions.is_empty() {
            String::new()
        } else {
            format!("WHERE {}", conditions.join(" AND "))
        };

        let query = format!(
            "SELECT timestamp, router_id, interface, protocol, packets_sent, packets_received, bytes_sent, bytes_received, latency_ms, packet_loss_percent, throughput_bps, jitter_ms FROM network_metrics {} ORDER BY timestamp DESC",
            where_clause
        );

        let url = format!("{}/?database={}&query={}", self.base_url, self.database, urlencoding::encode(&query));
        
        let response = self.client
            .get(&url)
            .basic_auth(&self.username, Some(&self.password))
            .send()
            .await?;

        if !response.status().is_success() {
            return Err(format!("ClickHouse query failed: {}", response.status()).into());
        }

        let text = response.text().await?;
        let mut metrics = Vec::new();
        
        for line in text.lines() {
            if line.is_empty() {
                continue;
            }
            
            let parts: Vec<&str> = line.split('\t').collect();
            if parts.len() >= 12 {
                let timestamp = chrono::DateTime::parse_from_rfc3339(&format!("{}Z", parts[0]))?;
                let router_id = parts[1].to_string();
                let interface = parts[2].to_string();
                let protocol = parts[3].to_string();
                let packets_sent = parts[4].parse()?;
                let packets_received = parts[5].parse()?;
                let bytes_sent = parts[6].parse()?;
                let bytes_received = parts[7].parse()?;
                let latency_ms = parts[8].parse()?;
                let packet_loss_percent = parts[9].parse()?;
                let throughput_bps = parts[10].parse()?;
                let jitter_ms = parts[11].parse()?;

                metrics.push(NetworkMetrics {
                    timestamp: timestamp.with_timezone(&Utc),
                    router_id,
                    interface,
                    protocol,
                    packets_sent,
                    packets_received,
                    bytes_sent,
                    bytes_received,
                    latency_ms,
                    packet_loss_percent,
                    throughput_bps,
                    jitter_ms,
                });
            }
        }

        Ok(metrics)
    }

    pub async fn get_aggregated_metrics(&self, 
        router_id: Option<&str>,
        time_window_minutes: u64
    ) -> Result<HashMap<String, f64>, Box<dyn std::error::Error>> {
        let mut conditions = Vec::new();
        
        if let Some(router_id) = router_id {
            conditions.push(format!("router_id = '{}'", router_id));
        }
        
        conditions.push(format!("timestamp >= now() - INTERVAL {} MINUTE", time_window_minutes));

        let where_clause = conditions.join(" AND ");

        let query = format!(
            "SELECT 
                avg(latency_ms) as avg_latency,
                max(latency_ms) as max_latency,
                avg(packet_loss_percent) as avg_packet_loss,
                max(packet_loss_percent) as max_packet_loss,
                avg(throughput_bps) as avg_throughput,
                max(throughput_bps) as max_throughput,
                avg(jitter_ms) as avg_jitter,
                max(jitter_ms) as max_jitter
            FROM network_metrics 
            WHERE {}",
            where_clause
        );

        let url = format!("{}/?database={}&query={}", self.base_url, self.database, urlencoding::encode(&query));
        
        let response = self.client
            .get(&url)
            .basic_auth(&self.username, Some(&self.password))
            .send()
            .await?;

        if !response.status().is_success() {
            return Err(format!("ClickHouse query failed: {}", response.status()).into());
        }

        let text = response.text().await?;
        let mut result = HashMap::new();
        
        if let Some(line) = text.lines().next() {
            let parts: Vec<&str> = line.split('\t').collect();
            if parts.len() >= 8 {
                result.insert("avg_latency".to_string(), parts[0].parse().unwrap_or(0.0));
                result.insert("max_latency".to_string(), parts[1].parse().unwrap_or(0.0));
                result.insert("avg_packet_loss".to_string(), parts[2].parse().unwrap_or(0.0));
                result.insert("max_packet_loss".to_string(), parts[3].parse().unwrap_or(0.0));
                result.insert("avg_throughput".to_string(), parts[4].parse().unwrap_or(0.0));
                result.insert("max_throughput".to_string(), parts[5].parse().unwrap_or(0.0));
                result.insert("avg_jitter".to_string(), parts[6].parse().unwrap_or(0.0));
                result.insert("max_jitter".to_string(), parts[7].parse().unwrap_or(0.0));
            }
        }

        Ok(result)
    }
}

pub struct AnalyticsEngine {
    clickhouse: Arc<ClickHouseClient>,
    metrics_buffer: Arc<RwLock<Vec<NetworkMetrics>>>,
    bgp_metrics_buffer: Arc<RwLock<Vec<BGPConvergenceMetrics>>>,
    traffic_shaping_buffer: Arc<RwLock<Vec<TrafficShapingMetrics>>>,
    impairment_buffer: Arc<RwLock<Vec<NetworkImpairmentMetrics>>>,
}

impl AnalyticsEngine {
    pub fn new(clickhouse: ClickHouseClient) -> Self {
        Self {
            clickhouse: Arc::new(clickhouse),
            metrics_buffer: Arc::new(RwLock::new(Vec::new())),
            bgp_metrics_buffer: Arc::new(RwLock::new(Vec::new())),
            traffic_shaping_buffer: Arc::new(RwLock::new(Vec::new())),
            impairment_buffer: Arc::new(RwLock::new(Vec::new())),
        }
    }

    pub async fn record_network_metrics(&self, metrics: NetworkMetrics) -> Result<(), Box<dyn std::error::Error>> {
        let mut buffer = self.metrics_buffer.write().await;
        buffer.push(metrics);
        
        // Flush buffer if it's full
        if buffer.len() >= 1000 {
            let metrics_to_flush = buffer.clone();
            buffer.clear();
            drop(buffer); // Release the lock
            
            self.clickhouse.insert_network_metrics(&metrics_to_flush).await?;
        }
        
        Ok(())
    }

    pub async fn record_bgp_metrics(&self, metrics: BGPConvergenceMetrics) -> Result<(), Box<dyn std::error::Error>> {
        let mut buffer = self.bgp_metrics_buffer.write().await;
        buffer.push(metrics);
        
        if buffer.len() >= 100 {
            let metrics_to_flush = buffer.clone();
            buffer.clear();
            drop(buffer);
            
            self.clickhouse.insert_bgp_metrics(&metrics_to_flush).await?;
        }
        
        Ok(())
    }

    pub async fn record_traffic_shaping_metrics(&self, metrics: TrafficShapingMetrics) -> Result<(), Box<dyn std::error::Error>> {
        let mut buffer = self.traffic_shaping_buffer.write().await;
        buffer.push(metrics);
        
        if buffer.len() >= 100 {
            let metrics_to_flush = buffer.clone();
            buffer.clear();
            drop(buffer);
            
            self.clickhouse.insert_traffic_shaping_metrics(&metrics_to_flush).await?;
        }
        
        Ok(())
    }

    pub async fn record_impairment_metrics(&self, metrics: NetworkImpairmentMetrics) -> Result<(), Box<dyn std::error::Error>> {
        let mut buffer = self.impairment_buffer.write().await;
        buffer.push(metrics);
        
        if buffer.len() >= 100 {
            let metrics_to_flush = buffer.clone();
            buffer.clear();
            drop(buffer);
            
            self.clickhouse.insert_impairment_metrics(&metrics_to_flush).await?;
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

        // Flush BGP metrics
        {
            let mut buffer = self.bgp_metrics_buffer.write().await;
            if !buffer.is_empty() {
                let metrics_to_flush = buffer.clone();
                buffer.clear();
                drop(buffer);
                self.clickhouse.insert_bgp_metrics(&metrics_to_flush).await?;
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

        Ok(())
    }

    pub async fn get_network_metrics(&self, 
        router_id: Option<&str>, 
        interface: Option<&str>,
        start_time: Option<DateTime<Utc>>,
        end_time: Option<DateTime<Utc>>
    ) -> Result<Vec<NetworkMetrics>, Box<dyn std::error::Error>> {
        self.clickhouse.query_network_metrics(router_id, interface, start_time, end_time).await
    }

    pub async fn get_aggregated_metrics(&self, 
        router_id: Option<&str>,
        time_window_minutes: u64
    ) -> Result<HashMap<String, f64>, Box<dyn std::error::Error>> {
        self.clickhouse.get_aggregated_metrics(router_id, time_window_minutes).await
    }
}
