#pragma once

namespace pwse {

// The single simulated user. Owns a daily capacity budget that is
// reset every tick and consumed as tasks are worked on.
class Agent {
public:
    explicit Agent(int dailyCapacity);

    int dailyCapacity() const { return dailyCapacity_; }
    int remainingCapacity() const { return remainingCapacity_; }

    // Called by the engine at the start of each tick/day.
    void resetDailyCapacity();

    // Consumes up to `amount` of remaining capacity. Returns the amount
    // actually consumed (clamped to what's available).
    int consumeCapacity(int amount);

private:
    int dailyCapacity_;
    int remainingCapacity_;
};

} // namespace pwse
