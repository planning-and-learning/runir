# Runir: Representations for Generalized Planning in C++ and Python

Runir is designed to address several challenges in the integration of learning and planning based systems:

1. **Dataset construction** from planning tasks and classes of tasks over a common planning domain.

2. **State Model Representations** that expose a task's dynamics in a representation suitable for relational learning.

3. **Symbolic feature languages** with formal syntax, semantics, parsing, generation, and evaluation.

4. **Generalized policy evaluation** on planning examples with explicit access to policy failures and counterexamples.

5. **Equivalence abstractions** for comparing states and tasks through graph, object-graph, and equivalence-graph representations.

Runir is the learning and representation layer for the planning-and-learning ecosystem. Tyr provides the planning backend: PDDL/formalism objects, task repositories, state repositories, successor generation, search algorithms, and Python bindings. Runir builds on top of that backend to make planning tasks inspectable and learnable, not only solvable.

## Components

- `runir::graphs`: typed graph data structures, static/dynamic graphs, BGL adapters, shortest paths, strongly connected components, topological sort, color refinement, Weisfeiler-Lehman certificates, and nauty-backed graph isomorphism certificates.
- `runir::datasets`: task classes, per-task search contexts, state graphs, annotated state graphs, object graphs, and equivalence graphs built from policies such as identity or graph-isomorphism reduction.
- `runir::kr::dl`: description-logic constructors, grammars, CNF grammars, denotations, evaluation, parsing, formatting, and grammar factories.
- `runir::kr::gp`: rule-based generalized policies over typed features, conditions, effects, policy parsing, policy factories, and policy execution on annotated state graphs.
- `pyrunir`: Python bindings for the same components with stubs.

The native CMake package exports `runir::core` as the aggregate target and component targets such as `runir::graphs`, `runir::datasets`, and `runir::kr`. The Python package is `pyrunir`.

## Dependencies

- `pyyggdrasil >= 0.0.9` for shared third-party native dependencies.
- `pytyr >= 0.0.19` for Tyr planning, formalism, search, and C++ headers/libraries.
- `pypddl >= 1.0.6` through Tyr/PDDL parsing infrastructure.
- `scikit-build-core` for Python wheel builds.

The shared workspace layout and general Python/CMake integration pattern are
documented in the
[Planning and Learning build instructions](https://github.com/planning-and-learning/.github/blob/main/profile/README.md#local-development).

## Build C++

Install Runir's native dependency providers into the active Python environment,
then configure CMake with their native prefixes:

```console
python -m pip install 'pyyggdrasil>=0.0.9' 'pypddl>=1.0.6' 'pytyr>=0.0.19'

cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_PREFIX_PATH="$(python -c 'import os, pyyggdrasil, pypddl, pytyr; print(os.pathsep.join(map(str, [pyyggdrasil.native_prefix(), pypddl.native_prefix(), pytyr.native_prefix()])))')"

cmake --build build -j4
```

CMake options:

| Option | Default | Description |
| --- | --- | --- |
| `RUNIR_BUILD_TESTS` | `OFF` | Build Runir tests. |
| `RUNIR_BUILD_EXECUTABLES` | `OFF` | Build Runir executables. |
| `RUNIR_BUILD_PYRUNIR` | `OFF` | Build `pyrunir` Python bindings. |
| `RUNIR_ENABLE_FMT_FORMATTERS` | `ON` | Enable Runir's public `fmt::formatter` specializations. |
| `RUNIR_HEADER_INSTANTIATION` | `OFF` | Enable template definitions in public headers at higher compile-time cost. |

Run tests from a build configured with `-DRUNIR_BUILD_TESTS=ON`:

```console
ctest --test-dir build --output-on-failure
```

Install Runir from a configured build directory with:

```console
cmake --install build --prefix=<path/to/installation-directory>
```

## Build Python

```console
python -m pip install .[test]
pytest python/tests
```

## CMake Integration

The Python package `pyrunir` installs Runir's native headers, shared libraries,
and CMake package config under `pyrunir.native_prefix()`. Downstream CMake
projects should include the native prefixes of `pyrunir` and its native package
dependencies in `CMAKE_PREFIX_PATH`:

```console
cmake -S . -B build \
  -DCMAKE_PREFIX_PATH="$(python -c 'import os, pyyggdrasil, pypddl, pytyr, pyrunir; print(os.pathsep.join(map(str, [pyyggdrasil.native_prefix(), pypddl.native_prefix(), pytyr.native_prefix(), pyrunir.native_prefix()])))')"
```

Runir exports the `runir::core` aggregate target and component targets such as
`runir::graphs`, `runir::datasets`, and `runir::kr`.

## Intended Workflow

Runir is designed for experiments where a learning system needs structured access to planning behavior:

1. Parse and instantiate tasks with Tyr.
2. Build state graphs or equivalence graphs with Runir datasets.
3. Define symbolic features in a KR language such as description logics.
4. Build or parse a generalized policy over those features.
5. Execute the policy on examples and inspect failures or counterexamples.

This keeps low-level planning mechanics in Tyr while giving learning code a high-level, typed interface for representations, abstractions, and policy evaluation.
