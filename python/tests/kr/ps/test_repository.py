import pytest

from fixture_utils import read_fixture
from pyrunir.datasets import GroundTaskClass
from pyrunir.kr import DomainContext
from pyrunir.kr.dl.base.semantics import ConstructorRepositoryFactory as BaseDlRepositoryFactory
from pyrunir.kr.dl.ext import ConstructorRepositoryFactory as ExtDlRepositoryFactory
from pyrunir.kr.ps import base, ext
from pyrunir.kr.uns import dl as uns_dl
from pytyr.formalism.planning import PlanningDomain


def test_base_and_ext_repositories_construct_programmatically(gripper_planning_domain: PlanningDomain):
    base_repository = base.RepositoryFactory().create(BaseDlRepositoryFactory().create(gripper_planning_domain))
    rule_data = base.RuleData()
    rule_data.symbol = "r"
    rule = base_repository.get_or_create(rule_data)
    assert rule.get_symbol() == "r"
    assert base_repository.get_or_create(rule_data) == rule

    ext_repository = ext.RepositoryFactory().create(ExtDlRepositoryFactory().create(gripper_planning_domain))
    memory_state_data = ext.MemoryStateData()
    memory_state_data.name = "m0"
    memory_state = ext_repository.get_or_create(memory_state_data)
    assert memory_state.get_name() == "m0"
    assert ext_repository.get_or_create(memory_state_data) == memory_state


def test_uns_features_construct_programmatically(gripper_planning_domain: PlanningDomain):
    repository = DomainContext(gripper_planning_domain).uns_repository
    classifier = uns_dl.parse_classifier(read_fixture("kr/uns/positive.classifier"), gripper_planning_domain, repository)
    feature = classifier.get_features()[0]
    concrete_data = uns_dl.ConcreteBooleanFeatureData()
    concrete_data.feature = feature.get_expression().get_index()
    concrete_data.symbol = feature.get_symbol()
    concrete = repository.get_or_create(concrete_data)
    assert concrete == feature.get_variant()
    data = uns_dl.BooleanFeatureData()
    data.value = concrete.get_index()
    assert repository.get_or_create(data) == feature


def test_data_rich_comparison_is_bound():
    first = base.RuleData()
    first.symbol = "a"
    second = base.RuleData()
    second.symbol = "b"

    assert first == first
    assert first != second
    assert first < second
    assert first <= first
    assert second > first
    assert second >= second
    with pytest.raises(TypeError):
        hash(first)


def test_task_class_rich_comparison_is_bound():
    first = GroundTaskClass()
    second = GroundTaskClass()

    assert first == second
    assert first <= second
    assert first >= second
    with pytest.raises(TypeError):
        hash(first)
