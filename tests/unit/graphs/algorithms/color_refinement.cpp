#include <gtest/gtest.h>
#include <runir/graphs/algorithms.hpp>
#include <runir/graphs/algorithms/nauty.hpp>
#include <runir/graphs/properties.hpp>
#include <runir/graphs/static_graph.hpp>
#include <string>

namespace runir::tests
{

namespace
{

using GraphBuilder = graphs::StaticGraphBuilder<std::string>;
using Graph = graphs::StaticGraph<std::string>;

Graph make_two_triangles()
{
    auto builder = GraphBuilder();
    auto vertices = std::vector<graphs::VertexIndex>();
    for (int i = 0; i < 6; ++i)
        vertices.push_back(builder.add_vertex(std::string("x")));

    for (auto cycle : { std::array { 0, 1, 2 }, std::array { 3, 4, 5 } })
    {
        builder.add_undirected_edge(vertices[cycle[0]], vertices[cycle[1]]);
        builder.add_undirected_edge(vertices[cycle[1]], vertices[cycle[2]]);
        builder.add_undirected_edge(vertices[cycle[2]], vertices[cycle[0]]);
    }

    return Graph(std::move(builder));
}

Graph make_hexagon()
{
    auto builder = GraphBuilder();
    auto vertices = std::vector<graphs::VertexIndex>();
    for (int i = 0; i < 6; ++i)
        vertices.push_back(builder.add_vertex(std::string("x")));

    for (int i = 0; i < 6; ++i)
        builder.add_undirected_edge(vertices[i], vertices[(i + 1) % 6]);

    return Graph(std::move(builder));
}

void expect_sparse_graph_error(const Graph& graph, const char* message)
{
    EXPECT_THROW(
        try { static_cast<void>(graphs::nauty::SparseGraph(graph)); } catch (const std::runtime_error& error) {
            EXPECT_STREQ(error.what(), message);
            throw;
        },
        std::runtime_error);
}

}  // namespace

TEST(RunirTests, ColorRefinementFailsButWeisfeilerLeman2DistinguishesTwoTrianglesFromHexagon)
{
    const auto two_triangles = make_two_triangles();
    const auto hexagon = make_hexagon();

    const auto cr_two_triangles = graphs::algorithms::color_refinement_certificate(two_triangles);
    const auto cr_hexagon = graphs::algorithms::color_refinement_certificate(hexagon);

    EXPECT_EQ(cr_two_triangles.get_round_summaries(), cr_hexagon.get_round_summaries());
    EXPECT_EQ(cr_two_triangles.get_colors(), cr_hexagon.get_colors());

    const auto wl2_two_triangles = graphs::algorithms::weisfeiler_leman_2_certificate(two_triangles);
    const auto wl2_hexagon = graphs::algorithms::weisfeiler_leman_2_certificate(hexagon);

    EXPECT_NE(wl2_two_triangles.get_round_summaries(), wl2_hexagon.get_round_summaries());
    EXPECT_NE(wl2_two_triangles.get_colors(), wl2_hexagon.get_colors());
}

TEST(RunirTests, GraphPropertyPredicatesClassifyCommonShapes)
{
    {
        auto builder = GraphBuilder();
        const auto lhs = builder.add_vertex(std::string("x"));
        const auto rhs = builder.add_vertex(std::string("x"));
        builder.add_undirected_edge(lhs, rhs);
        const auto graph = Graph(std::move(builder));

        EXPECT_TRUE(graphs::is_loopless(graph));
        EXPECT_FALSE(graphs::is_multi_graph(graph));
        EXPECT_TRUE(graphs::is_symmetric(graph));
        EXPECT_TRUE(graphs::is_simple_undirected(graph));
    }

    {
        auto builder = GraphBuilder();
        const auto vertex = builder.add_vertex(std::string("x"));
        builder.add_directed_edge(vertex, vertex);
        const auto graph = Graph(std::move(builder));

        EXPECT_FALSE(graphs::is_loopless(graph));
        EXPECT_TRUE(graphs::is_symmetric(graph));
        EXPECT_FALSE(graphs::is_simple_undirected(graph));
    }

    {
        auto builder = GraphBuilder();
        const auto source = builder.add_vertex(std::string("x"));
        const auto target = builder.add_vertex(std::string("x"));
        builder.add_directed_edge(source, target);
        builder.add_directed_edge(source, target);
        builder.add_directed_edge(target, source);
        const auto graph = Graph(std::move(builder));

        EXPECT_TRUE(graphs::is_loopless(graph));
        EXPECT_TRUE(graphs::is_multi_graph(graph));
        EXPECT_TRUE(graphs::is_symmetric(graph));
        EXPECT_FALSE(graphs::is_simple_undirected(graph));
    }

    {
        auto builder = GraphBuilder();
        const auto source = builder.add_vertex(std::string("x"));
        const auto target = builder.add_vertex(std::string("x"));
        builder.add_directed_edge(source, target);
        const auto graph = Graph(std::move(builder));

        EXPECT_TRUE(graphs::is_loopless(graph));
        EXPECT_FALSE(graphs::is_multi_graph(graph));
        EXPECT_FALSE(graphs::is_symmetric(graph));
        EXPECT_FALSE(graphs::is_simple_undirected(graph));
    }
}

TEST(RunirTests, NautySparseGraphRejectsUnsupportedGraphShapes)
{
    {
        auto builder = GraphBuilder();
        const auto vertex = builder.add_vertex(std::string("x"));
        builder.add_directed_edge(vertex, vertex);

        expect_sparse_graph_error(Graph(std::move(builder)), "runir::graphs::nauty::SparseGraph: loop edges are not supported.");
    }

    {
        auto builder = GraphBuilder();
        const auto source = builder.add_vertex(std::string("x"));
        const auto target = builder.add_vertex(std::string("x"));
        builder.add_directed_edge(source, target);
        builder.add_directed_edge(source, target);
        builder.add_directed_edge(target, source);

        expect_sparse_graph_error(Graph(std::move(builder)), "runir::graphs::nauty::SparseGraph: parallel edges are not supported.");
    }

    {
        auto builder = GraphBuilder();
        const auto source = builder.add_vertex(std::string("x"));
        const auto target = builder.add_vertex(std::string("x"));
        builder.add_directed_edge(source, target);

        expect_sparse_graph_error(Graph(std::move(builder)), "runir::graphs::nauty::SparseGraph: directed graphs are not supported.");
    }
}

}  // namespace runir::tests
