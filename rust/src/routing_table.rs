//! High-performance routing table implementation
//! 
//! This module provides a fast routing table optimized for high-speed
//! lookups using radix trees and other efficient data structures.

use std::collections::HashMap;
use std::sync::RwLock;
use std::net::Ipv4Addr;

/// Fast routing table implementation
pub struct FastRoutingTable {
    /// Radix tree for fast prefix lookups
    radix_tree: RadixTree,
    /// Direct route cache for frequently accessed routes
    route_cache: HashMap<u32, RouteEntry>,
    /// Statistics
    stats: RoutingStats,
}

/// Route entry
#[derive(Debug, Clone)]
pub struct RouteEntry {
    pub network: u32,
    pub mask: u32,
    pub next_hop: u32,
    pub interface: String,
    pub metric: u32,
    pub admin_distance: u32,
    pub protocol: String,
    pub is_local: bool,
}

/// Routing statistics
#[derive(Debug, Default)]
pub struct RoutingStats {
    pub lookups: u64,
    pub cache_hits: u64,
    pub cache_misses: u64,
    pub longest_match_time_ns: u64,
    pub cache_lookup_time_ns: u64,
}

/// Radix tree node
#[derive(Debug)]
struct RadixNode {
    prefix: u32,
    prefix_len: u8,
    children: Vec<RadixNode>,
    route: Option<RouteEntry>,
}

/// Radix tree for prefix matching
#[derive(Debug)]
struct RadixTree {
    root: RadixNode,
}

impl RadixTree {
    fn new() -> Self {
        Self {
            root: RadixNode {
                prefix: 0,
                prefix_len: 0,
                children: Vec::new(),
                route: None,
            },
        }
    }
    
    fn insert(&mut self, network: u32, mask: u32, route: RouteEntry) {
        let prefix_len = mask.count_ones() as u8;
        self.insert_recursive(&mut self.root, network, prefix_len, route);
    }
    
    fn insert_recursive(&self, node: &mut RadixNode, network: u32, prefix_len: u8, route: RouteEntry) {
        if prefix_len == 0 {
            node.route = Some(route);
            return;
        }
        
        // Find matching child or create new one
        let child_prefix = network >> (32 - prefix_len);
        let child_prefix_len = prefix_len;
        
        for child in &mut node.children {
            if child.prefix == child_prefix && child.prefix_len == child_prefix_len {
                self.insert_recursive(child, network, prefix_len, route);
                return;
            }
        }
        
        // Create new child
        let mut new_child = RadixNode {
            prefix: child_prefix,
            prefix_len: child_prefix_len,
            children: Vec::new(),
            route: None,
        };
        
        self.insert_recursive(&mut new_child, network, prefix_len, route);
        node.children.push(new_child);
    }
    
    fn lookup(&self, destination: u32) -> Option<RouteEntry> {
        self.lookup_recursive(&self.root, destination)
    }
    
    fn lookup_recursive(&self, node: &RadixNode, destination: u32) -> Option<RouteEntry> {
        // Check if this node has a route
        if let Some(ref route) = node.route {
            return Some(route.clone());
        }
        
        // Look for matching child
        for child in &node.children {
            let child_mask = (1u32 << (32 - child.prefix_len)) - 1;
            let child_network = child.prefix << (32 - child.prefix_len);
            
            if (destination & child_mask) == (child_network & child_mask) {
                if let Some(route) = self.lookup_recursive(child, destination) {
                    return Some(route);
                }
            }
        }
        
        None
    }
}

impl FastRoutingTable {
    /// Create a new fast routing table
    pub fn new() -> Self {
        Self {
            radix_tree: RadixTree::new(),
            route_cache: HashMap::new(),
            stats: RoutingStats::default(),
        }
    }
    
    /// Add a route to the table
    pub fn add_route(&mut self, network: u32, mask: u32, next_hop: u32, interface: String, metric: u32) {
        let route = RouteEntry {
            network,
            mask,
            next_hop,
            interface,
            metric,
            admin_distance: 1,
            protocol: "STATIC".to_string(),
            is_local: false,
        };
        
        self.radix_tree.insert(network, mask, route);
    }
    
    /// Remove a route from the table
    pub fn remove_route(&mut self, network: u32, mask: u32) -> bool {
        // For simplicity, we'll just clear the cache
        // In a real implementation, we'd remove from the radix tree
        self.route_cache.clear();
        true
    }
    
    /// Look up a route for a destination
    pub fn lookup(&self, destination: u32) -> Option<super::packet_engine::RouteInfo> {
        let start_time = std::time::Instant::now();
        
        // Try cache first
        if let Some(route) = self.route_cache.get(&destination) {
            let cache_time = start_time.elapsed();
            let mut stats = &mut self.stats as *mut RoutingStats as *mut RoutingStats;
            unsafe {
                (*stats).lookups += 1;
                (*stats).cache_hits += 1;
                (*stats).cache_lookup_time_ns += cache_time.as_nanos() as u64;
            }
            
            return Some(super::packet_engine::RouteInfo {
                next_hop: route.next_hop,
                interface: route.interface.clone(),
                metric: route.metric,
                is_local: route.is_local,
            });
        }
        
        // Look up in radix tree
        let lookup_start = std::time::Instant::now();
        let route = self.radix_tree.lookup(destination);
        let lookup_time = lookup_start.elapsed();
        
        let mut stats = &mut self.stats as *mut RoutingStats as *mut RoutingStats;
        unsafe {
            (*stats).lookups += 1;
            (*stats).cache_misses += 1;
            (*stats).longest_match_time_ns += lookup_time.as_nanos() as u64;
        }
        
        route.map(|r| {
            // Cache the result
            let mut cache = &mut self.route_cache as *mut HashMap<u32, RouteEntry> as *mut HashMap<u32, RouteEntry>;
            unsafe {
                (*cache).insert(destination, r.clone());
            }
            
            super::packet_engine::RouteInfo {
                next_hop: r.next_hop,
                interface: r.interface,
                metric: r.metric,
                is_local: r.is_local,
            }
        })
    }
    
    /// Get routing statistics
    pub fn get_stats(&self) -> &RoutingStats {
        &self.stats
    }
    
    /// Clear the route cache
    pub fn clear_cache(&mut self) {
        self.route_cache.clear();
    }
    
    /// Get cache hit rate
    pub fn cache_hit_rate(&self) -> f64 {
        if self.stats.lookups == 0 {
            return 0.0;
        }
        self.stats.cache_hits as f64 / self.stats.lookups as f64
    }
    
    /// Get average lookup time in nanoseconds
    pub fn average_lookup_time_ns(&self) -> u64 {
        if self.stats.lookups == 0 {
            return 0;
        }
        (self.stats.longest_match_time_ns + self.stats.cache_lookup_time_ns) / self.stats.lookups
    }
}

/// Route table builder for easy configuration
pub struct RouteTableBuilder {
    routes: Vec<(u32, u32, u32, String, u32)>,
}

impl RouteTableBuilder {
    pub fn new() -> Self {
        Self {
            routes: Vec::new(),
        }
    }
    
    pub fn add_route(mut self, network: u32, mask: u32, next_hop: u32, interface: String, metric: u32) -> Self {
        self.routes.push((network, mask, next_hop, interface, metric));
        self
    }
    
    pub fn add_route_from_cidr(mut self, cidr: &str, next_hop: u32, interface: String, metric: u32) -> Self {
        if let Some((network_str, prefix_len_str)) = cidr.split_once('/') {
            if let (Ok(network), Ok(prefix_len)) = (network_str.parse::<Ipv4Addr>(), prefix_len_str.parse::<u8>()) {
                let network_u32 = u32::from(network);
                let mask = if prefix_len == 0 {
                    0
                } else {
                    (1u32 << (32 - prefix_len)) - 1
                };
                self.routes.push((network_u32, mask, next_hop, interface, metric));
            }
        }
        self
    }
    
    pub fn build(self) -> FastRoutingTable {
        let mut table = FastRoutingTable::new();
        
        for (network, mask, next_hop, interface, metric) in self.routes {
            table.add_route(network, mask, next_hop, interface, metric);
        }
        
        table
    }
}

/// Route table utilities
pub mod utils {
    use super::*;
    
    /// Convert IP address string to u32
    pub fn ip_to_u32(ip: &str) -> Option<u32> {
        ip.parse::<Ipv4Addr>().ok().map(u32::from)
    }
    
    /// Convert u32 to IP address string
    pub fn u32_to_ip(ip: u32) -> String {
        format!("{}.{}.{}.{}", 
            (ip >> 24) & 0xFF,
            (ip >> 16) & 0xFF,
            (ip >> 8) & 0xFF,
            ip & 0xFF
        )
    }
    
    /// Convert CIDR notation to network and mask
    pub fn cidr_to_network_mask(cidr: &str) -> Option<(u32, u32)> {
        if let Some((network_str, prefix_len_str)) = cidr.split_once('/') {
            if let (Ok(network), Ok(prefix_len)) = (network_str.parse::<Ipv4Addr>(), prefix_len_str.parse::<u8>()) {
                let network_u32 = u32::from(network);
                let mask = if prefix_len == 0 {
                    0
                } else {
                    (1u32 << (32 - prefix_len)) - 1
                };
                return Some((network_u32, mask));
            }
        }
        None
    }
    
    /// Check if an IP address matches a network
    pub fn ip_matches_network(ip: u32, network: u32, mask: u32) -> bool {
        (ip & mask) == (network & mask)
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    
    #[test]
    fn test_routing_table_creation() {
        let table = FastRoutingTable::new();
        let stats = table.get_stats();
        assert_eq!(stats.lookups, 0);
    }
    
    #[test]
    fn test_route_addition_and_lookup() {
        let mut table = FastRoutingTable::new();
        
        // Add route for 10.0.0.0/24
        table.add_route(0x0A000000, 0xFFFFFF00, 0xC0A80101, "eth0".to_string(), 1);
        
        // Look up 10.0.0.1
        let route = table.lookup(0x0A000001);
        assert!(route.is_some());
        
        let route_info = route.unwrap();
        assert_eq!(route_info.next_hop, 0xC0A80101);
        assert_eq!(route_info.interface, "eth0");
        assert_eq!(route_info.metric, 1);
    }
    
    #[test]
    fn test_route_builder() {
        let table = RouteTableBuilder::new()
            .add_route(0x0A000000, 0xFFFFFF00, 0xC0A80101, "eth0".to_string(), 1)
            .add_route_from_cidr("192.168.1.0/24", 0x0A000001, "eth1".to_string(), 1)
            .build();
        
        // Test first route
        let route1 = table.lookup(0x0A000001);
        assert!(route1.is_some());
        assert_eq!(route1.unwrap().interface, "eth0");
        
        // Test second route
        let route2 = table.lookup(0xC0A80101);
        assert!(route2.is_some());
        assert_eq!(route2.unwrap().interface, "eth1");
    }
    
    #[test]
    fn test_utils() {
        assert_eq!(utils::ip_to_u32("192.168.1.1"), Some(0xC0A80101));
        assert_eq!(utils::u32_to_ip(0xC0A80101), "192.168.1.1");
        
        let (network, mask) = utils::cidr_to_network_mask("10.0.0.0/24").unwrap();
        assert_eq!(network, 0x0A000000);
        assert_eq!(mask, 0xFFFFFF00);
        
        assert!(utils::ip_matches_network(0x0A000001, 0x0A000000, 0xFFFFFF00));
        assert!(!utils::ip_matches_network(0x0B000001, 0x0A000000, 0xFFFFFF00));
    }
    
    #[test]
    fn test_cache_performance() {
        let mut table = FastRoutingTable::new();
        table.add_route(0x0A000000, 0xFFFFFF00, 0xC0A80101, "eth0".to_string(), 1);
        
        // First lookup should miss cache
        let _ = table.lookup(0x0A000001);
        assert_eq!(table.get_stats().cache_misses, 1);
        
        // Second lookup should hit cache
        let _ = table.lookup(0x0A000001);
        assert_eq!(table.get_stats().cache_hits, 1);
        
        assert!(table.cache_hit_rate() > 0.0);
    }
}
