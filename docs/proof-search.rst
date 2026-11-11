Program proof search
====================

Ext ``find_solution`` requires a structurally terminating whole program. It checks this before execution and raises
``std::invalid_argument`` (``ValueError`` in Python) when termination is not established; a resource limit of the
structural analysis is an error too, not a certificate. This holds for grounded and lifted execution, universal or
not.

With this precondition, concrete execution has no cycles, and results are computed in depth-first postorder:

- One ``depth_first_search`` (``include/runir/kr/ps/ext/detail/proof_search.hpp``) handles greedy execution,
  backtracking and universal proofs in every storage mode.
- ``SearchStorage<Kind, StateMemorization>`` (``detail/search_storage.hpp``) specializes admission, memoization and
  retained transitions per mode. The mode is selected once at the entry point; the search checks none at run time.
- Pooled ``SearchPath`` records (``detail/search_path.hpp``) keep the active prefixes and the selected witnesses.

The diagnostic graph and the optional plan are constructed afterward. Failed alternatives do not invalidate a
successful Choose.

Feature evaluation
------------------

``RUNIR_DELTA_EVALUATION`` (default ``ON``) selects ``DeltaEvaluationPolicy<Kind>`` for Base, Ext and ICP proof search
and for UNS classification during search; ``-DRUNIR_DELTA_EVALUATION=OFF`` selects ``FullEvaluationPolicy<Kind>`` with
memoized, interned results. Both live in ``runir::kr::dl::semantics`` and satisfy
``EvaluationPolicyConcept<Policy, Kind>``; C++ expanders can also name a policy explicitly.

Delta evaluation prepares separate source and target graphs from the feature catalogs of the sketch, module or
classifier; every rule feature must be declared in its catalog. It keeps input snapshots, computes the state and
register differences between evaluations, and reuses result buffers across forward steps and backtracking.

- A change of module, caller or arguments reinitializes the dynamic results; task-static results stay.
- Call arguments use full evaluation and interning, because their results must survive the caller.
- Distances whose edges are a query (``n_distance`` over k-tuples) are not maintained from deltas, since that would
  materialize every edge. Such a node and every node depending on it are recomputed from the state on demand.
  Distances over role edges are maintained incrementally.
- Incremental row updates can change the Action enumeration order, so plans, traversal statistics and
  resource-limited outcomes may differ from full evaluation.

.. _state-memorization:

State memorization and returned paths
-------------------------------------

``ProgramSearchOptions.state_memorization`` controls the reuse of completed search results; the default is
``StateMemorization.ALL``.

.. list-table::
   :header-rows: 1

   * - Mode
     - States memoized during search
     - Returned graph
   * - ``NONE``
     - None; a shared continuation can be evaluated again.
     - Only the selected solution or diagnostic path.
   * - ``CHOICE``
     - Interned source states with admitted Choose obligations, including singleton and empty choices.
     - Only the selected solution or diagnostic path.
   * - ``ALL``
     - Every admitted program state; shared continuations reuse their completed result.
     - The full explored graph, including failed alternatives and repeated transitions.

.. code:: python

   options = ext.LiftedProgramSearchOptions()
   options.state_memorization = ext.StateMemorization.CHOICE
   result = ext.find_lifted_solution(task_context, program, options)

The modes change neither the AND/OR semantics nor the state and control data needed for execution and backtracking.

- **CHOICE.** The memoized result belongs to the source state's combined continuations under the selected search
  mode: in universal search every ordinary successor and every Choose rule, with AND backtracking one successful
  continuation. The first admitted Choose interns the complete source program state, with registers and caller
  frames, and its ``ProgramStateView`` is the memo key. Choice sources stay in the task repository even when the
  returned graph does not contain them.
- **Returned path.** ``NONE`` and ``CHOICE`` materialize the selected path after search, so graph labels and a
  returned plan stay valid once the search's builders are released: ``NONE`` does not mean a result without states.
- **Plans.** Only successful nonuniversal execution returns a plan; universal success and failures can still return
  a diagnostic graph.
- **Failures.** The reduced graph contains one failing path that execution reached. This can be a rejected Choose
  binding even when another binding later satisfied that Choose. It is an example: it neither shows that every
  alternative failed nor records all obligations of a universal search. The result status reports the outcome of
  the whole search.
- **Task repository.** Caller frames (each call interns the current module's saved registers and its caller frame;
  frames form an immutable chain), call arguments and their final denotations are interned in the task repository
  in every mode, and can accumulate across calls; reduced memorization does not bound this storage. Tyr also
  registers the initial planning state for goal checks and plan reconstruction.
- **Builders.** In the reduced modes, paths and memo entries hold program builders that own pooled execution values,
  and views borrow them. Builders may move as search buffers grow, so a borrowed view must not outlive its owner or
  survive its move. Execution storage owns the pools and must outlive their handles.

AND/OR semantics
----------------

Let ``W(s)`` mean that program state ``s`` has a finite successful continuation. A goal establishes ``W(s)``
immediately. For each enabled Choose rule ``r`` at ``s``, introduce a separate obligation ``C(s,r)``:

.. code:: text

   W(t)                                      => C(s,r)  for each admitted binding s --r--> t
   W(t1) AND ... AND W(tn) AND C(s,r1) AND ... => W(s)

The second implication is enabled only after successor enumeration finishes, and only for states without a local
failure. An empty Choose stays unsatisfied; a non-goal state with no applicable continuation is a failure, not an
empty successful conjunction. Classifier-pruned states do not seed success.

The options select which continuations must succeed:

.. list-table::
   :header-rows: 1

   * - ``universal``
     - ``and_backtracking``
     - Behavior
   * - ``false``
     - ``false`` (default)
     - Retain the first ordinary outcome or selected Choose obligation.
   * - ``false``
     - ``true``
     - Try alternative continuations after failure; stop at the first success.
   * - ``true``
     - Either
     - Require every ordinary outcome and every enabled Choose rule to succeed.

``and_backtracking`` (``options.and_backtracking = True`` in Python) makes ordinary (AND) branch points alternatives:
a failed or empty Choose does not rule out another continuation from the same source. Within a Choose, bindings are
alternatives in all modes. The search order is unchanged: ordinary successors in reverse emission order, then Choose
rules in reverse emission order. Enumeration is eager, so all admitted ordinary siblings count toward the state budget
even if an earlier one succeeds.

.. _backtrack-rules:

Backtrack rules
---------------

A Backtrack rule rejects the current continuation when its source memory and conditions match. It has no target memory
and no effects:

.. code:: lisp

   (:rule (:symbol reject)
     (:expression
       (:source-memory m0)
       (:backtrack (:conditions (positive Bad)))))

``Bad`` is a declared Boolean feature evaluated with the current planning state, registers and arguments. An empty
condition list always matches at the source memory. A source-only rule entry contains only Backtrack bodies; other
rule entries require a target and cannot contain them.

- After the goal check, expansion tests Backtrack rules before ordinary rules, whatever their declaration order.
- A match is a terminal failure with no successor and no caller return. The source becomes a dead end, and the
  search unwinds to the nearest alternative: a Choose with an untried binding, or another ordinary continuation with
  AND backtracking. Without one the search fails.
- This holds in every search and memorization mode, also during greedy continuations.
- Backtrack does not mark the planning state as unsolvable, and a reached goal still succeeds.
- Structural termination analysis ignores Backtrack rules: a guard removes no ordinary cycle or recursive call from
  it.

Depth-first evaluation
----------------------

A state receives its final success or failure once its children and choices finish:

- A failed Choose binding advances the cursor; a successful one completes that Choose without trying the rest.
- In universal search every Choose must succeed; empty choices, local failures and failed ordinary successors make
  the state fail.
- With AND backtracking the first successful continuation completes the state; only the exhaustion of all
  alternatives makes it fail.

``ALL`` reuses a completed state before expanding it. ``CHOICE`` detects a cached source only when enumeration reaches
its first admitted Choose, so classification and ordinary successor generation before it can repeat and still count
toward the statistics and the state budget. Structural termination excludes dependencies on an active ancestor, so
there is no success queue and no fixed-point propagation.

**Limits.** Time or state exhaustion returns a resource-limit status, not failure. ``max_num_states`` counts distinct
admitted states in ``ALL`` and admitted state occurrences in ``NONE`` and ``CHOICE``: it budgets search work, not
retained memory, and a reduced mode can exhaust it earlier than ``ALL`` on a graph with shared continuations.
``num_expanded`` and ``num_generated`` include repeated expansions and abandoned branches.

**Cost.** For ``ALL`` with ``V`` state slots, ``E`` recorded transitions and ``C`` attempted Choose obligations,
evaluation takes ``O(V + E + C)`` time and storage, excluding structural analysis, successor generation and the
construction of the final graph and plan. A terminating program can still have exponentially many branching
continuations. Reduced modes trade retained search data for possible recomputation.

Choose ordering
---------------

An optional trailing ``(:order (min feature) (max feature) ...)`` ranks the admitted bindings lexicographically.

- Features are Boolean or numerical and are evaluated with the chosen register tentatively bound, after effect
  filtering. Ties keep the denotation's iteration order.
- Each candidate is scored once per term, then the candidates are sorted.
- Ranking neither prunes bindings nor changes the AND/OR obligations, and scoring does not count as generating
  states; the search time limit includes it.
- Admitted bindings are collected eagerly, also for unordered choices; successor states are constructed only for
  attempted bindings.

Numerical constants and ``n_add``, ``n_sub``, ``n_mul``, ``n_div``, ``n_min`` and ``n_max`` are available in Ext with
the Uns arithmetic semantics.
