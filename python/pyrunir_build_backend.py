# Copyright (C) 2026 Dominik Drexler
# SPDX-License-Identifier: GPL-3.0-or-later

"""PEP 517 backend for the pyrunir wheel; machinery lives in pybrokkr.build_backend."""

from pybrokkr.build_backend import ProviderBackend

ProviderBackend(
    package="pyrunir",
    providers=("pypddl", "pytyr", "pyyggdrasil", "pyurd"),
    prefix_defines={"pyyggdrasil": "YGGDRASIL_NATIVE_PREFIX", "pyurd": "URD_NATIVE_PREFIX"},
    jobs_env="RUNIR_JOBS",
    strip_env="RUNIR_STRIP_WHEEL",
).install_hooks(globals())
