# Integration notes

## Python serialization

`pytyr.serialization` and `pyrunir.serialization` provide structured native
representations using a shared dictionary registry. Callers select the entity
types to deduplicate and assign table names and reference prefixes.

Search, execution, and validation use the native APIs directly. Downstream
applications combine the representations and choose how to render and save
them. Output documentation lives under each repository's `docs/serialization/`
directory.

## Table rendering

`pyyggdrasil.serialization.table.render_table` adapts serialized rows to `tabulate`,
encoding nested JSON cells compactly and optionally adding dictionary
references. `tabulate` provides the layouts and scalar formatting. Applications
choose their columns and files.
