#include "pwse/Scheduler.hpp"

#include <algorithm>

namespace pwse {

ScheduleResult Scheduler::schedule(std::vector<Task>& tasks, Agent& agent, int currentDay) {
    ScheduleResult result;

    // Work on indices so we can mutate the actual Task objects in-place.
    // Blocked tasks are filtered out here, before priority ordering, so
    // they can never compete for or receive capacity.
    std::vector<int> order;
    order.reserve(tasks.size());
    for (std::size_t i = 0; i < tasks.size(); ++i) {
        Task& task = tasks[i];
        if (task.isCompleted()) {
            continue;
        }
        if (task.isBlocked(tasks)) {
            result.blockedTaskIds.push_back(task.id());
            continue;
        }
        order.push_back(static_cast<int>(i));
    }

    std::sort(order.begin(), order.end(), [&](int a, int b) {
        if (tasks[a].priority() != tasks[b].priority()) {
            return tasks[a].priority() > tasks[b].priority(); // highest priority first
        }
        return tasks[a].id() < tasks[b].id(); // deterministic tie-break
    });

    for (int idx : order) {
        Task& task = tasks[idx];

        if (agent.remainingCapacity() <= 0) {
            result.starvedTaskIds.push_back(task.id());
            continue;
        }

        int want = task.remaining();
        int granted = agent.consumeCapacity(want);

        if (granted > 0) {
            task.applyProgress(granted, currentDay);
            result.assignments.push_back({task.id(), granted});
        } else {
            result.starvedTaskIds.push_back(task.id());
        }
    }

    return result;
}

} // namespace pwse
