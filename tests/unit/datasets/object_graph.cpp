#include "planning_fixtures.hpp"

#include <algorithm>
#include <fmt/format.h>
#include <gtest/gtest.h>
#include <runir/datasets/formatter.hpp>
#include <runir/datasets/object_graph.hpp>
#include <runir/graphs/properties.hpp>
#include <tyr/planning/ground/task.hpp>

namespace runir::tests
{

TEST(ObjectGraphTest, InitialStateObjectGraphIsSimpleAndCanonicalizesVertexColors)
{
    auto context = make_gripper_ground_context();
    auto color_repository_factory = datasets::ColorRepositoryFactory {};
    auto color_repository = color_repository_factory.create(context->task->get_domain().get_repository());

    const auto initial_state = context->successor_generator->get_initial_node(*context->state_repository, *context->axiom_evaluator).get_state();
    const auto graph = datasets::create_object_graph(initial_state, *color_repository);

    ASSERT_GT(graph->get_num_vertices(), 0);
    ASSERT_GT(graph->get_num_edges(), 0);
    EXPECT_TRUE(graphs::is_simple_undirected(*graph));

    const auto figure = fmt::format("{}", *graph);
    EXPECT_NE(figure.find("digraph G"), std::string::npos);
    EXPECT_NE(figure.find("state:"), std::string::npos);
    EXPECT_NE(figure.find("goal:"), std::string::npos);

    for (const auto vertex : graph->get_vertex_indices())
    {
        const auto& colors = graph->get_vertex(vertex).get_property().get_data().values;
        EXPECT_TRUE(std::ranges::is_sorted(colors));
        EXPECT_EQ(std::ranges::adjacent_find(colors), colors.end());
    }

    const auto num_colors = color_repository->size<datasets::Color>();
    const auto second_graph = datasets::create_object_graph(initial_state, *color_repository);
    EXPECT_EQ(second_graph->get_num_vertices(), graph->get_num_vertices());
    EXPECT_EQ(color_repository->size<datasets::Color>(), num_colors);
}

}  // namespace runir::tests
