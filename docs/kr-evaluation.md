# KR evaluation storage and rule evaluators

All DL expressions use `evaluate(expression, context)` in C++, or
`expression.evaluate(context)` in Python. Policy features use
`evaluate(feature, context)`. The context selects result repositories and
memoization; an additional repository argument is no longer part of evaluation.
Concept, role, Boolean, and numerical results are interned denotation views.
Queries return `QueryDenotationView`, a Yggdrasil relation view over typed Tyr
object indices with `DenotationRepository` as its context. Its rows are borrowed
`ObjectSpanView` values that lazily expose Tyr object views. Ordered columns and
a canonical set of rows retain the same storage model as other relations.
All result kinds belong to `DenotationRepository`.

`semantics/interning.hpp` provides three conversion operations:

- `insert(repository, data)` publishes prepared records; computed denotation
  builders additionally receive the reusable semantic builder as scratch.
- `assign(destination, view)` replaces mutable values while retaining buffers
  and clearing the destination's old canonical identity.
- `copy(view, CopyContext { repository, builder })` transfers register values or
  denotations into another repository. Register and bitset denotation copies
  require the same planning formalism. Same-owner results are reused directly.

Insertion and indexed copying return `(view, inserted)`, with the flag describing
only the top-level record. Python repository `insert(data)` returns the same pair;
unpack it as `view, inserted = repository.insert(data)`. The view retains its
repository even after the tuple is discarded. There are no old-name aliases.

`register_values_data.hpp` provides typed `assign_register(data, identifier, value)`
overloads with bounds checking and identity invalidation. Rule binding and
transient execution use assignment before register updates. Fused denotation set
operations keep their existing bitset loops. Database relation copying interns
source rows directly into destination storage without an intermediate builder.

Mutable `Data` uses Yggdrasil's shared `insert(repository, data)` from
`formalism/interning.hpp`. Each language supplies `prepare_for_insert`:
canonicalization runs once before the raw `repository.insert(data)`, and DL
constructors also prepare schema and staticness metadata. Both new and existing
symbol results update the input's index and retain the canonical owning context,
including an ancestor repository.

`make_view` reads an existing representation. `assign` extracts mutable values,
`insert` publishes compatible contents, and `copy` resolves repository-local
payloads in its destination. Stored relation records contain column/row-set
indices and stored denotations contain vector indices; copying those records
alone does not transfer the referenced storage.

## Storage and lifetime

`DenotationCaches<Family>` stores memoized views and owns no result payloads.
Cache instances belong to one evaluation configuration;
do not reuse cached entries with different result repositories.

`EvaluationStorage<Family>` owns reusable static and dynamic result repositories
and their matching caches. Construct it with a prototype denotation
repository: `EvaluationStorage(prototype)` in Python. Pass the storage directly
to the context; roots and children use its caches and repositories.
The semantic `Builder`, defined in `semantics/builder.hpp`, owns pooled builders
and the evaluation workspace, retaining mutable buffers across evaluations.

For pooled `Data`, use `checkout<T>(builder)` to obtain cleared data while
retaining its buffers. Yggdrasil's `BuilderStorage` implements the reset once;
DL, grammar, policy, and execution builders share that operation. Raw
`get_builder<T>()` on a `BuilderStorage` preserves prior contents, including
Ext and ICP execution builders. The semantic builder exposes raw `Data`
acquisition as `get_data<T>()`; its `get_builder<T>(...)` initializes mutable
denotation or relation builders instead.

For example, Python Ext evaluation with scratch ownership is:

```python
from pyrunir.kr.dl.ext import semantics as ext_sem

scratch = ext_sem.EvaluationStorage(denotation_repository)
context = ext_sem.GroundStateEvaluationContext(
    state, builder, scratch, arguments, registers
)
value = expression.evaluate(context)
```

Use the corresponding `LiftedStateEvaluationContext` for lifted states. Base
and Uns contexts omit `arguments` and `registers`. C++ context constructors
use the same argument order.

The Ext specialization of `StateEvaluationContext` carries argument and register
views alongside the shared evaluation state. `for_result(is_static)` returns
a context selecting the current result's static or dynamic repository.
`child_context()` returns a context using the intermediate storage's caches and
repositories. Both retain borrowed inputs without changing the original context
or copying result payloads.
Both Ext constructors require arguments and registers from the state's planning
repository. They may belong to different denotation repositories for that same
planning repository. The check runs when constructing the context, not when
copying it for child or result evaluation.

The reset operations have different ownership effects:

| Operation | Memo entries | Owned results |
| --- | --- | --- |
| `caches.reset_dynamic()` | Clear dynamic entries | Unchanged |
| `caches.reset_all()` | Clear both partitions | Unchanged |
| `storage.reset_dynamic()` | Clear dynamic entries | Reset dynamic repositories |
| `storage.reset_all()` | Clear both partitions | Reset both repositories |

Environment `reset_source()` and `reset_target()` apply the storage-level
dynamic reset to their respective sides. Successor expansion performs these
resets once per source and once per candidate target, so rules inspecting the
same configuration share computed features. Low-level callers perform their
own resets when state, registers, or arguments change. Create new storage for
a different planning task.

Result views remain valid until their owning repository is reset or destroyed.
Python keeps owners alive but does not prevent explicit resets. Memo-only
resets do not invalidate repository results. Storage resets clear affected
memoized views before resetting their results. A full reset also clears static
join indexes. Copies of `DenotationRepositoryFactory` share the relation
repository factory, keeping canonical row identities distinct for cached joins.
Mutable relation builders live in the semantic builder's arity-aware pool and
return there immediately after interning. In C++, builders, caches, and storage
must outlive contexts borrowing them; interned results depend only on their
repositories.

## Durable final results with scratch intermediates

Module-call arguments and retained grammar results need durable roots without
retaining every intermediate result. Pass separate root caches, the durable
repository, and intermediate storage directly:

```python
from pyrunir.kr.dl.ext import semantics as ext_sem

root_memo = ext_sem.DenotationCaches()
context = ext_sem.GroundStateEvaluationContext(
    state, builder, root_memo, denotation_repository, scratch, arguments, registers
)
value = expression.evaluate(context)

root_memo.reset_dynamic()
scratch.reset_dynamic()
# A computed root in denotation_repository remains valid here.
```

Use fresh configuration-appropriate memoization when changing result repositories.
The root memo must also be reset when its input state, registers, or arguments
change; its reset leaves the durable repositories intact. Call execution and
grammar evaluation configure these contexts internally.

Argument leaves return their existing interned argument views directly. They
do not relocate or copy a value merely because the context selects another
output repository. Callers must supply task-owned arguments when storing their
indices in task-owned call bundles. A query rename interns a schema change
while sharing its child's canonical rows. That child is evaluated with the same
output configuration, and the renamed result uses the child's owning partition.
Other child results can remain in scratch storage.

`DenotationRepository::get_relation_repository()` exposes its Yggdrasil relation
storage of typed object indices. Python exposes it as
`pyrunir.kr.dl.base.semantics.QueryDenotationRepository`;
`rename(query, columns)` returns a `QueryDenotation` with the same planning and
storage context, retaining both Python owners. Query result identity consists of
the ordered numeric column schema and row set. Equal schemas and rows share a result even when different constructor
repositories produced them; variable names remain on the query expressions.
Inserting the same rows in another order produces the same identity. Canonical
iteration follows interned row indices, so it need not match insertion order.
Clear the expression caches before clearing or reusing a constructor repository;
interned query results own their schemas and rows and remain valid. Views in a
durable denotation repository survive intermediate-storage resets; views from a
reset scratch repository do not.

## Rule evaluator organization

Each policy family constructs `detail::RuleEvaluators` for a task and its
sketch or program. Rule instances hold their bound rule views and resolved
metadata. Aggregates own shared rule workspaces and scratch buffers;
individual rule instances do not allocate separate workspaces. Base, Ext, and ICP
evaluators live under their respective `detail/rule_evaluation/` directories,
with aggregate dispatch in `detail/rule_evaluators.hpp`. Base's single rule
evaluator lives in `detail/rule_evaluation/rule.hpp`.

Ext's `detail/rule_evaluation/context.hpp` contains only borrowed task, storage,
and environment references. Its storage and the successor expander's storage
are constrained by `ExecutionStorageConcept<Storage, Kind>`, which checks the
shared interface. `StoredState` names the returned state handle: an interned program
state view or a pooled builder handle. `StateView` names the borrowed view used
for evaluation. Rules create a complete successor with
`storage.store(planning_state, module, memory, registers, arguments, call_stack)`.
Interned storage records repository indices; transient storage fills a pooled
program state's embedded module, retaining its planning and register buffers.
Applicability checks live in `compatibility.hpp`;
Load and Choose share register-binding helpers in `rule_evaluation/binding.hpp`.
Execution-step helpers, including `planning_step`, live in
`detail/execution_step.hpp`.

Aggregates retain unique evaluator records and separate occurrence schedules.
Program schedules are flat and selected by module and memory state where
applicable. This keeps rule occurrence order and parallel outcomes intact even
when evaluator records are shared. Dispatch remains templated and uses
value-held variants for heterogeneous rule kinds.

Base selects the first compatible rule for each planning successor. Ext
preserves natural rule order, deferring effect-bearing Sketch rules into one
binding-major enumeration. ICP keeps natural rule order for greedy execution
and groups applicable crules by action for universal execution. Grouped crules
share one planning candidate and one admitted history update per binding.
ICP history builders and action-group buffers retain capacity between uses.
Ext Action applicability and effect validation remain lazy for visited tuples;
ICP resolves action schemas during construction.

Successor callbacks must not reenter their expander or planning generator.
Pending Ext Choose obligations own pooled binding lists that survive child
evaluation resets. Scratch capacity may grow when a wider or deeper operation
first requires it; new persistent identities still require repository storage.
