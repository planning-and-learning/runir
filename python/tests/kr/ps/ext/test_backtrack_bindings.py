# Copyright (C) 2026 Dominik Drexler
# SPDX-License-Identifier: GPL-3.0-or-later

import pytest
from pytyr.formalism import PlanningDomain
from pyyggdrasil.serialization import Dictionaries

from pyrunir.kr import DomainContext
from pyrunir.kr.ps import ext
from pyrunir.kr.ps.ext.dl import parse_module


@pytest.mark.parametrize(
    "options_type", [ext.GroundProgramSearchOptions, ext.LiftedProgramSearchOptions], ids=["Ground", "Lifted"]
)
def test_and_backtracking_option_defaults_to_false(
    options_type: type[ext.GroundProgramSearchOptions] | type[ext.LiftedProgramSearchOptions],
) -> None:
    options = options_type()
    assert options.and_backtracking is False
    options.and_backtracking = True
    assert options.and_backtracking is True
    assert options.universal is False


@pytest.mark.parametrize("conditions", ["", "(positive bad)", "(positive bad) (positive bad)"])
def test_backtrack_rule_construct_format_and_serialize(
    gripper_planning_domain: PlanningDomain, conditions: str
) -> None:
    repository = DomainContext(gripper_planning_domain).ext_repository
    source = f"""(:module (:symbol prune) (:arguments) (:registers)
      (:entry m) (:memory m)
      (:features (:boolean (:symbol bad) (:expression (b_nonempty (c_top)))))
      (:rules (:rule (:symbol reject) (:expression (:source-memory m)
        (:backtrack (:conditions {conditions}))))))"""
    module = parse_module(source, gripper_planning_domain, repository)
    variant = module.memory_transitions()[0][0]
    rule = variant.variant()
    assert isinstance(rule, ext.RuleBacktrack)
    assert isinstance(rule.index(), ext.RuleBacktrackIndex)
    assert rule.source().name() == "m"
    assert len(rule.conditions()) == bool(conditions)
    assert "(:backtrack" in str(module)
    assert ":target-memory" not in str(module)
    assert str(parse_module(str(module), gripper_planning_domain, repository)) == str(module)

    data = ext.RuleBacktrackData()
    data.source = rule.source().index()
    data.conditions = [condition.index() for condition in rule.conditions()]
    assert repository.insert(data)[0] == rule

    assert list(ext.RuleBacktrack.Fields.__members__) == ["source", "conditions"]
    dictionaries = Dictionaries()
    dictionaries.register_table(ext.RuleExt, "variants", "v")
    dictionaries.register_table(ext.RuleBacktrack, "backtrack", "r")
    dictionaries.register_table(ext.MemoryStateExt, "memory", "m")
    dictionaries.register_table(
        ext.dl.ConcreteConditionExtBooleanPositive,
        "conditions",
        "c",
        project=lambda condition: {"text": str(condition)},
    )
    assert dictionaries.serialize(variant) == "v0"
    assert dictionaries.table(ext.RuleExt) == [{"symbol": "reject", "variant": "r0"}]
    assert dictionaries.table(ext.RuleBacktrack) == [{"source": "m0", "conditions": ["c0"] if conditions else []}]
    assert dictionaries.table(ext.dl.ConcreteConditionExtBooleanPositive) == (
        [{"text": "(positive bad)"}] if conditions else []
    )
