import gc
import sys
from typing import TypedDict, cast

import pytest

from fixture_utils import load_fixture, read_fixture
from pyrunir.kr.dl.base.semantics import ConstructorRepositoryFactory
from pyrunir.kr.ps.base import Repository, RepositoryFactory
from pyrunir.kr.ps.base.dl import (
    BooleanFeature,
    IncompleteStructuralTerminationStatus,
    SccStructuralTerminationResult,
    SieveStructuralTerminationResult,
    SketchFactory,
    incomplete_structural_termination,
    parse_sketch,
    structural_termination,
)
from pytyr.formalism.planning import PlanningDomain


class StructuralTerminationFixture(TypedDict):
    name: str
    file: str
    terminating: bool


_SUITE = load_fixture("kr/ps/structural_termination.json")
BASE_CASES = cast(list[StructuralTerminationFixture], _SUITE["base"])
OSCILLATOR = read_fixture("kr/ps/base/dl/oscillator.sketch")
NUMERICAL_CYCLE = read_fixture("kr/ps/base/dl/numerical_cycle.sketch")
TPP = read_fixture("kr/ps/base/dl/tpp.sketch")


def make_repository(planning_domain: PlanningDomain) -> Repository:
    dl_repository = ConstructorRepositoryFactory().create(planning_domain)
    return RepositoryFactory().create(dl_repository)


def test_counterexample_rule_keeps_the_graph_alive(gripper_planning_domain: PlanningDomain) -> None:
    repository = make_repository(gripper_planning_domain)
    sketch = parse_sketch(OSCILLATOR, gripper_planning_domain, repository)
    result = structural_termination(sketch)
    sieve_result = result.sieve_result
    assert sieve_result is not None
    graph = sieve_result.counterexample
    assert graph is not None
    references = sys.getrefcount(graph)
    rule = graph.get_edge_property(next(iter(graph.get_edge_indices())))
    assert sys.getrefcount(graph) > references
    expected = str(rule)

    del graph, sieve_result, result, sketch
    gc.collect()

    assert str(rule) == expected


def test_sieve_surviving_rule_uses_caller_owned_repository(gripper_planning_domain: PlanningDomain) -> None:
    repository = make_repository(gripper_planning_domain)
    sketch = parse_sketch(OSCILLATOR, gripper_planning_domain, repository)
    result = structural_termination(sketch)
    sieve_result = result.sieve_result
    assert sieve_result is not None
    rule = sieve_result.surviving_rules[0]
    expected = str(rule)

    del sieve_result, result, sketch  # The caller retains the repository owning the rule data.
    gc.collect()

    assert str(rule) == expected


@pytest.mark.parametrize("case", BASE_CASES, ids=[case["name"] for case in BASE_CASES])
def test_base_structural_termination_fixture(case: StructuralTerminationFixture, gripper_planning_domain: PlanningDomain) -> None:
    repository = make_repository(gripper_planning_domain)
    sketch = parse_sketch(read_fixture(case["file"]), gripper_planning_domain, repository)

    assert structural_termination(sketch).is_terminating() == case["terminating"]


@pytest.mark.parametrize("use_incomplete_preprocessing", [False, True])
def test_empty_sketch_distinguishes_skipped_sieve_from_empty_sccs(
    gripper_planning_domain: PlanningDomain, use_incomplete_preprocessing: bool
) -> None:
    repository = make_repository(gripper_planning_domain)
    sketch = SketchFactory.create_empty(repository)

    result = structural_termination(sketch, use_incomplete_preprocessing=use_incomplete_preprocessing)

    assert result.is_terminating()
    if use_incomplete_preprocessing:
        assert result.incomplete_result is not None
        assert result.incomplete_result.is_terminating()
        assert result.sieve_result is None
    else:
        assert result.incomplete_result is None
        sieve_result = result.sieve_result
        assert isinstance(sieve_result, SieveStructuralTerminationResult)
        assert sieve_result.is_terminating()
        assert sieve_result.counterexample is None
        assert sieve_result.scc_results == []
        assert sieve_result.surviving_rules == []


@pytest.mark.parametrize("num_booleans,num_numericals", [(64, 0), (65, 0), (32, 32), (32, 33)])
@pytest.mark.parametrize("with_rule", [False, True])
def test_structural_termination_feature_mask_boundary(
    gripper_planning_domain: PlanningDomain, num_booleans: int, num_numericals: int, with_rule: bool
) -> None:
    repository = make_repository(gripper_planning_domain)
    features = " ".join(
        f"(:boolean (:symbol b{i}) (:expression (b_nonempty (c_top))))" for i in range(num_booleans)
    ) + " " + " ".join(
        f"(:numerical (:symbol n{i}) (:expression (n_count (c_top))))" for i in range(num_numericals)
    )
    active = f"b{num_booleans - 1}"
    rule = (
        f"(:rule (:symbol keep) (:expression (:conditions (positive {active})) (:effects (unchanged {active}))))"
        if with_rule
        else ""
    )
    sketch = parse_sketch(f"(:sketch (:features {features}) (:rules {rule}))", gripper_planning_domain, repository)
    assert len(sketch.get_boolean_features()) == num_booleans
    assert len(sketch.get_numerical_features()) == num_numericals

    if num_booleans + num_numericals > 64:
        with pytest.raises(ValueError, match="64"):
            incomplete_structural_termination(sketch)
        for preprocessing in (False, True):
            with pytest.raises(ValueError, match="64"):
                structural_termination(sketch, use_incomplete_preprocessing=preprocessing)
        return

    assert incomplete_structural_termination(sketch).is_terminating() == (not with_rule)
    for preprocessing in (False, True):
        result = structural_termination(sketch, use_incomplete_preprocessing=preprocessing)
        assert result.is_terminating() == (not with_rule)
        if with_rule:
            sieve_result = result.sieve_result
            assert sieve_result is not None
            (scc_result,) = sieve_result.scc_results
            assert len(scc_result.booleans) == 1
            assert scc_result.numericals == []
            counterexample = sieve_result.counterexample
            assert counterexample is not None
            (vertex,) = [counterexample.get_vertex_property(index) for index in counterexample.get_vertex_indices()]
            assert type(vertex.boolean_values) is int
            assert vertex.boolean_values == 1 << (num_booleans - 1)
            assert vertex.numerical_values == 0


@pytest.mark.parametrize(
    "effects",
    ["(increases n) (decreases n)", "(increases n) (unchanged n)", "(decreases n) (unchanged n)"],
)
def test_structural_termination_rejects_conflicting_numerical_effects(
    gripper_planning_domain: PlanningDomain, effects: str
) -> None:
    repository = make_repository(gripper_planning_domain)
    sketch = parse_sketch(
        "(:sketch (:features (:numerical (:symbol n) (:expression (n_count (c_top))))) "
        f"(:rules (:rule (:symbol conflict) (:expression (:conditions) (:effects {effects})))))",
        gripper_planning_domain,
        repository,
    )

    with pytest.raises(ValueError, match="contradictory numerical effects"):
        incomplete_structural_termination(sketch)
    for preprocessing in (False, True):
        with pytest.raises(ValueError, match="contradictory numerical effects"):
            structural_termination(sketch, use_incomplete_preprocessing=preprocessing)


def test_structural_termination_tpp_sketch_is_terminating(gripper_planning_domain: PlanningDomain) -> None:
    repository = make_repository(gripper_planning_domain)
    sketch = parse_sketch(TPP, gripper_planning_domain, repository)

    result = structural_termination(sketch)
    numericals = sketch.get_numerical_features()
    without_incomplete = structural_termination(sketch, max_features=3, use_incomplete_preprocessing=False)

    assert result.is_terminating()
    assert result.incomplete_result is not None
    assert result.incomplete_result.status == IncompleteStructuralTerminationStatus.TERMINATING
    assert result.incomplete_result.surviving_rules == []
    assert result.sieve_result is None
    assert len(numericals) == 3
    assert without_incomplete.is_terminating()
    assert without_incomplete.incomplete_result is None
    sieve_result = without_incomplete.sieve_result
    assert sieve_result is not None
    assert sieve_result.is_terminating()
    assert sieve_result.counterexample is None
    assert sieve_result.surviving_rules == []
    (scc_result,) = sieve_result.scc_results
    assert isinstance(scc_result, SccStructuralTerminationResult)
    assert scc_result.booleans == []
    assert [feature.get_index() for feature in scc_result.numericals] == [feature.get_index() for feature in numericals]


def test_structural_termination_oscillator_counterexample_has_positional_valuations(gripper_planning_domain: PlanningDomain) -> None:
    repository = make_repository(gripper_planning_domain)
    sketch = parse_sketch(OSCILLATOR, gripper_planning_domain, repository)

    result = structural_termination(sketch)

    assert not result.is_terminating()
    assert result.incomplete_result is not None
    assert result.incomplete_result.status == IncompleteStructuralTerminationStatus.UNKNOWN
    sieve_result = result.sieve_result
    assert sieve_result is not None
    (scc_result,) = sieve_result.scc_results
    assert isinstance(scc_result, SccStructuralTerminationResult)
    counterexample = sieve_result.counterexample
    assert counterexample is not None
    assert counterexample.get_num_vertices() == 2
    assert counterexample.get_num_edges() == 2

    (feature,) = sketch.get_boolean_features()
    assert isinstance(feature, BooleanFeature)
    assert [item.get_index() for item in scc_result.booleans] == [feature.get_index()]
    assert scc_result.numericals == []

    # Positional valuations: one vertex per truth value of b1.
    valuations = {counterexample.get_vertex_property(vertex).boolean_values for vertex in counterexample.get_vertex_indices()}
    assert valuations == {0, 1}
    first_vertex = counterexample.get_vertex_property(next(iter(counterexample.get_vertex_indices())))
    assert type(first_vertex.boolean_values) is int
    assert type(first_vertex.numerical_values) is int
    assert first_vertex == first_vertex
    assert first_vertex <= first_vertex
    with pytest.raises(TypeError):
        hash(first_vertex)

    # The two-cycle uses both rules; each edge flips b1.
    rules = {counterexample.get_edge_property(edge).get_index() for edge in counterexample.get_edge_indices()}
    assert rules == {rule.get_index() for rule in sketch.get_rules()}
    for edge in counterexample.get_edge_indices():
        source = counterexample.get_vertex_property(counterexample.get_source(edge)).boolean_values & 1
        target = counterexample.get_vertex_property(counterexample.get_target(edge)).boolean_values & 1
        assert source != target


@pytest.mark.parametrize("use_incomplete_preprocessing", [False, True])
def test_structural_termination_numerical_cycle_counterexample_uses_both_rules(
    gripper_planning_domain: PlanningDomain, use_incomplete_preprocessing: bool
) -> None:
    repository = make_repository(gripper_planning_domain)
    sketch = parse_sketch(NUMERICAL_CYCLE, gripper_planning_domain, repository)

    result = structural_termination(sketch, use_incomplete_preprocessing=use_incomplete_preprocessing)

    assert not result.is_terminating()
    assert len(sketch.get_numerical_features()) == 1
    sieve_result = result.sieve_result
    assert sieve_result is not None
    assert not sieve_result.is_terminating()
    counterexample = sieve_result.counterexample
    assert counterexample is not None
    rules = {counterexample.get_edge_property(edge).get_index() for edge in counterexample.get_edge_indices()}
    assert rules == {rule.get_index() for rule in sketch.get_rules()}
    surviving_rules = sieve_result.surviving_rules
    assert len(surviving_rules) == len(rules)
    assert {rule.get_index() for rule in surviving_rules} == rules
    assert counterexample.get_num_edges() > len(surviving_rules)
    with pytest.raises(AttributeError):
        setattr(sieve_result, "surviving_rules", [])

    cycle = counterexample.find_edge_cycle()
    assert cycle
    assert len(cycle) == len(set(cycle))
    assert set(cycle) <= set(counterexample.get_edge_indices())
    for edge, next_edge in zip(cycle, cycle[1:] + cycle[:1]):
        assert counterexample.get_target(edge) == counterexample.get_source(next_edge)


def test_incomplete_structural_termination_reports_blocking_reasons(gripper_planning_domain: PlanningDomain) -> None:
    repository = make_repository(gripper_planning_domain)
    sketch = parse_sketch(OSCILLATOR, gripper_planning_domain, repository)

    result = incomplete_structural_termination(sketch)

    assert not result.is_terminating()
    assert len(result.surviving_rules) == 2
    assert {surviving.rule.get_index() for surviving in result.surviving_rules} == {rule.get_index() for rule in sketch.get_rules()}
    for surviving in result.surviving_rules:
        (reason,) = surviving.blocking_reasons
        assert isinstance(reason.feature, BooleanFeature)
        (opposing,) = reason.opposing_rules
        assert opposing.get_index() != surviving.rule.get_index()
