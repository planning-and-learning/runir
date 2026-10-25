# Exploring Choose rules

`pyrunir.kr.ps.ext.explore` answers, for an Ext program on a set of lifted tasks,
the questions that decide whether a Choose rule is needed: which bindings fail,
which feature separates successful from failed bindings, and whether a Load would
do. `pyrunir.kr.ps.ext.explore.server` serves the same API as an MCP server
(`pip install pyrunir[mcp]`).

## Session

```python
from pyrunir.kr.ps.ext.explore import Budget, Session, SessionOptions

options = SessionOptions(budget=Budget(max_states=100_000, max_seconds=10.0))
session = Session(domain_path, program_text, task_paths, options=options)
rule = session.rule("pick_dest")                  # names are resolved once, at the boundary
for point in session.points(rule):
    print(point, *session.alternatives(point), sep="\n  ")
print(session.contrast(rule))
print(session.separation(session.feature("(b_nonempty (c_and (c_register dest) (c_atomic_state \"cellspace\")))"), session.contrast(rule)))
print(session.try_load(rule))
```

| Call | Result |
|---|---|
| `failures(tasks)` | every task whose execution fails: failure vertex, memory, registers, actions no rule admits there |
| `run(task)` | status, failure category, statistics, failure vertex, path to the goal or the failure |
| `chooses(tasks)` | per Choose rule: occurrences on paths, width, failed and untried bindings |
| `points(rule, tasks, limit)` | choice points on paths, those with failed bindings and more bindings first |
| `alternatives(point, budget)` | every binding in `:order` rank: outcome, how it ended, subtree size, feature deltas |
| `contrast(rule, tasks, budget, max_points)` | (successful, failed) binding pairs at the same points, declared features ranked by how they order them |
| `separation(feature, contrast)` | the same comparison for one declared feature or a new expression |
| `try_load(rule, tasks)` | execute and prove with the rule's Choose replaced by a Load |
| `vertex(ref)` | memory state, registers, Boolean and numerical features, fluent atoms |
| `successors(ref)` | applicable actions, the rule admitting each there (or none), feature deltas |
| `dead_end(task, budget)` | the step on the failure path after which the task is unsolvable (binary search with plan searches; dead ends are absorbing) |
| `plan(ref, budget)` | a lazy GBFS/FF plan from the vertex's state to the goal, feature deltas per step |
| `termination()` | structural termination; per non-terminating module the surviving rules and one abstract cycle |
| `validate(tasks)` | execute and prove every task, as a learning loop validates a proposal; reports the slowest execution time per expanded state |

Each task is searched once, lazily, with `StateMemorization.ALL`, so the graph keeps
failed branches. Vertex status is computed in postorder: a vertex with Choose edges
succeeds if one tried binding does, any other vertex needs all its successors; leaves
are goals, open or dead-end states, or unexplored (unknown).

Bindings come from `LiftedSuccessorExpander`, so they are exactly the ones the
search would try, in `:order` rank. A tried binding's outcome is its subtree's
status. An untried binding is searched only when a budget is given, by a search
with `options.start_state = target`, from its target configuration.

Features follow the program's semantics. Boolean and numerical features are evaluated
in the configuration after binding, like `:order`. Concept features are evaluated
at the Choose source, where candidates are computed; their value for a binding is
membership. `contrast` compares a successful with a failed binding at the same point,
so features that only differ between points do not count; a feature that is higher
(or lower) on every pair is `consistent` and could decide the Choose.

`try_load` rewrites the printed program (`str(program)` round-trips through the
parser): every Choose body of the rule becomes a Load with the same conditions,
candidates, register and effects, and its `:order` is dropped.

Every search draws from the session's `total_seconds`; once used up, calls raise
`BudgetExhaustedError`.

## Additions to the core API

- `ProgramSearchOptions::start_state` (optional, like tyr's `start_node`) makes
  `find_solution` continue from an interned program state of the same task and
  program, e.g. a proof graph vertex. Dynamic goals are
  still judged against the task's initial state; no plan is returned.
- `parse_concept/role/boolean/numerical(text, domain, module, repository)` parse a
  standalone expression in the scope of a module, so registers and arguments may be
  referenced by name.

## MCP server

```
python -m pyrunir.kr.ps.ext.explore.server [--domain domain.pddl] [--program module-program.txt]
    [--tasks 'problems/*.pddl'] [--max-states N] [--max-seconds S] [--total-seconds S] [--workers N]
```

Executions and universal proofs are both explorable: `run`, `failures` and `dead_end` take `prove`, and
proof vertices are written `<task>:pv<n>`, which `vertex`, `successors`, `plan` and `alternatives` accept.

One session per server process. `open(workspace, program=None)` loads the files relative to the
workspace (`program` replaces a workspace program that does not parse); `set_program(text)` switches to a draft. Handles are strings: tasks by
name, vertices as `<task>:v<n>`, rules and features by name, features also as
expressions. Every call is logged to stderr with its duration and the search time used.
