from datetime import timedelta

import pytest
from fixture_utils import read_fixture
from pypddl.formalism import ParserOptions
from pyrunir.datasets import GroundTaskSearchContext, LiftedTaskSearchContext
from pyrunir.kr import DomainContext, GroundTaskContext, LiftedTaskContext
from pyrunir.kr.ps import base
from pytyr.formalism.planning import Parser
from pytyr.planning import lifted
from pyyggdrasil.execution import ExecutionContext


def _context(tmp_path, kind, initial_goal=False):
    domain_path = tmp_path / "domain.pddl"
    domain_path.write_text("""(define (domain counters)
      (:requirements :strips)
      (:predicates (start) (done))
      (:action finish-a :parameters () :precondition (start)
        :effect (and (not (start)) (done)))
      (:action finish-b :parameters () :precondition (start)
        :effect (and (not (start)) (done))))""")
    task_path = tmp_path / "task.pddl"
    initial = "done" if initial_goal else "start"
    task_path.write_text(f"""(define (problem counters-task)
      (:domain counters) (:init ({initial})) (:goal (done)))""")
    parser = Parser(domain_path, ParserOptions())
    task = lifted.Task(parser.parse_task(task_path, ParserOptions()))
    execution = ExecutionContext(1)
    domain = DomainContext(parser.get_domain())
    if kind == "ground":
        search = GroundTaskSearchContext(task.instantiate_ground_task(execution).task, execution)
        context = GroundTaskContext(domain, search)
    else:
        context = LiftedTaskContext(domain, LiftedTaskSearchContext(task, execution))
    sketch = base.dl.parse_sketch(
        read_fixture("kr/ps/base/executor/any_transition.sketch"),
        parser.get_domain(),
        domain.base_repository,
    )
    return context, sketch


@pytest.mark.parametrize("kind", ["ground", "lifted"])
@pytest.mark.parametrize("shuffle", [False, True])
def test_search_statistics_count_generation_before_filtering_and_deduplication(tmp_path, kind, shuffle):
    context, sketch = _context(tmp_path, kind)
    options = getattr(base, f"{kind.title()}SketchSearchOptions")()
    options.shuffle_choice_points = shuffle
    find_solution = getattr(base, f"find_{kind}_solution")

    greedy = find_solution(context, sketch, options)
    assert greedy.status == base.SketchProofStatus.SUCCESS
    assert isinstance(greedy.statistics, base.SketchSearchStatistics)
    assert greedy.statistics.num_expanded == 1
    assert greedy.statistics.num_generated == (2 if shuffle else 1)
    assert greedy.plan is not None
    assert greedy.plan.get_length() == 1
    step = greedy.plan.get_labeled_succ_nodes()[0]
    edge = next(iter(greedy.graph.get_edge_indices()))
    source = greedy.graph.get_vertex_property(greedy.graph.get_source(edge))
    target = greedy.graph.get_vertex_property(greedy.graph.get_target(edge))
    assert greedy.plan.get_start_node().get_state() == source.state
    assert step.node.get_state() == target.state
    assert step.label == greedy.graph.get_edge_property(edge).transition.action
    assert target.is_goal
    assert greedy.plan.get_cost() == step.node.get_metric()
    with pytest.raises(AttributeError):
        greedy.statistics.num_generated = 0
    with pytest.raises(AttributeError):
        greedy.statistics.num_expanded = 0

    options.universal = True
    universal = find_solution(context, sketch, options)
    assert universal.status == base.SketchProofStatus.SUCCESS
    assert universal.plan is None
    assert universal.statistics.num_expanded == 1
    assert universal.statistics.num_generated == 2
    assert universal.graph.get_num_vertices() == 2

    empty = base.dl.SketchFactory.create_empty(context.domain_context.base_repository)
    rejected = find_solution(context, empty, options)
    assert rejected.status == base.SketchProofStatus.FAILURE
    assert rejected.plan is None
    assert rejected.statistics.num_expanded == 1
    assert rejected.statistics.num_generated == 2
    assert rejected.graph.get_num_edges() == 0

    options.max_num_states = 1
    bounded = find_solution(context, sketch, options)
    assert bounded.status == base.SketchProofStatus.OUT_OF_STATES
    assert bounded.plan is None
    assert bounded.statistics.num_expanded == 1
    assert bounded.statistics.num_generated == (2 if shuffle else 1)

    options.max_time = timedelta(0)
    expired = find_solution(context, sketch, options)
    assert expired.status == base.SketchProofStatus.OUT_OF_TIME
    assert expired.plan is None
    assert expired.statistics.num_expanded == 0
    assert expired.statistics.num_generated == 0


@pytest.mark.parametrize("kind", ["ground", "lifted"])
def test_initial_goal_is_not_expanded(tmp_path, kind):
    context, sketch = _context(tmp_path, kind, initial_goal=True)
    options = getattr(base, f"{kind.title()}SketchSearchOptions")()
    result = getattr(base, f"find_{kind}_solution")(context, sketch, options)
    assert result.status == base.SketchProofStatus.SUCCESS
    assert result.statistics.num_expanded == 0
    assert result.statistics.num_generated == 0
    assert result.plan is not None
    assert result.plan.empty()
    assert result.plan.get_cost() == 0
    initial = result.graph.get_vertex_property(next(iter(result.graph.get_vertex_indices())))
    assert initial.is_initial and initial.is_goal
    assert result.plan.get_start_node().get_state() == initial.state

    options.universal = True
    universal = getattr(base, f"find_{kind}_solution")(context, sketch, options)
    assert universal.status == base.SketchProofStatus.SUCCESS
    assert universal.plan is None
