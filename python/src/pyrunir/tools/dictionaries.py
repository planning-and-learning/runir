"""Native Runir entities selected for output, with shared compact aliases."""

from __future__ import annotations

from typing import TypeAlias

from pytyr.planning import ground, lifted
from pytyr import tools as tyr_tools

from pyrunir.kr import uns
from pyrunir.kr.ps import base, ext


Feature: TypeAlias = (
    base.dl.BooleanFeature | base.dl.NumericalFeature
    | ext.dl.ConceptFeature | ext.dl.RoleFeature
    | ext.dl.BooleanFeature | ext.dl.NumericalFeature
    | uns.dl.BooleanFeature
)


class Dictionaries:
    def __init__(self, task: ground.Task | lifted.Task) -> None:
        self.planning = tyr_tools.Dictionaries(task)
        self.features: dict[Feature, str] = {}
        self.rules: dict[base.Rule, str] = {}
        self.module_rules: dict[ext.RuleVariant, str] = {}
        self.modules: dict[ext.Module, str] = {}
        self.memories: dict[ext.MemoryState, str] = {}
        self.programs: dict[ext.ModuleProgram, str] = {}
        self.call_stacks: dict[ext.GroundCallStack | ext.LiftedCallStack, str] = {}
        self.execution_states: dict[ext.GroundExecutionState | ext.LiftedExecutionState, str] = {}

    def feature(self, feature: Feature) -> str:
        return self.features.setdefault(feature, f"f{len(self.features)}")

    def rule(self, rule: base.Rule) -> str:
        if rule not in self.rules:
            self.rules[rule] = f"r{len(self.rules)}"
            for condition in rule.get_conditions():
                self.feature(condition.get_variant().get_variant().get_feature())
            for effect in rule.get_effects():
                self.feature(effect.get_variant().get_variant().get_feature())
        return self.rules[rule]

    def module_rule(self, rule: ext.RuleVariant) -> str:
        if rule not in self.module_rules:
            self.module_rules[rule] = f"R{len(self.module_rules)}"
            variant = rule.get_variant()
            self.memory(variant.get_source())
            self.memory(variant.get_target())
            if isinstance(variant, ext.ConceptLoadRule | ext.RoleLoadRule):
                self.feature(variant.get_feature())
            elif isinstance(variant, ext.DoRule):
                for feature in variant.get_action_arguments():
                    self.feature(feature)
        return self.module_rules[rule]

    def module(self, module: ext.Module) -> str:
        if module not in self.modules:
            self.modules[module] = f"M{len(self.modules)}"
            for feature in (
                *module.get_concept_features(), *module.get_role_features(),
                *module.get_boolean_features(), *module.get_numerical_features(),
            ):
                self.feature(feature)
            self.memory(module.get_entry_memory_state())
            for memory in module.get_memory_states():
                self.memory(memory)
            for transition in module.get_memory_transitions():
                for rule in transition:
                    self.module_rule(rule)
        return self.modules[module]

    def memory(self, memory: ext.MemoryState) -> str:
        return self.memories.setdefault(memory, f"m{len(self.memories)}")

    def program(self, program: ext.ModuleProgram) -> str:
        if program not in self.programs:
            self.programs[program] = f"P{len(self.programs)}"
            self.module(program.get_entry_module())
            for module in program.get_modules():
                self.module(module)
        return self.programs[program]

    def call_stack(self, stack: ext.GroundCallStack | ext.LiftedCallStack) -> str:
        if stack not in self.call_stacks:
            self.call_stacks[stack] = f"c{len(self.call_stacks)}"
            self.module(stack.module)
            self.memory(stack.memory_state)
            caller = stack.caller
            if caller is not None:
                self.call_stack(caller)
        return self.call_stacks[stack]

    def execution_state(self, state: ext.GroundExecutionState | ext.LiftedExecutionState) -> str:
        if state not in self.execution_states:
            self.planning.include_state(state.state)
            self.execution_states[state] = f"e{len(self.execution_states)}"
            self.program(state.program)
            self.call_stack(state.call_stack)
        return self.execution_states[state]
