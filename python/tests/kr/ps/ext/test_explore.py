"""Exploration of Choose rules: outcomes of bindings, contrasts, feature separation, and Choose-to-Load trials."""

from datetime import timedelta

import pytest

from pyrunir.kr.ps import ext
from pyrunir.kr.ps.ext.explore import Budget, Session

# Carry one ball to roomb. Only ball2 has that goal; the order tries ball1 first, which ends in an open state.
PROGRAM = """(:program (:entry carry) (:module (:symbol carry) (:arguments) (:registers (:concept selected))
  (:entry choose) (:memory choose pick move drop done)
  (:features
    (:concept (:symbol any) (:expression (c_top)))
    (:concept (:symbol located) (:expression (c_some (r_atomic_state "at") (c_top))))
    (:concept (:symbol selected_ball) (:expression (c_register selected)))
    (:concept (:symbol free_gripper) (:expression (c_atomic_state "free")))
    (:concept (:symbol here) (:expression (c_atomic_state "at-robby")))
    (:concept (:symbol there) (:expression (c_and (c_atomic_state "room") (c_not (c_atomic_state "at-robby")))))
    (:boolean (:symbol selected_has_goal) (:expression (b_nonempty (c_and (c_register selected) (c_some (r_atomic_goal "at" true) (c_top))))))
    (:numerical (:symbol num_located) (:expression (n_count (c_some (r_atomic_state "at") (c_top))))))
  (:rules
    (:rule (:symbol choose_ball) (:expression (:source-memory choose) (:target-memory pick)
      (:choose (:conditions) (:concept located) (:register (:concept selected)) (:effects) (:order (min selected_has_goal)))))
    (:rule (:symbol pick_it) (:expression (:source-memory pick) (:target-memory move)
      (:do (:conditions) (:action "pick") (:arguments selected_ball any free_gripper) (:effects))))
    (:rule (:symbol move_it) (:expression (:source-memory move) (:target-memory drop)
      (:do (:conditions) (:action "move") (:arguments here there) (:effects))))
    (:rule (:symbol drop_it) (:expression (:source-memory drop) (:target-memory done)
      (:do (:conditions) (:action "drop") (:arguments selected_ball any any) (:effects)))))))"""


@pytest.fixture
def session(gripper_data_dir):
    return Session(gripper_data_dir / "domain.pddl", PROGRAM, [gripper_data_dir / "test-1.pddl"], budget=Budget(10_000, 10.0), workers=1)


def test_run_reports_the_search_and_its_path(session):
    run = session.run(session.task("test-1"))
    assert run.status == "success"
    assert run.category is None and run.failure is None
    assert run.num_backtracks == 1
    assert run.path[0].task.name == "test-1" and len(run.path) > 1


def test_alternatives_show_the_failed_and_the_successful_binding_in_order_rank(session):
    rule = session.rule("choose_ball")
    [summary] = session.chooses()
    assert summary.rule == rule and summary.occurrences == 1 and summary.failed_bindings == 1 and summary.untried_bindings == 0
    [point] = session.points(rule)
    assert (point.width, point.chosen, point.failed) == (2, 2, 1)
    first, second = session.alternatives(point)
    assert (first.binding.binding, first.rank, first.outcome, first.ending) == ("ball1", 1, "failed", "subtree")
    assert (second.binding.binding, second.rank, second.outcome) == ("ball2", 2, "succeeded")
    assert first.deltas == {"selected_has_goal": (True, False)}


def test_untried_bindings_are_searched_only_with_a_budget(gripper_data_dir):
    # Ordering the goal ball first leaves ball1 untried; a budget searches it from its target.
    program = PROGRAM.replace("(:order (min selected_has_goal))", "(:order (max selected_has_goal))")
    session = Session(gripper_data_dir / "domain.pddl", program, [gripper_data_dir / "test-1.pddl"], workers=1)
    [point] = session.points(session.rule("choose_ball"))
    assert [a.outcome for a in session.alternatives(point)] == ["succeeded", "untried"]
    assert [a.outcome for a in session.alternatives(point, Budget(1_000, 5.0))] == ["succeeded", "fails"]


def test_contrast_ranks_the_feature_that_orders_success_above_failure(session):
    rule = session.rule("choose_ball")
    contrast = session.contrast(rule)
    assert len(contrast.pairs) == 1 and contrast.decided == 1
    best = contrast.ranking[0]
    assert str(best.feature) == "selected_has_goal" and best.consistent and best.higher == 1
    expression = session.feature('(b_nonempty (c_and (c_register selected) (c_some (r_atomic_goal "at" true) (c_top))))')
    assert expression.category == "boolean"
    # A concept counts as membership of the binding at the Choose source.
    goal_balls = session.feature('(c_some (r_atomic_goal "at" true) (c_top))')
    assert session.separation(goal_balls, contrast).consistent


def test_try_load_runs_the_program_with_the_choose_as_a_load(session):
    trial = session.try_load(session.rule("choose_ball"))
    assert trial.terminating
    assert [task.name for task in trial.failed[0][:1]] == ["test-1"]
    assert trial.failed[0][1] in ("execute", "prove")


def test_vertex_shows_registers_features_and_atoms(session):
    [point] = session.points(session.rule("choose_ball"))
    vertex = session.vertex(point.vertex)
    assert vertex.memory == "choose" and vertex.registers == {"selected": None}
    assert vertex.features["num_located"] == 2
    assert "(at ball1 rooma)" in vertex.atoms


def test_search_from_a_start_state_continues_from_a_proof_graph_vertex(session, gripper_data_dir):
    task = session.task("test-1")
    task_context = session._task_context(task)
    program = session._program
    options = ext.LiftedProgramSearchOptions()
    options.max_time = timedelta(seconds=10)
    full = ext.find_lifted_solution(task_context, program, options)
    graph = full.graph
    goal = next(v for v in graph.get_vertex_indices() if graph.get_vertex_property(v).is_goal)
    predecessor = next(graph.get_source(edge) for edge in graph.get_edge_indices() if graph.get_target(edge) == goal)
    start = graph.get_vertex_property(predecessor).program_state
    options.start_state = start
    resumed = ext.find_lifted_solution(task_context, program, options)
    assert resumed.is_successful() and resumed.plan is None
    initial = [v for v in resumed.graph.get_vertex_indices() if resumed.graph.get_vertex_property(v).is_initial]
    assert [resumed.graph.get_vertex_property(v).program_state for v in initial] == [start]


def test_standalone_expressions_reference_registers_by_name(gripper_planning_domain, session):
    module = session._module
    concept = ext.dl.parse_concept("(c_register selected)", gripper_planning_domain, module, session._domain.ext_repository)
    assert "selected" in str(concept)
    with pytest.raises(Exception):
        ext.dl.parse_concept("(c_register missing)", gripper_planning_domain, module, session._domain.ext_repository)


def test_validate_executes_and_proves_every_task(session):
    validation = session.validate()
    [task] = validation.tasks
    assert task.execute == "passed" and task.prove == "passed" and task.choice_depth == 1  # a Choose needs one binding
    assert str(validation).startswith("passed 1/1; choice_depth sum over passed tasks 1")
