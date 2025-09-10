use std::collections::HashMap;
use std::sync::Arc;
use std::time::{SystemTime, UNIX_EPOCH};
use tokio::sync::RwLock;
use serde::{Deserialize, Serialize};
use clickhouse_rs::{Pool, Block, types::Complex};

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct NetworkMetric {
    pub timestamp: u64,
    pub router_id: String,
    pub interface: String,
    pub metric_type: String,
    pub value: f64,
    pub tags: HashMap<String, String>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct PacketFlow {
    pub timestamp: u64,
    pub src_ip: String,
    pub dst_ip: String,
    pub src_port: u16,
    pub dst_port: u16,
    pub protocol: u8,
    pub bytes: u64,
    pub packets: u64,
    pub duration_ms: u64,
    pub router_id: String,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct BGPUpdate {
    pub timestamp: u64,
    pub router_id: String,
    pub neighbor_ip: String,
    pub prefix: String,
    pub prefix_length: u8,
    pub as_path: Vec<u32>,
    pub next_hop: String,
    pub origin: String,
    pub local_pref: u32,
    pub med: u32,
    pub communities: Vec<String>,
    pub action: String, // "advertise" or "withdraw"
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct OSPFUpdate {
    pub timestamp: u64,
    pub router_id: String,
    pub area_id: String,
    pub lsa_type: u8,
    pub lsa_id: String,
    pub advertising_router: String,
    pub sequence_number: u32,
    pub age: u16,
    pub checksum: u16,
    pub length: u16,
    pub action: String, // "advertise" or "withdraw"
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ISISUpdate {
    pub timestamp: u64,
    pub system_id: String,
    pub area_id: String,
    pub level: u8,
    pub lsp_id: String,
    pub sequence_number: u32,
    pub remaining_lifetime: u16,
    pub checksum: u16,
    pub pdu_length: u16,
    pub action: String, // "advertise" or "withdraw"
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct TrafficShapingMetric {
    pub timestamp: u64,
    pub router_id: String,
    pub interface: String,
    pub algorithm: String,
    pub class_id: u8,
    pub packets_processed: u64,
    pub packets_dropped: u64,
    pub bytes_processed: u64,
    pub bytes_dropped: u64,
    pub queue_length: u32,
    pub throughput_bps: f64,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct NetemImpairment {
    pub timestamp: u64,
    pub router_id: String,
    pub interface: String,
    pub impairment_type: String,
    pub parameters: HashMap<String, String>,
    pub active: bool,
}

pub struct ClickHouseAnalytics {
    pool: Pool,
    metrics_buffer: Arc<RwLock<Vec<NetworkMetric>>>,
    packet_flows_buffer: Arc<RwLock<Vec<PacketFlow>>>,
    bgp_updates_buffer: Arc<RwLock<Vec<BGPUpdate>>>,
    ospf_updates_buffer: Arc<RwLock<Vec<OSPFUpdate>>>,
    isis_updates_buffer: Arc<RwLock<Vec<ISISUpdate>>>,
    traffic_shaping_buffer: Arc<RwLock<Vec<TrafficShapingMetric>>>,
    netem_impairments_buffer: Arc<RwLock<Vec<NetemImpairment>>>,
}

impl ClickHouseAnalytics {
    pub async fn new(connection_string: &str) -> Result<Self, Box<dyn std::error::Error>> {
        let pool = Pool::new(connection_string)?;
        
        // Create tables if they don't exist
        let mut client = pool.get_handle().await?;
        
        // Create metrics table
        client.execute("
            CREATE TABLE IF NOT EXISTS network_metrics (
                timestamp UInt64,
                router_id String,
                interface String,
                metric_type String,
                value Float64,
                tags Map(String, String),
                date Date MATERIALIZED toDate(timestamp / 1000)
            ) ENGINE = MergeTree()
            PARTITION BY date
            ORDER BY (router_id, interface, metric_type, timestamp)
        ").await?;

        // Create packet flows table
        client.execute("
            CREATE TABLE IF NOT EXISTS packet_flows (
                timestamp UInt64,
                src_ip String,
                dst_ip String,
                src_port UInt16,
                dst_port UInt16,
                protocol UInt8,
                bytes UInt64,
                packets UInt64,
                duration_ms UInt64,
                router_id String,
                date Date MATERIALIZED toDate(timestamp / 1000)
            ) ENGINE = MergeTree()
            PARTITION BY date
            ORDER BY (router_id, src_ip, dst_ip, timestamp)
        ").await?;

        // Create BGP updates table
        client.execute("
            CREATE TABLE IF NOT EXISTS bgp_updates (
                timestamp UInt64,
                router_id String,
                neighbor_ip String,
                prefix String,
                prefix_length UInt8,
                as_path Array(UInt32),
                next_hop String,
                origin String,
                local_pref UInt32,
                med UInt32,
                communities Array(String),
                action String,
                date Date MATERIALIZED toDate(timestamp / 1000)
            ) ENGINE = MergeTree()
            PARTITION BY date
            ORDER BY (router_id, neighbor_ip, prefix, timestamp)
        ").await?;

        // Create OSPF updates table
        client.execute("
            CREATE TABLE IF NOT EXISTS ospf_updates (
                timestamp UInt64,
                router_id String,
                area_id String,
                lsa_type UInt8,
                lsa_id String,
                advertising_router String,
                sequence_number UInt32,
                age UInt16,
                checksum UInt16,
                length UInt16,
                action String,
                date Date MATERIALIZED toDate(timestamp / 1000)
            ) ENGINE = MergeTree()
            PARTITION BY date
            ORDER BY (router_id, area_id, lsa_type, timestamp)
        ").await?;

        // Create IS-IS updates table
        client.execute("
            CREATE TABLE IF NOT EXISTS isis_updates (
                timestamp UInt64,
                system_id String,
                area_id String,
                level UInt8,
                lsp_id String,
                sequence_number UInt32,
                remaining_lifetime UInt16,
                checksum UInt16,
                pdu_length UInt16,
                action String,
                date Date MATERIALIZED toDate(timestamp / 1000)
            ) ENGINE = MergeTree()
            PARTITION BY date
            ORDER BY (system_id, area_id, level, timestamp)
        ").await?;

        // Create traffic shaping metrics table
        client.execute("
            CREATE TABLE IF NOT EXISTS traffic_shaping_metrics (
                timestamp UInt64,
                router_id String,
                interface String,
                algorithm String,
                class_id UInt8,
                packets_processed UInt64,
                packets_dropped UInt64,
                bytes_processed UInt64,
                bytes_dropped UInt64,
                queue_length UInt32,
                throughput_bps Float64,
                date Date MATERIALIZED toDate(timestamp / 1000)
            ) ENGINE = MergeTree()
            PARTITION BY date
            ORDER BY (router_id, interface, algorithm, timestamp)
        ").await?;

        // Create netem impairments table
        client.execute("
            CREATE TABLE IF NOT EXISTS netem_impairments (
                timestamp UInt64,
                router_id String,
                interface String,
                impairment_type String,
                parameters Map(String, String),
                active UInt8,
                date Date MATERIALIZED toDate(timestamp / 1000)
            ) ENGINE = MergeTree()
            PARTITION BY date
            ORDER BY (router_id, interface, impairment_type, timestamp)
        ").await?;

        Ok(Self {
            pool,
            metrics_buffer: Arc::new(RwLock::new(Vec::new())),
            packet_flows_buffer: Arc::new(RwLock::new(Vec::new())),
            bgp_updates_buffer: Arc::new(RwLock::new(Vec::new())),
            ospf_updates_buffer: Arc::new(RwLock::new(Vec::new())),
            isis_updates_buffer: Arc::new(RwLock::new(Vec::new())),
            traffic_shaping_buffer: Arc::new(RwLock::new(Vec::new())),
            netem_impairments_buffer: Arc::new(RwLock::new(Vec::new())),
        })
    }

    pub async fn insert_metric(&self, metric: NetworkMetric) -> Result<(), Box<dyn std::error::Error>> {
        let mut buffer = self.metrics_buffer.write().await;
        buffer.push(metric);
        
        // Flush if buffer is full
        if buffer.len() >= 1000 {
            self.flush_metrics().await?;
        }
        
        Ok(())
    }

    pub async fn insert_packet_flow(&self, flow: PacketFlow) -> Result<(), Box<dyn std::error::Error>> {
        let mut buffer = self.packet_flows_buffer.write().await;
        buffer.push(flow);
        
        // Flush if buffer is full
        if buffer.len() >= 1000 {
            self.flush_packet_flows().await?;
        }
        
        Ok(())
    }

    pub async fn insert_bgp_update(&self, update: BGPUpdate) -> Result<(), Box<dyn std::error::Error>> {
        let mut buffer = self.bgp_updates_buffer.write().await;
        buffer.push(update);
        
        // Flush if buffer is full
        if buffer.len() >= 100 {
            self.flush_bgp_updates().await?;
        }
        
        Ok(())
    }

    pub async fn insert_ospf_update(&self, update: OSPFUpdate) -> Result<(), Box<dyn std::error::Error>> {
        let mut buffer = self.ospf_updates_buffer.write().await;
        buffer.push(update);
        
        // Flush if buffer is full
        if buffer.len() >= 100 {
            self.flush_ospf_updates().await?;
        }
        
        Ok(())
    }

    pub async fn insert_isis_update(&self, update: ISISUpdate) -> Result<(), Box<dyn std::error::Error>> {
        let mut buffer = self.isis_updates_buffer.write().await;
        buffer.push(update);
        
        // Flush if buffer is full
        if buffer.len() >= 100 {
            self.flush_isis_updates().await?;
        }
        
        Ok(())
    }

    pub async fn insert_traffic_shaping_metric(&self, metric: TrafficShapingMetric) -> Result<(), Box<dyn std::error::Error>> {
        let mut buffer = self.traffic_shaping_buffer.write().await;
        buffer.push(metric);
        
        // Flush if buffer is full
        if buffer.len() >= 1000 {
            self.flush_traffic_shaping_metrics().await?;
        }
        
        Ok(())
    }

    pub async fn insert_netem_impairment(&self, impairment: NetemImpairment) -> Result<(), Box<dyn std::error::Error>> {
        let mut buffer = self.netem_impairments_buffer.write().await;
        buffer.push(impairment);
        
        // Flush if buffer is full
        if buffer.len() >= 100 {
            self.flush_netem_impairments().await?;
        }
        
        Ok(())
    }

    async fn flush_metrics(&self) -> Result<(), Box<dyn std::error::Error>> {
        let mut buffer = self.metrics_buffer.write().await;
        if buffer.is_empty() {
            return Ok(());
        }

        let mut client = self.pool.get_handle().await?;
        let mut block = Block::new();
        
        for metric in buffer.drain(..) {
            block.push((
                metric.timestamp,
                metric.router_id,
                metric.interface,
                metric.metric_type,
                metric.value,
                metric.tags,
            ));
        }

        client.insert("network_metrics", block).await?;
        Ok(())
    }

    async fn flush_packet_flows(&self) -> Result<(), Box<dyn std::error::Error>> {
        let mut buffer = self.packet_flows_buffer.write().await;
        if buffer.is_empty() {
            return Ok(());
        }

        let mut client = self.pool.get_handle().await?;
        let mut block = Block::new();
        
        for flow in buffer.drain(..) {
            block.push((
                flow.timestamp,
                flow.src_ip,
                flow.dst_ip,
                flow.src_port,
                flow.dst_port,
                flow.protocol,
                flow.bytes,
                flow.packets,
                flow.duration_ms,
                flow.router_id,
            ));
        }

        client.insert("packet_flows", block).await?;
        Ok(())
    }

    async fn flush_bgp_updates(&self) -> Result<(), Box<dyn std::error::Error>> {
        let mut buffer = self.bgp_updates_buffer.write().await;
        if buffer.is_empty() {
            return Ok(());
        }

        let mut client = self.pool.get_handle().await?;
        let mut block = Block::new();
        
        for update in buffer.drain(..) {
            block.push((
                update.timestamp,
                update.router_id,
                update.neighbor_ip,
                update.prefix,
                update.prefix_length,
                update.as_path,
                update.next_hop,
                update.origin,
                update.local_pref,
                update.med,
                update.communities,
                update.action,
            ));
        }

        client.insert("bgp_updates", block).await?;
        Ok(())
    }

    async fn flush_ospf_updates(&self) -> Result<(), Box<dyn std::error::Error>> {
        let mut buffer = self.ospf_updates_buffer.write().await;
        if buffer.is_empty() {
            return Ok(());
        }

        let mut client = self.pool.get_handle().await?;
        let mut block = Block::new();
        
        for update in buffer.drain(..) {
            block.push((
                update.timestamp,
                update.router_id,
                update.area_id,
                update.lsa_type,
                update.lsa_id,
                update.advertising_router,
                update.sequence_number,
                update.age,
                update.checksum,
                update.length,
                update.action,
            ));
        }

        client.insert("ospf_updates", block).await?;
        Ok(())
    }

    async fn flush_isis_updates(&self) -> Result<(), Box<dyn std::error::Error>> {
        let mut buffer = self.isis_updates_buffer.write().await;
        if buffer.is_empty() {
            return Ok(());
        }

        let mut client = self.pool.get_handle().await?;
        let mut block = Block::new();
        
        for update in buffer.drain(..) {
            block.push((
                update.timestamp,
                update.system_id,
                update.area_id,
                update.level,
                update.lsp_id,
                update.sequence_number,
                update.remaining_lifetime,
                update.checksum,
                update.pdu_length,
                update.action,
            ));
        }

        client.insert("isis_updates", block).await?;
        Ok(())
    }

    async fn flush_traffic_shaping_metrics(&self) -> Result<(), Box<dyn std::error::Error>> {
        let mut buffer = self.traffic_shaping_buffer.write().await;
        if buffer.is_empty() {
            return Ok(());
        }

        let mut client = self.pool.get_handle().await?;
        let mut block = Block::new();
        
        for metric in buffer.drain(..) {
            block.push((
                metric.timestamp,
                metric.router_id,
                metric.interface,
                metric.algorithm,
                metric.class_id,
                metric.packets_processed,
                metric.packets_dropped,
                metric.bytes_processed,
                metric.bytes_dropped,
                metric.queue_length,
                metric.throughput_bps,
            ));
        }

        client.insert("traffic_shaping_metrics", block).await?;
        Ok(())
    }

    async fn flush_netem_impairments(&self) -> Result<(), Box<dyn std::error::Error>> {
        let mut buffer = self.netem_impairments_buffer.write().await;
        if buffer.is_empty() {
            return Ok(());
        }

        let mut client = self.pool.get_handle().await?;
        let mut block = Block::new();
        
        for impairment in buffer.drain(..) {
            block.push((
                impairment.timestamp,
                impairment.router_id,
                impairment.interface,
                impairment.impairment_type,
                impairment.parameters,
                if impairment.active { 1u8 } else { 0u8 },
            ));
        }

        client.insert("netem_impairments", block).await?;
        Ok(())
    }

    pub async fn flush_all(&self) -> Result<(), Box<dyn std::error::Error>> {
        self.flush_metrics().await?;
        self.flush_packet_flows().await?;
        self.flush_bgp_updates().await?;
        self.flush_ospf_updates().await?;
        self.flush_isis_updates().await?;
        self.flush_traffic_shaping_metrics().await?;
        self.flush_netem_impairments().await?;
        Ok(())
    }

    pub async fn query_metrics(&self, query: &str) -> Result<Vec<HashMap<String, String>>, Box<dyn std::error::Error>> {
        let mut client = self.pool.get_handle().await?;
        let mut cursor = client.query(query).fetch_all().await?;
        
        let mut results = Vec::new();
        while let Some(row) = cursor.next().await? {
            let mut row_data = HashMap::new();
            for (i, column) in row.columns().iter().enumerate() {
                let value = row.get::<String, _>(i)?;
                row_data.insert(column.name().to_string(), value);
            }
            results.push(row_data);
        }
        
        Ok(results)
    }

    pub async fn get_router_metrics(&self, router_id: &str, time_range: (u64, u64)) -> Result<Vec<HashMap<String, String>>, Box<dyn std::error::Error>> {
        let query = format!(
            "SELECT * FROM network_metrics 
             WHERE router_id = '{}' 
             AND timestamp >= {} 
             AND timestamp <= {} 
             ORDER BY timestamp DESC",
            router_id, time_range.0, time_range.1
        );
        
        self.query_metrics(&query).await
    }

    pub async fn get_interface_metrics(&self, router_id: &str, interface: &str, time_range: (u64, u64)) -> Result<Vec<HashMap<String, String>>, Box<dyn std::error::Error>> {
        let query = format!(
            "SELECT * FROM network_metrics 
             WHERE router_id = '{}' 
             AND interface = '{}' 
             AND timestamp >= {} 
             AND timestamp <= {} 
             ORDER BY timestamp DESC",
            router_id, interface, time_range.0, time_range.1
        );
        
        self.query_metrics(&query).await
    }

    pub async fn get_bgp_convergence_metrics(&self, router_id: &str, time_range: (u64, u64)) -> Result<Vec<HashMap<String, String>>, Box<dyn std::error::Error>> {
        let query = format!(
            "SELECT 
                neighbor_ip,
                COUNT(*) as update_count,
                COUNTIf(action = 'advertise') as advertisements,
                COUNTIf(action = 'withdraw') as withdrawals,
                uniq(prefix) as unique_prefixes
             FROM bgp_updates 
             WHERE router_id = '{}' 
             AND timestamp >= {} 
             AND timestamp <= {} 
             GROUP BY neighbor_ip 
             ORDER BY update_count DESC",
            router_id, time_range.0, time_range.1
        );
        
        self.query_metrics(&query).await
    }

    pub async fn get_traffic_flow_analysis(&self, router_id: &str, time_range: (u64, u64)) -> Result<Vec<HashMap<String, String>>, Box<dyn std::error::Error>> {
        let query = format!(
            "SELECT 
                src_ip,
                dst_ip,
                protocol,
                SUM(bytes) as total_bytes,
                SUM(packets) as total_packets,
                AVG(duration_ms) as avg_duration_ms
             FROM packet_flows 
             WHERE router_id = '{}' 
             AND timestamp >= {} 
             AND timestamp <= {} 
             GROUP BY src_ip, dst_ip, protocol 
             ORDER BY total_bytes DESC 
             LIMIT 100",
            router_id, time_range.0, time_range.1
        );
        
        self.query_metrics(&query).await
    }

    pub async fn get_traffic_shaping_effectiveness(&self, router_id: &str, time_range: (u64, u64)) -> Result<Vec<HashMap<String, String>>, Box<dyn std::error::Error>> {
        let query = format!(
            "SELECT 
                interface,
                algorithm,
                class_id,
                SUM(packets_processed) as total_processed,
                SUM(packets_dropped) as total_dropped,
                SUM(bytes_processed) as total_bytes_processed,
                SUM(bytes_dropped) as total_bytes_dropped,
                AVG(throughput_bps) as avg_throughput
             FROM traffic_shaping_metrics 
             WHERE router_id = '{}' 
             AND timestamp >= {} 
             AND timestamp <= {} 
             GROUP BY interface, algorithm, class_id 
             ORDER BY total_processed DESC",
            router_id, time_range.0, time_range.1
        );
        
        self.query_metrics(&query).await
    }

    pub async fn get_network_impairment_impact(&self, router_id: &str, time_range: (u64, u64)) -> Result<Vec<HashMap<String, String>>, Box<dyn std::error::Error>> {
        let query = format!(
            "SELECT 
                interface,
                impairment_type,
                COUNT(*) as impairment_events,
                COUNTIf(active = 1) as active_impairments
             FROM netem_impairments 
             WHERE router_id = '{}' 
             AND timestamp >= {} 
             AND timestamp <= {} 
             GROUP BY interface, impairment_type 
             ORDER BY impairment_events DESC",
            router_id, time_range.0, time_range.1
        );
        
        self.query_metrics(&query).await
    }
}

impl Drop for ClickHouseAnalytics {
    fn drop(&mut self) {
        // Flush all buffers on drop
        let rt = tokio::runtime::Runtime::new().unwrap();
        rt.block_on(self.flush_all()).ok();
    }
}
