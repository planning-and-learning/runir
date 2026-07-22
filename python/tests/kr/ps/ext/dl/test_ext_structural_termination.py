from typing import TypedDict, cast

import pytest

from fixture_utils import load_fixture, read_fixture
from pypddl.formalism import ParserOptions
from pypddl_datasets import data_root
from pyrunir.kr.dl import ext as dl_ext
from pyrunir.kr.ps import ext
from pyrunir.kr.ps.base.dl import NumericalChange
from pyrunir.kr.ps.ext import dl
from pytyr.formalism.planning import Parser, PlanningDomain


class StructuralTerminationFixture(TypedDict):
    name: str
    file: str
    terminating: bool


_SUITE = load_fixture("kr/ps/structural_termination.json")
EXT_CASES = cast(list[StructuralTerminationFixture], _SUITE["ext"])
TERMINATING_MODULE = read_fixture("kr/ps/ext/dl/terminating.module")
NON_TERMINATING_MODULE = read_fixture("kr/ps/ext/dl/non_terminating.module")
SEPARATED_BOOLEAN_RULES_MODULE = read_fixture("kr/ps/ext/dl/separated_boolean.module")
PROJECTED_COMPONENTS_MODULE = read_fixture("kr/ps/ext/dl/projected_components.module")


def _repositories() -> tuple[PlanningDomain, ext.Repository]:
    planning_domain = Parser(data_root() / "classical/tests/gripper/domain.pddl", ParserOptions()).get_domain()
    dl_repository = dl_ext.ConstructorRepositoryFactory().create(planning_domain)
    repository = ext.RepositoryFactory().create(dl_repository)
    return planning_domain, repository


@pytest.mark.parametrize("case", EXT_CASES, ids=[case["name"] for case in EXT_CASES])
def test_ext_structural_termination_fixture(case: StructuralTerminationFixture) -> None:
    planning_domain, repository = _repositories()
    module = dl.parse_module(read_fixture(case["file"]), planning_domain, repository)

    assert dl.structural_termination(module).is_terminating() == case["terminating"]


def test_ext_structural_termination_is_terminating() -> None:
    planning_domain, repository = _repositories()
    module = dl.parse_module(TERMINATING_MODULE, planning_domain, repository)

    incomplete_result = dl.incomplete_structural_termination(module)
    result = dl.structural_termination(module)
    without_incomplete = dl.structural_termination(module, max_features=1, use_incomplete_preprocessing=False)

    assert result.is_terminating()
    assert len(module.get_boolean_features()) == 1
    assert result.counterexample is None
    assert result.incomplete_result is not None
    assert result.incomplete_result.status == incomplete_result.status
    assert result.scc_results is None
    assert without_incomplete.is_terminating()
    assert without_incomplete.incomplete_result is None
    assert without_incomplete.scc_results is not None
    (scc_result,) = without_incomplete.scc_results
    assert isinstance(scc_result, dl.SccStructuralTerminationResult)
    assert scc_result.booleans == []
    assert [feature.get_index() for feature in scc_result.numericals] == [
        feature.get_index() for feature in module.get_numerical_features()
    ]


def test_ext_structural_termination_counterexample_spans_memory_states() -> None:
    planning_domain, repository = _repositories()
    module = dl.parse_module(NON_TERMINATING_MODULE, planning_domain, repository)

    result = dl.structural_termination(module)
    booleans = module.get_boolean_features()
    numericals = module.get_numerical_features()
    assert not result.is_terminating()
    counterexample = result.counterexample
    assert counterexample is not None
    (feature,) = numericals

    vertices = [counterexample.get_vertex_property(vertex) for vertex in counterexample.get_vertex_indices()]
    assert {vertex.memory_state.get_name() for vertex in vertices} == {"m0", "m1"}
    assert vertices[0] == vertices[0]
    assert vertices[0] <= vertices[0]
    with pytest.raises(TypeError):
        hash(vertices[0])
    for vertex in vertices:
        assert len(vertex.boolean_values) == len(booleans)
        assert len(vertex.numerical_values) == len(numericals)

    edges = [counterexample.get_edge_property(edge) for edge in counterexample.get_edge_indices()]
    assert len({edge.rule.get_index() for edge in edges}) == 2
    assert edges[0] == edges[0]
    assert edges[0] >= edges[0]
    with pytest.raises(TypeError):
        hash(edges[0])
    changes = {dict(zip(numericals, edge.numerical_changes))[feature] for edge in edges}
    assert changes == {NumericalChange.UNCHANGED}


def test_ext_structural_termination_lifts_projected_components() -> None:
    planning_domain, repository = _repositories()
    module = dl.parse_module(PROJECTED_COMPONENTS_MODULE, planning_domain, repository)

    result = dl.structural_termination(module)
    booleans = module.get_boolean_features()
    numericals = module.get_numerical_features()
    assert not result.is_terminating()
    assert result.counterexample is not None
    assert result.scc_results is not None
    assert len(booleans) == 1
    assert len(numericals) == 2

    feature_sets = {
        (
            tuple(feature.get_index() for feature in scc_result.booleans),
            tuple(feature.get_index() for feature in scc_result.numericals),
        )
        for scc_result in result.scc_results
    }
    assert feature_sets == {
        ((booleans[0].get_index(),), ()),
        ((), (numericals[0].get_index(),)),
        ((), (numericals[1].get_index(),)),
    }

    vertices = [result.counterexample.get_vertex_property(vertex) for vertex in result.counterexample.get_vertex_indices()]
    assert {vertex.memory_state.get_name() for vertex in vertices} == {"m0", "m1", "m2"}
    positive_n0 = False
    positive_n1 = False
    for vertex in vertices:
        assert len(vertex.boolean_values) == len(booleans)
        assert len(vertex.numerical_values) == len(numericals)
        if vertex.memory_state.get_name() == "m0":
            assert not any(vertex.numerical_values)
        elif vertex.memory_state.get_name() == "m1":
            assert not any(vertex.boolean_values)
            assert not vertex.numerical_values[0]
            positive_n1 |= vertex.numerical_values[1]
        else:
            assert not any(vertex.boolean_values)
            assert not vertex.numerical_values[1]
            positive_n0 |= vertex.numerical_values[0]
    assert positive_n0
    assert positive_n1

    edges = [result.counterexample.get_edge_property(edge) for edge in result.counterexample.get_edge_indices()]
    assert {edge.rule.get_symbol() for edge in edges} == {"keep_n0", "keep_n1", "to_false", "to_true"}
    assert all(len(edge.numerical_changes) == len(numericals) for edge in edges)
    expected_changes = {
        "keep_n0": [NumericalChange.UNCHANGED, NumericalChange.UNCONSTRAINED],
        "keep_n1": [NumericalChange.UNCONSTRAINED, NumericalChange.UNCHANGED],
        "to_false": [NumericalChange.UNCONSTRAINED, NumericalChange.UNCONSTRAINED],
        "to_true": [NumericalChange.UNCONSTRAINED, NumericalChange.UNCONSTRAINED],
    }
    for edge in edges:
        assert edge.numerical_changes == expected_changes[edge.rule.get_symbol()]


def test_ext_incomplete_structural_termination_uses_memory_components() -> None:
    planning_domain, repository = _repositories()
    module = dl.parse_module(SEPARATED_BOOLEAN_RULES_MODULE, planning_domain, repository)

    result = dl.incomplete_structural_termination(module)
    assert result.is_terminating()
    assert result.status == dl.IncompleteStructuralTerminationStatus.TERMINATING
    assert len(module.get_boolean_features()) == 1
    assert module.get_numerical_features() == []
    assert result.surviving_rules == []


def test_ext_incomplete_structural_termination_accepts_module_program() -> None:
    planning_domain, repository = _repositories()
    program = dl.parse_module_program(
        read_fixture("kr/ps/ext/dl/non_terminating.program"), planning_domain, repository
    )

    result = dl.incomplete_structural_termination(program)
    complete_result = dl.structural_termination(program, max_features=1, use_incomplete_preprocessing=False)
    assert not complete_result.is_terminating()
    assert not result.is_terminating()
    assert len(result.module_results) == 1
    assert result.recursive_call_rules == []
