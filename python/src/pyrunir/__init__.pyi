from pathlib import Path

from . import (
    graphs as graphs, 
    kr as kr,
)

__version__: str

def native_prefix() -> Path: ...
