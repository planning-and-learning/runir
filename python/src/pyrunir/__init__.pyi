from pathlib import Path

from . import (
    datasets as datasets,
    graphs as graphs, 
    kr as kr,
)

__version__: str

def native_prefix() -> Path: ...
