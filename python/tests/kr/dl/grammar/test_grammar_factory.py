from pathlib import Path
from typing import TypedDict, cast

import pytest

from fixture_utils import load_fixture, read_fixture
from pypddl.formalism import ParserOptions
from pypddl_datasets import data_root
from pyrunir.kr.dl import base
from pyrunir.kr.dl.base import cnf_grammar
from pyrunir.kr.dl.base.grammar import ConstructorRepositoryFactory, GrammarFactory, GrammarSpecification
from pytyr.formalism.planning import Parser


class GrammarFactoryFixture(TypedDict):
    name: str
    domain_file: str
    grammar_file: str
    cnf_file: str


_SUITE = load_fixture("kr/dl/grammar/factory.json")
CASES = cast(list[GrammarFactoryFixture], _SUITE["cases"])


@pytest.mark.parametrize("case", CASES, ids=[case["name"] for case in CASES])
def test_grammar_factory_fixture(case: GrammarFactoryFixture) -> None:
    planning_domain = Parser(Path(data_root()) / case["domain_file"], ParserOptions()).get_domain()
    domain = planning_domain.get_domain()
    repository = ConstructorRepositoryFactory().create(planning_domain)

    grammar = GrammarFactory.create(GrammarSpecification.FRANCE_ET_AL_AAAI2021, domain, repository)
    formatted = str(grammar)
    assert formatted == read_fixture(case["grammar_file"])
    assert str(base.parse_grammar(formatted, domain, repository)) == formatted

    cnf_repository = cnf_grammar.ConstructorRepositoryFactory().create(planning_domain)
    cnf_formatted = str(cnf_grammar.translate(grammar, cnf_repository))
    assert cnf_formatted == read_fixture(case["cnf_file"])
    reparsed = base.parse_grammar(cnf_formatted, domain, repository)
    assert str(cnf_grammar.translate(reparsed, cnf_repository)) == cnf_formatted
