#include "pwse/Task.hpp"

#include <algorithm>
#include <stdexcept>

namespace pwse {

Task::Task(int id, std::string name, int duration, int priority, TaskType type,
           int deadline)
    : id_(id),
      name_(std::move(name)),
      totalDuration_(duration),
      remaining_(duration),
      priority_(priority),
      type_(type),
      status_(TaskStatus::Pending),
      completedOnDay_(-1),
      deadline_(deadline) {
    if (duration <= 0) {
        throw std::invalid_argument("Task '" + name_ + "' must have positive duration");
    }
    if (deadline_ < -1 || deadline_ == 0) {
        throw std::invalid_argument("Task '" + name_ + "' has an invalid deadline");
    }
}

int Task::applyProgress(int amount, int currentDay) {
    if (amount <= 0 || status_ == TaskStatus::Completed) {
        return 0;
    }

    int consumed = std::min(amount, remaining_);
    remaining_ -= consumed;

    if (remaining_ == 0) {
        status_ = TaskStatus::Completed;
        completedOnDay_ = currentDay;
    } else if (consumed > 0) {
        status_ = TaskStatus::InProgress;
    }

    return consumed;
}

DeadlineStatus Task::deadlineStatus() const {
    if (!hasDeadline()) {
        return DeadlineStatus::NoDeadline;
    }
    if (status_ == TaskStatus::Completed) {
        return completedOnDay_ <= deadline_ ? DeadlineStatus::OnTime
                                             : DeadlineStatus::Late;
    }
    return DeadlineStatus::Missed;
}

} // namespace pwse