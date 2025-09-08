use std::collections::HashMap;
use std::net::{IpAddr, Ipv4Addr, Ipv6Addr};
use std::str::FromStr;

use crate::{Route, RoutingStats};

/// High-performance routing table implementation
pub struct RoutingTable {
    routes: HashMap<String, Route>,
    protocol_counts: HashMap<String, usize>,
    interface_counts: HashMap<String, usize>,
    last_update: u64,
    total_updates: u64,
}

impl RoutingTable {
    pub fn new() -> Self {
        Self {
            routes: HashMap::new(),
            protocol_counts: HashMap::new(),
            interface_counts: HashMap::new(),
            last_update: 0,
            total_updates: 0,
        }
    }

    /// Add a route to the routing table
    pub fn add_route(&mut self, route: &Route) -> Result<(), String> {
        // Validate route
        self.validate_route(route)?;

        let destination = route.destination.clone();
        let old_route = self.routes.insert(destination.clone(), route.clone());

        // Update statistics
        if old_route.is_none() {
            *self.protocol_counts.entry(route.protocol.clone()).or_insert(0) += 1;
            *self.interface_counts.entry(route.interface.clone()).or_insert(0) += 1;
        }

        self.update_timestamp();
        self.total_updates += 1;

        Ok(())
    }

    /// Remove a route from the routing table
    pub fn remove_route(&mut self, destination: &str) -> Result<(), String> {
        if let Some(route) = self.routes.remove(destination) {
            // Update statistics
            if let Some(count) = self.protocol_counts.get_mut(&route.protocol) {
                *count -= 1;
                if *count == 0 {
                    self.protocol_counts.remove(&route.protocol);
                }
            }

            if let Some(count) = self.interface_counts.get_mut(&route.interface) {
                *count -= 1;
                if *count == 0 {
                    self.interface_counts.remove(&route.interface);
                }
            }

            self.update_timestamp();
            self.total_updates += 1;
        }

        Ok(())
    }

    /// Update an existing route
    pub fn update_route(&mut self, destination: &str, route: &Route) -> Result<(), String> {
        if !self.routes.contains_key(destination) {
            return Err(format!("Route {} not found", destination));
        }

        self.validate_route(route)?;
        self.routes.insert(destination.to_string(), route.clone());
        self.update_timestamp();
        self.total_updates += 1;

        Ok(())
    }

    /// Get a route by destination
    pub fn get_route(&self, destination: &str) -> Option<&Route> {
        self.routes.get(destination)
    }

    /// Get all routes
    pub fn get_all_routes(&self) -> Vec<&Route> {
        self.routes.values().collect()
    }

    /// Get routes by protocol
    pub fn get_routes_by_protocol(&self, protocol: &str) -> Vec<&Route> {
        self.routes.values()
            .filter(|route| route.protocol == protocol)
            .collect()
    }

    /// Get routes by interface
    pub fn get_routes_by_interface(&self, interface: &str) -> Vec<&Route> {
        self.routes.values()
            .filter(|route| route.interface == interface)
            .collect()
    }

    /// Get active routes only
    pub fn get_active_routes(&self) -> Vec<&Route> {
        self.routes.values()
            .filter(|route| route.is_active)
            .collect()
    }

    /// Find the best route for a destination IP
    pub fn find_best_route(&self, dest_ip: &str) -> Option<&Route> {
        let dest_addr = match IpAddr::from_str(dest_ip) {
            Ok(addr) => addr,
            Err(_) => return None,
        };

        let mut best_route: Option<&Route> = None;
        let mut best_metric = u32::MAX;

        for route in self.routes.values() {
            if !route.is_active {
                continue;
            }

            if self.matches_destination(dest_addr, &route.destination) {
                if route.metric < best_metric {
                    best_metric = route.metric;
                    best_route = Some(route);
                }
            }
        }

        best_route
    }

    /// Check if a destination IP matches a route
    fn matches_destination(&self, dest_ip: IpAddr, route_dest: &str) -> bool {
        // Parse route destination (e.g., "192.168.1.0/24")
        let parts: Vec<&str> = route_dest.split('/').collect();
        if parts.len() != 2 {
            return false;
        }

        let network_addr = match IpAddr::from_str(parts[0]) {
            Ok(addr) => addr,
            Err(_) => return false,
        };

        let prefix_len = match parts[1].parse::<u8>() {
            Ok(len) => len,
            Err(_) => return false,
        };

        // Check if IPs are the same family
        match (dest_ip, network_addr) {
            (IpAddr::V4(dest), IpAddr::V4(net)) => {
                self.matches_ipv4_network(dest, net, prefix_len)
            }
            (IpAddr::V6(dest), IpAddr::V6(net)) => {
                self.matches_ipv6_network(dest, net, prefix_len)
            }
            _ => false,
        }
    }

    /// Check if an IPv4 address matches a network
    fn matches_ipv4_network(&self, dest: Ipv4Addr, network: Ipv4Addr, prefix_len: u8) -> bool {
        if prefix_len > 32 {
            return false;
        }

        let mask = if prefix_len == 0 {
            0
        } else {
            !((1u32 << (32 - prefix_len)) - 1)
        };

        let dest_u32 = u32::from(dest);
        let network_u32 = u32::from(network);

        (dest_u32 & mask) == (network_u32 & mask)
    }

    /// Check if an IPv6 address matches a network
    fn matches_ipv6_network(&self, dest: Ipv6Addr, network: Ipv6Addr, prefix_len: u8) -> bool {
        if prefix_len > 128 {
            return false;
        }

        let dest_bytes = dest.octets();
        let network_bytes = network.octets();

        let full_bytes = (prefix_len / 8) as usize;
        let remaining_bits = prefix_len % 8;

        // Check full bytes
        for i in 0..full_bytes {
            if dest_bytes[i] != network_bytes[i] {
                return false;
            }
        }

        // Check remaining bits
        if remaining_bits > 0 && full_bytes < 16 {
            let mask = 0xFF << (8 - remaining_bits);
            if (dest_bytes[full_bytes] & mask) != (network_bytes[full_bytes] & mask) {
                return false;
            }
        }

        true
    }

    /// Validate a route before adding/updating
    fn validate_route(&self, route: &Route) -> Result<(), String> {
        // Validate destination format
        if route.destination.is_empty() {
            return Err("Destination cannot be empty".to_string());
        }

        // Validate gateway format
        if route.gateway.is_empty() {
            return Err("Gateway cannot be empty".to_string());
        }

        // Validate interface
        if route.interface.is_empty() {
            return Err("Interface cannot be empty".to_string());
        }

        // Validate prefix length
        if route.prefix_length > 128 {
            return Err("Prefix length cannot exceed 128".to_string());
        }

        // Validate protocol
        if route.protocol.is_empty() {
            return Err("Protocol cannot be empty".to_string());
        }

        Ok(())
    }

    /// Update timestamp
    fn update_timestamp(&mut self) {
        self.last_update = std::time::SystemTime::now()
            .duration_since(std::time::UNIX_EPOCH)
            .unwrap()
            .as_millis() as u64;
    }

    /// Get routing statistics
    pub fn get_stats(&self) -> RoutingStats {
        let total_routes = self.routes.len();
        let active_routes = self.routes.values().filter(|r| r.is_active).count();
        let bgp_routes = self.protocol_counts.get("bgp").unwrap_or(&0);
        let ospf_routes = self.protocol_counts.get("ospf").unwrap_or(&0);
        let isis_routes = self.protocol_counts.get("isis").unwrap_or(&0);
        let static_routes = self.protocol_counts.get("static").unwrap_or(&0);

        RoutingStats {
            total_routes,
            active_routes,
            bgp_routes: *bgp_routes,
            ospf_routes: *ospf_routes,
            isis_routes: *isis_routes,
            static_routes: *static_routes,
            last_update: self.last_update,
        }
    }

    /// Get protocol distribution
    pub fn get_protocol_distribution(&self) -> HashMap<String, usize> {
        self.protocol_counts.clone()
    }

    /// Get interface distribution
    pub fn get_interface_distribution(&self) -> HashMap<String, usize> {
        self.interface_counts.clone()
    }

    /// Get routes by metric range
    pub fn get_routes_by_metric_range(&self, min_metric: u32, max_metric: u32) -> Vec<&Route> {
        self.routes.values()
            .filter(|route| route.metric >= min_metric && route.metric <= max_metric)
            .collect()
    }

    /// Get routes by prefix length range
    pub fn get_routes_by_prefix_length_range(&self, min_prefix: u8, max_prefix: u8) -> Vec<&Route> {
        self.routes.values()
            .filter(|route| route.prefix_length >= min_prefix && route.prefix_length <= max_prefix)
            .collect()
    }

    /// Clear all routes
    pub fn clear(&mut self) {
        self.routes.clear();
        self.protocol_counts.clear();
        self.interface_counts.clear();
        self.last_update = 0;
        self.total_updates = 0;
    }

    /// Reset statistics
    pub fn reset(&mut self) {
        self.last_update = 0;
        self.total_updates = 0;
    }

    /// Get detailed statistics
    pub fn get_detailed_stats(&self) -> DetailedRoutingStats {
        let basic_stats = self.get_stats();
        let protocol_distribution = self.get_protocol_distribution();
        let interface_distribution = self.get_interface_distribution();

        // Calculate average metric
        let total_metric: u32 = self.routes.values().map(|r| r.metric).sum();
        let average_metric = if self.routes.is_empty() {
            0.0
        } else {
            total_metric as f64 / self.routes.len() as f64
        };

        // Calculate metric distribution
        let mut metric_distribution = HashMap::new();
        for route in self.routes.values() {
            *metric_distribution.entry(route.metric).or_insert(0) += 1;
        }

        DetailedRoutingStats {
            basic_stats,
            protocol_distribution,
            interface_distribution,
            metric_distribution,
            average_metric,
            total_updates: self.total_updates,
        }
    }
}

/// Detailed routing statistics
#[derive(Debug, Clone)]
pub struct DetailedRoutingStats {
    pub basic_stats: RoutingStats,
    pub protocol_distribution: HashMap<String, usize>,
    pub interface_distribution: HashMap<String, usize>,
    pub metric_distribution: HashMap<u32, usize>,
    pub average_metric: f64,
    pub total_updates: u64,
}

impl Default for RoutingTable {
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_routing_table_creation() {
        let table = RoutingTable::new();
        let stats = table.get_stats();
        assert_eq!(stats.total_routes, 0);
        assert_eq!(stats.active_routes, 0);
    }

    #[test]
    fn test_add_route() {
        let mut table = RoutingTable::new();
        let route = Route::new(
            "192.168.1.0/24".to_string(),
            "192.168.1.1".to_string(),
            "eth0".to_string(),
            24,
            1,
            "static".to_string(),
        );

        assert!(table.add_route(&route).is_ok());
        let stats = table.get_stats();
        assert_eq!(stats.total_routes, 1);
        assert_eq!(stats.active_routes, 1);
        assert_eq!(stats.static_routes, 1);
    }

    #[test]
    fn test_remove_route() {
        let mut table = RoutingTable::new();
        let route = Route::new(
            "192.168.1.0/24".to_string(),
            "192.168.1.1".to_string(),
            "eth0".to_string(),
            24,
            1,
            "static".to_string(),
        );

        table.add_route(&route).unwrap();
        assert!(table.remove_route("192.168.1.0/24").is_ok());
        let stats = table.get_stats();
        assert_eq!(stats.total_routes, 0);
    }

    #[test]
    fn test_find_best_route() {
        let mut table = RoutingTable::new();
        
        // Add default route
        let default_route = Route::new(
            "0.0.0.0/0".to_string(),
            "192.168.1.1".to_string(),
            "eth0".to_string(),
            0,
            100,
            "static".to_string(),
        );
        table.add_route(&default_route).unwrap();

        // Add specific route
        let specific_route = Route::new(
            "192.168.1.0/24".to_string(),
            "192.168.1.1".to_string(),
            "eth0".to_string(),
            24,
            1,
            "static".to_string(),
        );
        table.add_route(&specific_route).unwrap();

        // Test route lookup
        let best_route = table.find_best_route("192.168.1.100");
        assert!(best_route.is_some());
        assert_eq!(best_route.unwrap().destination, "192.168.1.0/24");
    }

    #[test]
    fn test_route_validation() {
        let mut table = RoutingTable::new();
        
        // Test invalid route (empty destination)
        let invalid_route = Route::new(
            "".to_string(),
            "192.168.1.1".to_string(),
            "eth0".to_string(),
            24,
            1,
            "static".to_string(),
        );
        assert!(table.add_route(&invalid_route).is_err());

        // Test valid route
        let valid_route = Route::new(
            "192.168.1.0/24".to_string(),
            "192.168.1.1".to_string(),
            "eth0".to_string(),
            24,
            1,
            "static".to_string(),
        );
        assert!(table.add_route(&valid_route).is_ok());
    }

    #[test]
    fn test_ipv4_network_matching() {
        let table = RoutingTable::new();
        
        // Test exact match
        assert!(table.matches_destination(
            IpAddr::V4(Ipv4Addr::new(192, 168, 1, 100)),
            "192.168.1.100/32"
        ));

        // Test network match
        assert!(table.matches_destination(
            IpAddr::V4(Ipv4Addr::new(192, 168, 1, 100)),
            "192.168.1.0/24"
        ));

        // Test no match
        assert!(!table.matches_destination(
            IpAddr::V4(Ipv4Addr::new(192, 168, 2, 100)),
            "192.168.1.0/24"
        ));
    }
}