import json
import subprocess
import sys

import pytest

from ext_execution_utils import collect_steps, initial_node
from fixture_utils import read_fixture
from pytyr.formalism import planning as fp
from pytyr.planning import ground
import pytyr.serialization as tyr_serialization
from pyyggdrasil.serialization import Dictionaries

from pyrunir.datasets import StateGraphEdgeLabel
from pyrunir.kr import DomainContext, GroundTaskContext
from pyrunir.kr.dl.base import semantics
from pyrunir.kr.ps import base, ext
from pyrunir.kr.ps.base.dl import parse_sketch
from pyrunir.kr.ps.ext.dl import parse_program
import pyrunir.serialization as runir_serialization
from pyrunir.serialization import register_table, serialize, table


def test_fields_describe_native_layouts_without_instances():
    assert list(base.Rule.Fields.__members__) == ["symbol", "conditions", "effects"]
    assert list(ext.RuleVariant.Fields.__members__) == ["symbol", "variant"]
    assert list(base.SketchProofEdgeLabel.Fields.__members__) == ["action", "rule"]
    assert list(ext.ProgramProofEdgeLabel.Fields.__members__) == ["action", "rule"]
    assert list(StateGraphEdgeLabel.Fields.__members__) == ["action", "cost"]
    assert list(semantics.BooleanNonempty.Fields.__members__) == ["arg"]
    assert list(fp.FluentPredicateBinding.Fields.__members__) == ["relation", "objects"]
    for prefix in ("Ground", "Lifted"):
        assert list(getattr(ext, f"{prefix}ModuleState").Fields.__members__) == [
            "state", "module", "memory_state", "registers", "arguments",
        ]
        assert list(getattr(ext, f"{prefix}CallStack").Fields.__members__) == [
            "module", "return_memory_state", "registers", "arguments", "caller",
        ]
        assert list(getattr(ext, f"{prefix}ProgramState").Fields.__members__) == ["program", "module_state", "call_stack"]


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


@pytest.mark.parametrize("serialization, native_type, selected", [
    (runir_serialization, base.Rule, base.Rule.Fields.symbol),
    (tyr_serialization, fp.ActionBinding, fp.ActionBinding.Fields.objects),
    (tyr_serialization, fp.FluentPredicateBinding, fp.FluentPredicateBinding.Fields.relation),
])
def test_matching_field_enums_are_accepted_by_their_library(serialization, native_type, selected):
    dictionaries = Dictionaries()
    serialization.register_table(dictionaries, native_type, "selected", "s", fields=[selected])
    assert serialization.table(dictionaries, native_type) == []


def test_runir_does_not_register_tyr_types():
    with pytest.raises(TypeError):
        register_table(Dictionaries(), ground.State, "states", "s")


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


def test_runir_and_tyr_share_dictionary_references(ground_gripper_search_context):
    search = ground_gripper_search_context
    domain = search.task.get_formalism_task().get_domain()
    domain_context = DomainContext(domain)
    context = GroundTaskContext(domain_context, search)
    program = parse_program(
        read_fixture("kr/ps/ext/dl/acyclic_calls.program"), domain, domain_context.ext_repository,
    )
    expander = ext.GroundSuccessorExpander(context, program)
    node = initial_node(context)
    initial = expander.initial_state(node.get_state())
    step, = collect_steps(expander, initial)
    with pytest.raises(ValueError, match=r"^Unregistered serialization type: .*ProgramState.*GroundTag"):
        serialize(Dictionaries(), initial)
    with pytest.raises(ValueError, match=r"^Unregistered serialization type: .*PlanningDomain"):
        tyr_serialization.serialize(Dictionaries(), domain)

    dictionaries = Dictionaries()
    tyr_serialization.register_table(dictionaries, ground.State, "states", "s")
    tyr_serialization.register_table(dictionaries, fp.FluentGroundAtom, "atoms", "a", fields=())
    tyr_serialization.register_table(dictionaries, fp.FunctionExpression, "expressions", "x")
    register_table(dictionaries, ext.GroundProgramState, "program_states", "e")
    register_table(dictionaries, ext.GroundModuleState, "module_states", "z")
    register_table(dictionaries, ext.GroundCallStack, "call_stacks", "c")
    register_table(dictionaries, ext.Module, "modules", "m", fields=())
    register_table(dictionaries, ext.MemoryState, "memory_states", "q", fields=())
    register_table(dictionaries, semantics.RegisterValues, "registers", "r")
    register_table(dictionaries, semantics.CallArguments, "arguments", "b")
    register_table(
        dictionaries, ext.Program, "programs", "p",
        project=lambda value: {
            "entry_module": str(value.get_entry_module()),
            "modules": [str(module) for module in value.get_modules()],
        },
    )

    constant = domain.get_repository().create(fp.FunctionExpressionData(3.5))
    assert tyr_serialization.serialize(dictionaries, constant) == "x0"
    assert tyr_serialization.table(dictionaries, fp.FunctionExpression) == [{"variant": 3.5}]
    assert serialize(dictionaries, initial) == "e0"
    assert tyr_serialization.serialize(dictionaries, initial.state) == "s0"
    assert serialize(dictionaries, step.target) == "e1"
    assert serialize(dictionaries, initial) == "e0"
    first, second = table(dictionaries, ext.GroundProgramState)
    assert isinstance(first, dict) and isinstance(second, dict)
    assert first == {"program": "p0", "module_state": "z0", "call_stack": None}
    assert second == {"program": "p0", "module_state": "z1", "call_stack": "c0"}
    assert initial.call_stack is None
    assert serialize(dictionaries, initial.module_state) == "z0"
    module_states = table(dictionaries, ext.GroundModuleState)
    assert module_states[0]["state"] == module_states[1]["state"] == "s0"
    assert module_states[0] == {
        "state": "s0", "module": "m0", "memory_state": "q0", "registers": "r0", "arguments": "b0",
    }
    assert table(dictionaries, ext.GroundCallStack) == [{
        "module": "m0",
        "return_memory_state": serialize(dictionaries, step.target.call_stack.return_memory_state),
        "registers": "r0", "arguments": "b0", "caller": None,
    }]
    assert table(dictionaries, semantics.RegisterValues) == [{"concept_values": [], "role_values": []}]
    assert table(dictionaries, semantics.CallArguments) == [{
        "concept_arguments": [], "role_arguments": [], "boolean_arguments": [], "numerical_arguments": [],
    }]
    state, = tyr_serialization.table(dictionaries, ground.State)
    assert set(state) == {"fluent_ground_atoms", "derived_ground_atoms", "fluent_ground_function_term_values"}
    assert state["fluent_ground_atoms"] == [f"a{index}" for index in range(len(tyr_serialization.table(dictionaries, fp.FluentGroundAtom)))]
    assert dictionaries.tables()["program_states"]["rows"] == [first, second]

    assert table(dictionaries, ext.Program)[0]["modules"] == [str(module) for module in program.get_modules()]
    snapshot = dictionaries.tables()
    assert json.loads(json.dumps(snapshot)) == snapshot
    assert serialize(dictionaries, initial) == "e0"
    assert dictionaries.tables() == snapshot

    selected = Dictionaries()
    tyr_serialization.register_table(selected, ground.State, "states", "s")
    register_table(
        selected, ext.GroundProgramState, "program_states", "e",
        fields=("call_stack", "module_state"),
    )
    register_table(selected, ext.GroundModuleState, "module_states", "z", fields=())
    register_table(selected, ext.GroundCallStack, "call_stacks", "c", fields=())
    register_table(selected, ext.Program, "programs", "p")
    assert serialize(selected, initial) == "e0"
    program_state, = table(selected, ext.GroundProgramState)
    assert program_state == {"module_state": "z0", "call_stack": None}
    assert list(program_state) == ["module_state", "call_stack"]
    assert tyr_serialization.table(selected, ground.State) == []
    assert table(selected, ext.GroundModuleState) == [{}]
    assert table(selected, ext.GroundCallStack) == []
    assert table(selected, ext.Program) == []

    def project(value: ext.GroundProgramState) -> dict[str, object]:
        return {"planning_state": value.state, "has_caller": value.call_stack is not None}

    projected = Dictionaries()
    tyr_serialization.register_table(projected, ground.State, "states", "s", fields=())
    register_table(projected, ext.GroundProgramState, "program_states", "e", project=project)
    assert serialize(projected, initial) == "e0"
    assert table(projected, ext.GroundProgramState) == [{
        "planning_state": "s0", "has_caller": False,
    }]
    assert tyr_serialization.serialize(projected, initial.state) == "s0"
    assert tyr_serialization.table(projected, ground.State) == [{}]

    reverse = Dictionaries()
    tyr_serialization.register_table(
        reverse, ground.State, "states", "s",
        project=lambda _state: {"program_state": initial},
    )
    register_table(
        reverse, ext.GroundProgramState, "program_states", "e",
        project=lambda value: {"state": value.state},
    )
    assert tyr_serialization.serialize(reverse, initial.state) == "s0"
    assert tyr_serialization.table(reverse, ground.State) == [{"program_state": "e0"}]
    assert table(reverse, ext.GroundProgramState) == [{"state": "s0"}]
    assert serialize(reverse, initial) == "e0"
