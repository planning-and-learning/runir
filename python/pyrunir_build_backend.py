# Copyright (C) 2026 Dominik Drexler
# SPDX-License-Identifier: GPL-3.0-or-later

"""PEP 517 backend for the pyrunir wheel; machinery lives in pyyggdrasil.build_support."""

from pyyggdrasil.build_support import ProviderBackend

ProviderBackend(
    package="pyrunir",
    providers=("pypddl", "pytyr", "pyyggdrasil"),
    jobs_env="RUNIR_JOBS",
    strip_env="RUNIR_STRIP_WHEEL",
).install_hooks(globals())
