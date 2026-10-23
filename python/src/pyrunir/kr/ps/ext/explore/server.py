"""MCP server over `pyrunir.kr.ps.ext.explore`: one exploration session per server process.

    python -m pyrunir.kr.ps.ext.explore.server [--domain domain.pddl] [--program module-program.txt]
        [--tasks 'problems/*.pddl'] [--max-states N] [--max-seconds S] [--total-seconds S] [--workers N]

Paths are relative to the workspace given to `open` (default: the server's working directory).
Handles are plain strings: tasks by name (`p04`), vertices as `<task>:v<n>`, rules and features by
name, features also as expressions. Every call is logged to stderr with its duration.
Requires the `mcp` extra: `pip install pyrunir[mcp]`.
"""

from __future__ import annotations

import argparse
import functools
import sys
import time
from pathlib import Path

from mcp.server.mcpserver import MCPServer

from . import Budget, Session

_ARGS: argparse.Namespace
_SESSION: Session | None = None
_CONTRASTS: dict[tuple[str, tuple[str, ...] | None], object] = {}

server = MCPServer(
    "runir-explore",
    instructions=(
        "Explore how the module program behaves on the workspace tasks, centred on its Choose rules. "
        "Start with `chooses`, then `points` and `alternatives` for one rule, `contrast` to see which declared "
        "features order successful above failed bindings, `separation` to test a new feature, and `try_load` "
        "to check whether a Choose can become a Load. Call `open` first."
    ),
)


def _session() -> Session:
    if _SESSION is None:
        raise RuntimeError("call `open` first")
    return _SESSION


def _logged(function):
    @functools.wraps(function)
    def wrapper(*args, **kwargs):
        started = time.monotonic()
        try:
            return function(*args, **kwargs)
        except Exception as error:  # Report to the agent instead of failing the tool call.
            return f"error: {error}"
        finally:
            spent = _SESSION.spent_seconds if _SESSION else 0.0
            print(
                f"[runir-explore] {function.__name__} {kwargs} {time.monotonic() - started:.2f}s search_total={spent:.1f}s",
                file=sys.stderr,
                flush=True,
            )

    return wrapper


def _tasks(session: Session, tasks: list[str] | None):
    return None if tasks is None else [session.task(name) for name in tasks]


def _budget(max_states: int | None, max_seconds: float | None) -> Budget | None:
    if max_states is None and max_seconds is None:
        return None
    return Budget(max_states or _ARGS.max_states, max_seconds or _ARGS.max_seconds)


@server.tool(name="open")
@_logged
def open_workspace(workspace: str = ".") -> str:
    """Open the workspace: its domain, current module program and tasks. Earlier results are discarded."""
    global _SESSION
    root = Path(workspace).resolve()
    tasks = sorted(root.glob(_ARGS.tasks))
    _SESSION = Session(
        root / _ARGS.domain,
        (root / _ARGS.program).read_text(encoding="utf-8"),
        tasks,
        budget=Budget(_ARGS.max_states, _ARGS.max_seconds),
        total_seconds=_ARGS.total_seconds,
        workers=_ARGS.workers,
    )
    _CONTRASTS.clear()
    rules = ", ".join(str(rule) for rule in _SESSION.choose_rules()) or "(none)"
    return (
        f"{len(tasks)} tasks: {', '.join(task.stem for task in tasks)}\nChoose rules: {rules}\n"
        f"per-search budget: {_ARGS.max_states} states, {_ARGS.max_seconds} s; session search budget {_ARGS.total_seconds} s"
    )


@server.tool()
@_logged
def set_program(program: str) -> str:
    """Explore another module program (full `(:program ...)` text), e.g. a draft. Earlier results are discarded."""
    session = _session()
    session.set_program(program)
    _CONTRASTS.clear()
    return f"Choose rules: {', '.join(str(rule) for rule in session.choose_rules()) or '(none)'}"


@server.tool()
@_logged
def run(task: str) -> str:
    """Status, failure category, search statistics, failure vertex and path length of the program on one task."""
    session = _session()
    return str(session.run(session.task(task)))


@server.tool()
@_logged
def chooses(tasks: list[str] | None = None) -> str:
    """One row per Choose rule over the choice points on each task's path to the goal or the failure."""
    session = _session()
    runs = session.runs(_tasks(session, tasks))
    status = ", ".join(f"{run.task}:{run.status}" for run in runs if run.status != "success") or "all succeed"
    return "\n".join([*(str(row) for row in session.chooses(_tasks(session, tasks))), f"tasks not succeeding: {status}"])


@server.tool()
@_logged
def points(rule: str, tasks: list[str] | None = None, limit: int = 20) -> str:
    """Choice points of one Choose rule on paths, most informative first (with failed bindings, then wider)."""
    session = _session()
    return "\n".join(str(point) for point in session.points(session.rule(rule), _tasks(session, tasks), limit)) or "(none)"


@server.tool()
@_logged
def alternatives(point: str, max_states: int | None = None, max_seconds: float | None = None) -> str:
    """Every binding at a choice point (`<task>:v<n>`) in `:order` rank with its outcome and feature deltas.

    Untried bindings are searched from their target only when `max_states` or `max_seconds` is given."""
    session = _session()
    rows = session.alternatives(session.vertex_ref(point), _budget(max_states, max_seconds))
    return "\n".join(str(row) for row in rows)


@server.tool()
@_logged
def contrast(
    rule: str,
    tasks: list[str] | None = None,
    max_points: int = 50,
    max_states: int | None = None,
    max_seconds: float | None = None,
) -> str:
    """Pairs of successful and failed bindings of one rule at the same points, and how each declared feature
    orders them (`max`: higher on the successful binding). Untried bindings count when a budget is given."""
    session = _session()
    result = session.contrast(session.rule(rule), _tasks(session, tasks), _budget(max_states, max_seconds), max_points)
    _CONTRASTS[(rule, tuple(tasks) if tasks else None)] = result
    return str(result)


@server.tool()
@_logged
def separation(feature: str, rule: str, tasks: list[str] | None = None) -> str:
    """How a declared feature or a new expression (c_..., b_..., n_..., registers by name) orders the pairs of the
    last `contrast` for this rule and tasks. Concepts count as membership of the binding at the Choose source."""
    session = _session()
    key = (rule, tuple(tasks) if tasks else None)
    if key not in _CONTRASTS:
        _CONTRASTS[key] = session.contrast(session.rule(rule), _tasks(session, tasks))
    return str(session.separation(session.feature(feature), _CONTRASTS[key]))


@server.tool()
@_logged
def try_load(rule: str, tasks: list[str] | None = None) -> str:
    """Execute and prove the program with this rule's Choose replaced by a Load (first binding, no backtracking)."""
    session = _session()
    return str(session.try_load(session.rule(rule), _tasks(session, tasks)))


@server.tool()
@_logged
def vertex(vertex: str) -> str:
    """Memory state, registers, declared Boolean and numerical feature values, and fluent atoms of `<task>:v<n>`."""
    session = _session()
    return str(session.vertex(session.vertex_ref(vertex)))


def main(argv: list[str] | None = None) -> None:
    global _ARGS
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n", 1)[0])
    parser.add_argument("--domain", default="domain.pddl")
    parser.add_argument("--program", default="module-program.txt")
    parser.add_argument("--tasks", default="problems/*.pddl")
    parser.add_argument("--max-states", type=int, default=1_000_000)
    parser.add_argument("--max-seconds", type=float, default=30.0)
    parser.add_argument("--total-seconds", type=float, default=1800.0)
    parser.add_argument("--workers", type=int, default=8)
    _ARGS = parser.parse_args(argv)
    server.run("stdio")


if __name__ == "__main__":
    main()
