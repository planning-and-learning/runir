import json
import os
import shutil
import subprocess
import sys
from pathlib import Path

import pytemplate_feature
import pyyggdrasil
import pytest


DOWNSTREAM_PACKAGE_DIR = Path(__file__).resolve().parent / "minimal_downstream_package"


def test_downstream_python_binding_links_installed_template_feature(tmp_path):
    cmake = shutil.which("cmake")
    if cmake is None:
        pytest.skip("cmake is required for the downstream binding smoke test")

    template_prefix = Path(pytemplate_feature.native_prefix())
    yggdrasil_prefix = Path(pyyggdrasil.native_prefix())
    template_cmake_dir = template_prefix / "lib" / "cmake" / "template_feature"
    template_library_dir = template_prefix / "lib"

    if not (template_cmake_dir / "template_featureConfig.cmake").exists():
        pytest.skip("pytemplate_feature was not installed with template_feature CMake package files")

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
            f"-DCMAKE_PREFIX_PATH={template_prefix};{yggdrasil_prefix}",
            f"-Dtemplate_feature_DIR={template_cmake_dir}",
            f"-DPython_EXECUTABLE={sys.executable}",
            f"-DDOWNSTREAM_TEMPLATE_FEATURE_RUNTIME_LIBRARY_DIR={template_library_dir}",
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
                "import json, downstream_template_user; "
                "print(json.dumps({"
                "'formatted': downstream_template_user.format_summary('planner', 7), "
                "'sum': downstream_template_user.add(20, 22)"
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
