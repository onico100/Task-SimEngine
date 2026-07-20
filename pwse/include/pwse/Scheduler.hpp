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

// Result of one scheduling pass.
struct ScheduleResult {
    std::vector<Assignment> assignments;
    std::vector<int> starvedTaskIds; // eligible tasks that got 0 capacity today
    std::vector<int> blockedTaskIds; // tasks skipped entirely: unmet dependencies
};

// Simple greedy priority scheduler:
//   1. Consider all not-yet-completed tasks whose dependencies are all
//      satisfied (blocked tasks are set aside and never receive capacity).
//   2. Sort the eligible tasks by priority descending (ties broken by
//      task id for determinism).
//   3. Assign as much of the agent's remaining capacity as each task can
//      absorb, highest priority first, until capacity is exhausted.
class Scheduler {
public:
    // `currentDay` is stamped onto tasks that complete on this tick.
    ScheduleResult schedule(std::vector<Task>& tasks, Agent& agent, int currentDay);
};

} // namespace pwse
