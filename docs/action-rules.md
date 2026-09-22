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

Public `control_steps` and `steps` enumerate all applicable rule outcomes. They
validate every selected Action tuple they visit. Query evaluation still produces
the selected relation; successor-state construction can stop earlier during
search.

The expansion layer specializes `EagerExpansionPolicy` and
`LazyExpansionPolicy` while sharing the executor state and proof search. C++
callers can select a policy through
`expander.steps_until<LazyExpansionPolicy>(state, stop, steps)`; calls without
an explicit policy use `EagerExpansionPolicy`.

Nonuniversal execution with `shuffle_choice_points=False` stops ordinary rule
expansion after its first compatible outcome, across Load, Do, Sketch, and
Action rules. Choose rules retain the selected rule's full binding alternatives
for backtracking. Later rule outcomes are left unevaluated, so a later invalid
Action tuple can remain unvisited. Universal execution and execution with
shuffling use eager expansion. Shuffle behavior retains the existing seeded
choice ordering.

## Python access

Module evaluation uses persistent `ext.GroundExecutionState` or
`ext.LiftedExecutionState` values produced by the successor expander, with the
corresponding `GroundEvaluationEnvironment` or `LiftedEvaluationEnvironment`.

`module.get_query_features()` returns named `ext.dl.QueryFeature` values.
`ActionRule.get_query_feature()` returns the selector, and
`feature.get_expression().get_columns()` exposes its output schema.
Create an argument owner with
`ext.EvaluationArguments(execution_state.call_stack.arguments)`, then create a
state evaluation context with `environment.make_dl_context(execution_state, arguments)`.
`ext.evaluate(feature, state_context)` returns the feature's native
denotation for every category. Boolean and numerical denotations expose their
scalar value through `.get()`; concept and role denotations are iterable.

```python
from pyrunir.kr.ps import ext

feature = module.get_query_features()[0]
columns = tuple(column.get_name() for column in feature.get_expression().get_columns())
arguments = ext.EvaluationArguments(execution_state.call_stack.arguments)
state_context = environment.make_dl_context(execution_state, arguments)
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
after advancing execution, with an argument owner for that state's call arguments.
Each state context retains its argument owner, execution state, and environment.

Relations, rows, and iterators retain the evaluation environment, but clearing
their cache invalidates them. Do not access an old view after clearing; evaluate
again or use a Python snapshot made before the clear, as above.
`len(relation)` is the row count and `relation.arity()` is the column count.
A zero-column query has no rows when false and one empty row when true.

Action rule serialization exposes `source`, `target`, `conditions`, `effects`,
`action_name`, and `query_feature`. Register the referenced types using the
shared dictionary registry, or select/project the desired fields as described
in the [serialization reference](serialization/index.md).
