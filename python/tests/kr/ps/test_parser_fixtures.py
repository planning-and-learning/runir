# Copyright (C) 2026 Dominik Drexler
# SPDX-License-Identifier: GPL-3.0-or-later

from typing import Literal, TypedDict, cast

import pytest
from fixture_utils import load_fixture, read_fixture
from pytyr.formalism.planning import PlanningDomain

from pyrunir.kr import SemanticError
from pyrunir.kr.dl import ConstructorRepositoryFactory as DlRepositoryFactory
from pyrunir.kr.ps import ext
from pyrunir.kr.ps.base import RepositoryFactory as BaseRepositoryFactory
from pyrunir.kr.ps.base.dl import parse_sketch
from pyrunir.kr.ps.ext import dl


class ParserFixture(TypedDict):
    name: str
    parser: Literal["base_sketch", "ext_module", "ext_program"]
    file: str
    error: str


_SUITE = load_fixture("kr/ps/parser_negative.json")
CASES = cast("list[ParserFixture]", _SUITE["cases"])


def _parse(case: ParserFixture, description: str, domain: PlanningDomain) -> None:
    """Parse ``description`` with the parser that ``case`` names."""
    dl_repository = DlRepositoryFactory().create(domain)
    if case["parser"] == "base_sketch":
        parse_sketch(description, domain, BaseRepositoryFactory().create(dl_repository))
        return
    repository = ext.RepositoryFactory().create(dl_repository)
    if case["parser"] == "ext_module":
        dl.parse_module(description, domain, repository)
    else:
        dl.parse_program(description, domain, repository)


@pytest.mark.parametrize("case", CASES, ids=[case["name"] for case in CASES])
def test_parser_negative_fixture(case: ParserFixture, gripper_planning_domain: PlanningDomain) -> None:
    description = read_fixture(case["file"])

    with pytest.raises(SemanticError, match=case["error"]):
        _parse(case, description, gripper_planning_domain)
