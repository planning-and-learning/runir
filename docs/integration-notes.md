# Integration notes

## Python tools

`pytyr.tools` and `pyrunir.tools` provide compressed representations of native
entities. Runir reuses Tyr's planning dictionaries and adds its own symbolic
definitions. Shared dictionary instances let multiple representations reuse
the same aliases and definitions.

Search, execution, and validation use the native APIs directly. Downstream
applications combine the representations and choose how to render and save
them. Output documentation lives under each repository's `docs/tools/`
directory.

## Table rendering

Use `tabulate` directly where tables are needed. We do not need a custom table
renderer or a `pyyggdrasil.tables` module. The requirement is compact output;
reproducing the exact legacy formatting is unnecessary.
