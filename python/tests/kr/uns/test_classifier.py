import re
from typing import NotRequired, TypedDict, cast

import pytest

from fixture_utils import load_fixture, read_fixture
from pyrunir.kr import DomainContext, SemanticError
from pyrunir.kr.uns.dl import BooleanFeature, ConcreteBooleanFeature, parse_classifier
from pytyr.formalism.planning import PlanningDomain


class ClassifierFixture(TypedDict):
    name: str
    file: str
    valid: bool
    symbol: NotRequired[str]
    error: NotRequired[str]


_SUITE = load_fixture("kr/uns/classifier.json")
CASES = cast(list[ClassifierFixture], _SUITE["cases"])


@pytest.mark.parametrize("case", CASES, ids=[case["name"] for case in CASES])
def test_classifier_fixture(
    case: ClassifierFixture,
    gripper_planning_domain: PlanningDomain,
) -> None:
    domain_context = DomainContext(gripper_planning_domain)
    description = read_fixture(case["file"])

    if case["valid"]:
        symbol = case.get("symbol")
        assert isinstance(symbol, str)
        classifier = parse_classifier(description, gripper_planning_domain, domain_context.uns_repository)
        assert classifier.get_symbol() == symbol
        for feature in classifier.get_features():
            assert isinstance(feature, BooleanFeature)
            concrete = feature.get_variant()
            assert isinstance(concrete, ConcreteBooleanFeature)
            assert feature.get_expression() == concrete.get_expression()
            assert feature.get_symbol() == concrete.get_symbol()
        for clause in classifier.get_clauses():
            for literal in clause.get_literals():
                assert isinstance(literal.get_feature(), BooleanFeature)
        assert str(parse_classifier(str(classifier), gripper_planning_domain, domain_context.uns_repository)) == str(classifier)
    else:
        error = case.get("error")
        assert isinstance(error, str)
        with pytest.raises(SemanticError, match=re.escape(error)):
            parse_classifier(description, gripper_planning_domain, domain_context.uns_repository)
