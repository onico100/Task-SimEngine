#include "pwse/SimulationEngine.hpp"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include "pwse/DependencyValidator.hpp"

namespace pwse {

SimulationEngine::SimulationEngine(const SimulationConfig& config)
    : agent_(config.dailyCapacity), totalDays_(config.days) {
    tasks_.reserve(config.tasks.size());

    // ConfigLoader has already validated that every dependency id exists,
    // ids are unique, and the graph is acyclic. All that's left here is a
    // one-time resolution of dependency strings to internal indices, so
    // that Task::isBlocked() never has to touch strings on the hot path.
    auto idIndex = buildIdIndex(config.tasks);

    int id = 0;
    for (const auto& spec : config.tasks) {
        std::vector<int> dependencyIndices;
        dependencyIndices.reserve(spec.dependencies.size());
        for (const auto& depId : spec.dependencies) {
            dependencyIndices.push_back(idIndex.at(depId));
        }

        // ids are assigned sequentially (0..N-1), so id == index into tasks_.
        tasks_.emplace_back(id++, spec.id, spec.name, spec.duration, spec.priority, spec.type,
                             spec.deadline, std::move(dependencyIndices));
    }
}

SimulationSummary SimulationEngine::run() {
    SimulationSummary summary;
    summary.totalDays = totalDays_;
    summary.totalTasks = static_cast<int>(tasks_.size());

    for (int day = 1; day <= totalDays_; ++day) {
        runOneDay(day);
    }

    int completed = 0;
    long long completionDaySum = 0;
    for (const auto& task : tasks_) {
        if (task.isCompleted()) {
            ++completed;
            completionDaySum += task.completedOnDay();

            if (task.deadline() && task.completedOnDay() > *task.deadline()) {
                ++summary.lateTasks;
            }
        } else {
            if (task.deadline() && *task.deadline() <= totalDays_) {
                ++summary.missedTasks;
            }
            if (task.isBlocked(tasks_)) {
                ++summary.blockedTasksAtEnd;
                ++summary.neverAvailableTasks;
            }
        }

        for (int depId : task.dependencyIds()) {
            if (tasks_[depId].isCompleted()) {
                ++summary.dependencyConstraintsResolved;
            }
        }
    }

    summary.completedTasks = completed;
    summary.incompleteTasks = summary.totalTasks - completed;
    summary.avgCompletionDay = completed > 0
        ? static_cast<double>(completionDaySum) / completed
        : 0.0;
    summary.overloadDays = overloadDayCount_;

    printSummary(summary);
    return summary;
}

void SimulationEngine::runOneDay(int day) {
    agent_.resetDailyCapacity();

    ScheduleResult result = scheduler_.schedule(tasks_, agent_, day);

    printDayHeader(day);

    if (result.assignments.empty() && result.blockedTaskIds.empty()) {
        std::cout << "  (no capacity assigned - all tasks complete or no capacity)\n";
    }

    for (const auto& assignment : result.assignments) {
        const Task& task = tasks_[assignment.taskId];
        std::cout << "  worked '" << task.name() << "' [" << toString(task.type())
                   << ", prio " << task.priority() << "] +" << assignment.allocated
                   << " -> " << (task.totalDuration() - task.remaining())
                   << "/" << task.totalDuration();
        if (task.isCompleted()) {
            std::cout << "  (COMPLETED)";
        }
        std::cout << "\n";
    }

    if (!result.blockedTaskIds.empty()) {
        std::cout << "  blocked:\n";
        for (int id : result.blockedTaskIds) {
            const Task& task = tasks_[id];
            std::cout << "    - '" << task.name() << "' waiting for:";
            for (const auto& depName : task.unmetDependencyNames(tasks_)) {
                std::cout << " '" << depName << "'";
            }
            std::cout << "\n";
        }
    }

    if (!result.starvedTaskIds.empty()) {
        ++overloadDayCount_;
        std::cout << "  overload: " << result.starvedTaskIds.size()
                   << " task(s) got no capacity today:";
        for (int id : result.starvedTaskIds) {
            std::cout << " '" << tasks_[id].name() << "'";
        }
        std::cout << "\n";
    }

    std::cout << "  capacity used: " << (agent_.dailyCapacity() - agent_.remainingCapacity())
               << "/" << agent_.dailyCapacity() << "\n";
}

void SimulationEngine::printDayHeader(int day) const {
    std::cout << "--- Day " << day << "/" << totalDays_ << " ---\n";
}

void SimulationEngine::printSummary(const SimulationSummary& summary) const {
    std::cout << "\n=== Simulation Summary ===\n";
    std::cout << "Days simulated:      " << summary.totalDays << "\n";
    std::cout << "Total tasks:         " << summary.totalTasks << "\n";
    std::cout << "Completed:           " << summary.completedTasks << "\n";
    std::cout << "Incomplete:          " << summary.incompleteTasks << "\n";
    std::cout << "Late:                " << summary.lateTasks << "\n";
    std::cout << "Missed:              " << summary.missedTasks << "\n";
    std::cout << "Blocked (at end):    " << summary.blockedTasksAtEnd << "\n";
    std::cout << "Never available:     " << summary.neverAvailableTasks << "\n";
    std::cout << "Deps resolved:       " << summary.dependencyConstraintsResolved << "\n";
    std::cout << "Overload days:       " << summary.overloadDays << "\n";
    if (summary.completedTasks > 0) {
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "Avg completion day: " << summary.avgCompletionDay << "\n";
    }

    if (summary.incompleteTasks > 0) {
        std::cout << "\nIncomplete tasks (bottlenecks):\n";
        for (const auto& task : tasks_) {
            if (task.isCompleted()) {
                continue;
            }
            std::cout << "  - '" << task.name() << "' [" << toString(task.type())
                       << ", prio " << task.priority() << "] "
                       << (task.totalDuration() - task.remaining())
                       << "/" << task.totalDuration() << " done";
            if (task.isBlocked(tasks_)) {
                std::cout << "  (BLOCKED - waiting for";
                for (const auto& depName : task.unmetDependencyNames(tasks_)) {
                    std::cout << " '" << depName << "'";
                }
                std::cout << ")";
            }
            std::cout << "\n";
        }
    }
}

} // namespace pwse
