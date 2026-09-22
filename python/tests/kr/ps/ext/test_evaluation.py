import gc
import sys
from typing import Literal

import pytest
from fixture_utils import FIXTURE_ROOT
from pypddl.formalism import ParserOptions
from pypddl_datasets import data_root
from pyrunir.datasets import GroundTaskSearchContext, LiftedTaskSearchContext
from pyrunir.kr import DomainContext, GroundTaskContext, LiftedTaskContext
from pyrunir.kr.ps import ext
from pyrunir.kr.ps.ext.dl import parse_module_program
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


def _loaded_frame(kind: Literal["ground", "lifted"]):
    task_context, domain = _task_context(kind)
    program = parse_module_program(
        PROGRAM,
        domain,
        task_context.domain_context.ext_repository,
    )
    if kind == "ground":
        expander = ext.GroundSuccessorExpander(task_context, program)
    else:
        expander = ext.LiftedSuccessorExpander(task_context, program)
    initial = expander.initial_state()
    child = expander.control_steps(initial)[0].target
    with_concept = expander.load_steps(child)[0].target
    loaded = expander.load_steps(with_concept)[0].target
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
def test_evaluation_restores_arguments_registers_and_owns_dependencies(
    kind: Literal["ground", "lifted"],
) -> None:
    task_context, program, expander, loaded = _loaded_frame(kind)
    if kind == "ground":
        environment_type = ext.GroundEvaluationEnvironment
    else:
        environment_type = ext.LiftedEvaluationEnvironment
    frame = loaded.call_stack
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
    arguments = ext.EvaluationArguments(loaded.call_stack.arguments)
    dl_context = environment.make_dl_context(loaded, arguments)
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
    child = expander.control_steps(expander.initial_state())[0].target
    other_frame = expander.load_steps(expander.load_steps(child)[1].target)[0].target
    assert other_frame.state == loaded.state
    environment.get_dl_caches().clear(False)
    other_arguments = ext.EvaluationArguments(other_frame.call_stack.arguments)
    other_dl_context = environment.make_dl_context(other_frame, other_arguments)
    assert ext.evaluate(booleans["selected_goal_ball"], other_dl_context).get() is True
    environment.get_dl_caches().clear(False)
    assert ext.evaluate(booleans["selected_goal_ball"], dl_context).get() is False
    del child, other_frame, other_dl_context, other_arguments

    moved = next(
        step.target
        for step in expander.control_steps(loaded)
        if step.state_transition.action.get_relation().get_name() == "move"
        and step.state_transition.action.get_objects()[-1].get_name() == "roomb"
    )
    environment.get_dl_caches().clear(False)
    arguments = ext.EvaluationArguments(moved.call_stack.arguments)
    dl_context = environment.make_dl_context(moved, arguments)
    assert ext.evaluate(numericals["nearby_balls"], dl_context).get() == 0
    assert ext.evaluate(numericals["argument_count"], dl_context).get() == 2
    assert {
        name: ext.evaluate(feature, dl_context).get()
        for name, feature in booleans.items()
    } == expected_booleans
    environment.get_dl_caches().clear(False)
    arguments = ext.EvaluationArguments(loaded.call_stack.arguments)
    dl_context = environment.make_dl_context(loaded, arguments)
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
    del dl_context, arguments, environment, booleans, numericals
    gc.collect()
    assert {obj.get_name() for obj in concept} == {"ball1", "ball2"}
    assert {(first.get_name(), second.get_name()) for first, second in role} == {
        ("ball1", "rooma"), ("ball2", "rooma")
    }
    assert boolean.get() is True
    assert numerical.get() == 2


@pytest.mark.parametrize("kind", ["ground", "lifted"])
def test_state_evaluation_context_retains_execution_state_and_environment(
    kind: Literal["ground", "lifted"],
) -> None:
    task_context, program, expander, loaded = _loaded_frame(kind)
    environment_type = (
        ext.GroundEvaluationEnvironment if kind == "ground" else ext.LiftedEvaluationEnvironment
    )
    environment = environment_type(task_context, program)
    state_references = sys.getrefcount(loaded)
    environment_references = sys.getrefcount(environment)
    arguments = ext.EvaluationArguments(loaded.call_stack.arguments)
    argument_references = sys.getrefcount(arguments)
    context = environment.make_dl_context(loaded, arguments)
    assert sys.getrefcount(arguments) > argument_references
    assert sys.getrefcount(loaded) > state_references
    assert sys.getrefcount(environment) > environment_references
    boolean = loaded.call_stack.module.get_boolean_features()[0]
    numerical = loaded.call_stack.module.get_numerical_features()[0]
    del task_context, program, expander, loaded, environment, arguments
    gc.collect()
    assert ext.evaluate(boolean, context).get() is True
    assert ext.evaluate(numerical, context).get() == 2


@pytest.mark.parametrize("kind", ["ground", "lifted"])
def test_state_evaluation_contexts_keep_distinct_argument_owners(
    kind: Literal["ground", "lifted"],
) -> None:
    task_context, program, expander, loaded = _loaded_frame(kind)
    initial = expander.initial_state()
    environment = getattr(ext, f"{kind.title()}EvaluationEnvironment")(task_context, program)
    arguments = ext.EvaluationArguments(loaded.call_stack.arguments)
    other_arguments = ext.EvaluationArguments(initial.call_stack.arguments)
    contexts = [
        environment.make_dl_context(loaded, arguments),
        environment.make_dl_context(initial, other_arguments),
    ]
    features = [
        loaded.call_stack.module.get_numerical_features()[0],
        initial.call_stack.module.get_numerical_features()[0],
    ]
    del arguments, other_arguments, loaded, initial
    gc.collect()
    for position in (0, 1, 0):
        environment.get_dl_caches().clear(False)
        assert ext.evaluate(features[position], contexts[position]).get() == 2


@pytest.mark.parametrize("kind", ["ground", "lifted"])
def test_choose_steps_filter_effects_against_original_registers(kind: Literal["ground", "lifted"]) -> None:
    task_context, domain = _task_context(kind)
    source = PROGRAM.replace("(:load", "(:choose").replace(
        "(:register (:concept selected))",
        "(:register (:concept selected)) (:effects (increases concept_register_size))",
    ).replace(
        "(:register (:role location))",
        "(:register (:role location)) (:effects (increases role_register_size))",
    )
    program = parse_module_program(source, domain, task_context.domain_context.ext_repository)
    expander_type = ext.GroundSuccessorExpander if kind == "ground" else ext.LiftedSuccessorExpander
    expander = expander_type(task_context, program)
    child = expander.control_steps(expander.initial_state())[0].target
    concept_steps = expander.choose_steps(child)
    assert len(concept_steps) == 2
    assert child.call_stack.registers.concept_values[0] is None
    assert {
        step.target.call_stack.registers.concept_values[0].get_name() for step in concept_steps
    } == {"ball1", "ball2"}
    for step in concept_steps:
        role_steps = expander.choose_steps(step.target)
        assert len(role_steps) == 2
        assert step.target.call_stack.registers.role_values[0] is None
        for role_step in role_steps:
            assert role_step.target.state == child.state
            assert role_step.target.call_stack.registers.role_values[0] is not None


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
        options = ext.GroundModuleProgramSearchOptions()
        find_solution = ext.find_ground_solution
    else:
        context = LiftedTaskContext(domain, LiftedTaskSearchContext(task, execution))
        options = ext.LiftedModuleProgramSearchOptions()
        find_solution = ext.find_lifted_solution
    program = parse_module_program(
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
    assert result.choice_depth == 1
    assert result.num_choice_points == 1
    assert result.num_binding_attempts == 2
    assert result.num_backtracks == 1
    for name in ("choice_depth", "num_choice_points", "num_binding_attempts", "num_backtracks"):
        with pytest.raises(AttributeError):
            setattr(result, name, 99)
