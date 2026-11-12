KR evaluation storage and rule evaluators
=========================================

Every DL expression is evaluated with ``evaluate<Kind>(expression, context)`` in C++ or
``expression.evaluate(context)`` in Python, and a policy feature with ``evaluate<Kind>(feature, context)``. The caller
names the task kind (``tyr::GroundTag`` or ``tyr::LiftedTag``); the context selects the result repositories and the
memoization.

All results belong to a ``DenotationRepository``. Concept, role, Boolean and numerical results are interned
denotation views. A query returns a ``QueryDenotationView``, a Yggdrasil relation view over typed Tyr object indices
whose rows are borrowed ``ObjectSpanView`` values.

``semantics/interning.hpp`` provides three conversions:

- ``insert(repository, data)`` publishes prepared records. It returns ``(view, inserted)``, the flag describing only
  the top-level record; in Python, ``view, inserted = repository.insert(data)``.
- ``assign(destination, view)`` replaces mutable values, keeping buffers and clearing the destination's old identity.
- ``copy(view, CopyContext { repository, builder })`` transfers register values or denotations into another
  repository of the same planning formalism; results of the same owner are reused. Stored relation records and
  denotations hold indices, so copying a record alone does not transfer the storage it refers to.

``urd::resolve`` (or ``context.view(index)``) reads an existing representation. Each language supplies
``prepare_for_insert``, which canonicalizes once before the raw insertion; DL constructors also prepare their schema
and staticness metadata there. ``register_values_data.hpp`` provides bounds-checked
``assign_register(data, identifier, value)`` overloads.

Datalog evaluation
------------------

``q_head`` reads a head of a module's Datalog program (``DatalogProgram``, :doc:`datalog`). Full evaluation evaluates
the program once per state for all the heads its features read (``DatalogFeature`` in the evaluation workspace) and
copies the head's rows into the query's columns. In incremental evaluation a program is one node without an expression:
its ``q_head`` nodes depend on it and read its evaluator, which is initialized from its input queries and updated from
their exact changes.

``translations/ygg_query.hpp`` translates a query into a tyr query whose input slots are its leaves (each distinct leaf
is one slot), for the query optimizer.

Incremental evaluation
----------------------

``incremental::EvaluationGraph`` interns the features' expressions into nodes in topological order.
``incremental::IncrementalEvaluation`` schedules them over a sequence of states:

- ``enter(context, delta, workspace)`` starts the state reached by ``delta``, and ``enter(context)`` starts an
  unrelated one. Both return a ``StateEvaluation`` handle, which evaluates nodes on demand.
- A demanded node is updated from the delta if it was evaluated in the previous state and its children hold exact
  deltas. Otherwise it is initialized from the state.
- ``DeltaEvaluationPolicy`` computes the deltas between successive source and target states.

Storage and lifetime
--------------------

- ``DenotationCaches`` stores memoized views and owns no results. A cache belongs to one evaluation configuration:
  do not reuse its entries with other result repositories.
- ``EvaluationStorage`` owns reusable static and dynamic result repositories and their caches. Construct it from a
  prototype denotation repository (``EvaluationStorage(prototype)``) and pass it to the context; roots and children
  use its caches and repositories. Create new storage for another planning task.
- The semantic ``Builder`` (``semantics/builder.hpp``) owns pooled builders and the evaluation workspace, and keeps
  their buffers across evaluations. ``checkout<T>(builder)`` gives cleared pooled ``Data`` with its buffers;
  ``get_data<T>()`` gives raw ``Data``, and ``get_builder<T>(...)`` initializes a mutable denotation or relation
  builder.

.. code:: python

   from pyrunir.kr.dl import semantics

   scratch = semantics.EvaluationStorage(denotation_repository)
   context = semantics.GroundStateEvaluationContext(
       state, builder, scratch, arguments, registers
   )
   value = expression.evaluate(context)

Use ``LiftedStateEvaluationContext`` for lifted states; C++ constructors take the same arguments in the same order.
Contexts for sketches and classifiers omit ``arguments`` and ``registers``, and evaluating a register or argument
expression in them throws. Arguments and registers must come from the state's planning repository, which is checked
when the context is constructed. ``for_result(is_static)`` returns a context for the current result's static or
dynamic repository, and ``child_context()`` one for the intermediate storage; both borrow the same inputs.

The resets differ in what they release:

.. list-table::
   :header-rows: 1

   * - Operation
     - Memo entries
     - Owned results
   * - ``caches.reset_dynamic()``
     - Clear dynamic entries
     - Unchanged
   * - ``caches.reset_all()``
     - Clear both partitions
     - Unchanged
   * - ``storage.reset_dynamic()``
     - Clear dynamic entries
     - Reset dynamic repositories
   * - ``storage.reset_all()``
     - Clear both partitions
     - Reset both repositories

- An environment's ``reset_source()`` and ``reset_target()`` apply the dynamic storage reset to their side. Successor
  expansion resets once per source and once per candidate target, so rules inspecting the same configuration share
  computed features. Low-level callers reset themselves when the state, registers or arguments change.
- A result view is valid until its repository is reset or destroyed. Python keeps owners alive but does not prevent
  resets. Memo-only resets invalidate no results; storage resets clear the affected memo entries first, and a full
  reset also clears the static join indexes.
- Row identities come from one process-wide counter, so cached joins never confuse rows of different repositories or
  resets.
- In C++, builders, caches and storage must outlive the contexts that borrow them; interned results depend only on
  their repositories.

Durable final results with scratch intermediates
------------------------------------------------

Module-call arguments and retained grammar results need durable roots without keeping every intermediate result. Pass
separate root caches, the durable repository and the intermediate storage:

.. code:: python

   from pyrunir.kr.dl import semantics

   root_memo = semantics.DenotationCaches()
   context = semantics.GroundStateEvaluationContext(
       state, builder, root_memo, denotation_repository, scratch, arguments, registers
   )
   value = expression.evaluate(context)

   root_memo.reset_dynamic()
   scratch.reset_dynamic()
   # A computed root in denotation_repository remains valid here.

Reset the root memo when its input state, registers or arguments change; that leaves the durable repository intact.
Use fresh memoization when changing result repositories. Call execution and grammar evaluation configure these
contexts themselves.

- **Argument leaves** return their interned argument views as they are, whatever output repository the context
  selects. Supply task-owned arguments when storing their indices in task-owned call bundles.
- **A query rename** interns a schema change and shares its child's canonical rows; the renamed result lives in the
  child's partition.
- **Query identity** is the ordered numeric column schema and the row set: equal schemas and rows share a result
  whatever constructor repository or insertion order produced them, and variable names stay on the expressions.
  Iteration follows interned row indices.
- **Relation storage.** ``DenotationRepository::get_relation_repository()`` is the Yggdrasil relation storage
  (``pyyggdrasil.formalism.RelationRepository`` in Python, with ``RelationView`` results). Clear the expression
  caches before clearing or reusing a constructor repository; interned query results own their schemas and rows and
  stay valid.

Rule evaluators
---------------

Each policy family constructs ``detail::RuleEvaluators`` for a task and its sketch or program. Base, Ext and ICP
evaluators live under their ``detail/rule_evaluation/`` directories, with the aggregate dispatch in
``detail/rule_evaluators.hpp``. Rule instances hold their bound rule views and resolved metadata; the aggregate owns
the shared workspaces and scratch buffers.

**Interface.** A rule evaluator emits results through ``emit(context, source, emit, stop)`` and checks a supplied
candidate through ``matches(context, source, candidate)``, both const. ``rule_evaluator_concepts.hpp`` constrains the
callbacks: ``EmitConcept<Emit, Result>`` is a mutable callback returning whether to continue, ``StopConcept`` one
returning whether to stop. Matching takes Tyr's ``LabeledNode<Kind, State, Binding>``, so a borrowed action binding
need not be published. Callbacks must not reenter their expander or planning generator.

**Contexts.** ``RuleEvaluationContext`` is specialized per family and task kind in each
``detail/rule_evaluation/context.hpp``. Contexts borrow execution services; Ext and ICP keep one prepared source
planning view per evaluation, whose backing storage must stay alive. Feature evaluation and compatibility checks use
``StateEvaluationContextConcept<Context, Kind>``, and C++ calls name the kind: ``is_compatible_with<Kind>``,
``all_compatible<Kind>``, ``classify<Kind>``. Conditions take a state context or the source of a transition context;
effects take its source and target.

**Execution storage.** Ext's context storage and the successor expander's storage satisfy
``ExecutionStorageConcept<Storage, Kind>``. ``StoredState`` is the returned state handle (an interned program state
view or a pooled builder handle) and ``StateView`` the borrowed view used for evaluation. A rule creates a successor
with ``storage.store(planning_state, module, memory, registers, arguments, call_stack)``. Applicability checks are in
``compatibility.hpp``, the register-binding helpers of Load and Choose in ``rule_evaluation/binding.hpp``, and the
execution-step helpers in ``detail/execution_step.hpp``.

**Bindings and labels.** Planning candidates use Tyr's borrowed binding enumeration, valid throughout the callback.
``successor`` returns an unlabeled planning node; its label is interned only after a rule admits the transition
(including ICP's history admission), and grouped rules share it. Retained steps and graph edges carry indexed
bindings. Action rules read borrowed object spans from query rows.

**Order.**

- Base selects the first compatible rule for each planning successor.
- Ext checks Backtrack guards before emitting any ordinary outcome; a match emits only a terminal failure. Otherwise
  it keeps natural rule order, deferring effect-bearing Sketch rules into one binding-major enumeration. Action
  applicability and effect validation are lazy, for visited tuples only.
- ICP keeps natural rule order for greedy execution and groups applicable crules by action for universal execution;
  grouped crules share one planning candidate and one admitted history update per binding.

Order and repetitions of rule occurrences are preserved. Pending Ext Choose obligations own pooled binding lists that
survive child evaluation resets. Scratch capacity grows when a wider or deeper operation first needs it.
