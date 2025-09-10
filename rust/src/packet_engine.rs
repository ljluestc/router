use crate::routing_table::{RoutingTable, Route};
use crate::config::Config;
use crate::analytics::Statistics;
use pnet::datalink::{self, NetworkInterface};
use pnet::packet::{
    ethernet::{EthernetPacket, EtherTypes},
    ip::{IpNextHeaderProtocols, IpNextHeaderProtocol},
    ipv4::Ipv4Packet,
    ipv6::Ipv6Packet,
    tcp::TcpPacket,
    udp::UdpPacket,
    icmp::IcmpPacket,
    Packet,
};
use std::sync::Arc;
use std::sync::atomic::{AtomicU64, AtomicUsize, Ordering};
use std::collections::HashMap;
use tokio::sync::RwLock;
use tokio::time::{Duration, Instant};
use tracing::{info, warn, error, debug};
use anyhow::Result;

/// High-performance packet processing engine
pub struct PacketEngine {
    config: Config,
    routing_table: Arc<RwLock<RoutingTable>>,
    interfaces: Vec<NetworkInterface>,
    running: Arc<AtomicUsize>,
    stats: Arc<PacketStats>,
}

#[derive(Debug, Clone)]
struct PacketStats {
    packets_processed: AtomicU64,
    bytes_processed: AtomicU64,
    packets_dropped: AtomicU64,
    packets_forwarded: AtomicU64,
    packets_routed: AtomicU64,
    errors: AtomicU64,
    start_time: Instant,
}

impl PacketStats {
    fn new() -> Self {
        Self {
            packets_processed: AtomicU64::new(0),
            bytes_processed: AtomicU64::new(0),
            packets_dropped: AtomicU64::new(0),
            packets_forwarded: AtomicU64::new(0),
            packets_routed: AtomicU64::new(0),
            errors: AtomicU64::new(0),
            start_time: Instant::now(),
        }
    }

    fn to_statistics(&self) -> Statistics {
        Statistics {
            packets_processed: self.packets_processed.load(Ordering::Relaxed),
            bytes_processed: self.bytes_processed.load(Ordering::Relaxed),
            packets_dropped: self.packets_dropped.load(Ordering::Relaxed),
            packets_forwarded: self.packets_forwarded.load(Ordering::Relaxed),
            packets_routed: self.packets_routed.load(Ordering::Relaxed),
            errors: self.errors.load(Ordering::Relaxed),
            uptime_seconds: self.start_time.elapsed().as_secs(),
            packets_per_second: self.calculate_packets_per_second(),
            bytes_per_second: self.calculate_bytes_per_second(),
        }
    }

    fn calculate_packets_per_second(&self) -> f64 {
        let packets = self.packets_processed.load(Ordering::Relaxed) as f64;
        let uptime = self.start_time.elapsed().as_secs_f64();
        if uptime > 0.0 {
            packets / uptime
        } else {
            0.0
        }
    }

    fn calculate_bytes_per_second(&self) -> f64 {
        let bytes = self.bytes_processed.load(Ordering::Relaxed) as f64;
        let uptime = self.start_time.elapsed().as_secs_f64();
        if uptime > 0.0 {
            bytes / uptime
        } else {
            0.0
        }
    }
}

impl PacketEngine {
    /// Create a new packet engine
    pub fn new(config: Config, routing_table: Arc<RwLock<RoutingTable>>) -> Result<Self> {
        info!("Initializing packet engine");

        let interfaces = datalink::interfaces();
        info!("Found {} network interfaces", interfaces.len());

        let stats = Arc::new(PacketStats::new());

        Ok(Self {
            config,
            routing_table,
            interfaces,
            running: Arc::new(AtomicUsize::new(0)),
            stats,
        })
    }

    /// Start packet processing
    pub async fn start(&self) -> Result<()> {
        info!("Starting packet engine");

        self.running.store(1, Ordering::Relaxed);

        // Start packet capture on each interface
        for interface in &self.interfaces {
            if interface.is_up() && !interface.is_loopback() {
                self.start_interface_capture(interface.clone()).await?;
            }
        }

        info!("Packet engine started on {} interfaces", self.interfaces.len());
        Ok(())
    }

    /// Stop packet processing
    pub async fn stop(&self) -> Result<()> {
        info!("Stopping packet engine");
        self.running.store(0, Ordering::Relaxed);
        Ok(())
    }

    /// Get current statistics
    pub async fn get_stats(&self) -> Statistics {
        self.stats.to_statistics()
    }

    async fn start_interface_capture(&self, interface: NetworkInterface) -> Result<()> {
        let running = self.running.clone();
        let stats = self.stats.clone();
        let routing_table = self.routing_table.clone();
        let config = self.config.clone();

        tokio::spawn(async move {
            info!("Starting packet capture on interface: {}", interface.name);

            let (_, mut rx) = match datalink::channel(&interface, Default::default()) {
                Ok(datalink::Channel::Ethernet(tx, rx)) => (tx, rx),
                Ok(_) => {
                    error!("Unsupported datalink type for interface: {}", interface.name);
                    return;
                }
                Err(e) => {
                    error!("Failed to create datalink channel for interface {}: {}", interface.name, e);
                    return;
                }
            };

            while running.load(Ordering::Relaxed) == 1 {
                match rx.next() {
                    Ok(packet) => {
                        if let Err(e) = Self::process_packet(&packet, &stats, &routing_table, &config).await {
                            error!("Error processing packet: {}", e);
                            stats.errors.fetch_add(1, Ordering::Relaxed);
                        }
                    }
                    Err(e) => {
                        error!("Error receiving packet: {}", e);
                        stats.errors.fetch_add(1, Ordering::Relaxed);
                    }
                }
            }

            info!("Stopped packet capture on interface: {}", interface.name);
        });

        Ok(())
    }

    async fn process_packet(
        packet: &[u8],
        stats: &PacketStats,
        routing_table: &Arc<RwLock<RoutingTable>>,
        config: &Config,
    ) -> Result<()> {
        stats.packets_processed.fetch_add(1, Ordering::Relaxed);
        stats.bytes_processed.fetch_add(packet.len() as u64, Ordering::Relaxed);

        // Parse Ethernet frame
        let ethernet = match EthernetPacket::new(packet) {
            Some(ethernet) => ethernet,
            None => {
                debug!("Invalid Ethernet packet");
                stats.packets_dropped.fetch_add(1, Ordering::Relaxed);
                return Ok(());
            }
        };

        // Process based on EtherType
        match ethernet.get_ethertype() {
            EtherTypes::Ipv4 => {
                Self::process_ipv4_packet(&ethernet, stats, routing_table, config).await?;
            }
            EtherTypes::Ipv6 => {
                Self::process_ipv6_packet(&ethernet, stats, routing_table, config).await?;
            }
            EtherTypes::Arp => {
                Self::process_arp_packet(&ethernet, stats).await?;
            }
            _ => {
                debug!("Unsupported EtherType: {:?}", ethernet.get_ethertype());
                stats.packets_dropped.fetch_add(1, Ordering::Relaxed);
            }
        }

        Ok(())
    }

    async fn process_ipv4_packet(
        ethernet: &EthernetPacket,
        stats: &PacketStats,
        routing_table: &Arc<RwLock<RoutingTable>>,
        config: &Config,
    ) -> Result<()> {
        let ipv4 = match Ipv4Packet::new(ethernet.payload()) {
            Some(ipv4) => ipv4,
            None => {
                debug!("Invalid IPv4 packet");
                stats.packets_dropped.fetch_add(1, Ordering::Relaxed);
                return Ok(());
            }
        };

        debug!("Processing IPv4 packet: {} -> {}", ipv4.get_source(), ipv4.get_destination());

        // Check if packet is for this router
        if Self::is_local_packet(&ipv4, config) {
            Self::process_local_ipv4_packet(&ipv4, stats).await?;
        } else {
            // Forward packet
            Self::forward_ipv4_packet(&ipv4, stats, routing_table).await?;
        }

        Ok(())
    }

    async fn process_ipv6_packet(
        ethernet: &EthernetPacket,
        stats: &PacketStats,
        routing_table: &Arc<RwLock<RoutingTable>>,
        config: &Config,
    ) -> Result<()> {
        let ipv6 = match Ipv6Packet::new(ethernet.payload()) {
            Some(ipv6) => ipv6,
            None => {
                debug!("Invalid IPv6 packet");
                stats.packets_dropped.fetch_add(1, Ordering::Relaxed);
                return Ok(());
            }
        };

        debug!("Processing IPv6 packet: {} -> {}", ipv6.get_source(), ipv6.get_destination());

        // Check if packet is for this router
        if Self::is_local_ipv6_packet(&ipv6, config) {
            Self::process_local_ipv6_packet(&ipv6, stats).await?;
        } else {
            // Forward packet
            Self::forward_ipv6_packet(&ipv6, stats, routing_table).await?;
        }

        Ok(())
    }

    async fn process_arp_packet(
        ethernet: &EthernetPacket,
        stats: &PacketStats,
    ) -> Result<()> {
        debug!("Processing ARP packet");
        // ARP processing logic would go here
        stats.packets_forwarded.fetch_add(1, Ordering::Relaxed);
        Ok(())
    }

    fn is_local_packet(ipv4: &Ipv4Packet, config: &Config) -> bool {
        // Check if packet is destined for this router's interfaces
        for interface in &config.interfaces {
            if let Ok(ip) = interface.ip.parse::<std::net::Ipv4Addr>() {
                if ip == ipv4.get_destination() {
                    return true;
                }
            }
        }
        false
    }

    fn is_local_ipv6_packet(ipv6: &Ipv6Packet, config: &Config) -> bool {
        // Check if packet is destined for this router's interfaces
        for interface in &config.interfaces {
            if let Ok(ip) = interface.ip.parse::<std::net::Ipv6Addr>() {
                if ip == ipv6.get_destination() {
                    return true;
                }
            }
        }
        false
    }

    async fn process_local_ipv4_packet(ipv4: &Ipv4Packet, stats: &PacketStats) -> Result<()> {
        match ipv4.get_next_level_protocol() {
            IpNextHeaderProtocols::Tcp => {
                Self::process_tcp_packet(ipv4.payload(), stats).await?;
            }
            IpNextHeaderProtocols::Udp => {
                Self::process_udp_packet(ipv4.payload(), stats).await?;
            }
            IpNextHeaderProtocols::Icmp => {
                Self::process_icmp_packet(ipv4.payload(), stats).await?;
            }
            _ => {
                debug!("Unsupported IPv4 protocol: {:?}", ipv4.get_next_level_protocol());
            }
        }
        Ok(())
    }

    async fn process_local_ipv6_packet(ipv6: &Ipv6Packet, stats: &PacketStats) -> Result<()> {
        // IPv6 local processing logic would go here
        debug!("Processing local IPv6 packet");
        Ok(())
    }

    async fn process_tcp_packet(payload: &[u8], stats: &PacketStats) -> Result<()> {
        if let Some(tcp) = TcpPacket::new(payload) {
            debug!("Processing TCP packet: {} -> {}", tcp.get_source(), tcp.get_destination());
            // TCP processing logic would go here
        }
        Ok(())
    }

    async fn process_udp_packet(payload: &[u8], stats: &PacketStats) -> Result<()> {
        if let Some(udp) = UdpPacket::new(payload) {
            debug!("Processing UDP packet: {} -> {}", udp.get_source(), udp.get_destination());
            // UDP processing logic would go here
        }
        Ok(())
    }

    async fn process_icmp_packet(payload: &[u8], stats: &PacketStats) -> Result<()> {
        if let Some(icmp) = IcmpPacket::new(payload) {
            debug!("Processing ICMP packet");
            // ICMP processing logic would go here
        }
        Ok(())
    }

    async fn forward_ipv4_packet(
        ipv4: &Ipv4Packet,
        stats: &PacketStats,
        routing_table: &Arc<RwLock<RoutingTable>>,
    ) -> Result<()> {
        let destination = ipv4.get_destination();
        let table = routing_table.read().await;
        
        if let Some(route) = table.find_route(destination) {
            debug!("Forwarding IPv4 packet to next hop: {}", route.next_hop);
            stats.packets_routed.fetch_add(1, Ordering::Relaxed);
            // Forwarding logic would go here
        } else {
            debug!("No route found for destination: {}", destination);
            stats.packets_dropped.fetch_add(1, Ordering::Relaxed);
        }

        Ok(())
    }

    async fn forward_ipv6_packet(
        ipv6: &Ipv6Packet,
        stats: &PacketStats,
        routing_table: &Arc<RwLock<RoutingTable>>,
    ) -> Result<()> {
        let destination = ipv6.get_destination();
        let table = routing_table.read().await;
        
        if let Some(route) = table.find_route_v6(destination) {
            debug!("Forwarding IPv6 packet to next hop: {}", route.next_hop);
            stats.packets_routed.fetch_add(1, Ordering::Relaxed);
            // Forwarding logic would go here
        } else {
            debug!("No route found for destination: {}", destination);
            stats.packets_dropped.fetch_add(1, Ordering::Relaxed);
        }

        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::config::Config;

    #[tokio::test]
    async fn test_packet_engine_creation() {
        let config = Config::default();
        let routing_table = Arc::new(RwLock::new(RoutingTable::new()));
        let engine = PacketEngine::new(config, routing_table);
        assert!(engine.is_ok());
    }

    #[tokio::test]
    async fn test_packet_stats() {
        let stats = PacketStats::new();
        let statistics = stats.to_statistics();
        assert_eq!(statistics.packets_processed, 0);
        assert_eq!(statistics.bytes_processed, 0);
    }
}