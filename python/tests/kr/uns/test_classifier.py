import re
from typing import NotRequired, TypedDict, cast

import pytest

from fixture_utils import load_fixture, read_fixture
from pyrunir.datasets import GroundTaskSearchContext
from pyrunir.kr import GroundTaskContext, SemanticError
from pyrunir.kr.uns.dl import parse_classifier
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
    ground_gripper_search_context: GroundTaskSearchContext,
    gripper_planning_domain: PlanningDomain,
) -> None:
    task_context = GroundTaskContext(ground_gripper_search_context)
    description = read_fixture(case["file"])

    if case["valid"]:
        symbol = case.get("symbol")
        assert isinstance(symbol, str)
        classifier = parse_classifier(description, gripper_planning_domain, task_context.uns_repository)
        assert classifier.get_symbol() == symbol
        assert str(parse_classifier(str(classifier), gripper_planning_domain, task_context.uns_repository)) == str(classifier)
    else:
        error = case.get("error")
        assert isinstance(error, str)
        with pytest.raises(SemanticError, match=re.escape(error)):
            parse_classifier(description, gripper_planning_domain, task_context.uns_repository)
