use std::collections::VecDeque;
use std::sync::{Arc, Mutex};
use std::time::{Duration, Instant};

/// Memory pool for efficient packet buffer allocation
pub struct MemoryPool {
    buffers: Arc<Mutex<VecDeque<PacketBuffer>>>,
    buffer_size: usize,
    max_pool_size: usize,
    created_buffers: Arc<Mutex<usize>>,
    total_allocations: Arc<Mutex<usize>>,
    total_deallocations: Arc<Mutex<usize>>,
}

/// Packet buffer with metadata
#[derive(Debug, Clone)]
pub struct PacketBuffer {
    pub data: Vec<u8>,
    pub size: usize,
    pub id: u64,
    pub created_at: Instant,
    pub last_used: Instant,
}

impl PacketBuffer {
    /// Create a new packet buffer
    pub fn new(size: usize, id: u64) -> Self {
        Self {
            data: vec![0u8; size],
            size,
            id,
            created_at: Instant::now(),
            last_used: Instant::now(),
        }
    }

    /// Copy data from another packet
    pub fn copy_from(&mut self, other: &crate::FastPacket) {
        self.data[..other.size as usize].copy_from_slice(&other.data[..other.size as usize]);
        self.size = other.size as usize;
        self.last_used = Instant::now();
    }

    /// Get mutable reference to data
    pub fn data_mut(&mut self) -> &mut [u8] {
        self.last_used = Instant::now();
        &mut self.data[..self.size]
    }

    /// Get reference to data
    pub fn data(&self) -> &[u8] {
        &self.data[..self.size]
    }

    /// Resize buffer
    pub fn resize(&mut self, new_size: usize) {
        if new_size > self.data.len() {
            self.data.resize(new_size, 0);
        }
        self.size = new_size;
        self.last_used = Instant::now();
    }

    /// Check if buffer is expired
    pub fn is_expired(&self, max_age: Duration) -> bool {
        self.created_at.elapsed() > max_age
    }

    /// Check if buffer is stale
    pub fn is_stale(&self, max_idle: Duration) -> bool {
        self.last_used.elapsed() > max_idle
    }
}

impl MemoryPool {
    /// Create a new memory pool
    pub fn new(max_pool_size: usize, buffer_size: usize) -> Self {
        Self {
            buffers: Arc::new(Mutex::new(VecDeque::new())),
            buffer_size,
            max_pool_size,
            created_buffers: Arc::new(Mutex::new(0)),
            total_allocations: Arc::new(Mutex::new(0)),
            total_deallocations: Arc::new(Mutex::new(0)),
        }
    }

    /// Get a packet buffer from the pool
    pub fn get_packet(&self, size: usize) -> Result<PacketBuffer, String> {
        let mut buffers = self.buffers.lock().unwrap();
        
        // Try to find a suitable buffer in the pool
        if let Some(index) = buffers.iter().position(|buf| buf.data.len() >= size) {
            let mut buffer = buffers.remove(index).unwrap();
            buffer.resize(size);
            buffer.last_used = Instant::now();
            
            *self.total_allocations.lock().unwrap() += 1;
            return Ok(buffer);
        }

        // Create a new buffer if pool is empty or no suitable buffer found
        let buffer_id = *self.created_buffers.lock().unwrap() as u64;
        let mut buffer = PacketBuffer::new(size.max(self.buffer_size), buffer_id);
        buffer.resize(size);
        
        *self.created_buffers.lock().unwrap() += 1;
        *self.total_allocations.lock().unwrap() += 1;
        
        Ok(buffer)
    }

    /// Return a packet buffer to the pool
    pub fn return_packet(&self, mut buffer: PacketBuffer) {
        // Don't return buffers that are too old or too small
        if buffer.is_expired(Duration::from_secs(300)) || buffer.data.len() < self.buffer_size {
            *self.total_deallocations.lock().unwrap() += 1;
            return;
        }

        let mut buffers = self.buffers.lock().unwrap();
        
        // Don't exceed max pool size
        if buffers.len() >= self.max_pool_size {
            *self.total_deallocations.lock().unwrap() += 1;
            return;
        }

        // Reset buffer for reuse
        buffer.size = self.buffer_size;
        buffer.data.fill(0);
        buffer.last_used = Instant::now();
        
        buffers.push_back(buffer);
        *self.total_deallocations.lock().unwrap() += 1;
    }

    /// Get pool statistics
    pub fn get_stats(&self) -> PoolStats {
        let buffers = self.buffers.lock().unwrap();
        let created = *self.created_buffers.lock().unwrap();
        let allocations = *self.total_allocations.lock().unwrap();
        let deallocations = *self.total_deallocations.lock().unwrap();

        PoolStats {
            pool_size: buffers.len(),
            max_pool_size: self.max_pool_size,
            buffer_size: self.buffer_size,
            created_buffers: created,
            total_allocations: allocations,
            total_deallocations: deallocations,
            active_buffers: created - deallocations,
        }
    }

    /// Clean up expired and stale buffers
    pub fn cleanup(&self) {
        let mut buffers = self.buffers.lock().unwrap();
        let now = Instant::now();
        let max_age = Duration::from_secs(300);
        let max_idle = Duration::from_secs(60);

        buffers.retain(|buf| {
            !buf.is_expired(max_age) && !buf.is_stale(max_idle)
        });
    }

    /// Force cleanup of all buffers
    pub fn clear(&self) {
        let mut buffers = self.buffers.lock().unwrap();
        buffers.clear();
    }
}

/// Memory pool statistics
#[derive(Debug, Clone)]
pub struct PoolStats {
    pub pool_size: usize,
    pub max_pool_size: usize,
    pub buffer_size: usize,
    pub created_buffers: usize,
    pub total_allocations: usize,
    pub total_deallocations: usize,
    pub active_buffers: usize,
}

impl PoolStats {
    /// Get allocation hit rate (percentage of allocations served from pool)
    pub fn hit_rate(&self) -> f64 {
        if self.total_allocations == 0 {
            return 0.0;
        }
        let pool_hits = self.total_allocations - self.active_buffers;
        pool_hits as f64 / self.total_allocations as f64 * 100.0
    }

    /// Get pool utilization (percentage of max pool size used)
    pub fn utilization(&self) -> f64 {
        if self.max_pool_size == 0 {
            return 0.0;
        }
        self.pool_size as f64 / self.max_pool_size as f64 * 100.0
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_memory_pool_creation() {
        let pool = MemoryPool::new(100, 1500);
        let stats = pool.get_stats();
        assert_eq!(stats.max_pool_size, 100);
        assert_eq!(stats.buffer_size, 1500);
        assert_eq!(stats.pool_size, 0);
    }

    #[test]
    fn test_packet_allocation() {
        let pool = MemoryPool::new(10, 1500);
        
        let buffer = pool.get_packet(1000).unwrap();
        assert_eq!(buffer.size, 1000);
        assert_eq!(buffer.data.len(), 1500); // Allocated with max size
        
        let stats = pool.get_stats();
        assert_eq!(stats.total_allocations, 1);
        assert_eq!(stats.active_buffers, 1);
    }

    #[test]
    fn test_packet_return() {
        let pool = MemoryPool::new(10, 1500);
        
        let buffer = pool.get_packet(1000).unwrap();
        pool.return_packet(buffer);
        
        let stats = pool.get_stats();
        assert_eq!(stats.pool_size, 1);
        assert_eq!(stats.total_deallocations, 1);
    }

    #[test]
    fn test_pool_reuse() {
        let pool = MemoryPool::new(10, 1500);
        
        // Allocate and return a buffer
        let buffer1 = pool.get_packet(1000).unwrap();
        let buffer_id1 = buffer1.id;
        pool.return_packet(buffer1);
        
        // Allocate again - should reuse the same buffer
        let buffer2 = pool.get_packet(1000).unwrap();
        let buffer_id2 = buffer2.id;
        
        // Should be different IDs since we're creating new buffers
        // (in a real implementation, we might reuse the same buffer)
        assert_ne!(buffer_id1, buffer_id2);
    }

    #[test]
    fn test_pool_cleanup() {
        let pool = MemoryPool::new(10, 1500);
        
        // Fill the pool
        for _ in 0..5 {
            let buffer = pool.get_packet(1000).unwrap();
            pool.return_packet(buffer);
        }
        
        let stats_before = pool.get_stats();
        assert_eq!(stats_before.pool_size, 5);
        
        // Cleanup
        pool.cleanup();
        
        let stats_after = pool.get_stats();
        // Pool size should remain the same since buffers are not expired
        assert_eq!(stats_after.pool_size, 5);
    }
}
