from pyrunir.kr import GroundTaskContext
from pyrunir.kr.dl.base.semantics import (
    ConstructorRepositoryFactory,
    GroundEvaluationContext as GroundDLEvaluationContext,
    LiftedEvaluationContext as LiftedDLEvaluationContext,
    syntactic_complexity as dl_syntactic_complexity,
)
from pyrunir.kr.ps.base import (
    GroundSketchProofGraph,
    SketchProofEdgeLabel,
    SketchProofStatus,
    SuccessorExpander,
    find_ground_solution,
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


def test_base_sketch_exposes_declared_features(gripper_planning_domain):
    planning_domain = gripper_planning_domain
    dl_repository = ConstructorRepositoryFactory().create(planning_domain)
    sketch_repository = SketchRepositoryFactory().create(dl_repository)
    sketch = parse_sketch(
        """
(:sketch
    (:features
        (:numerical
            (:symbol n_balls)
            (:expression
                (n_count
                    (c_atomic_state "ball")
                )
            )
        )
        (:numerical
            (:symbol n_held)
            (:expression
                (n_count
                    (r_atomic_state "carry")
                )
            )
        )
    )
    (:rules
        (:rule
            (:symbol hold)
            (:expression
                (:conditions
                    (greater_zero n_balls)
                )
                (:effects
                    (increases n_held)
                )
            )
        )
    )
)
""",
        planning_domain,
        sketch_repository,
    )

    assert [feature.get_variant().get_symbol() for feature in sketch.get_boolean_features()] == []
    assert [feature.get_variant().get_symbol() for feature in sketch.get_numerical_features()] == ["n_balls", "n_held"]



def test_france_et_al_aaai2021_policy_executor_for_gripper_task(ground_gripper_search_context, gripper_planning_domain):
    search_context = ground_gripper_search_context
    task_context = GroundTaskContext(search_context)
    assert task_context.search_context is search_context
    planning_domain = gripper_planning_domain

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
    dl_builder = task_context.dl_builder
    dl_denotation_repository = task_context.dl_denotation_repository
    assert dl_builder is task_context.dl_builder
    assert dl_denotation_repository is task_context.dl_denotation_repository
    expander = SuccessorExpander(task_context, sketch)
    expander_context = expander.context_at(source_state)
    assert expander_context.state == source_state
    expander.matching_rule(expander_context, target_state)
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

    proof_result = find_ground_solution(task_context, sketch, True)
    assert proof_result.status == SketchProofStatus.SUCCESS
    assert proof_result.is_successful()
    assert proof_result.deadend_transitions == []
    assert proof_result.open_states == []
    assert proof_result.cycle == []
    assert isinstance(proof_result.graph, GroundSketchProofGraph)
    assert proof_result.graph.get_num_vertices() > 0
    vertex = next(iter(proof_result.graph.get_vertex_indices()))
    assert isinstance(proof_result.graph.get_successor_indices(vertex), list)
    if proof_result.graph.get_num_edges() > 0:
        edge = next(iter(proof_result.graph.get_edge_indices()))
        assert proof_result.graph.get_source(edge) in proof_result.graph.get_vertex_indices()
        assert proof_result.graph.get_target(edge) in proof_result.graph.get_vertex_indices()
        assert edge in proof_result.graph.get_out_edge_indices(proof_result.graph.get_source(edge))
        assert isinstance(proof_result.graph.get_edge_property(edge), SketchProofEdgeLabel)

    search_result = find_ground_solution(task_context, sketch)
    assert search_result.status == SketchProofStatus.SUCCESS
    assert search_result.is_successful()
    assert search_result.deadend_transitions == []
    assert search_result.open_states == []
    assert search_result.cycle == []
