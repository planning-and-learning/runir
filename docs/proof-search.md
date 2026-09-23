# Program proof search

The Ext executor separates depth-first exploration from proof resolution:

- [`depth_first_search`](../include/runir/kr/ps/ext/detail/proof_search.hpp) schedules each admitted program state once and advances lazy Choose cursors.
- [`ExecutionState`](../include/runir/kr/ps/ext/detail/execution_state.hpp) records transitions, first parents, limits and statistics.
- [`ProofPropagation`](../include/runir/kr/ps/ext/detail/proof_propagation.hpp) propagates newly established successes through incoming dependencies.

The diagnostic graph and optional plan are constructed afterward. The graph retains rejected alternatives, so a cycle in that graph does not by itself invalidate a successful proof.

[`Predecessors`](../include/runir/kr/ps/ext/detail/predecessors.hpp) owns the append-only transition records. Its typed `EdgeId` identifies an edge independently of vector reallocations. An expansion records its ordinary transitions consecutively before descending. Each DFS frame walks that interval in reverse using `EdgeId` boundaries, preserving the existing traversal order without a separate outgoing-edge index. Incoming dependencies use optional `EdgeId` links for propagation.

## AND/OR semantics

Let `W(s)` mean that program state `s` has a finite successful continuation. A goal establishes `W(s)` immediately. For each enabled Choose rule `r` at `s`, introduce a separate obligation `C(s,r)`:

```text
W(t)                                      => C(s,r)  for each admitted binding s --r--> t
W(t1) AND ... AND W(tn) AND C(s,r1) AND ... => W(s)
```

The second implication is enabled only after successor enumeration finishes and only for states without a local failure. An empty Choose remains unsatisfied; a non-goal state with no applicable continuation is a failure, not an empty successful conjunction. Classifier-pruned states do not seed success.

With `universal=false`, enumeration retains the first ordinary outcome or selected Choose obligation. With `universal=true`, every ordinary outcome and every enabled Choose rule is required. Different bindings of one Choose share an OR obligation; different Choose rules never share one.

## Incremental resolution

Each state has a remaining-requirements counter and a seal marking complete enumeration. [`ChoiceProofs`](../include/runir/kr/ps/ext/detail/choice_proofs.hpp) owns each Choose's permanent satisfied flag, independent of its temporary cursor. Frames and dependencies identify that flag with a `ChoiceId`, a distinct integer type. `satisfy(id)` returns true only on its first success.

A new state success is queued once and visits only incoming dependencies. An ordinary dependency decrements its source counter; a Choose dependency does so only on that obligation's first success. A sealed, unblocked state whose counter reaches zero succeeds.

A dependency registered after its target succeeds is credited immediately and is **not** linked for a second notification. Sealing prevents a successful early child from proving its parent before the parent's other requirements are known.

DFS tries another binding when the previous one remains unresolved; it does not wait for cyclic dependencies to become failures. For example, if `A` chooses `B` or Goal and `B` continues to `A`, proving `A` through Goal subsequently proves `B` through notification. There is no replay or repeated expansion. A cycle without a finite exit produces no success.

When exploration and notifications both finish, the initial state succeeds exactly when it has been proved. Untried bindings of satisfied Choices are unnecessary. Time or state exhaustion returns a resource-limit status, not failure. First-parent plan reconstruction and choice-depth accounting remain unchanged.

For `V` indexed state slots, `E` recorded transitions and `C` Choose obligations, resolution uses `O(V + E + C)` time and storage. This excludes successor generation and final graph/plan construction. The generated planning graph can still be exponential. Notifications may avoid alternatives that the previous replay implementation would have tried; expansion/generation counters continue to report actual work.

## References

The counter-and-worklist mechanism is an incremental application of Horn-clause propagation: W. F. Dowling and J. H. Gallier, [*Linear-time algorithms for testing the satisfiability of propositional Horn formulae*](https://doi.org/10.1016/0743-1066%2884%2990014-1), Journal of Logic Programming 1(3), 267–284, 1984. [Author-hosted PDF](https://www.seas.upenn.edu/~cis5110/Dowling-Gallier-Horn-sat.pdf).

For the equivalent AND/OR reachability-game formulation, see Algorithm 1 in Dietmar Berwanger's [*Graph games with perfect information*](https://lsv.ens-paris-saclay.fr/~dwb/gtc.pdf).
