# Copyright (C) 2026 Dominik Drexler
# SPDX-License-Identifier: GPL-3.0-or-later

import gc
import sys
from dataclasses import dataclass
from datetime import timedelta
from pathlib import Path
from typing import Literal, TypeAlias

import pytest
from ext_execution_utils import (
    DlContext,
    ExecutionStep,
    Expander,
    Outcome,
    PlanningState,
    ProgramState,
    TaskContext,
    apply_choice,
    collect_steps,
    for_each_successor,
    initial_node,
    initial_program_state,
    make_dl_context,
    make_environment,
    make_expander,
)
from fixture_utils import FIXTURE_ROOT, read_fixture
from pypddl.formalism import ParserOptions
from pypddl_datasets import data_root
from pytyr.formalism.planning import Object, Parser, PlanningDomain
from pytyr.planning import ground, lifted
from pyyggdrasil.execution import ExecutionContext

from pyrunir.datasets import GroundTaskSearchContext, LiftedTaskSearchContext
from pyrunir.kr import DomainContext, GroundTaskContext, LiftedTaskContext
from pyrunir.kr.dl import semantics
from pyrunir.kr.dl.semantics import (
    BooleanDenotation,
    CallArguments,
    CallArgumentsData,
    RegisterValues,
    RegisterValuesData,
)
from pyrunir.kr.ps import ext
from pyrunir.kr.ps.ext.dl import parse_program
from pyrunir.kr.uns.dl import parse_classifier

SearchOptions: TypeAlias = ext.GroundProgramSearchOptions | ext.LiftedProgramSearchOptions
ProofResults: TypeAlias = ext.GroundProgramProofResults | ext.LiftedProgramProofResults

PROGRAM = """(:program
  (:entry root)
  (:module
    (:symbol root) (:arguments) (:registers)
    (:entry m0) (:memory m0 m1)
    (:features
      (:concept (:symbol balls) (:expression (c_atomic_state "ball")))
      (:role (:symbol locations) (:expression (r_atomic_state "at")))
      (:boolean (:symbol present)
        (:expression (b_nonempty (c_atomic_state "ball"))))
      (:numerical (:symbol count)
        (:expression (n_count (c_atomic_state "ball")))))
    (:rules
      (:rule (:symbol call) (:expression
        (:source-memory m0) (:target-memory m1)
        (:call (:conditions) (:callee child)
          (:arguments balls locations present count))))))
  (:module
    (:symbol child)
    (:arguments (:concept B) (:role A) (:boolean flag) (:numerical count))
    (:registers (:concept selected) (:role location))
    (:entry m0) (:memory m0 m1 m2)
    (:features
      (:concept (:symbol balls) (:expression (c_argument B)))
      (:role (:symbol locations) (:expression (r_argument A)))
      (:boolean (:symbol argument_flag) (:expression (b_argument flag)))
      (:boolean (:symbol concept_match)
        (:expression (b_nonempty (c_and (c_argument B) (c_register selected)))))
      (:boolean (:symbol role_match)
        (:expression (b_nonempty (r_and (r_argument A) (r_register location)))))
      (:boolean (:symbol selected_goal_ball)
        (:expression (b_nonempty (c_and (c_register selected)
          (c_some (r_atomic_goal "at" true) (c_top))))))
      (:numerical (:symbol argument_count) (:expression (n_argument count)))
      (:numerical (:symbol argument_size) (:expression (n_count (c_argument B))))
      (:numerical (:symbol concept_register_size)
        (:expression (n_count (c_register selected))))
      (:numerical (:symbol role_register_size)
        (:expression (n_count (r_register location))))
      (:numerical (:symbol nearby_balls)
        (:expression (n_count (c_and (c_argument B)
          (c_some (r_atomic_state "at") (c_atomic_state "at-robby")))))))
    (:rules
      (:rule (:symbol load_ball) (:expression
        (:source-memory m0) (:target-memory m1)
        (:load (:conditions) (:concept balls) (:register (:concept selected)))))
      (:rule (:symbol load_location) (:expression
        (:source-memory m1) (:target-memory m2)
        (:load (:conditions) (:role locations) (:register (:role location)))))
      (:rule (:symbol move_away) (:expression
        (:source-memory m2) (:target-memory m2)
        (:sketch (:conditions) (:effects (decreases nearby_balls))))))))
"""


def _state_context(
    state: PlanningState,
    builder: semantics.Builder,
    storage: semantics.EvaluationStorage,
    arguments: CallArguments,
    registers: RegisterValues,
) -> DlContext:
    if isinstance(state, ground.State):
        return semantics.GroundStateEvaluationContext(state, builder, storage, arguments, registers)
    return semantics.LiftedStateEvaluationContext(state, builder, storage, arguments, registers)


def _same_state(first: PlanningState, second: PlanningState) -> bool:
    if isinstance(first, ground.State):
        assert isinstance(second, ground.State)
        return first == second
    assert isinstance(second, lifted.State)
    return first == second


def _options(kind: Literal["ground", "lifted"]) -> SearchOptions:
    return ext.GroundProgramSearchOptions() if kind == "ground" else ext.LiftedProgramSearchOptions()


def _find_solution(task_context: TaskContext, program: ext.Program, options: SearchOptions) -> ProofResults:
    if isinstance(task_context, GroundTaskContext):
        assert isinstance(options, ext.GroundProgramSearchOptions)
        return ext.find_ground_solution(task_context, program, options)
    assert isinstance(options, ext.LiftedProgramSearchOptions)
    return ext.find_lifted_solution(task_context, program, options)


def _object(value: object) -> Object:
    assert isinstance(value, Object)
    return value


def _object_pair(value: object) -> tuple[Object, Object]:
    match value:
        case (Object() as first, Object() as second):
            return first, second
        case _:
            raise AssertionError(value)


@dataclass(frozen=True)
class OrderCase:
    """A Choose order with the expected search statistics."""

    order: str
    expected: int
    depth: int
    width: int


def _task_context(
    kind: Literal["ground", "lifted"],
) -> tuple[GroundTaskContext | LiftedTaskContext, PlanningDomain]:
    directory = data_root() / "classical" / "tests" / "gripper"
    parser = Parser(directory / "domain.pddl", ParserOptions())
    task = lifted.Task(parser.parse_task(directory / "test-1.pddl", ParserOptions()))
    execution = ExecutionContext(1)
    domain = DomainContext(parser.get_domain())
    if kind == "ground":
        return GroundTaskContext(
            domain,
            GroundTaskSearchContext(task.instantiate_ground_task(execution).task, execution),
        ), parser.get_domain()
    return LiftedTaskContext(domain, LiftedTaskSearchContext(task, execution)), parser.get_domain()


def _loaded_frame(
    kind: Literal["ground", "lifted"], source: str = PROGRAM
) -> tuple[TaskContext, ext.Program, Expander, ProgramState]:
    task_context, domain = _task_context(kind)
    program = parse_program(
        source,
        domain,
        task_context.domain_context.ext_repository,
    )
    expander = make_expander(task_context, program)
    initial = initial_program_state(expander, initial_node(task_context).get_state())
    child = collect_steps(expander, initial)[0].target
    with_concept = collect_steps(expander, child)[0].target
    loaded = collect_steps(expander, with_concept)[0].target
    return task_context, program, expander, loaded


@pytest.mark.parametrize("kind", ["ground", "lifted"])
@pytest.mark.parametrize(
    "value_kind", ["concept_register", "role_register", "argument", "concept_iterator", "role_iterator"]
)
def test_nested_borrowed_values_retain_owner_after_container_scope(
    kind: Literal["ground", "lifted"], value_kind: str
) -> None:
    def extract() -> tuple[Object | BooleanDenotation, str | bool]:
        _task, _program, _expander, loaded = _loaded_frame(kind)
        if value_kind.endswith("register"):
            registers = loaded.module_state.registers
            before = sys.getrefcount(registers)
            if value_kind == "concept_register":
                value = next(value for value in registers.concept_values if value is not None)
            else:
                value = next(value for value in registers.role_values if value is not None)[0]
            assert sys.getrefcount(registers) > before
            return value, value.get_name()
        arguments = loaded.module_state.arguments
        if value_kind == "argument":
            before = sys.getrefcount(arguments)
            boolean = arguments.boolean_arguments[0]
            assert sys.getrefcount(arguments) > before
            return boolean, boolean.get()
        if value_kind == "concept_iterator":
            concepts = arguments.concept_arguments[0]
            before = sys.getrefcount(concepts)
            concept_iterator = iter(concepts)
            value = next(concept_iterator)
            del concept_iterator
            assert sys.getrefcount(concepts) > before
            return value, value.get_name()
        roles = arguments.role_arguments[0]
        before = sys.getrefcount(roles)
        role_iterator = iter(roles)
        value = next(role_iterator)[0]
        del role_iterator
        assert sys.getrefcount(roles) > before
        return value, value.get_name()

    value, expected = extract()
    gc.collect()
    assert (value.get() if isinstance(value, BooleanDenotation) else value.get_name()) == expected


def test_call_rule_arguments_preserve_feature_views_and_order() -> None:
    _task_context, program, _expander, _loaded = _loaded_frame("lifted")
    module = program.get_entry_module()
    rule = module.get_memory_transitions()[0][0].get_variant()
    assert isinstance(rule, ext.CallRule)
    arguments = rule.get_call_arguments()
    assert arguments == [
        module.get_concept_features()[0],
        module.get_role_features()[0],
        module.get_boolean_features()[0],
        module.get_numerical_features()[0],
    ]


@pytest.mark.parametrize("kind", ["ground", "lifted"])
def test_module_state_and_suspended_caller_preserve_fields_and_dependencies(kind: Literal["ground", "lifted"]) -> None:
    task_context, program, expander, loaded = _loaded_frame(kind)
    references = sys.getrefcount(loaded)
    module_state = loaded.module_state
    assert isinstance(module_state, ext.GroundModuleState if kind == "ground" else ext.LiftedModuleState)
    assert sys.getrefcount(loaded) > references
    assert len({module_state, loaded.module_state}) == 1
    assert _same_state(module_state.state, loaded.state)
    assert module_state.memory_state.get_name() == "m2"
    caller = loaded.call_stack
    assert isinstance(caller, ext.GroundCallStack if kind == "ground" else ext.LiftedCallStack)
    assert caller.module == program.get_entry_module()
    assert caller.return_memory_state.get_name() == "m1"
    assert caller.caller is None
    assert not caller.has_caller
    assert caller.registers.concept_values == []
    assert caller.arguments.concept_arguments == []
    del task_context, program, expander, loaded
    gc.collect()
    assert len(module_state.arguments.concept_arguments) == 1
    assert module_state.registers.concept_values[0] is not None
    assert module_state.registers.role_values[0] is not None
    assert caller.return_memory_state.get_name() == "m1"


@pytest.mark.parametrize("kind", ["ground", "lifted"])
def test_evaluation_restores_arguments_registers_and_owns_dependencies(
    kind: Literal["ground", "lifted"],
) -> None:
    task_context, program, expander, loaded = _loaded_frame(kind)
    frame = loaded.module_state
    assert len(frame.arguments.concept_arguments) == 1
    assert len(frame.arguments.role_arguments) == 1
    assert len(frame.arguments.boolean_arguments) == 1
    assert len(frame.arguments.numerical_arguments) == 1
    assert frame.registers.concept_values[0] is not None
    assert frame.registers.role_values[0] is not None

    environment = make_environment(task_context, program)
    references = sys.getrefcount(environment)
    storage = environment.get_dl_caches()
    target_storage = environment.get_dl_target_caches()
    assert storage is not target_storage
    assert sys.getrefcount(environment) > references
    del storage, target_storage
    dl_context = make_dl_context(environment, loaded)
    booleans = {feature.get_symbol(): feature for feature in frame.module.get_boolean_features()}
    numericals = {feature.get_symbol(): feature for feature in frame.module.get_numerical_features()}
    expected = {
        "argument_count": 2,
        "argument_size": 2,
        "concept_register_size": 1,
        "role_register_size": 1,
        "nearby_balls": 2,
    }
    expected_booleans = {
        "argument_flag": True,
        "concept_match": True,
        "role_match": True,
        "selected_goal_ball": False,
    }
    assert {name: ext.evaluate(feature, dl_context).get() for name, feature in booleans.items()} == expected_booleans
    assert {name: ext.evaluate(feature, dl_context).get() for name, feature in numericals.items()} == expected
    concept = ext.evaluate(frame.module.get_concept_features()[0], dl_context)
    role = ext.evaluate(frame.module.get_role_features()[0], dl_context)
    boolean = ext.evaluate(booleans["argument_flag"], dl_context)
    numerical = ext.evaluate(numericals["argument_count"], dl_context)
    assert concept == frame.arguments.concept_arguments[0]
    assert role == frame.arguments.role_arguments[0]
    assert boolean == frame.arguments.boolean_arguments[0]
    assert numerical == frame.arguments.numerical_arguments[0]
    child = collect_steps(expander, initial_program_state(expander, initial_node(task_context).get_state()))[0].target
    other_concept = collect_steps(expander, child)[1].target
    other_frame = collect_steps(expander, other_concept)[0].target
    assert _same_state(other_frame.state, loaded.state)
    environment.reset_source()
    other_dl_context = make_dl_context(environment, other_frame)
    assert ext.evaluate(booleans["selected_goal_ball"], other_dl_context).get() is True
    environment.reset_source()
    assert ext.evaluate(booleans["selected_goal_ball"], dl_context).get() is False
    del child, other_concept, other_frame, other_dl_context

    def moves_to_roomb(step: ExecutionStep) -> bool:
        transition = step.state_transition
        assert transition is not None
        action = transition.action
        return action.get_relation().get_name() == "move" and action.get_objects()[-1].get_name() == "roomb"

    moved = next(step.target for step in collect_steps(expander, loaded) if moves_to_roomb(step))
    environment.reset_source()
    dl_context = make_dl_context(environment, moved)
    assert ext.evaluate(numericals["nearby_balls"], dl_context).get() == 0
    assert ext.evaluate(numericals["argument_count"], dl_context).get() == 2
    assert {name: ext.evaluate(feature, dl_context).get() for name, feature in booleans.items()} == expected_booleans
    environment.reset_source()
    dl_context = make_dl_context(environment, loaded)
    assert ext.evaluate(numericals["nearby_balls"], dl_context).get() == 2

    del task_context, program, expander, loaded, frame
    del moved
    gc.collect()
    assert {name: ext.evaluate(feature, dl_context).get() for name, feature in numericals.items()} == expected
    assert {name: ext.evaluate(feature, dl_context).get() for name, feature in booleans.items()} == expected_booleans
    del dl_context, environment, booleans, numericals
    gc.collect()
    assert {obj.get_name() for obj in concept} == {"ball1", "ball2"}
    assert {(first.get_name(), second.get_name()) for first, second in role} == {("ball1", "rooma"), ("ball2", "rooma")}
    assert boolean.get() is True
    assert numerical.get() == 2


@pytest.mark.parametrize("kind", ["ground", "lifted"])
def test_state_evaluation_context_retains_program_state_and_environment(
    kind: Literal["ground", "lifted"],
) -> None:
    task_context, program, expander, loaded = _loaded_frame(kind)
    environment = make_environment(task_context, program)
    state_references = sys.getrefcount(loaded)
    environment_references = sys.getrefcount(environment)

    context = make_dl_context(environment, loaded)
    assert sys.getrefcount(loaded) > state_references
    assert sys.getrefcount(environment) > environment_references
    boolean = loaded.module_state.module.get_boolean_features()[0]
    numerical = loaded.module_state.module.get_numerical_features()[0]
    del task_context, program, expander, loaded, environment
    gc.collect()
    assert ext.evaluate(boolean, context).get() is True
    assert ext.evaluate(numerical, context).get() == 2


@pytest.mark.parametrize("kind", ["ground", "lifted"])
def test_state_evaluation_contexts_borrow_distinct_call_arguments(
    kind: Literal["ground", "lifted"],
) -> None:
    task_context, program, expander, loaded = _loaded_frame(kind)
    initial = initial_program_state(expander, initial_node(task_context).get_state())
    environment = make_environment(task_context, program)
    contexts = [
        make_dl_context(environment, loaded),
        make_dl_context(environment, initial),
    ]
    features = [
        loaded.module_state.module.get_numerical_features()[0],
        initial.module_state.module.get_numerical_features()[0],
    ]
    del loaded, initial
    gc.collect()
    for position in (0, 1, 0):
        environment.reset_source()
        assert ext.evaluate(features[position], contexts[position]).get() == 2


@pytest.mark.parametrize("kind", ["ground", "lifted"])
def test_more_than_four_registers_and_interned_binding_views(kind: Literal["ground", "lifted"]) -> None:
    declarations = " ".join(f"(:concept spare{i}) (:role spare_role{i})" for i in range(5))
    source = PROGRAM.replace(
        "(:registers (:concept selected) (:role location))",
        f"(:registers {declarations} (:concept selected) (:role location))",
    )
    task_context, program, _expander, loaded = _loaded_frame(kind, source)
    concept_values = loaded.module_state.registers.concept_values
    role_values = loaded.module_state.registers.role_values
    assert len(concept_values) == len(role_values) == 6
    assert concept_values[:5] == role_values[:5] == [None] * 5
    assert concept_values[5] is not None
    assert role_values[5] is not None
    features = {feature.get_symbol(): feature for feature in loaded.module_state.module.get_numerical_features()}
    environment = make_environment(task_context, program)
    context = make_dl_context(environment, loaded)
    assert ext.evaluate(features["concept_register_size"], context).get() == 1
    assert ext.evaluate(features["role_register_size"], context).get() == 1

    arguments = CallArgumentsData()
    module_arguments = loaded.module_state.arguments
    arguments.concept_arguments = [value.get_index() for value in module_arguments.concept_arguments]
    arguments.role_arguments = [value.get_index() for value in module_arguments.role_arguments]
    arguments.boolean_arguments = [value.get_index() for value in module_arguments.boolean_arguments]
    arguments.numerical_arguments = [value.get_index() for value in module_arguments.numerical_arguments]
    registers = RegisterValuesData()
    registers.concept_values = [None] * 5 + [concept_values[5].get_index()]
    first, second = role_values[5]
    expected_roles = [None] * 5 + [(first.get_index(), second.get_index())]
    registers.role_values = expected_roles
    assert registers.role_values == expected_roles
    repository = task_context.dl_denotation_repository
    argument_view = repository.insert(arguments)[0]
    register_view = repository.insert(registers)[0]
    assert isinstance(argument_view, CallArguments)
    assert isinstance(register_view, RegisterValues)
    assert repository.insert(arguments)[0] == argument_view == loaded.module_state.arguments
    assert repository.insert(registers)[0] == register_view == loaded.module_state.registers
    assert len({argument_view, loaded.module_state.arguments}) == 1
    assert len({register_view, loaded.module_state.registers}) == 1
    storage = semantics.EvaluationStorage(repository)
    context = _state_context(loaded.state, task_context.dl_builder, storage, argument_view, register_view)
    assert ext.evaluate(features["argument_count"], context).get() == 2
    assert ext.evaluate(features["concept_register_size"], context).get() == 1
    assert ext.evaluate(features["role_register_size"], context).get() == 1
    registers.concept_values = [None] * 6
    registers.role_values = [None] * 6
    empty_register_view = repository.insert(registers)[0]
    assert empty_register_view != register_view
    storage.reset_dynamic()
    assert ext.evaluate(features["concept_register_size"], context).get() == 1
    assert ext.evaluate(features["role_register_size"], context).get() == 1
    context = _state_context(loaded.state, task_context.dl_builder, storage, argument_view, empty_register_view)
    storage.reset_dynamic()
    assert ext.evaluate(features["concept_register_size"], context).get() == 0
    assert ext.evaluate(features["role_register_size"], context).get() == 0
    del arguments, registers, argument_view, register_view, empty_register_view
    gc.collect()
    storage.reset_dynamic()
    assert ext.evaluate(features["argument_count"], context).get() == 2


@pytest.mark.parametrize("kind", ["ground", "lifted"])
def test_choice_callbacks_filter_effects_and_keep_independent_cursors(kind: Literal["ground", "lifted"]) -> None:
    task_context, domain = _task_context(kind)
    source = (
        PROGRAM.replace("(:load", "(:choose")
        .replace(
            "(:register (:concept selected))",
            "(:register (:concept selected)) (:effects (increases concept_register_size))",
        )
        .replace(
            "(:register (:role location))",
            "(:register (:role location)) (:effects (increases role_register_size))",
        )
    )
    program = parse_program(source, domain, task_context.domain_context.ext_repository)
    expander = make_expander(task_context, program)
    child = collect_steps(expander, initial_program_state(expander, initial_node(task_context).get_state()))[0].target
    statistics = ext.ProgramSearchStatistics()

    def bindings(state: ProgramState, choice_type: type[ext.ConceptChoice | ext.RoleChoice]) -> list[ExecutionStep]:
        choices: list[Outcome] = []

        def emit(choice: Outcome) -> bool:
            choices.append(choice)
            return True

        generated = statistics.num_generated
        assert for_each_successor(expander, state, statistics, emit)
        assert statistics.num_generated == generated
        (choice,) = choices
        assert isinstance(choice, choice_type)
        assert choice.rule is not None
        assert choice.count() == 2
        expected = choice.bindings
        result: list[ExecutionStep] = []
        for value in expected:
            assert not choice.exhausted()
            assert choice.current() == value
            result.append(apply_choice(expander, state, choice, statistics))
            choice.advance()
        assert choice.exhausted()
        assert choice.bindings == expected
        assert statistics.num_generated == generated + len(result)
        with pytest.raises(IndexError):
            choice.current()
        with pytest.raises(IndexError):
            choice.advance()
        return result

    concept_steps = bindings(child, ext.ConceptChoice)
    assert len(concept_steps) == 2
    assert child.module_state.registers.concept_values[0] is None
    selected: set[str] = set()
    for step in concept_steps:
        value = step.target.module_state.registers.concept_values[0]
        assert value is not None
        selected.add(value.get_name())
    assert selected == {"ball1", "ball2"}
    for step in concept_steps:
        role_steps = bindings(step.target, ext.RoleChoice)
        assert len(role_steps) == 2
        assert step.target.module_state.registers.role_values[0] is None
        for role_step in role_steps:
            assert _same_state(role_step.target.state, child.state)
            assert role_step.target.module_state.registers.role_values[0] is not None


@pytest.mark.parametrize("kind", ["ground", "lifted"])
def test_choice_bindings_keep_pool_and_repositories_alive(kind: Literal["ground", "lifted"]) -> None:
    task_context, domain = _task_context(kind)
    program = parse_program(PROGRAM.replace("(:load", "(:choose"), domain, task_context.domain_context.ext_repository)
    expander = make_expander(task_context, program)
    child = collect_steps(expander, initial_program_state(expander, initial_node(task_context).get_state()))[0].target
    statistics = ext.ProgramSearchStatistics()
    choices: list[Outcome] = []

    def emit(choice: Outcome) -> bool:
        choices.append(choice)
        return True

    references = sys.getrefcount(expander)
    for_each_successor(expander, child, statistics, emit)
    (concept_choice,) = choices
    assert isinstance(concept_choice, ext.ConceptChoice)
    assert sys.getrefcount(expander) > references
    with_concept = apply_choice(expander, child, concept_choice, statistics).target
    choices.clear()
    for_each_successor(expander, with_concept, statistics, emit)
    (role_choice,) = choices
    assert isinstance(role_choice, ext.RoleChoice)
    concept = _object(concept_choice.current())
    role = _object_pair(role_choice.current())
    concepts = concept_choice.bindings
    roles = role_choice.bindings
    names = concept.get_name(), tuple(value.get_name() for value in role)

    choices.clear()
    del expander, with_concept, child, program, task_context, domain
    gc.collect()
    assert concept_choice.current() == concept
    assert role_choice.current() == role
    del concept_choice, role_choice
    gc.collect()
    assert (concept.get_name(), tuple(value.get_name() for value in role)) == names
    assert concept in concepts
    assert role in roles


@pytest.mark.parametrize("kind", ["ground", "lifted"])
def test_state_memorization_options(kind: Literal["ground", "lifted"]) -> None:
    options = _options(kind)
    assert options.state_memorization == ext.StateMemorization.ALL
    for mode in (ext.StateMemorization.NONE, ext.StateMemorization.CHOICE, ext.StateMemorization.ALL):
        options.state_memorization = mode
        assert options.state_memorization == mode


@pytest.mark.parametrize("kind", ["ground", "lifted"])
@pytest.mark.parametrize("mode", [ext.StateMemorization.NONE, ext.StateMemorization.CHOICE, ext.StateMemorization.ALL])
def test_state_memorization_preserves_axiom_closure(
    tmp_path: Path, kind: Literal["ground", "lifted"], mode: ext.StateMemorization
) -> None:
    domain_source = read_fixture("kr/ps/ext/choose/domain.pddl")
    domain_source = domain_source.replace(":requirements :strips", ":requirements :strips :derived-predicates")
    domain_source = domain_source.replace("(bad ?x)", "(bad ?x) (reached ?x)")
    domain_source = domain_source.rstrip()[:-1] + "\n(:derived (reached ?x) (at ?x)))\n"
    domain_path = tmp_path / "domain.pddl"
    domain_path.write_text(domain_source)
    task_path = tmp_path / "task.pddl"
    task_path.write_text(
        read_fixture("kr/ps/ext/choose/task.pddl").replace("(:goal (at goal))", "(:goal (reached goal))")
    )
    parser = Parser(domain_path, ParserOptions())
    task = lifted.Task(parser.parse_task(task_path, ParserOptions()))
    execution = ExecutionContext(1)
    domain = DomainContext(parser.get_domain())
    if kind == "ground":
        context = GroundTaskContext(
            domain, GroundTaskSearchContext(task.instantiate_ground_task(execution).task, execution)
        )
    else:
        context = LiftedTaskContext(domain, LiftedTaskSearchContext(task, execution))
    program = parse_program(
        """(:program (:entry main)
      (:module (:symbol main) (:arguments) (:registers (:concept selected))
        (:entry m0) (:memory m0 m1 m2 m3)
        (:features
          (:concept (:symbol candidates) (:expression (c_atomic_state "candidate")))
          (:concept (:symbol here) (:expression (c_atomic_state "reached")))
          (:concept (:symbol goal) (:expression (c_atomic_goal "reached" true)))
          (:concept (:symbol target) (:expression (c_register selected))))
        (:rules
          (:rule (:symbol select) (:expression (:source-memory m0) (:target-memory m1)
            (:choose (:conditions) (:concept candidates) (:register (:concept selected)))))
          (:rule (:symbol move) (:expression (:source-memory m1) (:target-memory m2)
            (:do (:conditions) (:action "move") (:arguments here target) (:effects))))
          (:rule (:symbol finish) (:expression (:source-memory m2) (:target-memory m3)
            (:do (:conditions) (:action "move") (:arguments here goal) (:effects)))))))""",
        parser.get_domain(),
        domain.ext_repository,
    )
    options = _options(kind)
    options.state_memorization = mode
    options.classifier = parse_classifier(
        """(:classifier (:symbol bad-location)
      (:features (:boolean (:symbol bad) (:expression
        (b_nonempty (c_and (c_atomic_state "reached") (c_atomic_state "bad"))))))
      (:expression (or (and bad))))""",
        parser.get_domain(),
        domain.uns_repository,
    )
    result = _find_solution(context, program, options)
    assert result.is_successful()
    assert result.plan is not None
    assert result.plan.get_length() == 2
    assert result.statistics.choice_depth == 1
    assert result.statistics.choice_width == 2
    assert bool(result.deadend_states) == (mode == ext.StateMemorization.ALL)


@pytest.mark.parametrize("kind", ["ground", "lifted"])
@pytest.mark.parametrize("universal", [False, True])
@pytest.mark.parametrize("program_file", ["non_terminating.program", "recursive_calls.program"])
def test_execution_requires_whole_program_structural_termination(
    kind: Literal["ground", "lifted"], universal: bool, program_file: str
) -> None:
    context, domain = _task_context(kind)
    program = parse_program(
        read_fixture(f"kr/ps/ext/dl/{program_file}"),
        domain,
        context.domain_context.ext_repository,
    )
    options = _options(kind)
    options.universal = universal
    options.max_time = timedelta(0)
    with pytest.raises(ValueError, match=r"^Ext find_solution requires a structurally terminating program\.$"):
        _find_solution(context, program, options)


@pytest.mark.parametrize("kind", ["ground", "lifted"])
@pytest.mark.parametrize("universal", [False, True])
@pytest.mark.parametrize("initial_goal_satisfied", [False, True])
def test_initial_goal_classifier_and_zero_time_results(
    tmp_path: Path,
    kind: Literal["ground", "lifted"],
    universal: bool,
    initial_goal_satisfied: bool,
) -> None:
    directory = FIXTURE_ROOT / "kr/ps/ext/choose"
    # Grounding rejects false static goals; exercise that case directly only in lifted mode.
    goal = "(at goal)" if kind == "ground" else "(and (at start) (bad start))"
    if initial_goal_satisfied:
        goal = "(at start)"
    task_path = tmp_path / "task.pddl"
    task_path.write_text(read_fixture("kr/ps/ext/choose/task.pddl").replace("(:goal (at goal))", f"(:goal {goal})"))
    parser = Parser(directory / "domain.pddl", ParserOptions())
    task = lifted.Task(parser.parse_task(task_path, ParserOptions()))
    execution = ExecutionContext(1)
    domain = DomainContext(parser.get_domain())
    if kind == "ground":
        context = GroundTaskContext(
            domain,
            GroundTaskSearchContext(task.instantiate_ground_task(execution).task, execution),
        )
    else:
        context = LiftedTaskContext(domain, LiftedTaskSearchContext(task, execution))
    program = parse_program(
        read_fixture("kr/ps/ext/execution/empty.program"),
        parser.get_domain(),
        domain.ext_repository,
    )
    options = _options(kind)
    options.universal = universal
    options.classifier = parse_classifier(
        read_fixture("kr/uns/always.classifier"),
        parser.get_domain(),
        domain.uns_repository,
    )
    result = _find_solution(context, program, options)
    assert result.status == (
        ext.ProgramProofStatus.SUCCESS if initial_goal_satisfied else ext.ProgramProofStatus.FAILURE
    )
    assert result.deadend_states == ([] if initial_goal_satisfied else [0])
    if initial_goal_satisfied and not universal:
        assert result.plan is not None
        assert result.plan.empty()
        start = result.plan.get_start_node()
        initial = initial_node(context).pack()
        if isinstance(start, ground.PackedNode):
            assert isinstance(initial, ground.PackedNode)
            assert start == initial
        else:
            assert isinstance(initial, lifted.PackedNode)
            assert start == initial
    else:
        assert result.plan is None

    options.max_time = timedelta(0)
    expired = _find_solution(context, program, options)
    assert expired.status == ext.ProgramProofStatus.OUT_OF_TIME
    assert expired.plan is None
    assert expired.deadend_states == []
    for proof in (result, expired):
        assert proof.statistics.num_expanded == 0
        assert proof.statistics.num_generated == 0
        assert proof.statistics.choice_depth == 0
        assert proof.statistics.choice_width == 0
        assert proof.open_states == []
        assert proof.graph.get_num_vertices() == 1
        assert proof.graph.get_num_edges() == 0
        label = proof.graph.get_vertex_property(0)
        assert label.is_initial
        assert label.is_goal == initial_goal_satisfied
        assert label.is_unsolvable == (not initial_goal_satisfied)


@pytest.mark.parametrize("kind", ["ground", "lifted"])
@pytest.mark.parametrize("universal", [False, True])
@pytest.mark.parametrize("mode", [ext.StateMemorization.NONE, ext.StateMemorization.CHOICE, ext.StateMemorization.ALL])
def test_choose_search_statistics_count_choice_depth_and_width(
    kind: Literal["ground", "lifted"],
    universal: bool,
    mode: ext.StateMemorization,
) -> None:
    assert ext.ProgramSearchStatistics().choice_width == 0
    directory = FIXTURE_ROOT / "kr/ps/ext/choose"
    parser = Parser(directory / "domain.pddl", ParserOptions())
    task = lifted.Task(parser.parse_task(directory / "task.pddl", ParserOptions()))
    execution = ExecutionContext(1)
    domain = DomainContext(parser.get_domain())
    if kind == "ground":
        context = GroundTaskContext(
            domain,
            GroundTaskSearchContext(task.instantiate_ground_task(execution).task, execution),
        )
    else:
        context = LiftedTaskContext(domain, LiftedTaskSearchContext(task, execution))
    options = _options(kind)
    program = parse_program(
        """(:program (:entry search)
          (:module (:symbol search) (:arguments) (:registers (:concept selected))
            (:entry m0) (:memory m0 m1 m2 m3)
            (:features
              (:concept (:symbol candidates) (:expression (c_atomic_state "candidate")))
              (:concept (:symbol here) (:expression (c_atomic_state "at")))
              (:concept (:symbol goal) (:expression (c_atomic_goal "at" true)))
              (:concept (:symbol target) (:expression (c_register selected))))
            (:rules
              (:rule (:symbol select) (:expression (:source-memory m0) (:target-memory m1)
                (:choose (:conditions) (:concept candidates) (:register (:concept selected)))))
              (:rule (:symbol move-selected) (:expression (:source-memory m1) (:target-memory m2)
                (:do (:conditions) (:action "move") (:arguments here target) (:effects))))
              (:rule (:symbol finish) (:expression (:source-memory m2) (:target-memory m3)
                (:do (:conditions) (:action "move") (:arguments here goal) (:effects)))))))""",
        parser.get_domain(),
        context.domain_context.ext_repository,
    )
    options.universal = universal
    options.state_memorization = mode
    result = _find_solution(context, program, options)
    assert result.is_successful()
    assert isinstance(result.statistics, ext.ProgramSearchStatistics)
    assert result.statistics.num_expanded == 5
    assert result.statistics.num_generated == 5
    assert result.statistics.choice_depth == 1
    assert result.statistics.choice_width == 2


@pytest.mark.parametrize("kind", ["ground", "lifted"])
@pytest.mark.parametrize("universal", [False, True])
@pytest.mark.parametrize(
    "case",
    [
        OrderCase("(:order (min score))", 3, 1, 2),
        OrderCase("(:order (max score))", 5, 1, 2),
        OrderCase("(:order (min infinite_score))", 3, 1, 2),
        OrderCase("(:order (max infinite_score))", 5, 1, 2),
        OrderCase("(:order (min bad_flag) (max score))", 3, 1, 2),
        OrderCase("(:order)", 5, 1, 2),
        OrderCase("(:order (min zero))", 5, 1, 2),
        OrderCase("(:effects (negative bad_flag)) (:order (max score))", 3, 0, 1),
    ],
    ids=lambda case: f"{case.order}-{case.expected}-{case.depth}-{case.width}",
)
@pytest.mark.parametrize("category", ["concept", "role"])
def test_choose_order_scores_bound_candidates(
    kind: Literal["ground", "lifted"], universal: bool, case: OrderCase, category: str
) -> None:
    directory = FIXTURE_ROOT / "kr/ps/ext/choose"
    parser = Parser(directory / "domain.pddl", ParserOptions())
    task = lifted.Task(parser.parse_task(directory / "task.pddl", ParserOptions()))
    execution = ExecutionContext(1)
    domain = DomainContext(parser.get_domain())
    if kind == "ground":
        context = GroundTaskContext(
            domain,
            GroundTaskSearchContext(task.instantiate_ground_task(execution).task, execution),
        )
    else:
        context = LiftedTaskContext(domain, LiftedTaskSearchContext(task, execution))
    options = _options(kind)
    candidates = '(c_atomic_state "candidate")'
    target = "(c_register selected)"
    if category == "role":
        candidates = f"(r_identity {candidates})"
        target = "(c_some (r_register selected) (c_top))"
    bad_count = f'(n_count (c_and {target} (c_atomic_state "bad")))'
    program = parse_program(
        f"""(:program (:entry search)
          (:module (:symbol search) (:arguments) (:registers (:{category} selected))
            (:entry m0) (:memory m0 m1 m2 m3)
            (:features
              (:{category} (:symbol candidates) (:expression {candidates}))
              (:concept (:symbol here) (:expression (c_atomic_state "at")))
              (:concept (:symbol goal) (:expression (c_atomic_goal "at" true)))
              (:concept (:symbol target) (:expression {target}))
              (:numerical (:symbol zero) (:expression (n_const 0)))
              (:boolean (:symbol bad_flag) (:expression (b_nonempty (c_and {target} (c_atomic_state "bad")))))
              (:numerical (:symbol infinite_score) (:expression (n_div (n_const 1) (n_sub (n_const 1) {bad_count}))))
              (:numerical (:symbol score)
                (:expression (n_add (n_count (c_and {target} (c_atomic_state "bad"))) (n_const 0)))))
            (:rules
              (:rule (:symbol select) (:expression (:source-memory m0) (:target-memory m1)
                (:choose (:conditions) (:{category} candidates) (:register (:{category} selected)) {case.order})))
              (:rule (:symbol move-selected) (:expression (:source-memory m1) (:target-memory m2)
                (:do (:conditions) (:action "move") (:arguments here target) (:effects))))
              (:rule (:symbol finish) (:expression (:source-memory m2) (:target-memory m3)
                (:do (:conditions) (:action "move") (:arguments here goal) (:effects)))))))""",
        parser.get_domain(),
        context.domain_context.ext_repository,
    )
    options.universal = universal
    result = _find_solution(context, program, options)
    assert result.is_successful()
    assert isinstance(result.statistics, ext.ProgramSearchStatistics)
    assert result.statistics.num_expanded == case.expected
    assert result.statistics.num_generated == case.expected
    assert result.statistics.choice_depth == case.depth
    assert result.statistics.choice_width == case.width


@pytest.mark.parametrize("kind", ["ground", "lifted"])
def test_ext_arithmetic_values_and_round_trip(kind: Literal["ground", "lifted"]) -> None:
    cases = [
        ("(n_const 7)", 7),
        ("(n_add (n_const 4) (n_const 5))", 9),
        ("(n_sub (n_const 2) (n_const 5))", 0),
        ("(n_mul (n_const 4) (n_const 5))", 20),
        ("(n_div (n_const 9) (n_const 2))", 4),
        ("(n_min (n_const 4) (n_const 5))", 4),
        ("(n_max (n_const 4) (n_const 5))", 5),
        ("(n_mul (n_const 0) (n_div (n_const 1) (n_const 0)))", 0),
        ("(n_div (n_const 9) (n_div (n_const 1) (n_const 0)))", 0),
    ]
    task_context, domain = _task_context(kind)
    features = "\n".join(
        f"(:numerical (:symbol f{i}) (:expression {expression}))" for i, (expression, _) in enumerate(cases)
    )
    source = (
        "(:program (:entry root) (:module (:symbol root) (:arguments) (:registers) (:entry m) (:memory m)"
        f" (:features {features}) (:rules)))"
    )
    repository = task_context.domain_context.ext_repository
    program = parse_program(source, domain, repository)
    assert str(parse_program(str(program), domain, repository)) == str(program)
    expander = make_expander(task_context, program)
    state = initial_program_state(expander, initial_node(task_context).get_state())
    context = make_dl_context(make_environment(task_context, program), state)
    values = {
        f.get_symbol(): ext.evaluate(f, context).get() for f in program.get_entry_module().get_numerical_features()
    }
    assert values == {f"f{i}": expected for i, (_, expected) in enumerate(cases)}
