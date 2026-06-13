"""PEP 517 backend for the pyrunir wheel; machinery lives in pyyggdrasil.build_support."""

from pyyggdrasil.build_support import ProviderBackend

_BACKEND = ProviderBackend(
    package="pyrunir",
    providers=("pypddl", "pyyggdrasil", "pytyr"),
    cmake_defines=(
        "-DRUNIR_BUILD_PYRUNIR=ON",
        "-DRUNIR_BUILD_TESTS=OFF",
        "-DCMAKE_INSTALL_LIBDIR=lib",
    ),
    rename_packages=("pyrunir", "pytyr", "pypddl", "pyyggdrasil"),
    jobs_env="RUNIR_JOBS",
    strip_env="PYRUNIR_STRIP_WHEEL",
)

_num_jobs = _BACKEND._num_jobs
_prepare_native_build = _BACKEND._prepare_native_build
_fix_wheel_stubs = _BACKEND._fix_wheel_stubs
_strip_wheel_native_libraries = _BACKEND._strip_wheel_native_libraries

_BACKEND.install_hooks(globals())
