#include "pwse/Task.hpp"

#include <algorithm>
#include <stdexcept>

namespace pwse {

Task::Task(int id, std::string externalId, std::string name, int duration, int priority,
           TaskType type, std::optional<int> deadline, std::vector<int> dependencyIds)
    : id_(id),
      externalId_(std::move(externalId)),
      name_(std::move(name)),
      totalDuration_(duration),
      remaining_(duration),
      priority_(priority),
      type_(type),
      status_(TaskStatus::Pending),
      completedOnDay_(-1),
      deadline_(deadline),
      dependencyIds_(std::move(dependencyIds)) {
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

bool Task::isBlocked(const std::vector<Task>& allTasks) const {
    if (isCompleted()) {
        return false;
    }
    for (int depId : dependencyIds_) {
        if (!allTasks[depId].isCompleted()) {
            return true;
        }
    }
    return false;
}

std::vector<std::string> Task::unmetDependencyNames(const std::vector<Task>& allTasks) const {
    std::vector<std::string> unmet;
    for (int depId : dependencyIds_) {
        if (!allTasks[depId].isCompleted()) {
            unmet.push_back(allTasks[depId].name());
        }
    }
    return unmet;
}

} // namespace pwse
