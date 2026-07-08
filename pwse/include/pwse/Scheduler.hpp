#pragma once

#include <vector>
#include "pwse/Task.hpp"
#include "pwse/Agent.hpp"

namespace pwse {

// A single scheduling decision: how much time was allocated to a task
// on the current tick.
struct Assignment {
    int taskId;
    int allocated;
};

// Result of one scheduling pass, including tasks that wanted work but
// got none because capacity ran out (useful for bottleneck analysis).
struct ScheduleResult {
    std::vector<Assignment> assignments;
    std::vector<int> starvedTaskIds; // pending/in-progress tasks that received 0
};

// Simple greedy priority scheduler:
//   1. Consider all not-yet-completed tasks.
//   2. Sort by priority descending (ties broken by task id for determinism).
//   3. Assign as much of the agent's remaining capacity as each task can
//      absorb, highest priority first, until capacity is exhausted.
class Scheduler {
public:
    // `currentDay` is stamped onto tasks that complete on this tick.
    ScheduleResult schedule(std::vector<Task>& tasks, Agent& agent, int currentDay);
};

} // namespace pwse
