use std::sync::Arc;
use std::time::Instant;
use tokio::sync::RwLock;
use bytes::{Bytes, BytesMut};
use crossbeam_channel::{Receiver, Sender, unbounded};
use dashmap::DashMap;
use serde::{Deserialize, Serialize};
use thiserror::Error;
use tracing::{info, warn, error, debug};

use crate::{PacketInfo, RouterError};

#[derive(Error, Debug)]
pub enum PacketEngineError {
    #[error("Packet processing error: {0}")]
    Processing(String),
    #[error("Queue error: {0}")]
    Queue(String),
    #[error("Configuration error: {0}")]
    Configuration(String),
}

pub struct PacketEngine {
    input_queue: Sender<(Bytes, PacketInfo)>,
    output_queue: Receiver<Bytes>,
    processing_workers: Vec<tokio::task::JoinHandle<()>>,
    statistics: Arc<DashMap<String, u64>>,
    running: Arc<RwLock<bool>>,
}

impl PacketEngine {
    pub fn new() -> Result<Self, RouterError> {
        let (input_tx, input_rx) = unbounded();
        let (output_tx, output_rx) = unbounded();
        
        let statistics = Arc::new(DashMap::new());
        let running = Arc::new(RwLock::new(false));
        
        Ok(Self {
            input_queue: input_tx,
            output_queue: output_rx,
            processing_workers: Vec::new(),
            statistics,
            running,
        })
    }

    pub async fn start(&self) -> Result<(), RouterError> {
        let mut running = self.running.write().await;
        *running = true;
        drop(running);

        // Start processing workers
        for i in 0..num_cpus::get() {
            let input_rx = input_rx.clone();
            let output_tx = output_tx.clone();
            let statistics = self.statistics.clone();
            let running = self.running.clone();

            let worker = tokio::spawn(async move {
                Self::processing_worker(i, input_rx, output_tx, statistics, running).await;
            });

            self.processing_workers.push(worker);
        }

        info!("Packet engine started with {} workers", num_cpus::get());
        Ok(())
    }

    pub async fn stop(&self) -> Result<(), RouterError> {
        let mut running = self.running.write().await;
        *running = false;
        drop(running);

        // Wait for workers to finish
        for worker in &self.processing_workers {
            let _ = worker.await;
        }

        info!("Packet engine stopped");
        Ok(())
    }

    pub async fn process_packet(&self, packet: Bytes, info: PacketInfo) -> Result<(), RouterError> {
        if self.input_queue.send((packet, info)).is_err() {
            return Err(RouterError::PacketProcessing("Failed to queue packet".to_string()));
        }
        Ok(())
    }

    async fn processing_worker(
        worker_id: usize,
        input_rx: Receiver<(Bytes, PacketInfo)>,
        output_tx: Sender<Bytes>,
        statistics: Arc<DashMap<String, u64>>,
        running: Arc<RwLock<bool>>,
    ) {
        debug!("Starting packet processing worker {}", worker_id);

        while *running.read().await {
            match input_rx.recv_timeout(std::time::Duration::from_millis(100)) {
                Ok((packet, info)) => {
                    let start = Instant::now();
                    
                    // Process packet
                    if let Err(e) = Self::process_packet_internal(&packet, &info, &statistics).await {
                        error!("Packet processing error: {}", e);
                        continue;
                    }
                    
                    // Forward processed packet
                    if let Err(_) = output_tx.send(packet) {
                        warn!("Failed to send processed packet");
                    }
                    
                    let duration = start.elapsed();
                    debug!("Processed packet in {:?}", duration);
                }
                Err(crossbeam_channel::RecvTimeoutError::Timeout) => {
                    // Continue loop
                }
                Err(crossbeam_channel::RecvTimeoutError::Disconnected) => {
                    break;
                }
            }
        }

        debug!("Packet processing worker {} stopped", worker_id);
    }

    async fn process_packet_internal(
        packet: &Bytes,
        info: &PacketInfo,
        statistics: &Arc<DashMap<String, u64>>,
    ) -> Result<(), PacketEngineError> {
        // Update statistics
        statistics.entry("packets_processed".to_string()).and_modify(|v| *v += 1).or_insert(1);
        statistics.entry("bytes_processed".to_string()).and_modify(|v| *v += packet.len() as u64).or_insert(packet.len() as u64);
        
        // Protocol-specific processing
        match info.protocol.as_str() {
            "TCP" => Self::process_tcp_packet(packet, info).await?,
            "UDP" => Self::process_udp_packet(packet, info).await?,
            "ICMP" => Self::process_icmp_packet(packet, info).await?,
            _ => {
                debug!("Unsupported protocol: {}", info.protocol);
            }
        }

        Ok(())
    }

    async fn process_tcp_packet(packet: &Bytes, info: &PacketInfo) -> Result<(), PacketEngineError> {
        // TCP-specific processing logic
        debug!("Processing TCP packet from {} to {}", info.source_ip, info.dest_ip);
        Ok(())
    }

    async fn process_udp_packet(packet: &Bytes, info: &PacketInfo) -> Result<(), PacketEngineError> {
        // UDP-specific processing logic
        debug!("Processing UDP packet from {} to {}", info.source_ip, info.dest_ip);
        Ok(())
    }

    async fn process_icmp_packet(packet: &Bytes, info: &PacketInfo) -> Result<(), PacketEngineError> {
        // ICMP-specific processing logic
        debug!("Processing ICMP packet from {} to {}", info.source_ip, info.dest_ip);
        Ok(())
    }

    pub fn get_statistics(&self) -> HashMap<String, u64> {
        self.statistics.iter().map(|entry| (entry.key().clone(), *entry.value())).collect()
    }
}