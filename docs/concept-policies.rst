Indexical concept policies
==========================

``runir::kr::ps::icp`` and ``pyrunir.kr.ps.icp`` implement a separate policy family: one module with memory states,
registers, Load rules and concept rules. Features use the DL language with concept and role registers and queries.
Boolean and numerical conditions and effects mean what they mean in Ext.

A program is one module with empty module arguments, at most four concept registers and four role registers, and a
reset specification after its rules. The family has no module calls and no structural termination analysis.

Rules
-----

A concept rule names an action and binds its original action parameters by position; bindings that action
normalization introduces are excluded. For example, a Gripper rule can pick up a ball:

.. code:: lisp

   (:rule (:symbol pick) (:expression
     (:source-memory m0) (:target-memory m0)
     (:crule (:action "pick") (:arguments ball room gripper)
       (:conditions)
       (:xconditions
         (not-belongs (:argument ball) (:concept carried)))
       (:xeffects
         (enter (:argument ball) (:concept carried)))
       (:effects (increases carried_count)))))

Here ``carried`` is a concept feature and ``carried_count`` a numerical feature.

- ``belongs`` and ``not-belongs`` test membership in the source configuration; ``enter`` and ``exit`` test a change
  of membership across the transition.
- A subject can also be a loaded concept register: ``(belongs (:register (:concept selected)) (:concept carried))``.
  Every direct indexical test on an unset register is false, including ``not-belongs``; inside DL expressions an
  unset register denotes the empty set.
- On each admitted concept-rule transition at least one original action argument must enter a declared concept, also
  when ``:xeffects`` is empty.

A Load branches over the elements of a concept or role and assigns the chosen value to a register. It can constrain
the resulting feature change:

.. code:: lisp

   (:load (:conditions) (:concept candidates)
     (:register (:concept selected))
     (:effects (increases selected_count)))

Role Loads use ``(:role feature)`` and ``(:register (:role register))``. Load effects compare the source and target
register assignments in the same planning state; concept-rule effects compare the source and target planning states
with unchanged registers. Missing or empty ``:effects`` constrain nothing. An enabled Load from an empty denotation is
an explicit failing alternative; rejected action candidates contribute no successor.

Histories and resets
--------------------

Each declared concept has a history, initially empty, of the objects that entered it.

1. For a candidate transition, compute the objects that newly enter each concept, from the planning state and the
   register assignment. Reject the transition if one of them is already in that concept's source history. This holds
   for Loads and concept rules.
2. After admission, add the entries to the histories, then apply the resets.

A module ends with ``(:reset-spo ...)``, which may be empty. ``(:prec C D)`` means that any change to ``D``, an entry
or an exit, clears ``C``'s history. Precedence is transitive and must be acyclic. A reset overrides the history
additions of the same transition, but cannot make a forbidden re-entry admissible, because admission checks the
source histories.

Memory, registers, histories and the planning state all belong to an execution state's identity, so two
configurations with the same planning state can have different legal successors.

Python execution
----------------

.. code:: python

   from pyrunir.kr.ps import icp

   program = icp.dl.parse_program(
       description, planning_domain, task_context.domain_context.icp_repository
   )
   options = icp.GroundProgramSearchOptions()
   options.universal = False
   result = icp.find_ground_solution(task_context, program, options)

Use the ``Lifted`` options and ``find_lifted_solution`` for lifted tasks.

- ``universal=False`` follows the first admitted outcome in canonical rule and binding order, without backtracking,
  so a rollout can fail although another alternative would succeed. Success returns a plan; Load transitions
  contribute no planning actions.
- ``universal=True`` explores every reachable outcome and reports a proof graph, dead ends, open states and cycles.
  A reachable cycle fails universal execution even when it has a goal exit.
- Goals terminate before any rule is applied. Time and state limits bound both modes; the state limit counts complete
  ICP configurations, including the initial one.
