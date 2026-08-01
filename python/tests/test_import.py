import pyrunir
import pytest

def test_native_prefix_points_to_installed_cmake_package() -> None:
    if not hasattr(pyrunir, "native_prefix"):
        pytest.skip("native_prefix is only available through the installed pyrunir package wrapper")

    prefix = pyrunir.native_prefix()

    assert (prefix / "include" / "runir" / "runir.hpp").exists()
    assert (prefix / "lib" / "cmake" / "runir" / "runirConfig.cmake").exists()
