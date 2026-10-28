# Datalog programs in policies

A module (`ps::ext`) may define relations by stratified Datalog rules in a `:datalog` section, placed after `:memory`
and before `:features`; sketches (`ps::base`) have no such section. Features read a rule head with the query constructor
`q_head`.
Rules are evaluated by yggdrasil's `ygg::database::datalog::Evaluator`.

```
(:module (:symbol entry) (:arguments) (:registers) (:entry m0) (:memory m0)
  (:datalog
    (:rule (reach ?x) (marker ?x))
    (:rule (reach ?y) (reach ?x) (edge ?x ?y))
    (:rule (degree ?x ?n) (reach ?x) (count ?n ((edge ?x ?y))))
    (:rule (unmarked ?x) (reach ?x) (not (marker ?x)))
    (:rule (achieved ?x) (triple ?x ?y ?z) (goal triple ?x ?y ?z)))
  (:features
    (:numerical (:symbol reached) (:expression (n_count (q_head reach (x)))))
    (:numerical (:symbol plain) (:expression (n_count (q_head unmarked (x)))))
    (:numerical (:symbol total) (:expression (n_add reached plain))))
  (:rules))
```

## Rules

A head whose last term is `(min t)` keeps, per row of its other terms, the least value of `t` its rules derive
(aggregates in recursion, Zaniolo et al., TPLP 2017; evaluated in order of value, Knuth's generalization of
Dijkstra's algorithm). Every rule of such a head has the `(min ...)`; it computes the value from its body's minimized
atoms' values only with `+` and `max` and uses those values nowhere else; a base rule may give a number:

```
(:rule (dist ?x (min 0)) (marker ?x))
(:rule (dist ?y (min ?v)) (dist ?x ?w) (edge ?x ?y) (= ?v (+ ?w 1)))
(:rule (to-target (min ?w)) (dist ?x ?w) (target ?x))
```

Other rules read such a head like any relation, its least value as its last column.

`(:rule HEAD LITERAL...)`, the head an atom over distinct variables. Literals:

- `(r t...)`: `r` is a head of the section, a feature (below), a register or argument of the module (a concept one has
  one column, a role one two; a concept register holds one object, a role register one pair; a Boolean argument is
  nullary, with its row iff true; a numerical argument has one row, holding its value), or a domain predicate
  (static, fluent or derived; names as the PDDL parser stores them, lower case; `object` holds every object);
- `(goal p t...)`: the goal's positive literals of predicate `p`; `(goal (not (p t...)))` its negative literals;
- `(not (r t...))`, and `(not (goal ...))` for a goal atom the goal does not contain; `(= a b)`, `(!= a b)`, `(< a b)`,
  `(<= a b)`, `(> a b)`, `(>= a b)`;
- `(= ?z (op a b))` with `op` one of `+ - * / min max`, binding `?z` from bound terms (in order, so later literals may
  read it). Numbers are runir's: the largest value is infinity, `+` and `*` saturate to infinity, `-` at 0, division by
  0 is infinity;
- aggregates `(count ?n (LITERAL...))`, `(sum ?n ?v (LITERAL...))`, `(min ?n ?v (...))`, `(max ?n ?v (...))`, grouped by
  the variables that the rule's other atoms bind; COUNT and SUM are 0 for an empty group.

Terms are `?variables`, domain constants, or numbers (digits). Every variable of the head, a negated atom or a comparison must occur in a
positive atom; negation and aggregation must be stratified. Violations are reported at the offending rule.

## Names

- `(q_head r (columns))` is a query: the rows of head `r`, with the given column names.
- `(:concept (:symbol s) (:head r))` is the rows of the unary head `r`, `(:role (:symbol s) (:head r))` those of the
  binary head `r` (`c_project`/`r_project` of `q_head`); they can be loaded into registers like any concept or role
  feature. A module prints such features back as `(:head r)` and its `:datalog` rules as written, so that a printed
  module parses again.
- `(:boolean (:symbol s) (:head r))` is true iff the nullary head `r` has its row (`b_nonempty` of `q_head`);
  `(:numerical (:symbol s) (:head r))` is the least value of the unary head `r`, infinity if it has no row (`n_value`
  of `q_head`). The column should hold numbers, as a `(min ...)` head, an aggregate or arithmetic gives them:

  ```
  (:rule (to-target (min ?w)) (dist ?x ?w) (target ?x))
  (:rule (reachable) (to-target ?w))
  ...
  (:numerical (:symbol distance) (:head to-target))
  (:boolean (:symbol can-reach) (:head reachable))
  ```
- A rule body may read a feature by name: a concept feature (one column), a role feature (two columns), a query feature
  (its columns), a Boolean feature (nullary, its row iff true) or a numerical feature (one row holding the value).
- A DL expression may name another feature: `(n_add reached plain)`, `(c_or start reachable)`, `(n_count pairs)`. The
  name stands for that feature's expression, so evaluation is shared. Bare feature names must not start with the
  reserved prefixes `c_`, `r_`, `b_`, `n_` or `q_`.
- Features are resolved at their first reference, in any order; a feature that reaches itself is an error. Head names
  must differ from predicates and features.
- A section is one program: a rule may not read a feature that reads the program's heads (that would need a second
  program evaluated after the first). Define such a feature as a rule instead.

## Evaluation

Each head that a feature reads has its own `dl::DatalogProgram` record, with that head as its one output, so that
reading a head evaluates only the strata it depends on: the section's rules in the repository's `DatalogRepository`
and one input query per input slot, an empty query (`q_empty`) for the slots the head does not depend on (an atomic state or goal query for a predicate, `q_concept` / `q_role` for a concept or role feature, the
query of a query feature). Each relation carries its name (a head's name, or a description of an input), which `q_head`
prints and which is part of the relation's identity.

- Full evaluation (`evaluate`): the first `q_head` of a program read in a state evaluates it; the others reuse it
  (`DatalogFeature` in the evaluation workspace, a generation per state in `DenotationCaches`).
- Incremental evaluation (`incremental::EvaluationGraph`): a program is one node, which the nodes of its `q_head`s depend
  on. It is initialized from its inputs, or updated from their exact changes (counting and Delete and Rederive in
  yggdrasil). A program is evaluated in a state only if a demanded feature reads one of its heads.

On Autoscale Termes `p16` (the ramp heights still needed for the goal, a 1000-step random walk), full evaluation takes
about 320 µs per state and incremental evaluation 0.4 µs median, 0.8 µs mean (the timing test in `tests/unit/kr/dl/datalog.cpp`).

## DL constructors as rules

Every DL constructor, `n_distance` and `q_must_pass` has a formulation as rules; the test
`DatalogExpressesTheDlConstructors` checks each one (`tests/fixtures/kr/dl/expressivity/cases.json`) against the
constructor along a random walk, in full and incremental evaluation. The patterns, for the head `r`:

| Constructor | Rules |
|---|---|
| `c_not C` | `(r ?x) (object ?x) (not (c ?x))` |
| `c_all R C` | `(viol ?x) (r ?x ?y) (not (c ?y))`; `(r ?x) (object ?x) (not (viol ?x))` |
| `c_some R C` | `(r ?x) (r ?x ?y) (c ?y)` |
| `c_at_least n R C` | `(r ?x) (object ?x) (count ?k ((r ?x ?y) (c ?y))) (>= ?k n)` (`<=`, `=` for `c_at_most`, `c_exactly`) |
| `c_subset R S` | `(viol ?x) (r ?x ?y) (not (s ?x ?y))`; `(r ?x) (object ?x) (not (viol ?x))` |
| `c_fillers R a b` | `(r ?x) (r ?x a) (r ?x b)` |
| `r_complement R` | `(r ?x ?y) (object ?x) (object ?y) (not (r ?x ?y))` |
| `r_transitive_closure R` | `(t ?x ?y) (r ?x ?y)`; `(t ?x ?z) (t ?x ?y) (r ?y ?z)` (reflexive: base `(object ?x) (object ?y) (= ?x ?y)`) |
| `b_lt A B` | `(r) (not (a)) (b)` (Booleans compare as numbers, false < true) |
| `n_count C`, `n_add A B` | `(r ?n) (count ?n ((c ?x)))`; `(r ?z) (a ?x) (b ?y) (= ?z (+ ?x ?y))` |
| `n_distance S E T` | `(d ?x (min 0)) (s ?x)`; `(d ?y (min ?v)) (d ?x ?w) (e ?x ?y) (= ?v (+ ?w 1))`; `(r (min ?w)) (d ?x ?w) (t ?x)` |
| `q_must_pass () S E T P` | reach from `S`; per probe `?x`, `(avoid ?x ?v)` reaches over `E` without `?x`; `(r ?x) (p ?x) (reach ?t) (t ?t) (not (escapes ?x))` with `(escapes ?x) (avoid ?x ?t) (t ?t)` |

Constants (`c_bot`, `b_const`, `n_const`) need a body: `(object ?x)`, with `(!= ?x ?x)` for false. The must-pass rules
compute one reachability per probe, where `q_must_pass` verifies only the objects on one shortest witness path.

