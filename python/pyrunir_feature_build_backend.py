import os

from scikit_build_core import build as scikit_build


def _num_jobs() -> int:
    return int(os.environ.get("TEMPLATE_FEATURE_JOBS", "8"))


def _prepend_cmake_args(*args: str) -> None:
    existing = os.environ.get("CMAKE_ARGS", "")
    os.environ["CMAKE_ARGS"] = " ".join([*args, existing]).strip()


def _yggdrasil_prefix() -> str:
    import pyyggdrasil

    return str(pyyggdrasil.native_prefix().resolve())


def _prepare_native_build() -> None:
    yggdrasil_prefix = _yggdrasil_prefix()

    os.environ.setdefault("CMAKE_BUILD_PARALLEL_LEVEL", str(_num_jobs()))
    _prepend_cmake_args(
        f"-DCMAKE_PREFIX_PATH={yggdrasil_prefix}",
        "-DBUILD_SHARED_LIBS=ON",
        "-DBUILD_PYTEMPLATE_FEATURE=ON",
        "-DTEMPLATE_FEATURE_BUILD_TESTS=OFF",
        "-DTEMPLATE_FEATURE_LINK_STATIC_DEPENDENCIES=OFF",
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

