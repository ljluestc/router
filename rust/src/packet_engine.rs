use std::collections::HashMap;
use std::sync::Mutex;
use std::time::{SystemTime, UNIX_EPOCH};

use crate::{Packet, PacketStats};

/// High-performance packet processing engine
pub struct PacketEngine {
    total_packets: u64,
    total_bytes: u64,
    dropped_packets: u64,
    dropped_bytes: u64,
    last_packet_time: u64,
    start_time: u64,
    packet_counts: HashMap<String, u64>,
    byte_counts: HashMap<String, u64>,
    protocol_counts: HashMap<u8, u64>,
    port_counts: HashMap<u16, u64>,
    source_ip_counts: HashMap<String, u64>,
    dest_ip_counts: HashMap<String, u64>,
}

impl PacketEngine {
    pub fn new() -> Self {
        let start_time = SystemTime::now()
            .duration_since(UNIX_EPOCH)
            .unwrap()
            .as_millis() as u64;

        Self {
            total_packets: 0,
            total_bytes: 0,
            dropped_packets: 0,
            dropped_bytes: 0,
            last_packet_time: 0,
            start_time,
            packet_counts: HashMap::new(),
            byte_counts: HashMap::new(),
            protocol_counts: HashMap::new(),
            port_counts: HashMap::new(),
            source_ip_counts: HashMap::new(),
            dest_ip_counts: HashMap::new(),
        }
    }

    /// Process a packet through the engine
    pub fn process_packet(&mut self, packet: &Packet) -> Result<(), String> {
        let current_time = SystemTime::now()
            .duration_since(UNIX_EPOCH)
            .unwrap()
            .as_millis() as u64;

        // Update basic statistics
        self.total_packets += 1;
        self.total_bytes += packet.size as u64;
        self.last_packet_time = current_time;

        // Update protocol statistics
        *self.protocol_counts.entry(packet.protocol).or_insert(0) += 1;

        // Update port statistics
        *self.port_counts.entry(packet.source_port).or_insert(0) += 1;
        *self.port_counts.entry(packet.dest_port).or_insert(0) += 1;

        // Update IP statistics
        *self.source_ip_counts.entry(packet.source_ip.clone()).or_insert(0) += 1;
        *self.dest_ip_counts.entry(packet.dest_ip.clone()).or_insert(0) += 1;

        // Update interface statistics (simplified)
        let interface_key = format!("{}->{}", packet.source_ip, packet.dest_ip);
        *self.packet_counts.entry(interface_key.clone()).or_insert(0) += 1;
        *self.byte_counts.entry(interface_key).or_insert(0) += packet.size as u64;

        Ok(())
    }

    /// Drop a packet (for traffic shaping, etc.)
    pub fn drop_packet(&mut self, packet: &Packet) {
        self.dropped_packets += 1;
        self.dropped_bytes += packet.size as u64;
    }

    /// Get current statistics
    pub fn get_stats(&self) -> PacketStats {
        let current_time = SystemTime::now()
            .duration_since(UNIX_EPOCH)
            .unwrap()
            .as_millis() as u64;

        let elapsed_seconds = if current_time > self.start_time {
            (current_time - self.start_time) as f64 / 1000.0
        } else {
            1.0
        };

        let packets_per_second = if elapsed_seconds > 0.0 {
            self.total_packets as f64 / elapsed_seconds
        } else {
            0.0
        };

        let bytes_per_second = if elapsed_seconds > 0.0 {
            self.total_bytes as f64 / elapsed_seconds
        } else {
            0.0
        };

        let average_packet_size = if self.total_packets > 0 {
            self.total_bytes as f64 / self.total_packets as f64
        } else {
            0.0
        };

        PacketStats {
            total_packets: self.total_packets,
            total_bytes: self.total_bytes,
            packets_per_second,
            bytes_per_second,
            dropped_packets: self.dropped_packets,
            dropped_bytes: self.dropped_bytes,
            average_packet_size,
            last_packet_time: self.last_packet_time,
        }
    }

    /// Get protocol statistics
    pub fn get_protocol_stats(&self) -> HashMap<u8, u64> {
        self.protocol_counts.clone()
    }

    /// Get port statistics
    pub fn get_port_stats(&self) -> HashMap<u16, u64> {
        self.port_counts.clone()
    }

    /// Get source IP statistics
    pub fn get_source_ip_stats(&self) -> HashMap<String, u64> {
        self.source_ip_counts.clone()
    }

    /// Get destination IP statistics
    pub fn get_dest_ip_stats(&self) -> HashMap<String, u64> {
        self.dest_ip_counts.clone()
    }

    /// Get interface statistics
    pub fn get_interface_stats(&self) -> HashMap<String, (u64, u64)> {
        let mut stats = HashMap::new();
        for (interface, packet_count) in &self.packet_counts {
            let byte_count = self.byte_counts.get(interface).unwrap_or(&0);
            stats.insert(interface.clone(), (*packet_count, *byte_count));
        }
        stats
    }

    /// Get top N protocols by packet count
    pub fn get_top_protocols(&self, n: usize) -> Vec<(u8, u64)> {
        let mut protocols: Vec<(u8, u64)> = self.protocol_counts.iter()
            .map(|(k, v)| (*k, *v))
            .collect();
        protocols.sort_by(|a, b| b.1.cmp(&a.1));
        protocols.truncate(n);
        protocols
    }

    /// Get top N ports by packet count
    pub fn get_top_ports(&self, n: usize) -> Vec<(u16, u64)> {
        let mut ports: Vec<(u16, u64)> = self.port_counts.iter()
            .map(|(k, v)| (*k, *v))
            .collect();
        ports.sort_by(|a, b| b.1.cmp(&a.1));
        ports.truncate(n);
        ports
    }

    /// Get top N source IPs by packet count
    pub fn get_top_source_ips(&self, n: usize) -> Vec<(String, u64)> {
        let mut ips: Vec<(String, u64)> = self.source_ip_counts.iter()
            .map(|(k, v)| (k.clone(), *v))
            .collect();
        ips.sort_by(|a, b| b.1.cmp(&a.1));
        ips.truncate(n);
        ips
    }

    /// Get top N destination IPs by packet count
    pub fn get_top_dest_ips(&self, n: usize) -> Vec<(String, u64)> {
        let mut ips: Vec<(String, u64)> = self.dest_ip_counts.iter()
            .map(|(k, v)| (k.clone(), *v))
            .collect();
        ips.sort_by(|a, b| b.1.cmp(&a.1));
        ips.truncate(n);
        ips
    }

    /// Get packet rate over time window
    pub fn get_packet_rate(&self, window_seconds: u64) -> f64 {
        let current_time = SystemTime::now()
            .duration_since(UNIX_EPOCH)
            .unwrap()
            .as_millis() as u64;

        let window_start = if current_time > window_seconds * 1000 {
            current_time - (window_seconds * 1000)
        } else {
            self.start_time
        };

        // This is a simplified implementation
        // In a real implementation, you would track packets over time windows
        let elapsed_seconds = if current_time > window_start {
            (current_time - window_start) as f64 / 1000.0
        } else {
            1.0
        };

        if elapsed_seconds > 0.0 {
            self.total_packets as f64 / elapsed_seconds
        } else {
            0.0
        }
    }

    /// Get byte rate over time window
    pub fn get_byte_rate(&self, window_seconds: u64) -> f64 {
        let current_time = SystemTime::now()
            .duration_since(UNIX_EPOCH)
            .unwrap()
            .as_millis() as u64;

        let window_start = if current_time > window_seconds * 1000 {
            current_time - (window_seconds * 1000)
        } else {
            self.start_time
        };

        let elapsed_seconds = if current_time > window_start {
            (current_time - window_start) as f64 / 1000.0
        } else {
            1.0
        };

        if elapsed_seconds > 0.0 {
            self.total_bytes as f64 / elapsed_seconds
        } else {
            0.0
        }
    }

    /// Check if packet should be dropped based on rate limiting
    pub fn should_drop_packet(&self, packet: &Packet, max_packets_per_second: u64) -> bool {
        let current_rate = self.get_packet_rate(1);
        current_rate > max_packets_per_second as f64
    }

    /// Check if packet should be dropped based on bandwidth limiting
    pub fn should_drop_packet_bandwidth(&self, packet: &Packet, max_bytes_per_second: u64) -> bool {
        let current_rate = self.get_byte_rate(1);
        current_rate > max_bytes_per_second as f64
    }

    /// Reset all statistics
    pub fn reset(&mut self) {
        self.total_packets = 0;
        self.total_bytes = 0;
        self.dropped_packets = 0;
        self.dropped_bytes = 0;
        self.last_packet_time = 0;
        self.start_time = SystemTime::now()
            .duration_since(UNIX_EPOCH)
            .unwrap()
            .as_millis() as u64;
        self.packet_counts.clear();
        self.byte_counts.clear();
        self.protocol_counts.clear();
        self.port_counts.clear();
        self.source_ip_counts.clear();
        self.dest_ip_counts.clear();
    }

    /// Get detailed statistics for analysis
    pub fn get_detailed_stats(&self) -> DetailedPacketStats {
        let current_time = SystemTime::now()
            .duration_since(UNIX_EPOCH)
            .unwrap()
            .as_millis() as u64;

        let elapsed_seconds = if current_time > self.start_time {
            (current_time - self.start_time) as f64 / 1000.0
        } else {
            1.0
        };

        DetailedPacketStats {
            basic_stats: self.get_stats(),
            protocol_distribution: self.get_protocol_stats(),
            port_distribution: self.get_port_stats(),
            source_ip_distribution: self.get_source_ip_stats(),
            dest_ip_distribution: self.get_dest_ip_stats(),
            interface_stats: self.get_interface_stats(),
            top_protocols: self.get_top_protocols(10),
            top_ports: self.get_top_ports(10),
            top_source_ips: self.get_top_source_ips(10),
            top_dest_ips: self.get_top_dest_ips(10),
            uptime_seconds: elapsed_seconds,
        }
    }
}

/// Detailed packet statistics
#[derive(Debug, Clone)]
pub struct DetailedPacketStats {
    pub basic_stats: PacketStats,
    pub protocol_distribution: HashMap<u8, u64>,
    pub port_distribution: HashMap<u16, u64>,
    pub source_ip_distribution: HashMap<String, u64>,
    pub dest_ip_distribution: HashMap<String, u64>,
    pub interface_stats: HashMap<String, (u64, u64)>,
    pub top_protocols: Vec<(u8, u64)>,
    pub top_ports: Vec<(u16, u64)>,
    pub top_source_ips: Vec<(String, u64)>,
    pub top_dest_ips: Vec<(String, u64)>,
    pub uptime_seconds: f64,
}

impl Default for PacketEngine {
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_packet_engine_creation() {
        let engine = PacketEngine::new();
        let stats = engine.get_stats();
        assert_eq!(stats.total_packets, 0);
        assert_eq!(stats.total_bytes, 0);
    }

    #[test]
    fn test_packet_processing() {
        let mut engine = PacketEngine::new();
        let packet = Packet::new(
            1,
            1500,
            1,
            "192.168.1.1".to_string(),
            "192.168.1.2".to_string(),
            80,
            8080,
            6,
        );

        assert!(engine.process_packet(&packet).is_ok());
        let stats = engine.get_stats();
        assert_eq!(stats.total_packets, 1);
        assert_eq!(stats.total_bytes, 1500);
    }

    #[test]
    fn test_packet_dropping() {
        let mut engine = PacketEngine::new();
        let packet = Packet::new(
            1,
            1500,
            1,
            "192.168.1.1".to_string(),
            "192.168.1.2".to_string(),
            80,
            8080,
            6,
        );

        engine.drop_packet(&packet);
        let stats = engine.get_stats();
        assert_eq!(stats.dropped_packets, 1);
        assert_eq!(stats.dropped_bytes, 1500);
    }

    #[test]
    fn test_protocol_statistics() {
        let mut engine = PacketEngine::new();
        let packet1 = Packet::new(
            1,
            1500,
            1,
            "192.168.1.1".to_string(),
            "192.168.1.2".to_string(),
            80,
            8080,
            6, // TCP
        );
        let packet2 = Packet::new(
            2,
            1000,
            1,
            "192.168.1.1".to_string(),
            "192.168.1.2".to_string(),
            53,
            53,
            17, // UDP
        );

        engine.process_packet(&packet1).unwrap();
        engine.process_packet(&packet2).unwrap();

        let protocol_stats = engine.get_protocol_stats();
        assert_eq!(protocol_stats.get(&6), Some(&1));
        assert_eq!(protocol_stats.get(&17), Some(&1));
    }

    #[test]
    fn test_reset() {
        let mut engine = PacketEngine::new();
        let packet = Packet::new(
            1,
            1500,
            1,
            "192.168.1.1".to_string(),
            "192.168.1.2".to_string(),
            80,
            8080,
            6,
        );

        engine.process_packet(&packet).unwrap();
        engine.reset();

        let stats = engine.get_stats();
        assert_eq!(stats.total_packets, 0);
        assert_eq!(stats.total_bytes, 0);
    }
}