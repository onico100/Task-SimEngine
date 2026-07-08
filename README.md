# Personal Workload Simulation Engine

Simulates how a single agent (you) manages study/work/life tasks over time
under limited daily capacity, to surface completion timing, delays, and
overload/bottleneck days.

## Build

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

This produces a `sim` executable (also directly buildable with:
`g++ -std=c++17 -Iinclude src/*.cpp -o sim`).

## Run

```bash
./sim run --config ../configs/example.json
```

## Config format

```json
{
  "simulation": { "days": 10 },
  "agent": { "daily_capacity": 8 },
  "tasks": [
    { "name": "Write Thesis Chapter", "duration": 20, "priority": 9, "type": "study" }
  ]
}
```

- `duration` / `daily_capacity` share a unit (e.g. hours). A task with
  `duration: 20` and a capacity of 8/day takes at least 3 days of full
  attention if nothing else competes for time.
- `priority`: higher number = scheduled first.
- `type`: one of `study`, `work`, `life`.

## Architecture

| Component          | Responsibility                                              |
|---------------------|--------------------------------------------------------------|
| `Task`              | Data + state machine (pending → in_progress → completed)    |
| `Agent`             | Daily capacity budget, reset each tick                      |
| `Scheduler`         | Greedy priority-first allocation of capacity to tasks        |
| `SimulationEngine`  | Owns the tick loop, applies scheduling, prints output/summary|
| `ConfigLoader`      | JSON config → `SimulationConfig`                             |
| `json` (vendored)   | Minimal dependency-free JSON parser                          |

Scheduling logic is intentionally separated from the engine's loop and
from I/O, so it can be swapped (e.g. deadline-aware, fairness-aware) or
observed by a future event/logging layer without touching the rest of
the system. `SimulationEngine::run()` returns a `SimulationSummary`
struct (not just printed text), so later features like scenario
comparison can consume it programmatically.

## Scheduling algorithm (MVP)

Each day:
1. Reset the agent's capacity to `daily_capacity`.
2. Sort all incomplete tasks by priority descending (ties broken by
   insertion order for determinism).
3. Walk the sorted list, giving each task as much of the remaining
   capacity as it can absorb, until capacity or tasks run out.
4. Any task that wanted capacity but got none is recorded as "starved"
   for that day (used to flag overload days and bottlenecks).

This is intentionally simple (no deadlines, no interruption cost, no
task dependencies) — the goal for Phase 1 is a correct, observable
baseline to extend later.

## Planned extensions (not in MVP)

- Event system: structured timeline of scheduling decisions instead of
  direct stdout prints, to support logging/observability.
- AI-generated explanations of *why* a schedule played out a certain way.
- Random/parameterized scenario generation.
- Comparing two schedules/configs against each other.

The current design supports these without rearchitecting: `Scheduler`
already returns structured `ScheduleResult` data (not just prints), and
`SimulationEngine::run()` returns a `SimulationSummary` value rather than
only writing to stdout.
