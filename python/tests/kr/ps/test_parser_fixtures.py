from typing import Literal, TypedDict, cast

import pytest

from fixture_utils import load_fixture, read_fixture
from pyrunir.kr import SemanticError
from pyrunir.kr.dl import ext as dl_ext
from pyrunir.kr.dl.base.semantics import ConstructorRepositoryFactory as BaseDlRepositoryFactory
from pyrunir.kr.ps import ext
from pyrunir.kr.ps.base import RepositoryFactory as BaseRepositoryFactory
from pyrunir.kr.ps.base.dl import parse_sketch
from pyrunir.kr.ps.ext import dl
from pytyr.formalism.planning import PlanningDomain


class ParserFixture(TypedDict):
    name: str
    parser: Literal["base_sketch", "ext_module", "ext_program"]
    file: str
    error: str


_SUITE = load_fixture("kr/ps/parser_negative.json")
CASES = cast(list[ParserFixture], _SUITE["cases"])


@pytest.mark.parametrize("case", CASES, ids=[case["name"] for case in CASES])
def test_parser_negative_fixture(case: ParserFixture, gripper_planning_domain: PlanningDomain) -> None:
    description = read_fixture(case["file"])

    with pytest.raises(SemanticError, match=case["error"]):
        if case["parser"] == "base_sketch":
            base_dl_repository = BaseDlRepositoryFactory().create(gripper_planning_domain)
            parse_sketch(description, gripper_planning_domain, BaseRepositoryFactory().create(base_dl_repository))
        else:
            ext_dl_repository = dl_ext.ConstructorRepositoryFactory().create(gripper_planning_domain)
            repository = ext.RepositoryFactory().create(ext_dl_repository)
            if case["parser"] == "ext_module":
                dl.parse_module(description, gripper_planning_domain, repository)
            else:
                dl.parse_module_program(description, gripper_planning_domain, repository)
