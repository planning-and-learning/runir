import gc
import sys

import pytest

from fixture_utils import read_fixture
from pyrunir.kr.dl import ext as dl_ext
from pyrunir.kr.ps import ext
from pyrunir.kr.ps.ext import dl
from pytyr.formalism.planning import PlanningDomain

TERMINATING_MODULE = read_fixture("kr/ps/ext/dl/terminating.module")
NON_TERMINATING_MODULE = read_fixture("kr/ps/ext/dl/non_terminating.module")
SEPARATED_BOOLEAN_RULES_MODULE = read_fixture("kr/ps/ext/dl/separated_boolean.module")
PROJECTED_COMPONENTS_MODULE = read_fixture("kr/ps/ext/dl/projected_components.module")


def _repository(planning_domain: PlanningDomain) -> ext.Repository:
    dl_repository = dl_ext.ConstructorRepositoryFactory().create(planning_domain)
    return ext.RepositoryFactory().create(dl_repository)


@pytest.mark.parametrize("vertex_property", [False, True])
def test_counterexample_properties_keep_the_graph_alive(
    gripper_planning_domain: PlanningDomain, vertex_property: bool
) -> None:
    repository = _repository(gripper_planning_domain)
    module = dl.parse_module(NON_TERMINATING_MODULE, gripper_planning_domain, repository)
    result = dl.structural_termination(module)
    sieve_result = result.sieve_result
    assert sieve_result is not None
    graph = sieve_result.counterexample
    assert graph is not None
    references = sys.getrefcount(graph)
    if vertex_property:
        label = graph.get_vertex_property(next(iter(graph.get_vertex_indices())))
        read_property = label.memory_state.get_name
    else:
        label = graph.get_edge_property(next(iter(graph.get_edge_indices())))
        read_property = lambda: str(label)
    assert sys.getrefcount(graph) > references
    expected = read_property()

    del graph, sieve_result, result, module
    gc.collect()

    assert read_property() == expected


def test_sieve_surviving_rule_uses_caller_owned_repository(gripper_planning_domain: PlanningDomain) -> None:
    repository = _repository(gripper_planning_domain)
    module = dl.parse_module(NON_TERMINATING_MODULE, gripper_planning_domain, repository)
    result = dl.structural_termination(module)
    sieve_result = result.sieve_result
    assert sieve_result is not None
    rule = sieve_result.surviving_rules[0]
    expected = str(rule)

    del sieve_result, result, module  # The caller retains the repository owning the rule data.
    gc.collect()

    assert str(rule) == expected


@pytest.mark.parametrize("use_incomplete_preprocessing", [False, True])
def test_empty_module_distinguishes_skipped_sieve_from_empty_sccs(
    gripper_planning_domain: PlanningDomain, use_incomplete_preprocessing: bool
) -> None:
    repository = _repository(gripper_planning_domain)
    module = dl.ModuleFactory.create_empty(repository)

    result = dl.structural_termination(module, use_incomplete_preprocessing=use_incomplete_preprocessing)

    assert result.is_terminating()
    if use_incomplete_preprocessing:
        assert result.incomplete_result is not None
        assert result.incomplete_result.is_terminating()
        assert result.sieve_result is None
    else:
        assert result.incomplete_result is None
        sieve_result = result.sieve_result
        assert isinstance(sieve_result, dl.ModuleSieveStructuralTerminationResult)
        assert sieve_result.is_terminating()
        assert sieve_result.counterexample is None
        assert sieve_result.scc_results == []
        assert sieve_result.surviving_rules == []


@pytest.mark.parametrize("num_booleans,num_numericals", [(0, 64), (0, 65), (32, 32), (32, 33)])
@pytest.mark.parametrize("with_rule", [False, True])
def test_ext_structural_termination_feature_mask_boundary(
    gripper_planning_domain: PlanningDomain, num_booleans: int, num_numericals: int, with_rule: bool
) -> None:
    repository = _repository(gripper_planning_domain)
    features = " ".join(
        f"(:boolean (:symbol b{i}) (:expression (b_nonempty (c_top))))" for i in range(num_booleans)
    ) + " " + " ".join(
        f"(:numerical (:symbol n{i}) (:expression (n_count (c_top))))" for i in range(num_numericals)
    )
    active = f"n{num_numericals - 1}"
    rule = (
        "(:rule (:symbol keep) (:expression (:source-memory m0) (:target-memory m0) "
        f"(:sketch (:conditions (greater_zero {active})) (:effects (unchanged {active})))))"
        if with_rule
        else ""
    )
    module = dl.parse_module(
        "(:module (:symbol boundary) (:arguments) (:registers) (:entry m0) (:memory m0) "
        f"(:features {features}) (:rules {rule}))", gripper_planning_domain, repository
    )
    assert len(module.get_boolean_features()) == num_booleans
    assert len(module.get_numerical_features()) == num_numericals

    if num_booleans + num_numericals > 64:
        with pytest.raises(ValueError, match="64"):
            dl.incomplete_structural_termination(module)
        for preprocessing in (False, True):
            with pytest.raises(ValueError, match="64"):
                dl.structural_termination(module, use_incomplete_preprocessing=preprocessing)
        return

    assert dl.incomplete_structural_termination(module).is_terminating() == (not with_rule)
    for preprocessing in (False, True):
        result = dl.structural_termination(module, use_incomplete_preprocessing=preprocessing)
        assert result.is_terminating() == (not with_rule)
        if with_rule:
            sieve_result = result.sieve_result
            assert sieve_result is not None
            (scc_result,) = sieve_result.scc_results
            assert scc_result.booleans == []
            assert len(scc_result.numericals) == 1
            counterexample = sieve_result.counterexample
            assert counterexample is not None
            (vertex,) = [counterexample.get_vertex_property(index) for index in counterexample.get_vertex_indices()]
            assert vertex.boolean_values == 0
            assert type(vertex.numerical_values) is int
            assert vertex.numerical_values == 1 << (num_numericals - 1)


@pytest.mark.parametrize(
    "effects",
    ["(increases n) (decreases n)", "(increases n) (unchanged n)", "(decreases n) (unchanged n)"],
)
def test_ext_structural_termination_rejects_conflicting_numerical_effects(
    gripper_planning_domain: PlanningDomain, effects: str
) -> None:
    repository = _repository(gripper_planning_domain)
    module = dl.parse_module(
        "(:module (:symbol conflict) (:arguments) (:registers) (:entry m0) (:memory m0) "
        "(:features (:numerical (:symbol n) (:expression (n_count (c_top))))) "
        "(:rules (:rule (:symbol conflict) (:expression (:source-memory m0) (:target-memory m0) "
        f"(:sketch (:conditions) (:effects {effects}))))))",
        gripper_planning_domain,
        repository,
    )

    with pytest.raises(ValueError, match="contradictory numerical effects"):
        dl.incomplete_structural_termination(module)
    for preprocessing in (False, True):
        with pytest.raises(ValueError, match="contradictory numerical effects"):
            dl.structural_termination(module, use_incomplete_preprocessing=preprocessing)


def test_ext_structural_termination_is_terminating(gripper_planning_domain: PlanningDomain) -> None:
    repository = _repository(gripper_planning_domain)
    module = dl.parse_module(TERMINATING_MODULE, gripper_planning_domain, repository)

    incomplete_result = dl.incomplete_structural_termination(module)
    result = dl.structural_termination(module)
    without_incomplete = dl.structural_termination(module, max_features=1, use_incomplete_preprocessing=False)

    assert result.is_terminating()
    assert len(module.get_boolean_features()) == 1
    assert result.incomplete_result is not None
    assert result.incomplete_result.status == incomplete_result.status
    assert incomplete_result.surviving_rules == []
    assert result.sieve_result is None
    assert without_incomplete.is_terminating()
    assert without_incomplete.incomplete_result is None
    sieve_result = without_incomplete.sieve_result
    assert sieve_result is not None
    assert sieve_result.is_terminating()
    assert sieve_result.counterexample is None
    assert sieve_result.surviving_rules == []
    (scc_result,) = sieve_result.scc_results
    assert isinstance(scc_result, dl.SccStructuralTerminationResult)
    assert scc_result.booleans == []
    assert [feature.get_index() for feature in scc_result.numericals] == [
        feature.get_index() for feature in module.get_numerical_features()
    ]


def test_ext_structural_termination_counterexample_spans_memory_states(gripper_planning_domain: PlanningDomain) -> None:
    repository = _repository(gripper_planning_domain)
    module = dl.parse_module(NON_TERMINATING_MODULE, gripper_planning_domain, repository)

    result = dl.structural_termination(module)
    booleans = module.get_boolean_features()
    numericals = module.get_numerical_features()
    assert not result.is_terminating()
    sieve_result = result.sieve_result
    assert sieve_result is not None
    assert not sieve_result.is_terminating()
    counterexample = sieve_result.counterexample
    assert counterexample is not None
    vertices = [counterexample.get_vertex_property(vertex) for vertex in counterexample.get_vertex_indices()]
    assert {vertex.memory_state.get_name() for vertex in vertices} == {"m0", "m1"}
    assert vertices[0] == vertices[0]
    assert vertices[0] <= vertices[0]
    with pytest.raises(TypeError):
        hash(vertices[0])
    for vertex in vertices:
        assert 0 <= vertex.boolean_values < 1 << len(booleans)
        assert 0 <= vertex.numerical_values < 1 << len(numericals)

    edges = [counterexample.get_edge_property(edge) for edge in counterexample.get_edge_indices()]
    assert len({edge.get_index() for edge in edges}) == 2
    cycle = counterexample.find_edge_cycle()
    assert len(cycle) == 2
    assert len(set(cycle)) == 2
    for edge, next_edge in zip(cycle, cycle[1:] + cycle[:1]):
        assert counterexample.get_target(edge) == counterexample.get_source(next_edge)


@pytest.mark.parametrize("use_incomplete_preprocessing", [False, True])
def test_ext_structural_termination_lifts_projected_components(
    gripper_planning_domain: PlanningDomain, use_incomplete_preprocessing: bool
) -> None:
    repository = _repository(gripper_planning_domain)
    module = dl.parse_module(PROJECTED_COMPONENTS_MODULE, gripper_planning_domain, repository)

    result = dl.structural_termination(module, use_incomplete_preprocessing=use_incomplete_preprocessing)
    booleans = module.get_boolean_features()
    numericals = module.get_numerical_features()
    assert not result.is_terminating()
    sieve_result = result.sieve_result
    assert sieve_result is not None
    counterexample = sieve_result.counterexample
    assert counterexample is not None
    assert len(booleans) == 1
    assert len(numericals) == 2

    feature_sets = {
        (
            tuple(feature.get_index() for feature in scc_result.booleans),
            tuple(feature.get_index() for feature in scc_result.numericals),
        )
        for scc_result in sieve_result.scc_results
    }
    assert feature_sets == {
        ((booleans[0].get_index(),), ()),
        ((), (numericals[0].get_index(),)),
        ((), (numericals[1].get_index(),)),
    }

    vertices = [counterexample.get_vertex_property(vertex) for vertex in counterexample.get_vertex_indices()]
    assert {vertex.memory_state.get_name() for vertex in vertices} == {"m0", "m1", "m2"}
    positive_n0 = False
    positive_n1 = False
    for vertex in vertices:
        assert 0 <= vertex.boolean_values < 1 << len(booleans)
        assert 0 <= vertex.numerical_values < 1 << len(numericals)
        if vertex.memory_state.get_name() == "m0":
            assert vertex.numerical_values == 0
        elif vertex.memory_state.get_name() == "m1":
            assert vertex.boolean_values == 0
            assert not (vertex.numerical_values & 1)
            positive_n1 |= bool(vertex.numerical_values & 2)
        else:
            assert vertex.boolean_values == 0
            assert not (vertex.numerical_values & 2)
            positive_n0 |= bool(vertex.numerical_values & 1)
    assert positive_n0
    assert positive_n1

    edges = [counterexample.get_edge_property(edge) for edge in counterexample.get_edge_indices()]
    assert {edge.get_symbol() for edge in edges} == {"keep_n0", "keep_n1", "to_false", "to_true"}
    surviving_rules = sieve_result.surviving_rules
    surviving_rule_ids = {rule.get_index() for rule in surviving_rules}
    assert len(surviving_rules) == len(surviving_rule_ids) == 4
    assert surviving_rule_ids == {edge.get_index() for edge in edges}
    with pytest.raises(AttributeError):
        setattr(sieve_result, "surviving_rules", [])

    cycle = counterexample.find_edge_cycle()
    assert cycle
    assert len(cycle) == len(set(cycle))
    assert set(cycle) <= set(counterexample.get_edge_indices())
    for edge, next_edge in zip(cycle, cycle[1:] + cycle[:1]):
        assert counterexample.get_target(edge) == counterexample.get_source(next_edge)
    assert {counterexample.get_edge_property(edge).get_index() for edge in cycle} < surviving_rule_ids


def test_ext_incomplete_structural_termination_uses_memory_components(gripper_planning_domain: PlanningDomain) -> None:
    repository = _repository(gripper_planning_domain)
    module = dl.parse_module(SEPARATED_BOOLEAN_RULES_MODULE, gripper_planning_domain, repository)

    result = dl.incomplete_structural_termination(module)
    assert result.is_terminating()
    assert result.status == dl.IncompleteStructuralTerminationStatus.TERMINATING
    assert len(module.get_boolean_features()) == 1
    assert module.get_numerical_features() == []
    assert result.surviving_rules == []

    global_result = dl.incomplete_structural_termination(module, use_memory_scc_scope=False)
    assert not global_result.is_terminating()
    assert len(global_result.surviving_rules) == 2

    combined_result = dl.structural_termination(module, use_memory_scc_scope=False)
    assert dl.structural_termination(module).is_terminating()
    assert combined_result.incomplete_result is not None
    assert len(combined_result.incomplete_result.surviving_rules) == 2
    assert combined_result.is_terminating()
    sieve_result = combined_result.sieve_result
    assert sieve_result is not None
    assert sieve_result.counterexample is None
    assert len(sieve_result.scc_results) == 2
    assert sieve_result.surviving_rules == []


def test_ext_incomplete_structural_termination_accepts_module_program(gripper_planning_domain: PlanningDomain) -> None:
    repository = _repository(gripper_planning_domain)
    program = dl.parse_module_program(
        read_fixture("kr/ps/ext/dl/non_terminating.program"), gripper_planning_domain, repository
    )

    result = dl.incomplete_structural_termination(program)
    complete_result = dl.structural_termination(program, max_features=1, use_incomplete_preprocessing=False)
    assert not complete_result.is_terminating()
    assert not result.is_terminating()
    assert len(result.module_results) == 1
    assert result.recursive_call_rules == []
