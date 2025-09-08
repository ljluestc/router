#include "traffic_shaping/traffic_shaper.h"
#include "traffic_shaping/token_bucket.h"
#include "traffic_shaping/wfq.h"
#include <iostream>

namespace router_sim {

std::unique_ptr<TrafficShaper> TrafficShaperFactory::create(const std::string& algorithm) {
    if (algorithm == "token_bucket") {
        return std::make_unique<TokenBucketShaper>();
    } else if (algorithm == "wfq") {
        return std::make_unique<WFQShaper>();
    } else {
        std::cerr << "Unknown traffic shaping algorithm: " << algorithm << std::endl;
        return nullptr;
    }
}

std::vector<std::string> TrafficShaperFactory::get_available_algorithms() {
    return {"token_bucket", "wfq"};
}

} // namespace router_sim
