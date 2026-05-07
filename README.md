# Runir

Description logic feature library with Python bindings, CMake package exports, and downstream integration tests.

Runir uses:

- `pyyggdrasil` for native dependencies.
- `pytyr` for Tyr planning/formalism support.
- CMake exported target `runir::core`.
- Python package `pyrunir`.
- `nanobind` for bindings.
- `scikit-build-core` for wheels.



## Build C++

```console
python -m pip install pyyggdrasil>=0.0.7 pytyr>=0.0.13
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Debug \
  -DRUNIR_BUILD_TESTS=ON \
  -DCMAKE_PREFIX_PATH="$(python -c 'import pyyggdrasil; print(pyyggdrasil.native_prefix())')"
cmake --build build -j4
ctest --test-dir build --output-on-failure
```

## Build Python

```console
python -m pip install .[test]
pytest python/tests
```
