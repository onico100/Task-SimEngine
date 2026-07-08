#include "pwse/Agent.hpp"

#include <algorithm>
#include <stdexcept>

namespace pwse {

Agent::Agent(int dailyCapacity)
    : dailyCapacity_(dailyCapacity), remainingCapacity_(dailyCapacity) {
    if (dailyCapacity <= 0) {
        throw std::invalid_argument("Agent daily capacity must be positive");
    }
}

void Agent::resetDailyCapacity() {
    remainingCapacity_ = dailyCapacity_;
}

int Agent::consumeCapacity(int amount) {
    if (amount <= 0) {
        return 0;
    }
    int consumed = std::min(amount, remainingCapacity_);
    remainingCapacity_ -= consumed;
    return consumed;
}

} // namespace pwse
