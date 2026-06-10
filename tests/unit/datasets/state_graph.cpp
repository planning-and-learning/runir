#include "fixtures.hpp"

#include <gtest/gtest.h>
#include <limits>
#include <runir/datasets/state_graph.hpp>
#include <stdexcept>

namespace runir::tests
{

TEST(StateGraphTest, RejectsUnsupportedCostMode)
{
    namespace p = tyr::planning;

    auto context = make_gripper_ground_context();

    auto builder = datasets::StateGraphBuilder<p::GroundTag> {};
    const auto initial_state = context->successor_generator->get_initial_node().get_state();
    [[maybe_unused]] const auto initial_vertex = builder.add_vertex(datasets::StateGraphVertexLabel<p::GroundTag> { initial_state });
    auto graph = datasets::StateGraph<p::GroundTag>(std::move(builder));

    EXPECT_THROW(
        try {
            static_cast<void>(datasets::annotate_state_graph(*context, graph, static_cast<datasets::StateGraphCostMode>(999)));
        } catch (const std::runtime_error& error) {
            EXPECT_STREQ(error.what(), "Unsupported state graph cost mode: 999.");
            throw;
        },
        std::runtime_error);
}

TEST(StateGraphTest, AnnotatesGeneratedGraphWithReachabilityAndGoalMetadata)
{
    auto context = make_gripper_ground_context();
    const auto graph = datasets::generate_state_graph(*context);
    const auto annotated = datasets::annotate_state_graph(*context, *graph, datasets::StateGraphCostMode::UNIT_COST);
    const auto& forward_graph = annotated->get_forward_graph();

    ASSERT_GT(forward_graph.get_num_vertices(), 0);

    auto num_initial = std::size_t { 0 };
    auto num_goal = std::size_t { 0 };
    auto num_alive = std::size_t { 0 };
    for (auto vertex : forward_graph.get_vertex_indices())
    {
        const auto& label = forward_graph.get_vertex(vertex).get_property();
        if (label.is_initial)
        {
            ++num_initial;
            EXPECT_TRUE(label.is_alive);
            EXPECT_FALSE(label.is_unsolvable);
        }
        if (label.is_goal)
        {
            ++num_goal;
            EXPECT_EQ(label.goal_distance, 0);
            EXPECT_TRUE(label.is_alive);
            EXPECT_FALSE(label.is_unsolvable);
        }
        if (label.is_alive)
        {
            ++num_alive;
            EXPECT_FALSE(label.is_unsolvable);
            EXPECT_LT(label.goal_distance, std::numeric_limits<ygg::float_t>::infinity());
        }
        else
        {
            EXPECT_TRUE(label.is_unsolvable);
            EXPECT_EQ(label.goal_distance, std::numeric_limits<ygg::float_t>::infinity());
        }
    }

    EXPECT_EQ(num_initial, 1);
    EXPECT_GT(num_goal, 0);
    EXPECT_GT(num_alive, 0);
}

}  // namespace runir::tests
