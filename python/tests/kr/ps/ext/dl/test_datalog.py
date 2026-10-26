# Copyright (C) 2026 Dominik Drexler
# SPDX-License-Identifier: GPL-3.0-or-later

import pytest
from pytyr.formalism.planning import PlanningDomain

from pyrunir.kr.dl import ConstructorRepositoryFactory
from pyrunir.kr.ps.ext import RepositoryFactory
from pyrunir.kr.ps.ext.dl import parse_module

MODULE = """
(:module (:symbol entry) (:arguments) (:registers) (:entry m0) (:memory m0)
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
    module = parse_module(MODULE, gripper_planning_domain, repository)
    expressions = [str(feature) for feature in module.get_numerical_features()]
    assert any("(q_head held (b))" in expression for expression in expressions)
    assert any("(q_head placed (b))" in expression for expression in expressions)


def test_unknown_relation_is_rejected(gripper_planning_domain: PlanningDomain) -> None:
    repository = RepositoryFactory().create(ConstructorRepositoryFactory().create(gripper_planning_domain))
    with pytest.raises(Exception, match="Undefined predicate: unknown"):
        parse_module(MODULE.replace("(carry ?b ?g)", "(unknown ?b)"), gripper_planning_domain, repository)


def test_heads_print_their_own_names(gripper_planning_domain: PlanningDomain) -> None:
    # Programs with the same rules but different head names must stay distinct in one repository.
    repository = RepositoryFactory().create(ConstructorRepositoryFactory().create(gripper_planning_domain))
    for name in ("held", "carried"):
        module = parse_module(MODULE.replace("held", name), gripper_planning_domain, repository)
        expressions = [str(feature) for feature in module.get_numerical_features()]
        assert any(f"(q_head {name} (b))" in expression for expression in expressions)
