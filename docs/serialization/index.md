# Serialization output

Runir uses `ygg::serialization::Dictionaries` to produce JSON values. Registering
a native type assigns its values references such as `f0` or `r0` and stores
their declared fields in table rows. Every encountered native entity type
must be registered. An unregistered type raises `ValueError` in Python
(`std::invalid_argument` in C++) with `Unregistered serialization type: ...`.
Table names and prefixes are supplied by the caller.
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

For example, a registered rule stores its symbol and lists of condition and
effect references. Their types, including any intervening variants, must also
be registered unless the corresponding fields are omitted or explicitly
converted to text in a projection. Registered variant rows contain `kind` and
`value`.

Registering a descendant alone does not make its parents serializable. Lists
remain arrays, ordinary numeric values remain numbers, and absent optional
values become `null`. To store native text instead of following references,
return `str(value)` explicitly from a `project` callback.

Types that cannot be registered, including graphs and some native owners,
must be projected through their supported components or converted to text
explicitly. Structural-termination policy graphs are not serialized yet.

Tyr state rows contain changing state facts. Task metadata and static facts
are available from a registered formalism task view. Runir register values and
denotations also use Tyr serializers for their objects and object pairs.

The native entry point is `<runir/serialization/serialization.hpp>`. In Python,
import the shared `Dictionaries` registry from `pyyggdrasil.serialization`.
Runir provides `register_table(dictionaries, native_type, name, prefix, fields=None, project=None)`,
`serialize(dictionaries, value)`, and `table(dictionaries, native_type)` in
`pyrunir.serialization`. These functions support both Runir and Tyr entities.
Tyr's corresponding free functions support Tyr entities and can use the same
registry. Its `tables()` method returns the collected snapshots.

Use `fields(native_type)` to discover the default native columns without an
instance or registry. The result follows declaration order:

```python
from pyrunir.kr.ps import base
from pyrunir.serialization import fields

assert fields(base.Rule) == ["symbol", "conditions", "effects"]
```

The same C++ declaration provides both these names and the accessors used when
serializing a registered value:

```cpp
namespace ygg::serialization
{
template<typename Archive, typename C>
void describe_fields(Archive& ar, std::type_identity<View<Index<runir::kr::ps::base::Rule>, C>>)
{
    ar.field("symbol", [](const auto& value) -> decltype(auto) { return (value.get_symbol()); });
    ar.field("conditions", [](const auto& value) -> decltype(auto) { return (value.get_conditions()); });
    ar.field("effects", [](const auto& value) -> decltype(auto) { return (value.get_effects()); });
}
}
```

Discovery does not invoke the accessors. It describes the default layout only;
registration selections and `project` callbacks do not change its result.

The optional `fields` sequence selects immediate schema fields before recursive
serialization. Omitted fields do not collect descendants. `None` keeps all
fields; an empty sequence creates empty rows. Retained fields follow schema
order, regardless of selection order. Each native type exposes a `Fields` enum
from the same declaration. For example, this registration omits effects:

```python
from pyrunir.serialization import register_table
from pyyggdrasil.serialization import Dictionaries

dictionaries = Dictionaries()
register_table(
    dictionaries, base.Rule, "rules", "r",
    fields=[base.Rule.Fields.symbol, base.Rule.Fields.conditions],
)
```

Generated overloads associate each native type with its own `Fields` enum so
type checkers can detect selections from another type. Runir accepts Tyr's
existing field enums when registering Tyr entities. String selections remain
available, including arbitrary column names returned by a projection.

The optional `project` callable receives a native entity and returns a complete
row dictionary with arbitrary column names and transformed values. Its output
replaces the declared fields. Returned native Runir and Tyr entities are
recursively serialized through the same registry. When both options are given,
`fields` selects projected names before recursive conversion, preserving the
callable's column order. Without `project`, the native schema is unchanged.
For example, keep rule conditions and effects as native text without collecting
their descendants:

```python
dictionaries = Dictionaries()
register_table(
    dictionaries, base.Rule, "rules", "r",
    project=lambda rule: {
        "symbol": rule.get_symbol(),
        "conditions": [str(condition) for condition in rule.get_conditions()],
        "effects": [str(effect) for effect in rule.get_effects()],
    },
)
```

Or represent a Tyr action binding with its name and references to objects:

```python
from pyrunir.serialization import register_table
from pytyr.formalism import planning as fp
from pyyggdrasil.serialization import Dictionaries

dictionaries = Dictionaries()
register_table(dictionaries, fp.Object, "objects", "o")
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

Nested dictionaries expand into columns with grouped headers. Simple lists use
brackets and commas, such as `[a0,a1]`; nested or ambiguous lists use compact JSON.
The default layout uses unpadded pipe separators; pass `aligned=True` to pad
columns. Neither layout adds decorative lines. Callers can add
columns to snapshot rows before rendering. Match annotations by reference,
and retain snapshot row order when deriving references with `prefix`. For
sorted or filtered rows, carry their original references as explicit cells
and omit `prefix`. Snapshot edits do not modify the registry.

Register all tables before the first serialization. Native repositories and
graphs must remain valid while their values are used by the registry.
Registration controls both deduplication and where structural traversal
continues. After a serialization error, create a new registry: collection may
already have started before an unregistered descendant was encountered.
Output selection, annotations, and file handling belong to the application.
