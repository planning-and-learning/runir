import json
import subprocess
import sys

import pytest

from fixture_utils import read_fixture
from pytyr.formalism import planning as fp
from pytyr.planning import ground
import pytyr.serialization as tyr_serialization
from pyyggdrasil.serialization import Dictionaries

from pyrunir.kr import DomainContext, GroundTaskContext
from pyrunir.kr.dl.base import semantics
from pyrunir.kr.ps import base, ext
from pyrunir.kr.ps.base.dl import parse_sketch
from pyrunir.kr.ps.ext.dl import parse_module_program
from pyrunir.serialization import register_table, serialize, table


def test_fields_describe_native_layouts_without_instances():
    assert list(base.Rule.Fields.__members__) == ["symbol", "conditions", "effects"]
    assert list(ext.RuleVariant.Fields.__members__) == ["symbol", "variant"]
    assert list(semantics.BooleanNonempty.Fields.__members__) == ["arg"]
    assert list(fp.FluentPredicateBinding.Fields.__members__) == ["relation", "objects"]


def test_runir_preserves_tyr_field_enum_identity():
    subprocess.run(
        [sys.executable, "-c", """
from pytyr.formalism import planning as fp
import pytyr.serialization
fields = fp.ActionBinding.Fields
import pyrunir.serialization
assert fp.ActionBinding.Fields is fields
"""],
        check=True,
    )


@pytest.mark.parametrize("native_type, selected", [
    (base.Rule, base.Rule.Fields.symbol),
    (fp.ActionBinding, fp.ActionBinding.Fields.objects),
    (fp.FluentPredicateBinding, fp.FluentPredicateBinding.Fields.relation),
])
def test_matching_field_enums_are_accepted_for_both_libraries(native_type, selected):
    dictionaries = Dictionaries()
    register_table(dictionaries, native_type, "selected", "s", fields=[selected])
    assert table(dictionaries, native_type) == []


def test_field_enums_from_another_native_type_are_rejected():
    with pytest.raises(TypeError):
        register_table(
            Dictionaries(), base.Rule, "rules", "r", fields=[fp.ActionBinding.Fields.objects],
        )


@pytest.mark.parametrize("operand", ["c_top", "r_universal"])
def test_nonempty_requires_registered_operands_or_an_explicit_text_projection(gripper_planning_domain, operand):
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
    with pytest.raises(ValueError, match=r"^Unregistered serialization type: .*Constructor.*BooleanTag"):
        serialize(dictionaries, expression)

    dictionaries = Dictionaries()
    register_table(dictionaries, semantics.Boolean, "booleans", "x")
    register_table(dictionaries, semantics.BooleanNonempty, "nonempty", "b")
    with pytest.raises(ValueError, match=r"^Unregistered serialization type: .*ConceptTag.*RoleTag"):
        serialize(dictionaries, expression)

    dictionaries = Dictionaries()
    register_table(dictionaries, semantics.Boolean, "booleans", "x")
    register_table(
        dictionaries, semantics.BooleanNonempty, "nonempty", "b",
        project=lambda nonempty: {"arg": str(nonempty.get_arg())},
    )
    assert serialize(dictionaries, expression) == "x0"
    assert table(dictionaries, semantics.Boolean) == [{"variant": "b0"}]
    assert table(dictionaries, semantics.BooleanNonempty) == [{"arg": f"({operand})"}]


def test_rule_text_requires_an_explicit_projection(gripper_planning_domain):
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
    with pytest.raises(ValueError, match=r"^Unregistered serialization type: .*Sketch"):
        serialize(dictionaries, policy)

    dictionaries = Dictionaries()
    register_table(dictionaries, base.Rule, "rules", "r")
    with pytest.raises(ValueError, match=r"^Unregistered serialization type: .*ConditionVariant"):
        serialize(dictionaries, rule)

    dictionaries = Dictionaries()
    register_table(
        dictionaries, base.Rule, "rules", "r",
        project=lambda value: {
            "symbol": value.get_symbol(),
            "conditions": [str(condition) for condition in value.get_conditions()],
            "effects": [str(effect) for effect in value.get_effects()],
        },
    )
    register_table(
        dictionaries, base.dl.ConcreteNumericalFeature, "features", "f",
        project=lambda value: {"symbol": value.get_symbol(), "expression": str(value.get_expression())},
    )
    assert serialize(dictionaries, rule) == "r0"
    assert list(table(dictionaries, base.Rule)[0]) == list(base.Rule.Fields.__members__)
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

    selected = Dictionaries()
    register_table(
        selected, base.Rule, "rules", "r", fields=[base.Rule.Fields.symbol],
    )
    assert serialize(selected, rule) == "r0"
    row, = table(selected, base.Rule)
    assert row == {"symbol": rule.get_symbol()}


@pytest.mark.parametrize("register_planning_table", [register_table, tyr_serialization.register_table], ids=["runir", "tyr"])
def test_runir_and_tyr_share_dictionary_references(ground_gripper_search_context, register_planning_table):
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
    with pytest.raises(ValueError, match=r"^Unregistered serialization type: .*ExecutionState.*GroundTag"):
        serialize(Dictionaries(), initial)
    with pytest.raises(ValueError, match=r"^Unregistered serialization type: .*PlanningDomain"):
        serialize(Dictionaries(), domain)

    dictionaries = Dictionaries()
    register_planning_table(dictionaries, ground.State, "states", "s")
    register_planning_table(dictionaries, fp.FluentGroundAtom, "atoms", "a", fields=())
    register_planning_table(dictionaries, fp.FunctionExpression, "expressions", "x")
    register_table(dictionaries, ext.GroundExecutionState, "execution_states", "e")
    register_table(dictionaries, ext.GroundCallStack, "call_stacks", "c", fields=("caller",))
    register_table(
        dictionaries, ext.ModuleProgram, "programs", "p",
        project=lambda value: {
            "entry_module": str(value.get_entry_module()),
            "modules": [str(module) for module in value.get_modules()],
        },
    )

    constant = domain.get_repository().create(fp.FunctionExpressionData(3.5))
    assert serialize(dictionaries, constant) == "x0"
    assert table(dictionaries, fp.FunctionExpression) == [{"variant": 3.5}]
    assert serialize(dictionaries, initial) == "e0"
    assert serialize(dictionaries, initial.state) == "s0"
    assert tyr_serialization.serialize(dictionaries, initial.state) == "s0"
    assert tyr_serialization.table(dictionaries, ground.State) == table(dictionaries, ground.State)
    assert serialize(dictionaries, step.target) == "e1"
    assert serialize(dictionaries, initial) == "e0"
    first, second = table(dictionaries, ext.GroundExecutionState)
    assert isinstance(first, dict) and isinstance(second, dict)
    assert first["state"] == second["state"] == "s0"
    assert first["call_stack"] != second["call_stack"]
    state, = table(dictionaries, ground.State)
    assert set(state) == {"fluent_ground_atoms", "derived_ground_atoms", "fluent_ground_function_term_values"}
    assert state["fluent_ground_atoms"] == [f"a{index}" for index in range(len(table(dictionaries, fp.FluentGroundAtom)))]
    assert dictionaries.tables()["execution_states"]["rows"] == [first, second]

    assert first["phase"] == initial.phase.name
    assert table(dictionaries, ext.ModuleProgram)[0]["modules"] == [str(module) for module in program.get_modules()]
    snapshot = dictionaries.tables()
    assert json.loads(json.dumps(snapshot)) == snapshot
    assert serialize(dictionaries, initial) == "e0"
    assert dictionaries.tables() == snapshot

    selected = Dictionaries()
    register_table(selected, ground.State, "states", "s")
    register_table(
        selected, ext.GroundExecutionState, "execution_states", "e",
        fields=("call_stack", "phase"),
    )
    register_table(selected, ext.GroundCallStack, "call_stacks", "c", fields=())
    register_table(selected, ext.ModuleProgram, "programs", "p")
    assert serialize(selected, initial) == "e0"
    execution_state, = table(selected, ext.GroundExecutionState)
    assert execution_state == {"phase": initial.phase.name, "call_stack": "c0"}
    assert list(execution_state) == ["phase", "call_stack"]
    assert table(selected, ground.State) == []
    assert table(selected, ext.GroundCallStack) == [{}]
    assert table(selected, ext.ModuleProgram) == []

    def project(value: ext.GroundExecutionState) -> dict[str, object]:
        return {"planning_state": value.state, "phase_name": value.phase}

    projected = Dictionaries()
    register_table(projected, ground.State, "states", "s", fields=())
    register_table(projected, ext.GroundExecutionState, "execution_states", "e", project=project)
    assert serialize(projected, initial) == "e0"
    assert table(projected, ext.GroundExecutionState) == [{
        "planning_state": "s0", "phase_name": initial.phase.name,
    }]
    assert serialize(projected, initial.state) == "s0"
    assert table(projected, ground.State) == [{}]

    reverse = Dictionaries()
    register_table(
        reverse, ground.State, "states", "s", project=lambda _state: {"execution_state": initial},
    )
    register_table(reverse, ext.GroundExecutionState, "execution_states", "e", fields=("state",))
    assert tyr_serialization.serialize(reverse, initial.state) == "s0"
    assert serialize(reverse, initial.state) == "s0"
    assert table(reverse, ground.State) == [{"execution_state": "e0"}]
    assert table(reverse, ext.GroundExecutionState) == [{"state": "s0"}]
    assert serialize(reverse, initial) == "e0"
