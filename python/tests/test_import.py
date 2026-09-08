from pathlib import Path

import pyrunir
import pytest


def test_source_version_reads_project_table(tmp_path: Path, monkeypatch: pytest.MonkeyPatch) -> None:
    package = tmp_path / "package"
    package.mkdir()
    monkeypatch.setattr(pyrunir, "__file__", str(package / "__init__.py"))
    (package / "pyproject.toml").write_text("[tool.example]\nversion = '9.9.9'\n", encoding="utf-8")
    (tmp_path / "pyproject.toml").write_text("[project]\nversion = '1.2.3' # source version\n", encoding="utf-8")

    assert getattr(pyrunir, "_source_version")() == "1.2.3"


def test_native_prefix_points_to_installed_cmake_package() -> None:
    if not hasattr(pyrunir, "native_prefix"):
        pytest.skip("native_prefix is only available through the installed pyrunir package wrapper")

    prefix = pyrunir.native_prefix()

    assert (prefix / "include" / "runir" / "runir.hpp").exists()
    assert (prefix / "lib" / "cmake" / "runir" / "runirConfig.cmake").exists()
