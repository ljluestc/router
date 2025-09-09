#include "token_bucket.h"
#include <chrono>
#include <algorithm>
#include <iostream>

namespace RouterSim {

TokenBucket::TokenBucket(uint64_t capacity, uint64_t refill_rate, uint64_t refill_interval_ms)
    : capacity_(capacity), refill_rate_(refill_rate), refill_interval_ms_(refill_interval_ms),
      tokens_(capacity), last_refill_time_(std::chrono::steady_clock::now()) {
}

TokenBucket::~TokenBucket() {
}

bool TokenBucket::consume(uint64_t tokens) {
    refill();
    
    if (tokens_ >= tokens) {
        tokens_ -= tokens;
        return true;
    }
    
    return false;
}

bool TokenBucket::tryConsume(uint64_t tokens) {
    refill();
    return tokens_ >= tokens;
}

uint64_t TokenBucket::getAvailableTokens() const {
    return tokens_;
}

uint64_t TokenBucket::getCapacity() const {
    return capacity_;
}

uint64_t TokenBucket::getRefillRate() const {
    return refill_rate_;
}

void TokenBucket::setRefillRate(uint64_t rate) {
    refill_rate_ = rate;
}

void TokenBucket::setCapacity(uint64_t capacity) {
    capacity_ = capacity;
    tokens_ = std::min(tokens_, capacity_);
}

void TokenBucket::refill() {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_refill_time_);
    
    if (elapsed.count() >= refill_interval_ms_) {
        uint64_t tokens_to_add = (elapsed.count() / refill_interval_ms_) * refill_rate_;
        tokens_ = std::min(capacity_, tokens_ + tokens_to_add);
        last_refill_time_ = now;
    }
}

// TokenBucketShaper implementation
TokenBucketShaper::TokenBucketShaper(uint64_t capacity, uint64_t refill_rate, uint64_t refill_interval_ms)
    : bucket_(capacity, refill_rate, refill_interval_ms), total_packets_(0), dropped_packets_(0) {
}

TokenBucketShaper::~TokenBucketShaper() {
}

bool TokenBucketShaper::shapePacket(const Packet& packet) {
    total_packets_++;
    
    if (bucket_.consume(packet.size)) {
        return true;
    } else {
        dropped_packets_++;
        return false;
    }
}

bool TokenBucketShaper::shapePacket(uint64_t packet_size) {
    total_packets_++;
    
    if (bucket_.consume(packet_size)) {
        return true;
    } else {
        dropped_packets_++;
        return false;
    }
}

ShaperStats TokenBucketShaper::getStats() const {
    ShaperStats stats;
    stats.total_packets = total_packets_;
    stats.dropped_packets = dropped_packets_;
    stats.accepted_packets = total_packets_ - dropped_packets_;
    stats.drop_rate = total_packets_ > 0 ? (double)dropped_packets_ / total_packets_ : 0.0;
    stats.available_tokens = bucket_.getAvailableTokens();
    stats.capacity = bucket_.getCapacity();
    stats.refill_rate = bucket_.getRefillRate();
    
    return stats;
}

void TokenBucketShaper::resetStats() {
    total_packets_ = 0;
    dropped_packets_ = 0;
}

void TokenBucketShaper::updateRate(uint64_t new_rate) {
    bucket_.setRefillRate(new_rate);
}

void TokenBucketShaper::updateCapacity(uint64_t new_capacity) {
    bucket_.setCapacity(new_capacity);
}

// MultiRateTokenBucket implementation
MultiRateTokenBucket::MultiRateTokenBucket() {
}

MultiRateTokenBucket::~MultiRateTokenBucket() {
}

void MultiRateTokenBucket::addBucket(const std::string& name, uint64_t capacity, uint64_t refill_rate, uint64_t refill_interval_ms) {
    buckets_[name] = std::make_unique<TokenBucket>(capacity, refill_rate, refill_interval_ms);
}

bool MultiRateTokenBucket::consume(const std::string& bucket_name, uint64_t tokens) {
    auto it = buckets_.find(bucket_name);
    if (it != buckets_.end()) {
        return it->second->consume(tokens);
    }
    return false;
}

bool MultiRateTokenBucket::tryConsume(const std::string& bucket_name, uint64_t tokens) {
    auto it = buckets_.find(bucket_name);
    if (it != buckets_.end()) {
        return it->second->tryConsume(tokens);
    }
    return false;
}

uint64_t MultiRateTokenBucket::getAvailableTokens(const std::string& bucket_name) const {
    auto it = buckets_.find(bucket_name);
    if (it != buckets_.end()) {
        return it->second->getAvailableTokens();
    }
    return 0;
}

std::map<std::string, uint64_t> MultiRateTokenBucket::getAllAvailableTokens() const {
    std::map<std::string, uint64_t> result;
    for (const auto& pair : buckets_) {
        result[pair.first] = pair.second->getAvailableTokens();
    }
    return result;
}

void MultiRateTokenBucket::removeBucket(const std::string& name) {
    buckets_.erase(name);
}

std::vector<std::string> MultiRateTokenBucket::getBucketNames() const {
    std::vector<std::string> names;
    for (const auto& pair : buckets_) {
        names.push_back(pair.first);
    }
    return names;
}

} // namespace RouterSim