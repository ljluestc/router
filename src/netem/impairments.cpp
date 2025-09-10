#include "network_impairments.h"
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <algorithm>
#include <random>

namespace RouterSim {

// Base NetworkImpairment implementation
NetworkImpairment::NetworkImpairment(const std::string& interface)
    : interface_(interface), active_(false) {
}

NetworkImpairment::~NetworkImpairment() {
}

// PacketLossImpairment implementation
PacketLossImpairment::PacketLossImpairment(const std::string& interface)
    : NetworkImpairment(interface), loss_percentage_(0.0), packets_processed_(0), packets_dropped_(0) {
}

PacketLossImpairment::~PacketLossImpairment() {
    remove();
}

bool PacketLossImpairment::apply(const ImpairmentConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = config.parameters.find("percentage");
    if (it == config.parameters.end()) {
        return false;
    }
    
    loss_percentage_ = std::stod(it->second);
    
    // Build tc command for packet loss
    std::ostringstream cmd;
    cmd << "tc qdisc add dev " << interface_ 
        << " root netem loss " << loss_percentage_ << "%";
    
    int result = std::system(cmd.str().c_str());
    if (result == 0) {
        active_ = true;
        std::cout << "Applied packet loss impairment: " << loss_percentage_ << "% on " << interface_ << std::endl;
        return true;
    }
    
    return false;
}

bool PacketLossImpairment::remove() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!active_) {
        return true;
    }
    
    std::ostringstream cmd;
    cmd << "tc qdisc del dev " << interface_ << " root";
    
    int result = std::system(cmd.str().c_str());
    if (result == 0) {
        active_ = false;
        std::cout << "Removed packet loss impairment from " << interface_ << std::endl;
        return true;
    }
    
    return false;
}

bool PacketLossImpairment::isActive() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return active_;
}

ImpairmentStatistics PacketLossImpairment::getStatistics() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    ImpairmentStatistics stats;
    stats.type = "packet_loss";
    stats.packets_processed = packets_processed_;
    stats.packets_dropped = packets_dropped_;
    stats.loss_percentage = loss_percentage_;
    
    if (packets_processed_ > 0) {
        stats.loss_percentage = (double)packets_dropped_ / packets_processed_ * 100.0;
    }
    
    return stats;
}

void PacketLossImpairment::reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    packets_processed_ = 0;
    packets_dropped_ = 0;
}

// LatencyImpairment implementation
LatencyImpairment::LatencyImpairment(const std::string& interface)
    : NetworkImpairment(interface), delay_ms_(0.0), jitter_ms_(0.0), 
      packets_processed_(0), packets_delayed_(0), total_delay_(0.0), total_jitter_(0.0) {
}

LatencyImpairment::~LatencyImpairment() {
    remove();
}

bool LatencyImpairment::apply(const ImpairmentConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = config.parameters.find("delay");
    if (it == config.parameters.end()) {
        return false;
    }
    
    delay_ms_ = std::stod(it->second);
    
    it = config.parameters.find("jitter");
    if (it != config.parameters.end()) {
        jitter_ms_ = std::stod(it->second);
    }
    
    // Build tc command for latency
    std::ostringstream cmd;
    cmd << "tc qdisc add dev " << interface_ 
        << " root netem delay " << delay_ms_ << "ms";
    
    if (jitter_ms_ > 0) {
        cmd << " " << jitter_ms_ << "ms";
    }
    
    int result = std::system(cmd.str().c_str());
    if (result == 0) {
        active_ = true;
        std::cout << "Applied latency impairment: " << delay_ms_ << "ms delay on " << interface_ << std::endl;
        return true;
    }
    
    return false;
}

bool LatencyImpairment::remove() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!active_) {
        return true;
    }
    
    std::ostringstream cmd;
    cmd << "tc qdisc del dev " << interface_ << " root";
    
    int result = std::system(cmd.str().c_str());
    if (result == 0) {
        active_ = false;
        std::cout << "Removed latency impairment from " << interface_ << std::endl;
        return true;
    }
    
    return false;
}

bool LatencyImpairment::isActive() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return active_;
}

ImpairmentStatistics LatencyImpairment::getStatistics() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    ImpairmentStatistics stats;
    stats.type = "latency";
    stats.packets_processed = packets_processed_;
    stats.packets_delayed = packets_delayed_;
    stats.delay_ms = delay_ms_;
    stats.jitter_ms = jitter_ms_;
    
    if (packets_delayed_ > 0) {
        stats.delay_ms = total_delay_ / packets_delayed_;
        stats.jitter_ms = total_jitter_ / packets_delayed_;
    }
    
    return stats;
}

void LatencyImpairment::reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    packets_processed_ = 0;
    packets_delayed_ = 0;
    total_delay_ = 0.0;
    total_jitter_ = 0.0;
}

// BandwidthImpairment implementation
BandwidthImpairment::BandwidthImpairment(const std::string& interface)
    : NetworkImpairment(interface), bandwidth_bps_(0), packets_processed_(0), packets_dropped_(0) {
}

BandwidthImpairment::~BandwidthImpairment() {
    remove();
}

bool BandwidthImpairment::apply(const ImpairmentConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = config.parameters.find("bandwidth");
    if (it == config.parameters.end()) {
        return false;
    }
    
    bandwidth_bps_ = std::stoull(it->second);
    
    // Build tc command for bandwidth limiting
    std::ostringstream cmd;
    cmd << "tc qdisc add dev " << interface_ 
        << " root tbf rate " << (bandwidth_bps_ / 1000) << "kbit burst 32kbit latency 400ms";
    
    int result = std::system(cmd.str().c_str());
    if (result == 0) {
        active_ = true;
        std::cout << "Applied bandwidth impairment: " << bandwidth_bps_ << " bps on " << interface_ << std::endl;
        return true;
    }
    
    return false;
}

bool BandwidthImpairment::remove() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!active_) {
        return true;
    }
    
    std::ostringstream cmd;
    cmd << "tc qdisc del dev " << interface_ << " root";
    
    int result = std::system(cmd.str().c_str());
    if (result == 0) {
        active_ = false;
        std::cout << "Removed bandwidth impairment from " << interface_ << std::endl;
        return true;
    }
    
    return false;
}

bool BandwidthImpairment::isActive() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return active_;
}

ImpairmentStatistics BandwidthImpairment::getStatistics() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    ImpairmentStatistics stats;
    stats.type = "bandwidth";
    stats.packets_processed = packets_processed_;
    stats.packets_dropped = packets_dropped_;
    
    return stats;
}

void BandwidthImpairment::reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    packets_processed_ = 0;
    packets_dropped_ = 0;
}

// DuplicationImpairment implementation
DuplicationImpairment::DuplicationImpairment(const std::string& interface)
    : NetworkImpairment(interface), duplication_percentage_(0.0), packets_processed_(0), packets_duplicated_(0) {
}

DuplicationImpairment::~DuplicationImpairment() {
    remove();
}

bool DuplicationImpairment::apply(const ImpairmentConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = config.parameters.find("percentage");
    if (it == config.parameters.end()) {
        return false;
    }
    
    duplication_percentage_ = std::stod(it->second);
    
    // Build tc command for packet duplication
    std::ostringstream cmd;
    cmd << "tc qdisc add dev " << interface_ 
        << " root netem duplicate " << duplication_percentage_ << "%";
    
    int result = std::system(cmd.str().c_str());
    if (result == 0) {
        active_ = true;
        std::cout << "Applied duplication impairment: " << duplication_percentage_ << "% on " << interface_ << std::endl;
        return true;
    }
    
    return false;
}

bool DuplicationImpairment::remove() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!active_) {
        return true;
    }
    
    std::ostringstream cmd;
    cmd << "tc qdisc del dev " << interface_ << " root";
    
    int result = std::system(cmd.str().c_str());
    if (result == 0) {
        active_ = false;
        std::cout << "Removed duplication impairment from " << interface_ << std::endl;
        return true;
    }
    
    return false;
}

bool DuplicationImpairment::isActive() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return active_;
}

ImpairmentStatistics DuplicationImpairment::getStatistics() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    ImpairmentStatistics stats;
    stats.type = "duplication";
    stats.packets_processed = packets_processed_;
    stats.packets_duplicated = packets_duplicated_;
    stats.duplication_percentage = duplication_percentage_;
    
    if (packets_processed_ > 0) {
        stats.duplication_percentage = (double)packets_duplicated_ / packets_processed_ * 100.0;
    }
    
    return stats;
}

void DuplicationImpairment::reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    packets_processed_ = 0;
    packets_duplicated_ = 0;
}

// CorruptionImpairment implementation
CorruptionImpairment::CorruptionImpairment(const std::string& interface)
    : NetworkImpairment(interface), corruption_percentage_(0.0), packets_processed_(0), packets_corrupted_(0) {
}

CorruptionImpairment::~CorruptionImpairment() {
    remove();
}

bool CorruptionImpairment::apply(const ImpairmentConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = config.parameters.find("percentage");
    if (it == config.parameters.end()) {
        return false;
    }
    
    corruption_percentage_ = std::stod(it->second);
    
    // Build tc command for packet corruption
    std::ostringstream cmd;
    cmd << "tc qdisc add dev " << interface_ 
        << " root netem corrupt " << corruption_percentage_ << "%";
    
    int result = std::system(cmd.str().c_str());
    if (result == 0) {
        active_ = true;
        std::cout << "Applied corruption impairment: " << corruption_percentage_ << "% on " << interface_ << std::endl;
        return true;
    }
    
    return false;
}

bool CorruptionImpairment::remove() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!active_) {
        return true;
    }
    
    std::ostringstream cmd;
    cmd << "tc qdisc del dev " << interface_ << " root";
    
    int result = std::system(cmd.str().c_str());
    if (result == 0) {
        active_ = false;
        std::cout << "Removed corruption impairment from " << interface_ << std::endl;
        return true;
    }
    
    return false;
}

bool CorruptionImpairment::isActive() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return active_;
}

ImpairmentStatistics CorruptionImpairment::getStatistics() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    ImpairmentStatistics stats;
    stats.type = "corruption";
    stats.packets_processed = packets_processed_;
    stats.packets_corrupted = packets_corrupted_;
    stats.corruption_percentage = corruption_percentage_;
    
    if (packets_processed_ > 0) {
        stats.corruption_percentage = (double)packets_corrupted_ / packets_processed_ * 100.0;
    }
    
    return stats;
}

void CorruptionImpairment::reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    packets_processed_ = 0;
    packets_corrupted_ = 0;
}

// ReorderingImpairment implementation
ReorderingImpairment::ReorderingImpairment(const std::string& interface)
    : NetworkImpairment(interface), reordering_percentage_(0.0), reordering_delay_ms_(0), 
      packets_processed_(0), packets_reordered_(0) {
}

ReorderingImpairment::~ReorderingImpairment() {
    remove();
}

bool ReorderingImpairment::apply(const ImpairmentConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = config.parameters.find("percentage");
    if (it == config.parameters.end()) {
        return false;
    }
    
    reordering_percentage_ = std::stod(it->second);
    
    it = config.parameters.find("delay");
    if (it != config.parameters.end()) {
        reordering_delay_ms_ = std::stoul(it->second);
    }
    
    // Build tc command for packet reordering
    std::ostringstream cmd;
    cmd << "tc qdisc add dev " << interface_ 
        << " root netem reorder " << reordering_percentage_ << "%";
    
    if (reordering_delay_ms_ > 0) {
        cmd << " " << reordering_delay_ms_ << "ms";
    }
    
    int result = std::system(cmd.str().c_str());
    if (result == 0) {
        active_ = true;
        std::cout << "Applied reordering impairment: " << reordering_percentage_ << "% on " << interface_ << std::endl;
        return true;
    }
    
    return false;
}

bool ReorderingImpairment::remove() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!active_) {
        return true;
    }
    
    std::ostringstream cmd;
    cmd << "tc qdisc del dev " << interface_ << " root";
    
    int result = std::system(cmd.str().c_str());
    if (result == 0) {
        active_ = false;
        std::cout << "Removed reordering impairment from " << interface_ << std::endl;
        return true;
    }
    
    return false;
}

bool ReorderingImpairment::isActive() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return active_;
}

ImpairmentStatistics ReorderingImpairment::getStatistics() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    ImpairmentStatistics stats;
    stats.type = "reordering";
    stats.packets_processed = packets_processed_;
    stats.packets_delayed = packets_reordered_;
    
    return stats;
}

void ReorderingImpairment::reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    packets_processed_ = 0;
    packets_reordered_ = 0;
}

} // namespace RouterSim