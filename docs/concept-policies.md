# Indexical concept policies

`runir::kr::ps::icp` and `pyrunir.kr.ps.icp` implement a separate policy family
with one module, memory states, registers, Load rules, and concept rules.
Features use the complete Ext DL language, including concept and role registers
and queries. Standard Boolean/numerical conditions and effects have the same
meaning as in Ext. Structural termination analysis is not implemented.

## Rules

A concept rule names an action and binds its original action parameters by
position. Bindings introduced internally by action normalization are excluded.
For example, a Gripper rule can pick up a ball:

```lisp
(:rule (:symbol pick) (:expression
  (:source-memory m0) (:target-memory m0)
  (:crule (:action "pick") (:arguments ball room gripper)
    (:conditions)
    (:xconditions
      (not-belongs (:argument ball) (:concept carried)))
    (:xeffects
      (enter (:argument ball) (:concept carried)))
    (:effects (increases carried_count)))))
```

Here `carried` is a concept feature and `carried_count` is a numerical feature.
`belongs` and `not-belongs` test membership in the source configuration;
`enter` and `exit` test a change of membership across the transition. Subjects
can also be loaded concept registers, for example
`(belongs (:register (:concept selected)) (:concept carried))`.
Every direct indexical test on an unset register is false, including
`not-belongs`. An unset register still denotes the empty set inside DL expressions.

At least one original action argument must enter a declared concept on each
admitted concept-rule transition. This requirement also applies when
`:xeffects` is empty.

Loads branch over the elements of a concept or role and assign the chosen value
to a register. They can constrain the resulting feature change:

```lisp
(:load (:conditions) (:concept candidates)
  (:register (:concept selected))
  (:effects (increases selected_count)))
```

Role Loads use `(:role feature)` and `(:register (:role register))`.
Load effects compare source and target register assignments in the same
planning state. Concept-rule effects compare source and target planning states
with unchanged registers. Missing or empty `:effects` imposes no constraint.
An enabled Load from an empty denotation is an explicit failing alternative.
Rejected action candidates contribute no successor.

## Histories and resets

Each declared concept has an initially empty history of objects that entered it.
For each candidate transition, compute its newly entering objects using both
the planning state and register assignment. Reject the transition if any newly
entering object is already in that concept's source history. This applies to
Loads as well as concept rules.

After admission, add entries to histories, then apply resets. A module ends
with `(:reset-spo ...)`, which may be empty. `(:prec C D)` means that any change
to `D` clears `C`'s history. Precedence is transitive and must be acyclic.
Entries and exits both count as changes; a reset overrides history additions
made on the same transition. The reset cannot make an otherwise forbidden
re-entry admissible because admission checks the source histories.

Memory, registers, histories, and planning state all participate in execution
state identity. Two configurations with the same planning state can therefore
have different legal successors.

## Python execution

```python
from pyrunir.kr.ps import icp

program = icp.dl.parse_program(
    description, planning_domain, task_context.domain_context.icp_repository
)
options = icp.GroundProgramSearchOptions()
options.universal = False
result = icp.find_ground_solution(task_context, program, options)
```

Use the corresponding `Lifted` options and `find_lifted_solution` for lifted
tasks. `universal=False` follows the first admitted outcome in canonical rule
and binding order without backtracking. A failed rollout can coexist with
another successful alternative. Successful greedy execution returns a plan;
Load transitions do not contribute planning actions.

`universal=True` explores every reachable outcome and reports a proof graph,
dead ends, open states, and cycles. A reachable cycle fails universal execution
even when it has a goal exit. Goals terminate immediately before applying
rules. Time and state limits bound both modes; the state limit counts complete
ICP configurations, including the initial one.

Programs require one module with empty module arguments, at most four concept
registers and four role registers, and a reset specification after its rules.
Neither module calls nor structural termination analysis are part of this family.
