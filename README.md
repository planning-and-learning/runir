# Runir

Runir is a learning-oriented planning library built on top of Tyr. Its focus is on datasets and knowledge representation languages for learning in planning: constructing planning datasets, representing state-space structure, and generating symbolic features that can be evaluated on Tyr planning tasks.

Runir currently provides:

- Dataset utilities for planning tasks, task classes, state graphs, and equivalence-based graph construction.
- Knowledge representation language infrastructure under `runir::kr`, currently including description-logic constructors, grammars, denotations, evaluation, and generation.
- Graph data structures and algorithms used by the dataset and learning layers.
- C++ and Python APIs backed by Tyr's planning, formalism, and search infrastructure.

The native CMake package exports `runir::core`; the Python package is `pyrunir`.

Runir depends on:

- `pyyggdrasil` for native dependencies.
- `pytyr` for Tyr planning/formalism/search support.
- `nanobind` for bindings.
- `scikit-build-core` for wheels.

## Build C++

```console
python -m pip install pyyggdrasil>=0.0.8 pytyr>=0.0.12
export CMAKE_PREFIX_PATH="$(python -c 'import os, pypddl, pyyggdrasil, pytyr; print(os.pathsep.join(map(str, [pypddl.native_prefix(), pyyggdrasil.native_prefix(), pytyr.native_prefix()])))')"
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Debug \
  -DRUNIR_BUILD_TESTS=ON
cmake --build build -j4
ctest --test-dir build --output-on-failure
```

## Build Python

```console
python -m pip install .[test]
pytest python/tests
```
