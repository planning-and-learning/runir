# Copyright (C) 2026 Dominik Drexler
# SPDX-License-Identifier: GPL-3.0-or-later

"""Graph and program helpers of the exploration session."""

from __future__ import annotations

import re
from collections import deque
from collections.abc import Mapping

from pytyr.formalism.planning import Object
from pytyr.planning import lifted

from pyrunir.kr.ps.ext import (
    ConceptChooseRule,
    LiftedModuleState,
    LiftedProgramProofGraph,
    LiftedProgramProofResults,
    ProgramProofStatus,
    RoleChooseRule,
    RuleVariant,
)
from pyrunir.kr.ps.ext.explore.results import Value


def is_choose(rule: RuleVariant | None) -> bool:
    """Return whether `rule` is a concept or role Choose rule."""
    return rule is not None and isinstance(rule.get_variant(), (ConceptChooseRule, RoleChooseRule))


def failure_category(result: LiftedProgramProofResults) -> str | None:
    """Why a search did not succeed; None if it did.

    Open and dead-end states also occur in failed branches of a success.
    """
    if result.status == ProgramProofStatus.SUCCESS:
        return None
    if result.open_states:
        return "open_state"
    if result.deadend_states:
        return "empty_choose"
    return result.status.name.lower()


def _postorder(graph: LiftedProgramProofGraph) -> list[int]:
    """All vertices of `graph`, each after the successors first reached from it by depth-first search."""
    order: list[int] = []
    seen: set[int] = set()
    for root in graph.get_vertex_indices():
        if root in seen:
            continue
        stack = [(root, iter(graph.get_successor_indices(root)))]
        seen.add(root)
        while stack:
            vertex, successors = stack[-1]
            child = next(successors, None)
            if child is None:
                order.append(vertex)
                stack.pop()
            elif child not in seen:
                seen.add(child)
                stack.append((child, iter(graph.get_successor_indices(child))))
    return order


def vertex_status(result: LiftedProgramProofResults) -> dict[int, bool | None]:
    """True: a goal is reached from the vertex; False: every continuation fails; None: unknown (not fully explored).

    A vertex with Choose out-edges succeeds if one tried binding does; other vertices need all their successors.
    """
    graph = result.graph
    failed = set(result.open_states) | set(result.deadend_states)
    status: dict[int, bool | None] = {}
    for vertex in _postorder(graph):
        label = graph.get_vertex_property(vertex)
        edges = list(graph.get_out_edge_indices(vertex))
        if label.is_goal:
            status[vertex] = True
        elif vertex in failed or label.is_unsolvable:
            status[vertex] = False
        elif not edges:
            status[vertex] = None
        else:
            values = [status.get(graph.get_target(edge)) for edge in edges]
            if any(is_choose(graph.get_edge_property(edge).rule) for edge in edges):
                status[vertex] = (
                    True if any(v is True for v in values) else (False if all(v is False for v in values) else None)
                )
            else:
                status[vertex] = (
                    False if any(v is False for v in values) else (True if all(v is True for v in values) else None)
                )
    return status


def shortest_path(graph: LiftedProgramProofGraph, start: int, targets: set[int]) -> list[int]:
    """A shortest path from `start` to one of `targets`, or [] if none."""
    parents: dict[int, int | None] = {start: None}
    queue = deque([start])
    while queue:
        vertex = queue.popleft()
        if vertex in targets:
            path = [vertex]
            while (parent := parents[path[-1]]) is not None:
                path.append(parent)
            return path[::-1]
        for child in graph.get_successor_indices(vertex):
            if child not in parents:
                parents[child] = vertex
                queue.append(child)
    return []


def reachable_count(graph: LiftedProgramProofGraph, start: int) -> int:
    """Return the number of vertices reachable from `start`, including `start` itself."""
    seen = {start}
    stack = [start]
    while stack:
        for child in graph.get_successor_indices(stack.pop()):
            if child not in seen:
                seen.add(child)
                stack.append(child)
    return len(seen)


def deltas(before: Mapping[str, Value], after: Mapping[str, Value]) -> dict[str, tuple[Value, Value]]:
    """Map each name whose value changed from `before` to `after` to its (before, after) pair."""
    return {name: (value, after[name]) for name, value in before.items() if after[name] != value}


def object_name(value: Object | tuple[Object, Object] | None) -> str | None:
    """Return the name of an object, `(a b)` for an object pair, or None for no value."""
    if value is None:
        return None
    if isinstance(value, tuple):
        return f"({value[0].get_name()} {value[1].get_name()})"
    return str(value.get_name())


def register_value(module_state: LiftedModuleState, rule: RuleVariant) -> str | None:
    """Return the name of the value held by the register that the Choose rule `rule` binds.

    Raises:
        TypeError: If `rule` is not a Choose rule.
    """
    variant = rule.get_variant()
    registers = module_state.registers
    if isinstance(variant, ConceptChooseRule):
        return object_name(registers.concept_values[variant.get_register().get_identifier().value()])
    if isinstance(variant, RoleChooseRule):
        return object_name(registers.role_values[variant.get_register().get_identifier().value()])
    msg = f"{rule.get_symbol()} is not a Choose rule"
    raise TypeError(msg)


_RULE_BLOCK = re.compile(r"\(:rule\s+\(:symbol\s+([^\s()]+)\)")


def rewrite_choose_as_load(text: str, rule: str) -> str:
    """Replace every Choose body of `rule` by a Load with the same conditions, candidates, register and effects."""
    for match in _RULE_BLOCK.finditer(text):
        if match.group(1) != rule:
            continue
        depth, end = 0, match.start()
        for end in range(match.start(), len(text)):
            depth += {"(": 1, ")": -1}.get(text[end], 0)
            if depth == 0:
                break
        block = text[match.start() : end + 1]
        block = re.sub(r"\(:order(?:\s+\((?:min|max)\s+[^\s()]+\))*\s*\)", "", block)
        block = block.replace("(:choose", "(:load")
        return text[: match.start()] + block + text[end + 1 :]
    raise KeyError(rule)


def object_types(state: lifted.State) -> dict[str, frozenset[str]]:
    """Each object's static unary predicates, which include its domain types."""
    types: dict[str, set[str]] = {}
    for atom in state.static_atoms():
        objects = atom.get_objects()
        if len(objects) == 1:
            types.setdefault(str(objects[0].get_name()), set()).add(str(atom.get_predicate().get_name()))
    return {name: frozenset(values) for name, values in types.items()}


def object_atoms(state: lifted.State, name: str) -> list[tuple[str, tuple[str, ...]]]:
    """The static, fluent and derived atoms of `state` that mention the object `name`, with `*` for it."""
    atoms: list[tuple[str, tuple[str, ...]]] = []
    for atom in (*state.static_atoms(), *state.fluent_atoms(), *state.derived_atoms()):
        objects = tuple(str(item.get_name()) for item in atom.get_objects())
        if name in objects:
            atoms.append(
                (str(atom.get_predicate().get_name()), tuple("*" if item == name else item for item in objects))
            )
    return atoms


def distinguishing_atoms(
    profiles: list[list[tuple[str, tuple[str, ...]]]], types: dict[str, frozenset[str]], bindings: list[str]
) -> list[tuple[str, ...]]:
    """Per binding, the atoms around it that not every binding has as often.

    Other objects of the binding's own unary predicates (its peers) become `_` and equal atoms are counted; other
    objects, such as values, stay.
    """
    counts: list[dict[str, int]] = []
    for profile, binding in zip(profiles, bindings, strict=True):
        own = types.get(binding, frozenset())
        count: dict[str, int] = {}
        for predicate, arguments in profile:
            shown = " ".join(
                argument if argument == "*" or types.get(argument, frozenset()) != own else "_"
                for argument in arguments
            )
            atom = f"({predicate} {shown})"
            count[atom] = count.get(atom, 0) + 1
        counts.append(count)
    atoms = {atom for count in counts for atom in count}
    differing = {atom for atom in atoms if len({count.get(atom, 0) for count in counts}) > 1}
    return [
        tuple(atom if count[atom] == 1 else f"{atom}x{count[atom]}" for atom in sorted(differing & count.keys()))
        for count in counts
    ]
