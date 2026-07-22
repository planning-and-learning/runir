import pytest

from pypddl.formalism import ParserOptions
from pypddl_datasets import data_root
from pyrunir.datasets import GroundTaskClass
from pyrunir.kr.dl.base.semantics import ConstructorRepositoryFactory as BaseDlRepositoryFactory
from pyrunir.kr.dl.ext import ConstructorRepositoryFactory as ExtDlRepositoryFactory
from pyrunir.kr.ps import base, ext
from pyrunir.kr.uns import dl as uns_dl
from pytyr.formalism.planning import Parser


def test_base_and_ext_repositories_construct_programmatically():
    domain = Parser(data_root() / "classical/tests/gripper/domain.pddl", ParserOptions()).get_domain()

    base_repository = base.RepositoryFactory().create(BaseDlRepositoryFactory().create(domain))
    rule_data = base.RuleData()
    rule_data.symbol = "r"
    rule = base_repository.get_or_create(rule_data)
    assert rule.get_symbol() == "r"
    assert base_repository.get_or_create(rule_data) == rule

    ext_repository = ext.RepositoryFactory().create(ExtDlRepositoryFactory().create(domain))
    memory_state_data = ext.MemoryStateData()
    memory_state_data.name = "m0"
    memory_state = ext_repository.get_or_create(memory_state_data)
    assert memory_state.get_name() == "m0"
    assert ext_repository.get_or_create(memory_state_data) == memory_state


def test_uns_dl_feature_data_is_bound():
    data = uns_dl.FeatureData()
    assert data.index is not None


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
