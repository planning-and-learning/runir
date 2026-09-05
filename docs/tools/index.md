# pyrunir.tools

Compressed, JSON-compatible representations of native Runir entities.
Each representation is a plain dictionary with an explicit `TypedDict` schema.
Downstream applications choose how to combine, render, and save them.

## Dictionaries

Shared dictionaries assign compact aliases to selected entities and their
referenced dependencies. Repeated references reuse the same definition.
Representations using the same dictionary instance share those definitions.

Planning entities use `pytyr.tools.Dictionaries`: state references point
to state definitions, which reference atom and function dictionaries. Static
facts belong to the task representation and are not repeated in every state.
Runir adds definitions for its symbolic entities alongside these planning
dictionaries.

Format the dictionaries after the selected entities so that all referenced
definitions are included. Aliases are local to the dictionary instance.

| Dictionary | Alias prefix | Definition |
|---|---|---|
| `features` | `f` | Native `symbol` and `expression`. |
| `rules` | `r` | Sketch rule `symbol` and native `expression`. |
| `module_rules` | `R` | Rule `symbol`, native `expression`, and `source`/`target` memory aliases. |
| `modules` | `M` | Name, arguments, registers, feature aliases, entry memory, memories, and rule aliases. |
| `memories` | `m` | Native memory name. |
| `programs` | `P` | Entry module alias and module aliases. |
| `call_stacks` | `c` | Module and memory aliases, register values, argument values, and optional caller alias. |
| `execution_states` | `e` | Planning state alias, program alias, native phase name, and call-stack alias. |

Runir dictionaries map aliases to definitions. The nested `planning` field contains
Tyr's planning dictionaries. Empty sections are omitted. Aliases identify native
entities, so distinct features with the same symbol retain separate definitions.
Rule expressions retain their native syntax and refer to symbols in their
originating sketch or module.

## Representations

Sketches contain Boolean and numerical feature aliases and rule aliases.
Classifiers contain their symbol, declared feature aliases, and clauses of
`feature`/`polarity` literals. Module programs reference their module definitions.

Proof graphs contain vertices and edges using the native integer graph indices.
Sketch vertices reference planning states; module vertices reference execution
states. Both retain the native `is_initial`, `is_goal`, `is_alive`, and
`is_unsolvable` fields. Edges contain source/target vertex indices and action/rule
aliases. Module edges can omit a planning transition, represented by `null` action
and cost fields.

Policy graphs contain alias-keyed Boolean and numerical valuations and rule
edges. A numerical valuation is a Boolean indicating whether the feature is
greater than zero. Module policy vertices also reference their memory state.

Repeated planning states, execution states, and call stacks share definitions.
Different module executions can reference the same planning state while retaining
their own phase and call stack.
