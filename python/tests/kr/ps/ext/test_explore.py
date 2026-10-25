# Copyright (C) 2026 Dominik Drexler
# SPDX-License-Identifier: GPL-3.0-or-later

"""Exploration of Choose rules: outcomes of bindings, contrasts, feature separation, and Choose-to-Load trials."""

import argparse
from datetime import timedelta
from pathlib import Path

import pytest
from pypddl.formalism import ParserOptions
from pytyr.formalism.planning import Parser
from pytyr.planning import lifted
from pyyggdrasil.execution import ExecutionContext

from pyrunir.datasets import LiftedTaskSearchContext
from pyrunir.kr import DomainContext, LiftedTaskContext, UndefinedSymbolError
from pyrunir.kr.ps import ext
from pyrunir.kr.ps.ext.explore import Budget, Session, SessionOptions, Task, TaskValidation, Validation
from pyrunir.kr.ps.ext.explore.graph import distinguishing_atoms

CHOOSE_BALL = (
    "(:choose (:conditions) (:concept located) (:register (:concept selected)) (:effects)"
    " (:order (min selected_has_goal)))"
)
LOAD_BALL = "(:load (:conditions) (:concept located) (:register (:concept selected)) (:effects))"

# Carry one ball to roomb. Only ball2 has that goal; the order tries ball1 first, which ends in an open state.
PROGRAM = f"""(:program (:entry carry) (:module (:symbol carry) (:arguments) (:registers (:concept selected))
  (:entry choose) (:memory choose pick move drop done)
  (:features
    (:concept (:symbol any) (:expression (c_top)))
    (:concept (:symbol located) (:expression (c_some (r_atomic_state "at") (c_top))))
    (:concept (:symbol selected_ball) (:expression (c_register selected)))
    (:concept (:symbol free_gripper) (:expression (c_atomic_state "free")))
    (:concept (:symbol here) (:expression (c_atomic_state "at-robby")))
    (:concept (:symbol there) (:expression (c_and (c_atomic_state "room") (c_not (c_atomic_state "at-robby")))))
    (:boolean (:symbol selected_has_goal)
      (:expression (b_nonempty (c_and (c_register selected) (c_some (r_atomic_goal "at" true) (c_top))))))
    (:numerical (:symbol num_located) (:expression (n_count (c_some (r_atomic_state "at") (c_top))))))
  (:rules
    (:rule (:symbol choose_ball) (:expression (:source-memory choose) (:target-memory pick)
      {CHOOSE_BALL}))
    (:rule (:symbol pick_it) (:expression (:source-memory pick) (:target-memory move)
      (:do (:conditions) (:action "pick") (:arguments selected_ball any free_gripper) (:effects))))
    (:rule (:symbol move_it) (:expression (:source-memory move) (:target-memory drop)
      (:do (:conditions) (:action "move") (:arguments here there) (:effects))))
    (:rule (:symbol drop_it) (:expression (:source-memory drop) (:target-memory done)
      (:do (:conditions) (:action "drop") (:arguments selected_ball any any) (:effects)))))))"""


@pytest.fixture
def session(gripper_data_dir: Path) -> Session:
    return Session(
        gripper_data_dir / "domain.pddl",
        PROGRAM,
        [gripper_data_dir / "test-1.pddl"],
        options=SessionOptions(budget=Budget(10_000, 10.0), workers=1),
    )


def test_run_reports_the_search_and_its_path(session: Session) -> None:
    run = session.run(session.task("test-1"))
    assert run.status == "success"
    assert run.category is None
    assert run.failure is None
    assert run.num_backtracks == 1
    assert run.path[0].task.name == "test-1"
    assert len(run.path) > 1


def test_alternatives_show_the_failed_and_the_successful_binding_in_order_rank(session: Session) -> None:
    rule = session.rule("choose_ball")
    [summary] = session.chooses()
    assert summary.rule == rule
    assert summary.occurrences == 1
    assert summary.failed_bindings == 1
    assert summary.untried_bindings == 0
    [point] = session.points(rule)
    assert (point.width, point.chosen, point.failed) == (2, 2, 1)
    first, second = session.alternatives(point)
    assert (first.binding.binding, first.rank, first.outcome, first.ending) == ("ball1", 1, "failed", "subtree")
    assert (second.binding.binding, second.rank, second.outcome) == ("ball2", 2, "succeeded")
    assert first.deltas == {"selected_has_goal": (True, False)}
    assert first.atoms == second.atoms == ()  # both balls lie in rooma; only their goals differ, which are not atoms


def test_untried_bindings_are_searched_only_with_a_budget(gripper_data_dir: Path) -> None:
    # Ordering the goal ball first leaves ball1 untried; a budget searches it from its target.
    program = PROGRAM.replace("(:order (min selected_has_goal))", "(:order (max selected_has_goal))")
    session = Session(
        gripper_data_dir / "domain.pddl", program, [gripper_data_dir / "test-1.pddl"], options=SessionOptions(workers=1)
    )
    [point] = session.points(session.rule("choose_ball"))
    assert [a.outcome for a in session.alternatives(point)] == ["succeeded", "untried"]
    assert [a.outcome for a in session.alternatives(point, Budget(1_000, 5.0))] == ["succeeded", "fails"]


def test_contrast_ranks_the_feature_that_orders_success_above_failure(session: Session) -> None:
    rule = session.rule("choose_ball")
    contrast = session.contrast(rule)
    assert len(contrast.pairs) == 1
    assert contrast.decided == 1
    best = contrast.ranking[0]
    assert best.feature.describe() == "selected_has_goal"
    assert best.consistent
    assert best.higher == 1
    expression = session.feature(
        '(b_nonempty (c_and (c_register selected) (c_some (r_atomic_goal "at" true) (c_top))))'
    )
    assert expression.category == "boolean"
    # A concept counts as membership of the binding at the Choose source.
    goal_balls = session.feature('(c_some (r_atomic_goal "at" true) (c_top))')
    assert session.separation(goal_balls, contrast).consistent


def test_try_load_runs_the_program_with_the_choose_as_a_load(session: Session) -> None:
    trial = session.try_load(session.rule("choose_ball"))
    assert trial.terminating
    assert [task.name for task in trial.failed[0][:1]] == ["test-1"]
    assert trial.failed[0][1] in ("execute", "prove")


def test_vertex_shows_registers_features_and_atoms(session: Session) -> None:
    [point] = session.points(session.rule("choose_ball"))
    vertex = session.vertex(point.vertex)
    assert vertex.memory == "choose"
    assert vertex.registers == {"selected": None}
    assert vertex.features["num_located"] == 2
    assert "(at ball1 rooma)" in vertex.atoms


def _carry_program(parser: Parser) -> tuple[DomainContext, ext.Program]:
    domain = DomainContext(parser.get_domain())
    return domain, ext.dl.parse_program(PROGRAM, parser.get_domain(), domain.ext_repository)


def test_search_from_a_start_state_continues_from_a_proof_graph_vertex(
    gripper_parser: Parser, gripper_data_dir: Path
) -> None:
    domain, program = _carry_program(gripper_parser)
    planning_task = gripper_parser.parse_task(gripper_data_dir / "test-1.pddl", ParserOptions())
    task_context = LiftedTaskContext(domain, LiftedTaskSearchContext(lifted.Task(planning_task), ExecutionContext(1)))
    options = ext.LiftedProgramSearchOptions()
    options.max_time = timedelta(seconds=10)
    graph = ext.find_lifted_solution(task_context, program, options).graph
    goal = next(v for v in graph.get_vertex_indices() if graph.get_vertex_property(v).is_goal)
    predecessor = next(graph.get_source(edge) for edge in graph.get_edge_indices() if graph.get_target(edge) == goal)
    start = graph.get_vertex_property(predecessor).program_state
    options.start_state = start
    resumed = ext.find_lifted_solution(task_context, program, options)
    assert resumed.is_successful()
    assert resumed.plan is None
    initial = [v for v in resumed.graph.get_vertex_indices() if resumed.graph.get_vertex_property(v).is_initial]
    assert [resumed.graph.get_vertex_property(v).program_state for v in initial] == [start]


def test_standalone_expressions_reference_registers_by_name(gripper_parser: Parser) -> None:
    domain, program = _carry_program(gripper_parser)
    module = program.get_entry_module()
    concept = ext.dl.parse_concept("(c_register selected)", gripper_parser.get_domain(), module, domain.ext_repository)
    assert "selected" in str(concept)
    with pytest.raises(UndefinedSymbolError, match="missing"):
        ext.dl.parse_concept("(c_register missing)", gripper_parser.get_domain(), module, domain.ext_repository)


def test_validate_executes_and_proves_every_task(session: Session) -> None:
    validation = session.validate()
    [task] = validation.tasks
    assert task.execute == "passed"
    assert task.prove == "passed"
    assert task.choice_depth == 1  # a Choose needs one binding
    assert validation.describe().startswith("passed 1/1; choice_depth sum over passed tasks 1")


def test_successors_and_plan_from_a_failure_vertex(gripper_data_dir: Path) -> None:
    # As a Load the first ball is taken; carrying ball1 to roomb ends in an open state that a plan can still leave.
    program = PROGRAM.replace(CHOOSE_BALL, LOAD_BALL)
    session = Session(
        gripper_data_dir / "domain.pddl", program, [gripper_data_dir / "test-1.pddl"], options=SessionOptions(workers=1)
    )
    run = session.run(session.task("test-1"))
    assert run.status == "failure"
    assert run.failure is not None
    steps = session.successors(run.failure)
    assert steps
    assert all(step.rule is None for step in steps)  # memory `done` has no rules
    [failure] = session.failures()
    assert failure.run.failure == run.failure
    assert failure.memory == "done"
    [located] = session.failures(locate=Budget(1_000, 5.0))
    assert located.dead_end is not None
    assert located.dead_end.dead is None
    assert "stops in a solvable state" in located.describe()
    assert failure.forbidden == failure.applicable
    assert failure.applicable == len(steps)
    # Carrying ball1 to roomb leaves ball2 reachable: the program stops in a solvable state.
    assert session.dead_end(session.task("test-1")).dead is None
    plan = session.plan(run.failure)
    assert plan.status == "solved"
    assert plan.steps
    assert any("ball2" in step.action for step in plan.steps)


def test_validate_compares_a_draft_with_the_initial_program(gripper_data_dir: Path) -> None:
    # The draft orders the goal ball first: the same task passes without backtracking.
    session = Session(
        gripper_data_dir / "domain.pddl", PROGRAM, [gripper_data_dir / "test-1.pddl"], options=SessionOptions(workers=1)
    )
    assert session.validate().baseline is None
    session.set_program(PROGRAM.replace("(:order (min selected_has_goal))", "(:order (max selected_has_goal))"))
    validation = session.validate()
    assert validation.baseline == {session.task("test-1"): 1}
    assert "choice_depth 1 (initial program) -> 1 (this program)" in validation.describe()


def test_termination_reports_a_cycle_of_a_nonterminating_draft(session: Session) -> None:
    assert session.termination().terminating
    # A rule from `done` back to `choose` without effects lets the program loop.
    again = "(:rule (:symbol again) (:expression (:source-memory done) (:target-memory choose)"
    loop = PROGRAM.replace("(:memory choose pick move drop done)", "(:memory choose pick move drop done)").replace(
        "(:rule (:symbol drop_it)",
        f"{again} (:sketch (:conditions) (:effects))))\n    (:rule (:symbol drop_it)",
    )
    session.set_program(loop)
    termination = session.termination()
    assert not termination.terminating
    [module] = termination.modules
    assert "again" in module.surviving_rules
    assert any("--again-->" in step for step in module.cycle)
    with pytest.raises(ValueError, match="not changed"):
        session.set_program("(:program")
    assert not session.termination().terminating  # still the looping draft
    with pytest.raises(ValueError, match="`termination`"):
        session.validate()


def test_validation_lists_the_largest_increases_without_comparing_results() -> None:
    tasks = (Task("a"), Task("b"), Task("c"))
    validation = Validation(
        tuple(TaskValidation(task, "passed", "passed", 3) for task in tasks),
        {tasks[0]: 1, tasks[1]: 1, tasks[2]: 3},
    )
    assert "largest increases: a 1->3, b 1->3" in validation.describe()


def test_distinguishing_atoms_count_peers_and_keep_values() -> None:
    types = {name: frozenset({"position"}) for name in "pqrst"} | {"n0": frozenset({"numb"}), "n1": frozenset({"numb"})}
    profiles = [
        [("height", ("*", "n1")), ("neighbor", ("*", "r")), ("neighbor", ("*", "s"))],
        [("height", ("*", "n0")), ("neighbor", ("*", "t"))],
    ]
    first, second = distinguishing_atoms(profiles, types, ["p", "q"])
    assert first == ("(height * n1)", "(neighbor * _)x2")
    assert second == ("(height * n0)", "(neighbor * _)")


def test_proof_failures_are_explorable_with_proof_vertex_handles(gripper_data_dir: Path) -> None:
    # As a Load every located ball must lead to the goal in the proof; ball1 does not.
    program = PROGRAM.replace(CHOOSE_BALL, LOAD_BALL)
    session = Session(
        gripper_data_dir / "domain.pddl", program, [gripper_data_dir / "test-1.pddl"], options=SessionOptions(workers=1)
    )
    task = session.task("test-1")
    run = session.run(task, prove=True)
    assert run.status == "failure"
    assert run.failure is not None
    assert run.failure.proof
    assert run.failure.describe().startswith("test-1:pv")
    assert session.vertex_ref(run.failure.describe()) == run.failure
    assert session.vertex(run.failure).memory == "done"
    [failure] = session.failures(prove=True)
    assert failure.run.failure == run.failure


def test_server_releases_the_session_on_memory_errors(session: Session, monkeypatch: pytest.MonkeyPatch) -> None:
    server = pytest.importorskip("pyrunir.kr.ps.ext.explore.server")
    released: list[bool] = []
    monkeypatch.setattr(session, "release", lambda: released.append(True))
    monkeypatch.setattr(
        server, "_server", server.Server(argparse.Namespace(max_memory_mib=1, release_mib=2**30), session)
    )

    @server.logged
    def exhausting() -> str:
        raise MemoryError

    assert exhausting().startswith("error: out of memory")
    assert len(released) == 2
    calls: list[int] = []

    @server.logged
    def once() -> str:
        calls.append(1)
        if len(calls) == 1:
            raise MemoryError
        return "ok"

    assert once() == "ok"


def test_numerical_comparisons_are_boolean_features(session: Session) -> None:
    assert session.feature('(n_gt (n_count (c_atomic_state "ball")) (n_const 2))').category == "boolean"


def test_open_takes_a_program_when_the_workspace_program_does_not_parse(
    gripper_data_dir: Path, tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    server = pytest.importorskip("pyrunir.kr.ps.ext.explore.server")
    (tmp_path / "problems").mkdir()
    (tmp_path / "domain.pddl").write_text((gripper_data_dir / "domain.pddl").read_text())
    (tmp_path / "problems" / "t1.pddl").write_text((gripper_data_dir / "test-1.pddl").read_text())
    (tmp_path / "module-program.txt").write_text("(:program (:entry carry) (:module")
    options = argparse.Namespace(
        domain="domain.pddl",
        program="module-program.txt",
        tasks="problems/*.pddl",
        max_states=10_000,
        max_seconds=10.0,
        total_seconds=60.0,
        workers=1,
        release_mib=2**30,
    )
    monkeypatch.setattr(server, "_server", server.Server(options))
    assert "does not parse; pass a program that does as `program`" in server.open_workspace(str(tmp_path))
    assert server.open_workspace(str(tmp_path), PROGRAM).startswith("1 tasks")


def test_scans_that_evict_their_searches_give_the_same_results(gripper_data_dir: Path) -> None:
    def make(release_mib: float | None) -> Session:
        return Session(
            gripper_data_dir / "domain.pddl",
            PROGRAM,
            [gripper_data_dir / "test-1.pddl"],
            options=SessionOptions(budget=Budget(10_000, 10.0), workers=1, release_mib=release_mib),
        )

    kept, evicted = make(None), make(0.0)
    assert [row.describe() for row in evicted.chooses()] == [row.describe() for row in kept.chooses()]
    assert evicted.validate().describe().split("\n")[0] == kept.validate().describe().split("\n")[0]
    rule = kept.choose_rules()[0]
    assert [point.describe() for point in evicted.points(rule)] == [point.describe() for point in kept.points(rule)]
