#pragma once

#include <stdexcept>
#include <string>

namespace pwse {

// Category of a task. Kept as a closed enum for MVP; extend as needed.
enum class TaskType {
    Study,
    Work,
    Life
};

inline std::string toString(TaskType type) {
    switch (type) {
        case TaskType::Study: return "study";
        case TaskType::Work:  return "work";
        case TaskType::Life:  return "life";
    }
    return "unknown";
}

inline TaskType taskTypeFromString(const std::string& s) {
    if (s == "study") return TaskType::Study;
    if (s == "work")  return TaskType::Work;
    if (s == "life")  return TaskType::Life;
    throw std::invalid_argument("Unknown task type: " + s);
}

// Lifecycle state of a task within the simulation.
enum class TaskStatus {
    Pending,     // not started, waiting for capacity
    InProgress,  // has received partial progress
    Completed    // duration fully consumed
};

inline std::string toString(TaskStatus status) {
    switch (status) {
        case TaskStatus::Pending:    return "pending";
        case TaskStatus::InProgress: return "in_progress";
        case TaskStatus::Completed:  return "completed";
    }
    return "unknown";
}

} // namespace pwse
