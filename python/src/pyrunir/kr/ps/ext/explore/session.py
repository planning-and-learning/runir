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

import difflib
import time
from collections.abc import Iterable, Mapping, Sequence
from concurrent.futures import ThreadPoolExecutor
from dataclasses import dataclass, field
from datetime import timedelta
from pathlib import Path
from typing import cast

from pypddl.formalism import ParserOptions
from pytyr.formalism.planning import Parser
from pytyr.planning import SearchBudget, lifted
from pyyggdrasil.execution import ExecutionContext

from pyrunir.datasets import LiftedTaskSearchContext
from pyrunir.kr import DomainContext, LiftedTaskContext
from pyrunir.kr.dl.semantics import Boolean, Concept, EvaluationStorage, LiftedStateEvaluationContext, Numerical
from pyrunir.kr.ps.ext import (
    ConceptChoice,
    ConceptChooseRule,
    LiftedModuleState,
    LiftedProgramProofGraph,
    LiftedProgramProofResults,
    LiftedProgramSearchOptions,
    LiftedProgramState,
    LiftedSuccessorExpander,
    Program,
    ProgramProofStatus,
    ProgramSearchStatistics,
    RoleChoice,
    RoleChooseRule,
    RuleVariant,
    StateMemorization,
    evaluate,
    find_lifted_solution,
)
from pyrunir.kr.ps.ext.dl import (
    BooleanFeature,
    ConceptFeature,
    NumericalFeature,
    parse_boolean,
    parse_concept,
    parse_numerical,
    parse_program,
    structural_termination,
)

from .graph import (
    object_atoms,
    deltas,
    failure_category,
    is_choose,
    object_name,
    reachable_count,
    register_value,
    rewrite_choose_as_load,
    shortest_path,
    vertex_status,
)
from .results import (
    Alternative,
    BindingRef,
    Budget,
    BudgetExhausted,
    Category,
    ChoicePoint,
    ChooseRule,
    ChooseSummary,
    Contrast,
    DeadEnd,
    Failure,
    Feature,
    LoadTrial,
    ModuleTermination,
    Outcome,
    Plan,
    Separation,
    Step,
    Task,
    TaskRun,
    TaskValidation,
    Termination,
    Validation,
    Value,
    Vertex,
    VertexRef,
)


_Expression = Concept | Boolean | Numerical
_DeclaredFeature = ConceptFeature | BooleanFeature | NumericalFeature


@dataclass(slots=True)
class _Search:
    """One search and the solved/failed/unknown status of its vertices."""

    result: LiftedProgramProofResults
    seconds: float
    status: dict[int, bool | None] = field(default_factory=dict[int, bool | None])

    @property
    def graph(self) -> LiftedProgramProofGraph:
        return self.result.graph


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
        self._binding_cache: dict[tuple[int, VertexRef], tuple[RuleVariant, list[tuple[str | None, LiftedProgramState]]]] = {}
        self._parse_cache: dict[tuple[int, Feature], _DeclaredFeature | _Expression] = {}
        self._initial = parse_program(program, self._parser.get_domain(), self._domain.ext_repository)
        self._initial_text = program
        self._initial_depths: dict[str, int | None] = {}
        self.set_program(program)

    # -- boundary -----------------------------------------------------------------------------------------------------

    @property
    def tasks(self) -> tuple[Task, ...]:
        return tuple(Task(name) for name in self._task_paths)

    @property
    def spent_seconds(self) -> float:
        return self._spent

    def set_program(self, text: str, /) -> None:
        """Explore `text` from now on; earlier searches are discarded. A text that does not parse leaves the
        current program unchanged."""
        try:
            program = parse_program(text, self._parser.get_domain(), self._domain.ext_repository)
        except Exception as error:
            raise ValueError(f"the program was not changed: {error}") from error
        self._text = text
        self._program = program
        self._module = program.get_entry_module()
        self._searches: dict[str, _Search] = {}
        self._proofs: dict[str, LiftedProgramProofResults] = {}
        self._rules: dict[str, ChooseRule] = {}
        for bodies in self._module.get_memory_transitions():
            for body in bodies:
                if is_choose(body):
                    variant = cast(ConceptChooseRule | RoleChooseRule, body.get_variant())
                    self._rules[body.get_symbol()] = ChooseRule(body.get_symbol(), str(variant.get_register().get_name()))

    @property
    def program(self) -> str:
        return self._text

    def task(self, name: str, /) -> Task:
        if name not in self._task_paths:
            close = difflib.get_close_matches(name, self._task_paths, n=3)
            raise KeyError(f"unknown task {name!r}; {len(self._task_paths)} tasks" + (f", closest: {', '.join(close)}" if close else ""))
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
        target: set[int] = goals if result.status == ProgramProofStatus.SUCCESS else ({failure} if failure is not None else set())
        path = shortest_path(graph, initial, target) if initial is not None and target else []
        statistics: ProgramSearchStatistics = result.statistics
        return TaskRun(
            task=task,
            status=result.status.name.lower(),
            category=failure_category(result),
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

        def outcome(result: LiftedProgramProofResults) -> str:
            if result.status == ProgramProofStatus.SUCCESS:
                return "passed"
            if result.status == ProgramProofStatus.FAILURE:
                return f"failed ({failure_category(result)})"
            return f"unavailable ({result.status.name.lower()})"

        def prove(task: Task) -> LiftedProgramProofResults:
            if task.name not in self._proofs:
                self._proofs[task.name] = self._find(task, self._budget, universal=True)[0]
            return self._proofs[task.name]

        with ThreadPoolExecutor(self._workers) as pool:
            proofs = list(pool.map(prove, tasks))
        rows: list[TaskValidation] = []
        for task, proof in zip(tasks, proofs):
            execute = self._search(task).result
            rows.append(TaskValidation(
                task, outcome(execute), outcome(proof),
                execute.statistics.choice_depth if execute.status == ProgramProofStatus.SUCCESS else None,
            ))
        if self._text == self._initial_text:
            return Validation(tuple(rows))
        return Validation(tuple(rows), self._initial_choice_depths(tasks))

    def _initial_choice_depths(self, tasks: Sequence[Task]) -> dict[Task, int]:
        """Choice depth of the session's initial program on the tasks it executes successfully."""
        def depth(task: Task) -> int | None:
            if task.name not in self._initial_depths:
                result, _ = self._find(task, self._budget, program=self._initial)
                self._initial_depths[task.name] = (
                    result.statistics.choice_depth if result.status == ProgramProofStatus.SUCCESS else None
                )
            return self._initial_depths[task.name]

        with ThreadPoolExecutor(self._workers) as pool:
            depths = list(pool.map(depth, tasks))
        return {task: value for task, value in zip(tasks, depths) if value is not None}

    def termination(self) -> Termination:
        """Structural termination of the program; for each module that does not terminate, the rules left on
        cycles and one cycle of its abstract policy graph."""
        result = structural_termination(self._program)
        modules: list[ModuleTermination] = []
        for module, module_result in zip(self._program.get_modules(), result.module_results):
            if module_result.is_terminating():
                continue
            sieve = module_result.sieve_result
            surviving = tuple(rule.get_symbol() for rule in sieve.surviving_rules) if sieve is not None else ()
            cycle: list[str] = []
            if sieve is not None:
                graph = sieve.counterexample
                booleans = [feature.get_symbol() for feature in module.get_boolean_features()]
                numericals = [feature.get_symbol() for feature in module.get_numerical_features()]

                def state(vertex: int) -> str:
                    label = graph.get_vertex_property(vertex)
                    true = [name for i, name in enumerate(booleans) if label.boolean_values & (1 << i)]
                    positive = [name for i, name in enumerate(numericals) if label.numerical_values & (1 << i)]
                    return f"{label.memory_state.get_name()}[{' '.join(true)}; {' '.join(positive)}]"

                edges = graph.find_edge_cycle()
                for edge in edges:
                    cycle += [state(graph.get_source(edge)), f"--{graph.get_edge_property(edge).get_symbol()}-->"]
                if edges:
                    cycle.append(state(graph.get_target(edges[-1])))
            modules.append(ModuleTermination(str(module.get_name()), module_result.status.name.lower(), surviving, tuple(cycle)))
        return Termination(
            result.is_terminating(),
            result.status.name.lower(),
            tuple(modules),
            tuple(rule.get_symbol() for rule in result.recursive_call_rules),
        )

    def failures(self, tasks: Iterable[Task] | None = None, /) -> list[Failure]:
        """Every task whose execution does not succeed: its failure vertex, memory state, registers, and how many
        applicable actions no rule admits there."""
        failures: list[Failure] = []
        for run in self.runs(tasks):
            if run.status == "success":
                continue
            if run.failure is None:
                failures.append(Failure(run, None, {}, None, None))
                continue
            vertex = self.vertex(run.failure)
            steps = self.successors(run.failure)
            failures.append(Failure(run, vertex.memory, vertex.registers, sum(step.rule is None for step in steps), len(steps)))
        return failures

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
            register_value(graph.get_vertex_property(graph.get_target(edge)).program_state.module_state, rule): edge
            for edge in graph.get_out_edge_indices(ref.vertex)
            if is_choose(graph.get_edge_property(edge).rule)
        }
        rows: list[tuple[str, int, Outcome, str | None, int | None, LiftedProgramState]] = []
        for rank, (binding, target) in enumerate(bindings, 1):
            name = str(binding)
            if binding in tried:
                child = graph.get_target(tried[binding])
                solved = search.status.get(child)
                outcome: Outcome = "succeeded" if solved is True else "failed" if solved is False else "unknown"
                ending = self._ending(search, child) if outcome == "failed" else None
                rows.append((name, rank, outcome, ending, reachable_count(graph, child), target))
            elif budget is not None:
                result, _ = self._find(ref.task, budget, start=target)
                outcome = "succeeds" if result.status == ProgramProofStatus.SUCCESS else "fails" if result.status == ProgramProofStatus.FAILURE else "unknown"
                rows.append((name, rank, outcome, failure_category(result) if outcome == "fails" else None, result.statistics.num_expanded, target))
            else:
                rows.append((name, rank, "untried", None, None, target))
        task_context = self._task_context(ref.task)
        source = graph.get_vertex_property(ref.vertex).program_state
        values = [self._feature_values(task_context, source, row[5], rule, row[0]) for row in rows]
        reference = next((i for i, row in enumerate(rows) if row[2] in ("succeeded", "succeeds")), 0)
        profiles = [object_atoms(source.state, row[0]) for row in rows]
        common: set[str] = set(profiles[0]).intersection(*profiles[1:]) if profiles else set()
        return [
            Alternative(
                binding=BindingRef(ref, name),
                rank=rank,
                outcome=outcome,
                ending=ending,
                states=states,
                deltas={key: (values[reference][key], value) for key, value in values[i].items() if values[reference][key] != value},
                atoms=tuple(sorted(profiles[i] - common))[:12],
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

    def value(self, feature: Feature, binding: BindingRef, /) -> Value:
        """`feature` for one binding: membership at the Choose source for concepts, else the value after binding."""
        ref = binding.point
        _, bindings = self._bindings(ref)
        target = next(state for name, state in bindings if str(name) == binding.binding)
        source = self._search(ref.task).graph.get_vertex_property(ref.vertex).program_state
        task_context = self._task_context(ref.task)
        return self._evaluate(task_context, feature, source if feature.category == "concept" else target, binding.binding)

    # -- ablation ------------------------------------------------------------------------------------------------------

    def try_load(self, rule: ChooseRule, /, tasks: Iterable[Task] | None = None) -> LoadTrial:
        """Execute and prove the program with `rule`'s Choose bodies replaced by Loads (first binding, no backtracking)."""
        tasks = list(tasks) if tasks is not None else list(self.tasks)
        text = rewrite_choose_as_load(str(self._program), rule.name)
        program = parse_program(text, self._parser.get_domain(), self._domain.ext_repository)
        if not structural_termination(program).is_terminating():
            return LoadTrial(rule, False, (), (), (), (0, 0))
        self._prefetch(tasks)
        before = {task: self._search(task).result for task in tasks}

        def trial(task: Task) -> tuple[Task, LiftedProgramProofResults, LiftedProgramProofResults]:
            execute, _ = self._find(task, self._budget, program=program)
            prove, _ = self._find(task, self._budget, program=program, universal=True)
            return task, execute, prove

        passed: list[Task] = []
        failed: list[tuple[Task, str, str]] = []
        unavailable: list[tuple[Task, str]] = []
        depth_before = depth_after = 0
        with ThreadPoolExecutor(self._workers) as pool:
            for task, execute, prove in pool.map(trial, tasks):
                stages = (("execute", execute), ("prove", prove))
                bad = [(stage, r) for stage, r in stages if r.status == ProgramProofStatus.FAILURE]
                open_ = [(stage, r) for stage, r in stages if r.status not in (ProgramProofStatus.SUCCESS, ProgramProofStatus.FAILURE)]
                if bad:
                    failed.append((task, bad[0][0], failure_category(bad[0][1]) or "failure"))
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
        registers: dict[str, str | None] = {
            **{str(r.get_name()): object_name(v) for r, v in zip(module.get_concept_registers(), module_state.registers.concept_values)},
            **{str(r.get_name()): object_name(v) for r, v in zip(module.get_role_registers(), module_state.registers.role_values)},
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

    def successors(self, ref: VertexRef, /) -> list[Step]:
        """Every applicable action in the vertex's state, the rule that admits it there (None: no rule does), and its
        feature deltas. At a failure vertex this shows which moves the conditions and effects forbid."""
        task_context = self._task_context(ref.task)
        search = task_context.search_context
        program_state = self._search(ref.task).graph.get_vertex_property(ref.vertex).program_state
        module_state = program_state.module_state
        expander = LiftedSuccessorExpander(task_context, self._program)
        before = self._state_values(task_context, program_state.state, module_state)
        node = search.successor_generator.get_node(search.state_repository, program_state.state.get_index())
        steps: list[Step] = []
        for successor in search.successor_generator.get_labeled_successor_nodes(node, search.state_repository, search.axiom_evaluator):
            rule = expander.matching_rule(program_state, successor)
            after = self._state_values(task_context, successor.node.get_state(), module_state)
            steps.append(Step(str(successor.label), None if rule is None else rule.get_symbol(), deltas(before, after)))
        return steps

    def plan(self, ref: VertexRef, /, budget: Budget | None = None) -> Plan:
        """A plan from the vertex's state to the goal by lazy GBFS with FF; it need not follow the program.
        Each step shows its feature deltas under the vertex's registers and arguments."""
        budget = budget or self._budget
        task_context = self._task_context(ref.task)
        search = task_context.search_context
        module_state = self._search(ref.task).graph.get_vertex_property(ref.vertex).program_state.module_state
        state = self._search(ref.task).graph.get_vertex_property(ref.vertex).state
        options = lifted.gbfs_lazy.Options()
        options.start_node = search.successor_generator.get_node(search.state_repository, state.get_index())
        options.search_budget = SearchBudget(max_num_states=budget.max_states, max_time=timedelta(seconds=budget.max_seconds))
        if self._spent >= self._total_seconds:
            raise BudgetExhausted(f"the session's search budget of {self._total_seconds:.0f} s is used up")
        started = time.monotonic()
        found = lifted.gbfs_lazy.find_solution(
            search.task, search.state_repository, search.axiom_evaluator, search.successor_generator,
            lifted.FFRPGHeuristic(search.task, search.execution_context), options,
        )
        self._spent += time.monotonic() - started
        steps: list[Step] = []
        if found.plan is not None:
            source = found.plan.get_start_node().get_state()
            before = self._state_values(task_context, source, module_state)
            for successor in found.plan.get_labeled_succ_nodes():
                after = self._state_values(task_context, successor.node.get_state(), module_state)
                steps.append(Step(str(successor.label), "", deltas(before, after)))
                before = after
        return Plan(ref, found.status.name.lower(), tuple(steps))

    def dead_end(self, task: Task, /, budget: Budget | None = None) -> DeadEnd:
        """Where the failure path of `task` makes the task unsolvable: dead ends are absorbing, so a binary search
        with plan searches finds the step from the last solvable to the first unsolvable state."""
        path = self.run(task).path
        if not path:
            raise ValueError(f"{task.name} has no failure path")
        unknown = False

        def solvable(ref: VertexRef) -> bool:
            nonlocal unknown
            status = self.plan(ref, budget).status
            if status not in ("solved", "exhausted", "unsolvable"):
                unknown = True
            return status not in ("exhausted", "unsolvable")

        if solvable(path[-1]):
            return DeadEnd(task, path[-1], None, None, None, unknown)
        if not solvable(path[0]):
            return DeadEnd(task, None, path[0], None, None, unknown)
        low, high = 0, len(path) - 1  # path[low] solvable, path[high] dead
        while high - low > 1:
            middle = (low + high) // 2
            if solvable(path[middle]):
                low = middle
            else:
                high = middle
        graph = self._search(task).graph
        edge = next(
            edge for edge in graph.get_out_edge_indices(path[low].vertex) if graph.get_target(edge) == path[high].vertex
        )
        label = graph.get_edge_property(edge)
        return DeadEnd(
            task,
            path[low],
            path[high],
            None if label.rule is None else label.rule.get_symbol(),
            None if label.action is None else str(label.action),
            unknown,
        )

    # -- internals -----------------------------------------------------------------------------------------------------

    def _state_values(self, task_context: LiftedTaskContext, state: lifted.State, module_state: LiftedModuleState) -> dict[str, Value]:
        storage = EvaluationStorage(task_context.dl_denotation_repository)
        context = LiftedStateEvaluationContext(state, task_context.dl_builder, storage, module_state.arguments, module_state.registers)
        values: dict[str, Value] = {}
        for feature in self._module.get_boolean_features():
            values[feature.get_symbol()] = evaluate(feature, context).get()
        for feature in self._module.get_numerical_features():
            values[feature.get_symbol()] = evaluate(feature, context).get()
        return values

    def _task_context(self, task: Task) -> LiftedTaskContext:
        if task.name not in self._task_contexts:
            planning_task = self._parser.parse_task(self._task_paths[task.name], ParserOptions())
            self._task_contexts[task.name] = LiftedTaskContext(
                self._domain, LiftedTaskSearchContext(lifted.Task(planning_task), ExecutionContext(1))
            )
        return self._task_contexts[task.name]

    def _find(
        self, task: Task, budget: Budget, *, program: Program | None = None, universal: bool = False, start: LiftedProgramState | None = None
    ) -> tuple[LiftedProgramProofResults, float]:
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
            self._searches[task.name] = _Search(result, seconds, vertex_status(result))
        return self._searches[task.name]

    def _prefetch(self, tasks: Sequence[Task]) -> None:
        missing = [task for task in tasks if task.name not in self._searches]
        for task in missing:
            self._task_context(task)
        def find(task: Task) -> tuple[LiftedProgramProofResults, float]:
            return self._find(task, self._budget)

        with ThreadPoolExecutor(self._workers) as pool:
            for task, (result, seconds) in zip(missing, pool.map(find, missing)):
                self._searches[task.name] = _Search(result, seconds, vertex_status(result))

    def _points_on_path(self, task: Task) -> list[ChoicePoint]:
        search = self._search(task)
        graph = search.graph
        run = self.run(task)
        on_path = [ref.vertex for ref in run.path]
        next_on_path = dict(zip(on_path, on_path[1:]))
        points: list[ChoicePoint] = []
        for vertex in on_path:
            edges = [edge for edge in graph.get_out_edge_indices(vertex) if is_choose(graph.get_edge_property(edge).rule)]
            if not edges:
                continue
            ref = VertexRef(task, vertex)
            rule, bindings = self._bindings(ref)
            children = [graph.get_target(edge) for edge in edges]
            successor = next_on_path.get(vertex)
            chosen = next((rank for rank, edge in enumerate(edges, 1) if graph.get_target(edge) == successor), None)
            solved = search.status.get(successor) if successor is not None else None
            points.append(
                ChoicePoint(
                    vertex=ref,
                    rule=self._rules[rule.get_symbol()],
                    width=len(bindings),
                    chosen=chosen if solved is True else None,
                    failed=sum(search.status.get(child) is False for child in children),
                    untried=len(bindings) - len(edges),
                )
            )
        return points

    def _bindings(self, ref: VertexRef) -> tuple[RuleVariant, list[tuple[str | None, LiftedProgramState]]]:
        """The Choose rule at `ref` and its bindings in `:order` rank as (object name, target program state)."""
        key = (id(self._program), ref)
        if key in self._binding_cache:
            return self._binding_cache[key]
        task_context = self._task_context(ref.task)
        graph = self._search(ref.task).graph
        source = graph.get_vertex_property(ref.vertex).program_state
        expander = LiftedSuccessorExpander(task_context, self._program)
        choices: list[ConceptChoice | RoleChoice] = []

        def emit(expansion: object) -> bool:
            if isinstance(expansion, (ConceptChoice, RoleChoice)):
                choices.append(expansion)
            return True

        # The generated stub types `emit` as a bare Callable, whose parameters pyright cannot know.
        expander.for_each_successor(source, ProgramSearchStatistics(), emit, lambda: False)  # pyright: ignore[reportUnknownMemberType]
        choice = choices[0]
        rule = choice.rule
        bindings: list[tuple[str | None, LiftedProgramState]] = []
        while not choice.exhausted():
            step = expander.apply_choice(source, choice, ProgramSearchStatistics())
            bindings.append((register_value(step.target.module_state, rule), step.target))
            choice.advance()
        self._binding_cache[key] = (rule, bindings)
        return rule, bindings

    def _ending(self, search: _Search, vertex: int) -> str:
        result = search.result
        if vertex in set(result.open_states):
            return "open_state"
        if vertex in set(result.deadend_states):
            return "empty_choose"
        return "subtree"

    def _declared_features(self) -> dict[str, Feature]:
        features: dict[str, Feature] = {}
        for feature in self._module.get_concept_features():
            features[feature.get_symbol()] = Feature(feature.get_symbol(), "concept", feature.get_symbol())
        for feature in self._module.get_boolean_features():
            features[feature.get_symbol()] = Feature(feature.get_symbol(), "boolean", feature.get_symbol())
        for feature in self._module.get_numerical_features():
            features[feature.get_symbol()] = Feature(feature.get_symbol(), "numerical", feature.get_symbol())
        return features

    def _expression_category(self, expression: str) -> Category:
        head = expression.strip().lstrip("(").split(None, 1)[0]
        prefix = head.split("_", 1)[0]
        if prefix == "c":
            return "concept"
        if prefix == "b":
            return "boolean"
        if prefix == "n":
            return "numerical"
        raise ValueError(f"{expression!r} is neither a declared feature nor a concept (c_), Boolean (b_) or numerical (n_) expression")

    def _parse(self, feature: Feature) -> _DeclaredFeature | _Expression:
        key = (id(self._program), feature)
        if key not in self._parse_cache:
            domain, repository = self._parser.get_domain(), self._domain.ext_repository
            parsed: _DeclaredFeature | _Expression
            if feature.name is not None:
                declared: Sequence[_DeclaredFeature] = (
                    self._module.get_concept_features() if feature.category == "concept"
                    else self._module.get_boolean_features() if feature.category == "boolean"
                    else self._module.get_numerical_features()
                )
                parsed = next(f for f in declared if f.get_symbol() == feature.name)
            elif feature.category == "concept":
                parsed = parse_concept(feature.expression, domain, self._module, repository)
            elif feature.category == "boolean":
                parsed = parse_boolean(feature.expression, domain, self._module, repository)
            else:
                parsed = parse_numerical(feature.expression, domain, self._module, repository)
            self._parse_cache[key] = parsed
        return self._parse_cache[key]

    def _context(self, task_context: LiftedTaskContext, program_state: LiftedProgramState) -> LiftedStateEvaluationContext:
        storage = EvaluationStorage(task_context.dl_denotation_repository)
        module_state = program_state.module_state
        return LiftedStateEvaluationContext(
            program_state.state, task_context.dl_builder, storage, module_state.arguments, module_state.registers
        )

    def _evaluate(self, task_context: LiftedTaskContext, feature: Feature, program_state: LiftedProgramState, binding: str | None) -> Value:
        parsed = self._parse(feature)
        context = self._context(task_context, program_state)
        if isinstance(parsed, (ConceptFeature, Concept)):
            denotation = evaluate(parsed, context) if isinstance(parsed, ConceptFeature) else parsed.evaluate(context)
            members = {str(member.get_name()) for member in denotation}
            return binding in members if binding is not None else len(members)
        if isinstance(parsed, BooleanFeature):
            return evaluate(parsed, context).get()
        if isinstance(parsed, NumericalFeature):
            return evaluate(parsed, context).get()
        return parsed.evaluate(context).get()

    def _feature_values(
        self,
        task_context: LiftedTaskContext,
        source: LiftedProgramState,
        target: LiftedProgramState,
        rule: RuleVariant | None,
        binding: str | None,
    ) -> dict[str, Value]:
        """Declared features for a binding (concepts: membership at the source) or, without a rule, at one state."""
        values: dict[str, Value] = {}
        for name, feature in self._declared_features().items():
            if feature.category == "concept":
                if rule is None:
                    continue
                values[name] = self._evaluate(task_context, feature, source, binding)
            else:
                values[name] = self._evaluate(task_context, feature, target, None)
        return values
