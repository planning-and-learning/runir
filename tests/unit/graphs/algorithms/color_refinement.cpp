#include <gtest/gtest.h>
#include <runir/graphs/algorithms.hpp>
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

}  // namespace runir::tests
