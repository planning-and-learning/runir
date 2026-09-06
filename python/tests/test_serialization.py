import json

import pytest

from fixture_utils import read_fixture
from pytyr.formalism import planning as fp
from pytyr.planning import ground

from pyrunir.kr import DomainContext, GroundTaskContext
from pyrunir.kr.dl.base import semantics
from pyrunir.kr.ps import base, ext
from pyrunir.kr.ps.base.dl import parse_sketch
from pyrunir.kr.ps.ext.dl import parse_module_program
from pyrunir.serialization import Dictionaries, register_table, serialize, table


@pytest.mark.parametrize("operand", ["c_top", "r_universal"])
def test_nonempty_uses_native_operand_text(gripper_planning_domain, operand):
    context = DomainContext(gripper_planning_domain)
    policy = parse_sketch(
        "(:sketch (:features (:boolean (:symbol b) (:expression "
        f"(b_nonempty ({operand}))"
        "))) (:rules))",
        gripper_planning_domain,
        context.base_repository,
    )
    feature, = policy.get_boolean_features()
    dictionaries = Dictionaries()
    register_table(dictionaries, semantics.BooleanNonempty, "nonempty", "b")
    expression = feature.get_expression()
    assert serialize(dictionaries, expression) == str(expression)
    assert table(dictionaries, semantics.BooleanNonempty) == []
    assert serialize(dictionaries, expression.get_variant()) == "b0"
    assert table(dictionaries, semantics.BooleanNonempty) == [{"arg": f"({operand})"}]


def test_registered_rule_keeps_conditions_and_effects_as_native_text(gripper_planning_domain):
    context = DomainContext(gripper_planning_domain)
    policy = parse_sketch(
        read_fixture("kr/ps/base/dl/declared_features.sketch"),
        gripper_planning_domain,
        context.base_repository,
    )
    rule, = policy.get_rules()
    feature = policy.get_numerical_features()[0].get_variant()
    dictionaries = Dictionaries()
    register_table(dictionaries, base.Rule, "rules", "r")
    register_table(dictionaries, base.dl.ConcreteNumericalFeature, "features", "f")
    assert serialize(dictionaries, policy) == str(policy)
    assert table(dictionaries, base.Rule) == []
    assert serialize(dictionaries, rule) == "r0"
    assert table(dictionaries, base.Rule) == [{
        "symbol": rule.get_symbol(),
        "conditions": [str(condition) for condition in rule.get_conditions()],
        "effects": [str(effect) for effect in rule.get_effects()],
    }]
    assert table(dictionaries, base.dl.ConcreteNumericalFeature) == []
    assert serialize(dictionaries, feature) == "f0"
    assert table(dictionaries, base.dl.ConcreteNumericalFeature) == [{
        "symbol": feature.get_symbol(), "expression": str(feature.get_expression()),
    }]


def test_runir_and_tyr_share_dictionary_references(ground_gripper_search_context):
    search = ground_gripper_search_context
    domain = search.task.get_formalism_task().get_domain()
    domain_context = DomainContext(domain)
    context = GroundTaskContext(domain_context, search)
    program = parse_module_program(
        read_fixture("kr/ps/ext/dl/acyclic_calls.program"), domain, domain_context.ext_repository,
    )
    expander = ext.GroundSuccessorExpander(context, program)
    initial = expander.initial_state()
    step, = expander.control_steps(initial)
    inline = Dictionaries()
    assert serialize(inline, initial) == str(initial)
    assert inline.tables() == {}

    dictionaries = Dictionaries()
    dictionaries.register_table(ground.State, "states", "s")
    dictionaries.register_table(fp.FunctionExpression, "expressions", "x")
    register_table(dictionaries, ext.GroundExecutionState, "execution_states", "e")
    register_table(dictionaries, ext.GroundCallStack, "call_stacks", "c")
    register_table(dictionaries, ext.ModuleProgram, "programs", "p")

    encoded_domain = dictionaries.serialize(domain)
    assert encoded_domain == str(domain)
    constant = domain.get_repository().create(fp.FunctionExpressionData(3.5))
    assert dictionaries.serialize(constant) == "x0"
    assert dictionaries.table(fp.FunctionExpression) == [{"kind": "constant", "value": 3.5}]
    assert dictionaries.serialize(initial.state) == "s0"
    assert serialize(dictionaries, initial) == "e0"
    assert serialize(dictionaries, step.target) == "e1"
    assert serialize(dictionaries, initial) == "e0"
    first, second = table(dictionaries, ext.GroundExecutionState)
    assert isinstance(first, dict) and isinstance(second, dict)
    assert first["state"] == second["state"] == "s0"
    assert first["call_stack"] != second["call_stack"]
    assert len(dictionaries.table(ground.State)) == 1
    assert dictionaries.tables()["execution_states"]["rows"] == [first, second]

    assert first["phase"] == initial.phase.name
    assert table(dictionaries, ext.ModuleProgram)[0]["modules"] == [str(module) for module in program.get_modules()]
    snapshot = dictionaries.tables()
    assert json.loads(json.dumps(snapshot)) == snapshot
    assert dictionaries.serialize(domain) == encoded_domain
    assert serialize(dictionaries, initial) == "e0"
    assert dictionaries.tables() == snapshot
