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

`(:weighted r...)`, first in the section, makes heads weighted: the last column of a weighted head is its weight, and it
keeps per row of the other columns the least weight its rules derive (evaluated in order of weight, Knuth's
generalization of Dijkstra's algorithm). A rule into a weighted head computes its weight from its weighted atoms'
weights only with `+` and `max`, and uses those weights nowhere else; a base rule may give a number:

```
(:weighted dist)
(:rule (dist ?x 0) (marker ?x))
(:rule (dist ?y ?v) (dist ?x ?w) (edge ?x ?y) (= ?v (+ ?w 1)))
```

Other rules read a weighted head like any relation, its weight as a value.

`(:rule HEAD LITERAL...)`, the head an atom over distinct variables. Literals:

- `(r t...)`: `r` is a head of the section, a feature (below), a concept or role register or argument of the module
  (one or two columns; a concept register holds one object, a role register one pair), or a domain predicate
  (static, fluent or derived; names as the PDDL parser stores them, lower case; `object` holds every object);
- `(goal p t...)`: the goal atoms of predicate `p`;
- `(not (r t...))`, `(= a b)`, `(!= a b)`, `(< a b)`, `(<= a b)`, `(> a b)`, `(>= a b)`;
- `(= ?z (op a b))` with `op` one of `+ - * / min max`, binding `?z` from bound terms (in order, so later literals may
  read it). Numbers are runir's: the largest value is infinity, `+` and `*` saturate to infinity, `-` at 0, division by
  0 is infinity;
- aggregates `(count ?n (LITERAL...))`, `(sum ?n ?v (LITERAL...))`, `(min ?n ?v (...))`, `(max ?n ?v (...))`, grouped by
  the variables that the rule's other atoms bind; COUNT and SUM are 0 for an empty group.

Terms are `?variables`, domain constants, or numbers (digits). Every variable of the head, a negated atom or a comparison must occur in a
positive atom; negation and aggregation must be stratified. Violations are reported at the offending rule.

## Names

- `(q_head r (columns))` is a query: the rows of head `r`, with the given column names.
- A rule body may read a concept feature (one column), a role feature (two columns) or a query feature (its columns) by
  name.
- A DL expression may name another feature: `(n_add reached plain)`, `(c_or start reachable)`, `(n_count pairs)`. The
  name stands for that feature's expression, so evaluation is shared. Bare feature names must not start with the
  reserved prefixes `c_`, `r_`, `b_`, `n_` or `q_`.
- Features are resolved at their first reference, in any order; a feature that reaches itself is an error. Head names
  must differ from predicates and features.
- A section is one program: a rule may not read a feature that reads the program's heads (that would need a second
  program evaluated after the first). Define such a feature as a rule instead.

## Evaluation

The program is a `dl::DatalogProgram` record: its rules in the repository's `DatalogRepository` and one input query
per input slot (an atomic state or goal query for a predicate, `q_concept` / `q_role` for a concept or role feature, the
query of a query feature). Each relation carries its name (a head's name, or a description of an input), which `q_head`
prints and which is part of the relation's identity.

- Full evaluation (`evaluate`): the first `q_head` of a program read in a state evaluates the program for all its heads;
  the others reuse it (`DatalogFeature` in the evaluation workspace, a generation per state in `DenotationCaches`).
- Incremental evaluation (`incremental::EvaluationGraph`): a program is one node, which the nodes of its `q_head`s depend
  on. Its evaluator is built at the first evaluation with the heads that the graph's roots read, so other heads are not
  evaluated. It is initialized from its inputs, or updated from their exact changes (counting and Delete and Rederive in
  yggdrasil). A program is evaluated in a state only if a demanded feature reads one of its heads.

On Autoscale Termes `p16` (the ramp heights still needed for the goal, a 1000-step random walk), full evaluation takes
about 320 µs per state and incremental evaluation 0.4 µs median, 0.8 µs mean (the timing test in `tests/unit/kr/dl/datalog.cpp`).
