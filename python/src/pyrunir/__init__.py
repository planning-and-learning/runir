from pathlib import Path

from . import graphs, kr


def native_prefix() -> Path:
    return Path(__file__).resolve().parent / "native"
