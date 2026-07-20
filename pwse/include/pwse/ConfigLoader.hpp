#pragma once

#include <optional>
#include <string>
#include <vector>
#include "pwse/Task.hpp"

namespace pwse {

// Plain-data description of a task as read from config, before it's
// turned into a live Task (which needs an assigned internal id and
// dependencies resolved to internal indices).
struct TaskSpec {
    std::string id;     // required, unique, external/config-facing identifier
    std::string name;
    int duration;
    int priority;
    TaskType type;
    std::optional<int> deadline;        // optional day number the task should finish by
    std::vector<std::string> dependencies; // ids of tasks that must complete first
};

struct SimulationConfig {
    int days = 0;
    int dailyCapacity = 0;
    std::vector<TaskSpec> tasks;
};

class ConfigLoader {
public:
    // Reads and parses the config file at `path`. Throws std::runtime_error
    // (file I/O issues) or std::exception-derived JSON/schema errors on
    // malformed input, including invalid or cyclic task dependencies.
    static SimulationConfig loadFromFile(const std::string& path);

    // Parses config from an already-read JSON string (useful for testing).
    static SimulationConfig loadFromString(const std::string& jsonText);
};

} // namespace pwse
