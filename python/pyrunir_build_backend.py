"""PEP 517 backend for the pyrunir wheel; machinery lives in pyyggdrasil.build_support."""

from pyyggdrasil.build_support import ProviderBackend

ProviderBackend(
    package="pyrunir",
    providers=("pypddl", "pytyr", "pyyggdrasil"),
    jobs_env="RUNIR_JOBS",
).install_hooks(globals())
