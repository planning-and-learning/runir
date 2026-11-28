# Copyright (C) 2026 Dominik Drexler
# SPDX-License-Identifier: GPL-3.0-or-later

import gc
import sys
from collections import Counter

from fixture_utils import read_fixture
from planning_utils import planning_module
from pytyr.formalism import PlanningDomain, RowActionLifted
from pytyr.planning.ground import (
    OwnedLabeledNodeRecordView as PackedLabeledNode,
    OwnedNodeBuilderView as Node,
    State,
)

from pyrunir.datasets import GroundTaskSearchContext
from pyrunir.kr import DomainContext, GroundTaskContext
from pyrunir.kr.dl import semantics
from pyrunir.kr.dl.semantics import (
    ConstructorRepository,
    EvaluationStorage,
    GroundStateEvaluationContext,
    LiftedStateEvaluationContext,
)
from pyrunir.kr.ps import ProofStatus, base
from pyrunir.kr.ps.base import (
    GroundSketchProofGraph,
    GroundSketchSearchOptions,
    LiftedSketchSearchOptions,
    Repository as SketchRepository,
    RuleBase,
    SketchProofEdgeLabelData,
    SketchSearchStatistics,
    SuccessorExpander,
    find_ground_solution,
)
from pyrunir.kr.ps.base.dl import (
    ConcreteFeatureBaseBoolean,
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
    vertex = graph.vertex_value(next(iter(graph.vertices())))
    assert isinstance(vertex.state, State)
    assert vertex.state.unpack().pack() == vertex.state
    assert sys.getrefcount(graph) > references
    references = sys.getrefcount(vertex)
    state = vertex.state
    # The state borrows the search context's repository, so it keeps its vertex alive.
    assert sys.getrefcount(vertex) > references
    assert state == vertex.state
    references = sys.getrefcount(graph)
    edge = graph.edge_value(next(iter(range(graph.num_edges))))
    assert sys.getrefcount(graph) > references
    rule = edge.rule
    action = edge.action
    expected = (str(rule), str(action), str(vertex.state.unpack()))

    del edge, graph, proof, sketch, context
    gc.collect()

    assert (str(rule), str(action), str(vertex.state.unpack())) == expected


def test_base_sketch_exposes_declared_features(gripper_planning_domain: PlanningDomain) -> None:
    planning_domain = gripper_planning_domain
    dl_repository = ConstructorRepository(planning_domain)
    sketch_repository = SketchRepository(dl_repository)
    sketch = parse_sketch(read_fixture("kr/ps/base/dl/declared_features.sketch"), planning_domain, sketch_repository)

    assert [feature.variant().symbol() for feature in sketch.boolean_features()] == []
    assert [feature.variant().symbol() for feature in sketch.numerical_features()] == ["n_balls", "n_held"]


def _check_successor_callbacks(
    search: GroundTaskSearchContext, expander: SuccessorExpander, rejecting_expander: SuccessorExpander, node: Node
) -> None:
    """Check that successor callbacks of both expanders filter, count, and stop generation from ``node``."""
    state = node.state()
    expected = planning_module(search).deterministic.get_successors(
        search.successor_generator, search.state_repository, search.axiom_evaluator, node
    )
    accepted = [step for step in expected if step.node().state().pack().index() != state.pack().index()]
    assert accepted
    # An emitted successor is its record through the state repository.
    actual: list[tuple[PackedLabeledNode, RuleBase]] = []
    statistics = SketchSearchStatistics()
    assert statistics.num_generated == 0

    def emit(step: PackedLabeledNode, rule: RuleBase) -> bool:
        assert statistics.num_generated >= len(actual) + 1
        actual.append((step, rule))
        return True

    assert expander.for_each_successor(state, statistics, emit, lambda: False) is True
    generated = len(expected)
    assert statistics.num_generated == generated
    assert Counter((str(step.label()), step.node().state().index()) for step, _ in actual) == Counter(
        (str(step.label()), step.node().state().pack().index()) for step in accepted
    )
    assert all(expander.matching_rule(state, step.node().state().unpack()) == rule for step, rule in actual)
    first_accepted = next(
        i for i, step in enumerate(expected) if step.node().state().pack().index() != state.pack().index()
    )
    assert expander.for_each_successor(state, statistics, lambda _step, _rule: False, lambda: False) is False
    generated += first_accepted + 1
    assert statistics.num_generated == generated

    actual.clear()
    assert expander.for_each_successor(state, statistics, emit, lambda: True) is False
    assert statistics.num_generated == generated
    assert actual == []
    assert rejecting_expander.for_each_successor(state, statistics, emit, lambda: False) is True
    generated += len(expected)
    assert statistics.num_generated == generated
    assert actual == []
    assert all(rejecting_expander.matching_rule(state, step.node().state()) is None for step in expected)

    assert (
        rejecting_expander.for_each_successor(state, statistics, emit, lambda: statistics.num_generated > generated)
        is False
    )
    assert statistics.num_generated == generated + 1
    assert statistics.num_expanded == 0
    assert actual == []


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
    initial = planning_module(search).get_initial_node(search.state_repository, search.axiom_evaluator)
    initial_successors = planning_module(search).deterministic.get_successors(
        search.successor_generator, search.state_repository, search.axiom_evaluator, initial
    )
    assert len(initial_successors) > 1

    for node in (initial, initial_successors[0].node()):
        _check_successor_callbacks(search, expander, rejecting_expander, node)


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
    assert empty_sketch.syntactic_complexity() == 0

    sketch = SketchFactory.create(
        SketchSpecification.GRIPPER_FRANCE_ET_AL_AAAI2021,
        planning_domain,
        sketch_repository,
    )
    sketch_description = str(sketch)
    reparsed_sketch = parse_sketch(sketch_description, planning_domain, sketch_repository)
    assert str(reparsed_sketch) == sketch_description
    assert sketch.syntactic_complexity() == 13

    initial_node = planning_module(search_context).get_initial_node(
        search_context.state_repository, search_context.axiom_evaluator
    )
    labeled_successor = planning_module(search_context).deterministic.get_successors(
        search_context.successor_generator,
        search_context.state_repository,
        search_context.axiom_evaluator,
        initial_node,
    )[0]
    source_state = initial_node.state()
    target_state = labeled_successor.node().state()
    dl_builder = task_context.dl_builder
    dl_denotation_repository = task_context.dl_denotation_repository
    assert dl_builder is task_context.dl_builder
    assert dl_denotation_repository is task_context.dl_denotation_repository
    expander = SuccessorExpander(task_context, sketch)
    expander.matching_rule(source_state, target_state)
    source_storage, target_storage = (
        EvaluationStorage(dl_denotation_repository),
        EvaluationStorage(dl_denotation_repository),
    )
    transition_context = GroundTransitionEvaluationContext(
        source_state, target_state, dl_builder, source_storage, target_storage
    )
    state_context = GroundStateEvaluationContext(source_state, dl_builder, source_storage)
    assert LiftedTransitionEvaluationContext is not None
    assert LiftedStateEvaluationContext is not None

    assert isinstance(base.is_compatible_with(sketch, transition_context), bool)
    rule = next(iter(sketch.rules()))
    assert isinstance(base.is_compatible_with(rule, transition_context), bool)
    condition = next(iter(rule.conditions()))
    assert isinstance(base.dl.is_compatible_with(condition, transition_context), bool)
    feature = condition.feature()
    concrete_feature = feature.variant()
    assert isinstance(concrete_feature, ConcreteFeatureBaseBoolean)
    dl_constructor = concrete_feature.feature()
    dl_denotation = semantics.evaluate(dl_constructor, state_context)
    assert isinstance(base.dl.evaluate(feature, state_context).value(), bool)
    assert base.dl.evaluate(concrete_feature, state_context) == dl_denotation
    assert feature.syntactic_complexity() == concrete_feature.syntactic_complexity()
    assert dl_denotation.index() is not None
    same_sketch = sketch
    assert sketch == same_sketch
    assert not sketch < same_sketch
    assert sketch <= same_sketch
    assert not sketch > same_sketch
    assert sketch >= same_sketch
    for view in (
        sketch,
        rule,
        condition,
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
    assert proof_result.status == ProofStatus.SUCCESS
    assert proof_result.is_successful()
    assert proof_result.deadend_states == []
    assert proof_result.open_states == []
    assert proof_result.cycle == []
    assert isinstance(proof_result.graph, GroundSketchProofGraph)
    assert proof_result.graph.num_vertices > 0
    vertex = next(iter(proof_result.graph.vertices()))
    assert isinstance(proof_result.graph.neighbours(vertex), list)
    if proof_result.graph.num_edges > 0:
        edge = next(iter(range(proof_result.graph.num_edges)))
        assert proof_result.graph.source(edge) in proof_result.graph.vertices()
        assert proof_result.graph.target(edge) in proof_result.graph.vertices()
        assert edge in list(proof_result.graph.out_edges(proof_result.graph.source(edge)))
        edge_label = proof_result.graph.edge_value(edge)
        assert isinstance(edge_label, SketchProofEdgeLabelData)
        assert isinstance(edge_label.action, RowActionLifted)

    search_options = GroundSketchSearchOptions()
    search_result = find_ground_solution(task_context, sketch, search_options)
    assert search_result.status == ProofStatus.SUCCESS
    assert search_result.is_successful()
    assert search_result.deadend_states == []
    assert search_result.open_states == []
    assert search_result.cycle == []

    classifier_dl_repository = task_context.domain_context.uns_repository.get_dl_repository()
    classifier_repository = task_context.domain_context.uns_repository
    classifier = parse_classifier(read_fixture("kr/uns/always.classifier"), planning_domain, classifier_repository)
    classifier_feature = classifier.features()[0]
    concrete_classifier_feature = classifier_feature.variant()
    assert classifier_feature.syntactic_complexity() == concrete_classifier_feature.syntactic_complexity()
    assert (
        concrete_classifier_feature.syntactic_complexity()
        == concrete_classifier_feature.feature().syntactic_complexity()
    )
    assert classifier.syntactic_complexity() == 1
    assert ClassifierFactory.create_empty(classifier_repository).syntactic_complexity() == 0
    classified_options = GroundSketchSearchOptions()
    assert classified_options.classifier is None
    classified_options.classifier = classifier
    lifted_options = LiftedSketchSearchOptions()
    assert lifted_options.classifier is None
    lifted_options.classifier = classifier
    assert lifted_options.classifier.index() == classifier.index()

    del classifier, classifier_repository, classifier_dl_repository
    gc.collect()
    classified_result = find_ground_solution(task_context, sketch, classified_options)
    assert classified_result.status == ProofStatus.FAILURE
    assert classified_result.graph.num_vertices == 1
    assert classified_result.graph.num_edges == 0
    assert len(classified_result.deadend_states) == 1
    assert classified_result.open_states == []
    classified_label = classified_result.graph.vertex_value(classified_result.deadend_states[0])
    assert not classified_label.is_goal
    assert not classified_label.is_alive
    assert classified_label.is_unsolvable
