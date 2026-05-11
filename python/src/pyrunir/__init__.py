from pathlib import Path

# Register pytyr nanobind types before pyrunir exposes signatures that refer
# to tyr C++ types. This keeps generated stubs typed against pytyr classes
# instead of falling back to raw C++ spellings.
import pytyr.common as _pytyr_common  # noqa: F401
import pytyr.formalism.planning as _pytyr_formalism_planning  # noqa: F401
import pytyr.planning as _pytyr_planning  # noqa: F401
import pytyr.planning.ground as _pytyr_planning_ground  # noqa: F401
import pytyr.planning.lifted as _pytyr_planning_lifted  # noqa: F401

from . import (
    graphs as graphs, 
    kr as kr,
)

def native_prefix() -> Path:
    return Path(__file__).resolve().parent / "native"
