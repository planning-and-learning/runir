import re
from typing import NotRequired, TypedDict, cast

import pytest

from fixture_utils import load_fixture, read_fixture
from pyrunir.kr import DomainContext, GroundTaskContext, SemanticError
from pyrunir.kr.dl.uns import semantics
from pyrunir.kr.uns import classify
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


def test_classifier_and_features_use_the_context_cache(gripper_planning_domain, ground_gripper_search_context):
    domain = DomainContext(gripper_planning_domain)
    task = GroundTaskContext(domain, ground_gripper_search_context)
    state = ground_gripper_search_context.state_repository.get_initial_state(ground_gripper_search_context.axiom_evaluator)
    caches = semantics.DenotationCaches()
    context = semantics.GroundStateEvaluationContext(state, task.dl_builder, task.dl_denotation_repository, caches)
    classifier = parse_classifier(read_fixture("kr/uns/always.classifier"), gripper_planning_domain, domain.uns_repository)
    feature = classifier.get_features()[0]

    assert classify(classifier, context) is True
    assert feature.evaluate(context).get() is True
    assert feature.get_variant().evaluate(context).get() is True
    caches.clear()
    assert classify(classifier, context) is True
