"""PEP 517 backend for the pyrunir wheel; machinery lives in pyyggdrasil.build_support."""

from pyyggdrasil.build_support import ProviderBackend

ProviderBackend(
    package="pyrunir",
    providers=("pypddl", "pytyr", "pyyggdrasil"),
    jobs_env="RUNIR_JOBS",
    strip_env="RUNIR_STRIP_WHEEL",
).install_hooks(globals())
