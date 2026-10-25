# Copyright (C) 2026 Dominik Drexler
# SPDX-License-Identifier: GPL-3.0-or-later

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
import ctypes
import functools
import logging
import resource
import sys
import time
from collections.abc import Callable
from dataclasses import dataclass, field
from pathlib import Path
from typing import ParamSpec

from mcp.server.mcpserver import MCPServer

from pyrunir.kr.ps.ext.explore.results import (
    Budget,
    Contrast,
    SessionOptions,
    Task,
)
from pyrunir.kr.ps.ext.explore.session import (
    Session,
    resident_mib,
)

_Params = ParamSpec("_Params")

logger = logging.getLogger(__name__)


@dataclass(slots=True)
class Server:
    """The server's options, its session once `open` was called, and the last contrast per rule and tasks."""

    options: argparse.Namespace = field(default_factory=argparse.Namespace)
    session: Session | None = None
    contrasts: dict[tuple[str, tuple[str, ...] | None], Contrast] = field(
        default_factory=dict[tuple[str, tuple[str, ...] | None], Contrast]
    )

    def require_session(self) -> Session:
        """The open session.

        Raises:
            RuntimeError: If `open` was not called yet.
        """
        if self.session is None:
            msg = "call `open` first"
            raise RuntimeError(msg)
        return self.session

    def tasks(self, names: list[str] | None) -> list[Task] | None:
        """The session's tasks named `names`, or None (all tasks) without names."""
        return None if names is None else [self.require_session().task(name) for name in names]

    def budget(self, max_states: int | None, max_seconds: float | None) -> Budget | None:
        """A per-search budget from the given limits, the server's options filling the missing one; None without."""
        if max_states is None and max_seconds is None:
            return None
        return Budget(max_states or self.options.max_states, max_seconds or self.options.max_seconds)


_server = Server()
server = MCPServer(
    "runir-explore",
    instructions=(
        "Explore how the module program behaves on the workspace tasks, centred on its Choose rules, and test drafts "
        "with `set_program` and `validate`. Start with `chooses`, then `points` and `alternatives` for one rule, "
        "`contrast` to see which declared features order successful above failed bindings, `separation` to test a new "
        "feature, and `try_load` to check whether a Choose can become a Load. Call `open` first."
    ),
)


def _libc() -> ctypes.CDLL | None:
    try:
        return ctypes.CDLL("libc.so.6")
    except OSError:
        return None


_LIBC = _libc()


def _tune_malloc() -> None:
    """Set glibc to a fixed mmap threshold and two arenas, so that freed searches give back address space.

    `--max-memory-mib` limits the address space; with the defaults it grows with every large search and never shrinks.
    """
    if _LIBC is not None:
        _LIBC.mallopt(-3, 128 * 1024)  # M_MMAP_THRESHOLD
        _LIBC.mallopt(-8, 2)  # M_ARENA_MAX


def _release() -> None:
    """Drop every cached search and return the freed memory to the system."""
    if _server.session is not None:
        _server.session.release()
    if _LIBC is not None:
        _LIBC.malloc_trim(0)


def logged(function: Callable[_Params, str]) -> Callable[_Params, str]:
    """Report errors to the agent instead of failing the tool call, and log every call to stderr."""

    @functools.wraps(function)
    def wrapper(*args: _Params.args, **kwargs: _Params.kwargs) -> str:
        started = time.monotonic()
        try:
            return function(*args, **kwargs)
        except MemoryError:
            # Cached searches of earlier calls may fill the limit; retry once without them.
            _release()
            try:
                return function(*args, **kwargs)
            except MemoryError:
                _release()
            return (
                f"error: out of memory (limit {_server.options.max_memory_mib} MiB); cached searches were dropped. "
                "Pass fewer tasks in `tasks`, or a smaller `max_states` to tools that take one; searches over all "
                "tasks keep every state of each task in memory."
            )
        except Exception as error:
            logger.exception("%s failed", function.__name__)
            return f"error: {error}"
        finally:
            if resident_mib() > _server.options.release_mib:
                _release()
            spent = _server.session.spent_seconds if _server.session else 0.0
            logger.info("%s %s %.2fs search_total=%.1fs", function.__name__, kwargs, time.monotonic() - started, spent)

    return wrapper


@server.tool(name="open", structured_output=False)
@logged
def open_workspace(workspace: str = ".", program: str | None = None) -> str:
    """Open the workspace: its domain, current module program and tasks. Earlier results are discarded.

    `program` (full `(:program ...)` text) replaces the workspace's program, e.g. when that one does not parse.
    """
    options = _server.options
    root = Path(workspace).resolve()
    tasks = sorted(root.glob(options.tasks))
    text = (root / options.program).read_text(encoding="utf-8") if program is None else program
    try:
        session = Session(
            root / options.domain,
            text,
            tasks,
            options=SessionOptions(
                budget=Budget(options.max_states, options.max_seconds),
                total_seconds=options.total_seconds,
                workers=options.workers,
                release_mib=options.release_mib,
            ),
        )
    except OSError:
        raise
    except Exception as error:
        if program is not None:
            raise
        msg = f"{options.program} does not parse; pass a program that does as `program`.\n{error}"
        raise ValueError(msg) from error
    _server.session = session
    _server.contrasts.clear()
    rules = ", ".join(rule.name for rule in _server.session.choose_rules()) or "(none)"
    return (
        f"{len(tasks)} tasks"
        + (f", e.g. {', '.join(task.stem for task in tasks[:3])}" if tasks else "")
        + f"\nChoose rules: {rules}\n"
        f"per-search budget: {options.max_states} states, {options.max_seconds} s; "
        f"session search budget {options.total_seconds} s"
    )


@server.tool(structured_output=False)
@logged
def set_program(program: str) -> str:
    """Explore another module program (full `(:program ...)` text), e.g. a draft. Earlier results are discarded."""
    session = _server.require_session()
    session.set_program(program)
    _server.contrasts.clear()
    terminating = (
        "structurally terminating"
        if session.termination().terminating
        else "NOT structurally terminating (see `termination`)"
    )
    return f"Choose rules: {', '.join(rule.name for rule in session.choose_rules()) or '(none)'}; {terminating}"


@server.tool(structured_output=False)
@logged
def termination() -> str:
    """Structural termination of the program.

    For each module that does not terminate, the rules left on cycles and one cycle of its abstract policy graph
    (memory states with true Booleans and positive numericals).
    """
    return _server.require_session().termination().describe()


@server.tool(structured_output=False)
@logged
def validate(tasks: list[str] | None = None) -> str:
    """Execute and prove the program on the tasks (default: all), as the learning loop validates a proposal.

    Reports how many pass, the choice depth summed over passed tasks, every task that does not pass, and for a draft
    the choice depth of the workspace program on the tasks both pass, with the largest increases.
    """
    return _server.require_session().validate(_server.tasks(tasks)).describe()


@server.tool(structured_output=False)
@logged
def failures(
    tasks: list[str] | None = None,
    *,
    prove: bool = False,
    locate: bool = False,
    max_states: int | None = None,
    max_seconds: float | None = None,
) -> str:
    """Every task (default: all) whose execution, or with `prove` universal proof, does not succeed, in one call.

    Reports the category, failure vertex, memory, registers, and how many applicable actions there no rule admits.
    With `locate`, also where the failure path makes the task unsolvable, or that the program stops in a solvable
    state.
    """
    budget = _server.budget(max_states, max_seconds) or Budget(_server.options.max_states, _server.options.max_seconds)
    found = _server.require_session().failures(_server.tasks(tasks), prove=prove, locate=budget if locate else None)
    return "\n".join(failure.describe() for failure in found) or (
        "all proofs succeed" if prove else "all executions succeed"
    )


@server.tool(structured_output=False)
@logged
def run(task: str, *, prove: bool = False) -> str:
    """Status, failure category, search statistics, failure vertex and path length of the program on one task.

    Of its execution, or with `prove` of its universal proof (vertices `<task>:pv<n>`).
    """
    session = _server.require_session()
    return session.run(session.task(task), prove=prove).describe()


@server.tool(structured_output=False)
@logged
def chooses(tasks: list[str] | None = None) -> str:
    """One row per Choose rule over the choice points on each task's path to the goal or the failure."""
    session = _server.require_session()
    runs = session.runs(_server.tasks(tasks))
    status = ", ".join(f"{run.task.name}:{run.status}" for run in runs if run.status != "success") or "all succeed"
    return "\n".join(
        [*(row.describe() for row in session.chooses(_server.tasks(tasks))), f"tasks not succeeding: {status}"]
    )


@server.tool(structured_output=False)
@logged
def points(rule: str, tasks: list[str] | None = None, limit: int = 20) -> str:
    """Choice points of one Choose rule on paths, most informative first (with failed bindings, then wider)."""
    session = _server.require_session()
    found = session.points(session.rule(rule), _server.tasks(tasks), limit)
    return "\n".join(point.describe() for point in found) or "(none)"


@server.tool(structured_output=False)
@logged
def alternatives(point: str, max_states: int | None = None, max_seconds: float | None = None) -> str:
    """Every binding at a choice point (`<task>:v<n>`) in `:order` rank with its outcome and feature deltas.

    Untried bindings are searched from their target only when `max_states` or `max_seconds` is given.
    """
    session = _server.require_session()
    rows = session.alternatives(session.vertex_ref(point), _server.budget(max_states, max_seconds))
    return "\n".join(row.describe() for row in rows)


@server.tool(structured_output=False)
@logged
def contrast(
    rule: str,
    tasks: list[str] | None = None,
    max_points: int = 50,
    max_states: int | None = None,
    max_seconds: float | None = None,
) -> str:
    """Pairs of successful and failed bindings of one rule at the same points, and how declared features order them.

    `max`: higher on the successful binding. Untried bindings count when a budget is given.
    """
    session = _server.require_session()
    result = session.contrast(
        session.rule(rule), _server.tasks(tasks), _server.budget(max_states, max_seconds), max_points
    )
    _server.contrasts[(rule, tuple(tasks) if tasks else None)] = result
    return result.describe()


@server.tool(structured_output=False)
@logged
def separation(feature: str, rule: str, tasks: list[str] | None = None) -> str:
    """How a declared feature or a new expression orders the pairs of the last `contrast` for this rule and tasks.

    Expressions are written c_..., b_..., n_..., registers by name. Concepts count as membership of the binding at
    the Choose source.
    """
    session = _server.require_session()
    key = (rule, tuple(tasks) if tasks else None)
    if key not in _server.contrasts:
        _server.contrasts[key] = session.contrast(session.rule(rule), _server.tasks(tasks))
    return session.separation(session.feature(feature), _server.contrasts[key]).describe()


@server.tool(structured_output=False)
@logged
def try_load(rule: str, tasks: list[str] | None = None) -> str:
    """Execute and prove the program with this rule's Choose replaced by a Load (first binding, no backtracking)."""
    session = _server.require_session()
    return session.try_load(session.rule(rule), _server.tasks(tasks)).describe()


@server.tool(structured_output=False)
@logged
def vertex(vertex: str) -> str:
    """Memory state, registers, declared Boolean and numerical feature values, and fluent atoms of `<task>:v<n>`."""
    session = _server.require_session()
    return session.vertex(session.vertex_ref(vertex)).describe()


@server.tool(structured_output=False)
@logged
def successors(vertex: str) -> str:
    """Every applicable action at `<task>:v<n>`, the rule that admits it there (or none), and its feature deltas.

    At a failure vertex this shows which moves the conditions and effects forbid.
    """
    session = _server.require_session()
    return (
        "\n".join(step.describe() for step in session.successors(session.vertex_ref(vertex)))
        or "(no applicable action)"
    )


@server.tool(structured_output=False)
@logged
def dead_end(task: str, max_states: int | None = None, max_seconds: float | None = None, *, prove: bool = False) -> str:
    """For a task whose execution (or with `prove`, universal proof) fails: where the task becomes unsolvable.

    The step on the failure path after which the task is unsolvable (found by binary search with plan searches), or
    that every state on it has a plan.
    """
    session = _server.require_session()
    return session.dead_end(session.task(task), _server.budget(max_states, max_seconds), prove=prove).describe()


@server.tool(structured_output=False)
@logged
def plan(vertex: str, max_states: int | None = None, max_seconds: float | None = None) -> str:
    """A plan from the state of `<task>:v<n>` (e.g. a failure vertex) to the goal.

    Found by lifted greedy best-first search with FF; it need not follow the program. Each step shows its feature
    deltas under the vertex's registers.
    """
    session = _server.require_session()
    return session.plan(session.vertex_ref(vertex), _server.budget(max_states, max_seconds)).describe()


def main(argv: list[str] | None = None) -> None:
    """Parse the server options from `argv`, limit the address space, and serve the tools over stdio."""
    parser = argparse.ArgumentParser(description="MCP server over pyrunir.kr.ps.ext.explore.")
    parser.add_argument("--domain", default="domain.pddl")
    parser.add_argument("--program", default="module-program.txt")
    parser.add_argument("--tasks", default="problems/*.pddl")
    parser.add_argument("--max-states", type=int, default=1_000_000)
    parser.add_argument("--max-seconds", type=float, default=30.0)
    parser.add_argument("--total-seconds", type=float, default=1800.0)
    parser.add_argument("--workers", type=int, default=8)
    parser.add_argument("--max-memory-mib", type=int, default=8192, help="address-space limit; searches beyond it fail")
    parser.add_argument("--release-mib", type=int, default=4096, help="drop cached searches above this resident size")
    _server.options = parser.parse_args(argv)
    handler = logging.StreamHandler(sys.stderr)
    handler.setFormatter(logging.Formatter("[runir-explore] %(message)s"))
    logger.addHandler(handler)
    logger.setLevel(logging.INFO)
    logger.propagate = False
    _tune_malloc()
    limit = _server.options.max_memory_mib * 2**20
    resource.setrlimit(resource.RLIMIT_AS, (limit, limit))
    server.run("stdio")


if __name__ == "__main__":
    main()
