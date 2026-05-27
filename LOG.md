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
- Retargeted the imported `TBB::tbb` target to pytyr's bundled TBB when consuming installed `pytyr`; `libtyr_core` already uses that runtime, while pyyggdrasil's TBB causes duplicate-runtime lifted datalog teardown crashes.
- Moved module execution helpers under `kr/ps/ext/detail`, leaving the public execution header as a compatibility include, and removed stale action/cost/call-depth/repository-owner data from module execution/proof labels.
- Kept explicit `TBB::tbb` linkage for C++ tests so inline tyr/TBB use resolves without depending on transitive DSO symbols.
- Added `pytyr.libs` to pyrunir native RPATHs so direct links to pytyr bundled hashed shared libraries resolve without relying on another DSO's RUNPATH.
- Added source-offset tracking to the module S-expression parser and moved module-program call signature validation into parser/lowering, including typed boolean/numerical call arguments.
- Extended module parser AST source-offset tracking through module lowering, so missing program entries, memory states, registers, and feature references are rejected with input offsets before execution.
- Added an x3-backed S-expression syntax preflight for module parser inputs so malformed parentheses/quotes report through Boost Spirit x3 error-handler positions before structural validation.
- Moved the module-program proof search loop into `kr/ps/ext/detail/proof_search.hpp`, leaving public `prove_solution` as a thin paper-level wrapper around detail execution/proof machinery.
- Made `TaskSearchContext` construction factory-only via `TaskSearchContext::create(...)`, changed `TaskSearchContextList` to hold shared pointers, and updated C++/Python call sites to pass shared context owners.
