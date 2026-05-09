import pytest

from pyrunir.graphs import (
    DynamicGraph, 
    StaticGraph, 
    StaticGraphBuilder
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
    assert graph.get_out_degree(source) == 1
    assert graph.get_in_degree(target) == 1
    assert graph.get_source(edge) == source
    assert graph.get_target(edge) == target
    assert graph.get_vertex_property(target) == ("target", 1)
    assert graph.get_edge_property(edge) == "edge"

    assert graph.contains_vertex(source)
    assert graph.contains_edge(edge)


def test_static_graph_preserves_builder_indices_and_properties():
    builder = StaticGraphBuilder()

    source = builder.add_vertex("source")
    target = builder.add_vertex("target")
    edge = builder.add_directed_edge(source, target, ("edge", 1))

    assert list(builder.get_vertex_indices()) == [source, target]
    assert list(builder.get_edge_indices()) == [edge]

    graph = StaticGraph(builder)

    assert list(graph.get_vertex_indices()) == [source, target]
    assert list(graph.get_edge_indices()) == [edge]
    assert list(graph.get_out_edge_indices(source)) == [edge]
    assert graph.get_out_degree(source) == 1
    assert graph.get_source(edge) == source
    assert graph.get_target(edge) == target
    assert graph.get_vertex_property(source) == "source"
    assert graph.get_edge_property(edge) == ("edge", 1)


def test_graph_properties_must_be_hashable():
    graph = DynamicGraph()

    with pytest.raises(TypeError):
        graph.add_vertex([])
