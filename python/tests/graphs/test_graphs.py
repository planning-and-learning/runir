import pytest

from pyrunir.graphs import (
    BidirectionalStaticGraph,
    DynamicGraph,
    StaticGraph,
    StaticGraphBuilder,
    StaticGraphTraversalVisitor,
    breadth_first_search,
    breadth_first_visit,
    color_refinement_certificate,
    depth_first_search,
    depth_first_visit,
    dijkstra_shortest_paths,
    floyd_warshall_all_pairs_shortest_paths,
    is_loopless,
    is_multi_graph,
    is_simple_undirected,
    is_symmetric,
    strong_components,
    topological_sort,
    weisfeiler_leman_2_certificate,
)


def test_dynamic_graph_exposes_borrowed_iterators_and_properties():
    graph = DynamicGraph()

    source = graph.add_vertex("source")
    target = graph.add_vertex(("target", 1))
    edge = graph.add_directed_edge(source, target, "edge")

    assert list(graph.get_vertex_indices()) == [source, target]
    assert list(graph.get_edge_indices()) == [edge]
    assert list(graph.get_out_edge_indices(source)) == [edge]
    assert list(graph.get_in_edge_indices(target)) == [edge]
    assert list(graph.get_successor_indices(source)) == [target]
    assert list(graph.get_predecessor_indices(target)) == [source]
    assert graph.get_out_degree(source) == 1
    assert graph.get_in_degree(target) == 1
    assert graph.get_source(edge) == source
    assert graph.get_target(edge) == target
    assert graph.get_vertex_property(target) == ("target", 1)
    assert graph.get_edge_property(edge) == "edge"

    assert graph.contains_vertex(source)
    assert graph.contains_edge(edge)


def test_dynamic_graph_removals_update_membership_and_adjacency():
    graph = DynamicGraph()

    source = graph.add_vertex("source")
    middle = graph.add_vertex("middle")
    target = graph.add_vertex("target")
    first_edge = graph.add_directed_edge(source, middle, "source-middle")
    second_edge = graph.add_directed_edge(middle, target, "middle-target")

    graph.remove_edge(first_edge)

    assert not graph.contains_edge(first_edge)
    assert graph.contains_edge(second_edge)
    assert graph.get_out_degree(source) == 0
    assert graph.get_in_degree(middle) == 0
    assert list(graph.get_successor_indices(source)) == []
    assert list(graph.get_predecessor_indices(middle)) == []

    graph.remove_vertex(middle)

    assert not graph.contains_vertex(middle)
    assert not graph.contains_edge(second_edge)
    assert graph.contains_vertex(source)
    assert graph.contains_vertex(target)
    assert graph.get_num_vertices() == 2
    assert graph.get_num_edges() == 0

    graph.clear()

    assert graph.get_num_vertices() == 0
    assert graph.get_num_edges() == 0
    assert not graph.contains_vertex(source)


def test_static_graph_preserves_builder_indices_and_properties():
    builder = StaticGraphBuilder()

    source = builder.add_vertex("source")
    target = builder.add_vertex("target")
    edge = builder.add_directed_edge(source, target, ("edge", 1))

    assert list(builder.get_vertex_indices()) == [source, target]
    assert list(builder.get_edge_indices()) == [edge]
    assert builder.get_out_edge_indices(source) == [edge]
    assert builder.get_in_edge_indices(target) == [edge]
    assert builder.get_successor_indices(source) == [target]
    assert builder.get_predecessor_indices(target) == [source]
    assert builder.get_out_degree(source) == 1
    assert builder.get_in_degree(target) == 1

    copied_builder = StaticGraphBuilder()
    copied_builder.add_vertex("copied")
    copied_builder.clear()
    assert copied_builder.get_num_vertices() == 0
    assert copied_builder.get_num_edges() == 0

    graph = StaticGraph(builder)

    assert list(graph.get_vertex_indices()) == [source, target]
    assert list(graph.get_edge_indices()) == [edge]
    assert list(graph.get_out_edge_indices(source)) == [edge]
    assert list(graph.get_successor_indices(source)) == [target]
    assert graph.get_out_degree(source) == 1
    assert graph.get_source(edge) == source
    assert graph.get_target(edge) == target
    assert graph.get_vertex_property(source) == "source"
    assert graph.get_edge_property(edge) == ("edge", 1)


def test_bidirectional_static_graph_exposes_forward_and_backward_graphs():
    builder = StaticGraphBuilder()

    a = builder.add_vertex("a")
    b = builder.add_vertex("b")
    c = builder.add_vertex("c")
    builder.add_directed_edge(a, b, "a-b")
    builder.add_directed_edge(c, b, "c-b")
    builder.add_directed_edge(b, a, "b-a")

    graph = BidirectionalStaticGraph(builder)

    forward_targets = [
        graph.get_forward_graph().get_target(edge)
        for edge in graph.get_forward_graph().get_out_edge_indices(b)
    ]
    backward_targets = [
        graph.get_backward_graph().get_target(edge)
        for edge in graph.get_backward_graph().get_out_edge_indices(b)
    ]

    assert forward_targets == [a]
    assert list(graph.get_forward_graph().get_successor_indices(b)) == [a]
    assert backward_targets == [a, c]
    assert list(graph.get_backward_graph().get_successor_indices(b)) == [a, c]


def test_graph_properties_must_be_hashable():
    graph = DynamicGraph()

    with pytest.raises(TypeError):
        graph.add_vertex([])


def test_static_graph_boost_algorithms():
    builder = StaticGraphBuilder()

    a = builder.add_vertex("a")
    b = builder.add_vertex("b")
    c = builder.add_vertex("c")
    builder.add_directed_edge(a, b, "a-b")
    builder.add_directed_edge(b, c, "b-c")
    builder.add_directed_edge(a, c, "a-c")

    graph = StaticGraph(builder)

    num_components, components = strong_components(graph)
    assert num_components == 3
    assert len(components) == 3

    predecessors, distances = breadth_first_search(graph, [a])
    assert predecessors[b] == a
    assert distances[c] == 1

    dfs_predecessors = depth_first_search(graph, [a])
    assert len(dfs_predecessors) == 3

    topological_order = topological_sort(graph)
    assert set(topological_order) == {a, b, c}

    weights = [1.0] * graph.get_num_edges()
    for edge in graph.get_edge_indices():
        if graph.get_source(edge) == a and graph.get_target(edge) == c:
            weights[edge] = 5.0

    _, weighted_distances = dijkstra_shortest_paths(graph, weights, [a])
    assert weighted_distances[c] == 2.0

    all_pairs_distances = floyd_warshall_all_pairs_shortest_paths(graph, weights)
    assert all_pairs_distances[a][c] == 2.0


def test_backward_static_graph_boost_algorithms():
    builder = StaticGraphBuilder()

    a = builder.add_vertex("a")
    b = builder.add_vertex("b")
    c = builder.add_vertex("c")
    builder.add_directed_edge(a, b, "a-b")
    builder.add_directed_edge(c, b, "c-b")

    graph = BidirectionalStaticGraph(builder)

    _, distances = breadth_first_search(graph.get_backward_graph(), [b])
    assert distances[a] == 1
    assert distances[c] == 1


def test_static_graph_traversal_visitors_dispatch_python_overrides():
    class Visitor(StaticGraphTraversalVisitor):
        def __init__(self):
            super().__init__()
            self.events = []

        def discover_vertex(self, vertex):
            self.events.append(("discover", vertex))

        def examine_edge(self, edge):
            self.events.append(("edge", edge))

        def finish_vertex(self, vertex):
            self.events.append(("finish", vertex))

    builder = StaticGraphBuilder()
    source = builder.add_vertex("source")
    target = builder.add_vertex("target")
    edge = builder.add_directed_edge(source, target, "edge")
    graph = StaticGraph(builder)

    bfs_visitor = Visitor()
    breadth_first_visit(graph, [source], bfs_visitor)
    assert bfs_visitor.events == [
        ("discover", source),
        ("edge", edge),
        ("discover", target),
        ("finish", source),
        ("finish", target),
    ]

    dfs_visitor = Visitor()
    depth_first_visit(graph, [source], dfs_visitor)
    assert dfs_visitor.events == [
        ("discover", source),
        ("edge", edge),
        ("discover", target),
        ("finish", target),
        ("finish", source),
    ]


def test_dynamic_graph_boost_algorithms():
    graph = DynamicGraph()

    a = graph.add_vertex("a")
    b = graph.add_vertex("b")
    c = graph.add_vertex("c")
    edge_ab = graph.add_directed_edge(a, b, "a-b")
    edge_bc = graph.add_directed_edge(b, c, "b-c")

    predecessors, distances = breadth_first_search(graph, [a])
    assert predecessors[b] == a
    assert distances[c] == 2

    weights = {
        edge_ab: 2.0,
        edge_bc: 3.0,
    }
    _, weighted_distances = dijkstra_shortest_paths(graph, weights, [a])
    assert weighted_distances[c] == 5.0


def test_graph_property_helpers_classify_common_shapes():
    builder = StaticGraphBuilder()
    lhs = builder.add_vertex("x")
    rhs = builder.add_vertex("x")
    builder.add_undirected_edge(lhs, rhs)

    assert is_loopless(builder)
    assert not is_multi_graph(builder)
    assert is_symmetric(builder)
    assert is_simple_undirected(builder)

    simple_graph = StaticGraph(builder)
    assert is_simple_undirected(simple_graph)

    directed = DynamicGraph()
    source = directed.add_vertex("x")
    target = directed.add_vertex("x")
    directed.add_directed_edge(source, target, "edge")

    assert is_loopless(directed)
    assert not is_multi_graph(directed)
    assert not is_symmetric(directed)
    assert not is_simple_undirected(directed)

    directed.add_directed_edge(source, target, "parallel")
    directed.add_directed_edge(target, source, "reverse")

    assert is_multi_graph(directed)
    assert is_symmetric(directed)
    assert not is_simple_undirected(directed)

    loop = DynamicGraph()
    vertex = loop.add_vertex("x")
    loop.add_directed_edge(vertex, vertex, "loop")

    assert not is_loopless(loop)
    assert is_symmetric(loop)
    assert not is_simple_undirected(loop)


def test_graph_certificates():
    builder = StaticGraphBuilder()

    a = builder.add_vertex("x")
    b = builder.add_vertex("x")
    c = builder.add_vertex("x")
    builder.add_directed_edge(a, b, "a-b")
    builder.add_directed_edge(b, c, "b-c")

    graph = StaticGraph(builder)

    color_refinement = color_refinement_certificate(graph)
    weisfeiler_leman = weisfeiler_leman_2_certificate(graph)

    assert len(color_refinement.get_colors()) == graph.get_num_vertices()
    assert len(color_refinement.get_round_summaries()) > 0
    assert len(weisfeiler_leman.get_colors()) == graph.get_num_vertices() ** 2
    assert len(weisfeiler_leman.get_round_summaries()) > 0
