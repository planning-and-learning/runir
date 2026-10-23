"""Explore how an Ext module program behaves on lifted tasks, centred on its Choose rules.

A `Session` holds a domain, a program and named tasks. Searches run lazily per task with
`StateMemorization.ALL`, so the full explored graph is available: which bindings a Choose
tried, which failed, and how large the failed subtrees were. Untried bindings can be
evaluated with a budgeted search that continues from the binding's target configuration.

Names (rules, tasks, feature expressions) are accepted only at the boundary (`Session.rule`,
`Session.task`, `Session.feature`); every other call takes and returns handles.

Feature values follow the program's semantics: Boolean and numerical expressions are evaluated
in the configuration after binding, like `:order`; a concept expression is evaluated at the
Choose source, where candidates are computed, and its value for a binding is membership.
"""

from __future__ import annotations

import re
import time
from collections import Counter, deque
from collections.abc import Iterable, Mapping, Sequence
from concurrent.futures import ThreadPoolExecutor
from dataclasses import dataclass, field
from datetime import timedelta
from pathlib import Path
from typing import Literal

from pypddl.formalism import ParserOptions
from pytyr.formalism.planning import Parser
from pytyr.planning import lifted
from pyyggdrasil.execution import ExecutionContext

from pyrunir.datasets import LiftedTaskSearchContext
from pyrunir.kr import DomainContext, LiftedTaskContext
from pyrunir.kr.dl.semantics import EvaluationStorage, LiftedStateEvaluationContext
from pyrunir.kr.ps.ext import (
    ConceptChooseRule,
    LiftedProgramProofResults,
    LiftedProgramSearchOptions,
    LiftedSuccessorExpander,
    ProgramProofStatus,
    ProgramSearchStatistics,
    RoleChooseRule,
    StateMemorization,
    evaluate,
    find_lifted_solution,
)
from pyrunir.kr.ps.ext.dl import parse_boolean, parse_concept, parse_numerical, parse_program, structural_termination

__all__ = [
    "Alternative",
    "BindingRef",
    "Budget",
    "BudgetExhausted",
    "ChoicePoint",
    "ChooseRule",
    "ChooseSummary",
    "Contrast",
    "Feature",
    "LoadTrial",
    "Separation",
    "Session",
    "Task",
    "TaskRun",
    "TaskValidation",
    "Validation",
    "Vertex",
    "VertexRef",
]

Outcome = Literal["succeeded", "failed", "untried", "succeeds", "fails", "unknown"]
"""`succeeded`/`failed`/`untried`: what the session's search did with a binding.
`succeeds`/`fails`/`unknown`: the result of a budgeted search from an untried binding."""


class BudgetExhausted(RuntimeError):
    """The session's search budget is used up."""


@dataclass(frozen=True, slots=True)
class Budget:
    """Limits of one search."""

    max_states: int = 100_000
    max_seconds: float = 10.0


# ---------------------------------------------------------------------------------------------------------------------
# Handles


@dataclass(frozen=True, slots=True)
class Task:
    name: str

    def __str__(self) -> str:
        return self.name


@dataclass(frozen=True, slots=True)
class ChooseRule:
    name: str
    register: str

    def __str__(self) -> str:
        return self.name


@dataclass(frozen=True, slots=True)
class Feature:
    expression: str
    category: Literal["concept", "boolean", "numerical"]
    name: str | None = None

    def __str__(self) -> str:
        return self.name or self.expression


@dataclass(frozen=True, slots=True)
class VertexRef:
    task: Task
    vertex: int

    def __str__(self) -> str:
        return f"{self.task}:v{self.vertex}"


@dataclass(frozen=True, slots=True)
class BindingRef:
    point: VertexRef
    binding: str

    def __str__(self) -> str:
        return f"{self.point}:{self.binding}"


# ---------------------------------------------------------------------------------------------------------------------
# Results


@dataclass(frozen=True, slots=True)
class TaskRun:
    task: Task
    status: str
    category: str | None
    num_expanded: int
    num_generated: int
    num_backtracks: int
    choice_depth: int
    choice_width: int
    seconds: float
    failure: VertexRef | None
    path: tuple[VertexRef, ...]
    """From the initial vertex to the first goal, or to the failure."""

    def __str__(self) -> str:
        head = f"{self.task}: {self.status}" + (f" ({self.category})" if self.category else "")
        stats = (
            f"expanded={self.num_expanded} backtracks={self.num_backtracks} "
            f"choice_depth={self.choice_depth} choice_width={self.choice_width} seconds={self.seconds:.2f}"
        )
        tail = f"failure={self.failure} " if self.failure else ""
        return f"{head}; {stats}; {tail}path={len(self.path)} vertices"


@dataclass(frozen=True, slots=True)
class TaskValidation:
    task: Task
    execute: str
    """`passed`, `failed (<category>)` or `unavailable (<status>)`."""
    prove: str
    choice_depth: int | None
    """Choice depth of a passed execution."""

    @property
    def passed(self) -> bool:
        return self.execute == "passed" and self.prove == "passed"

    def __str__(self) -> str:
        depth = f" choice_depth={self.choice_depth}" if self.choice_depth is not None else ""
        return f"{self.task}: execute {self.execute}, prove {self.prove}{depth}"


@dataclass(frozen=True, slots=True)
class Validation:
    tasks: tuple[TaskValidation, ...]

    def __str__(self) -> str:
        passed = [task for task in self.tasks if task.passed]
        depth = sum(task.choice_depth or 0 for task in passed)
        lines = [f"passed {len(passed)}/{len(self.tasks)}; choice_depth sum over passed tasks {depth}"]
        lines += [f"  {task}" for task in self.tasks if not task.passed]
        return "\n".join(lines)


@dataclass(frozen=True, slots=True)
class ChooseSummary:
    rule: ChooseRule
    occurrences: int
    """Choose steps with at least two bindings on first-goal or failure paths."""
    tasks: int
    max_width: int
    failed_bindings: int
    points_with_failure: int
    untried_bindings: int

    def __str__(self) -> str:
        return (
            f"{self.rule}: occurrences={self.occurrences} tasks={self.tasks} max_width={self.max_width} "
            f"failed_bindings={self.failed_bindings} points_with_failure={self.points_with_failure} "
            f"untried_bindings={self.untried_bindings}"
        )


@dataclass(frozen=True, slots=True)
class ChoicePoint:
    vertex: VertexRef
    rule: ChooseRule
    width: int
    chosen: int | None
    """1-based rank of the binding on the path, if the point lies on a path to the goal."""
    failed: int
    untried: int

    def __str__(self) -> str:
        chosen = f"{self.chosen}/{self.width}" if self.chosen else f"-/{self.width}"
        return f"{self.vertex} {self.rule} chosen={chosen} failed={self.failed} untried={self.untried}"


@dataclass(frozen=True, slots=True)
class Alternative:
    binding: BindingRef
    rank: int
    outcome: Outcome
    ending: str | None
    """For failures: `open_state`, `empty_choose`, or `subtree` when every continuation failed."""
    states: int | None
    deltas: Mapping[str, tuple[object, object]]
    """Feature: (value at the reference binding, value here); the reference is the successful binding, else the first."""

    def __str__(self) -> str:
        deltas = " ".join(f"{name}:{a}>{b}" for name, (a, b) in self.deltas.items()) or "-"
        ending = f" {self.ending}" if self.ending else ""
        states = f" states={self.states}" if self.states is not None else ""
        return f"{self.binding.binding} rank={self.rank} {self.outcome}{ending}{states} deltas: {deltas}"


@dataclass(frozen=True, slots=True)
class Separation:
    """How `feature` compares a successful binding with a failed one at the same choice point.

    Booleans count `True` as higher. A feature that is higher (or lower) on every pair would decide
    the Choose as an `:order` or a condition; one that is equal on every pair cannot."""

    feature: Feature
    pairs: int
    higher: int
    """Pairs where the successful binding has the higher value."""
    lower: int
    equal: int

    @property
    def score(self) -> float:
        """Share of pairs on the majority direction minus the minority: 1 consistent and total, 0 no signal."""
        return abs(self.higher - self.lower) / self.pairs if self.pairs else 0.0

    @property
    def consistent(self) -> bool:
        return self.pairs > 0 and self.equal == 0 and min(self.higher, self.lower) == 0

    def __str__(self) -> str:
        direction = "max" if self.higher >= self.lower else "min"
        return (
            f"{self.feature}: score={self.score:.2f} ({direction}){' consistent' if self.consistent else ''} "
            f"higher={self.higher} lower={self.lower} equal={self.equal} of {self.pairs} pairs"
        )


@dataclass(frozen=True, slots=True)
class Contrast:
    rule: ChooseRule
    pairs: tuple[tuple[BindingRef, BindingRef], ...]
    """(successful, failed) bindings at the same choice point."""
    decided: int
    """Points with a failed binding; the others had only successful or unknown bindings."""
    undecided: int
    ranking: tuple[Separation, ...]

    def __str__(self) -> str:
        informative = [separation for separation in self.ranking if separation.score > 0]
        lines = [
            f"{self.rule}: {len(self.pairs)} (succeeded, failed) pairs from {self.decided} points with a failed binding; "
            f"{self.undecided} points without one",
            *(f"  {separation}" for separation in informative[:10]),
            f"  ({len(self.ranking) - len(informative[:10])} other declared features separate less or not at all)",
        ]
        return "\n".join(lines)


@dataclass(frozen=True, slots=True)
class LoadTrial:
    rule: ChooseRule
    terminating: bool
    passed: tuple[Task, ...]
    failed: tuple[tuple[Task, str, str], ...]
    """(task, stage, category)."""
    unavailable: tuple[tuple[Task, str], ...]
    choice_depth: tuple[int, int]
    """Sum over the passed tasks: with the Choose, with the Load."""

    def __str__(self) -> str:
        if not self.terminating:
            return f"{self.rule} as Load: the rewritten program is not structurally terminating"
        failed = ", ".join(f"{task} {stage} {category}" for task, stage, category in self.failed) or "-"
        unavailable = ", ".join(f"{task} {stage}" for task, stage in self.unavailable) or "-"
        return (
            f"{self.rule} as Load: passed={len(self.passed)} failed={len(self.failed)} "
            f"unavailable={len(self.unavailable)} choice_depth {self.choice_depth[0]}->{self.choice_depth[1]}\n"
            f"  failed: {failed}\n  unavailable: {unavailable}"
        )


@dataclass(frozen=True, slots=True)
class Vertex:
    ref: VertexRef
    memory: str
    registers: Mapping[str, str | None]
    atoms: tuple[str, ...]
    features: Mapping[str, object]
    goal: bool

    def __str__(self) -> str:
        registers = " ".join(f"{name}={value}" for name, value in self.registers.items())
        features = " ".join(f"{name}={value}" for name, value in self.features.items())
        return (
            f"{self.ref} memory={self.memory}{' goal' if self.goal else ''}\n  registers: {registers}\n"
            f"  features: {features}\n  atoms: {' '.join(self.atoms)}"
        )


# ---------------------------------------------------------------------------------------------------------------------
# Internals


@dataclass(slots=True)
class _Search:
    """One search and the solved/failed/unknown status of its vertices."""

    result: LiftedProgramProofResults
    seconds: float
    status: dict[int, bool | None] = field(default_factory=dict)

    @property
    def graph(self):
        return self.result.graph


def _is_choose(rule) -> bool:
    return rule is not None and isinstance(rule.get_variant(), (ConceptChooseRule, RoleChooseRule))


def _category(result: LiftedProgramProofResults) -> str | None:
    """Why a search did not succeed; None if it did. Open and dead-end states also occur in failed branches of a success."""
    if result.status == ProgramProofStatus.SUCCESS:
        return None
    if result.open_states:
        return "open_state"
    if result.deadend_states:
        return "empty_choose"
    return result.status.name.lower()


def _vertex_status(result: LiftedProgramProofResults) -> dict[int, bool | None]:
    """True: a goal is reached from the vertex; False: every continuation fails; None: unknown (not fully explored).

    A vertex with Choose out-edges succeeds if one tried binding does; other vertices need all their successors."""
    graph = result.graph
    failed = set(result.open_states) | set(result.deadend_states)
    status: dict[int, bool | None] = {}
    order: list[int] = []
    seen: set[int] = set()
    for root in graph.get_vertex_indices():
        if root in seen:
            continue
        stack = [(root, iter(graph.get_successor_indices(root)))]
        seen.add(root)
        while stack:
            vertex, successors = stack[-1]
            child = next(successors, None)
            if child is None:
                order.append(vertex)
                stack.pop()
            elif child not in seen:
                seen.add(child)
                stack.append((child, iter(graph.get_successor_indices(child))))
    for vertex in order:
        label = graph.get_vertex_property(vertex)
        edges = list(graph.get_out_edge_indices(vertex))
        if label.is_goal:
            status[vertex] = True
        elif vertex in failed or label.is_unsolvable:
            status[vertex] = False
        elif not edges:
            status[vertex] = None
        else:
            values = [status.get(graph.get_target(edge)) for edge in edges]
            if any(_is_choose(graph.get_edge_property(edge).rule) for edge in edges):
                status[vertex] = True if any(v is True for v in values) else (False if all(v is False for v in values) else None)
            else:
                status[vertex] = False if any(v is False for v in values) else (True if all(v is True for v in values) else None)
    return status


def _path(graph, start: int, targets: set[int]) -> list[int]:
    """A shortest path from `start` to one of `targets`, or [] if none."""
    parents: dict[int, int | None] = {start: None}
    queue = deque([start])
    while queue:
        vertex = queue.popleft()
        if vertex in targets:
            path = [vertex]
            while (parent := parents[path[-1]]) is not None:
                path.append(parent)
            return path[::-1]
        for child in graph.get_successor_indices(vertex):
            if child not in parents:
                parents[child] = vertex
                queue.append(child)
    return []


def _reachable(graph, start: int) -> int:
    seen = {start}
    stack = [start]
    while stack:
        for child in graph.get_successor_indices(stack.pop()):
            if child not in seen:
                seen.add(child)
                stack.append(child)
    return len(seen)


def _object_name(value) -> str | None:
    if value is None:
        return None
    if isinstance(value, tuple):
        return f"({' '.join(str(_object_name(item)) for item in value)})"
    return str(value.get_name())


def _register_value(module_state, rule) -> str | None:
    variant = rule.get_variant()
    identifier = int(variant.get_register().get_identifier().value())
    registers = module_state.registers
    values = registers.concept_values if isinstance(variant, ConceptChooseRule) else registers.role_values
    return _object_name(values[identifier])


_RULE_BLOCK = re.compile(r"\(:rule\s+\(:symbol\s+([^\s()]+)\)")


def _rewrite_choose_as_load(text: str, rule: str) -> str:
    """Replace every Choose body of `rule` by a Load with the same conditions, candidates, register and effects."""
    for match in _RULE_BLOCK.finditer(text):
        if match.group(1) != rule:
            continue
        depth, end = 0, match.start()
        for end in range(match.start(), len(text)):
            depth += {"(": 1, ")": -1}.get(text[end], 0)
            if depth == 0:
                break
        block = text[match.start() : end + 1]
        block = re.sub(r"\(:order(?:\s+\((?:min|max)\s+[^\s()]+\))*\s*\)", "", block)
        block = block.replace("(:choose", "(:load")
        return text[: match.start()] + block + text[end + 1 :]
    raise KeyError(rule)


# ---------------------------------------------------------------------------------------------------------------------
# Session


class Session:
    """Lazy searches of one program on named tasks of one domain.

    `budget` limits every search the session starts; `total_seconds` limits their sum."""

    def __init__(
        self,
        domain: Path,
        program: str,
        tasks: Mapping[str, Path] | Sequence[Path],
        /,
        *,
        budget: Budget = Budget(),
        total_seconds: float = 1800.0,
        workers: int = 8,
    ) -> None:
        self._parser = Parser(Path(domain), ParserOptions())
        self._domain = DomainContext(self._parser.get_domain())
        self._task_paths = dict(tasks) if isinstance(tasks, Mapping) else {Path(path).stem: Path(path) for path in tasks}
        self._task_contexts: dict[str, LiftedTaskContext] = {}
        self._budget = budget
        self._total_seconds = total_seconds
        self._spent = 0.0
        self._workers = workers
        self.set_program(program)

    # -- boundary -----------------------------------------------------------------------------------------------------

    @property
    def tasks(self) -> tuple[Task, ...]:
        return tuple(Task(name) for name in self._task_paths)

    @property
    def spent_seconds(self) -> float:
        return self._spent

    def set_program(self, text: str, /) -> None:
        """Explore `text` from now on; earlier searches are discarded."""
        program = parse_program(text, self._parser.get_domain(), self._domain.ext_repository)
        self._text = text
        self._program = program
        self._module = program.get_entry_module()
        self._searches: dict[str, _Search] = {}
        self._proofs: dict[str, LiftedProgramProofResults] = {}
        self._rules = {}
        for bodies in self._module.get_memory_transitions():
            for body in bodies:
                if _is_choose(body):
                    self._rules[str(body.get_symbol())] = ChooseRule(
                        str(body.get_symbol()), str(body.get_variant().get_register().get_name())
                    )

    @property
    def program(self) -> str:
        return self._text

    def task(self, name: str, /) -> Task:
        if name not in self._task_paths:
            raise KeyError(f"unknown task {name!r}; tasks: {', '.join(sorted(self._task_paths))}")
        return Task(name)

    def rule(self, name: str, /) -> ChooseRule:
        if name not in self._rules:
            raise KeyError(f"no Choose rule {name!r}; Choose rules: {', '.join(sorted(self._rules)) or '(none)'}")
        return self._rules[name]

    def choose_rules(self) -> tuple[ChooseRule, ...]:
        return tuple(self._rules.values())

    def feature(self, expression: str, /) -> Feature:
        """A declared feature name or a concept, Boolean or numerical expression in the scope of the entry module."""
        declared = self._declared_features()
        if expression in declared:
            return declared[expression]
        self._parse(Feature(expression, self._expression_category(expression)))
        return Feature(expression, self._expression_category(expression))

    def vertex_ref(self, text: str, /) -> VertexRef:
        """`<task>:v<n>`."""
        task, _, vertex = text.rpartition(":")
        return VertexRef(self.task(task), int(vertex.removeprefix("v")))

    # -- searches -----------------------------------------------------------------------------------------------------

    def run(self, task: Task, /) -> TaskRun:
        search = self._search(task)
        result = search.result
        graph = result.graph
        initial = next((v for v in graph.get_vertex_indices() if graph.get_vertex_property(v).is_initial), None)
        failure = None if result.status == ProgramProofStatus.SUCCESS else next(iter((*result.open_states, *result.deadend_states)), None)
        goals = {v for v in graph.get_vertex_indices() if graph.get_vertex_property(v).is_goal}
        target = goals if result.status == ProgramProofStatus.SUCCESS else ({failure} if failure is not None else set())
        path = _path(graph, initial, target) if initial is not None and target else []
        statistics: ProgramSearchStatistics = result.statistics
        return TaskRun(
            task=task,
            status=result.status.name.lower(),
            category=_category(result),
            num_expanded=statistics.num_expanded,
            num_generated=statistics.num_generated,
            num_backtracks=statistics.num_backtracks,
            choice_depth=statistics.choice_depth,
            choice_width=statistics.choice_width,
            seconds=search.seconds,
            failure=VertexRef(task, failure) if failure is not None else None,
            path=tuple(VertexRef(task, v) for v in path),
        )

    def validate(self, tasks: Iterable[Task] | None = None, /) -> Validation:
        """Execute and prove the program on `tasks` (default: all), like the learning loop's validation."""
        tasks = list(tasks) if tasks is not None else list(self.tasks)
        self._prefetch(tasks)

        def outcome(result) -> str:
            if result.status == ProgramProofStatus.SUCCESS:
                return "passed"
            if result.status == ProgramProofStatus.FAILURE:
                return f"failed ({_category(result)})"
            return f"unavailable ({result.status.name.lower()})"

        def prove(task: Task):
            if task.name not in self._proofs:
                self._proofs[task.name] = self._find(task, self._budget, universal=True)[0]
            return self._proofs[task.name]

        with ThreadPoolExecutor(self._workers) as pool:
            proofs = list(pool.map(prove, tasks))
        rows = []
        for task, proof in zip(tasks, proofs):
            execute = self._search(task).result
            rows.append(TaskValidation(
                task, outcome(execute), outcome(proof),
                execute.statistics.choice_depth if execute.status == ProgramProofStatus.SUCCESS else None,
            ))
        return Validation(tuple(rows))

    def runs(self, tasks: Iterable[Task] | None = None, /) -> list[TaskRun]:
        tasks = list(tasks) if tasks is not None else list(self.tasks)
        self._prefetch(tasks)
        return [self.run(task) for task in tasks]

    # -- choice points -------------------------------------------------------------------------------------------------

    def chooses(self, tasks: Iterable[Task] | None = None, /) -> list[ChooseSummary]:
        """One row per Choose rule over the choice points on each task's path (goal or failure)."""
        tasks = list(tasks) if tasks is not None else list(self.tasks)
        self._prefetch(tasks)
        rows: dict[str, list[ChoicePoint]] = {name: [] for name in self._rules}
        for task in tasks:
            for point in self._points_on_path(task):
                rows[point.rule.name].append(point)
        return [
            ChooseSummary(
                rule=self._rules[name],
                occurrences=sum(point.width > 1 for point in points),
                tasks=len({point.vertex.task for point in points}),
                max_width=max((point.width for point in points), default=0),
                failed_bindings=sum(point.failed for point in points),
                points_with_failure=sum(point.failed > 0 for point in points),
                untried_bindings=sum(point.untried for point in points),
            )
            for name, points in rows.items()
        ]

    def points(self, rule: ChooseRule, /, tasks: Iterable[Task] | None = None, limit: int = 20) -> list[ChoicePoint]:
        """Choice points of `rule` on paths, most informative first: with failures, then with more bindings."""
        tasks = list(tasks) if tasks is not None else list(self.tasks)
        self._prefetch(tasks)
        points = [point for task in tasks for point in self._points_on_path(task) if point.rule == rule and point.width > 1]
        points.sort(key=lambda point: (-point.failed, -point.width, point.vertex.task.name, point.vertex.vertex))
        return points[:limit]

    def alternatives(self, point: ChoicePoint | VertexRef, /, budget: Budget | None = None) -> list[Alternative]:
        """Every binding of the Choose at the point, in `:order` rank, with its outcome.

        Tried bindings come from the session's search. Untried ones are searched from their target
        configuration only when `budget` is given; otherwise their outcome is `untried`."""
        ref = point.vertex if isinstance(point, ChoicePoint) else point
        search = self._search(ref.task)
        graph = search.graph
        rule, bindings = self._bindings(ref)
        tried = {
            _register_value(graph.get_vertex_property(graph.get_target(edge)).program_state.module_state, rule): edge
            for edge in graph.get_out_edge_indices(ref.vertex)
            if _is_choose(graph.get_edge_property(edge).rule)
        }
        rows: list[tuple[str, int, Outcome, str | None, int | None, object]] = []
        for rank, (name, target) in enumerate(bindings, 1):
            if name in tried:
                child = graph.get_target(tried[name])
                outcome = {True: "succeeded", False: "failed", None: "unknown"}[search.status.get(child)]
                ending = self._ending(search, child) if outcome == "failed" else None
                rows.append((name, rank, outcome, ending, _reachable(graph, child), target))
            elif budget is not None:
                result, _ = self._find(ref.task, budget, start=target)
                outcome = {ProgramProofStatus.SUCCESS: "succeeds", ProgramProofStatus.FAILURE: "fails"}.get(result.status, "unknown")
                rows.append((name, rank, outcome, _category(result) if outcome == "fails" else None, result.statistics.num_expanded, target))
            else:
                rows.append((name, rank, "untried", None, None, target))
        task_context = self._task_context(ref.task)
        source = graph.get_vertex_property(ref.vertex).program_state
        values = [self._feature_values(task_context, source, target, rule, name) for name, *_, target in rows]
        reference = next((i for i, row in enumerate(rows) if row[2] in ("succeeded", "succeeds")), 0)
        return [
            Alternative(
                binding=BindingRef(ref, name),
                rank=rank,
                outcome=outcome,
                ending=ending,
                states=states,
                deltas={key: (values[reference][key], value) for key, value in values[i].items() if values[reference][key] != value},
            )
            for i, (name, rank, outcome, ending, states, _) in enumerate(rows)
        ]

    def contrast(
        self, rule: ChooseRule, /, tasks: Iterable[Task] | None = None, budget: Budget | None = None, max_points: int = 50
    ) -> Contrast:
        """Successful against failed bindings of `rule` at the same points, and how each declared feature orders them."""
        pairs: list[tuple[BindingRef, BindingRef]] = []
        decided = undecided = 0
        for point in self.points(rule, tasks, limit=max_points):
            alternatives = self.alternatives(point, budget)
            good = [a.binding for a in alternatives if a.outcome in ("succeeded", "succeeds")]
            bad = [a.binding for a in alternatives if a.outcome in ("failed", "fails")]
            if good and bad:
                decided += 1
                pairs.extend((g, b) for g in good for b in bad)
            else:
                undecided += 1
        ranking = sorted(
            (self.separation(feature, pairs) for feature in self._declared_features().values()),
            key=lambda separation: (-separation.score, -separation.pairs, str(separation.feature)),
        )
        return Contrast(rule, tuple(pairs), decided, undecided, tuple(ranking))

    def separation(self, feature: Feature, pairs: Contrast | Sequence[tuple[BindingRef, BindingRef]], /) -> Separation:
        """How `feature` compares the successful with the failed binding of each pair."""
        pairs = pairs.pairs if isinstance(pairs, Contrast) else pairs
        higher = lower = equal = 0
        for good, bad in pairs:
            a, b = self.value(feature, good), self.value(feature, bad)
            if a == b:
                equal += 1
            elif a > b:
                higher += 1
            else:
                lower += 1
        return Separation(feature, len(pairs), higher, lower, equal)

    def value(self, feature: Feature, binding: BindingRef, /) -> object:
        """`feature` for one binding: membership at the Choose source for concepts, else the value after binding."""
        ref = binding.point
        rule, bindings = self._bindings(ref)
        target = dict(bindings)[binding.binding]
        source = self._search(ref.task).graph.get_vertex_property(ref.vertex).program_state
        task_context = self._task_context(ref.task)
        return self._evaluate(task_context, feature, source if feature.category == "concept" else target, binding.binding)

    # -- ablation ------------------------------------------------------------------------------------------------------

    def try_load(self, rule: ChooseRule, /, tasks: Iterable[Task] | None = None) -> LoadTrial:
        """Execute and prove the program with `rule`'s Choose bodies replaced by Loads (first binding, no backtracking)."""
        tasks = list(tasks) if tasks is not None else list(self.tasks)
        text = _rewrite_choose_as_load(str(self._program), rule.name)
        program = parse_program(text, self._parser.get_domain(), self._domain.ext_repository)
        if not structural_termination(program).is_terminating():
            return LoadTrial(rule, False, (), (), (), (0, 0))
        self._prefetch(tasks)
        before = {task: self._search(task).result for task in tasks}

        def trial(task: Task):
            outcomes = []
            for universal in (False, True):
                result, _ = self._find(task, self._budget, program=program, universal=universal)
                outcomes.append(result)
            return task, outcomes

        passed, failed, unavailable = [], [], []
        depth_before = depth_after = 0
        with ThreadPoolExecutor(self._workers) as pool:
            for task, (execute, prove) in pool.map(trial, tasks):
                stages = (("execute", execute), ("prove", prove))
                bad = [(stage, r) for stage, r in stages if r.status == ProgramProofStatus.FAILURE]
                open_ = [(stage, r) for stage, r in stages if r.status not in (ProgramProofStatus.SUCCESS, ProgramProofStatus.FAILURE)]
                if bad:
                    failed.append((task, bad[0][0], _category(bad[0][1]) or "failure"))
                elif open_:
                    unavailable.append((task, open_[0][0]))
                else:
                    passed.append(task)
                    depth_before += before[task].statistics.choice_depth
                    depth_after += execute.statistics.choice_depth
        return LoadTrial(rule, True, tuple(passed), tuple(failed), tuple(unavailable), (depth_before, depth_after))

    # -- vertices ------------------------------------------------------------------------------------------------------

    def vertex(self, ref: VertexRef, /) -> Vertex:
        label = self._search(ref.task).graph.get_vertex_property(ref.vertex)
        program_state = label.program_state
        module_state = program_state.module_state
        module = module_state.module
        registers = {
            **{str(r.get_name()): _object_name(v) for r, v in zip(module.get_concept_registers(), module_state.registers.concept_values)},
            **{str(r.get_name()): _object_name(v) for r, v in zip(module.get_role_registers(), module_state.registers.role_values)},
        }
        task_context = self._task_context(ref.task)
        features = self._feature_values(task_context, program_state, program_state, None, None)
        return Vertex(
            ref=ref,
            memory=str(module_state.memory_state.get_name()),
            registers=registers,
            atoms=tuple(str(atom) for atom in label.state.fluent_atoms()),
            features=features,
            goal=label.is_goal,
        )

    # -- internals -----------------------------------------------------------------------------------------------------

    def _task_context(self, task: Task) -> LiftedTaskContext:
        if task.name not in self._task_contexts:
            planning_task = self._parser.parse_task(self._task_paths[task.name], ParserOptions())
            self._task_contexts[task.name] = LiftedTaskContext(
                self._domain, LiftedTaskSearchContext(lifted.Task(planning_task), ExecutionContext(1))
            )
        return self._task_contexts[task.name]

    def _find(self, task: Task, budget: Budget, *, program=None, universal: bool = False, start=None):
        if self._spent >= self._total_seconds:
            raise BudgetExhausted(f"the session's search budget of {self._total_seconds:.0f} s is used up")
        options = LiftedProgramSearchOptions()
        options.universal = universal
        options.state_memorization = StateMemorization.ALL
        options.max_num_states = budget.max_states
        options.max_time = timedelta(seconds=budget.max_seconds)
        options.start_state = start
        started = time.monotonic()
        result = find_lifted_solution(self._task_context(task), program or self._program, options)
        seconds = time.monotonic() - started
        self._spent += seconds
        return result, seconds

    def _search(self, task: Task) -> _Search:
        if task.name not in self._searches:
            result, seconds = self._find(task, self._budget)
            self._searches[task.name] = _Search(result, seconds, _vertex_status(result))
        return self._searches[task.name]

    def _prefetch(self, tasks: Sequence[Task]) -> None:
        missing = [task for task in tasks if task.name not in self._searches]
        for task in missing:
            self._task_context(task)
        with ThreadPoolExecutor(self._workers) as pool:
            for task, (result, seconds) in zip(missing, pool.map(lambda t: self._find(t, self._budget), missing)):
                self._searches[task.name] = _Search(result, seconds, _vertex_status(result))

    def _points_on_path(self, task: Task) -> list[ChoicePoint]:
        search = self._search(task)
        graph = search.graph
        run = self.run(task)
        on_path = [ref.vertex for ref in run.path]
        next_on_path = dict(zip(on_path, on_path[1:]))
        points = []
        for vertex in on_path:
            edges = [edge for edge in graph.get_out_edge_indices(vertex) if _is_choose(graph.get_edge_property(edge).rule)]
            if not edges:
                continue
            ref = VertexRef(task, vertex)
            rule, bindings = self._bindings(ref)
            children = [graph.get_target(edge) for edge in edges]
            successor = next_on_path.get(vertex)
            chosen = next((rank for rank, edge in enumerate(edges, 1) if graph.get_target(edge) == successor), None)
            points.append(
                ChoicePoint(
                    vertex=ref,
                    rule=self._rules[str(rule.get_symbol())],
                    width=len(bindings),
                    chosen=chosen if search.status.get(successor) is True else None,
                    failed=sum(search.status.get(child) is False for child in children),
                    untried=len(bindings) - len(edges),
                )
            )
        return points

    def _bindings(self, ref: VertexRef):
        """The Choose rule at `ref` and its bindings in `:order` rank as (object name, target program state)."""
        cache = getattr(self, "_binding_cache", None)
        if cache is None:
            cache = self._binding_cache = {}
        key = (id(self._program), ref)
        if key in cache:
            return cache[key]
        task_context = self._task_context(ref.task)
        graph = self._search(ref.task).graph
        source = graph.get_vertex_property(ref.vertex).program_state
        expander = LiftedSuccessorExpander(task_context, self._program)
        choices = []
        expander.for_each_successor(
            source, ProgramSearchStatistics(), lambda expansion: choices.append(expansion) or True, lambda: False
        )
        choice = next(c for c in choices if hasattr(c, "bindings"))
        rule = choice.rule
        bindings = []
        while not choice.exhausted():
            step = expander.apply_choice(source, choice, ProgramSearchStatistics())
            target = step.target
            bindings.append((_register_value(target.module_state, rule), target))
            choice.advance()
        cache[key] = (rule, bindings)
        return cache[key]

    def _ending(self, search: _Search, vertex: int) -> str:
        result = search.result
        if vertex in set(result.open_states):
            return "open_state"
        if vertex in set(result.deadend_states):
            return "empty_choose"
        return "subtree"

    def _declared_features(self) -> dict[str, Feature]:
        features = {}
        for category, getter in (
            ("concept", self._module.get_concept_features),
            ("boolean", self._module.get_boolean_features),
            ("numerical", self._module.get_numerical_features),
        ):
            for feature in getter():
                name = str(feature.get_symbol())
                features[name] = Feature(name, category, name)
        return features

    def _expression_category(self, expression: str) -> Literal["concept", "boolean", "numerical"]:
        head = expression.strip().lstrip("(").split(None, 1)[0]
        prefix = head.split("_", 1)[0]
        if prefix == "c":
            return "concept"
        if prefix == "b":
            return "boolean"
        if prefix == "n":
            return "numerical"
        raise ValueError(f"{expression!r} is neither a declared feature nor a concept (c_), Boolean (b_) or numerical (n_) expression")

    def _parse(self, feature: Feature):
        cache = getattr(self, "_parse_cache", None)
        if cache is None:
            cache = self._parse_cache = {}
        key = (id(self._program), feature)
        if key not in cache:
            if feature.name is not None:
                getter = {
                    "concept": self._module.get_concept_features,
                    "boolean": self._module.get_boolean_features,
                    "numerical": self._module.get_numerical_features,
                }[feature.category]
                cache[key] = ("declared", next(f for f in getter() if str(f.get_symbol()) == feature.name))
            else:
                parse = {"concept": parse_concept, "boolean": parse_boolean, "numerical": parse_numerical}[feature.category]
                cache[key] = ("expression", parse(feature.expression, self._parser.get_domain(), self._module, self._domain.ext_repository))
        return cache[key]

    def _context(self, task_context: LiftedTaskContext, program_state) -> LiftedStateEvaluationContext:
        storage = EvaluationStorage(task_context.dl_denotation_repository)
        module_state = program_state.module_state
        return LiftedStateEvaluationContext(
            program_state.state, task_context.dl_builder, storage, module_state.arguments, module_state.registers
        )

    def _evaluate(self, task_context: LiftedTaskContext, feature: Feature, program_state, binding: str | None) -> object:
        kind, parsed = self._parse(feature)
        context = self._context(task_context, program_state)
        denotation = evaluate(parsed, context) if kind == "declared" else parsed.evaluate(context)
        if feature.category == "concept":
            members = {_object_name(member) for member in denotation}
            return binding in members if binding is not None else len(members)
        return denotation.get()

    def _feature_values(self, task_context, source, target, rule, binding: str | None) -> dict[str, object]:
        """Declared features for a binding (concepts: membership at the source) or, without a rule, at one state."""
        values = {}
        for name, feature in self._declared_features().items():
            if feature.category == "concept":
                if rule is None:
                    continue
                values[name] = self._evaluate(task_context, feature, source, binding)
            else:
                values[name] = self._evaluate(task_context, feature, target, None)
        return values
