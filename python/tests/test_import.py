import pyrunir


def test_python_bindings_expose_core_api():
    summary = pyrunir.describe("planner", 7)

    assert pyrunir.add(20, 22) == 42
    assert pyrunir.format_summary(summary) == "planner: 7"
    assert summary.name == "planner"
    assert summary.score == 7


def test_native_prefix_points_to_installed_cmake_package():
    prefix = pyrunir.native_prefix()

    assert (prefix / "include" / "runir" / "runir.hpp").exists()
    assert (prefix / "lib" / "cmake" / "runir" / "runirConfig.cmake").exists()

