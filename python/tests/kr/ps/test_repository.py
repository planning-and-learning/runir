from pathlib import Path

from pypddl.formalism import ParserOptions
from pyrunir.kr.dl.base.semantics import ConstructorRepositoryFactory as BaseDlRepositoryFactory
from pyrunir.kr.dl.ext import ConstructorRepositoryFactory as ExtDlRepositoryFactory
from pyrunir.kr.ps import base, ext
from pyrunir.kr.uns import dl as uns_dl
from pytyr.formalism.planning import Parser


def test_base_and_ext_repositories_construct_programmatically():
    root = Path(__file__).resolve().parents[4]
    domain = Parser(root / "data/planning-benchmarks/tests/classical/gripper/domain.pddl", ParserOptions()).get_domain()

    base_repository = base.RepositoryFactory().create(BaseDlRepositoryFactory().create(domain))
    rule_data = base.RuleData()
    rule_data.symbol = "r"
    rule, created = base_repository.get_or_create(rule_data)
    assert created and rule.get_symbol() == "r"
    assert not base_repository.get_or_create(rule_data)[1]

    ext_repository = ext.RepositoryFactory().create(ExtDlRepositoryFactory().create(domain))
    memory_state_data = ext.MemoryStateData()
    memory_state_data.name = "m0"
    memory_state, created = ext_repository.get_or_create(memory_state_data)
    assert created and memory_state.get_name() == "m0"
    assert not ext_repository.get_or_create(memory_state_data)[1]


def test_uns_dl_feature_data_is_bound():
    data = uns_dl.FeatureData()
    assert data.index is not None
