#include <fmt/format.h>
#include <gtest/gtest.h>
#include <ranges>
#include <runir/graphs/bgl/graph_adapters.hpp>
#include <runir/graphs/bidirectional_static_graph.hpp>
#include <runir/graphs/dynamic_graph.hpp>
#include <runir/graphs/formatter.hpp>
#include <runir/graphs/properties.hpp>
#include <runir/graphs/static_graph.hpp>
#include <runir/graphs/static_graph_builder.hpp>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace runir::tests
{

namespace
{

using Builder = graphs::StaticGraphBuilder<std::string, std::string>;
using StaticGraph = graphs::StaticGraph<std::string, std::string>;
using DynamicGraph = graphs::DynamicGraph<std::string, std::string>;
using BidirectionalGraph = graphs::BidirectionalStaticGraph<std::string, std::string>;

template<typename Graph>
void expect_vertex_and_edge_parents_rebound(const Graph& graph, graphs::VertexIndex vertex, graphs::EdgeIndex edge)
{
    EXPECT_EQ(&graph.get_vertex(vertex).get_parent(), &graph);
    EXPECT_EQ(&graph.get_edge(edge).get_parent(), &graph);
}

auto make_static_builder()
{
    auto builder = Builder();
    const auto source = builder.add_vertex(std::string("source"));
    const auto target = builder.add_vertex(std::string("target"));
    builder.add_directed_edge(source, target, std::string("edge"));
    return builder;
}

auto make_dynamic_graph()
{
    auto graph = DynamicGraph();
    const auto source = graph.add_vertex(std::string("source"));
    const auto target = graph.add_vertex(std::string("target"));
    graph.add_directed_edge(source, target, std::string("edge"));
    return graph;
}

template<typename Range>
auto materialize(Range&& range)
{
    auto result = std::vector<std::ranges::range_value_t<Range>>();
    for (auto value : range)
        result.push_back(value);
    return result;
}

template<typename Graph>
void expect_forward_adjacency(const Graph& graph)
{
    const auto source = graphs::VertexIndex { 0 };
    const auto target = graphs::VertexIndex { 1 };
    const auto edge = graphs::EdgeIndex { 0 };

    EXPECT_EQ(graph.get_source(edge), source);
    EXPECT_EQ(graph.get_target(edge), target);
    EXPECT_EQ(graph.get_out_degree(source), 1);
    EXPECT_EQ(graph.get_out_degree(target), 0);
    EXPECT_EQ(materialize(graph.get_out_edge_indices(source)), std::vector<graphs::EdgeIndex>({ edge }));
    EXPECT_TRUE(materialize(graph.get_out_edge_indices(target)).empty());
    EXPECT_EQ(materialize(graph.get_successor_indices(source)), std::vector<graphs::VertexIndex>({ target }));
    EXPECT_TRUE(materialize(graph.get_successor_indices(target)).empty());
    auto out_edges = graph.get_out_edges(source);
    auto successors = graph.get_successors(source);
    EXPECT_EQ(&*out_edges.begin(), &graph.get_edge(edge));
    EXPECT_EQ(&*successors.begin(), &graph.get_vertex(target));

    if constexpr (requires { graph.get_in_degree(source); })
    {
        EXPECT_EQ(graph.get_in_degree(source), 0);
        EXPECT_EQ(graph.get_in_degree(target), 1);
        EXPECT_TRUE(materialize(graph.get_in_edge_indices(source)).empty());
        EXPECT_EQ(materialize(graph.get_in_edge_indices(target)), std::vector<graphs::EdgeIndex>({ edge }));
        EXPECT_TRUE(materialize(graph.get_predecessor_indices(source)).empty());
        EXPECT_EQ(materialize(graph.get_predecessor_indices(target)), std::vector<graphs::VertexIndex>({ source }));
        auto in_edges = graph.get_in_edges(target);
        auto predecessors = graph.get_predecessors(target);
        EXPECT_EQ(&*in_edges.begin(), &graph.get_edge(edge));
        EXPECT_EQ(&*predecessors.begin(), &graph.get_vertex(source));
    }
}

}  // namespace

TEST(GraphContainersTest, DotFormatterEscapesVertexAndEdgeLabels)
{
    auto builder = Builder();
    const auto source = builder.add_vertex(std::string("source\n\"quoted\"\\slash\rcarriage"));
    const auto target = builder.add_vertex(std::string("target"));
    builder.add_directed_edge(source, target, std::string("edge\n\"quoted\"\\slash\rcarriage"));

    const auto graph = StaticGraph(std::move(builder));
    const auto dot = fmt::format("{}", graph);

    EXPECT_NE(dot.find("digraph G"), std::string::npos);
    EXPECT_NE(dot.find("source\\n\\\"quoted\\\"\\\\slashcarriage"), std::string::npos);
    EXPECT_NE(dot.find("edge\\n\\\"quoted\\\"\\\\slashcarriage"), std::string::npos);
}

TEST(GraphContainersTest, PropertyOwningGraphContainersAreMoveOnly)
{
    static_assert(!std::is_copy_constructible_v<Builder>);
    static_assert(!std::is_copy_assignable_v<Builder>);
    static_assert(std::is_move_constructible_v<Builder>);
    static_assert(std::is_move_assignable_v<Builder>);

    static_assert(!std::is_copy_constructible_v<StaticGraph>);
    static_assert(!std::is_copy_assignable_v<StaticGraph>);
    static_assert(std::is_move_constructible_v<StaticGraph>);
    static_assert(std::is_move_assignable_v<StaticGraph>);

    static_assert(!std::is_copy_constructible_v<DynamicGraph>);
    static_assert(!std::is_copy_assignable_v<DynamicGraph>);
    static_assert(std::is_move_constructible_v<DynamicGraph>);
    static_assert(std::is_move_assignable_v<DynamicGraph>);

    static_assert(!std::is_copy_constructible_v<BidirectionalGraph>);
    static_assert(!std::is_copy_assignable_v<BidirectionalGraph>);
    static_assert(std::is_move_constructible_v<BidirectionalGraph>);
    static_assert(std::is_move_assignable_v<BidirectionalGraph>);
}

TEST(GraphContainersTest, TraversalHelpersExposeForwardAndReverseAdjacency)
{
    const auto builder = make_static_builder();
    expect_forward_adjacency(builder);

    const auto static_graph = StaticGraph(make_static_builder());
    expect_forward_adjacency(static_graph);

    const auto dynamic_graph = make_dynamic_graph();
    expect_forward_adjacency(dynamic_graph);

    const auto bidirectional_graph = BidirectionalGraph(make_static_builder());
    const auto& backward_graph = bidirectional_graph.get_backward_graph();
    const auto source = graphs::VertexIndex { 0 };
    const auto target = graphs::VertexIndex { 1 };
    const auto edge = graphs::EdgeIndex { 0 };

    EXPECT_EQ(backward_graph.get_source(edge), target);
    EXPECT_EQ(backward_graph.get_target(edge), source);
    EXPECT_EQ(backward_graph.get_out_degree(source), 0);
    EXPECT_EQ(backward_graph.get_out_degree(target), 1);
    EXPECT_TRUE(materialize(backward_graph.get_out_edge_indices(source)).empty());
    EXPECT_EQ(materialize(backward_graph.get_out_edge_indices(target)), std::vector<graphs::EdgeIndex>({ edge }));
    EXPECT_TRUE(materialize(backward_graph.get_successor_indices(source)).empty());
    EXPECT_EQ(materialize(backward_graph.get_successor_indices(target)), std::vector<graphs::VertexIndex>({ source }));
    auto backward_out_edges = backward_graph.get_out_edges(target);
    auto backward_successors = backward_graph.get_successors(target);
    EXPECT_EQ(&*backward_out_edges.begin(), &backward_graph.get_edge(edge));
    EXPECT_EQ(&*backward_successors.begin(), &backward_graph.get_vertex(source));
}

TEST(GraphContainersTest, StaticGraphBuilderBglAdaptersExposeStableIterators)
{
    const auto builder = make_static_builder();
    const auto source = graphs::VertexIndex { 0 };
    const auto target = graphs::VertexIndex { 1 };
    const auto edge = graphs::EdgeIndex { 0 };

    const auto [vertex_begin, vertex_end] = graphs::vertices(builder);
    EXPECT_EQ(std::vector<graphs::VertexIndex>(vertex_begin, vertex_end), std::vector<graphs::VertexIndex>({ source, target }));

    const auto [edge_begin, edge_end] = graphs::edges(builder);
    EXPECT_EQ(std::vector<graphs::EdgeIndex>(edge_begin, edge_end), std::vector<graphs::EdgeIndex>({ edge }));

    const auto [out_begin, out_end] = graphs::out_edges(source, builder);
    EXPECT_EQ(std::vector<graphs::EdgeIndex>(out_begin, out_end), std::vector<graphs::EdgeIndex>({ edge }));

    EXPECT_FALSE(graphs::is_symmetric(builder));
}

TEST(GraphContainersTest, StaticGraphBuilderMoveRebindsVertexAndEdgeParents)
{
    const auto source = graphs::VertexIndex { 0 };
    const auto edge = graphs::EdgeIndex { 0 };

    auto moved = make_static_builder();
    auto move_constructed = std::move(moved);
    expect_vertex_and_edge_parents_rebound(move_constructed, source, edge);

    auto move_assigned = Builder();
    move_assigned = make_static_builder();
    expect_vertex_and_edge_parents_rebound(move_assigned, source, edge);
}

TEST(GraphContainersTest, StaticGraphMoveRebindsVertexAndEdgeParents)
{
    const auto source = graphs::VertexIndex { 0 };
    const auto edge = graphs::EdgeIndex { 0 };

    auto moved = StaticGraph(make_static_builder());
    auto move_constructed = std::move(moved);
    expect_vertex_and_edge_parents_rebound(move_constructed, source, edge);

    auto move_assigned = StaticGraph();
    move_assigned = StaticGraph(make_static_builder());
    expect_vertex_and_edge_parents_rebound(move_assigned, source, edge);
}

TEST(GraphContainersTest, DynamicGraphMoveRebindsVertexAndEdgeParents)
{
    const auto source = graphs::VertexIndex { 0 };
    const auto edge = graphs::EdgeIndex { 0 };

    auto moved = make_dynamic_graph();
    auto move_constructed = std::move(moved);
    expect_vertex_and_edge_parents_rebound(move_constructed, source, edge);

    auto move_assigned = DynamicGraph();
    move_assigned = make_dynamic_graph();
    expect_vertex_and_edge_parents_rebound(move_assigned, source, edge);
}

TEST(GraphContainersTest, BidirectionalStaticGraphMoveRebindsBackwardView)
{
    auto moved = BidirectionalGraph(make_static_builder());
    auto move_constructed = std::move(moved);
    EXPECT_EQ(&move_constructed.get_backward_graph().get_vertices(), &move_constructed.get_forward_graph().get_vertices());

    auto move_assigned = BidirectionalGraph();
    move_assigned = BidirectionalGraph(make_static_builder());
    EXPECT_EQ(&move_assigned.get_backward_graph().get_vertices(), &move_assigned.get_forward_graph().get_vertices());
}

}  // namespace runir::tests
