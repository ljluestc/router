use serde::{Deserialize, Serialize};
use std::collections::HashMap;
use std::time::{SystemTime, UNIX_EPOCH};
use tokio::time::{Duration, Instant};
use tokio_postgres::{Client, NoTls, Row};
use std::sync::Arc;
use tokio::sync::RwLock;

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct NetworkMetric {
    pub timestamp: u64,
    pub interface: String,
    pub metric_type: String,
    pub value: f64,
    pub tags: HashMap<String, String>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct RouteMetric {
    pub timestamp: u64,
    pub destination: String,
    pub prefix_length: u8,
    pub next_hop: String,
    pub protocol: String,
    pub metric: u32,
    pub is_active: bool,
    pub interface: String,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct TrafficMetric {
    pub timestamp: u64,
    pub interface: String,
    pub direction: String, // "in" or "out"
    pub packets: u64,
    pub bytes: u64,
    pub packets_per_second: f64,
    pub bytes_per_second: f64,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ProtocolMetric {
    pub timestamp: u64,
    pub protocol: String,
    pub state: String,
    pub neighbors_count: u32,
    pub routes_count: u32,
    pub updates_sent: u64,
    pub updates_received: u64,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct SystemMetric {
    pub timestamp: u64,
    pub cpu_usage: f64,
    pub memory_usage: f64,
    pub disk_usage: f64,
    pub network_usage: f64,
    pub load_average: f64,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ImpairmentMetric {
    pub timestamp: u64,
    pub interface: String,
    pub impairment_type: String,
    pub enabled: bool,
    pub value: f64,
    pub packets_affected: u64,
}

pub struct ClickHouseAnalytics {
    client: Arc<Client>,
    metrics_buffer: Arc<RwLock<Vec<NetworkMetric>>>,
    batch_size: usize,
    flush_interval: Duration,
}

impl ClickHouseAnalytics {
    pub async fn new(connection_string: &str) -> Result<Self, Box<dyn std::error::Error>> {
        let (client, connection) = tokio_postgres::connect(connection_string, NoTls).await?;
        
        // Spawn connection task
        tokio::spawn(async move {
            if let Err(e) = connection.await {
                eprintln!("Connection error: {}", e);
            }
        });
        
        let analytics = Self {
            client: Arc::new(client),
            metrics_buffer: Arc::new(RwLock::new(Vec::new())),
            batch_size: 1000,
            flush_interval: Duration::from_secs(5),
        };
        
        // Initialize tables
        analytics.initialize_tables().await?;
        
        // Start background flush task
        analytics.start_flush_task().await;
        
        Ok(analytics)
    }
    
    async fn initialize_tables(&self) -> Result<(), Box<dyn std::error::Error>> {
        let create_tables = r#"
            CREATE TABLE IF NOT EXISTS network_metrics (
                timestamp UInt64,
                interface String,
                metric_type String,
                value Float64,
                tags Map(String, String),
                date Date MATERIALIZED toDate(timestamp / 1000)
            ) ENGINE = MergeTree()
            PARTITION BY date
            ORDER BY (interface, metric_type, timestamp);
            
            CREATE TABLE IF NOT EXISTS route_metrics (
                timestamp UInt64,
                destination String,
                prefix_length UInt8,
                next_hop String,
                protocol String,
                metric UInt32,
                is_active UInt8,
                interface String,
                date Date MATERIALIZED toDate(timestamp / 1000)
            ) ENGINE = MergeTree()
            PARTITION BY date
            ORDER BY (protocol, destination, timestamp);
            
            CREATE TABLE IF NOT EXISTS traffic_metrics (
                timestamp UInt64,
                interface String,
                direction String,
                packets UInt64,
                bytes UInt64,
                packets_per_second Float64,
                bytes_per_second Float64,
                date Date MATERIALIZED toDate(timestamp / 1000)
            ) ENGINE = MergeTree()
            PARTITION BY date
            ORDER BY (interface, direction, timestamp);
            
            CREATE TABLE IF NOT EXISTS protocol_metrics (
                timestamp UInt64,
                protocol String,
                state String,
                neighbors_count UInt32,
                routes_count UInt32,
                updates_sent UInt64,
                updates_received UInt64,
                date Date MATERIALIZED toDate(timestamp / 1000)
            ) ENGINE = MergeTree()
            PARTITION BY date
            ORDER BY (protocol, timestamp);
            
            CREATE TABLE IF NOT EXISTS system_metrics (
                timestamp UInt64,
                cpu_usage Float64,
                memory_usage Float64,
                disk_usage Float64,
                network_usage Float64,
                load_average Float64,
                date Date MATERIALIZED toDate(timestamp / 1000)
            ) ENGINE = MergeTree()
            PARTITION BY date
            ORDER BY timestamp;
            
            CREATE TABLE IF NOT EXISTS impairment_metrics (
                timestamp UInt64,
                interface String,
                impairment_type String,
                enabled UInt8,
                value Float64,
                packets_affected UInt64,
                date Date MATERIALIZED toDate(timestamp / 1000)
            ) ENGINE = MergeTree()
            PARTITION BY date
            ORDER BY (interface, impairment_type, timestamp);
        "#;
        
        self.client.execute(create_tables, &[]).await?;
        println!("ClickHouse tables initialized successfully");
        Ok(())
    }
    
    async fn start_flush_task(&self) {
        let client = self.client.clone();
        let buffer = self.metrics_buffer.clone();
        let batch_size = self.batch_size;
        let flush_interval = self.flush_interval;
        
        tokio::spawn(async move {
            let mut interval = tokio::time::interval(flush_interval);
            
            loop {
                interval.tick().await;
                
                let mut metrics = {
                    let mut buffer = buffer.write().await;
                    if buffer.len() >= batch_size {
                        let metrics = buffer.drain(..).collect::<Vec<_>>();
                        metrics
                    } else {
                        continue;
                    }
                };
                
                if !metrics.is_empty() {
                    if let Err(e) = Self::flush_metrics(&client, &metrics).await {
                        eprintln!("Failed to flush metrics: {}", e);
                        // Re-add metrics to buffer on failure
                        let mut buffer = buffer.write().await;
                        buffer.extend(metrics);
                    }
                }
            }
        });
    }
    
    async fn flush_metrics(client: &Client, metrics: &[NetworkMetric]) -> Result<(), Box<dyn std::error::Error>> {
        let mut values = Vec::new();
        
        for metric in metrics {
            let tags_json = serde_json::to_string(&metric.tags)?;
            values.push(format!(
                "({}, '{}', '{}', {}, '{}')",
                metric.timestamp,
                metric.interface,
                metric.metric_type,
                metric.value,
                tags_json
            ));
        }
        
        let query = format!(
            "INSERT INTO network_metrics (timestamp, interface, metric_type, value, tags) VALUES {}",
            values.join(", ")
        );
        
        client.execute(&query, &[]).await?;
        println!("Flushed {} network metrics to ClickHouse", metrics.len());
        Ok(())
    }
    
    pub async fn record_network_metric(&self, metric: NetworkMetric) {
        let mut buffer = self.metrics_buffer.write().await;
        buffer.push(metric);
        
        // Flush immediately if buffer is full
        if buffer.len() >= self.batch_size {
            let metrics = buffer.drain(..).collect::<Vec<_>>();
            drop(buffer); // Release the lock
            
            if let Err(e) = Self::flush_metrics(&self.client, &metrics).await {
                eprintln!("Failed to flush metrics: {}", e);
                // Re-add metrics to buffer on failure
                let mut buffer = self.metrics_buffer.write().await;
                buffer.extend(metrics);
            }
        }
    }
    
    pub async fn record_route_metric(&self, metric: RouteMetric) -> Result<(), Box<dyn std::error::Error>> {
        let query = r#"
            INSERT INTO route_metrics 
            (timestamp, destination, prefix_length, next_hop, protocol, metric, is_active, interface)
            VALUES ($1, $2, $3, $4, $5, $6, $7, $8)
        "#;
        
        self.client.execute(
            query,
            &[
                &(metric.timestamp as i64),
                &metric.destination,
                &(metric.prefix_length as i16),
                &metric.next_hop,
                &metric.protocol,
                &(metric.metric as i32),
                &(metric.is_active as i16),
                &metric.interface,
            ],
        ).await?;
        
        Ok(())
    }
    
    pub async fn record_traffic_metric(&self, metric: TrafficMetric) -> Result<(), Box<dyn std::error::Error>> {
        let query = r#"
            INSERT INTO traffic_metrics 
            (timestamp, interface, direction, packets, bytes, packets_per_second, bytes_per_second)
            VALUES ($1, $2, $3, $4, $5, $6, $7)
        "#;
        
        self.client.execute(
            query,
            &[
                &(metric.timestamp as i64),
                &metric.interface,
                &metric.direction,
                &(metric.packets as i64),
                &(metric.bytes as i64),
                &metric.packets_per_second,
                &metric.bytes_per_second,
            ],
        ).await?;
        
        Ok(())
    }
    
    pub async fn record_protocol_metric(&self, metric: ProtocolMetric) -> Result<(), Box<dyn std::error::Error>> {
        let query = r#"
            INSERT INTO protocol_metrics 
            (timestamp, protocol, state, neighbors_count, routes_count, updates_sent, updates_received)
            VALUES ($1, $2, $3, $4, $5, $6, $7)
        "#;
        
        self.client.execute(
            query,
            &[
                &(metric.timestamp as i64),
                &metric.protocol,
                &metric.state,
                &(metric.neighbors_count as i32),
                &(metric.routes_count as i32),
                &(metric.updates_sent as i64),
                &(metric.updates_received as i64),
            ],
        ).await?;
        
        Ok(())
    }
    
    pub async fn record_system_metric(&self, metric: SystemMetric) -> Result<(), Box<dyn std::error::Error>> {
        let query = r#"
            INSERT INTO system_metrics 
            (timestamp, cpu_usage, memory_usage, disk_usage, network_usage, load_average)
            VALUES ($1, $2, $3, $4, $5, $6)
        "#;
        
        self.client.execute(
            query,
            &[
                &(metric.timestamp as i64),
                &metric.cpu_usage,
                &metric.memory_usage,
                &metric.disk_usage,
                &metric.network_usage,
                &metric.load_average,
            ],
        ).await?;
        
        Ok(())
    }
    
    pub async fn record_impairment_metric(&self, metric: ImpairmentMetric) -> Result<(), Box<dyn std::error::Error>> {
        let query = r#"
            INSERT INTO impairment_metrics 
            (timestamp, interface, impairment_type, enabled, value, packets_affected)
            VALUES ($1, $2, $3, $4, $5, $6)
        "#;
        
        self.client.execute(
            query,
            &[
                &(metric.timestamp as i64),
                &metric.interface,
                &metric.impairment_type,
                &(metric.enabled as i16),
                &metric.value,
                &(metric.packets_affected as i64),
            ],
        ).await?;
        
        Ok(())
    }
    
    pub async fn get_network_metrics(
        &self,
        interface: Option<&str>,
        metric_type: Option<&str>,
        start_time: Option<u64>,
        end_time: Option<u64>,
        limit: Option<usize>,
    ) -> Result<Vec<NetworkMetric>, Box<dyn std::error::Error>> {
        let mut query = "SELECT timestamp, interface, metric_type, value, tags FROM network_metrics WHERE 1=1".to_string();
        let mut params: Vec<Box<dyn tokio_postgres::types::ToSql + Sync>> = Vec::new();
        let mut param_count = 0;
        
        if let Some(iface) = interface {
            param_count += 1;
            query.push_str(&format!(" AND interface = ${}", param_count));
            params.push(Box::new(iface.to_string()));
        }
        
        if let Some(mtype) = metric_type {
            param_count += 1;
            query.push_str(&format!(" AND metric_type = ${}", param_count));
            params.push(Box::new(mtype.to_string()));
        }
        
        if let Some(start) = start_time {
            param_count += 1;
            query.push_str(&format!(" AND timestamp >= ${}", param_count));
            params.push(Box::new(start as i64));
        }
        
        if let Some(end) = end_time {
            param_count += 1;
            query.push_str(&format!(" AND timestamp <= ${}", param_count));
            params.push(Box::new(end as i64));
        }
        
        query.push_str(" ORDER BY timestamp DESC");
        
        if let Some(limit) = limit {
            query.push_str(&format!(" LIMIT {}", limit));
        }
        
        let rows = self.client.query(&query, &params).await?;
        let mut metrics = Vec::new();
        
        for row in rows {
            let timestamp: i64 = row.get(0);
            let interface: String = row.get(1);
            let metric_type: String = row.get(2);
            let value: f64 = row.get(3);
            let tags_json: String = row.get(4);
            let tags: HashMap<String, String> = serde_json::from_str(&tags_json)?;
            
            metrics.push(NetworkMetric {
                timestamp: timestamp as u64,
                interface,
                metric_type,
                value,
                tags,
            });
        }
        
        Ok(metrics)
    }
    
    pub async fn get_traffic_summary(
        &self,
        interface: Option<&str>,
        duration_hours: u32,
    ) -> Result<HashMap<String, f64>, Box<dyn std::error::Error>> {
        let end_time = SystemTime::now()
            .duration_since(UNIX_EPOCH)?
            .as_secs() * 1000;
        let start_time = end_time - (duration_hours as u64 * 3600 * 1000);
        
        let mut query = r#"
            SELECT 
                interface,
                direction,
                SUM(packets) as total_packets,
                SUM(bytes) as total_bytes,
                AVG(packets_per_second) as avg_packets_per_second,
                AVG(bytes_per_second) as avg_bytes_per_second
            FROM traffic_metrics 
            WHERE timestamp >= $1 AND timestamp <= $2
        "#.to_string();
        
        let mut params: Vec<Box<dyn tokio_postgres::types::ToSql + Sync>> = Vec::new();
        params.push(Box::new(start_time as i64));
        params.push(Box::new(end_time as i64));
        
        if let Some(iface) = interface {
            query.push_str(" AND interface = $3");
            params.push(Box::new(iface.to_string()));
        }
        
        query.push_str(" GROUP BY interface, direction ORDER BY interface, direction");
        
        let rows = self.client.query(&query, &params).await?;
        let mut summary = HashMap::new();
        
        for row in rows {
            let interface: String = row.get(0);
            let direction: String = row.get(1);
            let total_packets: i64 = row.get(2);
            let total_bytes: i64 = row.get(3);
            let avg_packets_per_second: f64 = row.get(4);
            let avg_bytes_per_second: f64 = row.get(5);
            
            summary.insert(format!("{}_{}_packets", interface, direction), total_packets as f64);
            summary.insert(format!("{}_{}_bytes", interface, direction), total_bytes as f64);
            summary.insert(format!("{}_{}_avg_packets_per_second", interface, direction), avg_packets_per_second);
            summary.insert(format!("{}_{}_avg_bytes_per_second", interface, direction), avg_bytes_per_second);
        }
        
        Ok(summary)
    }
    
    pub async fn get_protocol_statistics(
        &self,
        protocol: Option<&str>,
        duration_hours: u32,
    ) -> Result<Vec<ProtocolMetric>, Box<dyn std::error::Error>> {
        let end_time = SystemTime::now()
            .duration_since(UNIX_EPOCH)?
            .as_secs() * 1000;
        let start_time = end_time - (duration_hours as u64 * 3600 * 1000);
        
        let mut query = r#"
            SELECT 
                timestamp,
                protocol,
                state,
                neighbors_count,
                routes_count,
                updates_sent,
                updates_received
            FROM protocol_metrics 
            WHERE timestamp >= $1 AND timestamp <= $2
        "#.to_string();
        
        let mut params: Vec<Box<dyn tokio_postgres::types::ToSql + Sync>> = Vec::new();
        params.push(Box::new(start_time as i64));
        params.push(Box::new(end_time as i64));
        
        if let Some(proto) = protocol {
            query.push_str(" AND protocol = $3");
            params.push(Box::new(proto.to_string()));
        }
        
        query.push_str(" ORDER BY timestamp DESC");
        
        let rows = self.client.query(&query, &params).await?;
        let mut metrics = Vec::new();
        
        for row in rows {
            let timestamp: i64 = row.get(0);
            let protocol: String = row.get(1);
            let state: String = row.get(2);
            let neighbors_count: i32 = row.get(3);
            let routes_count: i32 = row.get(4);
            let updates_sent: i64 = row.get(5);
            let updates_received: i64 = row.get(6);
            
            metrics.push(ProtocolMetric {
                timestamp: timestamp as u64,
                protocol,
                state,
                neighbors_count: neighbors_count as u32,
                routes_count: routes_count as u32,
                updates_sent: updates_sent as u64,
                updates_received: updates_received as u64,
            });
        }
        
        Ok(metrics)
    }
    
    pub async fn get_system_metrics(
        &self,
        duration_hours: u32,
    ) -> Result<Vec<SystemMetric>, Box<dyn std::error::Error>> {
        let end_time = SystemTime::now()
            .duration_since(UNIX_EPOCH)?
            .as_secs() * 1000;
        let start_time = end_time - (duration_hours as u64 * 3600 * 1000);
        
        let query = r#"
            SELECT 
                timestamp,
                cpu_usage,
                memory_usage,
                disk_usage,
                network_usage,
                load_average
            FROM system_metrics 
            WHERE timestamp >= $1 AND timestamp <= $2
            ORDER BY timestamp DESC
        "#;
        
        let rows = self.client.query(
            query,
            &[&(start_time as i64), &(end_time as i64)],
        ).await?;
        
        let mut metrics = Vec::new();
        
        for row in rows {
            let timestamp: i64 = row.get(0);
            let cpu_usage: f64 = row.get(1);
            let memory_usage: f64 = row.get(2);
            let disk_usage: f64 = row.get(3);
            let network_usage: f64 = row.get(4);
            let load_average: f64 = row.get(5);
            
            metrics.push(SystemMetric {
                timestamp: timestamp as u64,
                cpu_usage,
                memory_usage,
                disk_usage,
                network_usage,
                load_average,
            });
        }
        
        Ok(metrics)
    }
    
    pub async fn flush_all_metrics(&self) -> Result<(), Box<dyn std::error::Error>> {
        let metrics = {
            let mut buffer = self.metrics_buffer.write().await;
            buffer.drain(..).collect::<Vec<_>>()
        };
        
        if !metrics.is_empty() {
            Self::flush_metrics(&self.client, &metrics).await?;
        }
        
        Ok(())
    }
}

// Analytics service for real-time metrics collection
pub struct AnalyticsService {
    clickhouse: Arc<ClickHouseAnalytics>,
    running: Arc<RwLock<bool>>,
}

impl AnalyticsService {
    pub async fn new(clickhouse: ClickHouseAnalytics) -> Self {
        Self {
            clickhouse: Arc::new(clickhouse),
            running: Arc::new(RwLock::new(false)),
        }
    }
    
    pub async fn start(&self) -> Result<(), Box<dyn std::error::Error>> {
        let mut running = self.running.write().await;
        if *running {
            return Ok(());
        }
        
        *running = true;
        drop(running);
        
        // Start background metric collection tasks
        self.start_network_metrics_collection().await;
        self.start_system_metrics_collection().await;
        
        println!("Analytics service started");
        Ok(())
    }
    
    pub async fn stop(&self) {
        let mut running = self.running.write().await;
        *running = false;
    }
    
    async fn start_network_metrics_collection(&self) {
        let clickhouse = self.clickhouse.clone();
        let running = self.running.clone();
        
        tokio::spawn(async move {
            let mut interval = tokio::time::interval(Duration::from_secs(1));
            
            while *running.read().await {
                interval.tick().await;
                
                // Collect network interface statistics
                if let Ok(interfaces) = Self::get_network_interfaces().await {
                    for interface in interfaces {
                        if let Ok(stats) = Self::get_interface_stats(&interface).await {
                            let timestamp = SystemTime::now()
                                .duration_since(UNIX_EPOCH)
                                .unwrap()
                                .as_secs() * 1000;
                            
                            // Record RX metrics
                            let mut tags = HashMap::new();
                            tags.insert("direction".to_string(), "rx".to_string());
                            
                            let rx_metric = NetworkMetric {
                                timestamp,
                                interface: interface.clone(),
                                metric_type: "packets".to_string(),
                                value: stats.rx_packets as f64,
                                tags: tags.clone(),
                            };
                            clickhouse.record_network_metric(rx_metric).await;
                            
                            tags.insert("direction".to_string(), "rx".to_string());
                            let rx_bytes_metric = NetworkMetric {
                                timestamp,
                                interface: interface.clone(),
                                metric_type: "bytes".to_string(),
                                value: stats.rx_bytes as f64,
                                tags,
                            };
                            clickhouse.record_network_metric(rx_bytes_metric).await;
                            
                            // Record TX metrics
                            let mut tags = HashMap::new();
                            tags.insert("direction".to_string(), "tx".to_string());
                            
                            let tx_metric = NetworkMetric {
                                timestamp,
                                interface: interface.clone(),
                                metric_type: "packets".to_string(),
                                value: stats.tx_packets as f64,
                                tags: tags.clone(),
                            };
                            clickhouse.record_network_metric(tx_metric).await;
                            
                            tags.insert("direction".to_string(), "tx".to_string());
                            let tx_bytes_metric = NetworkMetric {
                                timestamp,
                                interface: interface.clone(),
                                metric_type: "bytes".to_string(),
                                value: stats.tx_bytes as f64,
                                tags,
                            };
                            clickhouse.record_network_metric(tx_bytes_metric).await;
                        }
                    }
                }
            }
        });
    }
    
    async fn start_system_metrics_collection(&self) {
        let clickhouse = self.clickhouse.clone();
        let running = self.running.clone();
        
        tokio::spawn(async move {
            let mut interval = tokio::time::interval(Duration::from_secs(5));
            
            while *running.read().await {
                interval.tick().await;
                
                if let Ok(system_stats) = Self::get_system_stats().await {
                    let timestamp = SystemTime::now()
                        .duration_since(UNIX_EPOCH)
                        .unwrap()
                        .as_secs() * 1000;
                    
                    let metric = SystemMetric {
                        timestamp,
                        cpu_usage: system_stats.cpu_usage,
                        memory_usage: system_stats.memory_usage,
                        disk_usage: system_stats.disk_usage,
                        network_usage: system_stats.network_usage,
                        load_average: system_stats.load_average,
                    };
                    
                    if let Err(e) = clickhouse.record_system_metric(metric).await {
                        eprintln!("Failed to record system metric: {}", e);
                    }
                }
            }
        });
    }
    
    async fn get_network_interfaces() -> Result<Vec<String>, Box<dyn std::error::Error>> {
        use std::fs;
        
        let interfaces_dir = "/sys/class/net";
        let mut interfaces = Vec::new();
        
        for entry in fs::read_dir(interfaces_dir)? {
            let entry = entry?;
            let path = entry.path();
            
            if path.is_dir() {
                if let Some(name) = path.file_name() {
                    if let Some(name_str) = name.to_str() {
                        // Skip loopback and virtual interfaces for now
                        if !name_str.starts_with("lo") && !name_str.starts_with("veth") {
                            interfaces.push(name_str.to_string());
                        }
                    }
                }
            }
        }
        
        Ok(interfaces)
    }
    
    async fn get_interface_stats(interface: &str) -> Result<InterfaceStats, Box<dyn std::error::Error>> {
        use std::fs;
        
        let rx_packets = fs::read_to_string(format!("/sys/class/net/{}/statistics/rx_packets", interface))?
            .trim().parse::<u64>()?;
        let tx_packets = fs::read_to_string(format!("/sys/class/net/{}/statistics/tx_packets", interface))?
            .trim().parse::<u64>()?;
        let rx_bytes = fs::read_to_string(format!("/sys/class/net/{}/statistics/rx_bytes", interface))?
            .trim().parse::<u64>()?;
        let tx_bytes = fs::read_to_string(format!("/sys/class/net/{}/statistics/tx_bytes", interface))?
            .trim().parse::<u64>()?;
        
        Ok(InterfaceStats {
            rx_packets,
            tx_packets,
            rx_bytes,
            tx_bytes,
        })
    }
    
    async fn get_system_stats() -> Result<SystemStats, Box<dyn std::error::Error>> {
        use std::fs;
        
        // Read CPU usage from /proc/stat
        let cpu_info = fs::read_to_string("/proc/stat")?;
        let cpu_line = cpu_info.lines().next().unwrap();
        let cpu_values: Vec<&str> = cpu_line.split_whitespace().collect();
        
        let user: u64 = cpu_values[1].parse()?;
        let nice: u64 = cpu_values[2].parse()?;
        let system: u64 = cpu_values[3].parse()?;
        let idle: u64 = cpu_values[4].parse()?;
        let iowait: u64 = cpu_values[5].parse()?;
        let irq: u64 = cpu_values[6].parse()?;
        let softirq: u64 = cpu_values[7].parse()?;
        
        let total = user + nice + system + idle + iowait + irq + softirq;
        let idle_total = idle + iowait;
        let cpu_usage = if total > 0 {
            ((total - idle_total) as f64 / total as f64) * 100.0
        } else {
            0.0
        };
        
        // Read memory usage from /proc/meminfo
        let mem_info = fs::read_to_string("/proc/meminfo")?;
        let mut total_mem = 0;
        let mut available_mem = 0;
        
        for line in mem_info.lines() {
            if line.starts_with("MemTotal:") {
                let parts: Vec<&str> = line.split_whitespace().collect();
                total_mem = parts[1].parse::<u64>()?;
            } else if line.starts_with("MemAvailable:") {
                let parts: Vec<&str> = line.split_whitespace().collect();
                available_mem = parts[1].parse::<u64>()?;
            }
        }
        
        let memory_usage = if total_mem > 0 {
            ((total_mem - available_mem) as f64 / total_mem as f64) * 100.0
        } else {
            0.0
        };
        
        // Read load average from /proc/loadavg
        let load_avg = fs::read_to_string("/proc/loadavg")?;
        let load_values: Vec<&str> = load_avg.split_whitespace().collect();
        let load_average = load_values[0].parse::<f64>()?;
        
        Ok(SystemStats {
            cpu_usage,
            memory_usage,
            disk_usage: 0.0, // TODO: Implement disk usage
            network_usage: 0.0, // TODO: Implement network usage
            load_average,
        })
    }
}

#[derive(Debug)]
struct InterfaceStats {
    rx_packets: u64,
    tx_packets: u64,
    rx_bytes: u64,
    tx_bytes: u64,
}

#[derive(Debug)]
struct SystemStats {
    cpu_usage: f64,
    memory_usage: f64,
    disk_usage: f64,
    network_usage: f64,
    load_average: f64,
}
