# Copyright (C) 2026 Dominik Drexler
# SPDX-License-Identifier: GPL-3.0-or-later

import pytest
from pytyr.formalism import PlanningDomain

from pyrunir.datasets import GroundTaskSearchContext
from pyrunir.kr import DomainContext, GroundTaskContext
from pyrunir.kr.dl import semantics
from pyrunir.kr.ps import base
from pyrunir.kr.ps.base.dl import FeatureBaseNumerical, parse_sketch

_AT = '(q_atomic_state "at" (ball room))'
_AT_ROBBY = '(q_concept room (c_atomic_state "at-robby"))'
_BALL = '(q_concept ball (c_atomic_state "ball"))'


def _feature(expression: str, domain: PlanningDomain, context: DomainContext) -> FeatureBaseNumerical:
    sketch = parse_sketch(
        f"(:sketch (:features (:numerical (:symbol value) (:expression {expression}))) (:rules))",
        domain,
        context.base_repository,
    )
    return sketch.numerical_features()[0]


@pytest.mark.parametrize(
    "expression",
    [
        f"(n_count (q_join (q_join {_AT} {_AT_ROBBY}) {_BALL}))",
        f"(n_count (q_project (ball) (q_join {_AT} {_AT_ROBBY})))",
        f"(n_count (q_generic_join (ball room) (room ball) {_AT} {_BALL}))",
        f"(n_count (q_union {_AT} (q_join {_AT} {_BALL})))",
        # Distance edges are optimized for evaluation per vertex.
        f"(n_distance {_BALL} (q_join {_AT} {_BALL}) {_AT_ROBBY})",
    ],
)
def test_optimized_features_evaluate_like_originals(
    gripper_planning_domain: PlanningDomain, ground_gripper_search_context: GroundTaskSearchContext, expression: str
) -> None:
    domain = DomainContext(gripper_planning_domain)
    search = ground_gripper_search_context
    task = GroundTaskContext(domain, search)
    state = search.state_repository.get_initial_state(search.axiom_evaluator)
    storage = semantics.EvaluationStorage(task.dl_denotation_repository)
    context = semantics.GroundStateEvaluationContext(state, task.dl_builder, storage)

    feature = _feature(expression, gripper_planning_domain, domain)
    original = feature.variant().feature()
    statistics = base.dl.collect_query_statistics(feature, [context])
    assert isinstance(statistics, semantics.QueryStatistics)
    assert statistics.objects is not None
    assert statistics.num_leaves() > 0

    for optimized_feature in (
        base.dl.optimize(feature, domain.base_repository),
        base.dl.optimize(feature, domain.base_repository, statistics),
    ):
        assert optimized_feature.variant().symbol() == feature.variant().symbol()
        optimized = optimized_feature.variant().feature()
        assert semantics.evaluate(optimized, context).value() == semantics.evaluate(original, context).value()
        # The optimized expression prints as valid syntax that parses back to itself.
        assert _feature(str(optimized), gripper_planning_domain, domain).variant().feature() == optimized

    dl_repository = domain.base_repository.get_dl_repository()
    assert (
        semantics.evaluate(semantics.optimize(original, dl_repository), context).value()
        == semantics.evaluate(original, context).value()
    )
    assert (
        semantics.evaluate(
            semantics.optimize(original, dl_repository, semantics.collect_query_statistics(original, [context])),
            context,
        ).value()
        == semantics.evaluate(original, context).value()
    )


def _count_arg_variant(feature: FeatureBaseNumerical) -> object:
    count = feature.variant().feature().variant()
    assert isinstance(count, semantics.NumericalSemanticCount)
    return count.operand().variant()


def test_generic_join_and_empty_bindings(gripper_planning_domain: PlanningDomain) -> None:
    domain = DomainContext(gripper_planning_domain)
    feature = _feature(
        f"(n_count (q_generic_join (ball room) (room ball) {_AT} {_BALL}))", gripper_planning_domain, domain
    )
    join = _count_arg_variant(feature)
    assert isinstance(join, semantics.QueryGenericJoin)
    assert [str(column) for column in join.variable_order()] == ["ball", "room"]
    assert [str(column) for column in join.output_order()] == ["room", "ball"]
    assert len(join.args()) == 2
    empty = _count_arg_variant(_feature("(n_count (q_empty (ball)))", gripper_planning_domain, domain))
    assert isinstance(empty, semantics.QueryEmpty)
    assert [str(column) for column in empty.columns()] == ["ball"]
