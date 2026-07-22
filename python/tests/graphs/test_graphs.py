from typing import TypedDict, cast, override

import pytest

from fixture_utils import load_fixture

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


class ExpectedGraphProperties(TypedDict):
    loopless: bool
    multi_graph: bool
    symmetric: bool
    simple_undirected: bool


class GraphFixture(TypedDict):
    name: str
    num_vertices: int
    edges: list[list[int | bool]]
    expected: ExpectedGraphProperties
    nauty_error: str | None


class CertificatePairFixture(TypedDict):
    left: str
    right: str
    color_refinement_equal: bool
    color_refinement_colors_equal: bool
    weisfeiler_leman_2_equal: bool
    weisfeiler_leman_2_colors_equal: bool


_GRAPH_SUITE = load_fixture("graphs/shapes.json")
GRAPH_FIXTURES = cast(list[GraphFixture], _GRAPH_SUITE["graphs"])
CERTIFICATE_PAIRS = cast(list[CertificatePairFixture], _GRAPH_SUITE["certificate_pairs"])


def _populate_fixture_graph(graph: StaticGraphBuilder | DynamicGraph, case: GraphFixture) -> None:
    vertices = [graph.add_vertex("x") for _ in range(case["num_vertices"])]
    for raw_edge in case["edges"]:
        assert len(raw_edge) == 3
        source, target, undirected = raw_edge
        assert isinstance(source, int) and isinstance(target, int) and isinstance(undirected, bool)
        if undirected:
            graph.add_undirected_edge(vertices[source], vertices[target], "edge")
        else:
            graph.add_directed_edge(vertices[source], vertices[target], "edge")


def _make_fixture_graph(case: GraphFixture) -> StaticGraph:
    builder = StaticGraphBuilder()
    _populate_fixture_graph(builder, case)
    return StaticGraph(builder)


def test_dynamic_graph_exposes_borrowed_iterators_and_properties() -> None:
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


def test_dynamic_graph_removals_update_membership_and_adjacency() -> None:
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


def test_static_graph_preserves_builder_indices_and_properties() -> None:
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


def test_bidirectional_static_graph_exposes_forward_and_backward_graphs() -> None:
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


def test_graph_properties_must_be_hashable() -> None:
    graph = DynamicGraph()

    with pytest.raises(TypeError):
        graph.add_vertex([])


def test_static_graph_boost_algorithms() -> None:
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


def test_backward_static_graph_boost_algorithms() -> None:
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


def test_static_graph_traversal_visitors_dispatch_python_overrides() -> None:
    class Visitor(StaticGraphTraversalVisitor):
        def __init__(self) -> None:
            super().__init__()
            self.events: list[tuple[str, int]] = []

        @override
        def discover_vertex(self, vertex: int) -> None:
            self.events.append(("discover", vertex))

        @override
        def examine_edge(self, edge: int) -> None:
            self.events.append(("edge", edge))

        @override
        def finish_vertex(self, vertex: int) -> None:
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


def test_dynamic_graph_boost_algorithms() -> None:
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


@pytest.mark.parametrize("case", GRAPH_FIXTURES, ids=lambda case: case["name"])
def test_graph_fixture_properties(case: GraphFixture) -> None:
    builder = StaticGraphBuilder()
    dynamic = DynamicGraph()
    _populate_fixture_graph(builder, case)
    _populate_fixture_graph(dynamic, case)
    expected = case["expected"]

    for graph in (builder, StaticGraph(builder), dynamic):
        assert is_loopless(graph) == expected["loopless"]
        assert is_multi_graph(graph) == expected["multi_graph"]
        assert is_symmetric(graph) == expected["symmetric"]
        assert is_simple_undirected(graph) == expected["simple_undirected"]


@pytest.mark.parametrize("case", CERTIFICATE_PAIRS, ids=lambda case: f'{case["left"]}-{case["right"]}')
def test_graph_fixture_certificates(case: CertificatePairFixture) -> None:
    fixtures = {fixture["name"]: fixture for fixture in GRAPH_FIXTURES}
    left = _make_fixture_graph(fixtures[case["left"]])
    right = _make_fixture_graph(fixtures[case["right"]])

    left_cr = color_refinement_certificate(left)
    right_cr = color_refinement_certificate(right)
    left_wl2 = weisfeiler_leman_2_certificate(left)
    right_wl2 = weisfeiler_leman_2_certificate(right)

    assert left_cr.get_round_summaries()
    assert right_cr.get_round_summaries()
    assert left_wl2.get_round_summaries()
    assert right_wl2.get_round_summaries()
    assert len(left_cr.get_colors()) == fixtures[case["left"]]["num_vertices"]
    assert len(right_cr.get_colors()) == fixtures[case["right"]]["num_vertices"]
    assert len(left_wl2.get_colors()) == fixtures[case["left"]]["num_vertices"] ** 2
    assert len(right_wl2.get_colors()) == fixtures[case["right"]]["num_vertices"] ** 2
    assert (left_cr.get_round_summaries() == right_cr.get_round_summaries()) == case["color_refinement_equal"]
    assert (left_cr.get_colors() == right_cr.get_colors()) == case["color_refinement_colors_equal"]
    assert (left_wl2.get_round_summaries() == right_wl2.get_round_summaries()) == case["weisfeiler_leman_2_equal"]
    assert (left_wl2.get_colors() == right_wl2.get_colors()) == case["weisfeiler_leman_2_colors_equal"]
