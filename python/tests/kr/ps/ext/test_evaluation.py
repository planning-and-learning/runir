import gc
from typing import Literal

import pytest
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
        (:load (:conditions) (:role locations) (:register (:role location))))))))
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


@pytest.mark.parametrize("kind", ["ground", "lifted"])
def test_evaluation_restores_arguments_registers_and_owns_dependencies(
    kind: Literal["ground", "lifted"],
) -> None:
    task_context, program, expander, loaded = _loaded_frame(kind)
    if kind == "ground":
        context_type = ext.GroundEvaluationContext
        environment_type = ext.GroundEvaluationEnvironment
    else:
        context_type = ext.LiftedEvaluationContext
        environment_type = ext.LiftedEvaluationEnvironment
    frame = loaded.call_stack
    assert len(frame.arguments.concept_arguments) == 1
    assert len(frame.arguments.role_arguments) == 1
    assert len(frame.arguments.boolean_arguments) == 1
    assert len(frame.arguments.numerical_arguments) == 1
    assert frame.registers.concept_values[0] is not None
    assert frame.registers.role_values[0] is not None

    context = context_type(
        task_context.execution_repository, task_context.execution_builder, program, loaded
    )
    environment = environment_type(task_context, program)
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
        name: ext.evaluate(feature, context, environment)
        for name, feature in booleans.items()
    } == expected_booleans
    assert {
        name: ext.evaluate(feature, context, environment)
        for name, feature in numericals.items()
    } == expected
    concept = ext.evaluate_feature_denotation(
        frame.module.get_concept_features()[0], context, environment
    )
    role = ext.evaluate_feature_denotation(
        frame.module.get_role_features()[0], context, environment
    )
    boolean = ext.evaluate_feature_denotation(
        booleans["argument_flag"], context, environment
    )
    numerical = ext.evaluate_feature_denotation(
        numericals["argument_count"], context, environment
    )
    child = expander.control_steps(expander.initial_state())[0].target
    other_frame = expander.load_steps(expander.load_steps(child)[1].target)[0].target
    other_context = context_type(
        task_context.execution_repository, task_context.execution_builder, program, other_frame
    )
    assert other_context.state == context.state
    assert ext.evaluate(booleans["selected_goal_ball"], other_context, environment) is True
    assert ext.evaluate(booleans["selected_goal_ball"], context, environment) is False
    del child, other_frame, other_context

    original = context.state
    moved = next(
        successor.node.get_state()
        for successor in expander.labeled_successors(loaded)
        if successor.label.get_relation().get_name() == "move"
        and successor.label.get_objects()[-1].get_name() == "roomb"
    )
    context.state = moved
    assert ext.evaluate(numericals["nearby_balls"], context, environment) == 0
    assert ext.evaluate(numericals["argument_count"], context, environment) == 2
    assert {
        name: ext.evaluate(feature, context, environment)
        for name, feature in booleans.items()
    } == expected_booleans
    context.state = original
    assert ext.evaluate(numericals["nearby_balls"], context, environment) == 2

    del task_context, program, expander, loaded, frame
    del original, moved
    gc.collect()
    assert {
        name: ext.evaluate(feature, context, environment)
        for name, feature in numericals.items()
    } == expected
    assert {
        name: ext.evaluate(feature, context, environment)
        for name, feature in booleans.items()
    } == expected_booleans
    del context, environment, booleans, numericals
    gc.collect()
    assert {obj.get_name() for obj in concept} == {"ball1", "ball2"}
    assert {(first.get_name(), second.get_name()) for first, second in role} == {
        ("ball1", "rooma"), ("ball2", "rooma")
    }
    assert boolean.get() is True
    assert numerical.get() == 2


@pytest.mark.parametrize("kind", ["ground", "lifted"])
def test_evaluation_context_retains_its_interning_dependencies(
    kind: Literal["ground", "lifted"],
) -> None:
    task_context, program, expander, loaded = _loaded_frame(kind)
    context_type = (
        ext.GroundEvaluationContext if kind == "ground" else ext.LiftedEvaluationContext
    )
    context = context_type(
        task_context.execution_repository, task_context.execution_builder, program, loaded
    )
    del task_context, program, expander, loaded
    gc.collect()
    assert context.program.get_entry_module().get_name() == "root"
    retained = context.intern(ext.ExecutionPhase.EXTERNAL)
    del context
    gc.collect()
    assert retained.call_stack.module.get_name() == "child"
    assert retained.call_stack.registers.concept_values[0].get_name() == "ball1"
    assert retained.call_stack.arguments.boolean_arguments[0].get() is True
    assert retained.call_stack.arguments.numerical_arguments[0].get() == 2
