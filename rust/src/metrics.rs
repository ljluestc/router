use std::sync::atomic::{AtomicU64, Ordering};
use std::time::{Duration, Instant};
use serde::{Serialize, Deserialize};

/// Performance metrics collector
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Metrics {
    pub packets_processed: u64,
    pub bytes_processed: u64,
    pub packets_dropped: u64,
    pub packets_forwarded: u64,
    pub packets_routed: u64,
    pub errors: u64,
    pub uptime_seconds: u64,
    pub packets_per_second: f64,
    pub bytes_per_second: f64,
    pub average_latency_ns: u64,
    pub max_latency_ns: u64,
    pub min_latency_ns: u64,
    pub cpu_usage: f64,
    pub memory_usage: f64,
    pub routing_table_size: u64,
    pub active_neighbors: u32,
    pub active_interfaces: u32,
}

impl Default for Metrics {
    fn default() -> Self {
        Self {
            packets_processed: 0,
            bytes_processed: 0,
            packets_dropped: 0,
            packets_forwarded: 0,
            packets_routed: 0,
            errors: 0,
            uptime_seconds: 0,
            packets_per_second: 0.0,
            bytes_per_second: 0.0,
            average_latency_ns: 0,
            max_latency_ns: 0,
            min_latency_ns: 0,
            cpu_usage: 0.0,
            memory_usage: 0.0,
            routing_table_size: 0,
            active_neighbors: 0,
            active_interfaces: 0,
        }
    }
}

/// Thread-safe metrics collector
pub struct MetricsCollector {
    packets_processed: AtomicU64,
    bytes_processed: AtomicU64,
    packets_dropped: AtomicU64,
    packets_forwarded: AtomicU64,
    packets_routed: AtomicU64,
    errors: AtomicU64,
    start_time: Instant,
    latency_sum: AtomicU64,
    latency_count: AtomicU64,
    max_latency: AtomicU64,
    min_latency: AtomicU64,
}

impl MetricsCollector {
    /// Create a new metrics collector
    pub fn new() -> Self {
        Self {
            packets_processed: AtomicU64::new(0),
            bytes_processed: AtomicU64::new(0),
            packets_dropped: AtomicU64::new(0),
            packets_forwarded: AtomicU64::new(0),
            packets_routed: AtomicU64::new(0),
            errors: AtomicU64::new(0),
            start_time: Instant::now(),
            latency_sum: AtomicU64::new(0),
            latency_count: AtomicU64::new(0),
            max_latency: AtomicU64::new(0),
            min_latency: AtomicU64::new(0),
        }
    }

    /// Record a processed packet
    pub fn record_packet_processed(&self, size: u64) {
        self.packets_processed.fetch_add(1, Ordering::Relaxed);
        self.bytes_processed.fetch_add(size, Ordering::Relaxed);
    }

    /// Record a dropped packet
    pub fn record_packet_dropped(&self) {
        self.packets_dropped.fetch_add(1, Ordering::Relaxed);
    }

    /// Record a forwarded packet
    pub fn record_packet_forwarded(&self) {
        self.packets_forwarded.fetch_add(1, Ordering::Relaxed);
    }

    /// Record a routed packet
    pub fn record_packet_routed(&self) {
        self.packets_routed.fetch_add(1, Ordering::Relaxed);
    }

    /// Record an error
    pub fn record_error(&self) {
        self.errors.fetch_add(1, Ordering::Relaxed);
    }

    /// Record processing latency
    pub fn record_latency(&self, latency_ns: u64) {
        self.latency_sum.fetch_add(latency_ns, Ordering::Relaxed);
        self.latency_count.fetch_add(1, Ordering::Relaxed);

        // Update max latency
        loop {
            let current_max = self.max_latency.load(Ordering::Relaxed);
            if latency_ns <= current_max {
                break;
            }
            if self.max_latency.compare_exchange_weak(
                current_max,
                latency_ns,
                Ordering::Relaxed,
                Ordering::Relaxed,
            ).is_ok() {
                break;
            }
        }

        // Update min latency
        loop {
            let current_min = self.min_latency.load(Ordering::Relaxed);
            if current_min == 0 || latency_ns >= current_min {
                break;
            }
            if self.min_latency.compare_exchange_weak(
                current_min,
                latency_ns,
                Ordering::Relaxed,
                Ordering::Relaxed,
            ).is_ok() {
                break;
            }
        }
    }

    /// Get current metrics
    pub fn get_metrics(&self) -> Metrics {
        let uptime = self.start_time.elapsed();
        let uptime_seconds = uptime.as_secs();
        let uptime_f64 = uptime.as_secs_f64();

        let packets_processed = self.packets_processed.load(Ordering::Relaxed);
        let bytes_processed = self.bytes_processed.load(Ordering::Relaxed);
        let packets_dropped = self.packets_dropped.load(Ordering::Relaxed);
        let packets_forwarded = self.packets_forwarded.load(Ordering::Relaxed);
        let packets_routed = self.packets_routed.load(Ordering::Relaxed);
        let errors = self.errors.load(Ordering::Relaxed);

        let packets_per_second = if uptime_f64 > 0.0 {
            packets_processed as f64 / uptime_f64
        } else {
            0.0
        };

        let bytes_per_second = if uptime_f64 > 0.0 {
            bytes_processed as f64 / uptime_f64
        } else {
            0.0
        };

        let latency_sum = self.latency_sum.load(Ordering::Relaxed);
        let latency_count = self.latency_count.load(Ordering::Relaxed);
        let average_latency_ns = if latency_count > 0 {
            latency_sum / latency_count
        } else {
            0
        };

        let max_latency_ns = self.max_latency.load(Ordering::Relaxed);
        let min_latency_ns = self.min_latency.load(Ordering::Relaxed);

        Metrics {
            packets_processed,
            bytes_processed,
            packets_dropped,
            packets_forwarded,
            packets_routed,
            errors,
            uptime_seconds,
            packets_per_second,
            bytes_per_second,
            average_latency_ns,
            max_latency_ns,
            min_latency_ns,
            cpu_usage: 0.0, // Would be populated by system monitoring
            memory_usage: 0.0, // Would be populated by system monitoring
            routing_table_size: 0, // Would be populated by routing table
            active_neighbors: 0, // Would be populated by protocol handlers
            active_interfaces: 0, // Would be populated by interface manager
        }
    }

    /// Reset all metrics
    pub fn reset(&self) {
        self.packets_processed.store(0, Ordering::Relaxed);
        self.bytes_processed.store(0, Ordering::Relaxed);
        self.packets_dropped.store(0, Ordering::Relaxed);
        self.packets_forwarded.store(0, Ordering::Relaxed);
        self.packets_routed.store(0, Ordering::Relaxed);
        self.errors.store(0, Ordering::Relaxed);
        self.latency_sum.store(0, Ordering::Relaxed);
        self.latency_count.store(0, Ordering::Relaxed);
        self.max_latency.store(0, Ordering::Relaxed);
        self.min_latency.store(0, Ordering::Relaxed);
    }
}

impl Default for MetricsCollector {
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_metrics_collector_creation() {
        let collector = MetricsCollector::new();
        let metrics = collector.get_metrics();
        assert_eq!(metrics.packets_processed, 0);
        assert_eq!(metrics.bytes_processed, 0);
    }

    #[test]
    fn test_metrics_recording() {
        let collector = MetricsCollector::new();
        
        collector.record_packet_processed(1500);
        collector.record_packet_processed(1000);
        collector.record_packet_dropped();
        collector.record_error();
        collector.record_latency(1000);
        collector.record_latency(2000);

        let metrics = collector.get_metrics();
        assert_eq!(metrics.packets_processed, 2);
        assert_eq!(metrics.bytes_processed, 2500);
        assert_eq!(metrics.packets_dropped, 1);
        assert_eq!(metrics.errors, 1);
        assert_eq!(metrics.average_latency_ns, 1500);
        assert_eq!(metrics.max_latency_ns, 2000);
        assert_eq!(metrics.min_latency_ns, 1000);
    }

    #[test]
    fn test_metrics_reset() {
        let collector = MetricsCollector::new();
        
        collector.record_packet_processed(1500);
        collector.record_packet_dropped();
        
        let metrics_before = collector.get_metrics();
        assert_eq!(metrics_before.packets_processed, 1);
        assert_eq!(metrics_before.packets_dropped, 1);

        collector.reset();
        
        let metrics_after = collector.get_metrics();
        assert_eq!(metrics_after.packets_processed, 0);
        assert_eq!(metrics_after.packets_dropped, 0);
    }
}
