"""Regenerate grammar and CNF golden files from the factory manifest."""

import json
from pathlib import Path
from typing import TypedDict, cast

from pypddl.formalism import ParserOptions
from pypddl_datasets import data_root
from pyrunir.kr.dl import base
from pyrunir.kr.dl.base import cnf_grammar
from pyrunir.kr.dl.base.grammar import ConstructorRepositoryFactory, GrammarFactory, GrammarSpecification
from pytyr.formalism.planning import Parser


ROOT = Path(__file__).resolve().parents[5]
SUITE = Path(__file__).with_name("factory.json")


class Case(TypedDict):
    name: str
    domain_file: str
    grammar_file: str
    cnf_file: str


class Suite(TypedDict):
    cases: list[Case]


def main() -> None:
    suite = cast(Suite, json.loads(SUITE.read_text(encoding="utf-8")))
    for case in suite["cases"]:
        planning_domain = Parser(Path(data_root()) / case["domain_file"], ParserOptions()).get_domain()
        domain = planning_domain.get_domain()
        repository = ConstructorRepositoryFactory().create(planning_domain)
        grammar = GrammarFactory.create(GrammarSpecification.FRANCE_ET_AL_AAAI2021, domain, repository)
        grammar_text = str(grammar)
        (ROOT / "tests/fixtures" / case["grammar_file"]).write_text(grammar_text, encoding="utf-8")

        cnf_repository = cnf_grammar.ConstructorRepositoryFactory().create(planning_domain)
        cnf_text = str(cnf_grammar.translate(base.parse_grammar(grammar_text, domain, repository), cnf_repository))
        (ROOT / "tests/fixtures" / case["cnf_file"]).write_text(cnf_text, encoding="utf-8")


if __name__ == "__main__":
    main()
