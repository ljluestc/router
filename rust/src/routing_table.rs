use std::collections::HashMap;
use std::time::{Duration, SystemTime, UNIX_EPOCH};
use anyhow::Result;
use tracing::{debug, error};
use crate::{RoutingUpdate, RoutingStats};

pub struct RoutingTable {
    routes: HashMap<String, RouteEntry>,
    stats: RoutingStats,
}

#[derive(Debug, Clone)]
struct RouteEntry {
    prefix: String,
    next_hop: String,
    protocol: String,
    metric: u32,
    as_path: Vec<u32>,
    community: Vec<String>,
    last_update: u64,
}

impl RoutingTable {
    pub fn new() -> Self {
        Self {
            routes: HashMap::new(),
            stats: RoutingStats {
                total_routes: 0,
                bgp_routes: 0,
                ospf_routes: 0,
                isis_routes: 0,
                convergence_time: Duration::from_millis(0),
                last_update: 0,
            },
        }
    }

    pub async fn update(&mut self, update: RoutingUpdate) -> Result<()> {
        let timestamp = SystemTime::now()
            .duration_since(UNIX_EPOCH)
            .unwrap()
            .as_millis() as u64;

        let entry = RouteEntry {
            prefix: update.prefix.clone(),
            next_hop: update.next_hop.clone(),
            protocol: update.protocol.clone(),
            metric: update.metric,
            as_path: update.as_path.clone(),
            community: update.community.clone(),
            last_update: timestamp,
        };

        self.routes.insert(update.prefix, entry);
        self.update_stats();
        self.stats.last_update = timestamp;

        debug!("Updated route: {} via {}", update.prefix, update.next_hop);
        Ok(())
    }

    fn update_stats(&mut self) {
        self.stats.total_routes = self.routes.len() as u32;
        self.stats.bgp_routes = self.routes.values()
            .filter(|r| r.protocol == "BGP")
            .count() as u32;
        self.stats.ospf_routes = self.routes.values()
            .filter(|r| r.protocol == "OSPF")
            .count() as u32;
        self.stats.isis_routes = self.routes.values()
            .filter(|r| r.protocol == "ISIS")
            .count() as u32;
    }

    pub async fn get_stats(&self) -> RoutingStats {
        self.stats.clone()
    }

    pub async fn lookup(&self, prefix: &str) -> Option<RouteEntry> {
        self.routes.get(prefix).cloned()
    }

    pub async fn get_routes_by_protocol(&self, protocol: &str) -> Vec<RouteEntry> {
        self.routes.values()
            .filter(|r| r.protocol == protocol)
            .cloned()
            .collect()
    }
}