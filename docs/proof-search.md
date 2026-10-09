# Program proof search

Ext `find_solution` requires a structurally terminating whole program. It checks
this before execution and raises `std::invalid_argument` (`ValueError` in Python)
when termination is not established. Structural-analysis resource limits also
remain errors, not termination certificates. The requirement applies to both
grounded and lifted execution, in both universal and nonuniversal modes.

With this precondition, concrete execution has no cycles and results can be
computed directly in depth-first postorder:

- One [`depth_first_search`](../include/runir/kr/ps/ext/detail/proof_search.hpp) handles greedy execution, backtracking and universal proofs with every storage mode.
- [`SearchStorage<Kind, StateMemorization>`](../include/runir/kr/ps/ext/detail/search_storage.hpp) specializes admission, memoization and retained transitions for each mode. The entry-point switch selects the policy once; the DFS performs no runtime memorization-mode checks. `NONE` has no memo table.
- Pooled [`SearchPath`](../include/runir/kr/ps/ext/detail/search_path.hpp) records retain active prefixes and selected witnesses; released paths return their storage to the pool.

The diagnostic graph and optional plan are constructed afterward. Their retained
states depend on the selected memorization mode below. Failed alternatives do
not invalidate a successful Choose.

In `ALL` mode, [`SearchStorage`](../include/runir/kr/ps/ext/detail/search_storage.hpp)
keeps an append-only vector of admitted transitions, including parallel edges.
The shared DFS enumerates an expansion before descending, then processes ordinary
successors and Choose obligations in reverse order. Storage policies preserve
`ALL`'s first-parent plan reconstruction without a separate traversal algorithm.

## Feature evaluation

`RUNIR_DELTA_EVALUATION` defaults to `ON`, selecting
`DeltaEvaluationPolicy<Family, Kind>` for Base, Ext, and ICP proof search and UNS
classification during search. Configure with `-DRUNIR_DELTA_EVALUATION=OFF` to use
`FullEvaluationPolicy<Family, Kind>` and the existing memoized, interned results.
Both live in `runir::kr::dl::semantics` and satisfy
`EvaluationPolicyConcept<Policy, Family, Kind>`. Their family is the DL family;
ICP uses `ExtFamilyTag`.
C++ expanders can select an explicit policy; their default follows the flag.

Delta evaluation prepares separate source and target graphs from the sketch,
module, or classifier’s feature catalogs. Every rule feature must be declared
in its catalog.
It retains input snapshots, computes state/register differences between evaluations,
and reuses result buffers across forward steps and backtracking. The DFS path
does not retain deltas. Module, caller, or argument changes reinitialize dynamic
results; task-static results remain reusable. Call arguments still use full
evaluation and interning because their results must survive the caller.

Query relations keep set semantics, but incremental row updates can change Action
enumeration order. Plans, traversal statistics, and resource-limited outcomes may
therefore differ from full evaluation.

## State memorization and returned paths

`ProgramSearchOptions.state_memorization` controls reuse of completed search
results. It defaults to `StateMemorization.ALL`, preserving the existing behavior.

| Mode | States memoized during search | Returned graph |
| --- | --- | --- |
| `NONE` | None; a shared continuation can be evaluated again. | Only the selected solution or diagnostic path. |
| `CHOICE` | Interned source states with admitted Choose obligations, including singleton and empty choices. | Only the selected solution or diagnostic path. |
| `ALL` | Every admitted program state; shared continuations reuse their completed result. | The full explored graph, including failed alternatives and repeated transitions. |

In `CHOICE`, the memoized result belongs to the source state's **combined**
continuations under the selected search mode. In universal search, it includes
every ordinary successor and every selected Choose rule; satisfying one Choose
does not satisfy its siblings. With AND backtracking, one successful continuation
suffices. The first
admitted Choose interns the complete source program state, including registers
and caller frames. Its `ProgramStateView` is the memo key; the table does not
copy builders. Choice sources remain in the task repository even when they are
absent from the returned witness graph. The modes do not change the AND/OR
semantics or eliminate the active state and control data needed for execution
and backtracking.

`NONE` and `CHOICE` materialize the selected returned path after search. Its states
are retained so graph labels and a returned plan remain valid after search-local
builders are released. This final witness storage is separate from memoizing
states during search: selecting `NONE` does not mean that the result contains no
states. Tyr also registers the initial planning seed for goal checks and plan
reconstruction, even with a zero state budget; this is not a search memo entry,
and other reduced-mode successors stay pooled until the selected path is
materialized.
Transient execution builders own child values in their lowest-level construction
representation: `Builder<T>` where available, otherwise `Data<T>`. This means an
inline planning-state builder and register data. Definitions and interned call
arguments are referenced by indices; borrowed views interpret them using the
execution repository. Saved callers form an immutable chain: each call-stack
frame is always interned, with an optional index identifying its parent. The
program builder stores the optional head index, just like interned program data.
Calls intern the current module's saved registers and caller frame; ordinary
transitions and returns reuse existing indices. Programs without calls create no
caller frames. Caller frames remain in the task repository even in `NONE` and
`CHOICE` modes. Copying a program builder copies its mutable module values and
the caller index. Execution storage owns the remaining pools and must outlive
their handles.
Transitions initialize final values directly instead of copying planning states
or registers that they immediately replace.
Planning successor generation reuses one scratch builder owned by execution
storage. Its returned node borrows that builder until the next successor call;
accepted candidates are copied into program states before the scratch is reused.

Only successful nonuniversal execution returns a plan; universal success
and failures can still return a diagnostic graph.

On failure, the reduced graph contains one failing path that execution actually
reached. This can be a rejected Choose binding, even when another binding later
satisfied that particular Choose. The graph is a diagnostic example: it does not
show that every alternative failed, or record all obligations of a universal
search. The result status reports the outcome of the whole search. Failure
suffixes are not retained in the memo table or reconstructed from cached results.

```python
options = ext.LiftedProgramSearchOptions()
options.state_memorization = ext.StateMemorization.CHOICE
result = ext.find_lifted_solution(task_context, program, options)
```

## AND/OR semantics

Let `W(s)` mean that program state `s` has a finite successful continuation. A goal establishes `W(s)` immediately. For each enabled Choose rule `r` at `s`, introduce a separate obligation `C(s,r)`:

```text
W(t)                                      => C(s,r)  for each admitted binding s --r--> t
W(t1) AND ... AND W(tn) AND C(s,r1) AND ... => W(s)
```

The second implication is enabled only after successor enumeration finishes and only for states without a local failure. An empty Choose remains unsatisfied; a non-goal state with no applicable continuation is a failure, not an empty successful conjunction. Classifier-pruned states do not seed success.

The options select which continuations must succeed:

| `universal` | `and_backtracking` | Behavior |
| --- | --- | --- |
| `false` | `false` (default) | Retain the first ordinary outcome or selected Choose obligation. |
| `false` | `true` | Try alternative continuations after failure; stop at the first success. |
| `true` | Either | Require every ordinary outcome and every enabled Choose rule to succeed. |

`and_backtracking` enables exhaustive scout search at ordinary (AND) branch
points. A failed or empty Choose does not rule out another continuation from
the same source. Within each Choose, bindings remain alternatives in all modes.
The option uses the existing DFS order: ordinary successors in reverse emission
order, then Choose rules in reverse emission order. Enumeration remains eager,
so all admitted ordinary siblings count toward the state budget even if an
earlier successful continuation makes exploring them unnecessary.

In Python, set `options.and_backtracking = True` on either
`GroundProgramSearchOptions` or `LiftedProgramSearchOptions`.

## Backtrack rules

A Backtrack rule rejects the current continuation when its source memory and
conditions match. It has no target memory or effects:

```lisp
(:rule (:symbol reject)
  (:expression
    (:source-memory m0)
    (:backtrack (:conditions (positive Bad)))))
```

`Bad` is a declared Boolean feature evaluated with the current planning state,
registers, and arguments. An empty condition list always matches at the source
memory. A source-only rule entry contains only Backtrack bodies; ordinary rule
entries still require a target and cannot contain Backtrack bodies.

After the goal check, expansion tests Backtrack rules before ordinary rules,
regardless of their declaration order. A match produces a terminal failure with
no successor or caller return. Search marks the source as a dead end and uses
the existing backtracking: a containing Choose may try its next binding, or
AND backtracking may try another ordinary continuation. This
applies in both universal modes and every memorization mode. Backtrack does not
mark the planning state as unsolvable, and a reached goal still succeeds.

Guards are checked during greedy continuations too. Failure unwinds intervening
steps to the nearest permitted alternative: a Choose with an untried binding,
or a remaining continuation when AND backtracking is enabled. If no alternative
remains, the search fails.

Structural termination analysis omits Backtrack rules because they create no
transitions. It remains conservative about other rules: a Backtrack guard does
not remove an ordinary cycle or recursive call from that analysis.

## Depth-first evaluation

DFS frames are small values in one stack vector: a pooled path handle, offsets
into shared pending-successor and choice buffers, an optional interned memo key,
and two result flags. Frames
own no vectors and are not individually allocated. Children append to the shared
buffers and consume their entries before returning, preserving pending siblings.
The next path is processed before returning to its parent frame.

A failed Choose binding advances the same cursor; a successful binding completes
that Choose without trying the remaining alternatives. In universal search,
every selected Choose must succeed; empty choices, local failures and failed
ordinary successors make their state fail. With AND backtracking, the first
successful continuation completes the state and releases its pending siblings;
only exhaustion of all alternatives makes the state fail.

Once its children and choices finish, the state receives its final success or
failure. `ALL` reuses a completed state before expansion. `CHOICE` detects a
cached source when enumeration reaches its first admitted Choose and discards
any buffered successors from that repeated expansion. Classification and
ordinary successor generation before that Choose can therefore repeat; this
work still counts toward the statistics and state budget. Structural termination
excludes dependencies on an active ancestor, so there is no success queue,
reverse-dependency storage or fixed-point propagation.

Time or state exhaustion returns a resource-limit status, not failure.
`max_num_states` counts distinct admitted states in `ALL`, and admitted state
occurrences in `NONE` and `CHOICE`. It is a search-work budget, not a bound on
simultaneously retained memory. Repeated expansions in reduced modes contribute
to `num_expanded` and repeated emitted successors contribute to `num_generated`;
the counters include abandoned branches. Consequently a reduced mode can exhaust
the same budget earlier than `ALL` on a graph with shared continuations.

For `ALL`, with `V` indexed state slots, `E` recorded transitions and `C` attempted Choose
obligations, evaluation uses `O(V + E + C)` time and storage, excluding structural
analysis, successor generation and final graph/plan construction. A terminating
program can still have exponentially many branching continuations. Reduced modes
trade less retained search data for possible recomputation; final witness storage
also grows with the returned path.

Call arguments and their final denotations are interned in the task repository
in every mode, including `NONE` and `CHOICE`. They can accumulate across calls;
reduced state memorization does not bound this storage.

## Choose ordering

An optional trailing `(:order (min feature) (max feature) ...)` ranks admitted bindings lexicographically. Features are Boolean or numerical and are evaluated with the chosen register tentatively bound, after effect filtering. Ties retain denotation iteration order. The executor scores each candidate once per term, then sorts; it never evaluates features from the comparator. All modes retain admitted bindings in pooled vectors, so later evaluation can clear its denotation caches without invalidating pending choices. These vectors are filled eagerly, including for unordered choices; successor states are still constructed only for attempted bindings.

Ranking does not prune bindings or change AND/OR obligations. Scoring does not increment generated-state statistics. Search time limits include ranking. Numerical constants and `n_add`, `n_sub`, `n_mul`, `n_div`, `n_min`, and `n_max` are available in Ext with the existing Uns arithmetic semantics.
