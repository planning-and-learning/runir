# Query features and Action rules

Extended modules can declare named query features and use their complete rows
as action parameter tuples. Every selected tuple must be applicable. A query
can preserve relationships between parameters, such as an edge's source and target, without taking a Cartesian
product of separate concept denotations.

```lisp
(:module
  (:symbol navigate) (:arguments) (:registers)
  (:entry moving) (:memory moving)
  (:features
    (:query (:symbol moves)
      (:expression
        (q_join (q_atomic_state "at" (from))
                (q_atomic_state "edge" (from to)))))
    (:numerical (:symbol occupied)
      (:expression (n_count (c_atomic_state "at")))))
  (:rules
    (:rule (:symbol move-selected)
      (:expression
        (:source-memory moving) (:target-memory moving)
        (:action
          (:conditions)
          (:action "move")
          (:query moves)
          (:effects (unchanged occupied)))))))
```

Here `move` has two parameters. Each query row supplies the complete normalized
action binding, in schema-variable order (`get_variables()`). The parser checks
that the action exists in the supplied domain and that the query width matches
the normalized schema's `get_arity()`. This includes any
existential witness variables introduced during Tyr normalization. Use
`q_project` to reorder output columns and `q_rename` to rename them;
column names need not equal the PDDL parameter names. Query features belong to
extended modules and are not Boolean or numerical conditions/effects or module
call arguments. Queries can read the module's existing registers and arguments.

An empty query selects no tuple. A query with no columns distinguishes false
(no rows) from true (one empty row), allowing zero-parameter actions. Conditions
and the current memory state must also match before an Action rule is used.

## Effect contract

The Action rule's `:effects` is an invariant over its selected applicable
transitions. For every relevant source state and module register/argument
valuation satisfying the rule's conditions, **every selected applicable tuple**
must produce a successor satisfying the declared effects. Selected tuples must
also satisfy the action's types and preconditions. Unmentioned features
remain unconstrained. In the example, moving preserves the number of occupied
positions.

Execution checks applicability and the effects on transitions it visits. It
raises `std::logic_error` in C++ (`RuntimeError` in Python) if a visited tuple is
inapplicable or its successor violates the declaration.
A violation is an error, not an alternative to silently discard. Successful
execution checks only the transitions actually visited; it does not verify the
contract over all possible tasks, states, or query rows.

Structural termination uses the same condition/effect abstraction as a Do rule
with identical conditions, effects, and memory endpoints. A concrete forward
termination guarantee for Action rules is conditional on their universal effect
contract. Structural analysis does not prove the query/action-effect contract,
and forward termination does not establish completeness of backtracking search.

## Enumeration and search

Ext `find_solution` requires a structurally terminating whole program and
rejects programs without that certificate before execution. This precondition
does not apply to direct successor-expander calls.

`SuccessorExpander::for_each_successor(state, statistics, emit, stop)`
visits immediate outcomes without collecting successor states. The interned
`ProgramState` contains the planning state and the module's control, registers,
arguments, and call stack. The callback returns `true` to continue and
`false` to stop. Enumeration returns `true` only when exhausted; cancellation
or callback termination returns `false`.

An ordinary outcome is a `ProgramExecutionStep`. A Choose rule emits a
`ConceptChoice` or `RoleChoice`, retaining its effect-filtered bindings in a
pooled vector and a current position. All memorization modes use this same
representation; clearing evaluation caches does not invalidate pending choices.
Applying a choice constructs only its current binding's successor.
An enabled empty Choose remains a failed obligation. Each Choose rule keeps
its own obligation in universal search. Do rules enumerate bindings only for
their action schema and filter arguments before constructing successor states.

Nonuniversal execution stops ordinary expansion at its first compatible
outcome. Universal execution requires every ordinary outcome and one successful
binding for each Choose rule. Rule occurrences retain their existing order.
Action tuples follow their relation's canonical row order; Choose bindings
follow their denotation order. Enumeration supports early termination.
Action applicability and effect contracts are checked
only for visited tuples, so unvisited invalid tuples remain unchecked.
Query evaluation still materializes the selected relation.

`statistics.num_generated` counts emitted extended successors, including caller
returns and attempted Choose bindings. Rejected planning candidates, failure
markers, and unattempted Choose bindings do not contribute. The search owns
`num_expanded`; direct expander calls do not increment it. The default
`StateMemorization.ALL` expands each program state at most once and retains every
diagnostic edge. `NONE` and `CHOICE` can repeat non-memoized states and return only
the selected solution or diagnostic path, materialized after search. See
[state memorization and returned paths](proof-search.md#state-memorization-and-returned-paths)
for the storage and state-limit contract. Choose cursors iterate independently.

`statistics.choice_depth` counts non-singleton Choose bindings on the first
discovered goal's predecessor path. `statistics.choice_width` is the maximum
number of bindings at any Choose on that same path, after effect filtering.
A singleton Choose has width 1; a path without Choose has width 0. Both
statistics are zero unless the search succeeds, and describe one solution path,
including in universal mode. Choices on abandoned branches do not contribute.

## Python access

Module evaluation uses persistent `ext.GroundProgramState` or
`ext.LiftedProgramState` values produced by the successor expander, with the
corresponding `GroundEvaluationEnvironment` or `LiftedEvaluationEnvironment`.
Call `expander.initial_state(planning_state)` with a Tyr planning state, such as
`initial_node.get_state()`. Python exposes
`expander.for_each_successor(state, statistics, emit, stop)` in natural
order, with a constructible `ext.ProgramSearchStatistics` object.

Each callback receives a step or a `ConceptChoice`/`RoleChoice` by value.
Choices expose `rule`, `bindings`, `current()`, `advance()`, `exhausted()`,
and `count()`. After enumeration returns, use
`expander.apply_choice(state, choice, statistics)` to apply the current
binding, then advance the cursor to try another. Do not reenter the same
expander from its callback. In C++, a choice must be destroyed before its
expander, which owns the binding pool. Python choices retain their expander, and
binding views retain their choice. Accessing or advancing an exhausted Python
choice raises `IndexError`.

Continue execution with `step.target` for both planning and control-only steps.
A planning step also exposes its owned `planning_successor`.
`matching_rule(state, successor)` and `apply(state, rule, successor)` accept a
Tyr labeled successor. Ext expansion uses states without accumulated metrics;
successful plans obtain cumulative metrics when their actions are replayed by Tyr.

`module.get_query_features()` returns named `ext.dl.QueryFeature` values.
`ActionRule.get_query_feature()` returns the selector, and
`feature.get_expression().get_columns()` exposes its output schema.
Create a state evaluation context with `environment.make_dl_context(program_state)`.
It borrows the registers and call arguments stored in that execution state.
`ext.evaluate(feature, state_context)` returns the feature's native
denotation for every category. Boolean and numerical denotations expose their
scalar value through `.get()`; concept and role denotations are iterable.

Argument features return views of the stored call arguments without copying
their values into the feature cache. In every memorization mode, final argument
values are evaluated in the caller's state and interned with their argument
bundle in the task's denotation repository. Pooled execution states and saved
callers borrow an indexed `CallArgumentsView` into that repository. Clearing feature caches does not invalidate argument views;
argument bundles and their final denotations remain until the task repository
is cleared or destroyed.

The C++ execution and evaluation interfaces accept indexed, borrowed-data and
builder views through semantic view concepts. In `NONE` and `CHOICE`, paths and
memo entries retain program builders that own pooled execution values; views
borrow those values during evaluation. Builders may move as search buffers grow,
so borrowed views must not outlive their owners or survive a move of the viewed
builder. Only returned witness states are copied into the result repositories.

```python
from pyrunir.kr.ps import ext

feature = module.get_query_features()[0]
columns = tuple(column.get_name() for column in feature.get_expression().get_columns())
state_context = environment.make_dl_context(program_state)
relation = ext.evaluate(feature, state_context)
snapshot = tuple(tuple(int(object_.get_index()) for object_ in row) for row in relation)
```

`relation` is an interned `pyrunir.kr.dl.base.semantics.QueryDenotation`,
shared by all DL families. Import shared query result types and
`QueryColumnData`/`QueryColumnIndex` from `pyrunir.kr.dl.base.semantics`.
The relation exposes `get_index()`, iteration, row access, and
shape operations directly. Each `QueryDenotationRow` lazily yields the existing
`pytyr.formalism.planning.Object` views, with values ordered by `columns`.
Use `object_.get_name()` to inspect a value or `int(object_.get_index())` for an
independent numeric snapshot, as above.
Interning preserves column order; rows form a canonical set whose iteration
order follows interned row identities rather than insertion order.
Repeated evaluation in an unchanged context reuses the cached query identity.
Rule occurrence order is unchanged, but canonical tuple order can change which
binding a nonuniversal search visits first.

Before evaluating a different source state, register binding, or argument
binding, call `environment.reset_source()` and create a context for the new
configuration. `environment.reset_target()` resets target evaluations between
candidate transitions. These operations reset dynamic memo entries and their
owned result repositories while preserving static results. Context creation
alone does not reset anything. Each Python context retains its execution state
and environment; constructor repositories must also stay alive and unchanged.

Low-level `pyrunir.kr.dl.ext.semantics` contexts take
`(state, builder, storage, arguments, registers)`, or
`(state, builder, caches, denotation_repository, intermediates, arguments, registers)`
when the final result needs different ownership from its children. Construct a
reusable `EvaluationStorage(denotation_repository)` and pass it directly for
ordinary evaluation. The builder owns the shared DL evaluation workspace.
Every expression uses `expression.evaluate(context)`;
the context selects where computed results are interned. See
[KR evaluation storage](kr-evaluation.md) for durable results and reset
rules.

`CallArgumentsData` holds the four lists of denotation indices;
`RegisterValuesData` holds optional object indices or pairs of indices. Both are
defined in `pyrunir.kr.dl.base.semantics`. Evaluate call-argument roots with
separate caches and the task's durable denotation repository, then intern each
bundle with `arguments, inserted = denotation_repository.insert(data)`. Pass the resulting
`CallArguments` and `RegisterValues` views to the context. Argument expressions
pass through existing argument views, so supplied argument indices must already
refer to the task repository. Changing source data does not change an interned
value; intern the updated data and reset evaluation for the new context.
Register storage is sized from the module's declarations.

Denotations, query results, rows, and iterators retain their Python owners, but
resetting an owning result repository invalidates its old views. Memo-only
`DenotationCaches.reset_dynamic()` and `.reset_all()` leave result storage
intact. Use storage or environment resets to release reusable evaluation
results, and do not access those views afterward; evaluate again or take a
Python snapshot first, as above.
`len(relation)` is the row count and `relation.arity()` is the column count.
A zero-column query has no rows when false and one empty row when true.

Action rule serialization exposes `source`, `target`, `conditions`, `effects`,
`action_name`, and `query_feature`. Register the referenced types using the
shared dictionary registry, or select/project the desired fields as described
in the [serialization reference](serialization/index.md).
