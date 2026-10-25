# Copyright (C) 2026 Dominik Drexler
# SPDX-License-Identifier: GPL-3.0-or-later

import pytest
from pytyr.formalism.planning import PlanningDomain

from pyrunir.kr.dl.semantics import ConstructorRepositoryFactory
from pyrunir.kr.ps.base import RepositoryFactory
from pyrunir.kr.ps.base.dl import parse_sketch

SKETCH = """
(:sketch
  (:datalog
    (:rule (held ?b) (carry ?b ?g))
    (:rule (placed ?b) (goal at ?b ?r) (at ?b ?r)))
  (:features
    (:numerical (:symbol holding) (:expression (n_count (q_head held (b)))))
    (:numerical (:symbol todo) (:expression (n_sub (n_count (q_atomic_goal "at" true (b r))) done)))
    (:numerical (:symbol done) (:expression (n_count (q_head placed (b))))))
  (:rules))
"""


def test_heads_are_read_by_features(gripper_planning_domain: PlanningDomain) -> None:
    repository = RepositoryFactory().create(ConstructorRepositoryFactory().create(gripper_planning_domain))
    sketch = parse_sketch(SKETCH, gripper_planning_domain, repository)
    expressions = [str(feature) for feature in sketch.get_numerical_features()]
    assert any("(q_head held (b))" in expression for expression in expressions)
    assert any("(q_head placed (b))" in expression for expression in expressions)


def test_unknown_relation_is_rejected(gripper_planning_domain: PlanningDomain) -> None:
    repository = RepositoryFactory().create(ConstructorRepositoryFactory().create(gripper_planning_domain))
    with pytest.raises(Exception, match="Undefined predicate: unknown"):
        parse_sketch(SKETCH.replace("(carry ?b ?g)", "(unknown ?b)"), gripper_planning_domain, repository)
