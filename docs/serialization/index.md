# Serialization output

Runir uses `ygg::serialization::Dictionaries` to produce JSON values. Registering
a native type assigns its values references such as `f0` or `r0` and stores
their declared fields in table rows. Unregistered native entities use their
existing text formatter, equivalent to Python `str(value)`. Table names and
prefixes are supplied by the caller.
Rows follow first encounter order, and repeated native values reuse their row.

Runir and Tyr share one registry. A Runir execution state can reference a Tyr
state from the same `states` table:

```json
{
  "state": "s0",
  "program": "p0",
  "phase": "EXTERNAL",
  "call_stack": "c0"
}
```

`tables()` returns each registered table as `{"prefix": "s", "rows": [...]}`.
The row position supplies the integer part of its reference. References remain
stable across Tyr and Runir serialization calls on the same registry. Enum
fields use their native text, such as `EXTERNAL`. Registered variants store the
native alternative's type name directly in `kind`. Ordinary numeric data,
including graph indices and feature values, remains numeric.

For example, a registered rule stores its symbol and lists of conditions and
effects. Unregistered conditions and effects are native strings such as
`(greater_zero n)` and `(decreases n)`, rather than nested variant objects.
A registered feature stores its symbol and native expression text unless the
expression type is also registered. Registered variant rows contain `kind`
and `value`.

Text formatting stops recursive collection. Registering a descendant alone
does not collect it through an unregistered parent: serialize the descendant
directly or register the intervening types. Lists remain arrays, ordinary
numeric values remain numbers, and absent optional values become `null`.

Unregistered graphs and native labels likewise use their native text
formatters. Structural-termination policy graphs are not serialized yet.

Tyr state rows contain changing state facts. Task metadata and static facts
are available by serializing the task separately. Runir register values and
denotations also use Tyr serializers for their objects and object pairs.

The native entry point is `<runir/serialization/serialization.hpp>`. In Python,
`pyrunir.serialization` re-exports Tyr's `Dictionaries` and provides
`register_table(dictionaries, native_type, name, prefix)`,
`serialize(dictionaries, value)`, and `table(dictionaries, native_type)` for
Runir entities. Tyr entities use the registry's corresponding methods.
Both paths populate the same `tables()` output.

Yggdrasil provides shared snapshot types and rendering through
`pyyggdrasil.serialization`:

```python
from pyyggdrasil.serialization import render_table

for name, snapshot in dictionaries.tables().items():
    print(name)
    print(render_table(snapshot["rows"], prefix=snapshot["prefix"]))
```

Nested dictionaries expand into columns with grouped headers. Lists render as
compact JSON cells; other leaves use scalar formatting. Pass `tablefmt="github"`
for Markdown; `tabulate` owns layout and scalar formatting. Callers can add
columns to snapshot rows before rendering. Match annotations by reference,
and retain snapshot row order when deriving references with `prefix`. For
sorted or filtered rows, carry their original references as explicit cells
and omit `prefix`. Snapshot edits do not modify the registry.

Register all tables before the first serialization. Native repositories and
graphs must remain valid while their values are used by the registry.
Registration controls both deduplication and where structural traversal
continues. Every serialized native type must provide a text formatter; a
missing formatter is a compile-time error. Output selection, annotations,
and file handling belong to the application.
