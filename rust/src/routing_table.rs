use std::collections::HashMap;
use std::net::{IpAddr, Ipv4Addr, Ipv6Addr};
use std::sync::Arc;
use std::time::{Duration, Instant};
use tokio::sync::RwLock;
use dashmap::DashMap;
use serde::{Deserialize, Serialize};
use thiserror::Error;
use tracing::{info, warn, error, debug};

use crate::Route;

#[derive(Error, Debug)]
pub enum RoutingTableError {
    #[error("Route not found: {0}")]
    RouteNotFound(String),
    #[error("Invalid route: {0}")]
    InvalidRoute(String),
    #[error("Table full: {0}")]
    TableFull(String),
}

pub struct RoutingTable {
    routes: DashMap<String, Route>,
    route_cache: DashMap<String, String>, // destination -> best route key
    statistics: DashMap<String, u64>,
}

impl RoutingTable {
    pub fn new() -> Self {
        Self {
            routes: DashMap::new(),
            route_cache: DashMap::new(),
            statistics: DashMap::new(),
        }
    }

    pub async fn add_route(&self, route: Route) -> Result<(), RoutingTableError> {
        let key = self.generate_route_key(&route);
        
        // Validate route
        self.validate_route(&route)?;
        
        // Check if route already exists
        if let Some(existing) = self.routes.get(&key) {
            if existing.metric < route.metric {
                // New route has better metric, update
                self.routes.insert(key.clone(), route);
                self.update_route_cache().await;
                self.statistics.entry("route_updates".to_string()).and_modify(|v| *v += 1).or_insert(1);
                debug!("Updated route: {}", key);
            } else {
                // Existing route is better, ignore
                debug!("Ignoring route with worse metric: {}", key);
            }
        } else {
            // New route
            self.routes.insert(key.clone(), route);
            self.update_route_cache().await;
            self.statistics.entry("route_adds".to_string()).and_modify(|v| *v += 1).or_insert(1);
            debug!("Added route: {}", key);
        }
        
        Ok(())
    }

    pub async fn remove_route(&self, destination: &str) -> Result<(), RoutingTableError> {
        let mut removed = false;
        
        // Find and remove all routes for this destination
        self.routes.retain(|key, route| {
            if route.destination == destination {
                removed = true;
                false
            } else {
                true
            }
        });
        
        if removed {
            self.update_route_cache().await;
            self.statistics.entry("route_removes".to_string()).and_modify(|v| *v += 1).or_insert(1);
            debug!("Removed routes for destination: {}", destination);
        } else {
            return Err(RoutingTableError::RouteNotFound(destination.to_string()));
        }
        
        Ok(())
    }

    pub async fn get_route(&self, destination: &str) -> Option<Route> {
        if let Some(route_key) = self.route_cache.get(destination) {
            if let Some(route) = self.routes.get(&*route_key) {
                return Some(route.clone());
            }
        }
        
        // Find best route manually
        self.find_best_route(destination).await
    }

    pub async fn get_all_routes(&self) -> Vec<Route> {
        self.routes.iter().map(|entry| entry.value().clone()).collect()
    }

    pub async fn get_routes_by_protocol(&self, protocol: &str) -> Vec<Route> {
        self.routes
            .iter()
            .filter(|entry| entry.protocol == protocol)
            .map(|entry| entry.value().clone())
            .collect()
    }

    pub async fn get_routes_by_interface(&self, interface: &str) -> Vec<Route> {
        self.routes
            .iter()
            .filter(|entry| entry.interface == interface)
            .map(|entry| entry.value().clone())
            .collect()
    }

    pub fn get_statistics(&self) -> HashMap<String, u64> {
        self.statistics.iter().map(|entry| (entry.key().clone(), *entry.value())).collect()
    }

    pub fn get_route_count(&self) -> usize {
        self.routes.len()
    }

    async fn find_best_route(&self, destination: &str) -> Option<Route> {
        let mut best_route: Option<Route> = None;
        let mut best_metric = u32::MAX;
        
        for entry in self.routes.iter() {
            let route = entry.value();
            
            // Check if route matches destination
            if self.route_matches(route, destination) {
                if route.metric < best_metric {
                    best_metric = route.metric;
                    best_route = Some(route.clone());
                }
            }
        }
        
        // Update cache
        if let Some(ref route) = best_route {
            let key = self.generate_route_key(route);
            self.route_cache.insert(destination.to_string(), key);
        }
        
        best_route
    }

    fn route_matches(&self, route: &Route, destination: &str) -> bool {
        // Simple prefix matching - in real implementation, this would be more sophisticated
        if let (Ok(route_ip), Ok(dest_ip)) = (route.destination.parse::<Ipv4Addr>(), destination.parse::<Ipv4Addr>()) {
            // Extract network from route destination
            let route_network = self.extract_network(&route.destination);
            if let Ok(network) = route_network.parse::<Ipv4Addr>() {
                // Check if destination is in the network
                return self.ip_in_network(dest_ip, network, self.get_network_mask(&route.destination));
            }
        }
        
        // Fallback to string matching
        destination.starts_with(&route.destination)
    }

    fn extract_network(&self, route_dest: &str) -> String {
        // Simple implementation - extract network from CIDR notation
        if let Some(pos) = route_dest.find('/') {
            route_dest[..pos].to_string()
        } else {
            route_dest.to_string()
        }
    }

    fn get_network_mask(&self, route_dest: &str) -> u8 {
        // Extract CIDR prefix length
        if let Some(pos) = route_dest.find('/') {
            route_dest[pos + 1..].parse().unwrap_or(32)
        } else {
            32
        }
    }

    fn ip_in_network(&self, ip: Ipv4Addr, network: Ipv4Addr, prefix_len: u8) -> bool {
        let mask = if prefix_len == 0 {
            0
        } else {
            !((1u32 << (32 - prefix_len)) - 1)
        };
        
        let ip_u32 = u32::from(ip);
        let network_u32 = u32::from(network);
        
        (ip_u32 & mask) == (network_u32 & mask)
    }

    fn generate_route_key(&self, route: &Route) -> String {
        format!("{}:{}:{}", route.destination, route.gateway, route.interface)
    }

    fn validate_route(&self, route: &Route) -> Result<(), RoutingTableError> {
        // Validate destination
        if route.destination.is_empty() {
            return Err(RoutingTableError::InvalidRoute("Empty destination".to_string()));
        }
        
        // Validate gateway
        if route.gateway.is_empty() {
            return Err(RoutingTableError::InvalidRoute("Empty gateway".to_string()));
        }
        
        // Validate interface
        if route.interface.is_empty() {
            return Err(RoutingTableError::InvalidRoute("Empty interface".to_string()));
        }
        
        // Validate metric
        if route.metric > 65535 {
            return Err(RoutingTableError::InvalidRoute("Invalid metric".to_string()));
        }
        
        Ok(())
    }

    async fn update_route_cache(&self) {
        self.route_cache.clear();
        
        // Rebuild cache by finding best routes for each unique destination
        let mut destinations = std::collections::HashSet::new();
        for entry in self.routes.iter() {
            destinations.insert(entry.destination.clone());
        }
        
        for destination in destinations {
            if let Some(route) = self.find_best_route(&destination).await {
                let key = self.generate_route_key(&route);
                self.route_cache.insert(destination, key);
            }
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::time::Duration;

    #[tokio::test]
    async fn test_add_route() {
        let rt = RoutingTable::new();
        let route = Route {
            destination: "192.168.1.0/24".to_string(),
            gateway: "192.168.1.1".to_string(),
            interface: "eth0".to_string(),
            metric: 10,
            protocol: "static".to_string(),
            age: Duration::from_secs(0),
        };
        
        assert!(rt.add_route(route).await.is_ok());
        assert_eq!(rt.get_route_count(), 1);
    }

    #[tokio::test]
    async fn test_remove_route() {
        let rt = RoutingTable::new();
        let route = Route {
            destination: "192.168.1.0/24".to_string(),
            gateway: "192.168.1.1".to_string(),
            interface: "eth0".to_string(),
            metric: 10,
            protocol: "static".to_string(),
            age: Duration::from_secs(0),
        };
        
        rt.add_route(route).await.unwrap();
        assert_eq!(rt.get_route_count(), 1);
        
        rt.remove_route("192.168.1.0/24").await.unwrap();
        assert_eq!(rt.get_route_count(), 0);
    }

    #[tokio::test]
    async fn test_route_validation() {
        let rt = RoutingTable::new();
        let invalid_route = Route {
            destination: "".to_string(),
            gateway: "192.168.1.1".to_string(),
            interface: "eth0".to_string(),
            metric: 10,
            protocol: "static".to_string(),
            age: Duration::from_secs(0),
        };
        
        assert!(rt.add_route(invalid_route).await.is_err());
    }
}