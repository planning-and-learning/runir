#include "planning_fixtures.hpp"

#include <algorithm>
#include <fmt/format.h>
#include <gtest/gtest.h>
#include <runir/datasets/formatter.hpp>
#include <runir/datasets/object_graph.hpp>
#include <runir/datasets/successors.hpp>
#include <tyr/planning/successors/ground/successor_generator.hpp>
#include <tyr/planning/successors/successors.hpp>
#include <tyr/planning/tasks/ground/task.hpp>
#include <urd/dot.hpp>
#include <vefr/properties.hpp>

namespace runir::tests
{

TEST(ObjectGraphTest, InitialStateObjectGraphIsSimpleAndCanonicalizesVertexColors)
{
    auto context = make_gripper_ground_context();
    auto color_repository = std::make_shared<datasets::ColorRepository>(context->task->get_domain().get_repository());

    const auto initial = runir::datasets::initial_node(*context);
    const tyr::planning::StateBuilderView<tyr::GroundTag>& initial_state = initial.state();  // read as the borrowed view
    const auto graph = datasets::create_object_graph(initial_state, *color_repository);

    ASSERT_GT(graph->num_vertices(), 0);
    ASSERT_GT(graph->num_edges(), 0);
    EXPECT_TRUE(vefr::is_simple_undirected(*graph));

    const auto figure = urd::to_dot(*graph, urd::vertex_data, urd::unlabeled);
    EXPECT_NE(figure.find("digraph G"), std::string::npos);
    EXPECT_NE(figure.find("state:"), std::string::npos);
    EXPECT_NE(figure.find("goal:"), std::string::npos);

    for (const auto vertex : graph->vertices())
    {
        const auto color = graph->vertex_value(vertex);
        const auto& colors = color.context()[color.index()]->values;
        EXPECT_TRUE(std::ranges::is_sorted(colors));
        EXPECT_EQ(std::ranges::adjacent_find(colors), colors.end());
    }

    const auto num_colors = color_repository->size<datasets::Color>();
    const auto second_graph = datasets::create_object_graph(initial_state, *color_repository);
    EXPECT_EQ(second_graph->num_vertices(), graph->num_vertices());
    EXPECT_EQ(color_repository->size<datasets::Color>(), num_colors);
}

}  // namespace runir::tests
