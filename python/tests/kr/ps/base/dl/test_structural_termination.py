from typing import TypedDict, cast

import pytest

from fixture_utils import load_fixture, read_fixture
from pypddl_datasets import data_root

from pyrunir.kr.dl.base.semantics import ConstructorRepositoryFactory
from pyrunir.kr.ps.base import Repository, RepositoryFactory
from pyrunir.kr.ps.base.dl import (
    BooleanFeature,
    IncompleteStructuralTerminationStatus,
    NumericalChange,
    SccStructuralTerminationResult,
    incomplete_structural_termination,
    parse_sketch,
    structural_termination,
)
from pypddl.formalism import ParserOptions
from pytyr.formalism.planning import Parser, PlanningDomain


class StructuralTerminationFixture(TypedDict):
    name: str
    file: str
    terminating: bool


_SUITE = load_fixture("kr/ps/structural_termination.json")
BASE_CASES = cast(list[StructuralTerminationFixture], _SUITE["base"])
OSCILLATOR = read_fixture("kr/ps/base/dl/oscillator.sketch")
NUMERICAL_CYCLE = read_fixture("kr/ps/base/dl/numerical_cycle.sketch")
TPP = read_fixture("kr/ps/base/dl/tpp.sketch")


def make_repository() -> tuple[PlanningDomain, Repository]:
    domain_path = data_root() / "classical" / "tests" / "gripper" / "domain.pddl"
    planning_domain = Parser(domain_path, ParserOptions()).get_domain()
    dl_repository = ConstructorRepositoryFactory().create(planning_domain)
    repository = RepositoryFactory().create(dl_repository)
    return planning_domain, repository


@pytest.mark.parametrize("case", BASE_CASES, ids=[case["name"] for case in BASE_CASES])
def test_base_structural_termination_fixture(case: StructuralTerminationFixture) -> None:
    domain, repository = make_repository()
    sketch = parse_sketch(read_fixture(case["file"]), domain, repository)

    assert structural_termination(sketch).is_terminating() == case["terminating"]


def test_structural_termination_tpp_sketch_is_terminating() -> None:
    domain, repository = make_repository()
    sketch = parse_sketch(TPP, domain, repository)

    result = structural_termination(sketch)
    numericals = sketch.get_numerical_features()
    without_incomplete = structural_termination(sketch, max_features=3, use_incomplete_preprocessing=False)

    assert result.is_terminating()
    assert result.counterexample is None
    assert result.incomplete_result is not None
    assert result.incomplete_result.status == IncompleteStructuralTerminationStatus.TERMINATING
    assert result.scc_results is None
    assert len(numericals) == 3
    assert without_incomplete.is_terminating()
    assert without_incomplete.incomplete_result is None
    assert without_incomplete.scc_results is not None
    assert len(without_incomplete.scc_results) == 1
    (scc_result,) = without_incomplete.scc_results
    assert isinstance(scc_result, SccStructuralTerminationResult)
    assert scc_result.booleans == []
    assert [feature.get_index() for feature in scc_result.numericals] == [feature.get_index() for feature in numericals]


def test_structural_termination_oscillator_counterexample_has_positional_valuations() -> None:
    domain, repository = make_repository()
    sketch = parse_sketch(OSCILLATOR, domain, repository)

    result = structural_termination(sketch)

    assert not result.is_terminating()
    assert result.incomplete_result is not None
    assert result.incomplete_result.status == IncompleteStructuralTerminationStatus.UNKNOWN
    assert result.scc_results is not None
    (scc_result,) = result.scc_results
    assert isinstance(scc_result, SccStructuralTerminationResult)
    counterexample = result.counterexample
    assert counterexample is not None
    assert counterexample.get_num_vertices() == 2
    assert counterexample.get_num_edges() == 2

    (feature,) = sketch.get_boolean_features()
    assert isinstance(feature, BooleanFeature)
    assert [item.get_index() for item in scc_result.booleans] == [feature.get_index()]
    assert scc_result.numericals == []

    # Positional valuations: one vertex per truth value of b1.
    valuations = {counterexample.get_vertex_property(vertex).boolean_values[0] for vertex in counterexample.get_vertex_indices()}
    assert valuations == {True, False}
    first_vertex = counterexample.get_vertex_property(next(iter(counterexample.get_vertex_indices())))
    assert first_vertex == first_vertex
    assert first_vertex <= first_vertex
    with pytest.raises(TypeError):
        hash(first_vertex)

    # The two-cycle uses both rules; each edge flips b1.
    rules = {counterexample.get_edge_property(edge).rule.get_index() for edge in counterexample.get_edge_indices()}
    assert rules == {rule.get_index() for rule in sketch.get_rules()}
    first_edge = counterexample.get_edge_property(next(iter(counterexample.get_edge_indices())))
    assert first_edge == first_edge
    assert first_edge >= first_edge
    with pytest.raises(TypeError):
        hash(first_edge)
    for edge in counterexample.get_edge_indices():
        source = counterexample.get_vertex_property(counterexample.get_source(edge)).boolean_values[0]
        target = counterexample.get_vertex_property(counterexample.get_target(edge)).boolean_values[0]
        assert source != target


def test_structural_termination_edge_changes_are_positional() -> None:
    domain, repository = make_repository()
    sketch = parse_sketch(NUMERICAL_CYCLE, domain, repository)

    result = structural_termination(sketch)

    assert not result.is_terminating()
    assert len(sketch.get_numerical_features()) == 1
    counterexample = result.counterexample
    assert counterexample is not None
    changes = {counterexample.get_edge_property(edge).numerical_changes[0] for edge in counterexample.get_edge_indices()}
    assert changes == {NumericalChange.DECREASES, NumericalChange.INCREASES}


def test_incomplete_structural_termination_reports_blocking_reasons() -> None:
    domain, repository = make_repository()
    sketch = parse_sketch(OSCILLATOR, domain, repository)

    result = incomplete_structural_termination(sketch)

    assert not result.is_terminating()
    assert len(result.surviving_rules) == 2
    for surviving in result.surviving_rules:
        (reason,) = surviving.blocking_reasons
        assert isinstance(reason.feature, BooleanFeature)
        (opposing,) = reason.opposing_rules
        assert opposing.get_index() != surviving.rule.get_index()
