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


__version__: str

def native_prefix() -> Path: ...
def native_include_dir() -> Path: ...
def native_lib_dir() -> Path: ...
def cmake_prefix() -> Path: ...
def cmake_dir() -> Path: ...
