import gc
import sys
from typing import Literal

import pytest
from ext_execution_utils import collect_steps, initial_node
from fixture_utils import FIXTURE_ROOT
from pypddl.formalism import ParserOptions
from pypddl_datasets import data_root
from pyrunir.datasets import GroundTaskSearchContext, LiftedTaskSearchContext
from pyrunir.kr import DomainContext, GroundTaskContext, LiftedTaskContext
from pyrunir.kr.dl.base.semantics import CallArguments, CallArgumentsData, RegisterValues, RegisterValuesData
from pyrunir.kr.dl.ext import semantics
from pyrunir.kr.ps import ext
from pyrunir.kr.ps.ext.dl import parse_program
from pytyr.formalism.planning import Parser, PlanningDomain
from pytyr.planning import lifted
from pyyggdrasil.execution import ExecutionContext


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


def _loaded_frame(kind: Literal["ground", "lifted"], source: str = PROGRAM):
    task_context, domain = _task_context(kind)
    program = parse_program(
        source,
        domain,
        task_context.domain_context.ext_repository,
    )
    if kind == "ground":
        expander = ext.GroundSuccessorExpander(task_context, program)
    else:
        expander = ext.LiftedSuccessorExpander(task_context, program)
    node = initial_node(task_context)
    initial = expander.initial_state(node)
    child = collect_steps(expander, initial, node)[0].target
    with_concept = collect_steps(expander, child, node)[0].target
    loaded = collect_steps(expander, with_concept, node)[0].target
    return task_context, program, expander, loaded


def test_call_rule_arguments_preserve_feature_views_and_order() -> None:
    task_context, program, expander, loaded = _loaded_frame("lifted")
    module = program.get_entry_module()
    rule = module.get_memory_transitions()[0][0].get_variant()
    arguments = rule.get_call_arguments()
    assert isinstance(arguments, list)
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
    assert isinstance(module_state, getattr(ext, f"{kind.title()}ModuleState"))
    assert sys.getrefcount(loaded) > references
    assert len({module_state, loaded.module_state}) == 1
    assert module_state.state == loaded.state
    assert module_state.memory_state.get_name() == "m2"
    caller = loaded.call_stack
    assert isinstance(caller, getattr(ext, f"{kind.title()}CallStack"))
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
    if kind == "ground":
        environment_type = ext.GroundEvaluationEnvironment
    else:
        environment_type = ext.LiftedEvaluationEnvironment
    frame = loaded.module_state
    assert len(frame.arguments.concept_arguments) == 1
    assert len(frame.arguments.role_arguments) == 1
    assert len(frame.arguments.boolean_arguments) == 1
    assert len(frame.arguments.numerical_arguments) == 1
    assert frame.registers.concept_values[0] is not None
    assert frame.registers.role_values[0] is not None

    environment = environment_type(task_context, program)
    references = sys.getrefcount(environment)
    caches = environment.get_dl_caches()
    target_caches = environment.get_dl_target_caches()
    assert caches is not target_caches
    assert sys.getrefcount(environment) > references
    del caches, target_caches
    dl_context = environment.make_dl_context(loaded)
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
    assert {
        name: ext.evaluate(feature, dl_context).get()
        for name, feature in booleans.items()
    } == expected_booleans
    assert {
        name: ext.evaluate(feature, dl_context).get()
        for name, feature in numericals.items()
    } == expected
    concept = ext.evaluate(
        frame.module.get_concept_features()[0], dl_context
    )
    role = ext.evaluate(
        frame.module.get_role_features()[0], dl_context
    )
    boolean = ext.evaluate(
        booleans["argument_flag"], dl_context
    )
    numerical = ext.evaluate(
        numericals["argument_count"], dl_context
    )
    node = initial_node(task_context)
    child = collect_steps(expander, expander.initial_state(node), node)[0].target
    other_concept = collect_steps(expander, child, node)[1].target
    other_frame = collect_steps(expander, other_concept, node)[0].target
    assert other_frame.state == loaded.state
    environment.get_dl_caches().clear(False)
    other_dl_context = environment.make_dl_context(other_frame)
    assert ext.evaluate(booleans["selected_goal_ball"], other_dl_context).get() is True
    environment.get_dl_caches().clear(False)
    assert ext.evaluate(booleans["selected_goal_ball"], dl_context).get() is False
    del child, other_concept, other_frame, other_dl_context

    moved = next(
        step.target
        for step in collect_steps(expander, loaded, node)
        if step.state_transition.action.get_relation().get_name() == "move"
        and step.state_transition.action.get_objects()[-1].get_name() == "roomb"
    )
    environment.get_dl_caches().clear(False)
    dl_context = environment.make_dl_context(moved)
    assert ext.evaluate(numericals["nearby_balls"], dl_context).get() == 0
    assert ext.evaluate(numericals["argument_count"], dl_context).get() == 2
    assert {
        name: ext.evaluate(feature, dl_context).get()
        for name, feature in booleans.items()
    } == expected_booleans
    environment.get_dl_caches().clear(False)
    dl_context = environment.make_dl_context(loaded)
    assert ext.evaluate(numericals["nearby_balls"], dl_context).get() == 2

    del task_context, program, expander, loaded, frame
    del moved
    gc.collect()
    assert {
        name: ext.evaluate(feature, dl_context).get()
        for name, feature in numericals.items()
    } == expected
    assert {
        name: ext.evaluate(feature, dl_context).get()
        for name, feature in booleans.items()
    } == expected_booleans
    del dl_context, environment, booleans, numericals
    gc.collect()
    assert {obj.get_name() for obj in concept} == {"ball1", "ball2"}
    assert {(first.get_name(), second.get_name()) for first, second in role} == {
        ("ball1", "rooma"), ("ball2", "rooma")
    }
    assert boolean.get() is True
    assert numerical.get() == 2


@pytest.mark.parametrize("kind", ["ground", "lifted"])
def test_state_evaluation_context_retains_program_state_and_environment(
    kind: Literal["ground", "lifted"],
) -> None:
    task_context, program, expander, loaded = _loaded_frame(kind)
    environment_type = (
        ext.GroundEvaluationEnvironment if kind == "ground" else ext.LiftedEvaluationEnvironment
    )
    environment = environment_type(task_context, program)
    state_references = sys.getrefcount(loaded)
    environment_references = sys.getrefcount(environment)

    context = environment.make_dl_context(loaded)
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
    initial = expander.initial_state(initial_node(task_context))
    environment = getattr(ext, f"{kind.title()}EvaluationEnvironment")(task_context, program)
    contexts = [
        environment.make_dl_context(loaded),
        environment.make_dl_context(initial),
    ]
    features = [
        loaded.module_state.module.get_numerical_features()[0],
        initial.module_state.module.get_numerical_features()[0],
    ]
    del loaded, initial
    gc.collect()
    for position in (0, 1, 0):
        environment.get_dl_caches().clear(False)
        assert ext.evaluate(features[position], contexts[position]).get() == 2


@pytest.mark.parametrize("kind", ["ground", "lifted"])
def test_more_than_four_registers_and_interned_binding_views(kind: Literal["ground", "lifted"]) -> None:
    declarations = " ".join(f"(:concept spare{i}) (:role spare_role{i})" for i in range(5))
    source = PROGRAM.replace(
        "(:registers (:concept selected) (:role location))",
        f"(:registers {declarations} (:concept selected) (:role location))",
    )
    task_context, program, expander, loaded = _loaded_frame(kind, source)
    concept_values = loaded.module_state.registers.concept_values
    role_values = loaded.module_state.registers.role_values
    assert len(concept_values) == len(role_values) == 6
    assert concept_values[:5] == role_values[:5] == [None] * 5
    assert concept_values[5] is not None and role_values[5] is not None
    features = {feature.get_symbol(): feature for feature in loaded.module_state.module.get_numerical_features()}
    environment = getattr(ext, f"{kind.title()}EvaluationEnvironment")(task_context, program)
    context = environment.make_dl_context(loaded)
    assert ext.evaluate(features["concept_register_size"], context).get() == 1
    assert ext.evaluate(features["role_register_size"], context).get() == 1

    arguments = CallArgumentsData()
    for category in ("concept", "role", "boolean", "numerical"):
        field = f"{category}_arguments"
        setattr(arguments, field, [value.get_index() for value in getattr(loaded.module_state.arguments, field)])
    registers = RegisterValuesData()
    registers.concept_values = [None] * 5 + [concept_values[5].get_index()]
    expected_roles = [None] * 5 + [tuple(value.get_index() for value in role_values[5])]
    registers.role_values = expected_roles
    assert registers.role_values == expected_roles
    repository = task_context.dl_denotation_repository
    argument_view = repository.get_or_create(arguments)
    register_view = repository.get_or_create(registers)
    assert isinstance(argument_view, CallArguments)
    assert isinstance(register_view, RegisterValues)
    assert repository.get_or_create(arguments) == argument_view == loaded.module_state.arguments
    assert repository.get_or_create(registers) == register_view == loaded.module_state.registers
    assert len({argument_view, loaded.module_state.arguments}) == 1
    assert len({register_view, loaded.module_state.registers}) == 1
    caches = semantics.DenotationCaches()
    context = getattr(semantics, f"{kind.title()}StateEvaluationContext")(
        loaded.state, task_context.dl_builder, repository, caches, argument_view, register_view,
    )
    assert ext.evaluate(features["argument_count"], context).get() == 2
    assert ext.evaluate(features["concept_register_size"], context).get() == 1
    assert ext.evaluate(features["role_register_size"], context).get() == 1
    registers.concept_values = [None] * 6
    registers.role_values = [None] * 6
    empty_register_view = repository.get_or_create(registers)
    assert empty_register_view != register_view
    caches.clear(False)
    assert ext.evaluate(features["concept_register_size"], context).get() == 1
    assert ext.evaluate(features["role_register_size"], context).get() == 1
    context = getattr(semantics, f"{kind.title()}StateEvaluationContext")(
        loaded.state, task_context.dl_builder, repository, caches, argument_view, empty_register_view,
    )
    caches.clear(False)
    assert ext.evaluate(features["concept_register_size"], context).get() == 0
    assert ext.evaluate(features["role_register_size"], context).get() == 0
    del arguments, registers, argument_view, register_view, empty_register_view
    gc.collect()
    caches.clear(False)
    assert ext.evaluate(features["argument_count"], context).get() == 2


@pytest.mark.parametrize("kind", ["ground", "lifted"])
def test_choice_callbacks_filter_effects_and_keep_independent_cursors(kind: Literal["ground", "lifted"]) -> None:
    task_context, domain = _task_context(kind)
    source = PROGRAM.replace("(:load", "(:choose").replace(
        "(:register (:concept selected))",
        "(:register (:concept selected)) (:effects (increases concept_register_size))",
    ).replace(
        "(:register (:role location))",
        "(:register (:role location)) (:effects (increases role_register_size))",
    )
    program = parse_program(source, domain, task_context.domain_context.ext_repository)
    expander_type = ext.GroundSuccessorExpander if kind == "ground" else ext.LiftedSuccessorExpander
    expander = expander_type(task_context, program)
    node = initial_node(task_context)
    child = collect_steps(expander, expander.initial_state(node), node)[0].target
    statistics = ext.ProgramSearchStatistics()

    def bindings(state, choice_type):
        choices = []

        def emit(choice):
            choices.append(choice)
            return True

        generated = statistics.num_generated
        assert expander.for_each_successor(state, node, statistics, emit, lambda: False)
        assert statistics.num_generated == generated
        choice, = choices
        assert isinstance(choice, choice_type)
        assert choice.rule is not None
        assert choice.count() == 2
        expected = list(choice.denotation)
        result = []
        for value in expected:
            assert not choice.exhausted()
            assert choice.current() == value
            result.append(expander.apply_choice(state, node, choice, statistics))
            choice.advance()
        assert choice.exhausted()
        assert list(choice.denotation) == expected
        assert statistics.num_generated == generated + len(result)
        with pytest.raises(IndexError):
            choice.current()
        with pytest.raises(IndexError):
            choice.advance()
        return result

    concept_steps = bindings(child, ext.ConceptChoice)
    assert len(concept_steps) == 2
    assert child.module_state.registers.concept_values[0] is None
    assert {
        step.target.module_state.registers.concept_values[0].get_name() for step in concept_steps
    } == {"ball1", "ball2"}
    for step in concept_steps:
        role_steps = bindings(step.target, ext.RoleChoice)
        assert len(role_steps) == 2
        assert step.target.module_state.registers.role_values[0] is None
        for role_step in role_steps:
            assert role_step.target.state == child.state
            assert role_step.target.module_state.registers.role_values[0] is not None


@pytest.mark.parametrize("kind", ["ground", "lifted"])
@pytest.mark.parametrize("universal", [False, True])
def test_choose_search_statistics_are_read_only(
    kind: Literal["ground", "lifted"], universal: bool,
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
        options = ext.GroundProgramSearchOptions()
        find_solution = ext.find_ground_solution
    else:
        context = LiftedTaskContext(domain, LiftedTaskSearchContext(task, execution))
        options = ext.LiftedProgramSearchOptions()
        find_solution = ext.find_lifted_solution
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
    result = find_solution(context, program, options)
    assert result.is_successful()
    assert isinstance(result.statistics, ext.ProgramSearchStatistics)
    assert result.statistics.num_expanded == 5
    assert result.statistics.num_generated == 5
    assert result.statistics.choice_depth == 1
    for name in ("num_expanded", "num_generated", "choice_depth"):
        with pytest.raises(AttributeError):
            setattr(result.statistics, name, 99)
