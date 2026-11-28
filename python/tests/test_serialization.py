# Copyright (C) 2026 Dominik Drexler
# SPDX-License-Identifier: GPL-3.0-or-later

import json
import subprocess
import sys
from typing import TypeGuard

import pytest
from ext_execution_utils import collect_steps, initial_node
from fixture_utils import read_fixture
from pytyr import formalism
from pytyr.planning import ground
from pyyggdrasil.serialization import Dictionaries

from pyrunir.datasets import GroundTaskSearchContext, StateGraphEdgeLabelData
from pyrunir.kr import DomainContext, GroundTaskContext
from pyrunir.kr.dl import semantics
from pyrunir.kr.ps import base, ext
from pyrunir.kr.ps.base.dl import parse_sketch
from pyrunir.kr.ps.ext.dl import parse_program


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
    assert list(base.SketchProofEdgeLabelData.Fields.__members__) == ["action", "rule"]
    assert list(ext.ProgramProofEdgeLabelExtData.Fields.__members__) == ["action", "rule"]
    assert list(StateGraphEdgeLabelData.Fields.__members__) == ["action", "cost"]
    assert list(semantics.BooleanSemanticNonempty.Fields.__members__) == ["operand"]
    assert list(formalism.RowPredicateFluent.Fields.__members__) == ["relation", "values"]
    assert list(ext.ModuleState.Fields.__members__) == ["mod", "memory_state", "registers", "arguments"]
    assert list(ext.CallStack.Fields.__members__) == ["module_state", "caller"]
    for program_state in (ext.ProgramStateGround, ext.ProgramStateLifted):
        assert list(program_state.Fields.__members__) == ["state", "module_state", "call_stack"]


def test_runir_preserves_tyr_field_enum_identity() -> None:
    subprocess.run(
        [
            sys.executable,
            "-c",
            """
from pytyr import formalism
fields = formalism.RowActionLifted.Fields
import pyrunir
assert formalism.RowActionLifted.Fields is fields
""",
        ],
        check=True,
    )


@pytest.mark.parametrize("native_type", ["Rule", "RowActionLifted", "RowPredicateFluent"])
def test_matching_field_enums_are_accepted_by_their_library(native_type: str) -> None:
    dictionaries = Dictionaries()
    if native_type == "Rule":
        dictionaries.register_table(base.RuleBase, "selected", "s", fields=[base.RuleBase.Fields.symbol])
        assert dictionaries.table(base.RuleBase) == []
    elif native_type == "RowActionLifted":
        dictionaries.register_table(
            formalism.RowActionLifted, "selected", "s", fields=[formalism.RowActionLifted.Fields.values]
        )
        assert dictionaries.table(formalism.RowActionLifted) == []
    else:
        dictionaries.register_table(
            formalism.RowPredicateFluent,
            "selected",
            "s",
            fields=[formalism.RowPredicateFluent.Fields.relation],
        )
        assert dictionaries.table(formalism.RowPredicateFluent) == []


@pytest.mark.parametrize(("operand", "operand_tag"), [("c_top", "ConceptTag"), ("r_universal", "RoleTag")])
def test_nonempty_requires_registered_operands_or_an_explicit_text_projection(
    gripper_planning_domain: formalism.PlanningDomain, operand: str, operand_tag: str
) -> None:
    context = DomainContext(gripper_planning_domain)
    policy = parse_sketch(
        f"(:sketch (:features (:boolean (:symbol b) (:expression (b_nonempty ({operand}))))) (:rules))",
        gripper_planning_domain,
        context.base_repository,
    )
    (feature,) = policy.boolean_features()
    dictionaries = Dictionaries()
    dictionaries.register_table(semantics.BooleanSemanticNonempty, "nonempty", "b")
    expression = feature.variant().feature()
    with pytest.raises(ValueError, match=r"^Unregistered serialization type: .*Constructor.*BooleanTag"):
        dictionaries.serialize(expression)

    dictionaries = Dictionaries()
    dictionaries.register_table(semantics.ConstructorBoolean, "booleans", "x")
    dictionaries.register_table(semantics.BooleanSemanticNonempty, "nonempty", "b")
    # The message names the operand's own kind, the one that is unregistered.
    with pytest.raises(ValueError, match=rf"^Unregistered serialization type: .*Constructor<[^>]*{operand_tag}>"):
        dictionaries.serialize(expression)

    dictionaries = Dictionaries()
    dictionaries.register_table(semantics.ConstructorBoolean, "booleans", "x")
    dictionaries.register_table(
        semantics.BooleanSemanticNonempty,
        "nonempty",
        "b",
        project=lambda nonempty: {"operand": str(nonempty.operand())},
    )
    assert dictionaries.serialize(expression) == "x0"
    assert dictionaries.table(semantics.ConstructorBoolean) == [{"variant": "b0", "is_static": True}]
    assert dictionaries.table(semantics.BooleanSemanticNonempty) == [{"operand": f"({operand})"}]


def test_rule_text_requires_an_explicit_projection(gripper_planning_domain: formalism.PlanningDomain) -> None:
    context = DomainContext(gripper_planning_domain)
    policy = parse_sketch(
        read_fixture("kr/ps/base/dl/declared_features.sketch"),
        gripper_planning_domain,
        context.base_repository,
    )
    (rule,) = policy.rules()
    feature = policy.numerical_features()[0].variant()
    dictionaries = Dictionaries()
    dictionaries.register_table(base.RuleBase, "rules", "r")
    with pytest.raises(ValueError, match=r"^Unregistered serialization type: .*Sketch"):
        dictionaries.serialize(policy)

    dictionaries = Dictionaries()
    dictionaries.register_table(base.RuleBase, "rules", "r")
    with pytest.raises(ValueError, match=r"^Unregistered serialization type: .*Condition"):
        dictionaries.serialize(rule)

    dictionaries = Dictionaries()
    dictionaries.register_table(
        base.RuleBase,
        "rules",
        "r",
        project=lambda value: {
            "symbol": value.symbol(),
            "conditions": [str(condition) for condition in value.conditions()],
            "effects": [str(effect) for effect in value.effects()],
        },
    )
    dictionaries.register_table(
        base.dl.ConcreteFeatureBaseNumerical,
        "features",
        "f",
        project=lambda value: {"symbol": value.symbol(), "feature": str(value.feature())},
    )
    assert dictionaries.serialize(rule) == "r0"
    assert list(rows(dictionaries.table(base.RuleBase))[0]) == list(base.RuleBase.Fields.__members__)
    assert dictionaries.table(base.RuleBase) == [
        {
            "symbol": rule.symbol(),
            "conditions": [str(condition) for condition in rule.conditions()],
            "effects": [str(effect) for effect in rule.effects()],
        }
    ]
    assert dictionaries.table(base.dl.ConcreteFeatureBaseNumerical) == []
    assert dictionaries.serialize(feature) == "f0"
    assert dictionaries.table(base.dl.ConcreteFeatureBaseNumerical) == [
        {
            "symbol": feature.symbol(),
            "feature": str(feature.feature()),
        }
    ]

    selected = Dictionaries()
    selected.register_table(
        base.RuleBase,
        "rules",
        "r",
        fields=[base.RuleBase.Fields.symbol],
    )
    assert selected.serialize(rule) == "r0"
    (row,) = rows(selected.table(base.RuleBase))
    assert row == {"symbol": rule.symbol()}


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
    planning_state = initial_node(context).state()
    assert isinstance(planning_state, ground.StateBuilderView)
    initial = expander.initial_state(planning_state)
    (step,) = collect_steps(expander, initial)
    with pytest.raises(ValueError, match=r"^Unregistered serialization type: .*ProgramState.*GroundTag"):
        Dictionaries().serialize(initial)
    with pytest.raises(ValueError, match=r"^Unregistered serialization type: .*PlanningDomain"):
        Dictionaries().serialize(domain)

    dictionaries = Dictionaries()
    # A registered state is a record of its storages' keys: referenced here, its storages left out.
    dictionaries.register_table(ground.State, "states", "s", fields=())
    dictionaries.register_table(ext.ProgramStateGround, "program_states", "e")
    dictionaries.register_table(ext.ModuleState, "module_states", "z")
    dictionaries.register_table(ext.CallStack, "call_stacks", "c")
    dictionaries.register_table(ext.Module, "modules", "m", fields=())
    dictionaries.register_table(ext.MemoryStateExt, "memory_states", "q", fields=())
    dictionaries.register_table(semantics.RegisterValues, "registers", "r")
    dictionaries.register_table(semantics.CallArguments, "arguments", "b")
    dictionaries.register_table(
        ext.Program,
        "programs",
        "p",
        project=lambda value: {
            "entry_mod": str(value.entry_mod()),
            "modules": [str(module) for module in value.modules()],
        },
    )

    assert dictionaries.serialize(initial) == "e0"
    assert dictionaries.serialize(initial.state()) == "s0"
    assert dictionaries.serialize(step.target) == "e1"
    assert dictionaries.serialize(initial) == "e0"
    first, second = rows(dictionaries.table(ext.ProgramStateGround))
    assert first == {"state": "s0", "module_state": "z0", "call_stack": None}
    assert second == {"state": "s0", "module_state": "z1", "call_stack": "c0"}
    assert initial.call_stack() is None
    assert dictionaries.serialize(initial.module_state()) == "z0"
    module_states = rows(dictionaries.table(ext.ModuleState))
    assert module_states[0] == {
        "mod": "m0",
        "memory_state": "q0",
        "registers": "r0",
        "arguments": "b0",
    }
    call_stack = step.target.call_stack()
    assert call_stack is not None
    assert dictionaries.table(ext.CallStack) == [
        {
            "module_state": dictionaries.serialize(call_stack.module_state()),
            "caller": None,
        }
    ]
    assert dictionaries.table(semantics.RegisterValues) == [{"concept_values": [], "role_values": []}]
    assert dictionaries.table(semantics.CallArguments) == [
        {
            "concept_arguments": [],
            "role_arguments": [],
            "boolean_arguments": [],
            "numerical_arguments": [],
        }
    ]
    assert dictionaries.table(ground.State) == [{}]
    assert dictionaries.tables()["program_states"]["rows"] == [first, second]

    # A program state does not name its program (an input to the search): it is serialized on its own.
    assert dictionaries.serialize(program) == "p0"
    assert rows(dictionaries.table(ext.Program))[0]["modules"] == [str(module) for module in program.modules()]
    snapshot = dictionaries.tables()
    assert json.loads(json.dumps(snapshot)) == snapshot
    assert dictionaries.serialize(initial) == "e0"
    assert dictionaries.tables() == snapshot

    selected = Dictionaries()
    selected.register_table(ground.State, "states", "s")
    selected.register_table(
        ext.ProgramStateGround,
        "program_states",
        "e",
        fields=("call_stack", "module_state"),
    )
    selected.register_table(ext.ModuleState, "module_states", "z", fields=())
    selected.register_table(ext.CallStack, "call_stacks", "c", fields=())
    selected.register_table(ext.Program, "programs", "p")
    assert selected.serialize(initial) == "e0"
    (program_state,) = rows(selected.table(ext.ProgramStateGround))
    assert program_state == {"module_state": "z0", "call_stack": None}
    assert list(program_state) == ["module_state", "call_stack"]
    assert selected.table(ground.State) == []
    assert selected.table(ext.ModuleState) == [{}]
    assert selected.table(ext.CallStack) == []
    assert selected.table(ext.Program) == []

    def project(value: ext.ProgramStateGround) -> dict[str, object]:
        return {
            "planning_state": value.state(),
            "has_caller": value.call_stack() is not None,
        }

    projected = Dictionaries()
    projected.register_table(ground.State, "states", "s", fields=())
    projected.register_table(ext.ProgramStateGround, "program_states", "e", project=project)
    assert projected.serialize(initial) == "e0"
    assert projected.table(ext.ProgramStateGround) == [
        {
            "planning_state": "s0",
            "has_caller": False,
        }
    ]
    assert projected.serialize(initial.state()) == "s0"
    assert projected.table(ground.State) == [{}]

    reverse = Dictionaries()
    reverse.register_table(
        ground.State,
        "states",
        "s",
        project=lambda _state: {"program_state": initial},
    )
    reverse.register_table(
        ext.ProgramStateGround,
        "program_states",
        "e",
        project=lambda value: {"state": value.state()},
    )
    assert reverse.serialize(initial.state()) == "s0"
    assert reverse.table(ground.State) == [{"program_state": "e0"}]
    assert reverse.table(ext.ProgramStateGround) == [{"state": "s0"}]
    assert reverse.serialize(initial) == "e0"
