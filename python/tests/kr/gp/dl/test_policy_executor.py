from pathlib import Path

from pyrunir.datasets import (
    GroundTaskSearchContext,
)
from pyrunir.kr.dl.base.semantics import (
    Builder,
    ConstructorRepositoryFactory,
    DenotationRepositoryFactory,
    GroundEvaluationContext as GroundDLEvaluationContext,
    LiftedEvaluationContext as LiftedDLEvaluationContext,
    syntactic_complexity as dl_syntactic_complexity,
)
from pyrunir.kr.ps.base import (
    GroundSketchProofGraph,
    SketchProofEdgeLabel,
    SketchProofStatus,
    find_ground_solution,
    prove_ground_solution,
    syntactic_complexity,
)
from pyrunir.kr.ps.base import RepositoryFactory as SketchRepositoryFactory
from pyrunir.kr.ps.base.dl import (
    GroundEvaluationContext,
    LiftedEvaluationContext,
    SketchFactory,
    SketchSpecification,
    parse_sketch,
)
from pytyr.common import ExecutionContext
from pytyr.formalism.planning import Parser, ParserOptions
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

    dl_repository = ConstructorRepositoryFactory().create(planning_domain)
    sketch_repository = SketchRepositoryFactory().create(dl_repository)
    empty_sketch = SketchFactory.create_empty(sketch_repository)
    empty_sketch_description = str(empty_sketch)
    assert empty_sketch_description == str(parse_sketch(empty_sketch_description, planning_domain, sketch_repository))
    assert syntactic_complexity(empty_sketch) == 0

    sketch = SketchFactory.create(
        SketchSpecification.GRIPPER_FRANCE_ET_AL_AAAI2021,
        planning_domain,
        sketch_repository,
    )
    sketch_description = str(sketch)
    reparsed_sketch = parse_sketch(sketch_description, planning_domain, sketch_repository)
    assert str(reparsed_sketch) == sketch_description
    assert syntactic_complexity(sketch) == 16

    initial_node = search_context.successor_generator.get_initial_node()
    labeled_successor = search_context.successor_generator.get_labeled_successor_nodes(initial_node)[0]
    source_state = initial_node.get_state()
    target_state = labeled_successor.node.get_state()
    dl_builder = Builder()
    dl_denotation_repository = DenotationRepositoryFactory().create()
    evaluation_context = GroundEvaluationContext(source_state, target_state, dl_builder, dl_denotation_repository)
    dl_evaluation_context = GroundDLEvaluationContext(source_state, dl_builder, dl_denotation_repository)
    assert LiftedEvaluationContext is not None
    assert LiftedDLEvaluationContext is not None

    assert isinstance(sketch.is_compatible_with(evaluation_context), bool)
    rule = next(iter(sketch.get_rules()))
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
    assert syntactic_complexity(sketch) == sketch.syntactic_complexity()
    assert dl_denotation.get_index() is not None
    for view in (
        sketch,
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

    proof_result = prove_ground_solution(search_context, sketch)
    assert proof_result.status == SketchProofStatus.SUCCESS
    assert proof_result.is_successful()
    assert proof_result.deadend_transitions == []
    assert proof_result.open_states == []
    assert proof_result.cycle == []
    assert isinstance(proof_result.graph, GroundSketchProofGraph)
    assert proof_result.graph.get_num_vertices() > 0
    if proof_result.graph.get_num_edges() > 0:
        edge = next(iter(proof_result.graph.get_edge_indices()))
        assert isinstance(proof_result.graph.get_edge_property(edge), SketchProofEdgeLabel)

    search_result = find_ground_solution(search_context, sketch)
    assert search_result.status == SketchProofStatus.SUCCESS
    assert search_result.is_successful()
    assert search_result.deadend_transitions == []
    assert search_result.open_states == []
    assert search_result.cycle == []
