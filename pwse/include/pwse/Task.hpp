#pragma once

#include <string>
#include "pwse/Types.hpp"

namespace pwse {

// A unit of work the agent can spend capacity on.
// Duration and remaining work are expressed in the same unit as the
// agent's daily capacity (e.g. "hours per day").
class Task {
public:
    Task(int id, std::string name, int duration, int priority, TaskType type,
         int deadline = -1);

    int id() const { return id_; }
    const std::string& name() const { return name_; }
    int totalDuration() const { return totalDuration_; }
    int remaining() const { return remaining_; }
    int priority() const { return priority_; }
    TaskType type() const { return type_; }
    TaskStatus status() const { return status_; }

    // The day this task became fully completed (-1 if not yet completed).
    int completedOnDay() const { return completedOnDay_; }

    // -1 if no deadline was configured for this task.
    int deadline() const { return deadline_; }
    bool hasDeadline() const { return deadline_ >= 0; }

    // Applies `amount` of work to the task, clamped to remaining().
    // Updates status accordingly. Returns the amount actually consumed.
    int applyProgress(int amount, int currentDay);

    bool isCompleted() const { return status_ == TaskStatus::Completed; }

    // Evaluates this task's deadline outcome. Should be called once the
    // simulation has finished (i.e. no more progress will be applied).
    // - NoDeadline if the task never had one.
    // - OnTime / Late based on completedOnDay_ vs deadline_ if completed.
    // - Missed if the task is still not completed and it had a deadline.
    DeadlineStatus deadlineStatus() const;

private:
    int id_;
    std::string name_;
    int totalDuration_;
    int remaining_;
    int priority_;
    TaskType type_;
    TaskStatus status_;
    int completedOnDay_;
    int deadline_; // -1 means "no deadline"
};

} // namespace pwse
