import pytest

from pyrunir.kr import DomainContext
from pyrunir.kr.ps import ext
from pyrunir.kr.ps.ext.dl import parse_module
from pyrunir.serialization import register_table, serialize, table
from pyyggdrasil.serialization import Dictionaries


@pytest.mark.parametrize("kind", ["load", "choose"])
@pytest.mark.parametrize("category,expression", [("concept", "c_top"), ("role", "r_universal")])
@pytest.mark.parametrize("effects", ["", "(:effects)", "(:effects (increases n))"])
def test_binding_rules_parse_construct_and_serialize(gripper_planning_domain, kind, category, expression, effects):
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
    rule_type = getattr(ext, f"{category.title()}{kind.title()}Rule")
    data = getattr(ext, f"{category.title()}{kind.title()}RuleData")()
    assert isinstance(rule, rule_type)
    assert len(rule.get_effects()) == int("increases" in effects)
    assert ("(:effects" in str(module)) == ("increases" in effects)
    assert str(parse_module(str(module), gripper_planning_domain, repository)) == str(module)

    data.source = rule.get_source().get_index()
    data.target = rule.get_target().get_index()
    data.conditions = [condition.get_index() for condition in rule.get_conditions()]
    data.feature = rule.get_feature().get_index()
    data.reg = rule.get_register().get_index()
    data.effects = [effect.get_index() for effect in rule.get_effects()]
    assert repository.get_or_create(data) == rule
    data.effects = []
    assert (repository.get_or_create(data) == rule) == (not rule.get_effects())

    assert list(rule_type.Fields.__members__) == ["source", "target", "conditions", "effects", "feature", "register"] + (["order"] if kind == "choose" else [])
    dictionaries = Dictionaries()
    register_table(dictionaries, ext.RuleVariant, "variants", "v")
    register_table(dictionaries, rule_type, "bindings", "r", fields=("source", "target", "effects"))
    register_table(dictionaries, ext.MemoryState, "memory", "m")
    register_table(dictionaries, ext.EffectVariant, "effects", "e", project=lambda effect: {"text": str(effect)})
    assert serialize(dictionaries, variant) == "v0"
    assert table(dictionaries, ext.RuleVariant) == [{"symbol": "bind", "variant": "r0"}]
    assert table(dictionaries, rule_type) == [{
        "source": "m0", "target": "m1", "effects": [f"e{i}" for i in range(len(rule.get_effects()))],
    }]
    assert table(dictionaries, ext.EffectVariant) == [{"text": str(effect)} for effect in rule.get_effects()]


@pytest.mark.parametrize("category,expression", [("concept", "c_top"), ("role", "r_universal")])
def test_choose_order_data_and_serialization(gripper_planning_domain, category, expression):
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
    terms = rule.get_order()
    assert terms[0].get_index() == terms[2].get_index()
    assert [t.get_direction() for t in terms] == [ext.OrderDirection.MAX, ext.OrderDirection.MIN, ext.OrderDirection.MAX]
    assert [t.get_feature().get_symbol() for t in terms] == ["flag", "n", "flag"]
    assert str(parse_module(str(module), gripper_planning_domain, repository)) == str(module)
    data = getattr(ext, f"{category.title()}ChooseRuleData")()
    data.source, data.target = rule.get_source().get_index(), rule.get_target().get_index()
    data.feature, data.reg = rule.get_feature().get_index(), rule.get_register().get_index()
    values = []
    for term in terms:
        value = ext.OrderTermData()
        value.direction, value.feature = term.get_direction(), term.get_feature().get_index()
        interned = repository.get_or_create(value)
        assert interned == term
        values.append(interned.get_index())
    data.order = values
    assert repository.get_or_create(data) == rule
    data.order = list(reversed(values[:2]))
    assert repository.get_or_create(data) != rule
    dictionaries = Dictionaries()
    register_table(dictionaries, ext.dl.BooleanFeature, "booleans", "b", project=lambda f: {"symbol": f.get_symbol()})
    register_table(dictionaries, ext.dl.NumericalFeature, "numericals", "n", project=lambda f: {"symbol": f.get_symbol()})
    register_table(dictionaries, ext.OrderTerm, "ordering", "o")
    register_table(dictionaries, type(rule), "rules", "r", fields=("order",))
    assert serialize(dictionaries, rule) == "r0"
    assert table(dictionaries, type(rule)) == [{"order": ["o0", "o1", "o0"]}]
    assert table(dictionaries, ext.OrderTerm) == [
        {"direction": "max", "variant": "b0"}, {"direction": "min", "variant": "n0"},
    ]
