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
- Changed planning-sketch repository factories so `create(...)` returns `std::shared_ptr<Repository>` directly, removed the `create_shared` alias, and updated C++/Python call sites to keep repository ownership explicit.
- Moved static `do` action validation into module lowering: unknown action names and action arity mismatches now fail in the parser/lowering path with source offsets instead of surfacing as execution-time no-action failures.
- Shared module-set validation between `parse_module_program(...)` and `parse_modules(...)`, so duplicate module names, unknown callees, and call signature mismatches are rejected before lowering with source offsets.

## 2026-05-27

- Labeled module-program proof vertices with explicit internal/external memory-state variants, keyed proof configurations by the variant kind, and covered the successful paper-module proof graph with assertions for both memory-state kinds.
- Tightened module parser structural validation: modules now reject missing or duplicate required sections, empty `:memory`, and duplicate program `:entry` sections with source-offset diagnostics before lowering/execution.
- Stopped exporting module execution internals through the public `kr/ps/ext/execution.hpp`; tests that exercise low-level execution now include the `detail` header explicitly.
- Added parser validation for rule sections: duplicate sections, malformed section nodes, and sections invalid for a rule kind now fail at the offending section offset instead of being ignored by rule lowering.
- Moved module declaration identifier validation into lowering: duplicate/out-of-range argument identifiers and duplicate/out-of-range register identifiers now fail with AST source offsets before repository construction or execution.
- Added module-level expression reference validation before lowering: feature, load, do, and call expressions now reject out-of-range argument references and undeclared/out-of-range register references at the expression source offset.
- Preserved nested DL expression offsets during module expression validation, so invalid argument/register references now report the offending subexpression or identifier token instead of only the outer expression.
- Aligned DL constructor repository factories with the shared-ownership API shape: `create(...)` now returns the owning repository pointer directly, and the duplicate `create_shared(...)` alias was removed from base, grammar, and CNF grammar constructor repositories.
- Rebuilt and smoke-tested the Python extension after the factory ownership cleanup; updated Python module-parser tests so unknown `do` actions are expected to fail in parser/lowering with offsets instead of reaching execution.
- Carried rule `:arguments` section offsets through the module parser AST so `do` action arity errors now point at the offending arguments section instead of the whole rule.
- Audited the remaining module executor failure statuses after parser/lowering validation: empty load denotations, no concrete applicable actions, and no state-dependent applicable rules remain runtime outcomes; malformed calls are still guarded in detail execution for manually assembled repository views, while parsed module sets/programs reject unknown callees and signature mismatches before lowering.
- Re-ran focused verification with two build jobs: `runir_kr_ps_ext_module_program_executor`, `runir_kr_ps_sketch_executor`, and `runir_kr_grammar_factory` all passed; rebuilt the Python extension with `RUNIR_JOBS=2` and reran the focused grammar/module parser Python tests.
- Refactored module-program proof vertex deduplication to use `ModuleProgramProofVertexLabel<Kind>` directly as the lookup key; `ExtendedState` identity now ignores proof-only annotations and keys on the planning state, transient memory phase, and register valuations, with the active `ModuleView` supplied by the vertex label.
- Removed the string-based module-program configuration history and deleted the separate solution-search path; module `find_solution` now returns the same `ModuleProgramProofResults` shape as `prove_solution`, including final state and plan data, through a greedy solution-path executor instead of dispatching through the proof wrapper.
- Changed sketch `find_solution` to return `SketchProofResults` backed by the same `SketchProofGraph` type as proofs, with greedy execution delegated to Tyr SIW and then projected into one proof-graph path.
