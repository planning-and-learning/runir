from pathlib import Path

from pyrunir.datasets import (
    GroundTaskSearchContext,
    StateGraphCostMode,
    annotate_ground_state_graph,
    generate_ground_state_graph,
)
from pyrunir.kr.dl.semantics import (
    Builder,
    ConstructorRepositoryFactory,
    DenotationRepositoryFactory,
    GroundEvaluationContext as GroundDLEvaluationContext,
    LiftedEvaluationContext as LiftedDLEvaluationContext,
    syntactic_complexity as dl_syntactic_complexity,
)
from pyrunir.kr.gp import (
    PolicyProofStatus,
    find_ground_solution,
    prove_ground_solution,
    syntactic_complexity,
)
from pyrunir.kr.gp import RepositoryFactory as PolicyRepositoryFactory
from pyrunir.kr.gp.dl import (
    GroundEvaluationContext,
    LiftedEvaluationContext,
    PolicyFactory,
    PolicySpecification,
    parse_policy,
)
from pytyr.common import ExecutionContext
from pytyr.formalism.planning import Parser, ParserOptions
from pytyr.planning import SearchStatus
from pytyr.planning.lifted import GroundTaskInstantiationOptions, Task


def test_france_et_al_aaai2021_policy_executor_for_gripper_task():
    root = Path(__file__).resolve().parents[5]
    data_dir = root / "data" / "planning-benchmarks" / "tests" / "classical" / "gripper"

    parser_options = ParserOptions()
    parser = Parser(data_dir / "domain.pddl", parser_options)
    planning_task = parser.parse_task(data_dir / "test-1.pddl", parser_options)
    planning_domain = parser.get_domain()

    execution_context = ExecutionContext(1)
    lifted_task = Task(planning_task)
    ground_task = lifted_task.instantiate_ground_task(execution_context, GroundTaskInstantiationOptions()).task
    search_context = GroundTaskSearchContext(ground_task, execution_context)

    state_graph = generate_ground_state_graph(search_context)
    annotated_state_graph = annotate_ground_state_graph(search_context, state_graph, StateGraphCostMode.UNIT_COST)

    dl_repository = ConstructorRepositoryFactory().create(planning_domain)
    policy_repository = PolicyRepositoryFactory().create(dl_repository)
    empty_policy = PolicyFactory.create_empty(policy_repository)
    empty_policy_description = str(empty_policy)
    assert empty_policy_description == str(parse_policy(empty_policy_description, planning_domain, policy_repository))
    assert syntactic_complexity(empty_policy) == 0

    policy = PolicyFactory.create(
        PolicySpecification.GRIPPER_FRANCE_ET_AL_AAAI2021,
        planning_domain,
        policy_repository,
    )
    policy_description = str(policy)
    reparsed_policy = parse_policy(policy_description, planning_domain, policy_repository)
    assert str(reparsed_policy) == policy_description
    assert syntactic_complexity(policy) == 16

    graph = state_graph.get_forward_graph()
    edge = next(iter(graph.get_edge_indices()))
    source_label = graph.get_vertex_property(graph.get_source(edge))
    target_label = graph.get_vertex_property(graph.get_target(edge))
    dl_builder = Builder()
    dl_denotation_repository = DenotationRepositoryFactory().create()
    evaluation_context = GroundEvaluationContext(source_label.state, target_label.state, dl_builder, dl_denotation_repository)
    dl_evaluation_context = GroundDLEvaluationContext(source_label.state, dl_builder, dl_denotation_repository)
    assert LiftedEvaluationContext is not None
    assert LiftedDLEvaluationContext is not None

    assert isinstance(policy.is_compatible_with(evaluation_context), bool)
    rule = next(iter(policy.get_rules()))
    assert isinstance(rule.is_compatible_with(evaluation_context), bool)
    condition = next(iter(rule.get_conditions()))
    assert isinstance(condition.is_compatible_with(evaluation_context), bool)
    concrete_condition_variant = condition.get_variant()
    concrete_condition = concrete_condition_variant.get_variant()
    feature = concrete_condition.get_feature()
    concrete_feature = feature.get_variant()
    dl_constructor = concrete_feature.get_feature()
    dl_denotation = dl_constructor.evaluate(dl_evaluation_context)
    assert isinstance(feature.evaluate(dl_evaluation_context), bool)
    assert dl_syntactic_complexity(dl_constructor) == dl_constructor.syntactic_complexity()
    assert feature.syntactic_complexity() == concrete_feature.syntactic_complexity()
    assert syntactic_complexity(policy) == policy.syntactic_complexity()
    assert dl_denotation.get_index() is not None
    for view in (
        policy,
        rule,
        condition,
        concrete_condition_variant,
        concrete_condition,
        feature,
        concrete_feature,
        dl_constructor,
        dl_denotation,
    ):
        assert str(view)
        assert isinstance(hash(view), int)

    proof_result = prove_ground_solution(annotated_state_graph, policy)
    assert proof_result.status == PolicyProofStatus.SUCCESS
    assert proof_result.is_successful()
    assert proof_result.deadend_transitions == []
    assert proof_result.open_states == []
    assert proof_result.cycle == []

    search_result = find_ground_solution(search_context, policy)
    assert search_result.status == SearchStatus.SOLVED
    assert search_result.plan is not None
