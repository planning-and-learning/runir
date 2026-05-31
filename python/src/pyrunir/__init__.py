from pathlib import Path

# Register pytyr nanobind types before pyrunir exposes signatures that refer
# to tyr C++ types. This keeps generated stubs typed against pytyr classes
# instead of falling back to raw C++ spellings.
import pytyr.formalism.planning as _pytyr_formalism_planning  # noqa: F401
import pytyr.planning as _pytyr_planning  # noqa: F401
import pytyr.planning.ground as _pytyr_planning_ground  # noqa: F401
import pytyr.planning.lifted as _pytyr_planning_lifted  # noqa: F401

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
