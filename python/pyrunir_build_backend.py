import os
from pathlib import Path

from scikit_build_core import build as scikit_build


def _num_jobs() -> int:
    return int(os.environ.get("RUNIR_JOBS", "8"))


def _prepend_cmake_args(*args: str) -> None:
    existing = os.environ.get("CMAKE_ARGS", "")
    os.environ["CMAKE_ARGS"] = " ".join([*args, existing]).strip()


def _native_prefixes() -> list[Path]:
    import pyyggdrasil
    import pytyr

    return [
        pyyggdrasil.native_prefix().resolve(),
        pytyr.native_prefix().resolve(),
    ]


def _native_library_dirs(native_prefixes: list[Path]) -> list[Path]:
    result = []
    for prefix in native_prefixes:
        for path in sorted(prefix.glob("lib*")):
            if path.is_dir():
                result.append(path)
    return result


def _prepend_env_paths(name: str, paths: list[Path]) -> None:
    existing = os.environ.get(name, "")
    entries = [str(path) for path in paths]
    if existing:
        entries.append(existing)
    os.environ[name] = os.pathsep.join(entries)


def _prepare_native_build() -> None:
    native_prefixes = _native_prefixes()
    native_library_dirs = _native_library_dirs(native_prefixes)

    os.environ.setdefault("CMAKE_BUILD_PARALLEL_LEVEL", str(_num_jobs()))
    _prepend_env_paths("LD_LIBRARY_PATH", native_library_dirs)
    _prepend_env_paths("DYLD_LIBRARY_PATH", native_library_dirs)
    _prepend_cmake_args(
        f"-DCMAKE_PREFIX_PATH={';'.join(str(prefix) for prefix in native_prefixes)}",
        "-DBUILD_SHARED_LIBS=ON",
        "-DBUILD_PYRUNIR=ON",
        "-DRUNIR_BUILD_TESTS=OFF",
        "-DRUNIR_LINK_STATIC_DEPENDENCIES=OFF",
        "-DCMAKE_INSTALL_LIBDIR=lib",
    )


def get_requires_for_build_wheel(config_settings=None):
    return scikit_build.get_requires_for_build_wheel(config_settings)


def get_requires_for_build_editable(config_settings=None):
    return scikit_build.get_requires_for_build_editable(config_settings)


def prepare_metadata_for_build_wheel(metadata_directory, config_settings=None):
    return scikit_build.prepare_metadata_for_build_wheel(metadata_directory, config_settings)


def build_wheel(wheel_directory, config_settings=None, metadata_directory=None):
    _prepare_native_build()
    return scikit_build.build_wheel(wheel_directory, config_settings, metadata_directory)


def build_editable(wheel_directory, config_settings=None, metadata_directory=None):
    _prepare_native_build()
    return scikit_build.build_editable(wheel_directory, config_settings, metadata_directory)


def build_sdist(sdist_directory, config_settings=None):
    return scikit_build.build_sdist(sdist_directory)
