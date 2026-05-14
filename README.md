# Runir

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

- `pytyr >= 0.0.13` for Tyr planning, formalism, search, and C++ headers/libraries.
- `pypddl >= 1.0.4` through Tyr/PDDL parsing infrastructure.
- `scikit-build-core` for Python wheel builds.

## Build C++

```console
python -m pip install 'pyyggdrasil>=0.0.8' 'pypddl>=1.0.4' 'pytyr>=0.0.13'
export CMAKE_PREFIX_PATH="$(python -c 'import os, pypddl, pyyggdrasil, pytyr; print(os.pathsep.join(map(str, [pypddl.native_prefix(), pyyggdrasil.native_prefix(), pytyr.native_prefix()])))')"
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Debug \
  -DRUNIR_BUILD_TESTS=ON \
  -DRUNIR_BUILD_SHARED=ON \
  -DRUNIR_LINK_STATIC_DEPENDENCIES=OFF
cmake --build build -j4
ctest --test-dir build --output-on-failure
```

## Build Python

```console
python -m pip install .[test]
pytest python/tests
```

## Intended Workflow

Runir is designed for experiments where a learning system needs structured access to planning behavior:

1. Parse and instantiate tasks with Tyr.
2. Build state graphs or equivalence graphs with Runir datasets.
3. Define symbolic features in a KR language such as description logics.
4. Build or parse a generalized policy over those features.
5. Execute the policy on examples and inspect failures or counterexamples.

This keeps low-level planning mechanics in Tyr while giving learning code a high-level, typed interface for representations, abstractions, and policy evaluation.
