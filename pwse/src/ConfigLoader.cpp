#include "pwse/ConfigLoader.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include "pwse/Json.hpp"

namespace pwse {

SimulationConfig ConfigLoader::loadFromFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open config file: " + path);
    }
    std::ostringstream buffer;
    buffer << file.rdbuf();
    return loadFromString(buffer.str());
}

SimulationConfig ConfigLoader::loadFromString(const std::string& jsonText) {
    json::Value root = json::parse(jsonText);

    SimulationConfig config;

    const json::Value& sim = root.at("simulation");
    config.days = sim.at("days").asInt();
    if (config.days <= 0) {
        throw std::runtime_error("simulation.days must be positive");
    }

    const json::Value& agent = root.at("agent");
    config.dailyCapacity = agent.at("daily_capacity").asInt();
    if (config.dailyCapacity <= 0) {
        throw std::runtime_error("agent.daily_capacity must be positive");
    }

    const json::Value& tasks = root.at("tasks");
    for (const json::Value& t : tasks.asArray()) {
        TaskSpec spec;
        spec.name = t.at("name").asString();
        spec.duration = t.at("duration").asInt();
        spec.priority = t.at("priority").asInt();
        spec.type = taskTypeFromString(t.at("type").asString());
        spec.deadline = t.at("deadline").asInt();
        config.tasks.push_back(std::move(spec));
    }

    if (config.tasks.empty()) {
        throw std::runtime_error("Config must define at least one task");
    }

    return config;
}

} // namespace pwse
