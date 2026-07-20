#pragma once

#include <optional>
#include <string>
#include <vector>
#include "pwse/Types.hpp"

namespace pwse {

// A unit of work the agent can spend capacity on.
// Duration and remaining work are expressed in the same unit as the
// agent's daily capacity (e.g. "hours per day").
//
// Two identifiers coexist by design:
//   - `id()` is a dense 0..N-1 internal index (unchanged from before),
//     used everywhere performance-sensitive code needs O(1) lookup into
//     the owning tasks vector (Scheduler::Assignment, dependencyIds_).
//   - `externalId()` is the human/config-supplied string id (e.g.
//     "write-thesis") that dependencies are expressed in terms of.
// Dependencies are resolved from external string ids to internal indices
// once, at construction time (see SimulationEngine), so hot-path checks
// never touch strings.
class Task {
public:
    Task(int id, std::string externalId, std::string name, int duration, int priority,
         TaskType type, std::optional<int> deadline = std::nullopt,
         std::vector<int> dependencyIds = {});

    int id() const { return id_; }
    const std::string& externalId() const { return externalId_; }
    const std::string& name() const { return name_; }
    int totalDuration() const { return totalDuration_; }
    int remaining() const { return remaining_; }
    int priority() const { return priority_; }
    TaskType type() const { return type_; }
    TaskStatus status() const { return status_; }
    const std::optional<int>& deadline() const { return deadline_; }

    // Internal indices (into the same tasks vector this Task lives in)
    // of the tasks that must complete before this one can be scheduled.
    const std::vector<int>& dependencyIds() const { return dependencyIds_; }

    // The day this task became fully completed (-1 if not yet completed).
    int completedOnDay() const { return completedOnDay_; }

    // Applies `amount` of work to the task, clamped to remaining().
    // Updates status accordingly. Returns the amount actually consumed.
    int applyProgress(int amount, int currentDay);

    bool isCompleted() const { return status_ == TaskStatus::Completed; }

    // True while at least one dependency (looked up in `allTasks`, which
    // must be the tasks vector this task's dependencyIds_ index into)
    // has not yet completed. Dependency satisfaction is monotonic: once
    // a task is unblocked it stays unblocked, since completed tasks
    // never become incomplete again.
    bool isBlocked(const std::vector<Task>& allTasks) const;

    // Names of not-yet-completed dependency tasks, for status reporting.
    // Empty whenever isBlocked() is false.
    std::vector<std::string> unmetDependencyNames(const std::vector<Task>& allTasks) const;

private:
    int id_;
    std::string externalId_;
    std::string name_;
    int totalDuration_;
    int remaining_;
    int priority_;
    TaskType type_;
    TaskStatus status_;
    int completedOnDay_;
    std::optional<int> deadline_;
    std::vector<int> dependencyIds_;
};

} // namespace pwse
