# Choose rules: nondeterministic choice and backtracking for extended sketches

Design note for extending the `runir::kr::ps::ext` module language with
existential choice over register bindings and an executor that backtracks
over those choices. Written 2026-09-17 as a specification to implement later.
Nothing in this note is implemented yet.

## 1. Purpose

Extended sketches (ICAPS 2024, `bonet-et-al-icaps2024.pdf`) solve a class of
tasks in polynomial time when the sketch is reducible, terminating and of
bounded serialized width (Theorem 10). Classes such as TSP with a cost
bound, Sokoban or assignment tasks have no such sketch unless P = NP, so
something exponential has to enter. The idea, following the backdoor view
of Kronegger, Ordyniak and Pfandler (AIJ 269, 2019), is to isolate the
exponential part in a small number of explicit choice points and keep
everything between them polynomial and verified:

- **Choice points** are `choose` rules. They bind a register to an object,
  or a role register to an object pair, drawn from the denotation of a
  concept or role feature. The executor searches over the bindings.
- **Everything else** is the existing language: `load`, `sketch`, `do`,
  `call` rules, memory states, registers, indexical features, modules.
  These segments are verified to reach their subgoal for every binding the
  choice can produce, and the executor never backtracks into them.

Nothing about *which* binding to take is learned. The learned object is the
decomposition: where choices sit, which feature supplies the candidates, and
which verified rules run between choices. Correctness of the decomposition is
completeness: for every solvable task in the class, some branch of the choice
tree reaches the goal.

## 2. Current language, as implemented

For reference, the pieces the extension touches. File paths are relative to
the `runir` repository root.

| Piece | Where | Behaviour today |
|---|---|---|
| Rule kinds | `include/runir/kr/ps/ext/declarations.hpp` | `LoadTag<Concept>`, `LoadTag<Role>`, `SketchTag`, `DoTag`, `CallTag`; `RuleKind` concept and the `LoadRuleTypes`, `ControlRuleTypes` type lists |
| Rule data | `include/runir/kr/ps/ext/rule_data.hpp` | each rule has `source`, `target` memory state and `conditions`; load has `feature` and `reg` |
| Concrete syntax | `include/runir/kr/ps/ext/dl/ast/ast.hpp`, `src/kr/ps/ext/dl/parser.cpp`, `include/runir/kr/ps/ext/formatter.hpp` | `(:rule (:symbol s) (:expression (:source-memory m) (:target-memory m') <variant>...))`, variants `(:load (:conditions ...) (:concept F) (:register (:concept r)))`, `(:sketch (:conditions ...) (:effects ...))`, `(:do ...)`, `(:call ...)`; several variants under one rule symbol are alternatives |
| Registers | `include/runir/kr/dl/declarations.hpp` | `num_registers = 4` concept registers and 4 role registers per module; features refer to them with `(c_register r0)` and `(r_register r0)` |
| Execution state | `include/runir/kr/ps/ext/execution_data.hpp` | interned tuple `(program, planning state, call stack, phase)`; the call stack frame holds `(module, memory state, registers, arguments, caller)` |
| Steps | `include/runir/kr/ps/ext/successor_expander.hpp` | one `ProgramStep` per applicable rule and binding: a load rule yields one step per object in the denotation; a sketch rule with effects yields one step per compatible successor state |
| Search | `include/runir/kr/ps/ext/detail/proof_search.hpp` | `find_solution` with `options.universal = true` explores every step and reports `FAILURE` if any leaf is open or a dead end (verification); with `universal = false` it keeps only the first step at each state (greedy execution) |
| No rule applies | `SuccessorExpander::fallback` | pops the call stack (`RESTORED_CALLER`) or reports `NO_APPLICABLE_ACTION` |
| Termination | `include/runir/kr/ps/ext/dl/structural_termination.hpp` | Sieve over `(feature valuation, memory state)` vertices; load rules use the Phi(r) semantics |

Two facts about the current semantics matter for the extension:

1. **Load is universal.** In verification mode every binding of a load rule
   is explored and every one must succeed. In execution mode an arbitrary
   binding is taken and never revisited. This is the paper's `Load(C, r)`
   and it stays as it is.
2. **Sketch rules are universal over successors.** Every successor state
   compatible with the effects is explored in verification mode, and any one
   of them is acceptable in execution mode. This is the ordinary sketch
   semantics and it also stays as it is.

The extension adds the one construct with the opposite quantifier.

## 3. Syntax

One new rule variant. It lives inside `(:expression ...)` next to the
existing variants and carries `(:conditions ...)` like every other rule.
Nothing else is added to the grammar; in particular there is no failure
construct, because failure is already expressible (section 4.3).

### 3.1 `choose`

```lisp
(:rule (:symbol pick_city) (:expression
  (:source-memory m0) (:target-memory m1)
  (:choose (:conditions (greater_zero n_unvisited))
           (:concept unvisited)
           (:register (:concept r0)))))
```

Grammar, mirroring `load` exactly:

```
choose-rule  ::= "(:choose" conditions ( "(:concept" ident ")" | "(:role" ident ")" ) "(:register" register ")" ")"
register     ::= "(:concept" ident ")" | "(:role" ident ")"
```

Static checks, the same as for `load`:

- the feature named after `:concept` or `:role` is declared in the module's
  `(:features ...)` block with matching category;
- the register is declared in the module's `(:registers ...)` block with
  matching category;
- no non-emptiness check. A `choose` over an empty denotation yields no
  steps, exactly as a `load` does today, and the consequences are those of
  section 4.3. Programs should guard the rule with `(greater_zero F)` on the
  count of the chosen feature, as the paper does for `Load`, but the parser
  does not enforce it.

The rule symbol `choose` already appears as a *rule name* in
`tests/fixtures/kr/ps/parser/alternative_rules.module`. Rule names are free
identifiers and do not collide with the keyword `:choose`, but rename that
fixture's rule to avoid confusion when reading test output.

### 3.2 What is deliberately not added

- **No `choose` over a pair of concepts.** A dependent pair choice is a role
  denotation, and a role register already exists. Sequential concept choices
  cover arity three and above.
- **No ordering annotation.** Bindings are iterated in object index order
  (concept registers) or lexicographic object index order (role registers).
  Completeness does not depend on the order. An ordering can be added later
  as an option on the executor without touching the semantics.
- **No cut or commit.** Segments between choices are already deterministic
  from the point of view of backtracking (section 4.2), so there is nothing
  to commit.
- **No failure rule.** "No applicable rule" already means failure at the
  top level and return inside a callee (section 4.3). A dedicated `fail`
  construct was considered and rejected because it can be expressed with
  what exists.

## 4. Semantics

### 4.1 Execution states and steps

Unchanged. A `choose` rule in memory state `m` with feature `F` and
register `r` is applicable in execution state `(s, m, v)` when its
conditions hold under `(s, v)`. It produces one step per element `o` of the
denotation `F(s, v)`, each step moving to `(s, m', v[r := o])` in phase
`INTERNAL`, exactly as `append_steps` does for a load rule today. The only
difference is how the search interprets the set of steps.

**Sketch rules keep their current width-0 semantics.** The `ext` executor
has no subproblem search: a sketch rule with effects selects one planning
transition whose successor is compatible with the effects, and a subgoal
such as "agent at the city in `r0`" is reached by a chain of such
single-step rule applications, typically cycling through memory states.
That is a policy, and the greedy executor already runs it. Backtracking
frames are pushed only at `choose` steps, so the policy segment between two
choices executes exactly as it does today. If an IW-style subproblem search
for sketch rules of width greater than zero is added later, only the step
producer for sketch rules changes; the AND/OR semantics, the executor and
the nogoods are unaffected.

### 4.2 AND/OR semantics

Define *solved* on execution states by structural recursion over the proof
graph:

- A goal state is solved.
- A state with no applicable rule and an empty call stack is not solved
  (`NO_APPLICABLE_ACTION`). A state with no applicable rule and a non-empty
  call stack is solved iff the state after `restore_caller` is solved.
- Otherwise, let `R` be the applicable rules. The state is solved iff for
  every rule `r` in `R`:
  - if `r` is a `choose` rule: **some** step of `r` leads to a solved state
    (OR over bindings);
  - if `r` is any other rule: **every** step of `r` leads to a solved state
    (AND over bindings for `load`, over compatible successors for `sketch`
    and `do`, and the single step for `call`).

In words: universal over rules and over the alternatives of every existing
rule kind, existential over the bindings of a `choose` rule. A module
program solves a task iff the initial execution state is solved.

Consequences worth stating in the implementation comments:

- **Backtracking never enters a verified segment.** Sketch rules say any
  compatible successor is acceptable and load rules say any binding is
  acceptable, so the executor commits to the first one and this is sound
  whenever the module program passes verification (section 4.4). The only
  backtrack points are `choose` steps.
- **Alternatives in one rule entry remain AND.** If two variants under one
  rule symbol both apply, both must be solved, as today. If a memory state
  needs "try rule A, else rule B", that is a `choose` over a two-element
  concept, not rule alternatives. Keep this distinction sharp in the
  documentation of the language.
- **Nested choices need nothing new.** A `choose` in `m1` after a `choose`
  in `m0` is an OR node below an OR node. A `load` between two chooses is an
  AND node between two OR nodes. Chooses inside a called module are OR nodes
  whose frames carry the call stack, so exhausting them unwinds the call.

### 4.3 Failure

No construct is added for failure. A branch fails, and the executor
backtracks to the most recent `choose` frame, on any of:

| Outcome | Where it comes from |
|---|---|
| `NO_APPLICABLE_ACTION` | `fallback`: no rule applies and the call stack is empty |
| `MALFORMED_CALL` | `execute_call` |
| revisit of an execution state | section 4.5; subsumes cycles on the current branch and nogoods from abandoned branches |
| `is_unsolvable` from the optional classifier | `ProgramProofBuilder::get_or_create_vertex` |

`OUT_OF_TIME` and `OUT_OF_STATES` abort the whole search as today.

The enum values `SEARCH_FAILURE` and `CYCLE` exist in `ProgramOutcome`
but nothing produces them today. Cycles are currently detected after the
search by `ProgramProofBuilder::finish`, which runs `find_cycle` over
the whole proof graph and downgrades `SUCCESS` to `FAILURE`. That post-hoc
check cannot be kept for the backtracking executor: a cycle inside an
abandoned branch must not fail the overall result. Cycle detection moves
into the executor, per branch, as described in section 4.5. Verification
mode can keep the post-hoc check only if it is restricted to the solved
subgraph; simpler is to let the least-fixpoint labelling handle cycles,
since an unsolved cycle never becomes solved under a least fixpoint.

How to write a failure condition with existing components:

- **Before a choice.** Guard the `choose` rule with a condition, or narrow
  the candidate feature. If the guard is false or the denotation is empty,
  no step is produced. This is the preferred form because the branch is cut
  before the choice is made.
- **After a verified segment.** Lead the offending states into a memory
  state whose outgoing rules do not apply to them. In the entry module this
  is `NO_APPLICABLE_ACTION` and the branch fails. Inside a callee it is
  `RESTORED_CALLER`: the module returns, and the caller must check the
  condition itself after the call. A callee cannot fail the branch on its
  own; it can only return, and the caller decides. This is the one
  ergonomic cost of not adding a construct, and it is accepted.

Because "no rule applies" is also the ordinary way a policy ends and a
module returns, verification must not report every such state as a
counterexample. Section 4.4 says which states are reported.

### 4.4 Two modes, as today

`ProgramSearchOptions::universal` keeps its meaning and both modes
must implement section 4.2.

**Verification (`universal = true`).** Build the proof graph over all steps
as now. Replace the current "any failed leaf means `FAILURE`" with an
AND/OR labelling: propagate *solved* bottom-up per section 4.2, treating
edges labelled with a `choose` rule as OR edges grouped by rule, and every
other edge as AND. The graph is finite because execution states are
interned and `get_or_create_vertex` deduplicates. Cycles in the graph are
unsolved unless they pass through a solved vertex, so run the labelling to a
fixpoint from goal vertices (least fixpoint of *solved*), never a greatest
one.

Result of verification, in addition to `SUCCESS`/`FAILURE`:

- `open_states` and `deadend_states`: as today, but restricted to
  *relevant* unsolved vertices. An unsolved vertex is relevant iff it is
  reachable from the initial vertex along edges that stay in unsolved
  vertices and never leave an OR node through a solved sibling. An open
  state under a `choose` whose other binding is solved is an intended dead
  branch and is not reported. Without this restriction the diagnostics would
  list every rejected branch as an incompleteness of the policy.
- `exhausted_choices` (new): relevant vertices where a `choose` rule applied
  and no binding is solved. This is the counterexample type for an
  incomplete decomposition: the candidate concept is too narrow or a choice
  point is missing.

**Execution (`universal = false`).** Depth-first search over `choose`
frames, greedy everywhere else:

```
frame = (program_state_index, choose_rule, next_binding_index, plan_length)

run(initial):
  state <- initial; stack <- []; plan <- []
  loop:
    if goal(state): return SUCCESS, plan
    steps <- expander.steps(state)
    if steps is empty or the only step has a failure outcome (4.3):
        goto backtrack
    r <- the applicable rule that produced the first step   # today's greedy order
    if r is a choose rule:
        if state in nogoods: goto backtrack
        push (state, r, 0, |plan|)
        state <- target of binding 0
    else:
        state <- target of first step; append plan_suffix
    continue
  backtrack:
    loop:
      if stack empty: return FAILURE
      (st, r, i, n) <- top
      i <- i + 1
      if i == |denotation of r at st|:
          add st to nogoods; pop; continue
      top.next_binding_index <- i
      truncate plan to n
      state <- target of binding i of r at st
      break
```

The frame stores the interned execution state index, so restoring the state
is an index copy and the denotation can be recomputed rather than stored.
Store `plan_length` so the plan prefix can be truncated on backtrack; today
`plan_steps` is only ever appended.

Add to `ProgramSearchOptions`:

- `max_backtracks` (default unlimited) for the executor;
- `use_nogoods` (default true), section 4.5;
- `shuffle_choice_points` already exists and applies to binding order if
  set; the default order stays object index order.

Add to `ProgramProofResults`:

- `num_backtracks`, `num_choice_frames` (peak depth) for evaluation;
- `exhausted_choices` as above.

### 4.5 Nogoods and duplicate detection

An execution state fully determines the executor's behaviour from that
point, because it contains the planning state, the call stack, all
registers and the memory state. Therefore an execution state that was
reached, fully explored over all bindings of its `choose` rule, and not
solved, can never be solved later on a different branch. Record it in a
nogood set keyed by `ProgramStateView` index. Entering a nogood state on
any branch triggers backtracking. This is sound and it is what turns
factorial enumeration into the dynamic-programming recurrence on TSP,
because the planning state records the visited set.

The same argument does not apply to states reached inside a verified
segment, and none is needed there since those are never backtracked.

In execution mode the nogood check collapses to one rule: **revisiting any
interned execution state fails the branch.** A revisited state is either on
the current path, which is a cycle, or on an abandoned branch. Every state
on an abandoned branch is a nogood, because the segment between the failed
leaf and the frame that was popped contains no `choose` frame and is
therefore deterministic, so its outcome is failure. Hence the executor
needs no separate cycle detection and no separate nogood set; the set of
execution states interned so far is both. Solved states are never revisited
because the executor stops at the first goal.

If the domain models a cost bound as a numeric fluent, the bound is part of
the planning state and the nogood key is still exact. If the bound is
imposed by the executor rather than the domain, nogoods are unsound for the
bounded problem; do not combine an executor-side plan-length bound with
nogoods without adding the remaining budget to the key.

### 4.6 Termination

Two separate claims.

**Per branch.** Treat `choose` exactly like `load` in
`structural_termination`: applying it unconstrains the features in Phi(r)
and nothing else. Every branch of the execution is then a run of an
extended sketch in the paper's sense, and Theorem 10 applies unchanged. No
new termination machinery is needed, only the new rule kind in the
projection.

**Choice tree.** The tree is finite because each frame iterates a finite
denotation once, the depth of a branch is bounded by the number of internal
steps of a terminating sketch, and nogoods only remove subtrees. State this
as a lemma in the implementation notes; it is a two-line argument once the
per-branch claim is in place.

Recursive module calls are currently reported non-terminating by
`structural_termination` for module programs. That remains true and is
unaffected.

### 4.7 Complexity

If every branch has at most `d` `choose` steps, each over a denotation of
size at most `n`, and the sketch between choices is reducible, terminating
and of serialized width at most `k` on the class, then the executor runs in
time `O(n^d · poly(N))` with `N` the number of ground atoms, and in
polynomial space. With the current executor `k = 0`: every segment is a
policy and each step is one transition. With nogoods the factor `n^d` is bounded by the number of
distinct execution states, which for TSP is `O(2^n · n)` rather than `n!`.
Plain extended sketches are the case `d = 0`.

## 5. Guarantees to prove and to test

- **Soundness.** Whatever `run` returns is a plan of the task. Follows from
  the fact that every step is a planning transition, a verified sketch
  step, or an internal step.
- **Completeness relative to the program.** If the initial execution state
  is solved in the sense of section 4.2, `run` returns `SUCCESS`. Follows
  from the DFS visiting every binding of every frame unless cut by a sound
  nogood.
- **Agreement of the two modes.** On any task, `find_solution` with
  `universal = true` reports `SUCCESS` iff `run` reports `SUCCESS` within
  unlimited resources. This is the test that pins the semantics down; write
  it as a property test over small instances.

## 6. Worked examples

Schematic; feature expressions assume the obvious predicates and are written
in the existing description-logic grammar.

### 6.1 Single repeated choice: tour with a cost bound

The planning domain has `at(agent, city)`, `visited(city)`, a `move`
action and a cost bound tracked by the domain, for example as a `budget`
predicate over cities that are still affordable. The candidate concept is
the intersection of unvisited and affordable cities, so an over-budget
branch is cut before the choice rather than after.

```lisp
(:module
  (:symbol tour) (:arguments) (:registers (:concept r0))
  (:entry m0) (:memory m0 m1 m2)
  (:features
    (:concept   (:symbol candidates)  (:expression (c_and (c_not (c_atomic_state "visited")) (c_atomic_state "affordable"))))
    (:numerical (:symbol n_candidates) (:expression (n_count (c_and (c_not (c_atomic_state "visited")) (c_atomic_state "affordable")))))
    (:numerical (:symbol n_unvisited) (:expression (n_count (c_not (c_atomic_state "visited")))))
    (:boolean   (:symbol at_r0)       (:expression (b_nonempty (c_and (c_register r0) (c_some (r_inverse (r_atomic_state "at")) (c_top)))))))
  (:rules
    (:rule (:symbol pick) (:expression
      (:source-memory m0) (:target-memory m1)
      (:choose (:conditions (greater_zero n_candidates)) (:concept candidates) (:register (:concept r0)))))
    (:rule (:symbol go) (:expression
      (:source-memory m1) (:target-memory m0)
      (:sketch (:conditions (negative at_r0)) (:effects (positive at_r0) (decrease n_unvisited)))))
    (:rule (:symbol done) (:expression
      (:source-memory m0) (:target-memory m2)
      (:sketch (:conditions (equal_zero n_unvisited)) (:effects))))))
```

In `m0` with unvisited cities left but no affordable one, neither `pick`
nor `done` applies. The module is the entry module, so this is
`NO_APPLICABLE_ACTION` and the branch fails.

The `go` rule is the verified segment: reaching the chosen city is a
subproblem whose goal is defined by the register. The register is reused on
every round because the choice is committed to the planning state through
`visited`.

### 6.2 Dependent choices: box and target

Two registers, the second denotation indexical on the first.

```lisp
(:rules
  (:rule (:symbol pick_box) (:expression
    (:source-memory m0) (:target-memory m1)
    (:choose (:conditions (greater_zero n_unplaced)) (:concept unplaced_boxes) (:register (:concept r0)))))
  (:rule (:symbol pick_goal) (:expression
    (:source-memory m1) (:target-memory m2)
    (:choose (:conditions (greater_zero n_free_goals_r0)) (:concept free_goals_reachable_from_r0) (:register (:concept r1)))))
  (:rule (:symbol push) (:expression
    (:source-memory m2) (:target-memory m0)
    (:call (:conditions) (:callee push_box_to) (:arguments R0 R1)))))
```

If the box in `r0` has no free reachable goal, `pick_goal` produces no
step, nothing else applies in `m1`, and the branch fails if this is the
entry module. Exhausting `pick_goal` pops its frame and advances `pick_box`
to the next box. The call to `push_box_to` is a verified module; if it
stalls it returns to `m0`, where the caller's rules decide whether the
state is acceptable.

### 6.3 Symmetric choices

When two chosen objects play interchangeable roles, make the second
candidate concept indexical on the first with a static less-than role over
object indices, so that only pairs with `r0 < r1` are ever generated.
Without it the executor enumerates both orders; nogoods catch many of the
duplicates but not before the second choice has been made.

## 7. Placement

The extension goes into `runir::kr::ps::ext`, not into a new family.
Decided 2026-09-17.

- A language family in `runir` is a tag that threads through the feature,
  condition and effect type lists, the description-logic constructor
  repository, `DomainContext.ext_repository`, the execution repository,
  serialization, structural termination and the `pyrunir.kr.ps.ext`
  package. A second family would duplicate all of it for one rule kind,
  and every existing module program would have to be re-parsed into the
  new family before it could use a `choose`.
- The extension is backward compatible in both modes. A program without
  `choose` rules has only AND nodes, so the AND/OR labelling is today's
  "every leaf must succeed". The backtracking executor with no frames is
  today's greedy executor. Structural termination treats `choose` like
  `load`, so choose-free programs get the same verdicts.
- What differs is the guarantee, and that is made visible in the API rather
  than in a namespace: `ProgramView::is_deterministic()` is true iff
  no module has a `choose` rule, and Theorem 10 is claimed only for
  deterministic programs. The AND/OR verification and the backtracking
  executor live in new entry points next to `find_solution`; `find_solution`
  keeps its current behaviour for deterministic programs and rejects
  programs with choices, so the ICAPS 2024 behaviour stays reproducible.

## 8. Implementation checklist

Ordered so that each step compiles and tests on its own.

1. **Declarations.** `include/runir/kr/ps/ext/declarations.hpp`: add
   `template<CategoryTag> struct ChooseTag { keyword = "choose"; }`; extend
   `RuleKind`, add `ChooseRuleTypes`, extend `ConcreteRuleTypes`; add
   `ChooseRuleView` concept next to `LoadRuleView`.
2. **Data, index, view.** `rule_data.hpp`: `Data<Rule<ChooseTag<C>>>` with
   the same members as the load rule. Mirror in `rule_index.hpp`, `rule_view.hpp`,
   `rule_variant_*.hpp`, `canonicalization.hpp`, `syntactic_complexity.hpp`
   and the serialization headers under `include/runir/serialization/kr/ps/ext`.
3. **AST, parser, formatter.** `dl/ast/ast.hpp`: `ChooseRule<Category>`
   (same fields as `LoadRule`); add it to the `Rule` variant.
   `src/kr/ps/ext/dl/parser.cpp` and `parser/` grammar: keyword `:choose`,
   reuse the load argument grammar. Formatter:
   round-trip test like `ExtendedModuleFormatterPreservesAlternativeRuleGrouping`.
4. **Expander.** `successor_expander.hpp::append_steps`: `choose` branch
   identical to the load branch, phase `INTERNAL`. Add `choose` to
   `load_steps_until` so internal-phase collection sees it, and to the
   `LoadRuleView || CallTag` case of `selects`.
5. **Verification.** `detail/proof_search.hpp` and `detail/proof_builder.hpp`:
   record, per vertex, the rule that produced each out-edge (already on the
   edge label); after the graph is built, compute the least fixpoint of
   *solved* per section 4.2 and set `status` from the initial vertex.
   Restrict `open_states` and `deadend_states` to relevant vertices and
   populate `exhausted_choices` (section 4.4).
6. **Execution.** Add `ProgramView::is_deterministic()`. Add the
   DFS of section 4.4 as a new entry point next to `find_solution`; make
   `find_solution` reject programs with `choose` rules. Implement the
   revisit rule of section 4.5 on the interned execution states instead of
   a separate nogood set and instead of the post-hoc `find_cycle`. Add the
   new options and counters.
7. **Termination.** `src/kr/ps/ext/dl/structural_termination.cpp` and the
   incomplete preprocessing: treat `ChooseTag<C>` wherever `LoadTag<C>` is
   handled.
8. **Python.** Bind the new rule views, options and result fields in
   `python/`, regenerate stubs, and add `test_choose_*` next to
   `python/tests/kr/ps/ext/test_evaluation.py`.
9. **Tests.**
   - Parser positives and negatives in `tests/fixtures/kr/ps/parser/` and
     `parser_negative.json` (undeclared register, wrong category).
   - Expander: a `choose` over a three-object concept yields three
     `INTERNAL` steps and over an empty concept yields none.
   - Diagnostics: a program with one `choose` whose first binding leads to
     an open state and whose second binding is solved verifies as
     `SUCCESS` with empty `open_states`.
   - Semantics: the agreement property of section 5 on tiny Gripper and
     Blocksworld programs, including a program where a `load` sits between
     two `choose` rules and a program with a `choose` inside a called module.
   - Revisits: a hand-built program whose choice tree has a repeated
     execution state; assert the backtrack count with and without
     `use_nogoods`, and a program with a cycle inside one abandoned branch
     that still reports `SUCCESS`.
   - Regression: every existing `ext` test passes unchanged, and
     `find_solution` on the factory programs returns byte-identical proof
     graphs.
   - Termination: `structural_termination` accepts the tour module of
     section 6.1 and rejects a variant whose `go` rule has no decreasing
     effect.

## 9. Open questions

- **Failure inside callees.** A callee cannot fail a branch; it returns and
  the caller re-checks (section 4.3). If this proves too awkward in
  practice, the smallest addition would be a reserved sink memory state,
  not a rule kind. Do not add it before the need is shown on real programs.
- **Rule alternatives versus choice.** Section 4.2 keeps rule alternatives
  universal. If, in practice, most programs want "try A, else B", consider
  a `(:choose-rule ...)` grouping later rather than changing the meaning of
  existing alternatives.
- **Executor-side bounds.** A plan-length or time bound imposed by the
  executor interacts with nogoods (section 4.5). The first version should
  not offer an executor-side cost bound at all; bounds belong in the
  domain.
- **Variable ordering.** The memory-state sequence fixes a static order of
  choices. Dynamic ordering would be a `choose` over which register to fill
  next and is out of scope.
