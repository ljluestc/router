//! Router Simulation - Rust Performance Components
//! 
//! This crate provides high-performance Rust implementations for
//! performance-critical parts of the router simulation.

use std::sync::Arc;
use std::collections::HashMap;
use std::sync::RwLock;
use std::time::{Duration, Instant};

/// High-performance packet processing engine
pub mod packet_engine;
/// Fast routing table implementation
pub mod routing_table;
/// Performance monitoring and metrics
pub mod metrics;
/// Memory pool for packet buffers
pub mod memory_pool;

/// Packet structure optimized for performance
#[derive(Clone, Debug)]
pub struct FastPacket {
    pub id: u64,
    pub timestamp: u64,
    pub src_ip: u32,
    pub dst_ip: u32,
    pub src_port: u16,
    pub dst_port: u16,
    pub protocol: u8,
    pub size: u32,
    pub ttl: u8,
    pub priority: u32,
    pub data: Vec<u8>,
}

impl FastPacket {
    /// Create a new packet with pre-allocated buffer
    pub fn new(id: u64, size: u32) -> Self {
        Self {
            id,
            timestamp: 0,
            src_ip: 0,
            dst_ip: 0,
            src_port: 0,
            dst_port: 0,
            protocol: 0,
            size,
            ttl: 64,
            priority: 0,
            data: vec![0u8; size as usize],
        }
    }
    
    /// Get packet header as bytes for fast processing
    pub fn header_bytes(&self) -> [u8; 20] {
        let mut header = [0u8; 20];
        header[0..8].copy_from_slice(&self.id.to_le_bytes());
        header[8..12].copy_from_slice(&self.src_ip.to_le_bytes());
        header[12..16].copy_from_slice(&self.dst_ip.to_le_bytes());
        header[16..18].copy_from_slice(&self.src_port.to_le_bytes());
        header[18..20].copy_from_slice(&self.dst_port.to_le_bytes());
        header
    }
}

/// Performance statistics
#[derive(Default, Debug)]
pub struct PerformanceStats {
    pub packets_processed: u64,
    pub bytes_processed: u64,
    pub processing_time_ns: u64,
    pub average_latency_ns: u64,
    pub max_latency_ns: u64,
    pub min_latency_ns: u64,
    pub errors: u64,
}

impl PerformanceStats {
    /// Update statistics with a new packet processing time
    pub fn update(&mut self, packet_size: u32, processing_time_ns: u64) {
        self.packets_processed += 1;
        self.bytes_processed += packet_size as u64;
        self.processing_time_ns += processing_time_ns;
        
        if self.min_latency_ns == 0 || processing_time_ns < self.min_latency_ns {
            self.min_latency_ns = processing_time_ns;
        }
        
        if processing_time_ns > self.max_latency_ns {
            self.max_latency_ns = processing_time_ns;
        }
        
        self.average_latency_ns = self.processing_time_ns / self.packets_processed;
    }
    
    /// Get throughput in packets per second
    pub fn throughput_pps(&self) -> f64 {
        if self.processing_time_ns == 0 {
            return 0.0;
        }
        self.packets_processed as f64 * 1_000_000_000.0 / self.processing_time_ns as f64
    }
    
    /// Get throughput in bits per second
    pub fn throughput_bps(&self) -> f64 {
        if self.processing_time_ns == 0 {
            return 0.0;
        }
        self.bytes_processed as f64 * 8.0 * 1_000_000_000.0 / self.processing_time_ns as f64
    }
}

/// High-performance router core
pub struct FastRouter {
    routing_table: Arc<RwLock<routing_table::FastRoutingTable>>,
    packet_engine: Arc<packet_engine::PacketEngine>,
    memory_pool: Arc<memory_pool::MemoryPool>,
    stats: Arc<RwLock<PerformanceStats>>,
}

impl FastRouter {
    /// Create a new fast router instance
    pub fn new() -> Self {
        Self {
            routing_table: Arc::new(RwLock::new(routing_table::FastRoutingTable::new())),
            packet_engine: Arc::new(packet_engine::PacketEngine::new()),
            memory_pool: Arc::new(memory_pool::MemoryPool::new(1000, 1500)),
            stats: Arc::new(RwLock::new(PerformanceStats::default())),
        }
    }
    
    /// Process a packet with high performance
    pub fn process_packet(&self, packet: &mut FastPacket) -> Result<(), String> {
        let start_time = Instant::now();
        
        // Get packet from memory pool
        let mut pooled_packet = self.memory_pool.get_packet(packet.size)?;
        pooled_packet.copy_from(packet);
        
        // Process packet
        let result = self.packet_engine.process_packet(&mut pooled_packet, &self.routing_table);
        
        // Update statistics
        let processing_time = start_time.elapsed();
        let mut stats = self.stats.write().unwrap();
        stats.update(packet.size, processing_time.as_nanos() as u64);
        
        // Return packet to pool
        self.memory_pool.return_packet(pooled_packet);
        
        result
    }
    
    /// Add a route to the routing table
    pub fn add_route(&self, network: u32, mask: u32, next_hop: u32, interface: String, metric: u32) {
        let mut table = self.routing_table.write().unwrap();
        table.add_route(network, mask, next_hop, interface, metric);
    }
    
    /// Remove a route from the routing table
    pub fn remove_route(&self, network: u32, mask: u32) -> bool {
        let mut table = self.routing_table.write().unwrap();
        table.remove_route(network, mask)
    }
    
    /// Get performance statistics
    pub fn get_stats(&self) -> PerformanceStats {
        self.stats.read().unwrap().clone()
    }
    
    /// Reset statistics
    pub fn reset_stats(&self) {
        let mut stats = self.stats.write().unwrap();
        *stats = PerformanceStats::default();
    }
}

impl Default for FastRouter {
    fn default() -> Self {
        Self::new()
    }
}

/// Benchmark utilities
pub mod benchmark {
    use super::*;
    use std::time::Instant;
    
    /// Benchmark packet processing performance
    pub fn benchmark_packet_processing(router: &FastRouter, num_packets: usize) -> BenchmarkResult {
        let mut packets = Vec::with_capacity(num_packets);
        
        // Create test packets
        for i in 0..num_packets {
            let mut packet = FastPacket::new(i as u64, 1500);
            packet.src_ip = 0xC0A80101; // 192.168.1.1
            packet.dst_ip = 0x0A000001; // 10.0.0.1
            packet.src_port = 12345;
            packet.dst_port = 80;
            packet.protocol = 6; // TCP
            packet.size = 1500;
            packet.ttl = 64;
            packet.priority = 0;
            packets.push(packet);
        }
        
        // Benchmark processing
        let start_time = Instant::now();
        
        for packet in &mut packets {
            let _ = router.process_packet(packet);
        }
        
        let end_time = Instant::now();
        let duration = end_time.duration_since(start_time);
        
        let stats = router.get_stats();
        
        BenchmarkResult {
            num_packets,
            duration_ms: duration.as_millis() as u64,
            packets_per_second: stats.throughput_pps(),
            bits_per_second: stats.throughput_bps(),
            average_latency_ns: stats.average_latency_ns,
            min_latency_ns: stats.min_latency_ns,
            max_latency_ns: stats.max_latency_ns,
        }
    }
    
    /// Benchmark result
    #[derive(Debug)]
    pub struct BenchmarkResult {
        pub num_packets: usize,
        pub duration_ms: u64,
        pub packets_per_second: f64,
        pub bits_per_second: f64,
        pub average_latency_ns: u64,
        pub min_latency_ns: u64,
        pub max_latency_ns: u64,
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    
    #[test]
    fn test_fast_packet_creation() {
        let packet = FastPacket::new(1, 1500);
        assert_eq!(packet.id, 1);
        assert_eq!(packet.size, 1500);
        assert_eq!(packet.data.len(), 1500);
    }
    
    #[test]
    fn test_fast_router_creation() {
        let router = FastRouter::new();
        let stats = router.get_stats();
        assert_eq!(stats.packets_processed, 0);
    }
    
    #[test]
    fn test_route_management() {
        let router = FastRouter::new();
        
        // Add route
        router.add_route(0x0A000000, 0xFFFFFF00, 0xC0A80101, "eth0".to_string(), 1);
        
        // Process packet
        let mut packet = FastPacket::new(1, 1500);
        packet.src_ip = 0xC0A80102;
        packet.dst_ip = 0x0A000001;
        
        let result = router.process_packet(&mut packet);
        assert!(result.is_ok());
        
        let stats = router.get_stats();
        assert_eq!(stats.packets_processed, 1);
    }
    
    #[test]
    fn test_performance_stats() {
        let mut stats = PerformanceStats::default();
        
        stats.update(1500, 1000);
        stats.update(1500, 2000);
        
        assert_eq!(stats.packets_processed, 2);
        assert_eq!(stats.bytes_processed, 3000);
        assert_eq!(stats.average_latency_ns, 1500);
        assert_eq!(stats.min_latency_ns, 1000);
        assert_eq!(stats.max_latency_ns, 2000);
    }
}
