#include "planning_fixtures.hpp"

#include <gtest/gtest.h>
#include <limits>
#include <memory>
#include <runir/datasets/state_graph.hpp>
#include <stdexcept>
#include <tuple>
#include <vector>
#include <tyr/planning/ground/successor_generator.hpp>
#include <tyr/planning/state_builder.hpp>
#include <tyr/planning/state_repository.hpp>
#include <utility>

namespace runir::tests
{

namespace
{
/// A state graph of one state, without transitions.
datasets::StateGraph<tyr::GroundTag> single_state_graph(tyr::planning::PackedStateView<tyr::GroundTag> state)
{
    using Graph = datasets::StateGraph<tyr::GroundTag>;
    using Edge = std::tuple<VertexIndex, VertexIndex, datasets::StateGraphEdgeLabel>;
    using Label = vertex_label_t<Graph>;
    const auto vertices = std::vector<Label> { Label { std::move(state) } };
    return Graph(urd::graph_data<Edge, Label> { 1, {}, {}, vertices });
}
}  // namespace

TEST(StateGraphTest, RejectsUnsupportedCostMode)
{
    auto context = make_gripper_ground_context();

    const auto initial_state = context->successor_generator->get_initial_node(*context->state_repository, *context->axiom_evaluator).get_state();
    const auto graph = single_state_graph(initial_state.pack());

    EXPECT_THROW(
        try {
            static_cast<void>(datasets::annotate_state_graph(*context, graph, static_cast<datasets::StateGraphCostMode>(255)));
        } catch (const std::runtime_error& error) {
            EXPECT_STREQ(error.what(), "Unsupported state graph cost mode: 255.");
            throw;
        },
        std::runtime_error);
}

TEST(StateGraphTest, AnnotatesGeneratedGraphWithReachabilityAndGoalMetadata)
{
    auto context = make_gripper_ground_context();
    const auto graph = datasets::generate_state_graph(*context);
    const auto annotated = datasets::annotate_state_graph(*context, *graph, datasets::StateGraphCostMode::UNIT_COST);
    const auto& forward_graph = *annotated;

    ASSERT_GT(forward_graph.num_vertices(), 0);

    auto num_initial = std::size_t { 0 };
    auto num_goal = std::size_t { 0 };
    auto num_alive = std::size_t { 0 };
    for (auto vertex : forward_graph.vertices())
    {
        const auto& label = forward_graph.vertex_value(vertex);
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

TEST(StateGraphTest, KeepsInitialVertexWhenStateLimitIsZero)
{
    auto context = make_gripper_ground_context();
    auto options = datasets::StateGraphGenerationOptions {};
    options.max_num_states = 0;

    const auto result = datasets::generate_state_graph_result(*context, options);

    EXPECT_EQ(result.status, tyr::planning::SearchStatus::OUT_OF_STATES);
    ASSERT_NE(result.graph, nullptr);
    const auto& graph = *result.graph;
    ASSERT_EQ(graph.num_vertices(), 1);
    EXPECT_EQ(graph.vertex_value(0).state,
              context->successor_generator->get_initial_node(*context->state_repository, *context->axiom_evaluator).get_state().pack());
    EXPECT_EQ(&graph.vertex_value(0).state.get_state_repository(), context->state_repository.get());
}

TEST(StateGraphTest, LabelsDoNotRetainUnpackedBuilders)
{
    // Labels borrow the context's repository: the caller keeps it alive.
    const auto context = make_gripper_ground_context();
    const auto retained = [&]
    {
        const urd::data<tyr::planning::State<tyr::GroundTag>>* released_builder = nullptr;
        auto graph = [&]
        {
            const auto node = context->successor_generator->get_initial_node(*context->state_repository, *context->axiom_evaluator);
            released_builder = &node.get_state().data();
            return single_state_graph(node.get_state().pack());
        }();
        {
            const auto recycled_builder = context->state_repository->get_state_builder();
            EXPECT_EQ(recycled_builder.get(), released_builder);
        }
        const auto annotated = datasets::annotate_state_graph(*context, graph, datasets::StateGraphCostMode::UNIT_COST);
        return annotated->vertex_value(0).state;
    }();

    EXPECT_EQ(retained.unpack().pack(), retained);
}

}  // namespace runir::tests
