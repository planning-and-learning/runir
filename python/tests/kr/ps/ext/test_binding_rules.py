# Copyright (C) 2026 Dominik Drexler
# SPDX-License-Identifier: GPL-3.0-or-later

from collections.abc import Sequence
from typing import TypeAlias

import pytest
from pytyr.formalism import PlanningDomain
from pyyggdrasil.serialization import Dictionaries

from pyrunir.kr import DomainContext
from pyrunir.kr.ps import ext
from pyrunir.kr.ps.ext.dl import parse_module

BindingRule: TypeAlias = ext.RuleLoadConcept | ext.RuleLoadRole | ext.RuleChooseConcept | ext.RuleChooseRole
BindingRuleData: TypeAlias = (
    ext.RuleLoadConceptData | ext.RuleLoadRoleData | ext.RuleChooseConceptData | ext.RuleChooseRoleData
)
EffectIndex: TypeAlias = (
    ext.dl.ConcreteEffectExtBooleanPositiveIndex
    | ext.dl.ConcreteEffectExtBooleanNegativeIndex
    | ext.dl.ConcreteEffectExtBooleanUnchangedIndex
    | ext.dl.ConcreteEffectExtNumericalIncreasesIndex
    | ext.dl.ConcreteEffectExtNumericalDecreasesIndex
    | ext.dl.ConcreteEffectExtNumericalUnchangedIndex
)
RULE_TYPES = {
    ("concept", "load"): ext.RuleLoadConcept,
    ("role", "load"): ext.RuleLoadRole,
    ("concept", "choose"): ext.RuleChooseConcept,
    ("role", "choose"): ext.RuleChooseRole,
}


def _set_common_fields(data: BindingRuleData, rule: BindingRule, effects: Sequence[EffectIndex] | None) -> None:
    """Copy the fields every binding rule shares; ``effects=None`` leaves the data's effects untouched."""
    data.source = rule.get_source().index()
    data.target = rule.get_target().index()
    data.conditions = [condition.index() for condition in rule.get_conditions()]
    if effects is not None:
        data.effects = effects


def _interns_to(
    repository: ext.Repository,
    rule: BindingRule,
    effects: Sequence[EffectIndex] | None,
    order: Sequence[ext.OrderTermIndex] | None = None,
) -> bool:
    """Rebuild ``rule``'s data with ``effects`` and ``order`` and report whether it interns to ``rule``."""
    if isinstance(rule, ext.RuleLoadConcept):
        concept_load = ext.RuleLoadConceptData()
        _set_common_fields(concept_load, rule, effects)
        concept_load.feature = rule.get_feature().index()
        concept_load.slot = rule.get_slot().index()
        return repository.insert(concept_load)[0] == rule
    if isinstance(rule, ext.RuleLoadRole):
        role_load = ext.RuleLoadRoleData()
        _set_common_fields(role_load, rule, effects)
        role_load.feature = rule.get_feature().index()
        role_load.slot = rule.get_slot().index()
        return repository.insert(role_load)[0] == rule
    if isinstance(rule, ext.RuleChooseConcept):
        concept_choose = ext.RuleChooseConceptData()
        _set_common_fields(concept_choose, rule, effects)
        concept_choose.feature = rule.get_feature().index()
        concept_choose.slot = rule.get_slot().index()
        if order is not None:
            concept_choose.order = order
        return repository.insert(concept_choose)[0] == rule
    role_choose = ext.RuleChooseRoleData()
    _set_common_fields(role_choose, rule, effects)
    role_choose.feature = rule.get_feature().index()
    role_choose.slot = rule.get_slot().index()
    if order is not None:
        role_choose.order = order
    return repository.insert(role_choose)[0] == rule


@pytest.mark.parametrize("kind", ["load", "choose"])
@pytest.mark.parametrize(("category", "expression"), [("concept", "c_top"), ("role", "r_universal")])
@pytest.mark.parametrize("effects", ["", "(:effects)", "(:effects (increases n))"])
def test_binding_rules_parse_construct_and_serialize(
    gripper_planning_domain: PlanningDomain, kind: str, category: str, expression: str, effects: str
) -> None:
    repository = DomainContext(gripper_planning_domain).ext_repository
    source = f"""(:module (:symbol bindings) (:arguments)
      (:registers (:{category} selected)) (:entry m0) (:memory m0 m1)
      (:features
        (:{category} (:symbol candidates) (:expression ({expression})))
        (:numerical (:symbol n) (:expression (n_count ({category[0]}_register selected)))))
      (:rules (:rule (:symbol bind) (:expression (:source-memory m0) (:target-memory m1)
        (:{kind} (:conditions) (:{category} candidates)
          (:register (:{category} selected)) {effects})))))"""
    module = parse_module(source, gripper_planning_domain, repository)
    variant = module.get_memory_transitions()[0][0]
    rule = variant.get_variant()
    rule_type = RULE_TYPES[category, kind]
    assert isinstance(rule, rule_type)
    assert len(rule.get_effects()) == int("increases" in effects)
    assert ("(:effects" in str(module)) == ("increases" in effects)
    assert str(parse_module(str(module), gripper_planning_domain, repository)) == str(module)

    assert _interns_to(repository, rule, [effect.index() for effect in rule.get_effects()])
    assert _interns_to(repository, rule, []) == (not rule.get_effects())

    assert list(rule_type.Fields.__members__) == [
        "source",
        "target",
        "conditions",
        "feature",
        "slot",
        "effects",
    ] + (["order"] if kind == "choose" else [])
    dictionaries = Dictionaries()
    dictionaries.register_table(ext.RuleExt, "variants", "v")
    dictionaries.register_table(rule_type, "bindings", "r", fields=("source", "target", "effects"))
    dictionaries.register_table(ext.MemoryStateExt, "memory", "m")
    dictionaries.register_table(
        ext.dl.ConcreteEffectExtNumericalIncreases,
        "effects",
        "e",
        project=lambda effect: {"text": str(effect)},
    )
    assert dictionaries.serialize(variant) == "v0"
    assert dictionaries.table(ext.RuleExt) == [{"symbol": "bind", "variant": "r0"}]
    assert dictionaries.table(rule_type) == [
        {
            "source": "m0",
            "target": "m1",
            "effects": [f"e{i}" for i in range(len(rule.get_effects()))],
        }
    ]
    effects_table = [{"text": str(effect)} for effect in rule.get_effects()]
    assert dictionaries.table(ext.dl.ConcreteEffectExtNumericalIncreases) == effects_table


@pytest.mark.parametrize(("category", "expression"), [("concept", "c_top"), ("role", "r_universal")])
def test_choose_order_data_and_serialization(
    gripper_planning_domain: PlanningDomain, category: str, expression: str
) -> None:
    repository = DomainContext(gripper_planning_domain).ext_repository
    source = f"""(:module (:symbol ranked) (:arguments) (:registers (:{category} selected))
      (:entry a) (:memory a b)
      (:features (:{category} (:symbol candidates) (:expression ({expression})))
        (:boolean (:symbol flag) (:expression (b_nonempty (c_top))))
        (:numerical (:symbol n) (:expression (n_const 2))))
      (:rules (:rule (:symbol pick) (:expression (:source-memory a) (:target-memory b)
        (:choose (:conditions) (:{category} candidates) (:register (:{category} selected))
          (:order (max flag) (min n) (max flag)))))))"""
    module = parse_module(source, gripper_planning_domain, repository)
    rule = module.get_memory_transitions()[0][0].get_variant()
    assert isinstance(rule, (ext.RuleChooseConcept, ext.RuleChooseRole))
    terms = rule.get_order()
    assert terms[0].index() == terms[2].index()
    assert [t.get_direction() for t in terms] == [
        ext.OrderDirection.MAX,
        ext.OrderDirection.MIN,
        ext.OrderDirection.MAX,
    ]
    assert [t.get_feature().get_variant().get_symbol() for t in terms] == ["flag", "n", "flag"]
    assert str(parse_module(str(module), gripper_planning_domain, repository)) == str(module)
    values: list[ext.OrderTermIndex] = []
    for term in terms:
        value = ext.OrderTermData()
        value.direction, value.feature = term.get_direction(), term.get_feature().index()
        interned = repository.insert(value)[0]
        assert interned == term
        values.append(interned.index())
    assert _interns_to(repository, rule, None, values)
    assert not _interns_to(repository, rule, None, list(reversed(values[:2])))
    dictionaries = Dictionaries()
    dictionaries.register_table(
        ext.dl.FeatureExtBoolean,
        "booleans",
        "b",
        project=lambda f: {"symbol": f.get_variant().get_symbol()},
    )
    dictionaries.register_table(
        ext.dl.FeatureExtNumerical,
        "numericals",
        "n",
        project=lambda f: {"symbol": f.get_variant().get_symbol()},
    )
    dictionaries.register_table(ext.OrderTerm, "ordering", "o")
    dictionaries.register_table(type(rule), "rules", "r", fields=("order",))
    assert dictionaries.serialize(rule) == "r0"
    assert dictionaries.table(type(rule)) == [{"order": ["o0", "o1", "o0"]}]
    assert dictionaries.table(ext.OrderTerm) == [
        {"direction": "max", "feature": "b0"},
        {"direction": "min", "feature": "n0"},
    ]
