# Work Log

## 2026-05-26

- Fixed the current `TaskSearchContext` constructor syntax before continuing proof-graph work.
- Removed the temporary const pointer aliases from this work: task search contexts now use only `TaskSearchContextPtr`, and planning-sketch repositories now use only `RepositoryPtr`.
- Added `SketchProofGraph` with `AnnotatedStateGraphVertexLabel` vertices and edge labels containing the concrete `StateGraphEdgeLabel` plus the compatible `RuleView`.
- Added concrete typed memory-state wrappers (`InternalMemoryState`, `ExternalMemoryState`) and `MemoryStateVariant` to make the internal/external memory partition explicit without colliding with the repository entity `MemoryState`.
- Added `ExtendedState` for module proof vertices. It contains only annotated planning state, typed memory state, and fixed-size concept/role register arrays; module/control ownership data stays outside it.
- Simplified module proof edge labels to view/value data: optional `StateGraphEdgeLabel` plus optional `RuleVariantView`.
- Removed borrowed-reference proof/search APIs for sketch and module executors; `TaskSearchContextPtr` is now the only public entry-point type, so Python and C++ callers share the same ownership boundary.
- Moved no parser validation yet; this remains the next item after restoring a clean build and lifted teardown test.
