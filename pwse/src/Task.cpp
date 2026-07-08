#include "pwse/Task.hpp"

#include <algorithm>
#include <stdexcept>

namespace pwse {

Task::Task(int id, std::string name, int duration, int priority, TaskType type)
    : id_(id),
      name_(std::move(name)),
      totalDuration_(duration),
      remaining_(duration),
      priority_(priority),
      type_(type),
      status_(TaskStatus::Pending),
      completedOnDay_(-1) {
    if (duration <= 0) {
        throw std::invalid_argument("Task '" + name_ + "' must have positive duration");
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

} // namespace pwse
