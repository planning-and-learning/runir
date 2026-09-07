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
import the shared `Dictionaries` registry from `pyyggdrasil.serialization`.
Runir provides `register_table(dictionaries, native_type, name, prefix, fields=None, project=None)`,
`serialize(dictionaries, value)`, and `table(dictionaries, native_type)` in
`pyrunir.serialization`. These functions support both Runir and Tyr entities.
Tyr's corresponding free functions support Tyr entities and can use the same
registry. Its `tables()` method returns the collected snapshots.

The optional `fields` sequence selects immediate schema fields before recursive
serialization. Omitted fields do not collect descendants. `None` keeps all
fields; an empty sequence creates empty rows. Retained fields follow schema
order, regardless of selection order. For example,
`register_table(dictionaries, Rule, "rules", "r", fields=["symbol", "conditions"])`
omits effects.

The optional `project` callable receives a native entity and returns a complete
row dictionary with arbitrary column names and transformed values. Its output
replaces the declared fields. Returned native Runir and Tyr entities are
recursively serialized through the same registry. When both options are given,
`fields` selects projected names before recursive conversion, preserving the
callable's column order. Without `project`, the native schema is unchanged.
For example, represent a Tyr action binding with only its name and objects:

```python
from pyrunir.serialization import register_table
from pytyr.formalism import planning as fp
from pyyggdrasil.serialization import Dictionaries

dictionaries = Dictionaries()
register_table(
    dictionaries,
    fp.ActionBinding,
    "actions",
    "a",
    project=lambda binding: {
        "name": binding.get_relation().get_name(),
        "objects": binding.get_objects(),
    },
)
```

`table()` and `tables()` expose generic rows to accommodate selected and
projected columns.

Yggdrasil provides shared snapshot types and rendering through
`pyyggdrasil.serialization.table`:

```python
from pyyggdrasil.serialization.table import render_table

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
