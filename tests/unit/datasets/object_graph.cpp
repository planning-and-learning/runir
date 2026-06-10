#include "fixtures.hpp"

#include <algorithm>
#include <fmt/format.h>
#include <gtest/gtest.h>
#include <runir/datasets/formatter.hpp>
#include <runir/datasets/object_graph.hpp>
#include <runir/graphs/properties.hpp>
#include <yggdrasil/semantics/comparators.hpp>
#include <yggdrasil/semantics/equal_to.hpp>

namespace runir::tests
{

TEST(ObjectGraphTest, InitialStateObjectGraphIsSimpleAndCanonicalizesVertexLabels)
{
    auto context = make_gripper_ground_context();

    const auto initial_state = context->successor_generator->get_initial_node().get_state();
    const auto graph = datasets::create_object_graph(initial_state);

    ASSERT_GT(graph->get_num_vertices(), 0);
    ASSERT_GT(graph->get_num_edges(), 0);
    EXPECT_TRUE(graphs::is_simple_undirected(*graph));

    const auto figure = fmt::format("{}", *graph);
    EXPECT_NE(figure.find("digraph G"), std::string::npos);
    EXPECT_NE(figure.find("state:"), std::string::npos);
    EXPECT_NE(figure.find("goal:"), std::string::npos);

    for (const auto vertex : graph->get_vertex_indices())
    {
        const auto& labels = graph->get_vertex(vertex).get_property().labels;
        EXPECT_TRUE(std::ranges::is_sorted(labels, ygg::Less<datasets::ObjectGraphVertexLabelEntry> {}));
        EXPECT_EQ(std::ranges::adjacent_find(labels, ygg::EqualTo<datasets::ObjectGraphVertexLabelEntry> {}), labels.end());
    }
}

}  // namespace runir::tests
