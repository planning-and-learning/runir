# C++/Python Project Template

Template for a planning-and-learning C++ library with Python bindings, CMake package exports, and downstream integration tests.

The sample project uses:

- `pyyggdrasil` for native org dependencies.
- CMake exported target `template_feature::core`.
- Python package `pytemplate_feature`.
- `nanobind` for bindings.
- `scikit-build-core` for wheels.

Rename the `template_feature` and `pytemplate_feature` identifiers when creating a real project.

## Build C++

```console
python -m pip install pyyggdrasil==0.0.6
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Debug \
  -DTEMPLATE_FEATURE_BUILD_TESTS=ON \
  -DCMAKE_PREFIX_PATH="$(python -c 'import pyyggdrasil; print(pyyggdrasil.native_prefix())')"
cmake --build build -j4
ctest --test-dir build --output-on-failure
```

## Build Python

```console
python -m pip install .[test]
pytest python/tests
```

