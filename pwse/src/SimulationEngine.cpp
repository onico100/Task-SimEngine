#include "pwse/SimulationEngine.hpp"

#include <algorithm>
#include <iomanip>
#include <iostream>

namespace pwse {

SimulationEngine::SimulationEngine(const SimulationConfig& config)
    : agent_(config.dailyCapacity), totalDays_(config.days) {
    tasks_.reserve(config.tasks.size());
    int id = 0;
    for (const auto& spec : config.tasks) {
        // ids are assigned sequentially (0..N-1), so id == index into tasks_.
        tasks_.emplace_back(id++, spec.name, spec.duration, spec.priority, spec.type,
                             spec.deadline);
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
        }
    }
    summary.completedTasks = completed;
    summary.incompleteTasks = summary.totalTasks - completed;
    summary.avgCompletionDay = completed > 0
        ? static_cast<double>(completionDaySum) / completed
        : 0.0;
    summary.overloadDays = overloadDayCount_;

    for (const auto& task : tasks_) {
        DeadlineStatus ds = task.deadlineStatus();
        switch (ds) {
            case DeadlineStatus::OnTime:
                ++summary.onTimeTasks;
                ++summary.tasksWithDeadlines;
                break;
            case DeadlineStatus::Late:
                ++summary.lateTasks;
                ++summary.tasksWithDeadlines;
                break;
            case DeadlineStatus::Missed:
                ++summary.missedTasks;
                ++summary.tasksWithDeadlines;
                break;
            case DeadlineStatus::NoDeadline:
                break;
        }
    }
    summary.onTimeRate = summary.tasksWithDeadlines > 0
        ? static_cast<double>(summary.onTimeTasks) / summary.tasksWithDeadlines
        : 0.0;

    printSummary(summary);
    return summary;
}

void SimulationEngine::runOneDay(int day) {
    agent_.resetDailyCapacity();

    ScheduleResult result = scheduler_.schedule(tasks_, agent_, day);

    printDayHeader(day);

    if (result.assignments.empty()) {
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
    std::cout << "Days simulated:     " << summary.totalDays << "\n";
    std::cout << "Total tasks:        " << summary.totalTasks << "\n";
    std::cout << "Completed:          " << summary.completedTasks << "\n";
    std::cout << "Incomplete:         " << summary.incompleteTasks << "\n";
    std::cout << "Overload days:      " << summary.overloadDays << "\n";
    if (summary.completedTasks > 0) {
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "Avg completion day: " << summary.avgCompletionDay << "\n";
    }

    if (summary.tasksWithDeadlines > 0) {
        std::cout << std::fixed << std::setprecision(1);
        std::cout << "\n=== Deadlines ===\n";
        std::cout << "Tasks with deadlines: " << summary.tasksWithDeadlines << "\n";
        std::cout << "On-time rate:         " << (summary.onTimeRate * 100.0) << "%\n";
        std::cout << "On time:              " << summary.onTimeTasks << "\n";
        std::cout << "Late:                 " << summary.lateTasks << "\n";
        std::cout << "Missed:               " << summary.missedTasks << "\n";

        if (summary.lateTasks > 0) {
            std::cout << "\nLate tasks:\n";
            for (const auto& task : tasks_) {
                if (task.deadlineStatus() == DeadlineStatus::Late) {
                    std::cout << "  - '" << task.name() << "' completed day "
                               << task.completedOnDay() << " (deadline day "
                               << task.deadline() << ")\n";
                }
            }
        }

        if (summary.missedTasks > 0) {
            std::cout << "\nMissed tasks:\n";
            for (const auto& task : tasks_) {
                if (task.deadlineStatus() == DeadlineStatus::Missed) {
                    std::cout << "  - '" << task.name() << "' deadline was day "
                               << task.deadline() << ", "
                               << (task.totalDuration() - task.remaining())
                               << "/" << task.totalDuration() << " done\n";
                }
            }
        }
    }

    if (summary.incompleteTasks > 0) {
        std::cout << "\nIncomplete tasks (bottlenecks):\n";
        for (const auto& task : tasks_) {
            if (!task.isCompleted()) {
                std::cout << "  - '" << task.name() << "' [" << toString(task.type())
                           << ", prio " << task.priority() << "] "
                           << (task.totalDuration() - task.remaining())
                           << "/" << task.totalDuration() << " done\n";
            }
        }
    }
}

} // namespace pwse
