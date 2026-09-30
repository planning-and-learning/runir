# Program proof search

Ext `find_solution` requires a structurally terminating whole program. It checks
this before execution and raises `std::invalid_argument` (`ValueError` in Python)
when termination is not established. Structural-analysis resource limits also
remain errors, not termination certificates. The requirement applies to both
grounded and lifted execution, in both universal and nonuniversal modes.

With this precondition, concrete execution has no cycles and results can be
computed directly in depth-first postorder:

- [`depth_first_search`](../include/runir/kr/ps/ext/detail/proof_search.hpp) evaluates each admitted program state once and advances lazy Choose cursors.
- [`ExecutionState`](../include/runir/kr/ps/ext/detail/execution_state.hpp) records transitions, first parents, limits and statistics.

The diagnostic graph and optional plan are constructed afterward. The graph
retains failed alternatives, which do not invalidate a successful Choose.
Planning states, program states and completed search results remain interned;
shared continuations reuse their completed success or failure.

[`Predecessors`](../include/runir/kr/ps/ext/detail/predecessors.hpp) owns the append-only transition records. Its typed `EdgeId` identifies an edge independently of vector reallocations. An expansion records its ordinary transitions consecutively before descending. Each DFS frame walks that interval in reverse using `EdgeId` boundaries, preserving the existing traversal order without a separate outgoing-edge index.

## AND/OR semantics

Let `W(s)` mean that program state `s` has a finite successful continuation. A goal establishes `W(s)` immediately. For each enabled Choose rule `r` at `s`, introduce a separate obligation `C(s,r)`:

```text
W(t)                                      => C(s,r)  for each admitted binding s --r--> t
W(t1) AND ... AND W(tn) AND C(s,r1) AND ... => W(s)
```

The second implication is enabled only after successor enumeration finishes and only for states without a local failure. An empty Choose remains unsatisfied; a non-goal state with no applicable continuation is a failure, not an empty successful conjunction. Classifier-pruned states do not seed success.

With `universal=false`, enumeration retains the first ordinary outcome or selected Choose obligation. With `universal=true`, every ordinary outcome and every enabled Choose rule is required. Different bindings of one Choose share an OR obligation; different Choose rules never share one.

## Depth-first evaluation

Each DFS frame remembers its pending child and whether all ordinary successors
have succeeded. A failed Choose binding advances the same cursor; a successful
binding completes that Choose without trying the remaining alternatives. Every
selected Choose must succeed. Empty choices, local failures and failed ordinary
successors make their state fail.

Once its children and choices finish, the state receives its final success or
failure. An already completed state is reused immediately. Structural termination
excludes dependencies on an active ancestor, so there is no success queue,
reverse-dependency storage or fixed-point propagation.

Time or state exhaustion returns a resource-limit status, not failure.
First-parent plan reconstruction and choice-depth accounting remain unchanged.
For `V` indexed state slots, `E` recorded transitions and `C` attempted Choose
obligations, evaluation uses `O(V + E + C)` time and storage, excluding structural
analysis, successor generation and final graph/plan construction. A terminating
program can still have exponentially many branching continuations.

## Choose ordering

An optional trailing `(:order (min feature) (max feature) ...)` ranks admitted bindings lexicographically. Features are Boolean or numerical and are evaluated with the chosen register tentatively bound, after effect filtering. Ties retain denotation iteration order. The executor scores each candidate once per term, then sorts; it never evaluates features from the comparator. Unordered choices retain their lazy cursor.

Ranking does not prune bindings or change AND/OR obligations. Scoring does not increment generated-state statistics. Search time limits include ranking. Numerical constants and `n_add`, `n_sub`, `n_mul`, `n_div`, `n_min`, and `n_max` are available in Ext with the existing Uns arithmetic semantics.
