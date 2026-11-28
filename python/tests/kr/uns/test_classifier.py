# Copyright (C) 2026 Dominik Drexler
# SPDX-License-Identifier: GPL-3.0-or-later

import re
from typing import NotRequired, TypedDict, cast

import pytest
from fixture_utils import load_fixture, read_fixture
from pytyr.formalism import PlanningDomain

from pyrunir.datasets import GroundTaskSearchContext
from pyrunir.kr import DomainContext, GroundTaskContext, SemanticError, uns
from pyrunir.kr.dl import semantics
from pyrunir.kr.uns import classify
from pyrunir.kr.uns.dl import ConcreteFeatureUnsBoolean, FeatureUnsBoolean, parse_classifier


class ClassifierFixture(TypedDict):
    name: str
    file: str
    valid: bool
    symbol: NotRequired[str]
    error: NotRequired[str]


_SUITE = load_fixture("kr/uns/classifier.json")
CASES = cast("list[ClassifierFixture]", _SUITE["cases"])


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
        assert classifier.symbol() == symbol
        for feature in classifier.features():
            assert isinstance(feature, FeatureUnsBoolean)
            concrete = feature.variant()
            assert isinstance(concrete, ConcreteFeatureUnsBoolean)
            assert feature.variant().feature() == concrete.feature()
            assert feature.variant().symbol() == concrete.symbol()
        for clause in classifier.clauses():
            for literal in clause.literals():
                assert isinstance(literal.feature(), FeatureUnsBoolean)
        assert str(parse_classifier(str(classifier), gripper_planning_domain, domain_context.uns_repository)) == str(
            classifier
        )
    else:
        error = case.get("error")
        assert isinstance(error, str)
        with pytest.raises(SemanticError, match=re.escape(error)):
            parse_classifier(description, gripper_planning_domain, domain_context.uns_repository)


def test_classifier_and_features_use_the_context_cache(
    gripper_planning_domain: PlanningDomain,
    ground_gripper_search_context: GroundTaskSearchContext,
) -> None:
    domain = DomainContext(gripper_planning_domain)
    task = GroundTaskContext(domain, ground_gripper_search_context)
    state = ground_gripper_search_context.state_repository.get_initial_state(
        ground_gripper_search_context.axiom_evaluator
    )
    storage = semantics.EvaluationStorage(task.dl_denotation_repository)
    context = semantics.GroundStateEvaluationContext(state, task.dl_builder, storage)
    classifier = parse_classifier(
        read_fixture("kr/uns/always.classifier"), gripper_planning_domain, domain.uns_repository
    )
    feature = classifier.features()[0]

    assert classify(classifier, context) is True
    assert uns.dl.evaluate(feature, context).value() is True
    assert uns.dl.evaluate(feature.variant(), context).value() is True
    storage.reset_all()
    assert classify(classifier, context) is True
