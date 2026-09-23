import gc
import sys
from collections import Counter

from fixture_utils import read_fixture
from pyrunir.datasets import GroundTaskSearchContext
from pytyr.formalism.planning import ActionBinding, PlanningDomain
from pytyr.planning.ground import PackedState

from pyrunir.kr import DomainContext, GroundTaskContext
from pyrunir.kr.dl.base.semantics import (
    ConstructorRepositoryFactory,
    DenotationCaches,
    GroundStateEvaluationContext,
    LiftedStateEvaluationContext,
    syntactic_complexity as dl_syntactic_complexity,
)
from pyrunir.kr.ps.base import (
    GroundSketchProofGraph,
    GroundSketchSearchOptions,
    LiftedSketchSearchOptions,
    SketchProofEdgeLabel,
    SketchProofStatus,
    SketchSearchStatistics,
    SuccessorExpander,
    find_ground_solution,
    syntactic_complexity,
)
from pyrunir.kr.ps.base import RepositoryFactory as SketchRepositoryFactory
from pyrunir.kr.ps.base.dl import (
    GroundTransitionEvaluationContext,
    LiftedTransitionEvaluationContext,
    SketchFactory,
    SketchSpecification,
    parse_sketch,
)
from pyrunir.kr.uns.dl import ClassifierFactory, parse_classifier


def test_base_proof_properties_keep_the_graph_and_sketch_alive(
    ground_gripper_search_context: GroundTaskSearchContext, gripper_planning_domain: PlanningDomain
) -> None:
    context = GroundTaskContext(DomainContext(gripper_planning_domain), ground_gripper_search_context)
    sketch = SketchFactory.create(
        SketchSpecification.GRIPPER_FRANCE_ET_AL_AAAI2021,
        gripper_planning_domain,
        context.domain_context.base_repository,
    )
    proof = find_ground_solution(context, sketch, GroundSketchSearchOptions())
    graph = proof.graph
    references = sys.getrefcount(graph)
    vertex = graph.get_vertex_property(next(iter(graph.get_vertex_indices())))
    assert isinstance(vertex.state, PackedState)
    assert vertex.state.unpack().pack() == vertex.state
    assert sys.getrefcount(graph) > references
    references = sys.getrefcount(vertex)
    state = vertex.state
    assert sys.getrefcount(vertex) == references
    assert state == vertex.state
    references = sys.getrefcount(graph)
    edge = graph.get_edge_property(next(iter(graph.get_edge_indices())))
    assert sys.getrefcount(graph) > references
    rule = edge.rule
    action = edge.action
    expected = (str(rule), str(action), str(vertex.state.unpack()))

    del edge, graph, proof, sketch, context
    gc.collect()

    assert (str(rule), str(action), str(vertex.state.unpack())) == expected


def test_base_sketch_exposes_declared_features(gripper_planning_domain: PlanningDomain) -> None:
    planning_domain = gripper_planning_domain
    dl_repository = ConstructorRepositoryFactory().create(planning_domain)
    sketch_repository = SketchRepositoryFactory().create(dl_repository)
    sketch = parse_sketch(read_fixture("kr/ps/base/dl/declared_features.sketch"), planning_domain, sketch_repository)

    assert [feature.get_variant().get_symbol() for feature in sketch.get_boolean_features()] == []
    assert [feature.get_variant().get_symbol() for feature in sketch.get_numerical_features()] == ["n_balls", "n_held"]



def test_base_successor_callbacks_filter_and_stop_generation(
    ground_gripper_search_context: GroundTaskSearchContext, gripper_planning_domain: PlanningDomain
) -> None:
    search = ground_gripper_search_context
    task_context = GroundTaskContext(DomainContext(gripper_planning_domain), search)
    sketch_repository = task_context.domain_context.base_repository
    sketch = parse_sketch(
        read_fixture("kr/ps/base/executor/any_transition.sketch"), gripper_planning_domain, sketch_repository
    )
    expander = SuccessorExpander(task_context, sketch)
    rejecting_expander = SuccessorExpander(task_context, SketchFactory.create_empty(sketch_repository))
    initial = search.successor_generator.get_initial_node(search.state_repository, search.axiom_evaluator)
    initial_successors = search.successor_generator.get_labeled_successor_nodes(
        initial, search.state_repository, search.axiom_evaluator
    )
    assert len(initial_successors) > 1

    for node in (initial, initial_successors[0].node):
        state = node.get_state()
        expected = search.successor_generator.get_labeled_successor_nodes(
            node, search.state_repository, search.axiom_evaluator
        )
        accepted = [step for step in expected if step.node.get_state().get_index() != state.get_index()]
        assert accepted
        actual = []
        statistics = SketchSearchStatistics()
        assert statistics.num_generated == 0

        def emit(step, rule):
            assert statistics.num_generated >= len(actual) + 1
            actual.append((step, rule))
            return True

        assert expander.for_each_successor(node, statistics, emit, lambda: False) is True
        generated = len(expected)
        assert statistics.num_generated == generated
        assert Counter((step.label, step.node.get_state()) for step, _ in actual) == Counter(
            (step.label, step.node.get_state()) for step in accepted
        )
        assert all(expander.matching_rule(state, step.node.get_state()) == rule for step, rule in actual)
        first_accepted = next(i for i, step in enumerate(expected) if step.node.get_state().get_index() != state.get_index())
        assert expander.for_each_successor(node, statistics, lambda step, rule: False, lambda: False) is False
        generated += first_accepted + 1
        assert statistics.num_generated == generated

        actual.clear()
        assert expander.for_each_successor(node, statistics, emit, lambda: True) is False
        assert statistics.num_generated == generated
        assert actual == []
        assert rejecting_expander.for_each_successor(node, statistics, emit, lambda: False) is True
        generated += len(expected)
        assert statistics.num_generated == generated
        assert actual == []
        assert all(rejecting_expander.matching_rule(state, step.node.get_state()) is None for step in expected)

        assert rejecting_expander.for_each_successor(
            node, statistics, emit, lambda: statistics.num_generated > generated
        ) is False
        assert statistics.num_generated == generated + 1
        assert statistics.num_expanded == 0
        assert actual == []


def test_france_et_al_aaai2021_policy_executor_for_gripper_task(
    ground_gripper_search_context: GroundTaskSearchContext, gripper_planning_domain: PlanningDomain
) -> None:
    search_context = ground_gripper_search_context
    task_context = GroundTaskContext(DomainContext(gripper_planning_domain), search_context)
    assert task_context.search_context is search_context
    planning_domain = gripper_planning_domain

    _dl_repository = task_context.domain_context.base_repository.get_dl_repository()
    sketch_repository = task_context.domain_context.base_repository
    empty_sketch = SketchFactory.create_empty(sketch_repository)
    empty_sketch_description = str(empty_sketch)
    assert empty_sketch_description == str(parse_sketch(empty_sketch_description, planning_domain, sketch_repository))
    assert syntactic_complexity(empty_sketch) == 0
    assert empty_sketch.syntactic_complexity() == 0

    sketch = SketchFactory.create(
        SketchSpecification.GRIPPER_FRANCE_ET_AL_AAAI2021,
        planning_domain,
        sketch_repository,
    )
    sketch_description = str(sketch)
    reparsed_sketch = parse_sketch(sketch_description, planning_domain, sketch_repository)
    assert str(reparsed_sketch) == sketch_description
    assert syntactic_complexity(sketch) == 13

    initial_node = search_context.successor_generator.get_initial_node(
        search_context.state_repository, search_context.axiom_evaluator
    )
    labeled_successor = search_context.successor_generator.get_labeled_successor_nodes(
        initial_node, search_context.state_repository, search_context.axiom_evaluator
    )[0]
    source_state = initial_node.get_state()
    target_state = labeled_successor.node.get_state()
    dl_builder = task_context.dl_builder
    dl_denotation_repository = task_context.dl_denotation_repository
    assert dl_builder is task_context.dl_builder
    assert dl_denotation_repository is task_context.dl_denotation_repository
    expander = SuccessorExpander(task_context, sketch)
    expander.matching_rule(source_state, target_state)
    source_caches, target_caches = DenotationCaches(), DenotationCaches()
    transition_context = GroundTransitionEvaluationContext(
        source_state, target_state, dl_builder, dl_denotation_repository, source_caches, target_caches
    )
    state_context = GroundStateEvaluationContext(source_state, dl_builder, dl_denotation_repository, source_caches)
    assert LiftedTransitionEvaluationContext is not None
    assert LiftedStateEvaluationContext is not None

    assert isinstance(sketch.is_compatible_with(transition_context), bool)
    rule = next(iter(sketch.get_rules()))
    assert isinstance(rule.is_compatible_with(transition_context), bool)
    condition = next(iter(rule.get_conditions()))
    assert isinstance(condition.is_compatible_with(transition_context), bool)
    concrete_condition_variant = condition.get_variant()
    concrete_condition = concrete_condition_variant.get_variant()
    feature = concrete_condition.get_feature()
    concrete_feature = feature.get_variant()
    dl_constructor = concrete_feature.get_feature()
    dl_denotation = dl_constructor.evaluate(state_context)
    assert isinstance(feature.evaluate(state_context).get(), bool)
    assert concrete_feature.evaluate(state_context) == dl_denotation
    assert dl_syntactic_complexity(dl_constructor) == dl_constructor.syntactic_complexity()
    assert feature.syntactic_complexity() == concrete_feature.syntactic_complexity()
    assert syntactic_complexity(sketch) == sketch.syntactic_complexity()
    assert dl_denotation.get_index() is not None
    assert sketch == sketch
    assert not sketch < sketch
    assert sketch <= sketch
    assert not sketch > sketch
    assert sketch >= sketch
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

    proof_options = GroundSketchSearchOptions()
    proof_options.universal = True
    proof_result = find_ground_solution(task_context, sketch, proof_options)
    assert proof_result.status == SketchProofStatus.SUCCESS
    assert proof_result.is_successful()
    assert proof_result.deadend_states == []
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
        edge_label = proof_result.graph.get_edge_property(edge)
        assert isinstance(edge_label, SketchProofEdgeLabel)
        assert isinstance(edge_label.action, ActionBinding)

    search_options = GroundSketchSearchOptions()
    search_result = find_ground_solution(task_context, sketch, search_options)
    assert search_result.status == SketchProofStatus.SUCCESS
    assert search_result.is_successful()
    assert search_result.deadend_states == []
    assert search_result.open_states == []
    assert search_result.cycle == []

    classifier_dl_repository = task_context.domain_context.uns_repository.get_dl_repository()
    classifier_repository = task_context.domain_context.uns_repository
    classifier = parse_classifier(read_fixture("kr/uns/always.classifier"), planning_domain, classifier_repository)
    classifier_feature = classifier.get_features()[0]
    concrete_classifier_feature = classifier_feature.get_variant()
    assert classifier_feature.syntactic_complexity() == concrete_classifier_feature.syntactic_complexity()
    assert (
        concrete_classifier_feature.syntactic_complexity()
        == concrete_classifier_feature.get_expression().syntactic_complexity()
    )
    assert classifier.syntactic_complexity() == 1
    assert ClassifierFactory.create_empty(classifier_repository).syntactic_complexity() == 0
    classified_options = GroundSketchSearchOptions()
    assert classified_options.classifier is None
    classified_options.classifier = classifier
    lifted_options = LiftedSketchSearchOptions()
    assert lifted_options.classifier is None
    lifted_options.classifier = classifier
    assert lifted_options.classifier.get_index() == classifier.get_index()

    del classifier, classifier_repository, classifier_dl_repository
    gc.collect()
    classified_result = find_ground_solution(task_context, sketch, classified_options)
    assert classified_result.status == SketchProofStatus.FAILURE
    assert classified_result.graph.get_num_vertices() == 1
    assert classified_result.graph.get_num_edges() == 0
    assert len(classified_result.deadend_states) == 1
    assert classified_result.open_states == []
    classified_label = classified_result.graph.get_vertex_property(classified_result.deadend_states[0])
    assert not classified_label.is_goal
    assert not classified_label.is_alive
    assert classified_label.is_unsolvable
