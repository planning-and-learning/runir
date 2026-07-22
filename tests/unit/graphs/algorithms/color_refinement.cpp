#include "fixtures.hpp"

#include <boost/json.hpp>
#include <gtest/gtest.h>
#include <runir/graphs/algorithms.hpp>
#include <runir/graphs/algorithms/nauty.hpp>
#include <runir/graphs/properties.hpp>
#include <runir/graphs/static_graph.hpp>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <yggdrasil/serialization/json.hpp>

namespace runir::tests
{

namespace
{

using GraphBuilder = graphs::StaticGraphBuilder<std::string>;
using Graph = graphs::StaticGraph<std::string>;

auto graph_cases() -> const boost::json::array&
{
    static const auto suite = load_fixture_json("graphs/shapes.json");
    static const auto& cases = ygg::common::as_array(ygg::common::require_member(ygg::common::as_object(suite, "suite"), "graphs", "suite"), "suite.graphs");
    return cases;
}

auto find_graph_case(std::string_view name) -> const boost::json::object&
{
    for (const auto& value : graph_cases())
    {
        const auto& object = ygg::common::as_object(value, "graph");
        if (ygg::common::as_string(object, "name", "graph") == name)
            return object;
    }
    throw std::runtime_error("Unknown graph fixture: " + std::string(name));
}

auto make_graph(const boost::json::object& object) -> Graph
{
    auto builder = GraphBuilder();
    auto vertices = std::vector<graphs::VertexIndex> {};
    const auto num_vertices = ygg::common::as_size(object, "num_vertices", "graph");
    vertices.reserve(num_vertices);
    for (std::size_t i = 0; i < num_vertices; ++i)
        vertices.push_back(builder.add_vertex(std::string("x")));

    for (const auto& edge_value : ygg::common::as_array(ygg::common::require_member(object, "edges", "graph"), "graph.edges"))
    {
        const auto& edge = ygg::common::as_array(edge_value, "graph.edge");
        if (edge.size() != 3)
            throw std::runtime_error("graph.edge must contain source, target, and undirected.");
        const auto source = ygg::common::as_size(edge[0], "graph.edge.source");
        const auto target = ygg::common::as_size(edge[1], "graph.edge.target");
        if (ygg::common::as_bool(edge[2], "graph.edge.undirected"))
            builder.add_undirected_edge(vertices.at(source), vertices.at(target));
        else
            builder.add_directed_edge(vertices.at(source), vertices.at(target));
    }
    return Graph(std::move(builder));
}

}  // namespace

TEST(RunirTests, GraphFixturePropertiesMatch)
{
    for (const auto& value : graph_cases())
    {
        const auto& object = ygg::common::as_object(value, "graph");
        const auto& expected = ygg::common::as_object(ygg::common::require_member(object, "expected", "graph"), "graph.expected");
        const auto graph = make_graph(object);
        EXPECT_EQ(graphs::is_loopless(graph), ygg::common::as_bool(expected, "loopless", "graph.expected"));
        EXPECT_EQ(graphs::is_multi_graph(graph), ygg::common::as_bool(expected, "multi_graph", "graph.expected"));
        EXPECT_EQ(graphs::is_symmetric(graph), ygg::common::as_bool(expected, "symmetric", "graph.expected"));
        EXPECT_EQ(graphs::is_simple_undirected(graph), ygg::common::as_bool(expected, "simple_undirected", "graph.expected"));
    }
}

TEST(RunirTests, GraphFixtureCertificatesMatch)
{
    const auto suite = load_fixture_json("graphs/shapes.json");
    const auto& object = ygg::common::as_object(suite, "suite");
    for (const auto& value : ygg::common::as_array(ygg::common::require_member(object, "certificate_pairs", "suite"), "suite.certificate_pairs"))
    {
        const auto& pair = ygg::common::as_object(value, "certificate_pair");
        const auto left = make_graph(find_graph_case(ygg::common::as_string(pair, "left", "certificate_pair")));
        const auto right = make_graph(find_graph_case(ygg::common::as_string(pair, "right", "certificate_pair")));
        const auto left_cr = graphs::algorithms::color_refinement_certificate(left);
        const auto right_cr = graphs::algorithms::color_refinement_certificate(right);
        const auto left_wl2 = graphs::algorithms::weisfeiler_leman_2_certificate(left);
        const auto right_wl2 = graphs::algorithms::weisfeiler_leman_2_certificate(right);

        EXPECT_EQ(left_cr.get_round_summaries() == right_cr.get_round_summaries(), ygg::common::as_bool(pair, "color_refinement_equal", "certificate_pair"));
        EXPECT_EQ(left_cr.get_colors() == right_cr.get_colors(), ygg::common::as_bool(pair, "color_refinement_colors_equal", "certificate_pair"));
        EXPECT_EQ(left_wl2.get_round_summaries() == right_wl2.get_round_summaries(),
                  ygg::common::as_bool(pair, "weisfeiler_leman_2_equal", "certificate_pair"));
        EXPECT_EQ(left_wl2.get_colors() == right_wl2.get_colors(), ygg::common::as_bool(pair, "weisfeiler_leman_2_colors_equal", "certificate_pair"));
    }
}

TEST(RunirTests, NautySparseGraphRejectsUnsupportedFixtureShapes)
{
    for (const auto& value : graph_cases())
    {
        const auto& object = ygg::common::as_object(value, "graph");
        const auto* error = object.if_contains("nauty_error");
        if (!error || error->is_null())
            continue;
        const auto graph = make_graph(object);
        EXPECT_THROW(
            try { static_cast<void>(graphs::nauty::SparseGraph(graph)); } catch (const std::runtime_error& exception) {
                EXPECT_EQ(exception.what(), ygg::common::as_string(*error, "graph.nauty_error"));
                throw;
            },
            std::runtime_error);
    }
}

}  // namespace runir::tests
