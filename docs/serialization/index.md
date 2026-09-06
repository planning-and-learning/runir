# Serialization output

Runir uses `ygg::serialization::Dictionaries` to produce JSON values. Registering
a native type assigns its values references such as `f0` or `r0`; unregistered
types appear inline. Table names and prefixes are supplied by the caller.
Rows follow first encounter order, and repeated native values reuse their row.

Runir and Tyr share one registry. A Runir execution state can reference a Tyr
state from the same `states` table:

```json
{
  "state": "s0",
  "program": "p0",
  "phase": "@0",
  "call_stack": "c0"
}
```

`tables()` returns each registered table as `{"prefix": "s", "rows": [...]}`.
The row position supplies the integer part of its reference. Enum fields and
variant kinds use a shared sequence of references such as `@0`. `enums()`
provides their native type, integer ID, and name, for example:

```json
{"ExecutionPhase": [{"ref": "@0", "id": 1, "name": "EXTERNAL"}]}
```

References remain stable across Tyr and Runir serialization calls on the same
registry. Legends are grouped by native type and sorted by native ID; resolve
references through their `ref` field, not their row position. The entity prefix
`@` is reserved. Ordinary numeric data, including graph indices and feature
values, remains numeric.

Feature expressions, conditions, effects, rules, modules, classifier clauses,
and call stacks retain their nested native structure. Variants contain `kind`
and `value`; their kind names appear in `enums()`. Registering expression types
also deduplicates repeated subexpressions. Empty collections remain empty,
and absent optional values become `null`.

Graphs contain `vertices` and `edges`. A vertex has `index` and `property`;
an edge also has `source` and `target` vertex indices. Properties hold the
native labels, including state references, transitions, rules, and flags.
Structural-termination policy graphs are not serialized yet.

Tyr state rows contain changing state facts. Task metadata and static facts
are available by serializing the task separately. Runir register values and
denotations also use Tyr serializers for their objects and object pairs.

The native entry point is `<runir/serialization/serialization.hpp>`. In Python,
`pyrunir.serialization` re-exports Tyr's `Dictionaries` and provides
`register_table(dictionaries, native_type, name, prefix)`,
`serialize(dictionaries, value)`, and `table(dictionaries, native_type)` for
Runir entities. Tyr entities use the registry's corresponding methods.
Both paths populate the same `tables()` and `enums()` output.

Yggdrasil provides shared snapshot types and rendering through
`pyyggdrasil.serialization`:

```python
from pyyggdrasil.serialization import render_table

for name, snapshot in dictionaries.tables().items():
    print(name)
    print(render_table(snapshot["rows"], prefix=snapshot["prefix"]))

for name, entries in dictionaries.enums().items():
    print(name)
    print(render_table(entries))
```

Nested lists and objects render as compact JSON cells. Pass `tablefmt="github"`
for Markdown; `tabulate` owns layout and scalar formatting. Callers can add
columns to snapshot rows before rendering. Match annotations by reference,
and retain snapshot row order when deriving references with `prefix`. For
sorted or filtered rows, carry their original references as explicit cells
and omit `prefix`. Snapshot edits do not modify the registry.

Register all tables before the first serialization. Native repositories and
graphs must remain valid while their values are used by the registry.
Registration controls deduplication; selecting serialization roots controls
which native structures are traversed. Output selection, annotations, and
file handling belong to the application.
