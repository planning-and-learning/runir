Query features and Action rules
===============================

Extended modules can declare named query features and use their rows as action parameter tuples. A query can preserve
relationships between parameters, such as an edge's source and target, without a Cartesian product of separate concept
denotations.

.. code:: lisp

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

Here ``move`` has two parameters, and each row of ``moves`` supplies its complete binding.

- **Rows are bindings.** A row is the complete normalized action binding in schema-variable order
  (``get_variables()``), including the existential witness variables Tyr's normalization introduces. The parser checks
  that the action exists in the domain and that the query's width is the normalized schema's ``get_arity()``.
- **Columns.** Column names need not equal the PDDL parameter names; ``q_project`` reorders and ``q_rename`` renames
  the output columns.
- **Declaration.** An Action rule's ``:query`` names a query feature declared in the module's ``:features``; inline
  query expressions are not allowed in rule bodies. Query features exist only in extended modules, may read the
  module's registers and arguments, and are not conditions, effects or call arguments.
- **Empty and nullary queries.** An empty query selects no tuple. A query with no columns is false with no rows and
  true with one empty row, which allows zero-parameter actions.
- **Conditions** and the current memory state must match before an Action rule is used.

Effects and applicability
-------------------------

The rule's ``:effects`` filter its transitions as for a Do rule: a tuple's successor is offered only if it satisfies
them, and unmentioned features are unconstrained. In the example, only moves that preserve the number of occupied
positions are offered.

Every selected tuple must be applicable. Execution checks the tuples it visits and raises ``std::logic_error``
(``RuntimeError`` in Python) for an inapplicable one: it is an error, not a binding to discard. Unvisited rows are not
checked.

Structural termination treats an Action rule as a Do rule with the same conditions, effects and memory endpoints.

Enumeration and search
----------------------

Ext ``find_solution`` requires a structurally terminating program (:doc:`proof-search`); direct successor-expander
calls do not.

``SuccessorExpander::for_each_successor(state, statistics, emit, stop)`` visits the immediate outcomes of a
``ProgramState`` (the planning state with the module's control, registers, arguments and call stack) without
collecting them. The callback returns ``true`` to continue. Enumeration returns ``true`` only when exhausted.

- **Outcomes.** An ordinary outcome is a ``ProgramExecutionStep``. A Choose rule emits a ``ConceptChoice`` or
  ``RoleChoice`` holding its effect-filtered bindings and a current position; applying a choice constructs only the
  current binding's successor. Clearing evaluation caches does not invalidate pending choices. An enabled empty
  Choose is a failed obligation.
- **Modes.** Nonuniversal execution stops at its first compatible outcome. Universal execution requires every
  ordinary outcome and one successful binding for each Choose rule.
- **Order.** Rule occurrences keep their order. Action tuples follow their relation's iteration order, which can
  differ between full and incremental evaluation; Choose bindings follow their denotation order.
- **Cost.** Do rules enumerate bindings only for their action schema and filter arguments before constructing
  successors. Query evaluation materializes the selected relation.

Statistics:

- ``num_generated`` counts emitted successors, including caller returns and attempted Choose bindings, but not
  rejected planning candidates, failure markers or unattempted bindings.
- ``num_expanded`` belongs to the search; direct expander calls do not increment it.
- ``choice_depth`` counts the non-singleton Choose bindings on the path to the first discovered goal, and
  ``choice_width`` is the largest number of bindings (after effect filtering) at a Choose on that path: 1 for a
  singleton, 0 without a Choose. Both are zero unless the search succeeds, and choices on abandoned branches do not
  count.

The default ``StateMemorization.ALL`` expands each program state at most once and keeps every diagnostic edge;
``NONE`` and ``CHOICE`` can repeat states and return only the selected path (:ref:`state-memorization`).

Python access
-------------

Module evaluation uses the ``ext.GroundProgramState`` or ``ext.LiftedProgramState`` values the successor expander
produces, with the matching ``GroundEvaluationEnvironment`` or ``LiftedEvaluationEnvironment``.

**Expanding.** ``expander.initial_state(planning_state)`` takes a Tyr planning state, such as
``initial_node.get_state()``. ``expander.for_each_successor(state, statistics, emit, stop)`` checks the
:ref:`Backtrack guards <backtrack-rules>` and then emits in natural order, with a constructible
``ext.ProgramSearchStatistics``. Do not reenter the expander from its callback.

- A **choice** has ``rule``, ``bindings``, ``current()``, ``advance()``, ``exhausted()`` and ``count()``. After
  enumeration returns, ``expander.apply_choice(state, choice, statistics)`` applies the current binding; advance the
  cursor to try another. Accessing or advancing an exhausted choice raises ``IndexError``. Python choices keep their
  expander alive; in C++ a choice must be destroyed before its expander, which owns the binding pool.
- A **step** continues with ``step.target``. A planning step also has its ``planning_successor``. A Backtrack match
  emits a ``failure`` step whose target is the source: it ends that continuation and is not a transition.
- ``matching_rule(state, successor)`` and ``apply(state, rule, successor)`` accept a Tyr labeled successor.
- Expansion uses states without accumulated metrics; a successful plan gets them when Tyr replays its actions.

**Evaluating features.** ``module.get_query_features()`` returns the named ``ext.dl.QueryFeature`` values,
``ActionRule.get_query_feature()`` the rule's selector, and ``feature.get_expression().get_columns()`` its schema.
``environment.make_dl_context(program_state)`` creates a context that borrows the state's registers and arguments, and
``ext.evaluate(feature, state_context)`` returns the feature's denotation: Boolean and numerical ones give their value
through ``.get()``, concept and role ones are iterable.

.. code:: python

   from pyrunir.kr.ps import ext

   feature = module.get_query_features()[0]
   columns = tuple(column.get_name() for column in feature.get_expression().get_columns())
   state_context = environment.make_dl_context(program_state)
   relation = ext.evaluate(feature, state_context)
   snapshot = tuple(tuple(row) for row in relation)

``relation`` is an interned ``pyyggdrasil.formalism.RelationView`` with ``get_index()``, ``columns()``, ``arity()``,
``at()``, ``len()`` and iteration. Each row yields object indices as integers in column order; look an object up with
``denotation_repository.get_object(ObjectIndex(value))`` (``pytyr.formalism.planning.ObjectIndex``). Rows form a
canonical set iterated by interned row identity, not insertion order, so the tuple order can change which binding a
nonuniversal search visits first.

**Resets.** Before evaluating another source state, register binding or argument binding, call
``environment.reset_source()`` and create a new context; ``environment.reset_target()`` resets target evaluations
between candidate transitions. Both clear the dynamic memo entries and their result repositories and keep static
results; creating a context resets nothing. Resetting a result repository invalidates its views even though Python
keeps their owners alive, so take a snapshot first, as above. Memo-only ``DenotationCaches.reset_dynamic()`` and
``reset_all()`` leave result storage intact.

**Arguments.** Argument features return views of the stored call arguments. In every memorization mode the final
argument values are evaluated in the caller's state and interned with their bundle in the task's denotation
repository, where they stay until that repository is cleared; clearing feature caches does not invalidate them.

**Low-level contexts.** ``pyrunir.kr.dl.semantics`` contexts take ``(state, builder, storage, arguments, registers)``,
or ``(state, builder, caches, denotation_repository, intermediates, arguments, registers)`` when the final result
needs other ownership than its children; both exist without ``arguments`` and ``registers``.
``CallArgumentsData`` holds the four lists of denotation indices and ``RegisterValuesData`` optional object indices or
pairs of them; intern a bundle with ``arguments, inserted = denotation_repository.insert(data)`` and pass the views to
the context. Supplied argument indices must already refer to the task repository. See :doc:`kr-evaluation`.

**Serialization.** An Action rule serializes ``source``, ``target``, ``conditions``, ``effects``, ``action_name`` and
``query_feature``; see the :doc:`serialization reference <serialization/index>`.
