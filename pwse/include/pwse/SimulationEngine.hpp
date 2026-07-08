#pragma once

#include <vector>
#include "pwse/Task.hpp"
#include "pwse/Agent.hpp"
#include "pwse/Scheduler.hpp"
#include "pwse/ConfigLoader.hpp"

namespace pwse {

// Aggregate outcome of the whole simulation run.
struct SimulationSummary {
    int totalDays = 0;
    int totalTasks = 0;
    int completedTasks = 0;
    int incompleteTasks = 0;
    int overloadDays = 0;      // days where at least one task was starved of capacity
    double avgCompletionDay = 0.0; // average day-of-completion across completed tasks
};

// Owns the world state (agent + tasks) and drives the tick loop.
// Printing is done inline for MVP; the per-tick and final-summary output
// points are isolated so they can later be redirected to an event/log
// system without touching the scheduling logic.
class SimulationEngine {
public:
    explicit SimulationEngine(const SimulationConfig& config);

    // Runs the full simulation (config.days ticks) and prints per-step
    // output plus a final summary to stdout. Returns the summary for
    // programmatic use (e.g. tests, future comparison features).
    SimulationSummary run();

private:
    void runOneDay(int day); // returns via overloadDayCount_ side-effect
    void printDayHeader(int day) const;
    void printSummary(const SimulationSummary& summary) const;

    Agent agent_;
    Scheduler scheduler_;
    std::vector<Task> tasks_;
    int totalDays_;
    int overloadDayCount_ = 0;
};

} // namespace pwse
