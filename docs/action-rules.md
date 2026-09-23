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
the query width against the normalized schema's `get_arity()`. This includes any
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
raises `pyrunir.kr.ps.ext.ActionRuleContractError` if a visited tuple is
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

`SuccessorExpander::for_each_successor(state, node, statistics, emit, stop)`
visits immediate outcomes without collecting successor states. The source combines
an interned `ProgramState` with the corresponding Tyr `Node`; the node carries
the accumulated planning metric. The callback returns `true` to continue and
`false` to stop. Enumeration returns `true` only when exhausted; cancellation
or callback termination returns `false`.

An ordinary outcome is a `ProgramExecutionStep`. A Choose rule emits a
`ConceptChoice` or `RoleChoice`, retaining its effect-filtered denotation and a
cursor. Applying a choice constructs only its current binding's successor.
An enabled empty Choose remains a failed obligation. Each Choose rule keeps
its own obligation in universal search. Do rules enumerate bindings only for
their action schema and filter arguments before constructing successor states.

Nonuniversal execution stops ordinary expansion at its first compatible
outcome. Universal execution requires every ordinary outcome and one successful
binding for each Choose rule. Rules and bindings are visited in their natural
order, and enumeration supports early termination. Choose bindings follow
their denotation order. Action applicability and effect contracts are checked
only for visited tuples, so unvisited invalid tuples remain unchecked.
Query evaluation still materializes the selected relation.

`statistics.num_generated` counts emitted extended successors, including caller
returns and attempted Choose bindings. Rejected planning candidates, failure
markers, and unattempted Choose bindings do not contribute. The search owns
`num_expanded`; each program state is expanded at most once per search, and direct
expander calls do not increment it. Arrivals at an already discovered state retain
their proof edges without repeating expansion. Choose cursors iterate independently;
no state discovery or first predecessor is undone when another binding is tried.

`statistics.choice_depth` counts non-singleton Choose bindings on the first
discovered goal's predecessor path, and is zero unless the search succeeds.
It describes one solution path, including in universal mode.

## Python access

Module evaluation uses persistent `ext.GroundProgramState` or
`ext.LiftedProgramState` values produced by the successor expander, with the
corresponding `GroundEvaluationEnvironment` or `LiftedEvaluationEnvironment`.
Call `expander.initial_state(node)` with the Tyr initial node. Python exposes
`expander.for_each_successor(state, node, statistics, emit, stop)` in natural
order, with a constructible `ext.ProgramSearchStatistics` object.

Each callback receives a step or a `ConceptChoice`/`RoleChoice` by value.
Choices expose `rule`, `denotation`, `current()`, `advance()`, `exhausted()`,
and `count()`. After enumeration returns, use
`expander.apply_choice(state, node, choice, statistics)` to apply the current
binding, then advance the cursor to try another. Do not reenter the same
expander from its callback. A choice borrows its repositories; keep the task
context and program alive while using it. Accessing or advancing an exhausted
Python choice raises `IndexError`.

A planning step exposes its owned `planning_successor`; pass
`step.planning_successor.node.unpack()` with `step.target` to continue.
For a control-only step, keep the source node and use `step.target`. This
preserves accumulated metrics across Load, Choose, calls, and returns.
`matching_rule(state, node, successor)` and
`apply(state, node, rule, successor)` accept a Tyr labeled successor.

`module.get_query_features()` returns named `ext.dl.QueryFeature` values.
`ActionRule.get_query_feature()` returns the selector, and
`feature.get_expression().get_columns()` exposes its output schema.
Create a state evaluation context with `environment.make_dl_context(program_state)`.
It borrows the registers and call arguments stored in that execution state.
`ext.evaluate(feature, state_context)` returns the feature's native
denotation for every category. Boolean and numerical denotations expose their
scalar value through `.get()`; concept and role denotations are iterable.

```python
from pyrunir.kr.ps import ext

feature = module.get_query_features()[0]
columns = tuple(column.get_name() for column in feature.get_expression().get_columns())
state_context = environment.make_dl_context(program_state)
relation = ext.evaluate(feature, state_context)
snapshot = tuple(tuple(row) for row in relation)
```

`relation` is a borrowed `pyyggdrasil.database.RelationView`. Iterating it yields
read-only `RelationRow` views of Tyr object indices, with values ordered by
`columns`. Repeated evaluation in an unchanged context reuses the cached relation.
Before evaluating a different state, register binding, or argument binding, call
`environment.get_dl_caches().clear(False)`. Clear the whole cache with `.clear()`
before changing tasks or constructor repositories. Evaluation does not clear
caches automatically. Constructor repositories remain caller-owned: keep them alive
and unchanged while their entries are cached or their views are in use.
Create a new state context from the next execution state
after advancing execution. Each state context retains its execution state and environment.

Low-level `pyrunir.kr.dl.ext.semantics` contexts take
`(state, builder, denotation_repository, caches, arguments, registers)`.
Create `CallArgumentsData` for the four lists of denotation indices and
`RegisterValuesData` for optional object indices or pairs of object indices.
Intern each with `denotation_repository.get_or_create(data)` and pass the
returned `CallArguments` and `RegisterValues` views to the context. These types
are defined in `pyrunir.kr.dl.base.semantics`. Changing the source data does not
change an interned value; intern the updated data and create a new context.
Clear dynamic cache entries before evaluating the new context. Register storage
is sized from the module's declarations.

Relations, rows, and iterators retain the evaluation environment, but clearing
their cache invalidates them. Do not access an old view after clearing; evaluate
again or use a Python snapshot made before the clear, as above.
`len(relation)` is the row count and `relation.arity()` is the column count.
A zero-column query has no rows when false and one empty row when true.

Action rule serialization exposes `source`, `target`, `conditions`, `effects`,
`action_name`, and `query_feature`. Register the referenced types using the
shared dictionary registry, or select/project the desired fields as described
in the [serialization reference](serialization/index.md).
