import json
import os
import shutil
import subprocess
import sys
from pathlib import Path

import pyrunir
import pyyggdrasil
import pytest


DOWNSTREAM_PACKAGE_DIR = Path(__file__).resolve().parent / "minimal_downstream_package"


def test_downstream_python_binding_links_installed_runir(tmp_path):
    cmake = shutil.which("cmake")
    if cmake is None:
        pytest.skip("cmake is required for the downstream binding smoke test")

    runir_prefix = Path(pyrunir.native_prefix())
    yggdrasil_prefix = Path(pyyggdrasil.native_prefix())
    runir_cmake_dir = runir_prefix / "lib" / "cmake" / "runir"
    runir_library_dir = runir_prefix / "lib"

    if not (runir_cmake_dir / "runirConfig.cmake").exists():
        pytest.skip("pyrunir was not installed with runir CMake package files")

    project_dir = tmp_path / "minimal_downstream_package"
    shutil.copytree(DOWNSTREAM_PACKAGE_DIR, project_dir)

    build_dir = tmp_path / "build"
    subprocess.run(
        [
            cmake,
            "-S",
            str(project_dir),
            "-B",
            str(build_dir),
            f"-DCMAKE_PREFIX_PATH={runir_prefix};{yggdrasil_prefix}",
            f"-Drunir_DIR={runir_cmake_dir}",
            f"-DPython_EXECUTABLE={sys.executable}",
            f"-DDOWNSTREAM_RUNIR_RUNTIME_LIBRARY_DIR={runir_library_dir}",
        ],
        check=True,
    )
    subprocess.run([cmake, "--build", str(build_dir), "-j4"], check=True)

    env = os.environ.copy()
    env["PYTHONPATH"] = str(build_dir) + os.pathsep + env.get("PYTHONPATH", "")

    result = subprocess.run(
        [
            sys.executable,
            "-c",
            (
                "import json, downstream_runir_user; "
                "print(json.dumps({"
                "'formatted': downstream_runir_user.format_summary('planner', 7), "
                "'sum': downstream_runir_user.add(20, 22)"
                "}))"
            ),
        ],
        check=True,
        capture_output=True,
        text=True,
        env=env,
    )

    assert json.loads(result.stdout) == {
        "formatted": "planner: 7",
        "sum": 42,
    }
