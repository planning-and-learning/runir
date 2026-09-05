"""Compressed representations of native Runir entities using shared dictionaries."""

from __future__ import annotations

from typing import TypedDict

from pytyr.tools.output import DictionariesJSON as PlanningDictionariesJSON
from pytyr.tools.output import format_dictionaries as format_planning_dictionaries

from pyrunir.kr import uns
from pyrunir.kr.ps import base, ext

from .dictionaries import Dictionaries, Feature


# Shared symbols and graph edges


class SymbolJSON(TypedDict):
    symbol: str
    expression: str


def format_feature(feature: Feature) -> SymbolJSON:
    return {"symbol": feature.get_symbol(), "expression": str(feature.get_expression())}


class ProofEdgeJSON(TypedDict):
    index: int
    source: int
    target: int
    action: str | None
    cost: float | None
    rule: str | None


class PolicyEdgeJSON(TypedDict):
    index: int
    source: int
    target: int
    rule: str


# Sketches


def format_rule(rule: base.Rule) -> SymbolJSON:
    return {"symbol": rule.get_symbol(), "expression": str(rule)}


class SketchJSON(TypedDict):
    boolean_features: list[str]
    numerical_features: list[str]
    rules: list[str]


def format_sketch(sketch: base.Sketch, dictionaries: Dictionaries) -> SketchJSON:
    return {
        "boolean_features": [dictionaries.feature(feature) for feature in sketch.get_boolean_features()],
        "numerical_features": [dictionaries.feature(feature) for feature in sketch.get_numerical_features()],
        "rules": [dictionaries.rule(rule) for rule in sketch.get_rules()],
    }


class SketchVertexJSON(TypedDict):
    index: int
    state: str
    is_initial: bool
    is_goal: bool
    is_alive: bool
    is_unsolvable: bool


class SketchGraphJSON(TypedDict):
    vertices: list[SketchVertexJSON]
    edges: list[ProofEdgeJSON]


def format_sketch_graph(
    graph: base.GroundSketchProofGraph | base.LiftedSketchProofGraph, dictionaries: Dictionaries,
) -> SketchGraphJSON:
    vertices: list[SketchVertexJSON] = []
    for index in graph.get_vertex_indices():
        vertex = graph.get_vertex_property(index)
        dictionaries.planning.include_state(vertex.state)
        vertices.append({
            "index": index, "state": dictionaries.planning.states[vertex.state],
            "is_initial": vertex.is_initial, "is_goal": vertex.is_goal,
            "is_alive": vertex.is_alive, "is_unsolvable": vertex.is_unsolvable,
        })
    edges: list[ProofEdgeJSON] = []
    for index in graph.get_edge_indices():
        edge = graph.get_edge_property(index)
        edges.append({
            "index": index, "source": graph.get_source(index), "target": graph.get_target(index),
            "action": dictionaries.planning.action(edge.transition.action), "cost": edge.transition.cost,
            "rule": dictionaries.rule(edge.rule),
        })
    return {"vertices": vertices, "edges": edges}


class PolicyVertexJSON(TypedDict):
    index: int
    boolean_values: dict[str, bool]
    numerical_values: dict[str, bool]


class PolicyGraphJSON(TypedDict):
    vertices: list[PolicyVertexJSON]
    edges: list[PolicyEdgeJSON]


def format_policy_graph(graph: base.dl.PolicyGraph, sketch: base.Sketch, dictionaries: Dictionaries) -> PolicyGraphJSON:
    booleans = [dictionaries.feature(feature) for feature in sketch.get_boolean_features()]
    numericals = [dictionaries.feature(feature) for feature in sketch.get_numerical_features()]
    vertices: list[PolicyVertexJSON] = []
    for index in graph.get_vertex_indices():
        vertex = graph.get_vertex_property(index)
        vertices.append({
            "index": index,
            "boolean_values": dict(zip(booleans, vertex.boolean_values, strict=True)),
            "numerical_values": dict(zip(numericals, vertex.numerical_values, strict=True)),
        })
    return {"vertices": vertices, "edges": [
        {"index": index, "source": graph.get_source(index), "target": graph.get_target(index),
         "rule": dictionaries.rule(graph.get_edge_property(index))}
        for index in graph.get_edge_indices()
    ]}


# Modules


class ModuleRuleJSON(SymbolJSON):
    source: str
    target: str


def format_module_rule(rule: ext.RuleVariant, dictionaries: Dictionaries) -> ModuleRuleJSON:
    dictionaries.module_rule(rule)
    variant = rule.get_variant()
    return {
        "symbol": rule.get_symbol(),
        "expression": str(rule),
        "source": dictionaries.memory(variant.get_source()),
        "target": dictionaries.memory(variant.get_target()),
    }


class ModuleJSON(TypedDict):
    name: str
    concept_arguments: list[str]
    role_arguments: list[str]
    boolean_arguments: list[str]
    numerical_arguments: list[str]
    concept_registers: list[str]
    role_registers: list[str]
    concept_features: list[str]
    role_features: list[str]
    boolean_features: list[str]
    numerical_features: list[str]
    entry_memory_state: str
    memory_states: list[str]
    rules: list[str]


def format_module(module: ext.Module, dictionaries: Dictionaries) -> ModuleJSON:
    dictionaries.module(module)
    return {
        "name": module.get_name(),
        "concept_arguments": [str(argument) for argument in module.get_concept_arguments()],
        "role_arguments": [str(argument) for argument in module.get_role_arguments()],
        "boolean_arguments": [str(argument) for argument in module.get_boolean_arguments()],
        "numerical_arguments": [str(argument) for argument in module.get_numerical_arguments()],
        "concept_registers": [str(register) for register in module.get_concept_registers()],
        "role_registers": [str(register) for register in module.get_role_registers()],
        "concept_features": [dictionaries.feature(feature) for feature in module.get_concept_features()],
        "role_features": [dictionaries.feature(feature) for feature in module.get_role_features()],
        "boolean_features": [dictionaries.feature(feature) for feature in module.get_boolean_features()],
        "numerical_features": [dictionaries.feature(feature) for feature in module.get_numerical_features()],
        "entry_memory_state": dictionaries.memory(module.get_entry_memory_state()),
        "memory_states": [dictionaries.memory(memory) for memory in module.get_memory_states()],
        "rules": [dictionaries.module_rule(rule) for transition in module.get_memory_transitions() for rule in transition],
    }


class ModuleProgramJSON(TypedDict):
    entry_module: str
    modules: list[str]


def format_module_program(program: ext.ModuleProgram, dictionaries: Dictionaries) -> ModuleProgramJSON:
    dictionaries.program(program)
    return {
        "entry_module": dictionaries.module(program.get_entry_module()),
        "modules": [dictionaries.module(module) for module in program.get_modules()],
    }


class RegistersJSON(TypedDict):
    concepts: list[str | None]
    roles: list[list[str] | None]


class CallArgumentsJSON(TypedDict):
    concepts: list[str]
    roles: list[str]
    booleans: list[bool]
    numericals: list[int]


class CallStackJSON(TypedDict):
    module: str
    memory_state: str
    registers: RegistersJSON
    arguments: CallArgumentsJSON
    caller: str | None


def format_call_stack(stack: ext.GroundCallStack | ext.LiftedCallStack, dictionaries: Dictionaries) -> CallStackJSON:
    dictionaries.call_stack(stack)
    registers = stack.registers
    arguments = stack.arguments
    caller = stack.caller
    return {
        "module": dictionaries.module(stack.module),
        "memory_state": dictionaries.memory(stack.memory_state),
        "registers": {
            "concepts": [None if value is None else str(value) for value in registers.concept_values],
            "roles": [None if value is None else [str(value[0]), str(value[1])] for value in registers.role_values],
        },
        "arguments": {
            "concepts": [str(value) for value in arguments.concept_arguments],
            "roles": [str(value) for value in arguments.role_arguments],
            "booleans": [value.get() for value in arguments.boolean_arguments],
            "numericals": [value.get() for value in arguments.numerical_arguments],
        },
        "caller": None if caller is None else dictionaries.call_stack(caller),
    }


class ExecutionStateJSON(TypedDict):
    state: str
    program: str
    phase: str
    call_stack: str


def format_execution_state(
    state: ext.GroundExecutionState | ext.LiftedExecutionState, dictionaries: Dictionaries,
) -> ExecutionStateJSON:
    dictionaries.execution_state(state)
    return {
        "state": dictionaries.planning.states[state.state],
        "program": dictionaries.program(state.program),
        "phase": state.phase.name,
        "call_stack": dictionaries.call_stack(state.call_stack),
    }


class ModuleVertexJSON(TypedDict):
    index: int
    execution_state: str
    is_initial: bool
    is_goal: bool
    is_alive: bool
    is_unsolvable: bool


class ModuleGraphJSON(TypedDict):
    vertices: list[ModuleVertexJSON]
    edges: list[ProofEdgeJSON]


def format_module_graph(
    graph: ext.GroundModuleProgramProofGraph | ext.LiftedModuleProgramProofGraph, dictionaries: Dictionaries,
) -> ModuleGraphJSON:
    vertices: list[ModuleVertexJSON] = []
    for index in graph.get_vertex_indices():
        vertex = graph.get_vertex_property(index)
        vertices.append({
            "index": index, "execution_state": dictionaries.execution_state(vertex.execution_state),
            "is_initial": vertex.is_initial, "is_goal": vertex.is_goal,
            "is_alive": vertex.is_alive, "is_unsolvable": vertex.is_unsolvable,
        })
    edges: list[ProofEdgeJSON] = []
    for index in graph.get_edge_indices():
        edge = graph.get_edge_property(index)
        transition = edge.state_transition
        edges.append({
            "index": index, "source": graph.get_source(index), "target": graph.get_target(index),
            "action": None if transition is None else dictionaries.planning.action(transition.action),
            "cost": None if transition is None else transition.cost,
            "rule": None if edge.rule is None else dictionaries.module_rule(edge.rule),
        })
    return {"vertices": vertices, "edges": edges}


class ModulePolicyVertexJSON(PolicyVertexJSON):
    memory_state: str


class ModulePolicyGraphJSON(TypedDict):
    vertices: list[ModulePolicyVertexJSON]
    edges: list[PolicyEdgeJSON]


def format_module_policy_graph(
    graph: ext.dl.ModulePolicyGraph, module: ext.Module, dictionaries: Dictionaries,
) -> ModulePolicyGraphJSON:
    dictionaries.module(module)
    booleans = [dictionaries.feature(feature) for feature in module.get_boolean_features()]
    numericals = [dictionaries.feature(feature) for feature in module.get_numerical_features()]
    vertices: list[ModulePolicyVertexJSON] = []
    for index in graph.get_vertex_indices():
        vertex = graph.get_vertex_property(index)
        vertices.append({
            "index": index, 
            "memory_state": dictionaries.memory(vertex.memory_state),
            "boolean_values": dict(zip(booleans, vertex.boolean_values, strict=True)),
            "numerical_values": dict(zip(numericals, vertex.numerical_values, strict=True)),
        })
    return {"vertices": vertices, "edges": [
        {"index": index, "source": graph.get_source(index), "target": graph.get_target(index),
         "rule": dictionaries.module_rule(graph.get_edge_property(index))}
        for index in graph.get_edge_indices()
    ]}


# Classifiers


class ClassifierLiteralJSON(TypedDict):
    feature: str
    polarity: bool


class ClassifierJSON(TypedDict):
    symbol: str
    features: list[str]
    clauses: list[list[ClassifierLiteralJSON]]


def format_classifier(classifier: uns.Classifier, dictionaries: Dictionaries) -> ClassifierJSON:
    return {
        "symbol": classifier.get_symbol(),
        "features": [dictionaries.feature(feature) for feature in classifier.get_features()],
        "clauses": [
            [{"feature": dictionaries.feature(literal.get_feature()), "polarity": literal.get_polarity()}
             for literal in clause.get_literals()]
            for clause in classifier.get_clauses()
        ],
    }


# Dictionaries


class DictionariesJSON(TypedDict, total=False):
    planning: PlanningDictionariesJSON
    features: dict[str, SymbolJSON]
    rules: dict[str, SymbolJSON]
    module_rules: dict[str, ModuleRuleJSON]
    modules: dict[str, ModuleJSON]
    memories: dict[str, str]
    programs: dict[str, ModuleProgramJSON]
    call_stacks: dict[str, CallStackJSON]
    execution_states: dict[str, ExecutionStateJSON]


def format_dictionaries(dictionaries: Dictionaries) -> DictionariesJSON:
    tables: DictionariesJSON = {
        "planning": format_planning_dictionaries(dictionaries.planning),
        "features": {alias: format_feature(feature) for feature, alias in dictionaries.features.items()},
        "rules": {alias: format_rule(rule) for rule, alias in dictionaries.rules.items()},
        "module_rules": {alias: format_module_rule(rule, dictionaries) for rule, alias in dictionaries.module_rules.items()},
        "modules": {alias: format_module(module, dictionaries) for module, alias in dictionaries.modules.items()},
        "memories": {alias: memory.get_name() for memory, alias in dictionaries.memories.items()},
        "programs": {alias: format_module_program(program, dictionaries) for program, alias in dictionaries.programs.items()},
        "call_stacks": {alias: format_call_stack(stack, dictionaries) for stack, alias in dictionaries.call_stacks.items()},
        "execution_states": {alias: format_execution_state(state, dictionaries) for state, alias in dictionaries.execution_states.items()},
    }
    for name in ("planning", "features", "rules", "module_rules", "modules", "memories", "programs", "call_stacks", "execution_states"):
        if not tables[name]:
            del tables[name]
    return tables
