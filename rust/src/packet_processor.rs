//! High-performance packet processor implementation in Rust

use std::collections::HashMap;
use std::sync::Arc;
use std::time::Instant;
use anyhow::Result;
use serde::{Deserialize, Serialize};
use tokio::sync::RwLock;
use tracing::{info, warn, error, debug};
use rayon::prelude::*;
use crossbeam::channel::{self, Receiver, Sender};
use dashmap::DashMap;

/// Packet structure for high-performance processing
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Packet {
    pub data: Vec<u8>,
    pub interface: String,
    pub timestamp: Instant,
    pub size: usize,
    pub protocol: u16,
    pub src_ip: String,
    pub dst_ip: String,
    pub src_port: u16,
    pub dst_port: u16,
    pub ttl: u8,
    pub tos: u8,
    pub is_fragmented: bool,
    pub fragment_offset: u16,
    pub identification: u16,
}

/// Packet processing statistics
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct PacketStats {
    pub packets_received: u64,
    pub packets_processed: u64,
    pub packets_dropped: u64,
    pub packets_forwarded: u64,
    pub bytes_received: u64,
    pub bytes_processed: u64,
    pub bytes_dropped: u64,
    pub bytes_forwarded: u64,
    pub errors: u64,
    pub processing_time_ns: u64,
    pub last_update: Instant,
}

impl Default for PacketStats {
    fn default() -> Self {
        Self {
            packets_received: 0,
            packets_processed: 0,
            packets_dropped: 0,
            packets_forwarded: 0,
            bytes_received: 0,
            bytes_processed: 0,
            bytes_dropped: 0,
            bytes_forwarded: 0,
            errors: 0,
            processing_time_ns: 0,
            last_update: Instant::now(),
        }
    }
}

/// High-performance packet processor
pub struct PacketProcessor {
    interfaces: Arc<DashMap<String, InterfaceData>>,
    packet_channels: Arc<DashMap<String, (Sender<Packet>, Receiver<Packet>)>>,
    processing_workers: Vec<tokio::task::JoinHandle<()>>,
    stats: Arc<RwLock<PacketStats>>,
    is_running: Arc<RwLock<bool>>,
}

struct InterfaceData {
    name: String,
    device: String,
    is_active: bool,
    stats: Arc<RwLock<PacketStats>>,
}

impl PacketProcessor {
    pub fn new() -> Self {
        Self {
            interfaces: Arc::new(DashMap::new()),
            packet_channels: Arc::new(DashMap::new()),
            processing_workers: Vec::new(),
            stats: Arc::new(RwLock::new(PacketStats::default())),
            is_running: Arc::new(RwLock::new(false)),
        }
    }

    pub async fn initialize(&self) -> Result<()> {
        info!("Initializing packet processor");
        
        // Initialize packet processing workers
        let worker_count = num_cpus::get();
        for i in 0..worker_count {
            let interfaces = self.interfaces.clone();
            let packet_channels = self.packet_channels.clone();
            let stats = self.stats.clone();
            let is_running = self.is_running.clone();
            
            let worker = tokio::spawn(async move {
                Self::processing_worker(i, interfaces, packet_channels, stats, is_running).await;
            });
            
            self.processing_workers.push(worker);
        }
        
        info!("Packet processor initialized with {} workers", worker_count);
        Ok(())
    }

    pub async fn start(&self) -> Result<()> {
        info!("Starting packet processor");
        
        let mut running = self.is_running.write().await;
        *running = true;
        drop(running);
        
        info!("Packet processor started");
        Ok(())
    }

    pub async fn stop(&self) -> Result<()> {
        info!("Stopping packet processor");
        
        let mut running = self.is_running.write().await;
        *running = false;
        drop(running);
        
        // Wait for all workers to finish
        for worker in &self.processing_workers {
            worker.abort();
        }
        
        info!("Packet processor stopped");
        Ok(())
    }

    pub async fn add_interface(&self, name: String, device: String) -> Result<()> {
        info!("Adding interface: {} -> {}", name, device);
        
        let (tx, rx) = channel::unbounded();
        self.packet_channels.insert(name.clone(), (tx, rx));
        
        let interface_data = InterfaceData {
            name: name.clone(),
            device,
            is_active: true,
            stats: Arc::new(RwLock::new(PacketStats::default())),
        };
        
        self.interfaces.insert(name, interface_data);
        
        info!("Interface added successfully");
        Ok(())
    }

    pub async fn remove_interface(&self, name: &str) -> Result<()> {
        info!("Removing interface: {}", name);
        
        self.interfaces.remove(name);
        self.packet_channels.remove(name);
        
        info!("Interface removed successfully");
        Ok(())
    }

    pub async fn process_packet(&self, packet: Packet) -> Result<()> {
        let interface_name = packet.interface.clone();
        
        if let Some((tx, _)) = self.packet_channels.get(&interface_name) {
            tx.send(packet)?;
            Ok(())
        } else {
            Err(anyhow::anyhow!("Interface not found: {}", interface_name))
        }
    }

    pub async fn get_stats(&self) -> PacketStats {
        self.stats.read().await.clone()
    }

    pub async fn get_interface_stats(&self, interface: &str) -> Option<PacketStats> {
        if let Some(interface_data) = self.interfaces.get(interface) {
            Some(interface_data.stats.read().await.clone())
        } else {
            None
        }
    }

    async fn processing_worker(
        worker_id: usize,
        interfaces: Arc<DashMap<String, InterfaceData>>,
        packet_channels: Arc<DashMap<String, (Sender<Packet>, Receiver<Packet>)>>,
        global_stats: Arc<RwLock<PacketStats>>,
        is_running: Arc<RwLock<bool>>,
    ) {
        info!("Starting packet processing worker {}", worker_id);
        
        let mut packet_batch = Vec::with_capacity(1000);
        
        loop {
            // Check if we should stop
            {
                let running = is_running.read().await;
                if !*running {
                    break;
                }
            }
            
            // Collect packets from all interfaces
            for entry in packet_channels.iter() {
                let (_, rx) = entry.value();
                
                // Try to collect a batch of packets
                while let Ok(packet) = rx.try_recv() {
                    packet_batch.push(packet);
                    
                    if packet_batch.len() >= 1000 {
                        break;
                    }
                }
            }
            
            // Process the batch of packets
            if !packet_batch.is_empty() {
                Self::process_packet_batch(&packet_batch, &interfaces, &global_stats).await;
                packet_batch.clear();
            }
            
            // Small delay to prevent busy waiting
            tokio::time::sleep(tokio::time::Duration::from_micros(100)).await;
        }
        
        info!("Packet processing worker {} stopped", worker_id);
    }

    async fn process_packet_batch(
        packets: &[Packet],
        interfaces: &Arc<DashMap<String, InterfaceData>>,
        global_stats: &Arc<RwLock<PacketStats>>,
    ) {
        let start_time = Instant::now();
        
        // Process packets in parallel
        let results: Vec<_> = packets
            .par_iter()
            .map(|packet| Self::process_single_packet(packet))
            .collect();
        
        // Update statistics
        let mut stats = global_stats.write().await;
        for result in results {
            match result {
                Ok(processed) => {
                    if processed {
                        stats.packets_processed += 1;
                        stats.bytes_processed += packet.size as u64;
                    } else {
                        stats.packets_dropped += 1;
                        stats.bytes_dropped += packet.size as u64;
                    }
                }
                Err(_) => {
                    stats.errors += 1;
                }
            }
        }
        
        stats.packets_received += packets.len() as u64;
        stats.bytes_received += packets.iter().map(|p| p.size as u64).sum::<u64>();
        stats.processing_time_ns += start_time.elapsed().as_nanos() as u64;
        stats.last_update = Instant::now();
    }

    fn process_single_packet(packet: &Packet) -> Result<bool> {
        // Basic packet validation
        if packet.data.len() < 14 {
            return Ok(false); // Too small for Ethernet
        }
        
        // Parse Ethernet header
        let eth_header = &packet.data[0..14];
        let ethertype = u16::from_be_bytes([eth_header[12], eth_header[13]]);
        
        match ethertype {
            0x0800 => {
                // IPv4
                Self::process_ipv4_packet(packet)
            }
            0x86DD => {
                // IPv6
                Self::process_ipv6_packet(packet)
            }
            _ => {
                // Other protocols
                Ok(true)
            }
        }
    }

    fn process_ipv4_packet(packet: &Packet) -> Result<bool> {
        if packet.data.len() < 34 {
            return Ok(false); // Too small for IPv4 header
        }
        
        let ip_header = &packet.data[14..34];
        let version = (ip_header[0] >> 4) & 0x0F;
        
        if version != 4 {
            return Ok(false); // Not IPv4
        }
        
        let ihl = (ip_header[0] & 0x0F) as usize * 4;
        if ihl < 20 || packet.data.len() < 14 + ihl {
            return Ok(false); // Invalid header length
        }
        
        // Extract IP addresses
        let src_ip = format!("{}.{}.{}.{}", 
            ip_header[12], ip_header[13], ip_header[14], ip_header[15]);
        let dst_ip = format!("{}.{}.{}.{}", 
            ip_header[16], ip_header[17], ip_header[18], ip_header[19]);
        
        // Extract protocol
        let protocol = ip_header[9];
        
        // Extract ports if TCP/UDP
        let src_port = if packet.data.len() >= 14 + ihl + 4 {
            u16::from_be_bytes([packet.data[14 + ihl], packet.data[14 + ihl + 1]])
        } else {
            0
        };
        
        let dst_port = if packet.data.len() >= 14 + ihl + 4 {
            u16::from_be_bytes([packet.data[14 + ihl + 2], packet.data[14 + ihl + 3]])
        } else {
            0
        };
        
        debug!("Processed IPv4 packet: {}:{} -> {}:{} (protocol: {})", 
            src_ip, src_port, dst_ip, dst_port, protocol);
        
        Ok(true)
    }

    fn process_ipv6_packet(packet: &Packet) -> Result<bool> {
        if packet.data.len() < 54 {
            return Ok(false); // Too small for IPv6 header
        }
        
        let ip_header = &packet.data[14..54];
        let version = (ip_header[0] >> 4) & 0x0F;
        
        if version != 6 {
            return Ok(false); // Not IPv6
        }
        
        // Extract IPv6 addresses
        let src_ip = format!("{:02x}{:02x}:{:02x}{:02x}:{:02x}{:02x}:{:02x}{:02x}:{:02x}{:02x}:{:02x}{:02x}:{:02x}{:02x}:{:02x}{:02x}",
            ip_header[8], ip_header[9], ip_header[10], ip_header[11],
            ip_header[12], ip_header[13], ip_header[14], ip_header[15],
            ip_header[16], ip_header[17], ip_header[18], ip_header[19],
            ip_header[20], ip_header[21], ip_header[22], ip_header[23]);
        
        let dst_ip = format!("{:02x}{:02x}:{:02x}{:02x}:{:02x}{:02x}:{:02x}{:02x}:{:02x}{:02x}:{:02x}{:02x}:{:02x}{:02x}:{:02x}{:02x}",
            ip_header[24], ip_header[25], ip_header[26], ip_header[27],
            ip_header[28], ip_header[29], ip_header[30], ip_header[31],
            ip_header[32], ip_header[33], ip_header[34], ip_header[35],
            ip_header[36], ip_header[37], ip_header[38], ip_header[39]);
        
        // Extract next header (protocol)
        let next_header = ip_header[6];
        
        debug!("Processed IPv6 packet: {} -> {} (next header: {})", 
            src_ip, dst_ip, next_header);
        
        Ok(true)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[tokio::test]
    async fn test_packet_processor_initialization() {
        let processor = PacketProcessor::new();
        assert!(processor.initialize().await.is_ok());
    }

    #[tokio::test]
    async fn test_packet_processing() {
        let processor = PacketProcessor::new();
        processor.initialize().await.unwrap();
        processor.start().await.unwrap();
        
        // Add a test interface
        processor.add_interface("eth0".to_string(), "eth0".to_string()).await.unwrap();
        
        // Create a test packet
        let packet = Packet {
            data: vec![0u8; 64], // Minimal Ethernet frame
            interface: "eth0".to_string(),
            timestamp: Instant::now(),
            size: 64,
            protocol: 0x0800, // IPv4
            src_ip: "192.168.1.1".to_string(),
            dst_ip: "192.168.1.2".to_string(),
            src_port: 80,
            dst_port: 8080,
            ttl: 64,
            tos: 0,
            is_fragmented: false,
            fragment_offset: 0,
            identification: 0,
        };
        
        // Process the packet
        assert!(processor.process_packet(packet).await.is_ok());
        
        processor.stop().await.unwrap();
    }
}
