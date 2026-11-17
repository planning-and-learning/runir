# Copyright (C) 2026 Dominik Drexler
# SPDX-License-Identifier: GPL-3.0-or-later

import json
import subprocess
import sys
from typing import TypeGuard

import pytest
import pytyr.serialization as tyr_serialization
from ext_execution_utils import collect_steps, initial_node
from fixture_utils import read_fixture
from pytyr.formalism import planning as fp
from pytyr.planning import ground
from pyyggdrasil.serialization import Dictionaries

import pyrunir.serialization as runir_serialization
from pyrunir.datasets import GroundTaskSearchContext, StateGraphEdgeLabel
from pyrunir.kr import DomainContext, GroundTaskContext
from pyrunir.kr.dl import semantics
from pyrunir.kr.ps import base, ext
from pyrunir.kr.ps.base.dl import parse_sketch
from pyrunir.kr.ps.ext.dl import parse_program
from pyrunir.serialization import register_table, serialize, table


def _is_row(value: object) -> TypeGuard[dict[str, object]]:
    if not isinstance(value, dict):
        return False
    keys: list[object] = [*value]
    return all(isinstance(key, str) for key in keys)


def rows(value: object) -> list[dict[str, object]]:
    """Return a serialization table, checked to be a list of rows keyed by field name."""
    assert isinstance(value, list)
    items: list[object] = [*value]
    checked = [row for row in items if _is_row(row)]
    assert len(checked) == len(items)
    return checked


def test_fields_describe_native_layouts_without_instances() -> None:
    assert list(base.RuleBase.Fields.__members__) == ["symbol", "conditions", "effects"]
    assert list(ext.RuleExt.Fields.__members__) == ["symbol", "variant"]
    assert list(base.SketchProofEdgeLabel.Fields.__members__) == ["action", "rule"]
    assert list(ext.ProgramProofEdgeLabel.Fields.__members__) == ["action", "rule"]
    assert list(StateGraphEdgeLabel.Fields.__members__) == ["action", "cost"]
    assert list(semantics.BooleanSemanticNonempty.Fields.__members__) == ["operand"]
    assert list(fp.RowPredicateFluent.Fields.__members__) == ["relation", "objects"]
    for module_state, call_stack, program_state in (
        (ext.ModuleStateGround, ext.ground.CallStack, ext.ProgramStateGround),
        (ext.ModuleStateLifted, ext.lifted.CallStack, ext.ProgramStateLifted),
    ):
        assert list(module_state.Fields.__members__) == [
            "state",
            "mod",
            "memory_state",
            "registers",
            "arguments",
        ]
        assert list(call_stack.Fields.__members__) == [
            "mod",
            "return_memory_state",
            "registers",
            "arguments",
            "caller",
        ]
        assert list(program_state.Fields.__members__) == [
            "program",
            "module_state",
            "call_stack",
        ]


def test_runir_preserves_tyr_field_enum_identity() -> None:
    subprocess.run(
        [
            sys.executable,
            "-c",
            """
from pytyr.formalism import planning as fp
import pytyr.serialization
fields = fp.RowActionLifted.Fields
import pyrunir.serialization
assert fp.RowActionLifted.Fields is fields
""",
        ],
        check=True,
    )


@pytest.mark.parametrize("native_type", ["Rule", "RowActionLifted", "RowPredicateFluent"])
def test_matching_field_enums_are_accepted_by_their_library(native_type: str) -> None:
    dictionaries = Dictionaries()
    if native_type == "Rule":
        runir_serialization.register_table(
            dictionaries, base.RuleBase, "selected", "s", fields=[base.RuleBase.Fields.symbol]
        )
        assert runir_serialization.table(dictionaries, base.RuleBase) == []
    elif native_type == "RowActionLifted":
        tyr_serialization.register_table(
            dictionaries, fp.RowActionLifted, "selected", "s", fields=[fp.RowActionLifted.Fields.objects]
        )
        assert tyr_serialization.table(dictionaries, fp.RowActionLifted) == []
    else:
        tyr_serialization.register_table(
            dictionaries, fp.RowPredicateFluent, "selected", "s", fields=[fp.RowPredicateFluent.Fields.relation]
        )
        assert tyr_serialization.table(dictionaries, fp.RowPredicateFluent) == []


@pytest.mark.parametrize("operand", ["c_top", "r_universal"])
def test_nonempty_requires_registered_operands_or_an_explicit_text_projection(
    gripper_planning_domain: fp.PlanningDomain, operand: str
) -> None:
    context = DomainContext(gripper_planning_domain)
    policy = parse_sketch(
        f"(:sketch (:features (:boolean (:symbol b) (:expression (b_nonempty ({operand}))))) (:rules))",
        gripper_planning_domain,
        context.base_repository,
    )
    (feature,) = policy.get_boolean_features()
    dictionaries = Dictionaries()
    register_table(dictionaries, semantics.BooleanSemanticNonempty, "nonempty", "b")
    expression = feature.get_variant().get_feature()
    with pytest.raises(ValueError, match=r"^Unregistered serialization type: .*Constructor.*BooleanTag"):
        serialize(dictionaries, expression)

    dictionaries = Dictionaries()
    register_table(dictionaries, semantics.ConstructorBoolean, "booleans", "x")
    register_table(dictionaries, semantics.BooleanSemanticNonempty, "nonempty", "b")
    with pytest.raises(ValueError, match=r"^Unregistered serialization type: .*ConceptTag.*RoleTag"):
        serialize(dictionaries, expression)

    dictionaries = Dictionaries()
    register_table(dictionaries, semantics.ConstructorBoolean, "booleans", "x")
    register_table(
        dictionaries,
        semantics.BooleanSemanticNonempty,
        "nonempty",
        "b",
        project=lambda nonempty: {"operand": str(nonempty.get_operand())},
    )
    assert serialize(dictionaries, expression) == "x0"
    assert table(dictionaries, semantics.ConstructorBoolean) == [{"variant": "b0"}]
    assert table(dictionaries, semantics.BooleanSemanticNonempty) == [{"operand": f"({operand})"}]


def test_rule_text_requires_an_explicit_projection(gripper_planning_domain: fp.PlanningDomain) -> None:
    context = DomainContext(gripper_planning_domain)
    policy = parse_sketch(
        read_fixture("kr/ps/base/dl/declared_features.sketch"),
        gripper_planning_domain,
        context.base_repository,
    )
    (rule,) = policy.get_rules()
    feature = policy.get_numerical_features()[0].get_variant()
    dictionaries = Dictionaries()
    register_table(dictionaries, base.RuleBase, "rules", "r")
    with pytest.raises(ValueError, match=r"^Unregistered serialization type: .*Sketch"):
        serialize(dictionaries, policy)

    dictionaries = Dictionaries()
    register_table(dictionaries, base.RuleBase, "rules", "r")
    with pytest.raises(ValueError, match=r"^Unregistered serialization type: .*Condition"):
        serialize(dictionaries, rule)

    dictionaries = Dictionaries()
    register_table(
        dictionaries,
        base.RuleBase,
        "rules",
        "r",
        project=lambda value: {
            "symbol": value.get_symbol(),
            "conditions": [str(condition) for condition in value.get_conditions()],
            "effects": [str(effect) for effect in value.get_effects()],
        },
    )
    register_table(
        dictionaries,
        base.dl.ConcreteFeatureBaseDlNumerical,
        "features",
        "f",
        project=lambda value: {"symbol": value.get_symbol(), "feature": str(value.get_feature())},
    )
    assert serialize(dictionaries, rule) == "r0"
    assert list(rows(table(dictionaries, base.RuleBase))[0]) == list(base.RuleBase.Fields.__members__)
    assert table(dictionaries, base.RuleBase) == [
        {
            "symbol": rule.get_symbol(),
            "conditions": [str(condition) for condition in rule.get_conditions()],
            "effects": [str(effect) for effect in rule.get_effects()],
        }
    ]
    assert table(dictionaries, base.dl.ConcreteFeatureBaseDlNumerical) == []
    assert serialize(dictionaries, feature) == "f0"
    assert table(dictionaries, base.dl.ConcreteFeatureBaseDlNumerical) == [
        {
            "symbol": feature.get_symbol(),
            "feature": str(feature.get_feature()),
        }
    ]

    selected = Dictionaries()
    register_table(
        selected,
        base.RuleBase,
        "rules",
        "r",
        fields=[base.RuleBase.Fields.symbol],
    )
    assert serialize(selected, rule) == "r0"
    (row,) = rows(table(selected, base.RuleBase))
    assert row == {"symbol": rule.get_symbol()}


def test_runir_and_tyr_share_dictionary_references(ground_gripper_search_context: GroundTaskSearchContext) -> None:
    search = ground_gripper_search_context
    domain = search.task.get_formalism_task().get_domain()
    domain_context = DomainContext(domain)
    context = GroundTaskContext(domain_context, search)
    program = parse_program(
        read_fixture("kr/ps/ext/dl/acyclic_calls.program"),
        domain,
        domain_context.ext_repository,
    )
    expander = ext.GroundSuccessorExpander(context, program)
    planning_state = initial_node(context).get_state()
    assert isinstance(planning_state, ground.State)
    initial = expander.initial_state(planning_state)
    (step,) = collect_steps(expander, initial)
    with pytest.raises(ValueError, match=r"^Unregistered serialization type: .*ProgramState.*GroundTag"):
        serialize(Dictionaries(), initial)
    with pytest.raises(ValueError, match=r"^Unregistered serialization type: .*PlanningDomain"):
        tyr_serialization.serialize(Dictionaries(), domain)

    dictionaries = Dictionaries()
    tyr_serialization.register_table(dictionaries, ground.State, "states", "s")
    tyr_serialization.register_table(dictionaries, fp.AtomGroundFluent, "atoms", "a", fields=())
    register_table(dictionaries, ext.ProgramStateGround, "program_states", "e")
    register_table(dictionaries, ext.ModuleStateGround, "module_states", "z")
    register_table(dictionaries, ext.ground.CallStack, "call_stacks", "c")
    register_table(dictionaries, ext.Module, "modules", "m", fields=())
    register_table(dictionaries, ext.MemoryStateExt, "memory_states", "q", fields=())
    register_table(dictionaries, semantics.RegisterValues, "registers", "r")
    register_table(dictionaries, semantics.CallArguments, "arguments", "b")
    register_table(
        dictionaries,
        ext.Program,
        "programs",
        "p",
        project=lambda value: {
            "entry_mod": str(value.get_entry_mod()),
            "modules": [str(module) for module in value.get_modules()],
        },
    )

    assert serialize(dictionaries, initial) == "e0"
    assert tyr_serialization.serialize(dictionaries, initial.get_module_state().get_state().unpack()) == "s0"
    assert serialize(dictionaries, step.target) == "e1"
    assert serialize(dictionaries, initial) == "e0"
    first, second = rows(table(dictionaries, ext.ProgramStateGround))
    assert first == {"program": "p0", "module_state": "z0", "call_stack": None}
    assert second == {"program": "p0", "module_state": "z1", "call_stack": "c0"}
    assert initial.get_call_stack() is None
    assert serialize(dictionaries, initial.get_module_state()) == "z0"
    module_states = rows(table(dictionaries, ext.ModuleStateGround))
    assert module_states[0]["state"] == module_states[1]["state"] == "s0"
    assert module_states[0] == {
        "state": "s0",
        "mod": "m0",
        "memory_state": "q0",
        "registers": "r0",
        "arguments": "b0",
    }
    call_stack = step.target.get_call_stack()
    assert call_stack is not None
    assert table(dictionaries, ext.ground.CallStack) == [
        {
            "mod": "m0",
            "return_memory_state": serialize(dictionaries, call_stack.get_return_memory_state()),
            "registers": "r0",
            "arguments": "b0",
            "caller": None,
        }
    ]
    assert table(dictionaries, semantics.RegisterValues) == [{"concept_values": [], "role_values": []}]
    assert table(dictionaries, semantics.CallArguments) == [
        {
            "concept_arguments": [],
            "role_arguments": [],
            "boolean_arguments": [],
            "numerical_arguments": [],
        }
    ]
    (state,) = rows(tyr_serialization.table(dictionaries, ground.State))
    assert set(state) == {"fluent_ground_atoms", "derived_ground_atoms", "fluent_ground_function_term_values"}
    assert state["fluent_ground_atoms"] == [
        f"a{index}" for index in range(len(rows(tyr_serialization.table(dictionaries, fp.AtomGroundFluent))))
    ]
    assert dictionaries.tables()["program_states"]["rows"] == [first, second]

    assert rows(table(dictionaries, ext.Program))[0]["modules"] == [str(module) for module in program.get_modules()]
    snapshot = dictionaries.tables()
    assert json.loads(json.dumps(snapshot)) == snapshot
    assert serialize(dictionaries, initial) == "e0"
    assert dictionaries.tables() == snapshot

    selected = Dictionaries()
    tyr_serialization.register_table(selected, ground.State, "states", "s")
    register_table(
        selected,
        ext.ProgramStateGround,
        "program_states",
        "e",
        fields=("call_stack", "module_state"),
    )
    register_table(selected, ext.ModuleStateGround, "module_states", "z", fields=())
    register_table(selected, ext.ground.CallStack, "call_stacks", "c", fields=())
    register_table(selected, ext.Program, "programs", "p")
    assert serialize(selected, initial) == "e0"
    (program_state,) = rows(table(selected, ext.ProgramStateGround))
    assert program_state == {"module_state": "z0", "call_stack": None}
    assert list(program_state) == ["module_state", "call_stack"]
    assert tyr_serialization.table(selected, ground.State) == []
    assert table(selected, ext.ModuleStateGround) == [{}]
    assert table(selected, ext.ground.CallStack) == []
    assert table(selected, ext.Program) == []

    def project(value: ext.ProgramStateGround) -> dict[str, object]:
        return {
            "planning_state": value.get_module_state().get_state().unpack(),
            "has_caller": value.get_call_stack() is not None,
        }

    projected = Dictionaries()
    tyr_serialization.register_table(projected, ground.State, "states", "s", fields=())
    register_table(projected, ext.ProgramStateGround, "program_states", "e", project=project)
    assert serialize(projected, initial) == "e0"
    assert table(projected, ext.ProgramStateGround) == [
        {
            "planning_state": "s0",
            "has_caller": False,
        }
    ]
    assert tyr_serialization.serialize(projected, initial.get_module_state().get_state().unpack()) == "s0"
    assert tyr_serialization.table(projected, ground.State) == [{}]

    reverse = Dictionaries()
    tyr_serialization.register_table(
        reverse,
        ground.State,
        "states",
        "s",
        project=lambda _state: {"program_state": initial},
    )
    register_table(
        reverse,
        ext.ProgramStateGround,
        "program_states",
        "e",
        project=lambda value: {"state": value.get_module_state().get_state().unpack()},
    )
    assert tyr_serialization.serialize(reverse, initial.get_module_state().get_state().unpack()) == "s0"
    assert tyr_serialization.table(reverse, ground.State) == [{"program_state": "e0"}]
    assert table(reverse, ext.ProgramStateGround) == [{"state": "s0"}]
    assert serialize(reverse, initial) == "e0"
