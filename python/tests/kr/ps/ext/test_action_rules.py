# Copyright (C) 2026 Dominik Drexler
# SPDX-License-Identifier: GPL-3.0-or-later

import gc
from collections.abc import Callable, Iterable
from pathlib import Path
from typing import TypeAlias, TypeVar

import pytest
from ext_execution_utils import (
    DlContext,
    EvaluationEnvironment,
    ExecutionStep,
    Expander,
    ProgramState,
    TaskContext,
    collect_steps,
    initial_node,
    initial_program_state,
    make_dl_context,
    make_environment,
    make_expander,
)
from fixture_utils import FIXTURE_ROOT
from pypddl.formalism import ParserOptions
from pytyr.formalism.planning import ActionBinding, Object, ObjectIndex, Parser, PlanningDomain
from pytyr.planning import ground, lifted
from pyyggdrasil import database
from pyyggdrasil.execution import ExecutionContext
from pyyggdrasil.serialization import Dictionaries

from pyrunir.datasets import GroundTaskSearchContext, LiftedTaskSearchContext
from pyrunir.kr import ArityMismatchError, DomainContext, GroundTaskContext, LiftedTaskContext, UndefinedSymbolError
from pyrunir.kr.dl import semantics
from pyrunir.kr.ps import ext
from pyrunir.kr.ps.ext import dl
from pyrunir.serialization import register_table, serialize, table

Successor: TypeAlias = ground.LabeledNode | lifted.LabeledNode
T = TypeVar("T")

QUERY = '(q_join (q_atomic_state "at" (from)) (q_atomic_state "edge" (from to)))'


def _program(query: str = QUERY, effects: str = "(unchanged count)") -> str:
    return f"""(:program (:entry root)
      (:module (:symbol root) (:arguments) (:registers)
        (:entry m0) (:memory m0)
        (:features
          (:query (:symbol selected) (:expression {query}))
          (:query (:symbol edges) (:expression (q_atomic_state "edge" (from to))))
          (:query (:symbol reversed) (:expression (q_project (to from) {QUERY})))
          (:query (:symbol truth) (:expression (q_project () (q_atomic_state "at" (from)))))
          (:query (:symbol empty) (:expression
            (q_project () (q_difference (q_atomic_state "at" (from)) (q_atomic_state "at" (from))))))
          (:numerical (:symbol count) (:expression (n_count (c_atomic_state "at")))))
        (:rules (:rule (:symbol move-selected) (:expression
          (:source-memory m0) (:target-memory m0)
          (:action (:conditions) (:action "move") (:query selected) (:effects {effects})))))))"""


def _context(kind: str) -> tuple[TaskContext, PlanningDomain]:
    directory = FIXTURE_ROOT / "kr/ps/ext/choose"
    parser = Parser(directory / "domain.pddl", ParserOptions())
    task = lifted.Task(parser.parse_task(directory / "task.pddl", ParserOptions()))
    execution = ExecutionContext(1)
    domain = DomainContext(parser.get_domain())
    context: TaskContext
    if kind == "ground":
        context = GroundTaskContext(
            domain, GroundTaskSearchContext(task.instantiate_ground_task(execution).task, execution)
        )
    else:
        context = LiftedTaskContext(domain, LiftedTaskSearchContext(task, execution))
    return context, parser.get_domain()


def _matching_rule(expander: Expander, state: ProgramState, successor: Successor) -> ext.RuleVariant | None:
    if isinstance(expander, ext.GroundSuccessorExpander):
        assert isinstance(state, ext.GroundProgramState)
        assert isinstance(successor, ground.LabeledNode)
        return expander.matching_rule(state, successor)
    assert isinstance(state, ext.LiftedProgramState)
    assert isinstance(successor, lifted.LabeledNode)
    return expander.matching_rule(state, successor)


def _apply(
    expander: Expander, state: ProgramState, rule: ext.RuleVariant, successor: Successor
) -> ExecutionStep | None:
    if isinstance(expander, ext.GroundSuccessorExpander):
        assert isinstance(state, ext.GroundProgramState)
        assert isinstance(successor, ground.LabeledNode)
        return expander.apply(state, rule, successor)
    assert isinstance(state, ext.LiftedProgramState)
    assert isinstance(successor, lifted.LabeledNode)
    return expander.apply(state, rule, successor)


def _labeled_successors(task_context: TaskContext) -> list[ground.LabeledNode] | list[lifted.LabeledNode]:
    """Return the planning successors of the task's initial node."""
    search = task_context.search_context
    if isinstance(search, GroundTaskSearchContext):
        node = search.successor_generator.get_initial_node(search.state_repository, search.axiom_evaluator)
        return search.successor_generator.get_labeled_successor_nodes(
            node, search.state_repository, search.axiom_evaluator
        )
    node = search.successor_generator.get_initial_node(search.state_repository, search.axiom_evaluator)
    return search.successor_generator.get_labeled_successor_nodes(node, search.state_repository, search.axiom_evaluator)


def _solve(task_context: TaskContext, program: ext.Program) -> bool:
    if isinstance(task_context, GroundTaskContext):
        return ext.find_ground_solution(task_context, program, ext.GroundProgramSearchOptions()).is_successful()
    return ext.find_lifted_solution(task_context, program, ext.LiftedProgramSearchOptions()).is_successful()


def _same_state(left: ProgramState, right: ProgramState) -> bool:
    if isinstance(left, ext.GroundProgramState):
        assert isinstance(right, ext.GroundProgramState)
        return left == right
    assert isinstance(right, ext.LiftedProgramState)
    return left == right


def _same_planning_state(left: ProgramState, right: ProgramState) -> bool:
    if isinstance(left, ext.GroundProgramState):
        assert isinstance(right, ext.GroundProgramState)
        return left.state == right.state
    assert isinstance(right, ext.LiftedProgramState)
    return left.state == right.state


def _successor(step: ExecutionStep) -> Successor:
    successor = step.planning_successor
    assert successor is not None
    return successor.unpack()


def _rule(step: ExecutionStep) -> ext.RuleVariant:
    rule = step.rule
    assert rule is not None
    return rule


def _action(step: ExecutionStep) -> ActionBinding:
    transition = step.state_transition
    assert transition is not None
    return transition.action


def _ints(row: Iterable[object]) -> tuple[int, ...]:
    values: list[int] = []
    for value in row:
        assert isinstance(value, int)
        values.append(value)
    return tuple(values)


def _register_value(state: ProgramState) -> int:
    value = state.module_state.registers.concept_values[0]
    assert value is not None
    return int(value.get_index())


def _runtime(
    kind: str, source: str | None = None
) -> tuple[TaskContext, ext.Program, Expander, ProgramState, EvaluationEnvironment]:
    task_context, domain = _context(kind)
    program = dl.parse_program(source or _program(), domain, task_context.domain_context.ext_repository)
    expander = make_expander(task_context, program)
    state = initial_program_state(expander, initial_node(task_context).get_state())
    environment = make_environment(task_context, program)
    return task_context, program, expander, state, environment


@pytest.mark.parametrize("kind", ["ground", "lifted"])
def test_expander_rejects_foreign_states_programs_and_candidates(kind: str) -> None:
    task_context, domain = _context(kind)
    repository = task_context.domain_context.ext_repository
    program = dl.parse_program(_program(), domain, repository)
    expander = make_expander(task_context, program)
    planning_state = initial_node(task_context).get_state()
    state = initial_program_state(expander, planning_state)
    step = collect_steps(expander, state)[0]
    successor = _successor(step)
    rule = _rule(step)
    _foreign_context, foreign_program, foreign_expander, foreign_state, _foreign_environment = _runtime(kind)
    other_program = dl.parse_program(_program(effects=""), domain, repository)
    other_expander = make_expander(task_context, other_program)
    other_state = initial_program_state(other_expander, planning_state)

    with pytest.raises(ValueError, match="domain context repository"):
        make_expander(task_context, foreign_program)
    with pytest.raises(ValueError, match="selected task's state repository"):
        initial_program_state(expander, foreign_state.state)
    for wrong_state, message in ((foreign_state, "selected task"), (other_state, "selected program")):
        statistics = ext.ProgramSearchStatistics()
        with pytest.raises(ValueError, match=message):
            collect_steps(expander, wrong_state, statistics)
        assert statistics.num_generated == statistics.num_expanded == statistics.num_backtracks == 0
        with pytest.raises(ValueError, match=message):
            _matching_rule(expander, wrong_state, successor)
        with pytest.raises(ValueError, match=message):
            _apply(expander, wrong_state, rule, successor)

    foreign_successor = _successor(collect_steps(foreign_expander, foreign_state)[0])
    with pytest.raises(ValueError, match="selected task's state repository"):
        _matching_rule(expander, state, foreign_successor)
    with pytest.raises(ValueError, match="selected task's state repository"):
        _apply(expander, state, rule, foreign_successor)


def test_action_and_query_feature_bindings_round_trip_and_serialize() -> None:
    task_context, domain = _context("lifted")
    repository = task_context.domain_context.ext_repository
    program = dl.parse_program(_program(), domain, repository)
    module = program.get_entry_module()
    feature = module.get_query_features()[0]
    variant = module.get_memory_transitions()[0][0]
    rule = variant.get_variant()
    assert isinstance(feature, dl.QueryFeature)
    assert isinstance(feature.get_variant(), dl.ConcreteQueryFeature)
    assert isinstance(rule, ext.ActionRule)
    assert rule.get_query_feature() == feature
    assert rule.get_action_name() == "move"
    assert [column.get_name() for column in feature.get_expression().get_columns()] == ["from", "to"]
    assert feature.syntactic_complexity() > 0
    assert str(dl.parse_program(str(program), domain, repository)) == str(program)

    concrete = dl.ConcreteQueryFeatureData()
    concrete.symbol = feature.get_symbol()
    concrete.feature = feature.get_expression().get_index()
    assert repository.insert(concrete)[0] == feature.get_variant()
    wrapped = dl.QueryFeatureData()
    wrapped.variant = feature.get_variant().get_index()
    assert repository.insert(wrapped)[0] == feature
    data = ext.ActionRuleData()
    data.source = rule.get_source().get_index()
    data.target = rule.get_target().get_index()
    data.conditions = [condition.get_index() for condition in rule.get_conditions()]
    data.effects = [effect.get_index() for effect in rule.get_effects()]
    data.action_name = rule.get_action_name()
    data.query_feature = feature.get_index()
    assert repository.insert(data)[0] == rule
    module_data = ext.ModuleData()
    module_data.query_features = [feature.get_index()]
    assert module_data.query_features == [feature.get_index()]

    dictionaries = Dictionaries()
    register_table(dictionaries, ext.RuleVariant, "rules", "r")
    register_table(dictionaries, ext.ActionRule, "actions", "a", fields=("action_name", "query_feature"))
    register_table(
        dictionaries,
        dl.QueryFeature,
        "queries",
        "q",
        project=lambda value: {
            "symbol": value.get_symbol(),
            "expression": str(value.get_expression()),
        },
    )
    assert serialize(dictionaries, variant) == "r0"
    assert table(dictionaries, ext.ActionRule) == [{"action_name": "move", "query_feature": "q0"}]
    assert table(dictionaries, dl.QueryFeature) == [{"symbol": "selected", "expression": QUERY}]


@pytest.mark.parametrize("kind", ["ground", "lifted"])
def test_query_relation_preserves_rows_column_order_nullary_truth_and_lifetimes(kind: str) -> None:
    task_context, program, expander, state, environment = _runtime(kind)
    features = {feature.get_symbol(): feature for feature in program.get_entry_module().get_query_features()}

    dl_context = make_dl_context(environment, state)
    relation = ext.evaluate(features["selected"], dl_context)
    assert isinstance(relation, database.RelationView)
    assert relation == features["selected"].get_expression().evaluate(dl_context)
    assert len(relation) == 2
    assert relation.arity() == 2
    assert len(relation.columns()) == 2
    assert relation == ext.evaluate(features["selected"], dl_context)
    assert len({relation, ext.evaluate(features["selected"], dl_context)}) == 1
    assert all(isinstance(row, database.RelationRow) and len(row) == 2 for row in relation)
    rows = {_ints(row) for row in relation}
    objects = task_context.dl_denotation_repository
    assert all(isinstance(objects.get_object(ObjectIndex(value)), Object) for row in rows for value in row)
    assert {tuple(objects.get_object(ObjectIndex(value)).get_name() for value in row) for row in rows} == {
        ("start", "bad"),
        ("start", "good"),
    }
    assert relation[-1][-1] == relation.at(len(relation) - 1)[relation.arity() - 1]
    with pytest.raises(IndexError):
        relation[len(relation)]
    with pytest.raises(IndexError):
        relation[0][relation.arity()]
    assert {tuple(row) for row in ext.evaluate(features["reversed"], dl_context)} == {
        (target, source) for source, target in rows
    }
    truth = ext.evaluate(features["truth"], dl_context)
    empty = ext.evaluate(features["empty"], dl_context)
    assert truth.arity() == empty.arity() == 0
    assert [tuple(row) for row in truth] == [()]
    assert empty.empty()
    assert {tuple(row) for row in ext.evaluate(features["selected"], dl_context)} == rows
    assert {tuple(row) for row in relation} == rows

    steps = collect_steps(expander, state)
    assert len(steps) == 2
    assert rows == {tuple(int(object_.get_index()) for object_ in _action(step).get_objects()) for step in steps}
    for step in steps:
        rule = _rule(step)
        assert isinstance(rule.get_variant(), ext.ActionRule)
        successor = _successor(step)
        assert _matching_rule(expander, state, successor) == rule
        applied = _apply(expander, state, rule, successor)
        assert applied is not None
        assert _same_state(applied.target, step.target)
        del step, rule, successor, applied
    good_step = next(step for step in steps if _action(step).get_objects()[1].get_name() == "good")
    del relation, truth, empty
    state = good_step.target
    environment.reset_source()
    dl_context = make_dl_context(environment, state)
    relation = ext.evaluate(features["selected"], dl_context)
    assert len(relation) == 1
    new_rows = {tuple(row) for row in relation}
    row = relation.at(0)
    iterated_row = next(iter(relation))
    expected_row = tuple(row)
    del (
        steps,
        good_step,
        features,
        environment,
        dl_context,
        state,
        expander,
        program,
        task_context,
        objects,
    )
    gc.collect()
    assert {tuple(value) for value in relation} == new_rows
    del relation
    gc.collect()
    assert tuple(row) == tuple(iterated_row) == expected_row


def _retained(
    kind: str, project: Callable[[database.RelationView, database.RelationRepository], T]
) -> tuple[T, tuple[object, ...]]:
    """Project the first query relation of a fresh runtime and drop every other owner."""
    task_context, program, _expander, state, _environment = _runtime(kind)
    repository = task_context.dl_denotation_repository
    caches = semantics.DenotationCaches()
    storage = semantics.EvaluationStorage(repository)
    arguments = state.module_state.arguments
    registers = state.module_state.registers
    context: DlContext
    if isinstance(state, ext.GroundProgramState):
        context = semantics.GroundStateEvaluationContext(
            state.state, task_context.dl_builder, caches, repository, storage, arguments, registers
        )
    else:
        context = semantics.LiftedStateEvaluationContext(
            state.state, task_context.dl_builder, caches, repository, storage, arguments, registers
        )
    feature = program.get_entry_module().get_query_features()[0]
    relation = feature.get_expression().evaluate(context)
    expected = tuple(relation[0])
    return project(relation, repository.get_relation_repository()), expected


def _renamed(relation: database.RelationView, relations: database.RelationRepository) -> database.RelationView:
    columns = list(relation.columns())[::-1]
    renamed = relations.rename(relation, columns)
    assert isinstance(renamed, database.RelationView)
    assert list(renamed.columns()) == columns
    return renamed


@pytest.mark.parametrize("kind", ["ground", "lifted"])
@pytest.mark.parametrize("access", ["object", "row_iterator", "query_iterator", "renamed"])
def test_query_objects_iterators_and_renames_retain_owners(kind: str, access: str) -> None:
    if access == "object":
        object_, expected = _retained(kind, lambda relation, _: relation[0][0])
        gc.collect()
    elif access == "row_iterator":
        objects, expected = _retained(kind, lambda relation, _: iter(relation[0]))
        gc.collect()
        object_ = next(objects)
        del objects
    elif access == "query_iterator":
        rows, expected = _retained(kind, lambda relation, _: iter(relation))
        gc.collect()
        row = next(rows)
        assert tuple(row) == expected
        object_ = next(iter(row))
        del row, rows
    else:
        renamed, expected = _retained(kind, _renamed)
        gc.collect()
        row = renamed[0]
        assert tuple(row) == expected
        object_ = next(iter(row))
        del row, renamed
    gc.collect()
    assert isinstance(object_, int)
    assert object_ == expected[0]


@pytest.mark.parametrize("kind", ["ground", "lifted"])
def test_action_effects_filter_transitions(kind: str) -> None:
    task_context, program, expander, state, _environment = _runtime(kind, _program(effects="(decreases count)"))
    assert [step.status for step in collect_steps(expander, state)] == ["no_applicable_action"]
    assert not _solve(task_context, program)


@pytest.mark.parametrize("query", ['(q_atomic_state "at" (from))', '(q_project () (q_atomic_state "at" (from)))'])
def test_action_parser_rejects_incomplete_binding_schema(query: str) -> None:
    task_context, domain = _context("lifted")
    with pytest.raises(ArityMismatchError):
        dl.parse_program(_program(query=query), domain, task_context.domain_context.ext_repository)


def test_action_parser_rejects_undefined_query_feature() -> None:
    task_context, domain = _context("lifted")
    with pytest.raises(UndefinedSymbolError):
        dl.parse_program(
            _program().replace("(:query selected)", "(:query missing)"),
            domain,
            task_context.domain_context.ext_repository,
        )


@pytest.mark.parametrize("preprocessing", [False, True])
def test_action_structural_counterexample_retains_query_rule(preprocessing: bool) -> None:
    task_context, domain = _context("lifted")
    program = dl.parse_program(_program(), domain, task_context.domain_context.ext_repository)
    result = dl.structural_termination(program.get_entry_module(), use_incomplete_preprocessing=preprocessing)
    assert not result.is_terminating()
    sieve_result = result.sieve_result
    assert sieve_result is not None
    graph = sieve_result.counterexample
    rule = sieve_result.surviving_rules[0]
    del result, sieve_result, program  # Rule views keep the existing caller-owned repository contract.
    gc.collect()
    variant = rule.get_variant()
    assert isinstance(variant, ext.ActionRule)
    assert variant.get_query_feature().get_symbol() == "selected"
    assert all(
        isinstance(graph.get_edge_property(index).get_variant(), ext.ActionRule) for index in graph.get_edge_indices()
    )


@pytest.mark.parametrize("kind", ["ground", "lifted"])
def test_action_rejects_visited_inapplicable_query_tuple(kind: str) -> None:
    _task_context, _program_value, expander, state, _environment = _runtime(
        kind,
        _program(query='(q_atomic_state "edge" (from to))'),
    )
    with pytest.raises(RuntimeError, match="offered action is not applicable"):
        collect_steps(expander, state)


@pytest.mark.parametrize("kind", ["ground", "lifted"])
def test_static_query_role_closure_query_composition(kind: str) -> None:
    closure = '(q_role (from to) (r_transitive_closure (r_project from to (q_atomic_state "edge" (from to)))))'
    task_context, program, expander, state, environment = _runtime(kind, _program(query=closure))
    features = {feature.get_symbol(): feature for feature in program.get_entry_module().get_query_features()}

    dl_context = make_dl_context(environment, state)
    edges = {tuple(row) for row in ext.evaluate(features["edges"], dl_context)}
    rows = {tuple(row) for row in ext.evaluate(features["selected"], dl_context)}
    assert len(edges) == 3
    assert len(rows) == 4
    assert rows == edges | {(source, target) for source, middle in edges for other, target in edges if middle == other}
    successor = _labeled_successors(task_context)[0]
    rule = _matching_rule(expander, state, successor)
    assert rule is not None
    step = _apply(expander, state, rule, successor)
    assert step is not None
    state = step.target
    environment.reset_source()
    dl_context = make_dl_context(environment, state)
    assert {tuple(row) for row in ext.evaluate(features["selected"], dl_context)} == rows


@pytest.mark.parametrize("kind", ["ground", "lifted"])
def test_query_features_follow_module_arguments_and_registers_in_the_same_state(kind: str) -> None:
    source = """(:program (:entry root)
      (:module (:symbol root) (:arguments) (:registers)
        (:entry m0) (:memory m0 m1)
        (:features (:concept (:symbol candidates) (:expression (c_atomic_state "candidate"))))
        (:rules (:rule (:symbol enter) (:expression
          (:source-memory m0) (:target-memory m1)
          (:call (:conditions) (:callee child) (:arguments candidates))))))
      (:module (:symbol child) (:arguments (:concept X)) (:registers (:concept selected))
        (:entry m0) (:memory m0 m1)
        (:features
          (:concept (:symbol candidates) (:expression (c_argument X)))
          (:query (:symbol argument) (:expression (q_concept choice (c_argument X))))
          (:query (:symbol register) (:expression (q_concept choice (c_register selected))))
          (:query (:symbol intersection) (:expression
            (q_join (q_concept choice (c_argument X)) (q_concept choice (c_register selected))))))
        (:rules (:rule (:symbol bind) (:expression
          (:source-memory m0) (:target-memory m1)
          (:load (:conditions) (:concept candidates) (:register (:concept selected))))))))"""
    _task_context, _program_value, expander, initial, environment = _runtime(kind, source)
    child = collect_steps(expander, initial)[0].target
    choices = [step.target for step in collect_steps(expander, child)]
    assert len(choices) == 2
    assert _same_planning_state(choices[0], choices[1])
    assert _same_planning_state(choices[1], initial)
    features = {feature.get_symbol(): feature for feature in child.module_state.module.get_query_features()}
    expected = [((_register_value(state),),) for state in choices]
    all_candidates = {row for selected in expected for row in selected}
    assert expected[0] != expected[1]

    snapshots: list[tuple[tuple[int, ...], ...]] = []
    for position in (0, 1, 0):
        state = choices[position]
        environment.reset_source()
        dl_context = make_dl_context(environment, state)
        assert {tuple(row) for row in ext.evaluate(features["argument"], dl_context)} == all_candidates
        selected = ext.evaluate(features["register"], dl_context)
        assert tuple(tuple(row) for row in selected) == expected[position]
        intersection = ext.evaluate(features["intersection"], dl_context)
        assert tuple(tuple(row) for row in intersection) == expected[position]
        snapshots.append(tuple(_ints(row) for row in selected))
        del selected, intersection
    expected_indices = [tuple(_ints(row) for row in rows) for rows in expected]
    assert snapshots == [expected_indices[0], expected_indices[1], expected_indices[0]]


@pytest.mark.parametrize("kind", ["ground", "lifted"])
def test_action_query_preserves_correlated_parameter_tuples(kind: str, tmp_path: Path) -> None:
    domain_file = tmp_path / "domain.pddl"
    task_file = tmp_path / "task.pddl"
    domain_file.write_text("""(define (domain correlated)
      (:requirements :strips :typing)
      (:types source target)
      (:predicates (allowed ?s - source ?t - target) (linked ?s - source ?t - target))
      (:action connect :parameters (?s - source ?t - target)
        :precondition (and) :effect (linked ?s ?t)))""")
    task_file.write_text("""(define (problem correlated-task) (:domain correlated)
      (:objects a b - source c d - target)
      (:init (allowed a c) (allowed b d))
      (:goal (and (linked a c) (linked a d) (linked b c) (linked b d))))""")
    parser = Parser(domain_file, ParserOptions())
    task = lifted.Task(parser.parse_task(task_file, ParserOptions()))
    execution = ExecutionContext(1)
    domain = DomainContext(parser.get_domain())
    task_context: TaskContext
    if kind == "ground":
        task_context = GroundTaskContext(
            domain, GroundTaskSearchContext(task.instantiate_ground_task(execution).task, execution)
        )
    else:
        task_context = LiftedTaskContext(domain, LiftedTaskSearchContext(task, execution))
    program = dl.parse_program(
        """(:program (:entry root)
      (:module (:symbol root) (:arguments) (:registers)
        (:entry m0) (:memory m0 m1)
        (:features (:query (:symbol pairs) (:expression (q_atomic_state "allowed" (source target)))))
        (:rules (:rule (:symbol connect-pair) (:expression
          (:source-memory m0) (:target-memory m1)
          (:action (:conditions) (:action "connect") (:query pairs) (:effects)))))))""",
        parser.get_domain(),
        domain.ext_repository,
    )
    expander = make_expander(task_context, program)
    state = initial_program_state(expander, initial_node(task_context).get_state())

    def arguments(action: ActionBinding) -> tuple[str, ...]:
        return tuple(object_.get_name() for object_ in action.get_objects())

    broad = _labeled_successors(task_context)
    assert {arguments(successor.label) for successor in broad} == {
        ("a", "c"),
        ("a", "d"),
        ("b", "c"),
        ("b", "d"),
    }
    allowed = {("a", "c"), ("b", "d")}
    steps = collect_steps(expander, state)
    assert len(steps) == 2
    assert {arguments(_action(step)) for step in steps} == allowed
    rule = program.get_entry_module().get_memory_transitions()[0][0]
    matched: set[tuple[str, ...]] = set()
    applied: set[tuple[str, ...]] = set()
    for successor in broad:
        if _matching_rule(expander, state, successor) is not None:
            matched.add(arguments(successor.label))
        result = _apply(expander, state, rule, successor)
        if result is not None:
            applied.add(arguments(_action(result)))
    assert matched == applied == allowed
