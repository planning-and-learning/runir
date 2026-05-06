from pathlib import Path

from ._core import FeatureSummary, add, describe, format_summary


def native_prefix() -> Path:
    return Path(__file__).resolve().parent / "native"


__all__ = [
    "FeatureSummary",
    "add",
    "describe",
    "format_summary",
    "native_prefix",
]

