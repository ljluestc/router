use std::collections::HashMap;
use std::time::Duration;
use anyhow::Result;
use tracing::{debug, error};
use crate::{ParsedPacket, TrafficStats};

pub struct AnalyticsEngine {
    config: crate::AnalyticsEngineConfig,
    packet_count: u64,
    byte_count: u64,
    protocol_stats: HashMap<String, u64>,
    flow_stats: HashMap<String, u64>,
}

impl AnalyticsEngine {
    pub fn new(config: crate::AnalyticsEngineConfig) -> Result<Self> {
        Ok(Self {
            config,
            packet_count: 0,
            byte_count: 0,
            protocol_stats: HashMap::new(),
            flow_stats: HashMap::new(),
        })
    }

    pub async fn start(&self) -> Result<()> {
        debug!("Starting analytics engine");
        Ok(())
    }

    pub async fn stop(&self) -> Result<()> {
        debug!("Stopping analytics engine");
        Ok(())
    }

    pub async fn process_packet(&self, packet: ParsedPacket) -> Result<()> {
        // Update statistics
        self.packet_count += 1;
        self.byte_count += packet.bytes as u64;
        
        *self.protocol_stats.entry(packet.protocol.clone()).or_insert(0) += 1;
        *self.flow_stats.entry(packet.flow_id.clone()).or_insert(0) += 1;

        debug!("Processed packet: {} bytes, protocol: {}", packet.bytes, packet.protocol);
        Ok(())
    }

    pub async fn get_traffic_stats(&self) -> Result<TrafficStats> {
        let mut top_protocols: Vec<(String, u64)> = self.protocol_stats
            .iter()
            .map(|(k, v)| (k.clone(), *v))
            .collect();
        top_protocols.sort_by(|a, b| b.1.cmp(&a.1));

        let mut top_flows: Vec<(String, u64)> = self.flow_stats
            .iter()
            .map(|(k, v)| (k.clone(), *v))
            .collect();
        top_flows.sort_by(|a, b| b.1.cmp(&a.1));

        Ok(TrafficStats {
            total_packets: self.packet_count,
            total_bytes: self.byte_count,
            packets_per_second: 0.0, // Would calculate based on time window
            bytes_per_second: 0.0,   // Would calculate based on time window
            top_protocols: top_protocols.into_iter().take(10).collect(),
            top_flows: top_flows.into_iter().take(10).collect(),
        })
    }
}
