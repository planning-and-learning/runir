"""Handles and results of an exploration session; each result has a compact `describe()` for agents."""

from __future__ import annotations

from collections.abc import Mapping
from dataclasses import dataclass
from typing import Literal

Value = bool | int | float
"""A feature value: Boolean, numerical, or for a concept the membership of a binding (or the size without one)."""
Category = Literal["concept", "boolean", "numerical"]
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

    def describe(self) -> str:
        return self.name


@dataclass(frozen=True, slots=True)
class ChooseRule:
    name: str
    register: str

    def describe(self) -> str:
        return self.name


@dataclass(frozen=True, slots=True)
class Feature:
    expression: str
    category: Category
    name: str | None = None

    def describe(self) -> str:
        return self.name or self.expression


@dataclass(frozen=True, slots=True)
class VertexRef:
    task: Task
    vertex: int

    def describe(self) -> str:
        return f"{self.task.name}:v{self.vertex}"


@dataclass(frozen=True, slots=True)
class BindingRef:
    point: VertexRef
    binding: str

    def describe(self) -> str:
        return f"{self.point.describe()}:{self.binding}"


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

    def describe(self) -> str:
        head = f"{self.task.name}: {self.status}" + (f" ({self.category})" if self.category else "")
        stats = (
            f"expanded={self.num_expanded} backtracks={self.num_backtracks} "
            f"choice_depth={self.choice_depth} choice_width={self.choice_width} seconds={self.seconds:.2f}"
        )
        tail = f"failure={self.failure.describe()} " if self.failure else ""
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

    def describe(self) -> str:
        depth = f" choice_depth={self.choice_depth}" if self.choice_depth is not None else ""
        return f"{self.task.name}: execute {self.execute}, prove {self.prove}{depth}"


@dataclass(frozen=True, slots=True)
class Validation:
    tasks: tuple[TaskValidation, ...]
    baseline: Mapping[Task, int] | None = None
    """Choice depth of the session's initial program per task it executes successfully; None while exploring it."""

    def describe(self) -> str:
        passed = [task for task in self.tasks if task.passed]
        depth = sum(task.choice_depth or 0 for task in passed)
        lines = [f"passed {len(passed)}/{len(self.tasks)}; choice_depth sum over passed tasks {depth}"]
        both = [task for task in passed if self.baseline is not None and task.task in self.baseline]
        if self.baseline is not None and both:
            baseline = self.baseline
            before = sum(baseline[task.task] for task in both)
            after = sum(task.choice_depth or 0 for task in both)
            increases = sorted(both, key=lambda task: (task.choice_depth or 0) - baseline[task.task], reverse=True)
            worse = [
                f"{task.task.name} {baseline[task.task]}->{task.choice_depth}"
                for task in increases
                if (task.choice_depth or 0) > baseline[task.task]
            ][:5]
            lines.append(
                f"on the {len(both)} tasks both pass, choice_depth {before} (initial program) -> {after} (this program)"
                + (f"; largest increases: {', '.join(worse)}" if worse else "")
            )
        lines += [f"  {task.describe()}" for task in self.tasks if not task.passed]
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

    def describe(self) -> str:
        return (
            f"{self.rule.name}: occurrences={self.occurrences} tasks={self.tasks} max_width={self.max_width} "
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

    def describe(self) -> str:
        chosen = f"{self.chosen}/{self.width}" if self.chosen else f"-/{self.width}"
        return f"{self.vertex.describe()} {self.rule.name} chosen={chosen} failed={self.failed} untried={self.untried}"


@dataclass(frozen=True, slots=True)
class Alternative:
    binding: BindingRef
    rank: int
    outcome: Outcome
    ending: str | None
    """For failures: `open_state`, `empty_choose`, or `subtree` when every continuation failed."""
    states: int | None
    deltas: Mapping[str, tuple[Value, Value]]
    """Feature: (value at the reference binding, value here); the reference is the successful binding, else the first."""

    def describe(self) -> str:
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

    def describe(self) -> str:
        direction = "max" if self.higher >= self.lower else "min"
        return (
            f"{self.feature.describe()}: score={self.score:.2f} ({direction}){' consistent' if self.consistent else ''} "
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

    def describe(self) -> str:
        informative = [separation for separation in self.ranking if separation.score > 0]
        lines = [
            f"{self.rule.name}: {len(self.pairs)} (succeeded, failed) pairs from {self.decided} points with a failed binding; "
            f"{self.undecided} points without one",
            *(f"  {separation.describe()}" for separation in informative[:10]),
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

    def describe(self) -> str:
        if not self.terminating:
            return f"{self.rule.name} as Load: the rewritten program is not structurally terminating"
        failed = ", ".join(f"{task.name} {stage} {category}" for task, stage, category in self.failed) or "-"
        unavailable = ", ".join(f"{task.name} {stage}" for task, stage in self.unavailable) or "-"
        return (
            f"{self.rule.name} as Load: passed={len(self.passed)} failed={len(self.failed)} "
            f"unavailable={len(self.unavailable)} choice_depth {self.choice_depth[0]}->{self.choice_depth[1]}\n"
            f"  failed: {failed}\n  unavailable: {unavailable}"
        )


@dataclass(frozen=True, slots=True)
class Step:
    action: str
    rule: str | None
    """For successors: the rule admitting the action at the vertex, or None if no rule does."""
    deltas: Mapping[str, tuple[Value, Value]]
    """Boolean and numerical features before and after the action, under the vertex's registers and arguments."""

    def describe(self) -> str:
        deltas = " ".join(f"{name}:{a}>{b}" for name, (a, b) in self.deltas.items()) or "-"
        rule = f" rule={self.rule}" if self.rule else (" rule=(none)" if self.rule is None else "")
        return f"{self.action}{rule} deltas: {deltas}"


@dataclass(frozen=True, slots=True)
class Plan:
    start: VertexRef
    status: str
    steps: tuple[Step, ...]

    def describe(self) -> str:
        start = self.start.describe()
        if self.status in ("exhausted", "unsolvable"):
            return f"no plan from {start}: its state is a dead end, so an earlier step on the path made the task unsolvable"
        if self.status != "solved":
            return f"no plan from {start} found within the budget ({self.status})"
        head = f"plan from {start}: {len(self.steps)} steps (need not follow the program)"
        return "\n".join([head, *(f"  {i}. {step.describe()}" for i, step in enumerate(self.steps, 1))])


@dataclass(frozen=True, slots=True)
class Vertex:
    ref: VertexRef
    memory: str
    registers: Mapping[str, str | None]
    atoms: tuple[str, ...]
    features: Mapping[str, Value]
    goal: bool

    def describe(self) -> str:
        registers = " ".join(f"{name}={value}" for name, value in self.registers.items())
        features = " ".join(f"{name}={value}" for name, value in self.features.items())
        return (
            f"{self.ref.describe()} memory={self.memory}{' goal' if self.goal else ''}\n  registers: {registers}\n"
            f"  features: {features}\n  atoms: {' '.join(self.atoms)}"
        )


@dataclass(frozen=True, slots=True)
class Failure:
    run: TaskRun
    memory: str | None
    registers: Mapping[str, str | None]
    forbidden: int | None
    """Applicable actions at the failure vertex that no rule admits; None if there is no failure vertex."""
    applicable: int | None

    def describe(self) -> str:
        if self.run.failure is None:
            return f"{self.run.task.name}: {self.run.status}" + (f" ({self.run.category})" if self.run.category else "")
        registers = " ".join(f"{name}={value}" for name, value in self.registers.items())
        return (
            f"{self.run.failure.describe()} {self.run.category} memory={self.memory} registers: {registers}; "
            f"{self.forbidden} of {self.applicable} applicable actions admitted by no rule"
        )


@dataclass(frozen=True, slots=True)
class ModuleTermination:
    module: str
    status: str
    surviving_rules: tuple[str, ...]
    """Rules that remain on cycles after removing every rule some measure orders."""
    cycle: tuple[str, ...]
    """One cycle of the abstract policy graph: states as `memory[true Booleans; positive numericals]` and rules."""

    def describe(self) -> str:
        lines = [f"module {self.module}: {self.status}; surviving rules: {', '.join(self.surviving_rules) or '-'}"]
        if self.cycle:
            lines.append("  cycle: " + " ".join(self.cycle))
        return "\n".join(lines)


@dataclass(frozen=True, slots=True)
class Termination:
    terminating: bool
    status: str
    modules: tuple[ModuleTermination, ...]
    recursive_call_rules: tuple[str, ...]

    def describe(self) -> str:
        if self.terminating:
            return "structurally terminating"
        lines = [f"not structurally terminating ({self.status})"]
        lines += [module.describe() for module in self.modules]
        if self.recursive_call_rules:
            lines.append(f"recursive call rules: {', '.join(self.recursive_call_rules)}")
        return "\n".join(lines)
