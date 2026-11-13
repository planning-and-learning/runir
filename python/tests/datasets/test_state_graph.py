# Copyright (C) 2026 Dominik Drexler
# SPDX-License-Identifier: GPL-3.0-or-later

import gc
import sys
from pathlib import Path
from typing import TypedDict, cast

import pytest
from fixture_utils import load_fixture
from pypddl.formalism import ParserOptions
from pypddl_datasets import data_root
from pytyr.formalism.planning import ActionRow, Parser
from pytyr.planning.ground import PackedState as GroundPackedState
from pytyr.planning.lifted import GroundTaskInstantiationOptions, PackedState as LiftedPackedState, Task
from pyyggdrasil.execution import ExecutionContext

from pyrunir.datasets import (
    ColorRepositoryFactory,
    EquivalenceGraphGenerationOptions,
    EquivalencePolicyMode,
    GroundTaskSearchContext,
    LiftedTaskSearchContext,
    StateGraphEdgeLabel,
    StateGraphGenerationOptions,
    generate_ground_equivalence_graph,
    generate_ground_state_graph,
    generate_lifted_state_graph,
)


class EquivalenceGraphFixture(TypedDict):
    name: str
    domain_file: str
    task_files: list[str]
    equivalence_policy: str
    expected_num_vertices: int
    expected_num_edges: int


_EQUIVALENCE_SUITE = load_fixture("datasets/equivalence_graph.json")
EQUIVALENCE_GRAPH_FIXTURES = cast("list[EquivalenceGraphFixture]", _EQUIVALENCE_SUITE["tests"])
EQUIVALENCE_POLICIES = {
    "identity": EquivalencePolicyMode.IDENTITY,
    "gi": EquivalencePolicyMode.GI,
}


def _make_ground_contexts(case: EquivalenceGraphFixture) -> list[GroundTaskSearchContext]:
    root = Path(data_root())
    parser_options = ParserOptions()
    parser = Parser(root / case["domain_file"], parser_options)
    execution_context = ExecutionContext(1)
    contexts: list[GroundTaskSearchContext] = []
    for task_file in case["task_files"]:
        planning_task = parser.parse_task(root / task_file, parser_options)
        task = Task(planning_task).instantiate_ground_task(execution_context, GroundTaskInstantiationOptions()).task
        contexts.append(GroundTaskSearchContext(task, execution_context))
    return contexts


def test_state_graph_labels_keep_the_graph_alive(ground_gripper_search_context: GroundTaskSearchContext) -> None:
    options = StateGraphGenerationOptions()
    options.max_num_states = 4
    graph = generate_ground_state_graph(ground_gripper_search_context, options)
    edge = graph.edges()[0]

    references = sys.getrefcount(graph)
    vertex = graph.vertex_value(edge.source)
    assert sys.getrefcount(graph) > references
    references = sys.getrefcount(graph)
    edge_label = graph.edge_value(edge.id)
    assert sys.getrefcount(graph) > references
    state = vertex.state
    assert isinstance(state, GroundPackedState)
    action = edge_label.action
    expected = (str(state.unpack()), str(action))

    del vertex, edge_label, graph
    gc.collect()

    assert (str(state.unpack()), str(action)) == expected


def _parse_gripper_task(gripper_data_dir: Path) -> Task:
    parser_options = ParserOptions()
    parser = Parser(gripper_data_dir / "domain.pddl", parser_options)
    return Task(parser.parse_task(gripper_data_dir / "test-1.pddl", parser_options))


def _state_graph_options() -> StateGraphGenerationOptions:
    options = StateGraphGenerationOptions()
    options.max_num_states = 4
    return options


def test_ground_state_graph_labels_own_packed_states_and_action_bindings(gripper_data_dir: Path) -> None:
    lifted_task = _parse_gripper_task(gripper_data_dir)
    execution_context = ExecutionContext(1)
    task = lifted_task.instantiate_ground_task(execution_context, GroundTaskInstantiationOptions()).task
    context = GroundTaskSearchContext(task, execution_context)

    graph = generate_ground_state_graph(context, _state_graph_options())
    edge_label = graph.edge_value(graph.edges()[0].id)

    assert isinstance(edge_label, StateGraphEdgeLabel)
    assert isinstance(edge_label.action, ActionRow)
    states = [graph.vertex_value(vertex).state for vertex in graph.vertices()]
    for state in states:
        assert isinstance(state, GroundPackedState)
        assert state.get_state_repository().layer() == context.state_repository.layer()

    state = states[-1]
    expected = str(state.unpack())
    del edge_label, graph, context, task, lifted_task, states
    gc.collect()

    assert str(state.unpack()) == expected
    assert state.unpack().pack() == state


def test_lifted_state_graph_labels_own_packed_states_and_action_bindings(gripper_data_dir: Path) -> None:
    task = _parse_gripper_task(gripper_data_dir)
    execution_context = ExecutionContext(1)
    context = LiftedTaskSearchContext(task, execution_context)

    graph = generate_lifted_state_graph(context, _state_graph_options())
    edge_label = graph.edge_value(graph.edges()[0].id)

    assert isinstance(edge_label, StateGraphEdgeLabel)
    assert isinstance(edge_label.action, ActionRow)
    states = [graph.vertex_value(vertex).state for vertex in graph.vertices()]
    for state in states:
        assert isinstance(state, LiftedPackedState)
        assert state.get_state_repository().layer() == context.state_repository.layer()

    state = states[-1]
    expected = str(state.unpack())
    del edge_label, graph, context, task, states
    gc.collect()

    assert str(state.unpack()) == expected
    assert state.unpack().pack() == state


def test_generate_ground_equivalence_graph_exposes_owned_result_graphs(
    ground_gripper_search_context: GroundTaskSearchContext,
) -> None:
    options = EquivalenceGraphGenerationOptions()
    options.policy_mode = EquivalencePolicyMode.IDENTITY
    options.state_graph_options.max_num_states = 4

    colors = ColorRepositoryFactory().create(ground_gripper_search_context.task)
    result = generate_ground_equivalence_graph([ground_gripper_search_context], options, colors)

    forward_graph = result.graph
    assert forward_graph.num_vertices > 0
    assert forward_graph.num_edges > 0
    assert result.get_num_state_graph_results() == 1
    state_graph_result = result.get_state_graph_result(0)
    state_graph = state_graph_result.graph
    assert state_graph.num_vertices > 0
    for vertex in state_graph.vertices():
        assert (
            state_graph.vertex_value(vertex).state.get_state_repository().layer()
            == ground_gripper_search_context.state_repository.layer()
        )
    with pytest.raises(IndexError, match="State graph result index is out of range"):
        result.get_state_graph_result(1)

    source = next(vertex for vertex in forward_graph.vertices() if forward_graph.out_degree(vertex) > 0)
    assert list(forward_graph.neighbours(source))


def test_object_graph_color_repository_can_be_reused(ground_gripper_search_context: GroundTaskSearchContext) -> None:
    options = EquivalenceGraphGenerationOptions()
    options.policy_mode = EquivalencePolicyMode.GI
    colors = ColorRepositoryFactory().create(ground_gripper_search_context.task)

    generate_ground_equivalence_graph([ground_gripper_search_context], options, colors)
    num_colors = colors.num_colors
    generate_ground_equivalence_graph([ground_gripper_search_context], options, colors)

    assert num_colors > 0
    assert colors.num_colors == num_colors


@pytest.mark.parametrize("case", EQUIVALENCE_GRAPH_FIXTURES, ids=[case["name"] for case in EQUIVALENCE_GRAPH_FIXTURES])
def test_equivalence_graph_fixture_counts(case: EquivalenceGraphFixture) -> None:
    options = EquivalenceGraphGenerationOptions()
    options.policy_mode = EQUIVALENCE_POLICIES[case["equivalence_policy"]]

    contexts = _make_ground_contexts(case)
    result = generate_ground_equivalence_graph(contexts, options, ColorRepositoryFactory().create(contexts[0].task))
    graph = result.graph

    assert graph.num_vertices == case["expected_num_vertices"]
    assert graph.num_edges == case["expected_num_edges"]
    assert result.get_num_state_graph_results() == len(case["task_files"])
