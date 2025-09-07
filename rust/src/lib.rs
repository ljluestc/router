//! Router Simulator Rust Components
//! 
//! This crate provides high-performance Rust components for the router simulator,
//! including packet processing, routing algorithms, and network topology management.

use std::collections::HashMap;
use std::sync::Arc;
use std::time::{Duration, Instant};
use anyhow::Result;
use serde::{Deserialize, Serialize};
use tokio::sync::RwLock;
use tracing::{info, warn, error, debug};

pub mod packet_processor;
pub mod routing_engine;
pub mod topology_manager;
pub mod performance_monitor;
pub mod ffi;

/// Main router simulator engine
pub struct RouterEngine {
    packet_processor: Arc<packet_processor::PacketProcessor>,
    routing_engine: Arc<routing_engine::RoutingEngine>,
    topology_manager: Arc<topology_manager::TopologyManager>,
    performance_monitor: Arc<performance_monitor::PerformanceMonitor>,
}

impl RouterEngine {
    pub fn new() -> Self {
        Self {
            packet_processor: Arc::new(packet_processor::PacketProcessor::new()),
            routing_engine: Arc::new(routing_engine::RoutingEngine::new()),
            topology_manager: Arc::new(topology_manager::TopologyManager::new()),
            performance_monitor: Arc::new(performance_monitor::PerformanceMonitor::new()),
        }
    }

    pub async fn initialize(&self) -> Result<()> {
        info!("Initializing router engine");
        
        self.packet_processor.initialize().await?;
        self.routing_engine.initialize().await?;
        self.topology_manager.initialize().await?;
        self.performance_monitor.initialize().await?;
        
        info!("Router engine initialized successfully");
        Ok(())
    }

    pub async fn start(&self) -> Result<()> {
        info!("Starting router engine");
        
        self.packet_processor.start().await?;
        self.routing_engine.start().await?;
        self.topology_manager.start().await?;
        self.performance_monitor.start().await?;
        
        info!("Router engine started successfully");
        Ok(())
    }

    pub async fn stop(&self) -> Result<()> {
        info!("Stopping router engine");
        
        self.performance_monitor.stop().await?;
        self.topology_manager.stop().await?;
        self.routing_engine.stop().await?;
        self.packet_processor.stop().await?;
        
        info!("Router engine stopped successfully");
        Ok(())
    }

    pub fn get_packet_processor(&self) -> Arc<packet_processor::PacketProcessor> {
        self.packet_processor.clone()
    }

    pub fn get_routing_engine(&self) -> Arc<routing_engine::RoutingEngine> {
        self.routing_engine.clone()
    }

    pub fn get_topology_manager(&self) -> Arc<topology_manager::TopologyManager> {
        self.topology_manager.clone()
    }

    pub fn get_performance_monitor(&self) -> Arc<performance_monitor::PerformanceMonitor> {
        self.performance_monitor.clone()
    }
}

/// Router configuration
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct RouterConfig {
    pub router_id: String,
    pub hostname: String,
    pub interfaces: Vec<InterfaceConfig>,
    pub protocols: ProtocolConfig,
    pub performance: PerformanceConfig,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct InterfaceConfig {
    pub name: String,
    pub ip_address: String,
    pub subnet_mask: String,
    pub bandwidth_mbps: u32,
    pub is_up: bool,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ProtocolConfig {
    pub bgp: Option<BgpConfig>,
    pub ospf: Option<OspfConfig>,
    pub isis: Option<IsisConfig>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct BgpConfig {
    pub as_number: u32,
    pub router_id: String,
    pub neighbors: Vec<BgpNeighbor>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct BgpNeighbor {
    pub address: String,
    pub as_number: u32,
    pub hold_time: u32,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct OspfConfig {
    pub router_id: String,
    pub area_id: String,
    pub interfaces: Vec<OspfInterface>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct OspfInterface {
    pub name: String,
    pub area_id: String,
    pub cost: u32,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct IsisConfig {
    pub system_id: String,
    pub area_id: String,
    pub level: u8,
    pub interfaces: Vec<IsisInterface>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct IsisInterface {
    pub name: String,
    pub level: u8,
    pub cost: u32,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct PerformanceConfig {
    pub max_packets_per_second: u64,
    pub max_routes: u64,
    pub max_neighbors: u64,
    pub enable_parallel_processing: bool,
    pub worker_threads: usize,
}

/// Performance metrics
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct PerformanceMetrics {
    pub packets_processed: u64,
    pub packets_per_second: f64,
    pub routes_processed: u64,
    pub memory_usage_mb: f64,
    pub cpu_usage_percent: f64,
    pub latency_ms: f64,
    pub timestamp: Instant,
}

impl Default for RouterConfig {
    fn default() -> Self {
        Self {
            router_id: "1.1.1.1".to_string(),
            hostname: "router".to_string(),
            interfaces: Vec::new(),
            protocols: ProtocolConfig {
                bgp: None,
                ospf: None,
                isis: None,
            },
            performance: PerformanceConfig {
                max_packets_per_second: 1_000_000,
                max_routes: 100_000,
                max_neighbors: 1000,
                enable_parallel_processing: true,
                worker_threads: num_cpus::get(),
            },
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[tokio::test]
    async fn test_router_engine_initialization() {
        let engine = RouterEngine::new();
        assert!(engine.initialize().await.is_ok());
    }

    #[tokio::test]
    async fn test_router_engine_lifecycle() {
        let engine = RouterEngine::new();
        assert!(engine.initialize().await.is_ok());
        assert!(engine.start().await.is_ok());
        assert!(engine.stop().await.is_ok());
    }
}
