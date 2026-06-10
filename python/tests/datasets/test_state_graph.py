import pytest

from pyrunir.datasets import (
    AnnotatedEquivalenceVertexLabel,
    DynamicAnnotatedEquivalenceGraph,
    DynamicEquivalenceGraph,
    EquivalenceEdgeLabel,
    EquivalenceGraphBuilder,
    EquivalenceGraphGenerationOptions,
    EquivalencePolicyMode,
    EquivalenceVertexLabel,
    GroundDynamicAnnotatedStateGraph,
    GroundDynamicStateGraph,
    GroundStateGraphBuilder,
    GroundStaticStateGraph,
    StaticEquivalenceGraph,
    StateGraphCostMode,
    StateGraphGenerationOptions,
    annotate_ground_state_graph,
    generate_ground_equivalence_graph,
    generate_ground_state_graph,
)


def test_ground_state_graph_builder_can_copy_generated_graph_labels(ground_gripper_search_context):
    context = ground_gripper_search_context
    options = StateGraphGenerationOptions()
    options.max_num_states = 4

    graph = generate_ground_state_graph(context, options)
    forward_graph = graph.get_forward_graph()
    assert forward_graph.get_num_vertices() > 0
    assert forward_graph.get_num_edges() > 0

    builder = GroundStateGraphBuilder()
    for vertex in forward_graph.get_vertex_indices():
        builder.add_vertex(forward_graph.get_vertex_property(vertex))
    for edge in forward_graph.get_edge_indices():
        builder.add_directed_edge(
            forward_graph.get_source(edge),
            forward_graph.get_target(edge),
            forward_graph.get_edge_property(edge),
        )

    assert builder.get_num_vertices() == forward_graph.get_num_vertices()
    assert builder.get_num_edges() == forward_graph.get_num_edges()

    first_vertex = next(iter(builder.get_vertex_indices()))
    assert builder.get_out_edge_indices(first_vertex) == list(forward_graph.get_out_edge_indices(first_vertex))
    assert builder.get_successor_indices(first_vertex) == list(forward_graph.get_successor_indices(first_vertex))

    copied_graph = GroundStaticStateGraph(builder)
    assert list(copied_graph.get_successor_indices(first_vertex)) == builder.get_successor_indices(first_vertex)


def test_equivalence_graph_builder_can_construct_static_graph_from_labels():
    source_label = EquivalenceVertexLabel()
    source_label.state_graph_index = 0
    source_label.state_vertex_index = 1
    target_label = EquivalenceVertexLabel()
    target_label.state_graph_index = 0
    target_label.state_vertex_index = 2
    edge_label = EquivalenceEdgeLabel()
    edge_label.state_graph_index = 0
    edge_label.state_edge_index = 3

    builder = EquivalenceGraphBuilder()
    source = builder.add_vertex(source_label)
    target = builder.add_vertex(target_label)
    edge = builder.add_directed_edge(source, target, edge_label)

    assert builder.get_out_edge_indices(source) == [edge]
    assert builder.get_in_edge_indices(target) == [edge]
    assert builder.get_successor_indices(source) == [target]
    assert builder.get_predecessor_indices(target) == [source]

    graph = StaticEquivalenceGraph(builder)
    assert list(graph.get_successor_indices(source)) == [target]
    assert graph.get_edge_property(edge).state_edge_index == edge_label.state_edge_index


def test_labeled_dataset_graphs_expose_undirected_edge_insertion():
    source_label = EquivalenceVertexLabel()
    source_label.state_graph_index = 0
    source_label.state_vertex_index = 1
    target_label = EquivalenceVertexLabel()
    target_label.state_graph_index = 0
    target_label.state_vertex_index = 2
    edge_label = EquivalenceEdgeLabel()
    edge_label.state_graph_index = 0
    edge_label.state_edge_index = 3

    builder = EquivalenceGraphBuilder()
    source = builder.add_vertex(source_label)
    target = builder.add_vertex(target_label)
    forward_edge, reverse_edge = builder.add_undirected_edge(source, target, edge_label)

    assert builder.get_source(forward_edge) == source
    assert builder.get_target(forward_edge) == target
    assert builder.get_source(reverse_edge) == target
    assert builder.get_target(reverse_edge) == source
    assert builder.get_successor_indices(source) == [target]
    assert builder.get_predecessor_indices(source) == [target]

    graph = DynamicEquivalenceGraph()
    dynamic_source = graph.add_vertex(source_label)
    dynamic_target = graph.add_vertex(target_label)
    dynamic_forward, dynamic_reverse = graph.add_undirected_edge(dynamic_source, dynamic_target, edge_label)

    assert graph.contains_edge(dynamic_forward)
    assert graph.contains_edge(dynamic_reverse)
    assert graph.get_successor_indices(dynamic_source) == [dynamic_target]
    assert graph.get_predecessor_indices(dynamic_source) == [dynamic_target]
    assert graph.get_edge_property(dynamic_reverse).state_edge_index == edge_label.state_edge_index


def test_generate_ground_equivalence_graph_exposes_owned_result_graphs(ground_gripper_search_context):
    context = ground_gripper_search_context
    options = EquivalenceGraphGenerationOptions()
    options.policy_mode = EquivalencePolicyMode.IDENTITY
    options.state_graph_options.max_num_states = 4

    result = generate_ground_equivalence_graph([context], options)

    forward_graph = result.graph.get_forward_graph()
    assert forward_graph.get_num_vertices() > 0
    assert forward_graph.get_num_edges() > 0
    assert result.get_num_state_graph_results() == 1
    state_graph_result = result.get_state_graph_result(0)
    assert state_graph_result.graph.get_forward_graph().get_num_vertices() > 0
    with pytest.raises(IndexError, match="State graph result index is out of range"):
        result.get_state_graph_result(1)

    source = next(vertex for vertex in forward_graph.get_vertex_indices() if forward_graph.get_out_degree(vertex) > 0)
    assert list(forward_graph.get_successor_indices(source))


def test_dataset_dynamic_graph_bindings_support_mutation_and_membership():
    source_label = EquivalenceVertexLabel()
    source_label.state_graph_index = 0
    source_label.state_vertex_index = 1
    target_label = EquivalenceVertexLabel()
    target_label.state_graph_index = 0
    target_label.state_vertex_index = 2
    edge_label = EquivalenceEdgeLabel()
    edge_label.state_graph_index = 0
    edge_label.state_edge_index = 3

    graph = DynamicEquivalenceGraph()
    source = graph.add_vertex(source_label)
    target = graph.add_vertex(target_label)
    edge = graph.add_directed_edge(source, target, edge_label)

    assert graph.contains_vertex(source)
    assert graph.contains_edge(edge)
    assert set(graph.get_vertex_indices()) == {source, target}
    assert list(graph.get_edge_indices()) == [edge]
    assert list(graph.get_out_edge_indices(source)) == [edge]
    assert graph.get_successor_indices(source) == [target]
    assert graph.get_predecessor_indices(target) == [source]
    assert graph.get_edge_property(edge).state_edge_index == edge_label.state_edge_index

    graph.remove_edge(edge)
    assert not graph.contains_edge(edge)
    assert graph.get_successor_indices(source) == []

    graph.remove_vertex(target)
    assert not graph.contains_vertex(target)

    graph.clear()
    assert graph.get_num_vertices() == 0
    assert graph.get_num_edges() == 0
    assert not graph.contains_vertex(source)


def test_ground_state_dynamic_graph_binding_can_copy_generated_labels(ground_gripper_search_context):
    context = ground_gripper_search_context
    options = StateGraphGenerationOptions()
    options.max_num_states = 4

    generated = generate_ground_state_graph(context, options).get_forward_graph()
    source = next(vertex for vertex in generated.get_vertex_indices() if generated.get_out_degree(vertex) > 0)
    edge = next(iter(generated.get_out_edge_indices(source)))
    target = generated.get_target(edge)

    graph = GroundDynamicStateGraph()
    copied_source = graph.add_vertex(generated.get_vertex_property(source))
    copied_target = graph.add_vertex(generated.get_vertex_property(target))
    copied_edge = graph.add_directed_edge(copied_source, copied_target, generated.get_edge_property(edge))

    assert graph.contains_vertex(copied_source)
    assert graph.contains_edge(copied_edge)
    assert graph.get_successor_indices(copied_source) == [copied_target]
    assert graph.get_edge_property(copied_edge).cost == generated.get_edge_property(edge).cost


def test_annotated_equivalence_dynamic_graph_binding_uses_annotated_labels():
    label = AnnotatedEquivalenceVertexLabel()
    label.state_graph_index = 1
    label.state_vertex_index = 2
    label.goal_distance = 3.0
    label.is_alive = True
    edge_label = EquivalenceEdgeLabel()
    edge_label.state_graph_index = 1
    edge_label.state_edge_index = 3

    graph = DynamicAnnotatedEquivalenceGraph()
    source = graph.add_vertex(label)
    target = graph.add_vertex(label)
    edge = graph.add_directed_edge(source, target, edge_label)

    assert graph.get_num_vertices() == 2
    assert graph.get_num_edges() == 1
    assert graph.get_successor_indices(source) == [target]
    assert graph.get_vertex_property(source).goal_distance == label.goal_distance
    assert graph.get_edge_property(edge).state_graph_index == edge_label.state_graph_index


def test_ground_annotated_state_dynamic_graph_binding_can_copy_annotated_labels(ground_gripper_search_context):
    context = ground_gripper_search_context
    options = StateGraphGenerationOptions()
    options.max_num_states = 4

    generated = generate_ground_state_graph(context, options)
    annotated = annotate_ground_state_graph(context, generated, StateGraphCostMode.UNIT_COST).get_forward_graph()
    source = next(vertex for vertex in annotated.get_vertex_indices() if annotated.get_out_degree(vertex) > 0)
    edge = next(iter(annotated.get_out_edge_indices(source)))
    target = annotated.get_target(edge)

    graph = GroundDynamicAnnotatedStateGraph()
    copied_source = graph.add_vertex(annotated.get_vertex_property(source))
    copied_target = graph.add_vertex(annotated.get_vertex_property(target))
    copied_edge = graph.add_directed_edge(copied_source, copied_target, annotated.get_edge_property(edge))

    assert graph.contains_vertex(copied_source)
    assert graph.contains_edge(copied_edge)
    assert graph.get_successor_indices(copied_source) == [copied_target]
    assert graph.get_vertex_property(copied_source).is_initial == annotated.get_vertex_property(source).is_initial

    graph.remove_vertex(copied_target)
    assert not graph.contains_vertex(copied_target)
    assert not graph.contains_edge(copied_edge)

