#pragma once

#include <string>
#include <vector>
#include "pwse/Task.hpp"

namespace pwse {

// Plain-data description of a task as read from config, before it's
// turned into a live Task (which needs an assigned id).
struct TaskSpec {
    std::string name;
    int duration;
    int priority;
    TaskType type;
    int deadline = -1; // -1 means "no deadline configured"
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
    // malformed input.
    static SimulationConfig loadFromFile(const std::string& path);

    // Parses config from an already-read JSON string (useful for testing).
    static SimulationConfig loadFromString(const std::string& jsonText);
};

} // namespace pwse
