use serde::{Deserialize, Serialize};
use std::collections::HashMap;
use std::sync::Arc;
use tokio::sync::RwLock;
use chrono::{DateTime, Utc};

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct NetworkMetric {
    pub timestamp: DateTime<Utc>,
    pub node_id: String,
    pub metric_type: MetricType,
    pub value: f64,
    pub tags: HashMap<String, String>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum MetricType {
    CpuUsage,
    MemoryUsage,
    NetworkIn,
    NetworkOut,
    PacketLoss,
    Latency,
    Throughput,
    ConnectionCount,
    ErrorRate,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct NetworkEvent {
    pub timestamp: DateTime<Utc>,
    pub event_type: EventType,
    pub node_id: String,
    pub description: String,
    pub severity: Severity,
    pub metadata: HashMap<String, String>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum EventType {
    GatewayConnected,
    GatewayDisconnected,
    HighLatency,
    PacketLoss,
    ConnectionError,
    ConfigurationChange,
    Maintenance,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum Severity {
    Info,
    Warning,
    Error,
    Critical,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct AnalyticsQuery {
    pub start_time: DateTime<Utc>,
    pub end_time: DateTime<Utc>,
    pub node_ids: Option<Vec<String>>,
    pub metric_types: Option<Vec<MetricType>>,
    pub aggregation: AggregationType,
    pub group_by: Option<Vec<String>>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum AggregationType {
    Sum,
    Average,
    Min,
    Max,
    Count,
    Percentile(f64),
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct AnalyticsResult {
    pub data: Vec<DataPoint>,
    pub total_count: u64,
    pub query_time_ms: u64,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct DataPoint {
    pub timestamp: DateTime<Utc>,
    pub node_id: String,
    pub metric_type: MetricType,
    pub value: f64,
    pub tags: HashMap<String, String>,
}

pub struct ClickHouseAnalytics {
    metrics: Arc<RwLock<Vec<NetworkMetric>>>,
    events: Arc<RwLock<Vec<NetworkEvent>>>,
    config: ClickHouseConfig,
}

#[derive(Debug, Clone)]
pub struct ClickHouseConfig {
    pub host: String,
    pub port: u16,
    pub database: String,
    pub username: String,
    pub password: String,
    pub batch_size: usize,
    pub flush_interval_ms: u64,
}

impl ClickHouseAnalytics {
    pub fn new(config: ClickHouseConfig) -> Self {
        Self {
            metrics: Arc::new(RwLock::new(Vec::new())),
            events: Arc::new(RwLock::new(Vec::new())),
            config,
        }
    }

    pub async fn insert_metric(&self, metric: NetworkMetric) -> Result<(), String> {
        let mut metrics = self.metrics.write().await;
        metrics.push(metric);
        
        // Batch insert when we reach the batch size
        if metrics.len() >= self.config.batch_size {
            self.flush_metrics().await?;
        }
        
        Ok(())
    }

    pub async fn insert_event(&self, event: NetworkEvent) -> Result<(), String> {
        let mut events = self.events.write().await;
        events.push(event);
        Ok(())
    }

    pub async fn query_metrics(&self, query: AnalyticsQuery) -> Result<AnalyticsResult, String> {
        let metrics = self.metrics.read().await;
        let start_time = query.start_time;
        let end_time = query.end_time;
        
        let filtered_metrics: Vec<&NetworkMetric> = metrics
            .iter()
            .filter(|m| {
                m.timestamp >= start_time && m.timestamp <= end_time
            })
            .filter(|m| {
                if let Some(ref node_ids) = query.node_ids {
                    node_ids.contains(&m.node_id)
                } else {
                    true
                }
            })
            .filter(|m| {
                if let Some(ref metric_types) = query.metric_types {
                    metric_types.contains(&m.metric_type)
                } else {
                    true
                }
            })
            .collect();

        let data_points: Vec<DataPoint> = filtered_metrics
            .iter()
            .map(|m| DataPoint {
                timestamp: m.timestamp,
                node_id: m.node_id.clone(),
                metric_type: m.metric_type.clone(),
                value: m.value,
                tags: m.tags.clone(),
            })
            .collect();

        Ok(AnalyticsResult {
            data: data_points,
            total_count: filtered_metrics.len() as u64,
            query_time_ms: 0, // TODO: Implement actual timing
        })
    }

    pub async fn get_node_metrics(&self, node_id: &str, metric_type: MetricType) -> Result<Vec<DataPoint>, String> {
        let metrics = self.metrics.read().await;
        let filtered_metrics: Vec<&NetworkMetric> = metrics
            .iter()
            .filter(|m| m.node_id == node_id && std::mem::discriminant(&m.metric_type) == std::mem::discriminant(&metric_type))
            .collect();

        let data_points: Vec<DataPoint> = filtered_metrics
            .iter()
            .map(|m| DataPoint {
                timestamp: m.timestamp,
                node_id: m.node_id.clone(),
                metric_type: m.metric_type.clone(),
                value: m.value,
                tags: m.tags.clone(),
            })
            .collect();

        Ok(data_points)
    }

    pub async fn get_network_health(&self) -> Result<NetworkHealth, String> {
        let metrics = self.metrics.read().await;
        let events = self.events.read().await;
        
        let now = Utc::now();
        let one_hour_ago = now - chrono::Duration::hours(1);
        
        let recent_metrics: Vec<&NetworkMetric> = metrics
            .iter()
            .filter(|m| m.timestamp >= one_hour_ago)
            .collect();

        let recent_events: Vec<&NetworkEvent> = events
            .iter()
            .filter(|e| e.timestamp >= one_hour_ago)
            .collect();

        let mut health = NetworkHealth {
            overall_score: 100.0,
            latency_ms: 0.0,
            packet_loss_percent: 0.0,
            throughput_mbps: 0.0,
            error_count: 0,
            warning_count: 0,
            active_nodes: 0,
            total_nodes: 0,
        };

        // Calculate latency
        let latency_metrics: Vec<f64> = recent_metrics
            .iter()
            .filter(|m| matches!(m.metric_type, MetricType::Latency))
            .map(|m| m.value)
            .collect();
        
        if !latency_metrics.is_empty() {
            health.latency_ms = latency_metrics.iter().sum::<f64>() / latency_metrics.len() as f64;
        }

        // Calculate packet loss
        let packet_loss_metrics: Vec<f64> = recent_metrics
            .iter()
            .filter(|m| matches!(m.metric_type, MetricType::PacketLoss))
            .map(|m| m.value)
            .collect();
        
        if !packet_loss_metrics.is_empty() {
            health.packet_loss_percent = packet_loss_metrics.iter().sum::<f64>() / packet_loss_metrics.len() as f64;
        }

        // Calculate throughput
        let throughput_metrics: Vec<f64> = recent_metrics
            .iter()
            .filter(|m| matches!(m.metric_type, MetricType::Throughput))
            .map(|m| m.value)
            .collect();
        
        if !throughput_metrics.is_empty() {
            health.throughput_mbps = throughput_metrics.iter().sum::<f64>() / throughput_metrics.len() as f64;
        }

        // Count events
        for event in recent_events {
            match event.severity {
                Severity::Error | Severity::Critical => health.error_count += 1,
                Severity::Warning => health.warning_count += 1,
                _ => {}
            }
        }

        // Calculate overall health score
        let mut score = 100.0;
        
        // Penalize high latency
        if health.latency_ms > 100.0 {
            score -= (health.latency_ms - 100.0) * 0.1;
        }
        
        // Penalize packet loss
        score -= health.packet_loss_percent * 10.0;
        
        // Penalize errors
        score -= health.error_count as f64 * 5.0;
        score -= health.warning_count as f64 * 2.0;
        
        health.overall_score = score.max(0.0);

        Ok(health)
    }

    pub async fn flush_metrics(&self) -> Result<(), String> {
        let mut metrics = self.metrics.write().await;
        if metrics.is_empty() {
            return Ok(());
        }

        // TODO: Implement actual ClickHouse insertion
        // For now, just clear the metrics
        metrics.clear();
        
        Ok(())
    }

    pub async fn create_tables(&self) -> Result<(), String> {
        // TODO: Implement ClickHouse table creation
        Ok(())
    }

    pub async fn optimize_tables(&self) -> Result<(), String> {
        // TODO: Implement ClickHouse table optimization
        Ok(())
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct NetworkHealth {
    pub overall_score: f64,
    pub latency_ms: f64,
    pub packet_loss_percent: f64,
    pub throughput_mbps: f64,
    pub error_count: u32,
    pub warning_count: u32,
    pub active_nodes: u32,
    pub total_nodes: u32,
}

impl NetworkHealth {
    pub fn get_health_status(&self) -> &'static str {
        if self.overall_score >= 90.0 {
            "excellent"
        } else if self.overall_score >= 80.0 {
            "good"
        } else if self.overall_score >= 70.0 {
            "fair"
        } else if self.overall_score >= 50.0 {
            "poor"
        } else {
            "critical"
        }
    }
}

// Real-time analytics processor
pub struct RealtimeAnalytics {
    analytics: Arc<ClickHouseAnalytics>,
    processors: Vec<Box<dyn MetricProcessor + Send + Sync>>,
}

pub trait MetricProcessor {
    fn process(&self, metric: &NetworkMetric) -> Result<(), String>;
    fn get_name(&self) -> &str;
}

pub struct LatencyProcessor;
pub struct ThroughputProcessor;
pub struct ErrorProcessor;

impl MetricProcessor for LatencyProcessor {
    fn process(&self, metric: &NetworkMetric) -> Result<(), String> {
        if matches!(metric.metric_type, MetricType::Latency) && metric.value > 100.0 {
            // Generate high latency alert
            println!("High latency detected: {}ms for node {}", metric.value, metric.node_id);
        }
        Ok(())
    }

    fn get_name(&self) -> &str {
        "latency_processor"
    }
}

impl MetricProcessor for ThroughputProcessor {
    fn process(&self, metric: &NetworkMetric) -> Result<(), String> {
        if matches!(metric.metric_type, MetricType::Throughput) && metric.value < 10.0 {
            // Generate low throughput alert
            println!("Low throughput detected: {}Mbps for node {}", metric.value, metric.node_id);
        }
        Ok(())
    }

    fn get_name(&self) -> &str {
        "throughput_processor"
    }
}

impl MetricProcessor for ErrorProcessor {
    fn process(&self, metric: &NetworkMetric) -> Result<(), String> {
        if matches!(metric.metric_type, MetricType::ErrorRate) && metric.value > 5.0 {
            // Generate high error rate alert
            println!("High error rate detected: {}% for node {}", metric.value, metric.node_id);
        }
        Ok(())
    }

    fn get_name(&self) -> &str {
        "error_processor"
    }
}

impl RealtimeAnalytics {
    pub fn new(analytics: Arc<ClickHouseAnalytics>) -> Self {
        Self {
            analytics,
            processors: Vec::new(),
        }
    }

    pub fn add_processor(&mut self, processor: Box<dyn MetricProcessor + Send + Sync>) {
        self.processors.push(processor);
    }

    pub async fn process_metric(&self, metric: NetworkMetric) -> Result<(), String> {
        // Insert into analytics
        self.analytics.insert_metric(metric.clone()).await?;

        // Process with real-time processors
        for processor in &self.processors {
            if let Err(e) = processor.process(&metric) {
                eprintln!("Error in processor {}: {}", processor.get_name(), e);
            }
        }

        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use chrono::Utc;

    #[tokio::test]
    async fn test_insert_and_query_metrics() {
        let config = ClickHouseConfig {
            host: "localhost".to_string(),
            port: 9000,
            database: "router_analytics".to_string(),
            username: "default".to_string(),
            password: "".to_string(),
            batch_size: 100,
            flush_interval_ms: 1000,
        };

        let analytics = ClickHouseAnalytics::new(config);
        
        let metric = NetworkMetric {
            timestamp: Utc::now(),
            node_id: "test-node".to_string(),
            metric_type: MetricType::CpuUsage,
            value: 75.0,
            tags: HashMap::new(),
        };

        analytics.insert_metric(metric).await.unwrap();

        let query = AnalyticsQuery {
            start_time: Utc::now() - chrono::Duration::hours(1),
            end_time: Utc::now(),
            node_ids: Some(vec!["test-node".to_string()]),
            metric_types: Some(vec![MetricType::CpuUsage]),
            aggregation: AggregationType::Average,
            group_by: None,
        };

        let result = analytics.query_metrics(query).await.unwrap();
        assert_eq!(result.data.len(), 1);
        assert_eq!(result.data[0].value, 75.0);
    }
}
