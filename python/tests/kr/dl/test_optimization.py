import pytest

from pyrunir.kr import DomainContext, GroundTaskContext
from pyrunir.kr.dl import semantics
from pyrunir.kr.ps.base.dl import parse_sketch

_AT = '(q_atomic_state "at" (ball room))'
_AT_ROBBY = '(q_concept room (c_atomic_state "at-robby"))'
_BALL = '(q_concept ball (c_atomic_state "ball"))'


def _feature(expression, domain, context):
    sketch = parse_sketch(
        f"(:sketch (:features (:numerical (:symbol value) (:expression {expression}))) (:rules))",
        domain,
        context.base_repository,
    )
    return sketch.get_numerical_features()[0]


@pytest.mark.parametrize("expression", [
    f"(n_count (q_join (q_join {_AT} {_AT_ROBBY}) {_BALL}))",
    f"(n_count (q_project (ball) (q_join {_AT} {_AT_ROBBY})))",
    f"(n_count (q_generic_join (ball room) (room ball) {_AT} {_BALL}))",
    f"(n_count (q_union {_AT} (q_join {_AT} {_BALL})))",
    # Distance edges are optimized for evaluation per vertex.
    f"(n_distance {_BALL} (q_join {_AT} {_BALL}) {_AT_ROBBY})",
])
def test_optimized_features_evaluate_like_originals(gripper_planning_domain, ground_gripper_search_context, expression):
    domain = DomainContext(gripper_planning_domain)
    search = ground_gripper_search_context
    task = GroundTaskContext(domain, search)
    state = search.state_repository.get_initial_state(search.axiom_evaluator)
    storage = semantics.EvaluationStorage(task.dl_denotation_repository)
    context = semantics.GroundStateEvaluationContext(state, task.dl_builder, storage)

    feature = _feature(expression, gripper_planning_domain, domain)
    original = feature.get_expression()
    statistics = feature.collect_query_statistics([context])
    assert isinstance(statistics, semantics.QueryStatistics)
    assert statistics.objects is not None and statistics.num_leaves() > 0

    for optimized_feature in (feature.optimize(domain.base_repository), feature.optimize(domain.base_repository, statistics)):
        assert optimized_feature.get_symbol() == feature.get_symbol()
        optimized = optimized_feature.get_expression()
        assert optimized.evaluate(context).get() == original.evaluate(context).get()
        # The optimized expression prints as valid syntax that parses back to itself.
        assert _feature(str(optimized), gripper_planning_domain, domain).get_expression() == optimized

    dl_repository = domain.base_repository.get_dl_repository()
    assert original.optimize(dl_repository).evaluate(context).get() == original.evaluate(context).get()
    assert original.optimize(dl_repository, original.collect_query_statistics([context])).evaluate(context).get() == original.evaluate(context).get()


def test_generic_join_and_empty_bindings(gripper_planning_domain, ground_gripper_search_context):
    domain = DomainContext(gripper_planning_domain)
    feature = _feature(f"(n_count (q_generic_join (ball room) (room ball) {_AT} {_BALL}))", gripper_planning_domain, domain)
    join = feature.get_expression().get_variant().get_arg().get_variant()
    assert isinstance(join, semantics.QueryGenericJoin)
    assert [str(column) for column in join.get_variable_order()] == ["ball", "room"]
    assert [str(column) for column in join.get_output_order()] == ["room", "ball"]
    assert len(join.get_args()) == 2
    empty = _feature("(n_count (q_empty (ball)))", gripper_planning_domain, domain).get_expression().get_variant().get_arg().get_variant()
    assert isinstance(empty, semantics.QueryEmpty)
    assert [str(column) for column in empty.get_columns()] == ["ball"]
