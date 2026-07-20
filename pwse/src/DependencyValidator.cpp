#include "pwse/DependencyValidator.hpp"

#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <unordered_set>

namespace pwse {

std::unordered_map<std::string, int> buildIdIndex(const std::vector<TaskSpec>& specs) {
    std::unordered_map<std::string, int> index;
    index.reserve(specs.size());
    for (std::size_t i = 0; i < specs.size(); ++i) {
        index[specs[i].id] = static_cast<int>(i);
    }
    return index;
}

namespace {

// Standard three-color DFS cycle detection:
//   Unvisited -> not yet reached
//   InProgress -> currently on the DFS stack (ancestor of the current node)
//   Done -> fully explored, known cycle-free
enum class VisitState { Unvisited, InProgress, Done };

// Explores dependencies of `nodeIndex` looking for a back-edge to an
// in-progress ancestor. `stack` mirrors the current DFS path (by id) so
// that, on finding a cycle, we can report the exact loop rather than just
// "a cycle exists somewhere."
void visit(int nodeIndex,
           const std::vector<TaskSpec>& specs,
           const std::unordered_map<std::string, int>& idIndex,
           std::vector<VisitState>& state,
           std::vector<std::string>& stack) {
    state[nodeIndex] = VisitState::InProgress;
    stack.push_back(specs[nodeIndex].id);

    for (const std::string& depId : specs[nodeIndex].dependencies) {
        int depIndex = idIndex.at(depId); // existence already validated by caller

        if (state[depIndex] == VisitState::InProgress) {
            std::ostringstream oss;
            oss << "Circular dependency detected: ";
            auto cycleStart = std::find(stack.begin(), stack.end(), specs[depIndex].id);
            for (auto it = cycleStart; it != stack.end(); ++it) {
                oss << *it << " -> ";
            }
            oss << specs[depIndex].id;
            throw std::runtime_error(oss.str());
        }

        if (state[depIndex] == VisitState::Unvisited) {
            visit(depIndex, specs, idIndex, state, stack);
        }
    }

    stack.pop_back();
    state[nodeIndex] = VisitState::Done;
}

} // namespace

void validateDependencies(const std::vector<TaskSpec>& specs) {
    // 1. Every task must have a non-empty, unique id.
    std::unordered_set<std::string> seenIds;
    seenIds.reserve(specs.size());
    for (const auto& spec : specs) {
        if (spec.id.empty()) {
            throw std::runtime_error(
                "Task '" + spec.name + "' is missing a required 'id' field");
        }
        if (!seenIds.insert(spec.id).second) {
            throw std::runtime_error("Duplicate task id: '" + spec.id + "'");
        }
    }

    // 2. Every dependency must reference an id that actually exists.
    auto idIndex = buildIdIndex(specs);
    for (const auto& spec : specs) {
        for (const auto& depId : spec.dependencies) {
            if (idIndex.find(depId) == idIndex.end()) {
                throw std::runtime_error(
                    "Task '" + spec.id + "' depends on unknown task id '" + depId + "'");
            }
        }
    }

    // 3. The dependency graph must be acyclic.
    std::vector<VisitState> state(specs.size(), VisitState::Unvisited);
    std::vector<std::string> stack;
    for (std::size_t i = 0; i < specs.size(); ++i) {
        if (state[i] == VisitState::Unvisited) {
            visit(static_cast<int>(i), specs, idIndex, state, stack);
        }
    }
}

} // namespace pwse
