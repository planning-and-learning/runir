import pytest

from pyrunir.kr import DomainContext
from pyrunir.kr.ps import ext
from pyrunir.kr.ps.ext.dl import parse_module
from pyrunir.serialization import register_table, serialize, table
from pyyggdrasil.serialization import Dictionaries


@pytest.mark.parametrize("conditions", ["", "(positive bad)", "(positive bad) (positive bad)"])
def test_backtrack_rule_construct_format_and_serialize(gripper_planning_domain, conditions):
    repository = DomainContext(gripper_planning_domain).ext_repository
    source = f"""(:module (:symbol prune) (:arguments) (:registers)
      (:entry m) (:memory m)
      (:features (:boolean (:symbol bad) (:expression (b_nonempty (c_top)))))
      (:rules (:rule (:symbol reject) (:expression (:source-memory m)
        (:backtrack (:conditions {conditions}))))))"""
    module = parse_module(source, gripper_planning_domain, repository)
    variant = module.get_memory_transitions()[0][0]
    rule = variant.get_variant()
    assert isinstance(rule, ext.BacktrackRule)
    assert isinstance(rule.get_index(), ext.BacktrackRuleIndex)
    assert rule.get_source().get_name() == "m"
    assert len(rule.get_conditions()) == bool(conditions)
    assert not hasattr(rule, "get_target")
    assert not hasattr(rule, "get_effects")
    assert "(:backtrack" in str(module)
    assert ":target-memory" not in str(module)
    assert str(parse_module(str(module), gripper_planning_domain, repository)) == str(module)

    data = ext.BacktrackRuleData()
    data.source = rule.get_source().get_index()
    data.conditions = [condition.get_index() for condition in rule.get_conditions()]
    assert not hasattr(data, "target")
    assert not hasattr(data, "effects")
    assert repository.insert(data)[0] == rule

    assert list(ext.BacktrackRule.Fields.__members__) == ["source", "conditions"]
    dictionaries = Dictionaries()
    register_table(dictionaries, ext.RuleVariant, "variants", "v")
    register_table(dictionaries, ext.BacktrackRule, "backtrack", "r")
    register_table(dictionaries, ext.MemoryState, "memory", "m")
    register_table(dictionaries, ext.ConditionVariant, "conditions", "c", project=lambda condition: {"text": str(condition)})
    assert serialize(dictionaries, variant) == "v0"
    assert table(dictionaries, ext.RuleVariant) == [{"symbol": "reject", "variant": "r0"}]
    assert table(dictionaries, ext.BacktrackRule) == [{"source": "m0", "conditions": ["c0"] if conditions else []}]
    assert table(dictionaries, ext.ConditionVariant) == ([{"text": "(positive bad)"}] if conditions else [])
