use std::collections::HashMap;
use std::sync::{Arc, Mutex};
use std::time::{SystemTime, UNIX_EPOCH};
use serde::{Deserialize, Serialize};

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Packet {
    pub timestamp: u64,
    pub source_ip: String,
    pub dest_ip: String,
    pub source_port: u16,
    pub dest_port: u16,
    pub protocol: u8,
    pub size: u32,
    pub interface: String,
    pub router_id: String,
    pub flow_id: String,
    pub priority: u8,
    pub dscp: u8,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct FlowStats {
    pub flow_id: String,
    pub packets: u64,
    pub bytes: u64,
    pub start_time: u64,
    pub last_seen: u64,
    pub source_ip: String,
    pub dest_ip: String,
    pub protocol: u8,
    pub interface: String,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct InterfaceStats {
    pub interface: String,
    pub packets_in: u64,
    pub packets_out: u64,
    pub bytes_in: u64,
    pub bytes_out: u64,
    pub drops: u64,
    pub errors: u64,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct RouterStats {
    pub router_id: String,
    pub total_packets: u64,
    pub total_bytes: u64,
    pub total_drops: u64,
    pub total_errors: u64,
    pub active_flows: u32,
    pub interfaces: Vec<InterfaceStats>,
}

pub struct PacketEngine {
    flows: Arc<Mutex<HashMap<String, FlowStats>>>,
    interfaces: Arc<Mutex<HashMap<String, InterfaceStats>>>,
    router_stats: Arc<Mutex<RouterStats>>,
    flow_timeout: u64,
}

impl PacketEngine {
    pub fn new(router_id: String, flow_timeout: u64) -> Self {
        Self {
            flows: Arc::new(Mutex::new(HashMap::new())),
            interfaces: Arc::new(Mutex::new(HashMap::new())),
            router_stats: Arc::new(Mutex::new(RouterStats {
                router_id,
                total_packets: 0,
                total_bytes: 0,
                total_drops: 0,
                total_errors: 0,
                active_flows: 0,
                interfaces: Vec::new(),
            })),
            flow_timeout,
        }
    }

    pub fn process_packet(&self, packet: Packet) -> Result<(), String> {
        let current_time = SystemTime::now()
            .duration_since(UNIX_EPOCH)
            .unwrap()
            .as_secs();

        // Update flow statistics
        self.update_flow_stats(&packet, current_time)?;

        // Update interface statistics
        self.update_interface_stats(&packet)?;

        // Update router statistics
        self.update_router_stats(&packet)?;

        // Clean up expired flows
        self.cleanup_expired_flows(current_time);

        Ok(())
    }

    fn update_flow_stats(&self, packet: &Packet, current_time: u64) -> Result<(), String> {
        let mut flows = self.flows.lock().map_err(|_| "Failed to lock flows")?;
        
        let flow_entry = flows.entry(packet.flow_id.clone()).or_insert(FlowStats {
            flow_id: packet.flow_id.clone(),
            packets: 0,
            bytes: 0,
            start_time: current_time,
            last_seen: current_time,
            source_ip: packet.source_ip.clone(),
            dest_ip: packet.dest_ip.clone(),
            protocol: packet.protocol,
            interface: packet.interface.clone(),
        });

        flow_entry.packets += 1;
        flow_entry.bytes += packet.size as u64;
        flow_entry.last_seen = current_time;

        Ok(())
    }

    fn update_interface_stats(&self, packet: &Packet) -> Result<(), String> {
        let mut interfaces = self.interfaces.lock().map_err(|_| "Failed to lock interfaces")?;
        
        let interface_entry = interfaces.entry(packet.interface.clone()).or_insert(InterfaceStats {
            interface: packet.interface.clone(),
            packets_in: 0,
            packets_out: 0,
            bytes_in: 0,
            bytes_out: 0,
            drops: 0,
            errors: 0,
        });

        // Determine if packet is incoming or outgoing based on source IP
        // This is a simplified heuristic - in reality, you'd check routing tables
        if packet.source_ip.starts_with("192.168.") || packet.source_ip.starts_with("10.") {
            interface_entry.packets_in += 1;
            interface_entry.bytes_in += packet.size as u64;
        } else {
            interface_entry.packets_out += 1;
            interface_entry.bytes_out += packet.size as u64;
        }

        Ok(())
    }

    fn update_router_stats(&self, packet: &Packet) -> Result<(), String> {
        let mut router_stats = self.router_stats.lock().map_err(|_| "Failed to lock router stats")?;
        
        router_stats.total_packets += 1;
        router_stats.total_bytes += packet.size as u64;

        // Update active flows count
        let flows = self.flows.lock().map_err(|_| "Failed to lock flows")?;
        router_stats.active_flows = flows.len() as u32;

        // Update interface list
        let interfaces = self.interfaces.lock().map_err(|_| "Failed to lock interfaces")?;
        router_stats.interfaces = interfaces.values().cloned().collect();

        Ok(())
    }

    fn cleanup_expired_flows(&self, current_time: u64) {
        let mut flows = match self.flows.lock() {
            Ok(flows) => flows,
            Err(_) => return,
        };

        flows.retain(|_, flow| {
            current_time - flow.last_seen < self.flow_timeout
        });
    }

    pub fn get_flow_stats(&self, flow_id: &str) -> Option<FlowStats> {
        let flows = self.flows.lock().ok()?;
        flows.get(flow_id).cloned()
    }

    pub fn get_all_flows(&self) -> Vec<FlowStats> {
        let flows = self.flows.lock().unwrap_or_else(|_| std::sync::Mutex::new(HashMap::new()).lock().unwrap());
        flows.values().cloned().collect()
    }

    pub fn get_interface_stats(&self, interface: &str) -> Option<InterfaceStats> {
        let interfaces = self.interfaces.lock().ok()?;
        interfaces.get(interface).cloned()
    }

    pub fn get_all_interface_stats(&self) -> Vec<InterfaceStats> {
        let interfaces = self.interfaces.lock().unwrap_or_else(|_| std::sync::Mutex::new(HashMap::new()).lock().unwrap());
        interfaces.values().cloned().collect()
    }

    pub fn get_router_stats(&self) -> RouterStats {
        let router_stats = self.router_stats.lock().unwrap_or_else(|_| std::sync::Mutex::new(RouterStats {
            router_id: "unknown".to_string(),
            total_packets: 0,
            total_bytes: 0,
            total_drops: 0,
            total_errors: 0,
            active_flows: 0,
            interfaces: Vec::new(),
        }));
        router_stats.clone()
    }

    pub fn get_top_flows(&self, limit: usize) -> Vec<FlowStats> {
        let mut flows: Vec<FlowStats> = self.get_all_flows();
        flows.sort_by(|a, b| b.bytes.cmp(&a.bytes));
        flows.truncate(limit);
        flows
    }

    pub fn get_flows_by_interface(&self, interface: &str) -> Vec<FlowStats> {
        self.get_all_flows()
            .into_iter()
            .filter(|flow| flow.interface == interface)
            .collect()
    }

    pub fn get_flows_by_protocol(&self, protocol: u8) -> Vec<FlowStats> {
        self.get_all_flows()
            .into_iter()
            .filter(|flow| flow.protocol == protocol)
            .collect()
    }

    pub fn reset_stats(&self) {
        if let Ok(mut flows) = self.flows.lock() {
            flows.clear();
        }
        if let Ok(mut interfaces) = self.interfaces.lock() {
            interfaces.clear();
        }
        if let Ok(mut router_stats) = self.router_stats.lock() {
            router_stats.total_packets = 0;
            router_stats.total_bytes = 0;
            router_stats.total_drops = 0;
            router_stats.total_errors = 0;
            router_stats.active_flows = 0;
            router_stats.interfaces.clear();
        }
    }

    pub fn generate_flow_id(&self, packet: &Packet) -> String {
        // Generate a flow ID based on 5-tuple
        format!("{}-{}-{}-{}-{}", 
                packet.source_ip, 
                packet.dest_ip, 
                packet.source_port, 
                packet.dest_port, 
                packet.protocol)
    }

    pub fn classify_packet(&self, packet: &Packet) -> String {
        // Simple packet classification based on protocol and ports
        match packet.protocol {
            1 => "ICMP".to_string(),
            6 => {
                match packet.dest_port {
                    22 => "SSH".to_string(),
                    23 => "Telnet".to_string(),
                    25 => "SMTP".to_string(),
                    53 => "DNS".to_string(),
                    80 => "HTTP".to_string(),
                    443 => "HTTPS".to_string(),
                    993 => "IMAPS".to_string(),
                    995 => "POP3S".to_string(),
                    _ => "TCP".to_string(),
                }
            },
            17 => {
                match packet.dest_port {
                    53 => "DNS".to_string(),
                    67 => "DHCP".to_string(),
                    68 => "DHCP".to_string(),
                    123 => "NTP".to_string(),
                    161 => "SNMP".to_string(),
                    162 => "SNMP".to_string(),
                    _ => "UDP".to_string(),
                }
            },
            47 => "GRE".to_string(),
            50 => "ESP".to_string(),
            51 => "AH".to_string(),
            89 => "OSPF".to_string(),
            _ => "Unknown".to_string(),
        }
    }

    pub fn calculate_bandwidth_utilization(&self, interface: &str, duration_seconds: u64) -> f64 {
        let interfaces = self.interfaces.lock().unwrap_or_else(|_| std::sync::Mutex::new(HashMap::new()).lock().unwrap());
        
        if let Some(interface_stats) = interfaces.get(interface) {
            let total_bytes = interface_stats.bytes_in + interface_stats.bytes_out;
            let bits_per_second = (total_bytes * 8) as f64 / duration_seconds as f64;
            bits_per_second / 1_000_000.0 // Convert to Mbps
        } else {
            0.0
        }
    }

    pub fn get_protocol_distribution(&self) -> HashMap<u8, u64> {
        let mut distribution = HashMap::new();
        let flows = self.flows.lock().unwrap_or_else(|_| std::sync::Mutex::new(HashMap::new()).lock().unwrap());
        
        for flow in flows.values() {
            *distribution.entry(flow.protocol).or_insert(0) += flow.packets;
        }
        
        distribution
    }

    pub fn get_interface_utilization(&self) -> HashMap<String, f64> {
        let mut utilization = HashMap::new();
        let interfaces = self.interfaces.lock().unwrap_or_else(|_| std::sync::Mutex::new(HashMap::new()).lock().unwrap());
        
        for (interface, stats) in interfaces.iter() {
            let total_bytes = stats.bytes_in + stats.bytes_out;
            let utilization_percent = if stats.packets_in + stats.packets_out > 0 {
                (total_bytes as f64 / (stats.packets_in + stats.packets_out) as f64) * 100.0
            } else {
                0.0
            };
            utilization.insert(interface.clone(), utilization_percent);
        }
        
        utilization
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_packet_processing() {
        let engine = PacketEngine::new("router1".to_string(), 300);
        
        let packet = Packet {
            timestamp: 1234567890,
            source_ip: "192.168.1.1".to_string(),
            dest_ip: "192.168.1.2".to_string(),
            source_port: 12345,
            dest_port: 80,
            protocol: 6,
            size: 1500,
            interface: "eth0".to_string(),
            router_id: "router1".to_string(),
            flow_id: "flow1".to_string(),
            priority: 0,
            dscp: 0,
        };

        assert!(engine.process_packet(packet).is_ok());
        
        let stats = engine.get_router_stats();
        assert_eq!(stats.total_packets, 1);
        assert_eq!(stats.total_bytes, 1500);
    }

    #[test]
    fn test_flow_classification() {
        let engine = PacketEngine::new("router1".to_string(), 300);
        
        let packet = Packet {
            timestamp: 1234567890,
            source_ip: "192.168.1.1".to_string(),
            dest_ip: "192.168.1.2".to_string(),
            source_port: 12345,
            dest_port: 80,
            protocol: 6,
            size: 1500,
            interface: "eth0".to_string(),
            router_id: "router1".to_string(),
            flow_id: "flow1".to_string(),
            priority: 0,
            dscp: 0,
        };

        let classification = engine.classify_packet(&packet);
        assert_eq!(classification, "HTTP");
    }
}