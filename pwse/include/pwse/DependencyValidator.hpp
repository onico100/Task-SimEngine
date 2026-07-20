#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include "pwse/ConfigLoader.hpp"

namespace pwse {

// Builds a lookup from external (config-supplied) task id to its index
// within `specs`. Callers should run validateDependencies() first (or
// otherwise know ids are unique) -- this does not itself check for
// duplicates.
std::unordered_map<std::string, int> buildIdIndex(const std::vector<TaskSpec>& specs);

// Validates the dependency graph described by `specs`:
//   - every task id is present and unique
//   - every dependency refers to a task id that exists in `specs`
//   - the dependency graph contains no cycles
// Throws std::runtime_error with a descriptive message on the first
// violation found. Intended to run once, at config-load time, before
// any Task objects are constructed.
void validateDependencies(const std::vector<TaskSpec>& specs);

} // namespace pwse
