from pathlib import Path

# Load public native dependency packages before this package loads native extensions.
import pypddl as pypddl
import pytyr as pytyr
import pyyggdrasil as pyyggdrasil

from . import (
    datasets as datasets,
    graphs as graphs,
    kr as kr,
)


def native_prefix() -> Path:
    package_dir = Path(__file__).resolve().parent
    native_dir = package_dir / "native"
    if (native_dir / "include" / "runir").is_dir():
        return native_dir
    for parent in package_dir.parents:
        if (parent / "include" / "runir").is_dir():
            return parent
    return native_dir
